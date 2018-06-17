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

Maybe<Byte> select(ByteSelector selector, Word value) noexcept
{
  switch (selector)
  {
    case ByteSelector::Low:
      return value & 0xff;

    case ByteSelector::High:
      return value >> 8;

    default:
      if (value > 0xff)
        return nullptr;
      return value;
  }
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

// ----------------------------------------------------------------------------
//      Label
// ----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& s, const Label& label)
{
  switch (label.type_)
  {
    case LabelType::Empty:
      s << "<empty>";
      break;

    case LabelType::Symbolic:
      s << label.name_;
      break;

    case LabelType::Temporary:
      s << "<temporary>";
      break;

    case LabelType::TemporaryForward:
      s << "<temporary-forward>";
      break;

    case LabelType::TemporaryBackward:
      s << "<temporary-backward>";
      break;
  }

  return s;
}

}
