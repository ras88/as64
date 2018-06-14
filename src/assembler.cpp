#include <iostream>
#include "str.h"
#include "error.h"
#include "instruction.h"
#include "expr.h"
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
      throwSourceError("Symbol '%s' already exists", name.c_str());

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
        throwSourceError("Invalid instruction ('%s')", token.text.c_str());
      handleInstruction(reader, *ins);
      return;
    }
    if (token.type == TokenType::Punctuator && token.punctuator == '.')
    {
      handleDirective(reader);
      return;
    }
    throwSourceError("Expected instruction or directive");
  }

  if (token.type == TokenType::Punctuator)
  {
    if (token.punctuator == '.')
    {
      handleDirective(reader);
      return;
    }
    if (token.punctuator == '*')
    {
      token = reader.nextToken();
      if (token.type == TokenType::Punctuator && token.punctuator == '=')
      {
        auto addr = evalExpression(reader);
        if (writer_.offset() == 0)
          writer_.pc(addr);                                   // Behave like .org if no code has been emitted yet
        else
        {
          if (addr < writer_.pc())
            throwSourceError("Attempt to reverse program counter (from 0x%04x to 0x%04x)", writer_.pc(), addr);
          writer_.fill(addr - writer_.pc());
        }
        return;
      }
      throwSourceError("Expected '='");
    }
    if (token.punctuator == '-')
    {
      // TODO
    }
    if (token.punctuator == '+')
    {
      // TODO
    }
    if (token.punctuator == '/')
    {
      // TODO
    }
  }

  // TODO: throw
}

void Assembler::handleInstruction(LineReader& reader, Instruction& ins)
{
  if (ins.isRelative())
  {
    handleRelative(reader, ins);
    return;
  }
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

      default:
        throwSourceError("Unexpected character ('%c')", token.punctuator);
    }
  }
  handleDirect(reader, ins, false);
}

void Assembler::handleImmediate(LineReader& reader, Instruction& ins)
{
  auto selector = optionalByteSelector(reader);
  auto value = byteExpression(reader, selector);
  auto opcode = ins.opcode(AddrMode::Immediate);
  if (! isValid(opcode))
    throwSourceError("Immediate mode is not supported by instruction '%s'", ins.name().c_str());
  writer_.byte(opcode);
  writer_.byte(value);
}

void Assembler::handleDirect(LineReader& reader, Instruction& ins, bool forceAbsolute)
{
  auto addr = evalExpression(reader);
  auto index = optionalIndex(reader);
  auto size = ins.encodeDirect(writer_, addr, index, forceAbsolute);
  if (size == 0)
    throwSourceError("Invalid addressing mode for instruction '%s'", ins.name().c_str());
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
    throwSourceError("Expected a directive name");

  auto *handler = directives_.get(toLowerCase(token.text));
  if (! handler)
    throwSourceError("Unknown directive '%s'", token.text.c_str());

  (this->**handler)(reader);
}

void Assembler::handleOrg(LineReader& reader)
{
  writer_.pc(evalExpression(reader));
}

void Assembler::handleBuf(LineReader& reader)
{
  writer_.fill(evalExpression(reader));
}

// TODO: subject to forward references
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
        throwSourceError("Invalid value (%d); expected a number between 0 and 255", value);
      return value;
  }
}

// TODO: subject to forward references
int Assembler::evalExpression(LineReader& reader)
{
  Expression expr(reader);

  auto value = expr.eval(writer_.pc(), symbols_, false);
  std::cout << "EXPR:" << std::endl;
  expr.dump(std::cout, 2);
  std::cout << "Expr value = " << value << std::endl;
  return value;
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

IndexRegister Assembler::optionalIndex(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Punctuator || token.punctuator != ',')
  {
    reader.unget(token);
    return IndexRegister::None;
  }
  token = reader.nextToken();
  if (token.type == TokenType::Identifier)
  {
    auto index = toLowerCase(token.text);
    if (index == "x")
      return IndexRegister::X;
    if (index == "y")
      return IndexRegister::Y;
  }
  throwSourceError("Expected 'x' or 'y'");
}

SymbolTable<Assembler::DirectiveHandler> Assembler::directives_([](auto& table)
{
  table.emplace("org",      &Assembler::handleOrg);
  table.emplace("buf",      &Assembler::handleBuf);
});

}
