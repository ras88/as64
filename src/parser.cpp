#include <iostream>
#include "str.h"
#include "error.h"
#include "table.h"
#include "parser.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Parser
// ----------------------------------------------------------------------------

class Parser
{
public:
  Parser(Context& context);

  void file(const std::string& filename);

  void parse();

private:
  std::unique_ptr<Statement> handleStatement(LineReader& reader);
  std::unique_ptr<Operation> handleInstruction(LineReader& reader, Instruction& ins, SourcePos insPos);
  std::unique_ptr<Operation> handleImmediate(LineReader& reader, Instruction& ins, SourcePos insPos);
  std::unique_ptr<Operation> handleDirect(LineReader& reader, Instruction& ins, SourcePos insPos, bool forceAbsolute);
  std::unique_ptr<Operation> handleIndirect(LineReader& reader, Instruction& ins, SourcePos insPos);
  std::unique_ptr<Operation> handleRelative(LineReader& reader, Instruction& ins, SourcePos insPos);
  std::unique_ptr<Directive> handleDirective(LineReader& reader);
  std::unique_ptr<Directive> handleOrg(LineReader& reader);
  std::unique_ptr<Directive> handleBuf(LineReader& reader);
  std::unique_ptr<Directive> handleSeq(LineReader& reader);
  std::unique_ptr<Expression> parseExpression(LineReader& reader);
  std::unique_ptr<ExprNode> parseOperand(LineReader& reader);
  ByteSelector optionalByteSelector(LineReader& reader);
  IndexRegister optionalIndex(LineReader& reader);

  Context& context_;

  using DirectiveHandler = std::unique_ptr<Directive> (Parser::*)(LineReader& reader);
  static SymbolTable<DirectiveHandler> directives_;
};

void parseFile(Context& context, const std::string& filename)
{
  Parser parser(context);
  parser.file(filename);
  parser.parse();
}

Parser::Parser(Context& context)
  : context_(context)
{
}

void Parser::file(const std::string& filename)
{
  context_.source().includeFile(filename);
}

void Parser::parse()
{
  Line *line;
  while((line = context_.source().nextLine()) != nullptr)
  {
    try
    {
      LineReader reader(*line);
      auto statement = handleStatement(reader);
      if (statement)
        context_.statements().add(std::move(statement));

      // TODO: ensure we're at end of line
    }
    catch (SourceError& err)
    {
      context_.messages().add(Severity::Error, err.pos(), err.message());
    }
  }
}

std::unique_ptr<Statement> Parser::handleStatement(LineReader& reader)
{
  auto first = reader.nextToken();
  if (first.type == TokenType::Identifier)
  {
    auto *ins = instructionNamed(first.text);
    if (ins)
      return handleInstruction(reader, *ins, first.pos);

    // Must be a symbol to define
    auto second = reader.nextToken();
    if (second.type == TokenType::Punctuator && second.punctuator == '=')
      return std::make_unique<SymbolDefinition>(first.pos, first.text, parseExpression(reader));

    if (second.type == TokenType::Identifier)
    {
      ins = instructionNamed(second.text);
      if (! ins)
        throwSourceError(second.pos, "Invalid instruction ('%s')", second.text.c_str());
      auto node = handleInstruction(reader, *ins, second.pos);
      if (node)
        node->setLabel(first.text);
      return node;
    }
    if (second.type == TokenType::Punctuator && second.punctuator == '.')
    {
      auto node = handleDirective(reader);
      if (node)
        node->setLabel(first.text);
      return node;
    }
    throwSourceError(second.pos, "Expected instruction or directive");
  }

  if (first.type == TokenType::Punctuator)
  {
    if (first.punctuator == '.')
      return handleDirective(reader);
    if (first.punctuator == '*')
    {
      auto second = reader.nextToken();
      if (second.type == TokenType::Punctuator && second.punctuator == '=')
        return std::make_unique<ProgramCounterAssignment>(first.pos, parseExpression(reader));
      throwSourceError(second.pos, "Expected '='");
    }
    if (first.punctuator == '-')
    {
      // TODO
    }
    if (first.punctuator == '+')
    {
      // TODO
    }
    if (first.punctuator == '/')
    {
      // TODO
    }
  }

  // TODO: throw
  return nullptr;
}

std::unique_ptr<Operation> Parser::handleInstruction(LineReader& reader, Instruction& ins, SourcePos insPos)
{
  if (ins.isRelative())
    return handleRelative(reader, ins, insPos);
  auto token = reader.nextToken();
  if (token.type == TokenType::Punctuator)
  {
    switch (token.punctuator)
    {
      case '#':
        return handleImmediate(reader, ins, insPos);

      case '"':
      case '@':
      case '<':
      case '>':
        reader.unget(token);
        return handleImmediate(reader, ins, insPos);

      case '(':
        return handleIndirect(reader, ins, insPos);

      case '!':
        return handleDirect(reader, ins, insPos, true);

      default:
        throwSourceError(token.pos, "Unexpected character ('%c')", token.punctuator);
    }
  }
  reader.unget(token);
  return handleDirect(reader, ins, insPos, false);
}

