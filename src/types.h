#ifndef _INCLUDED_CASSM_TYPES_H
#define _INCLUDED_CASSM_TYPES_H

#include <string>
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

std::string toString(ByteSelector selector) noexcept;

// ----------------------------------------------------------------------------
//      BranchDirection
// ----------------------------------------------------------------------------

enum class BranchDirection
{
  Forward,
  Backward
};

std::string toString(BranchDirection direction) noexcept;

// ----------------------------------------------------------------------------
//      StringEncoding
// ----------------------------------------------------------------------------

enum class StringEncoding
{
  Petscii,
  Screen
};

std::string toString(StringEncoding encoding) noexcept;

}
#endif
