// Copyright (c) 2018 Robert A. Stoerrle
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
// OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <iostream>
#include <cstdarg>
#include "message.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      Message
// ----------------------------------------------------------------------------

bool operator<(const Message& a, const Message& b) noexcept
{
  return a.severity == b.severity ? a.pos < b.pos : a.severity >= b.severity;
}

std::ostream& operator<<(std::ostream& s, const Message& obj) noexcept
{
  s << obj.pos << ": ";
  s << (obj.severity == Severity::Error || obj.severity == Severity::FatalError ? "error: " : "warning: ");
  s << obj.summary;
  if (obj.pos.isValid())
  {
    s << std::endl;
    s << "  " << obj.pos.line()->text() << std::endl;
    s << "  " << std::string(obj.pos.offset(), ' ') << '^';
  }
  return s;
}

// ----------------------------------------------------------------------------
//      MessageList
// ----------------------------------------------------------------------------

MessageList::MessageList() noexcept
  : errorCount_(0), warningCount_(0), fatal_(false)
{
}

void MessageList::add(Severity severity, SourcePos pos, const std::string& summary) noexcept
{
  Message message{ severity, pos, summary };
  auto i = std::lower_bound(std::begin(messages_), std::end(messages_), message);
  messages_.insert(i, message);
  if (severity == Severity::Error || severity == Severity::FatalError)
    ++ errorCount_;
  else
    ++ warningCount_;
  if (severity == Severity::FatalError)
    fatal_ = true;
}

void MessageList::error(SourcePos pos, const char *format, ...) noexcept
{
  va_list ap;
  va_start(ap, format);

  char buf[1024];
  vsnprintf(buf, sizeof(buf), format, ap);
  add(Severity::Error, pos, buf);
}

void MessageList::warning(SourcePos pos, const char *format, ...) noexcept
{
  va_list ap;
  va_start(ap, format);

  char buf[1024];
  vsnprintf(buf, sizeof(buf), format, ap);
  add(Severity::Warning, pos, buf);
}

std::ostream& operator<<(std::ostream& s, const MessageList& obj) noexcept
{
  for (size_t index = 0; index < obj.messages_.size(); ++ index)
  {
    if (index)
      s << std::endl;
    s << obj.messages_[index];
  }
  return s;
}

}
