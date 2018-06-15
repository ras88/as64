#include <iostream>
#include "str.h"
#include "error.h"
#include "table.h"
#include "parser.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      LabelStatement
// ----------------------------------------------------------------------------

void LabelStatement::dump(std::ostream& s) const noexcept
{
  s << "Label: " << name_;
}

// ----------------------------------------------------------------------------
//      SymbolDefinitionStatement
// ----------------------------------------------------------------------------

void SymbolDefinitionStatement::dump(std::ostream& s) const noexcept
{
  s << "Define: " << name_ << std::endl;
  expr_.dump(s, 2);
}

// ----------------------------------------------------------------------------
//      ProgramCounterAssignmentStatement
// ----------------------------------------------------------------------------

void ProgramCounterAssignmentStatement::dump(std::ostream& s) const noexcept
{
  s << "Set Program Counter:" << std::endl;
  expr_.dump(s, 2);
}

// ----------------------------------------------------------------------------
//      ImmediateInstructionStatement
// ----------------------------------------------------------------------------

void ImmediateInstructionStatement::dump(std::ostream& s) const noexcept
{
  s << "Immediate Mode Instruction: " << instruction().name() << std::endl;
  expr_.dump(s, 2);
}

// ----------------------------------------------------------------------------
//      Parser
// ----------------------------------------------------------------------------

void Parser::file(const std::string& filename)
{
  input_.includeFile(filename);
}

void Parser::parse()
{
  Line line;
  while((line = input_.nextLine()).isValid())
  {
    try
    {
      LineReader reader(line);
      handleLine(reader);                                     // TODO: ensure we're at end of line
    }
    catch (SourceError& err)
    {
      // Errors on a single line are not fatal.
      err.setLocation(line);
      std::cerr << "[Error] " << err.format() << std::endl;
    }
  }
}

void Parser::handleLine(LineReader& reader)
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
    token = reader.nextToken();
    if (token.type == TokenType::Punctuator && token.punctuator == '=')
    {
      emit(std::make_unique<SymbolDefinitionStatement>(reader.line(), name, Expression(reader)));
      return;
    }
    emit(std::make_unique<LabelStatement>(reader.line(), name));

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
        emit(std::make_unique<ProgramCounterAssignmentStatement>(reader.line(), Expression(reader)));
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

void Parser::handleInstruction(LineReader& reader, Instruction& ins)
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

void Parser::handleImmediate(LineReader& reader, Instruction& ins)
{
  auto selector = optionalByteSelector(reader);
  if (! ins.supports(AddrMode::Immediate))
    throwSourceError("Immediate mode is not supported by instruction '%s'", ins.name().c_str());
  emit(std::make_unique<ImmediateInstructionStatement>(reader.line(), ins, selector, Expression(reader)));
}

void Parser::handleDirect(LineReader& reader, Instruction& ins, bool forceAbsolute)
{
#ifdef TODO
  auto addr = evalExpression(reader);
  auto index = optionalIndex(reader);
  auto size = ins.encodeDirect(writer_, addr, index, forceAbsolute);
  if (size == 0)
    throwSourceError("Invalid addressing mode for instruction '%s'", ins.name().c_str());
#endif
}

void Parser::handleIndirect(LineReader& reader, Instruction& ins)
{

}

void Parser::handleRelative(LineReader& reader, Instruction& ins)
{

}

void Parser::handleDirective(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Identifier)
    throwSourceError("Expected a directive name");

  auto *handler = directives_.get(toLowerCase(token.text));
  if (! handler)
    throwSourceError("Unknown directive '%s'", token.text.c_str());

  (this->**handler)(reader);
}

void Parser::handleOrg(LineReader& reader)
{
// TODO  writer_.pc(evalExpression(reader));
}

void Parser::handleBuf(LineReader& reader)
{
// TODO  writer_.fill(evalExpression(reader));
}

ByteSelector Parser::optionalByteSelector(LineReader& reader)
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

IndexRegister Parser::optionalIndex(LineReader& reader)
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

void Parser::emit(std::unique_ptr<Statement> statement)
{
  // TODO: REMOVE
  statement->dump(std::cout);
  std::cout << std::endl;

  statements_.push_back(std::move(statement));
}

SymbolTable<Parser::DirectiveHandler> Parser::directives_([](auto& table)
{
  table.emplace("org",      &Parser::handleOrg);
  table.emplace("buf",      &Parser::handleBuf);
});

}
