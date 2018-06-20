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

#include "enum.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      EnumTagsBase
// ----------------------------------------------------------------------------

std::string EnumTagsBase::fromValue(int value, const std::string& defaultTag) const
{
  const auto& iter = byId_.find(value);
  return iter != std::end(byId_) ? iter->second : std::string{defaultTag};
}

std::string EnumTagsBase::fromMask(int mask) const
{
  std::string result;
  for (const auto& item: byId_)
    if ((mask & item.first) != 0)
    {
      if (result != "")
        result += ", ";
      result += item.second;
    }
  return result;
}

int EnumTagsBase::fromName(const std::string& name, int defaultValue) const
{
  const auto& iter = byName_.find(name);
  return iter != std::end(byName_) ? iter->second : defaultValue;
}

}
