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
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdarg>
#include "error.h"
#include "source.h"
#include "path.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Line
// ----------------------------------------------------------------------------

Line::Line(SourceStream& stream, int fileIndex, int lineNumber, std::string&& text) noexcept
  : stream_(stream), fileIndex_(fileIndex), lineNumber_(lineNumber), text_(std::move(text))
{
}

std::string Line::filename() const noexcept
{
  return stream_.filename(fileIndex_);
}

std::string Line::shortFilename() const noexcept
{
  return stream_.shortFilename(fileIndex_);
}

bool operator==(const Line& a, const Line& b) noexcept
{
  return a.fileIndex_ == b.fileIndex_ && a.lineNumber_ == b.lineNumber_;
}

bool operator<(const Line& a, const Line& b) noexcept
{
  return a.fileIndex_ == b.fileIndex_ ? a.lineNumber_ < b.lineNumber_ : a.fileIndex_ < b.fileIndex_;
}

// ----------------------------------------------------------------------------
//      SourceStream
// ----------------------------------------------------------------------------

void SourceStream::includeFile(const std::string& filename)
{
  auto normalizedFilename = normalizePath(filename);

  auto input = std::make_unique<std::ifstream>(normalizedFilename);
  if (! input->is_open())
    throw SystemError(normalizedFilename);

  if (std::find_if(std::begin(files_), std::end(files_),
                [=](const auto& info) { return info.filename == normalizedFilename; }) != std::end(files_))
    throw DuplicateIncludeError(normalizedFilename);

  int fileIndex = files_.size();
  files_.push_back({ normalizedFilename, basename(normalizedFilename) });
  sources_.emplace(fileIndex, std::move(input));
}

Line *SourceStream::nextLine()
{
  for ( ; ; )
  {
    if (sources_.empty())
      return nullptr;

    auto& source = sources_.top();
    std::string text;
    if (std::getline(*source.input, text))
    {
      auto line = std::make_unique<Line>(*this, source.fileIndex, ++ source.lineNumber, std::move(text));
      auto *p = line.get();
      lines_.push_back(std::move(line));
      return p;
    }

    if (source.input->bad())
    {
      auto filename = files_[source.fileIndex].filename;
      sources_.pop();
      throw SystemError(filename);
    }

    sources_.pop();
  }
}

// ----------------------------------------------------------------------------
//      SourcePos
// ----------------------------------------------------------------------------

std::string SourcePos::toString() const noexcept
{
  std::stringstream s;
  s << *this;
  return s.str();
}

std::ostream& operator<<(std::ostream& s, const SourcePos& pos) noexcept
{
  if (pos.line_)
  {
    auto filename = pos.line_->filename();
    if (! filename.empty())
      s << filename << ':';
    s << pos.line_->lineNumber() << ':';
    s << pos.offset_;
  }
  return s;
}

bool operator==(const SourcePos& a, const SourcePos& b) noexcept
{
  return a.line_ == b.line_ && a.offset_ == b.offset_;
}

bool operator<(const SourcePos& a, const SourcePos& b) noexcept
{
  if (! a.line_)
    return b.line_ ? true : false;
  if (! b.line_)
    return false;

  return *a.line_ == *b.line_ ? a.offset_ < b.offset_ : *a.line_ < *b.line_;
}

// ----------------------------------------------------------------------------
//      Token
// ----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& s, const Token& token) noexcept
{
  s << '[' << token.pos << "] ";
  switch (token.type)
  {
    case TokenType::End:
      s << "END";
      break;

    case TokenType::Identifier:
      s << "IDENTIFIER: " << token.text;
      break;

    case TokenType::Number:
      s << "NUMBER: " << token.number;
      break;

    case TokenType::Literal:
      s << "LITERAL: \"" << token.text << '"';
      break;

    case TokenType::Punctuator:
      s << "PUNCTUATION: " << token.punctuator;
      break;

    default:
      s << "UNKNOWN";
      break;
  }

  return s;
}

// ----------------------------------------------------------------------------
//      LineReader
// ----------------------------------------------------------------------------

LineReader::LineReader(const Line& line) noexcept
  : line_(line), offset_(0)
{
  unget_.type = TokenType::End;               // Indicates no token to unget
}

