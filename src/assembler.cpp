#include <iostream>
#include <cstdarg>
#include "error.h"
#include "instruction.h"
#include "assembler.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Assembler
// ----------------------------------------------------------------------------

Assembler::Assembler()
  : writer_(code_)
{
}

void Assembler::file(const std::string& filename)
{
  input_.includeFile(filename);
}

void Assembler::assemble()
{
  Line line;
  while((line = input_.nextLine()).isValid())
  {
    try
    {
      LineReader reader(line);
      handleLine(reader);
    }
    catch (SourceError& err)
    {
      // Errors on a single line are not fatal.
      err.setLocation(line);
      std::cerr << "[Error] " << err.format() << std::endl;
    }
  }
}

void Assembler::handleLine(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type == TokenType::Identifier)
  {
    auto *ins = instructionNamed(token.text);
    if (ins)
    {
      handleInstruction(reader, *ins);
      return;
    }

    // Must be a symbol to define
    std::string name = token.text;
    if (symbols_.exists(name))
      throwError("Symbol '%s' already exists", name.c_str());

    token = reader.nextToken();
    if (token.type == TokenType::Punctuator && token.punctuator == '=')
    {
      symbols_.emplace(name, evalExpression(reader));
      return;
    }

    symbols_.emplace(name, writer_.pc());
    if (token.type == TokenType::Identifier)
    {
      ins = instructionNamed(token.text);
      if (! ins)
        throwError("Invalid instruction ('%s')", token.text.c_str());
      handleInstruction(reader, *ins);
      return;
    }
    if (token.type == TokenType::Punctuator && token.punctuator == '.')
    {
      handleDirective(reader);
      return;
    }
    throwError("Expected instruction or directive");
  }

  if (token.type == TokenType::Punctuator && token.punctuator == '.')
  {
    handleDirective(reader);
    return;
  }
  // TODO
}

void Assembler::handleInstruction(LineReader& reader, Instruction& ins)
{
  auto token = reader.nextToken();
  if (token.type == TokenType::Punctuator)
  {
    switch (token.punctuator)
    {
      case '#':
        handleImmediate(reader, ins);
        return;

      case '"':
      case '@':
      case '<':
      case '>':
        reader.unget(token);
        handleImmediate(reader, ins);
        return;

      case '(':
        handleIndirect(reader, ins);
        return;

      case '!':
        handleDirect(reader, ins, true);
        return;

      case '+':
      case '-':
        reader.unget(token);
        handleRelative(reader, ins);
        return;

      default:
        throwError("Unexpected character ('%c')", token.punctuator);
    }
  }
}

void Assembler::handleImmediate(LineReader& reader, Instruction& ins)
{
  auto selector = optionalByteSelector(reader);
  auto value = byteExpression(reader, selector);
  auto opcode = ins.opcode(AddrMode::Immediate);
  if (! isValid(opcode))
    throwError("Immediate mode is not supported by instruction '%s'", ins.name().c_str());
  writer_.byte(opcode);
  writer_.byte(value);
}

void Assembler::handleDirect(LineReader& reader, Instruction& ins, bool forceAbsolute)
{

}

void Assembler::handleIndirect(LineReader& reader, Instruction& ins)
{

}

void Assembler::handleRelative(LineReader& reader, Instruction& ins)
{

}

void Assembler::handleDirective(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Identifier)
    throwError("Expected a directive name");

  // TODO: table of directives with functions to handle them
  // allow shorter or longer names as long as unambiguous
  std::cout << token.text << std::endl;             // TODO

}

int Assembler::byteExpression(LineReader& reader, ByteSelector selector)
{
  auto value = evalExpression(reader);
  switch (selector)
  {
    case ByteSelector::Low:
      return value & 0xff;

    case ByteSelector::High:
      return value >> 8;

    case ByteSelector::Unspecified:
      if (value > 0xff)
        throwError("Invalid value (%d); expected a number between 0 and 255", value);
      return value;
  }
}

int Assembler::evalExpression(LineReader& reader)
{
  auto value = evalOperand(reader);
  Token token;
  while ((token = reader.nextToken()).type == TokenType::Punctuator)
  {
    switch (token.punctuator)
    {
      case '+':
        value += evalOperand(reader);
        break;

      case '-':
        value -= evalOperand(reader);
        break;

      case '*':
        value *= evalOperand(reader);
        break;

      case '/':
        value /= evalOperand(reader);
        break;

      default:
        return value;
    }
  }
  if (value < 0 || value > 0xffff)
    throwError("Invalid expression result (%d); expected a number between 0 and 65535", value);
  return value;
}

int Assembler::evalOperand(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type == TokenType::Number)
    return token.number;
  if (token.type == TokenType::Identifier)
  {
    auto *value = symbols_.get(token.text);
    if (! value)
      throwError("Undefined symbol '%s'", token.text.c_str());
    return *value;
  }
  if (token.type == TokenType::Literal)
  {
    // TODO: Expect a single character and return its PETSCII value
    return 0;
  }
  if (token.type == TokenType::Punctuator)
  {
    switch (token.punctuator)
    {
      case '*':
        return writer_.pc();

      case '@':
        return 0;                     // TODO: screen code

      default:
        throwError("Unexpected character ('%c')", token.punctuator);
    }
  }

  throwError("Expected a valid operand");
}

Assembler::ByteSelector Assembler::optionalByteSelector(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type == TokenType::Punctuator)
  {
    switch (token.punctuator)
    {
      case '<':
        return ByteSelector::Low;

      case '>':
        return ByteSelector::High;
    }
  }
  reader.unget(token);
  return ByteSelector::Unspecified;
}

void Assembler::throwError(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);

  char buf[1024];
  vsnprintf(buf, sizeof(buf), format, ap);
  throw SourceError(buf);
}

}
