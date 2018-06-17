#ifndef _INCLUDED_CASSM_SOURCE_H
#define _INCLUDED_CASSM_SOURCE_H

#include <string>
#include <memory>
#include <istream>
#include <stack>
#include <vector>
#include "types.h"
#include "error.h"

namespace cassm
{

class SourceStream;

// ----------------------------------------------------------------------------
//      Line
// ----------------------------------------------------------------------------

class Line
{
public:
  Line(SourceStream& stream, int fileIndex, int lineNumber, std::string&& text) noexcept;

  std::string filename() const noexcept;
  int lineNumber() const noexcept { return lineNumber_; }

  size_t length() const noexcept { return text_.length(); }
  std::string text() const noexcept { return text_; }
  char operator[](int index) const noexcept { return text_[index]; }

  friend bool operator==(const Line& a, const Line& b) noexcept;
  friend bool operator<(const Line& a, const Line& b) noexcept;

private:
  SourceStream& stream_;
  int fileIndex_;
  int lineNumber_;
  std::string text_;
};

// ----------------------------------------------------------------------------
//      SourceStream
// ----------------------------------------------------------------------------

class SourceStream
{
public:
  Line *nextLine();
  void includeFile(const std::string& filename);

  std::string filename(int fileIndex) const noexcept { return filenames_[fileIndex]; }

private:
  struct Source
  {
    Source(int fileIndex, std::unique_ptr<std::istream> input)
      : fileIndex(fileIndex), input(std::move(input)), lineNumber(0) { }

    int fileIndex;
    std::unique_ptr<std::istream> input;
    int lineNumber;
  };

  std::stack<Source> sources_;
  std::vector<std::string> filenames_;
  std::vector<std::unique_ptr<Line>> lines_;
};

// ----------------------------------------------------------------------------
//      SourcePos
// ----------------------------------------------------------------------------

class SourcePos
{
public:
  SourcePos(const Line *line = nullptr, int offset = 0) noexcept : line_(line), offset_(offset) { }
  SourcePos(const SourcePos& other) noexcept = default;
  SourcePos& operator=(const SourcePos& other) = default;

  bool isValid() const noexcept { return line_; }
  const Line *line() const noexcept { return line_; }
  int offset() const noexcept { return offset_; }

  std::string filename() const noexcept { return line_ ? line_->filename() : ""; }
  int lineNumber() const noexcept { return line_ ? line_->lineNumber() : 0; }

  std::string toString() const noexcept;
  friend std::ostream& operator<<(std::ostream& s, const SourcePos& pos) noexcept;

  friend bool operator==(const SourcePos& a, const SourcePos& b) noexcept;
  friend bool operator<(const SourcePos& a, const SourcePos& b) noexcept;

private:
  const Line *line_;
  int offset_;
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
  SourcePos pos;
  TokenType type;
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
  void expectPunctuator(char c);
  bool optionalPunctuator(char c);
  void unget(Token& token) noexcept;

  const Line& line() const noexcept { return line_; }

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
  SourceError(SourcePos pos, const std::string& message) noexcept;

  const char *what() const noexcept override { return "Source Error"; }
  std::string message() const noexcept override { return message_; }
  std::string format() const noexcept override;

  SourcePos pos() const noexcept { return pos_; }

private:
  SourcePos pos_;
  std::string message_;
};

[[noreturn]] void throwSourceError(SourcePos pos, const char *format, ...) CHECK_FORMAT(2, 3);

}
#endif