Token LineReader::nextToken()
{
  if (unget_.type != TokenType::End)
  {
    Token token = unget_;
    unget_.type = TokenType::End;
    return token;
  }

  int c;
  while (std::isspace(c = get()))
  {
  }

  Token token;
  if (c == -1 || c == ';')
  {
    if (c != -1)
      unget();
    token.type = TokenType::End;
    token.pos = { &line_, offset_ };
    return token;
  }

  token.pos = { &line_, offset_ - 1 };

  if (std::isalpha(c) || c == '_' || c == '\'')
  {
    token.text += c;
    while ((c = get()) == '$' || c == '_' || c == '\'' || std::isalnum(c))
      token.text += c;
    if (c != -1)
      unget();
    token.type = TokenType::Identifier;
    return token;
  }

  if (std::isdigit(c))
  {
    // Decimal constant
    int value = c - '0';
    while (std::isdigit(c = get()))
    {
      if (value >= 0xffffffff / 10)
        throw SourceError(token.pos, "Integer constant overflow");
      value = value * 10 + (c - '0');
    }
    if (c != -1)
      unget();
    token.type = TokenType::Number;
    token.number = value;
    return token;
  }
    
  if (c == '$')
  {
    // Hexadecimal constant
    int value = 0, digits;
    for (digits = 0; ; ++ digits)
    {
      c = get();
      if (c >= '0' && c <= '9')
        c -= '0';
      else if (c >= 'a' && c <= 'f')
        c -= 'a' - 10;
      else if (c >= 'A' && c <= 'F')
        c -= 'A' - 10;
      else if (c == -1)
        break;
      else
      {
        unget();
        break;
      }
      value = (value << 4) + c;
    }
    if (digits < 1)
      throw SourceError(token.pos, "Invalid hexadecimal constant");
    if (digits > 8)
      throw SourceError(token.pos, "Integer constant overflow");
    token.type = TokenType::Number;
    token.number = value;
    return token;
  }

  if (c == '%')
  {
    // Binary constant
    int value = 0, digits;
    for (digits = 0; ; ++ digits)
    {
      c = get();
      if (c == '0')
        c = 0;
      else if (c == '1')
        c = 1;
      else if (c == -1)
        break;
      else
      {
        unget();
        break;
      }
      value = (value << 1) + c;
    }
    if (digits < 1)
      throw SourceError(token.pos, "Invalid binary constant");
    if (digits > 32)
      throw SourceError(token.pos, "Integer constant overflow");
    token.type = TokenType::Number;
    token.number = value;
    return token;
  }

  if (c == '"')
  {
    while ((c = get()) != '"' && c != -1)
      token.text += c;
    token.type = TokenType::Literal;
    return token;
  }

  token.punctuator = c;
  token.type = TokenType::Punctuator;
  return token;
}

void LineReader::expectPunctuator(char c)
{
  auto token = nextToken();
  if (token.type != TokenType::Punctuator || token.punctuator != c)
    throwSourceError(token.pos, "Expected '%c'", c);
}

bool LineReader::optionalPunctuator(char c)
{
  auto token = nextToken();
  if (token.type == TokenType::Punctuator && token.punctuator == c)
    return true;
  unget(token);
  return false;
}

void LineReader::unget(Token& token) noexcept
{
  unget_ = token;
}

// ----------------------------------------------------------------------------
//      SourceError
// ----------------------------------------------------------------------------

SourceError::SourceError(SourcePos pos, const std::string& message, bool fatal) noexcept
  : message_(message), pos_(pos), fatal_(fatal)
{
}

std::string SourceError::format() const noexcept
{
  std::stringstream s;
  s << pos_ << ':' << message();
  return s.str();
}

void throwSourceError(SourcePos pos, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  char buf[1024];
  vsnprintf(buf, sizeof(buf), format, ap);
  throw SourceError(pos, buf);
}

void throwFatalSourceError(SourcePos pos, const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  char buf[1024];
  vsnprintf(buf, sizeof(buf), format, ap);
  throw SourceError(pos, buf, true);
}

// ----------------------------------------------------------------------------
//      DuplicateIncludeError
// ----------------------------------------------------------------------------

std::string DuplicateIncludeError::message() const noexcept
{
  std::stringstream s;
  s << "File '" << filename_ << "' has already been included";
  return s.str();
}

}
