#include "uploader.hpp"

Uploader::Uploader(const Settings& settings): settings(settings) {
  initializeBackends();
  if(settings.getMode() == Settings::Mode::List) {
    printAvailableBackends();
    quit::success();
  }
}

std::string Uploader::uploadFile(const File& file) {
  //This loop does not need to lock the mutex
  while(checkedBackends.empty() && !backends.empty()) {
    backends.front().get();
    backends.pop();
  }

  size_t pos;
  //This loop does not need to lock the mutex here
  for(pos = 0;pos < checkedBackends.size() ;pos++) {

    std::shared_ptr<Backend> backend;
    {
      std::unique_lock<std::mutex> lock(checkedBackendsMutex);
      backend = checkedBackends[pos];
    }
    try {
      return uploadFile(file, backend);
    } catch(const std::runtime_error& e) {
      logger.log(Logger::Info) << "Failed to upload " << file.getName() << " to " << backend->getName() << ". " << e.what() << '\n';
    } catch(...) {
      logger.log(Logger::Info) << "Unexpected error while uploading " << file.getName() << " to " << backend->getName() << "." << '\n';
    }

    //This loop does not need to lock the mutex
    while(pos == checkedBackends.size()-1 && !backends.empty()) {
      backends.front().get();
      backends.pop();
    }
  }

  std::stringstream message;
  message << "Failed to upload " << file.getName() << " to any backend.";
  if(settings.getContinueUploading()) {
    throw std::runtime_error(message.str());
  } else {
    logger.log(Logger::Fatal) << message.str() << '\n';
    quit::failedToUpload();
  }
}

std::string Uploader::uploadFile(const File& file, const std::shared_ptr<Backend>& backend) {
  if(!backend->staticFileCheck(settings.getBackendRequirements(), file)) {
    std::stringstream message;
    message << backend->getName() << " does not accept files like " << file.getName() << ".";
    throw std::runtime_error(message.str());
  }
  std::promise<std::string> urlPromise;
  backend->uploadFile(
      settings.getBackendRequirements(),
      file,
      [this, &urlPromise](const std::string& url) {
        try {
          urlPromise.set_value(url);
        } catch(const std::exception& e) {
          logger.log(Logger::Fatal) << "Failed to set urlPromise value. If your are compiling this code yourself, you probably forgot to "
                                       "enable threads . Try linking with '-pthread'."
                                    << '\n';
          quit::unexpectedFailure();
        }
      },
      [&urlPromise](const std::string& message) {
        urlPromise.set_exception(std::make_exception_ptr(std::runtime_error(message)));
      });

  return urlPromise.get_future().get();
}

void Uploader::printAvailableBackends() {
  while(!backends.empty()) {
    backends.front().get();
    backends.pop();
  }
  std::unique_lock<std::mutex> lock(checkedBackendsMutex);
  for(const std::shared_ptr<Backend>& backend : checkedBackends) {
    logger.log(Logger::Print) << backend->getName() << '\n';
  }
}

void Uploader::initializeBackends() {
  std::vector<std::shared_ptr<Backend>> loadedBackends = loadBackends();

  // Find all backends with a requested name, possibly multiple with the same name, but none twice
  if(!settings.getRequestedBackends().empty()) {
    std::vector<std::shared_ptr<Backend>> unmatchedBackends = loadedBackends;
    std::vector<std::shared_ptr<Backend>> orderedBackends;
    std::vector<std::shared_ptr<Backend>> nextBackends;
    for(const std::string& backendName : settings.getRequestedBackends()) {
      bool found = false;
      for(const std::shared_ptr<Backend>& backend : unmatchedBackends) {
        if(backend->getName() == backendName) {
          orderedBackends.push_back(backend);
          found = true;
        } else {
          nextBackends.push_back(backend);
        }
      }
      if(!found) {
        logger.log(Logger::Topic::Fatal) << "Unable to find requested backend '" << backendName << "'. Maybe check for a typo in its name."
                                         << '\n';
        quit::invalidCliUsage();
      }
      unmatchedBackends = nextBackends;
    }
    loadedBackends = orderedBackends;
  }

  for(const std::string& name : settings.getExcludedBackends()) {
    for(auto backend = loadedBackends.rbegin(); backend != loadedBackends.rend(); ++backend) {
      if(backend->get()->getName() == name) {
        loadedBackends.erase(std::next(backend).base());
      }
    }
  }

  std::launch launchPolicy;
  if(settings.getCheckWhenNeeded()){
    launchPolicy = std::launch::deferred;
  }else{
    launchPolicy = std::launch::async;
  }

  for(const std::shared_ptr<Backend>& backend : loadedBackends) {
    if(backend->staticSettingsCheck(settings.getBackendRequirements())) {
      logger.log(Logger::Debug) << backend->getName() << " has all required features." << '\n';
      backends.emplace(std::async(launchPolicy, &Uploader::checkBackend, this, backend));
    } else {
      logger.log(Logger::Debug) << backend->getName() << " does not have all required features." << '\n';
    }
  }
}

void Uploader::checkBackend(const std::shared_ptr<Backend>& backend) {
  backend->dynamicSettingsCheck(
      settings.getBackendRequirements(),
      [this, &backend]() {
          std::unique_lock<std::mutex> lock(checkedBackendsMutex);
          checkedBackends.push_back(backend);
      },
      [this](const std::string& message) {
        logger.log(Logger::Info) << "Failed to check backend: " << message << "." << '\n';
      },
      200);
}
