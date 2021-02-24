#ifndef BACKEND_REQUIREMENTS_HPP
#define BACKEND_REQUIREMENTS_HPP

#include <memory>

// This struct represents requirements a Bbckend must meet
struct BackendRequirements {
  std::shared_ptr<bool> http;
  std::shared_ptr<bool> https;
  std::shared_ptr<long long> minSize;
  std::shared_ptr<bool> preserveName;
  std::shared_ptr<long long> maxRetention;
  std::shared_ptr<long long> minRetention;
  std::shared_ptr<long> maxDownloads;
};

// This struct represents the capabilities of a backend
struct BackendCapabilities {
  // Http support
  bool http;
  // Https support
  bool https;
  // The maximum filesize
  long long maxSize;
  // True if a backend is capable of preserving filenames, False if incapable, nullptr if both
  std::shared_ptr<bool> preserveName;
  // The minimum amount a file will be stored for, before autodeleting
  long long minRetention;
  // The maximum time a file can be stored for
  long long maxRetention;
  // If set, the file can be deleted after a maximum of this many downloads
  // It is assumed, that there is always an option to not delete files
  std::shared_ptr<long> maxDownloads;

  bool meetsRequirements(BackendRequirements requirements) const;
};
#endif
