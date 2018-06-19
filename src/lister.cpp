#include <iostream>
#include <algorithm>
#include <cstdio>
#include "lister.h"
#include "context.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Lister
// ----------------------------------------------------------------------------

static const char *bytesToHex(CodeRange range, Offset offset)
{
  static char buf[32];
  auto count = std::min(3, range.length() - offset);
  switch (count)
  {
    case 1:
      snprintf(buf, sizeof(buf), "%02x      ", static_cast<int>(range[offset]));
      break;

    case 2:
      snprintf(buf, sizeof(buf), "%02x %02x   ", static_cast<int>(range[offset]), static_cast<int>(range[offset + 1]));
      break;

    case 3:
      snprintf(buf, sizeof(buf), "%02x %02x %02x", static_cast<int>(range[offset]), static_cast<int>(range[offset + 1]),
               static_cast<int>(range[offset + 2]));
      break;

    default:
      snprintf(buf, sizeof(buf), "        ");
      break;
  }
  return buf;
}

void list(std::ostream& s, Context& context)
{
  char buf[1024];

  for (const auto& node: context.statements)
  {
    auto range = node->range();
    snprintf(buf, sizeof(buf), "%04x [+%04x]  %s    %s\n",
             node->pc(), range.start(), bytesToHex(range, 0), node->sourceText().c_str());
    s << buf;

    for (Offset offset = 3; offset < range.length(); offset += 3)
    {
      snprintf(buf, sizeof(buf), "%04x [+%04x]  %s\n",
               node->pc(), range.start() + offset, bytesToHex(range, offset));
      s << buf;
    }
  }
}

}
