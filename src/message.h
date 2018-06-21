#ifndef _INCLUDED_AS64_MESSAGE_H
#define _INCLUDED_AS64_MESSAGE_H

#include <string>
#include <ostream>
#include <vector>
#include "types.h"
#include "source.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      Severity
// ----------------------------------------------------------------------------

enum class Severity
{
  Warning,
  Error,
  FatalError
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
  bool hasFatalError() const noexcept { return fatal_; }

  void add(Severity severity, SourcePos pos, const std::string& summary) noexcept;
  void error(SourcePos pos, const char *format, ...) noexcept CHECK_FORMAT(3, 4);
  void warning(SourcePos pos, const char *format, ...) noexcept CHECK_FORMAT(3, 4);

  friend std::ostream& operator<<(std::ostream& s, const MessageList& obj) noexcept;

private:
  std::vector<Message> messages_;
  int errorCount_;
  int warningCount_;
  bool fatal_;
};

}
#endif
