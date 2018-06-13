#include <iostream>
#include "error.h"
#include "instruction.h"
#include "assembler.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Assembler
// ----------------------------------------------------------------------------

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
    catch (Error& err)
    {
      // Errors on a single line are not fatal.
      std::cerr << "[Error] " << err.message() << std::endl;
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






  }
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
        throw -1;               // TODO
    }
  }
}

void Assembler::handleImmediate(LineReader& reader, Instruction& ins)
{
  auto selector = optionalByteSelector(reader);
  auto value = byteExpression(reader, selector);
  auto opcode = ins.opcode(AddrMode::Immediate);
  if (! isValid(opcode))
    throw -1;                                   // TODO
  std::cout << opcode << ' ' << value << std::endl;
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

int Assembler::byteExpression(LineReader& reader, ByteSelector selector)
{
  auto value = evalExpression(reader);
  if (value < 0 || value >= 0xffff)
    throw -1;                       // TODO
  switch (selector)
  {
    case ByteSelector::Low:
      return value & 0xff;

    case ByteSelector::High:
      return value >> 8;

    case ByteSelector::Unspecified:
      if (value > 0xff)
        throw -1;                   // TODO
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
  return value;
}

int Assembler::evalOperand(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type == TokenType::Number)
    return token.number;
  if (token.type == TokenType::Identifier)
  {
    // TODO: Lookup symbol
    return 0;
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
        return 0;                     // TODO: program counter

      case '@':
        return 0;                     // TODO: screen code

      default:
        throw -1;                     // TODO
    }
  }

  throw -1;                           // TODO
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

}
