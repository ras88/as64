// Copyright (c) 2018 Robert A. Stoerrle
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
// OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

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
