#ifndef _INCLUDED_CASSM_TYPES_H
#define _INCLUDED_CASSM_TYPES_H

#include <cstdint>

namespace cassm
{

using Address = int;

// ----------------------------------------------------------------------------
//      ByteSelector
// ----------------------------------------------------------------------------

enum class ByteSelector
{
  Unspecified,
  Low,
  High
};


}
#endif
