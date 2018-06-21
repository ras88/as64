#ifndef _INCLUDED_AS64_ENUM_H
#define _INCLUDED_AS64_ENUM_H

#include <string>
#include <map>
#include <algorithm>
#include <initializer_list>

namespace as64
{

// ----------------------------------------------------------------------------
//      EnumTagsBase
// ----------------------------------------------------------------------------

class EnumTagsBase
{
public:
  std::string fromValue(int value, const std::string& defaultTag = "") const;
  std::string fromMask(int mask) const;

  int fromName(const std::string& name, int defaultValue = 0) const;
  template<typename StringContainer> int maskFromNames(StringContainer names) const;

protected:
  std::map<int, std::string> byId_;
  std::map<std::string, int, std::less<>> byName_;
};

template<typename StringContainer> int EnumTagsBase::maskFromNames(StringContainer names) const
{
  int result = 0;
  for (const auto& name: names)
    result |= fromName(name, 0);
  return result;
}

// ----------------------------------------------------------------------------
//      EnumTags
// ----------------------------------------------------------------------------

template<typename T> class EnumTags : public EnumTagsBase
{
public:
  EnumTags(std::initializer_list<std::pair<T, const char *>> items)
  {
    for (const auto& item: items)
    {
      byId_[static_cast<int>(item.first)] = item.second;
      byName_[item.second] = static_cast<int>(item.first);
    }
  }

  std::string fromValue(T value, const std::string& defaultTag = "") const
  {
    return EnumTagsBase::fromValue(static_cast<int>(value), defaultTag);
  }
  std::string fromMask(T mask) const { return EnumTagsBase::fromMask(static_cast<int>(mask)); }

  T fromName(const std::string& name, T defaultValue = static_cast<T>(0)) const
  {
    return static_cast<T>(EnumTagsBase::fromName(name, static_cast<int>(defaultValue)));
  }
  
  template<typename StringContainer> T maskFromNames(StringContainer names) const
  {
    return static_cast<T>(EnumTagsBase::maskFromNames(names));
  }
};

}
#endif
