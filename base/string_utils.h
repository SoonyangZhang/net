#pragma once
#include <string>
#include <sstream>
namespace basic{
template <typename... Args>
inline std::string StrCatImpl(const Args&... args) {
  std::ostringstream oss;
  int dummy[] = {1, (oss << args, 0)...};
  static_cast<void>(dummy);
  return oss.str();
}
// Merges given strings or numbers with no delimiter.
template <typename... Args>
inline std::string StrCat(const Args&... args) {
  return StrCatImpl(std::forward<const Args&>(args)...);
}
}