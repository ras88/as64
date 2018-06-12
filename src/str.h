#ifndef _INCLUDED_CASSM_STR
#define _INCLUDED_CASSM_STR

#include <string>

namespace cassm
{

// ----------------------------------------------------------------------------
//      String Utilities
// ----------------------------------------------------------------------------

std::string toLowerCase(const std::string& str);
std::string toUpperCase(const std::string& str);
int stoi(const std::string& str, int defaultValue = 0);
std::string trim(const std::string& str);

}
#endif
