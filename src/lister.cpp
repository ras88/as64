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

  size_t maxFilenameLength = 0;
  for (const auto& node: context.statements)
  {
    auto length = node->pos().line()->filename().length();
    if (length > maxFilenameLength)
      maxFilenameLength = length;
  }

  const Line *prevLine = nullptr;
  for (const auto& node: context.statements)
  {
    auto range = node->range();
    const auto *line = node->pos().line();
    Offset offset = 0;
    do
    {
      snprintf(buf, sizeof(buf), "%s:%05d [+%04x] %04x: %s    %s\n",
               padRight(line->filename(), maxFilenameLength).c_str(), line->lineNumber(),
               range.start() + offset, node->pc() + offset, bytesToHex(range, offset),
               offset < 3 && line != prevLine ? node->sourceText().c_str() : "");
      s << buf;
      offset += 3;
    }
    while (offset < range.length());

    prevLine = line;
  }
}

}
