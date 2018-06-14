#include <iostream>
#include <sstream>
#include <cstring>
#include "error.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      SystemError
// ----------------------------------------------------------------------------

std::string SystemError::message() const noexcept
{
  return strerror(code_);
}

std::string SystemError::format() const noexcept
{
  std::stringstream s;
  if (! path_.empty())
    s << path_ << ": ";
  s << message();

  return s.str();
}

}
