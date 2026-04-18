#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

#include <gperftools/profiler.h>

namespace utils::profiler {

inline bool enabled() {
  const char *profile = std::getenv("OTHELLO_PROFILE");
  return profile != nullptr && std::string(profile) == "1";
}

inline std::string output_path(const char *filename) {
  const char *profile_dir = std::getenv("OTHELLO_PROFILE_DIR");
  if (profile_dir == nullptr || profile_dir[0] == '\0') {
    return filename;
  }

  return (std::filesystem::path(profile_dir) / filename).string();
}

inline void start(const char *filename) {
  if (!enabled()) {
    return;
  }

  const std::string path = output_path(filename);
  ProfilerStart(path.c_str());
}

inline void stop() {
  if (!enabled()) {
    return;
  }

  ProfilerStop();
}

inline void enable() {
  if (!enabled()) {
    return;
  }

  ProfilerEnable();
}

inline void disable() {
  if (!enabled()) {
    return;
  }

  ProfilerDisable();
}

}  // namespace utils::profiler
