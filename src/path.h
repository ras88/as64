#ifndef _INCLUDED_AS64_PATH_H
#define _INCLUDED_AS64_PATH_H

#include <string>
#include "str.h"

namespace as64
{

#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STRING "\\"
#else
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#endif

// ----------------------------------------------------------------------------
//      Path Utilities
// ----------------------------------------------------------------------------

std::string dirname(const std::string& path) noexcept;
std::string basename(const std::string& path) noexcept;
std::string joinPath(const std::string& a, const std::string& b) noexcept;
std::string normalizePath(const std::string& path) noexcept;

}
#endif
