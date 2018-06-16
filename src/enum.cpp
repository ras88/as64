#include "enum.h"

namespace cassm
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
