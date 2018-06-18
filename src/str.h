#ifndef _INCLUDED_CASSM_STR
#define _INCLUDED_CASSM_STR

#include <string>
#include "types.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      String Utilities
// ----------------------------------------------------------------------------

std::string toLowerCase(const std::string& str);
std::string toUpperCase(const std::string& str);
int stoi(const std::string& str, int defaultValue = 0);
std::string trim(const std::string& str);

// ----------------------------------------------------------------------------
//      StringEncoding
// ----------------------------------------------------------------------------

enum class StringEncoding
{
  Petscii,
  Screen
};

std::string toString(StringEncoding encoding) noexcept;
Byte encode(StringEncoding encoding, Byte c) noexcept;
std::string encode(StringEncoding encoding, const std::string& str) noexcept;

}
#endif
