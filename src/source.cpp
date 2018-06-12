#include <iostream>
#include <fstream>
#include <sstream>
#include "source.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Line
// ----------------------------------------------------------------------------

Line::Line(int index, const char *text, size_t length)
  : index_(index), text_(text, length)
{
}

// ----------------------------------------------------------------------------
//      Source
// ----------------------------------------------------------------------------

Source::Source(const std::string& filename, const std::string& text)
  : filename_(filename)
{
  const auto *p = text.c_str();
  while (*p)
  {
    const auto *sp = p;
    while (*p && *p != '\n' && *p != '\r')
      ++ p;
    lines_.emplace_back(lines_.size(), sp, p - sp);
    if (*p == '\r')
      ++ p;
    if (*p == '\n')
      ++ p;
  }
}

Source load(const std::string& filename)
{
  std::ifstream s(filename);
  if (! s.good())
    throw -1;     // TODO

  std::stringstream buf;
  buf << s.rdbuf();
  s.close();
  if (! s.good())
    throw -1;     // TODO

  return { filename, buf.str().c_str() };
}

// ----------------------------------------------------------------------------
//      Token
// ----------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& s, const Token& token)
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

    case TokenType::Directive:
      s << "DIRECTIVE: " << token.text;
      break;

    case TokenType::Punctuation:
      s << "PUNCTUATION: " << token.text;
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

LineReader::LineReader(const Line& line)
  : line_(line), offset_(0)
{
}

Token LineReader::nextToken()
{
  int c;
  while ((c = get()) != -1 && std::isspace(c))
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


  if (std::isalpha(c) || c == '_')
  {
    token.text += c;
    while ((c = get()) == '$' || c == '_' || std::isalpha(c))
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
    while ((c = get()) != -1 && std::isdigit(c))
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

  if (c == '.')
  {
    token.text += c;
    while ((c = get()) != -1 && std::isalpha(c))
      token.text += c;
    if (c != -1)
      unget();
    if (token.text.length() == 1)
      throw -1;                                               // TODO
    token.type = TokenType::Directive;
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

  token.text += c;
  token.type = TokenType::Punctuation;
  return token;
}

}
