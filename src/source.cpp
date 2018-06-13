#include <iostream>
#include <fstream>
#include <sstream>
#include "error.h"
#include "source.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Line
// ----------------------------------------------------------------------------

Line::Line() noexcept
  : lineNumber_(-1)
{
}

Line::Line(const std::string& filename, int lineNumber, std::string&& text) noexcept
  : filename_(filename), lineNumber_(lineNumber), text_(std::move(text))
{
}

// ----------------------------------------------------------------------------
//      SourceStream
// ----------------------------------------------------------------------------

void SourceStream::includeFile(const std::string& filename)
{
  auto input = std::make_unique<std::ifstream>(filename);
  if (! input->is_open())
    throw SystemError(filename);

  sources_.emplace(filename, std::move(input));
}

Line SourceStream::nextLine()
{
  for ( ; ; )
  {
    if (sources_.empty())
      return { };

    auto& source = sources_.top();
    std::string line;
    if (std::getline(*source.input, line))
      return { source.filename, ++ source.lineNumber, std::move(line) };

    if (source.input->bad())
    {
      auto filename = source.filename;
      sources_.pop();
      throw SystemError(filename);
    }

    sources_.pop();
  }
}

// ----------------------------------------------------------------------------
//      Token
// ----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& s, const Token& token) noexcept
{
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
    token.type = TokenType::End;
    token.offset = offset_;
    return token;
  }

  token.offset = offset_ - 1;


  if (std::isalpha(c) || c == '_' || c == '\'')
  {
    token.text += c;
    while ((c = get()) == '$' || c == '_' || c == '\'' || std::isalpha(c))
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
        throw -1;                                             // TODO
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
      throw -1;                                               // TODO
    if (digits > 8)
      throw -1;                                               // TODO
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
      throw -1;                                               // TODO
    if (digits > 32)
      throw -1;                                               // TODO
    token.type = TokenType::Number;
    token.number = value;
    return token;
  }

  if (c == '"')
  {
    // TODO: did PowerAssembler support any escapes?
    while ((c = get()) != '"' && c != -1)
      token.text += c;
    if (c == -1)
      throw -1;                                               // TODO
    token.type = TokenType::Literal;
    return token;
  }

  token.punctuator = c;
  token.type = TokenType::Punctuator;
  return token;
}

void LineReader::unget(Token& token)
{
  unget_ = token;
}

}
