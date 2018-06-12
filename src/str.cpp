#include <cstring>
#include <exception>
#include <algorithm>
#include "str.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      String Utilities
// ----------------------------------------------------------------------------

std::string toLowerCase(const std::string& str)
{
  std::string result{str};
  for (auto& c: result)
    c = std::tolower(c);
  return result;
}

std::string toUpperCase(const std::string& str)
{
  std::string result{str};
  for (auto& c: result)
    c = std::toupper(c);
  return result;
}

int stoi(const std::string& str, int defaultValue)
{
  try
  {
    return std::stoi(str);
  }
  catch (std::exception&)
  {
    return defaultValue;
  }
}

std::string trim(const std::string& str)
{
  const char *start = str.data(), *end = start + str.length();

  while (start < end && std::isspace(*start))
    ++ start;

  while (end > start && std::isspace(*(end-1)))
    -- end;

  return std::string(start, end - start);
}

}
