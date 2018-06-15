#ifndef _INCLUDED_CASSM_MESSAGE_H
#define _INCLUDED_CASSM_MESSAGE_H

#include <string>
#include <ostream>
#include <vector>
#include "source.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Severity
// ----------------------------------------------------------------------------

enum class Severity
{
  Warning,
  Error
};

// ----------------------------------------------------------------------------
//      Message
// ----------------------------------------------------------------------------

struct Message
{
  Severity severity;
  SourcePos pos;
  std::string summary;
};

bool operator<(const Message& a, const Message& b) noexcept;
std::ostream& operator<<(std::ostream& s, const Message& obj) noexcept;

// ----------------------------------------------------------------------------
//      MessageList
// ----------------------------------------------------------------------------

class MessageList
{
public:
  MessageList() noexcept;

  int count() const noexcept { return messages_.size(); }
  int errorCount() const noexcept { return errorCount_; }
  int warningCount() const noexcept { return warningCount_; }

  void add(Severity severity, SourcePos pos, const std::string& summary) noexcept;
  void error(SourcePos pos, const char *format, ...) noexcept;
  void warning(SourcePos pos, const char *format, ...) noexcept;

  friend std::ostream& operator<<(std::ostream& s, const MessageList& obj) noexcept;

private:
  std::vector<Message> messages_;
  int errorCount_;
  int warningCount_;
};

}
#endif