std::unique_ptr<Operation> Parser::handleImmediate(LineReader& reader, Instruction& ins, SourcePos insPos)
{
  auto selector = optionalByteSelector(reader);
  if (! ins.supports(AddrMode::Immediate))
    throwSourceError(insPos, "Immediate mode is not supported by instruction '%s'", ins.name().c_str());
  return std::make_unique<ImmediateOperation>(insPos, ins, selector, parseExpression(reader));
}

std::unique_ptr<Operation> Parser::handleDirect(LineReader& reader, Instruction& ins, SourcePos insPos, bool forceAbsolute)
{
  auto expr = parseExpression(reader);
  auto index = optionalIndex(reader);
  return std::make_unique<DirectOperation>(insPos, ins, index, forceAbsolute, std::move(expr));
}

std::unique_ptr<Operation> Parser::handleIndirect(LineReader& reader, Instruction& ins, SourcePos insPos)
{
  // TODO

  return nullptr;
}

std::unique_ptr<Operation> Parser::handleRelative(LineReader& reader, Instruction& ins, SourcePos insPos)
{
  return std::make_unique<BranchOperation>(insPos, ins, parseExpression(reader));
}

std::unique_ptr<Directive> Parser::handleDirective(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Identifier)
    throwSourceError(token.pos, "Expected a directive name");

  auto *handler = directives_.get(toLowerCase(token.text));
  if (! handler)
    throwSourceError(token.pos, "Unknown directive '%s'", token.text.c_str());

  return (this->**handler)(reader);
}

std::unique_ptr<Directive> Parser::handleOrg(LineReader& reader)
{
// TODO  writer_.pc(evalExpression(reader));
  return nullptr;
}

std::unique_ptr<Directive> Parser::handleBuf(LineReader& reader)
{
// TODO  writer_.fill(evalExpression(reader));
  return nullptr;
}

std::unique_ptr<Directive> Parser::handleSeq(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Literal)
    throwSourceError(token.pos, "Expected a quoted filename");
  try
  {
    context_.source().includeFile(token.text);
    return nullptr;
  }
  catch (SystemError& err)
  {
    throwSourceError(token.pos, "%s", err.message().c_str());
  }
}

std::unique_ptr<Expression> Parser::parseExpression(LineReader& reader)
{
  // Expressions are evaluated strictly left to right, with no operator precedence,
  // in order to match the behavior of the original assembler.
  auto root = parseOperand(reader);
  Token token;
  while ((token = reader.nextToken()).type == TokenType::Punctuator)
  {
    auto op = token.punctuator;
    switch (op)
    {
      case '+':
        root = std::make_unique<ExprOperator>(root->pos(), std::move(root), parseOperand(reader), op,
                                              [](int a, int b) { return a + b; });
        break;

      case '-':
        root = std::make_unique<ExprOperator>(root->pos(), std::move(root), parseOperand(reader), op,
                                              [](int a, int b) { return a - b; });
        break;

      case '*':
        root = std::make_unique<ExprOperator>(root->pos(), std::move(root), parseOperand(reader), op,
                                              [](int a, int b) { return a * b; });
        break;

      case '/':
        // TODO: check for divide by zero in the handler
        root = std::make_unique<ExprOperator>(root->pos(), std::move(root), parseOperand(reader), op,
                                              [](int a, int b) { return a / b; });
        break;

      default:
      {
        auto expr = std::make_unique<Expression>(root->pos(), std::move(root));
        reader.unget(token);
        return expr;
      }
    }
  }
  auto expr = std::make_unique<Expression>(root->pos(), std::move(root));
  reader.unget(token);
  return expr;
}

std::unique_ptr<ExprNode> Parser::parseOperand(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type == TokenType::Number)
    return std::make_unique<ExprConstant>(token.pos, token.number);
  if (token.type == TokenType::Identifier)
    return std::make_unique<ExprSymbol>(token.pos, token.text);
  if (token.type == TokenType::Literal)
  {
    // TODO: Expect a single character and return its PETSCII value
    return std::make_unique<ExprConstant>(token.pos, 0);
  }
  if (token.type == TokenType::Punctuator)
  {
    switch (token.punctuator)
    {
      case '*':
        return std::make_unique<ExprProgramCounter>(token.pos);

      case '@':
        // TODO: screen code
        return std::make_unique<ExprConstant>(token.pos, 0);

      default:
        throwSourceError(token.pos, "Unexpected character ('%c')", token.punctuator);
    }
  }

  throwSourceError(token.pos, "Expected a valid operand");
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
  throwSourceError(token.pos, "Expected 'x' or 'y'");
}

SymbolTable<Parser::DirectiveHandler> Parser::directives_([](auto& table)
{
  table.emplace("org",      &Parser::handleOrg);
  table.emplace("buf",      &Parser::handleBuf);
  table.emplace("seq",      &Parser::handleSeq);
});

}
