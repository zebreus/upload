#include "uploader.hpp"

Uploader::Uploader(const Settings& settings): settings(settings) {
  initializeBackends();
  if(settings.getMode() == Settings::Mode::List) {
    printAvailableBackends();
    quit::success();
  }
}

std::string Uploader::uploadFile(const File& file) {
  for(const std::shared_ptr<Backend>& backend : checkedBackends) {
    try {
      return uploadFile(file, backend);
    } catch(const std::runtime_error& e) {
      logger.log(Logger::Info) << "Failed to upload " << file.getName() << " to " << backend->getName() << ". " << e.what() << '\n';
    } catch(...) {
      logger.log(Logger::Info) << "Unexpected error while uploading " << file.getName() << " to " << backend->getName() << "." << '\n';
    }
  }

  while(checkNextBackend()) {
    std::shared_ptr<Backend> backend = checkedBackends.back();
    try {
      return uploadFile(file, backend);
    } catch(const std::runtime_error& e) {
      logger.log(Logger::Info) << "Failed to upload " << file.getName() << " to " << backend->getName() << ". " << e.what() << '\n';
    } catch(...) {
      logger.log(Logger::Info) << "Unexpected error while uploading " << file.getName() << " to " << backend->getName() << "." << '\n';
    }
  }

  std::stringstream message;
  message << "Failed to upload " << file.getName() << " to any backend.";
  if(settings.getContinue()) {
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
  for(const std::shared_ptr<Backend>& backend : checkedBackends) {
    logger.log(Logger::Print) << backend->getName() << '\n';
  }
  for(const std::shared_ptr<Backend>& backend : backends) {
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
        logger.log(Logger::Topic::Fatal) << "Unable to find requested backend '" << backendName << "'. Maybe check for a typo in its name.";
        quit::invalidCliUsage();
      }
      unmatchedBackends = nextBackends;
    }
    loadedBackends = orderedBackends;
  }

  for(const std::shared_ptr<Backend>& backend : loadedBackends) {
    if(backend->staticSettingsCheck(settings.getBackendRequirements())) {
      logger.log(Logger::Debug) << backend->getName() << " has all required features." << '\n';
      backends.push_back(backend);
    } else {
      logger.log(Logger::Debug) << backend->getName() << " does not have all required features." << '\n';
    }
  }
}

bool Uploader::checkNextBackend() {
  std::promise<std::shared_ptr<Backend>> nextBackendPromise;
  std::future<std::shared_ptr<Backend>> future = nextBackendPromise.get_future();
  checkNextBackend(nextBackendPromise);
  try {
    std::shared_ptr<Backend> nextBackend = future.get();
    checkedBackends.push_back(nextBackend);
    return true;
  } catch(const std::runtime_error& e) {
    logger.log(Logger::Debug) << e.what() << '\n';
    return false;
  }
}

void Uploader::checkNextBackend(std::promise<std::shared_ptr<Backend>>& promise) {
  if(backends.empty()) {
    promise.set_exception(std::make_exception_ptr(std::runtime_error("There is no backend matching your requirements")));
    return;
  }
  std::shared_ptr<Backend> nextBackend = backends.front();
  backends.pop_front();
  nextBackend->dynamicSettingsCheck(
      settings.getBackendRequirements(),
      [this, &promise, &nextBackend]() {
        try {
          promise.set_value(nextBackend);
        } catch(const std::exception& e) {
          logger.log(Logger::Fatal) << "Failed to set promise value. If your are compiling this code yourself, you probably forgot to "
                                       "enable threads . Try '-pthread'."
                                    << '\n';
          quit::unexpectedFailure();
        }
      },
      [this, &promise](const std::string& message) {
        checkNextBackend(promise);
      },
      200);
}
