#include <iostream>
#include "types.h"
#include "enum.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      ByteSelector
// ----------------------------------------------------------------------------

static EnumTags<ByteSelector> g_byteSelectorTags =
{
  { ByteSelector::Unspecified,        "Unspecified" },
  { ByteSelector::Low,                "Low" },
  { ByteSelector::High,               "High" }
};

std::string toString(ByteSelector selector) noexcept
{
  return g_byteSelectorTags.fromValue(selector);
}

// ----------------------------------------------------------------------------
//      BranchDirection
// ----------------------------------------------------------------------------

static EnumTags<BranchDirection> g_branchDirectionTags =
{
  { BranchDirection::Forward,         "Forward" },
  { BranchDirection::Backward,        "Backward" }
};

std::string toString(BranchDirection direction) noexcept
{
  return g_branchDirectionTags.fromValue(direction);
}

// ----------------------------------------------------------------------------
//      StringEncoding
// ----------------------------------------------------------------------------

static EnumTags<StringEncoding> g_stringEncodingTags =
{
  { StringEncoding::Petscii,          "PETSCII" },
  { StringEncoding::Screen,           "Screen" }
};

std::string toString(StringEncoding encoding) noexcept
{
  return g_stringEncodingTags.fromValue(encoding);
}

}
