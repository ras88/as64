#ifndef _INCLUDED_CASSM_STR_H
#define _INCLUDED_CASSM_STR_H

#include <string>
#include <vector>
#include <functional>
#include "types.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      String Utilities
// ----------------------------------------------------------------------------

std::string toLowerCase(const std::string& str) noexcept;
std::string toUpperCase(const std::string& str) noexcept;
int stoi(const std::string& str, int defaultValue = 0) noexcept;
std::string trim(const std::string& str) noexcept;
std::string padLeft(const std::string& str, size_t width) noexcept;
std::string padRight(const std::string& str, size_t width) noexcept;
std::string join(const std::vector<std::string> items, const std::string& separator) noexcept;
void split(const std::string& str, char separator, std::function<void (std::string str)> fn) noexcept;

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
