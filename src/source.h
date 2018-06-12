#ifndef _INCLUDED_CASSM_SOURCE_H
#define _INCLUDED_CASSM_SOURCE_H

#include <vector>
#include <string>

namespace cassm
{

// ----------------------------------------------------------------------------
//      Line
// ----------------------------------------------------------------------------

class Line
{
public:
  Line(int index, const char *text, size_t length);

  int index() const { return index_; }

  size_t length() const { return text_.length(); }
  std::string text() const { return text_; }
  char operator[](int index) const { return text_[index]; }

private:
  int index_;
  std::string text_;
};

// ----------------------------------------------------------------------------
//      Source
// ----------------------------------------------------------------------------

class Source
{
public:
  Source(const std::string& filename, const std::string& text);

  const auto begin() const { return lines_.begin(); }
  const auto end() const { return lines_.end(); }
  const auto rbegin() const { return lines_.rbegin(); }
  const auto rend() const { return lines_.rend(); }

private:
  std::string filename_;
  std::vector<Line> lines_;
};

Source load(const std::string& filename);

// ----------------------------------------------------------------------------
//      TokenType
// ----------------------------------------------------------------------------

enum class TokenType
{
  End,
  Identifier,
  Number,
  Literal,
  Directive,
  Punctuation
};

// ----------------------------------------------------------------------------
//      Token
// ----------------------------------------------------------------------------

struct Token
{
  TokenType type;
  int offset;
  std::string text;
  int number;
};

std::ostream& operator<<(std::ostream& s, const Token& token);

// ----------------------------------------------------------------------------
//      LineReader
// ----------------------------------------------------------------------------

class LineReader
{
public:
  LineReader(const Line& line);

  Token nextToken();

private:
  int get() { return offset_ == line_.length() ? -1 : line_[offset_++]; }
  void unget() { -- offset_; }

  const Line& line_;
  int offset_;
};

}
#endif
