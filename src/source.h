#ifndef _INCLUDED_CASSM_SOURCE_H
#define _INCLUDED_CASSM_SOURCE_H

#include <string>
#include <memory>
#include <istream>
#include <stack>
#include "error.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Line
// ----------------------------------------------------------------------------

class Line
{
public:
  Line() noexcept;
  Line(const std::string& filename, int lineNumber, std::string&& text) noexcept;

  bool isValid() const noexcept { return lineNumber_ != -1; }

  std::string filename() const noexcept { return filename_; }
  int lineNumber() const noexcept { return lineNumber_; }

  size_t length() const noexcept { return text_.length(); }
  std::string text() const noexcept { return text_; }
  char operator[](int index) const noexcept { return text_[index]; }

private:
  std::string filename_;
  int lineNumber_;
  std::string text_;
};

// ----------------------------------------------------------------------------
//      SourceStream
// ----------------------------------------------------------------------------

class SourceStream
{
public:
  Line nextLine();

  void includeFile(const std::string& filename);

private:
  struct Source
  {
    Source(const std::string& filename, std::unique_ptr<std::istream> input)
      : filename(filename), input(std::move(input)), lineNumber(0) { }

    std::string filename;
    std::unique_ptr<std::istream> input;
    int lineNumber;
  };

  std::stack<Source> sources_;
};

// ----------------------------------------------------------------------------
//      TokenType
// ----------------------------------------------------------------------------

enum class TokenType
{
  End,
  Identifier,
  Number,
  Literal,
  Punctuator
};

// ----------------------------------------------------------------------------
//      Token
// ----------------------------------------------------------------------------

struct Token
{
  TokenType type;
  int offset;
  std::string text;
  union
  {
    int number;
    char punctuator;
  };
};

std::ostream& operator<<(std::ostream& s, const Token& token) noexcept;

// ----------------------------------------------------------------------------
//      LineReader
// ----------------------------------------------------------------------------

class LineReader
{
public:
  LineReader(const Line& line) noexcept;

  Token nextToken();
  void unget(Token& token);

private:
  int get() noexcept { return offset_ == line_.length() ? -1 : line_[offset_++]; }
  void unget() noexcept { -- offset_; }

  const Line& line_;
  int offset_;
  Token unget_;
};

// ----------------------------------------------------------------------------
//      SourceError
// ----------------------------------------------------------------------------

class SourceError : public Error
{
public:
  SourceError(const std::string& message) noexcept;

  const char *what() const noexcept override { return "Source Error"; }
  std::string message() const noexcept override { return message_; }
  std::string format() const noexcept override;

  std::string filename() const noexcept { return filename_; }
  int lineNumber() const noexcept { return lineNumber_; }
  void setLocation(const Line& line) noexcept;

private:
  std::string message_;
  std::string filename_;
  int lineNumber_;
};

}
#endif
