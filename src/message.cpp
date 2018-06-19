#include <iostream>
#include <cstdarg>
#include "message.h"

namespace cassm
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
  s << (obj.severity == Severity::Error ? "error: " : "warning: ");
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

void MessageList::add(Severity severity, SourcePos pos, const std::string& summary, bool fatal) noexcept
{
  Message message{ severity, pos, summary };
  auto i = std::lower_bound(std::begin(messages_), std::end(messages_), message);
  messages_.insert(i, message);
  if (severity == Severity::Error)
    ++ errorCount_;
  else
    ++ warningCount_;
  if (fatal)
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
