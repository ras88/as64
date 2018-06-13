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
  std::stringstream s;
  if (path_ != "")
    s << "File \"" << path_ << "\": ";
  s << strerror(number_);

  return s.str();
}

}
