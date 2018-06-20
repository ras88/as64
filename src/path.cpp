#include <iostream>
#include "path.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Path Utilities
// ----------------------------------------------------------------------------

std::string dirname(const std::string& path) noexcept
{
  auto str = normalizePath(path);
  auto pos = str.find_last_of(PATH_SEPARATOR);
  if (pos == std::string::npos)
    return ".";
  if (pos == 0)
    return PATH_SEPARATOR_STRING;
  return str.substr(0, pos);
}

std::string basename(const std::string& path) noexcept
{
  auto str = normalizePath(path);
  if (str == PATH_SEPARATOR_STRING)
    return str;
  auto pos = str.find_last_of(PATH_SEPARATOR);
  if (pos == std::string::npos)
    return str;
  return str.substr(pos + 1);
}

std::string joinPath(const std::string& a, const std::string& b) noexcept
{
  if (a.empty())
    return normalizePath(b);
  if (b.empty())
    return normalizePath(a);
  return normalizePath(a + PATH_SEPARATOR_STRING + b);
}

std::string normalizePath(const std::string& path) noexcept
{
  std::vector<std::string> parts;
  split(path, PATH_SEPARATOR, [&](auto part)
  {
    if (part == "")
    {
      if (parts.empty())
        parts.push_back(part);
    }
    else if (part == "..")
    {
      if (parts.empty() || parts.back() == "..")
        parts.push_back(part);
      else if (parts.size() != 1 || parts[0] != "")
        parts.pop_back();
    }
    else if (part != ".")
      parts.push_back(part);
  });
  if (parts.empty())
    return ".";
  if (parts.size() == 1 && parts.front() == "")
    return "/";
  return join(parts, PATH_SEPARATOR_STRING);
}

}
