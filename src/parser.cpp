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
  std::unique_ptr<Statement> handleInstructionOrDirective(LineReader& reader, const std::string& label,
                                                          SourcePos labelPos,bool allowDef);
  std::unique_ptr<Operation> handleInstruction(LineReader& reader, Instruction& ins, SourcePos insPos);
  std::unique_ptr<Operation> handleImmediate(LineReader& reader, Instruction& ins, SourcePos insPos);
  std::unique_ptr<Operation> handleDirect(LineReader& reader, Instruction& ins, SourcePos insPos, bool forceAbsolute);
  std::unique_ptr<Operation> handleIndirect(LineReader& reader, Instruction& ins, SourcePos insPos);
  std::unique_ptr<Operation> handleRelative(LineReader& reader, Instruction& ins, SourcePos insPos);
  std::unique_ptr<Directive> handleDirective(LineReader& reader);
  std::unique_ptr<Directive> handleOrg(LineReader& reader);
  std::unique_ptr<Directive> handleBuf(LineReader& reader);
  std::unique_ptr<Directive> handleSeq(LineReader& reader);
  std::unique_ptr<Expression> parseExpression(LineReader& reader, bool optional = false);
  std::unique_ptr<ExprNode> parseOperand(LineReader& reader, bool optional = false);
  ByteSelector optionalByteSelector(LineReader& reader);
  IndexRegister optionalIndex(LineReader& reader, SourcePos *pos = nullptr);

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

    return handleInstructionOrDirective(reader, first.text, first.pos, true);
  }

  if (first.type == TokenType::Punctuator)
  {
    switch (first.punctuator)
    {
      case '.':
        return handleDirective(reader);

      case '*':
      {
        auto second = reader.nextToken();
        if (second.type == TokenType::Punctuator && second.punctuator == '=')
          return std::make_unique<ProgramCounterAssignment>(first.pos, parseExpression(reader));
        throwSourceError(second.pos, "Expected '='");
      }

      case '+':
      case '-':
      case '/':
        return handleInstructionOrDirective(reader, std::string(1, first.punctuator), first.pos, false);
    }
  }

  // TODO: throw
  return nullptr;
}

std::unique_ptr<Statement> Parser::handleInstructionOrDirective(LineReader& reader, const std::string& label,
                                                                SourcePos labelPos,bool allowDef)
{
  auto token = reader.nextToken();
  if (allowDef && token.type == TokenType::Punctuator && token.punctuator == '=')
    return std::make_unique<SymbolDefinition>(labelPos, label, parseExpression(reader));

  if (token.type == TokenType::Identifier)
  {
    auto ins = instructionNamed(token.text);
    if (! ins)
      throwSourceError(token.pos, "Invalid instruction ('%s')", token.text.c_str());
    auto node = handleInstruction(reader, *ins, token.pos);
    if (node && ! label.empty())
      node->setLabel(label);
    return node;
  }
  if (token.type == TokenType::Punctuator && token.punctuator == '.')
  {
    auto node = handleDirective(reader);
    if (node && ! label.empty())
      node->setLabel(label);
    return node;
  }
  throwSourceError(token.pos, "Expected instruction or directive");
}

std::unique_ptr<Operation> Parser::handleInstruction(LineReader& reader, Instruction& ins, SourcePos insPos)
{
  if (ins.isImplied())
    return std::make_unique<ImpliedOperation>(insPos, ins);
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

      case '+':
      case '-':
        break;

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
  return std::make_unique<ImmediateOperation>(insPos, ins, selector, parseExpression(reader));
}

std::unique_ptr<Operation> Parser::handleDirect(LineReader& reader, Instruction& ins, SourcePos insPos, bool forceAbsolute)
{
  auto expr = parseExpression(reader, true);
  if (! expr)
  {
    if (! ins.supports(AddrMode::Accumulator))
      throwSourceError(insPos, "Instruction '%s' does not support accumulator addressing", ins.name().c_str());
    return std::make_unique<AccumulatorOperation>(insPos, ins);
  }
  auto index = optionalIndex(reader);
  return std::make_unique<DirectOperation>(insPos, ins, index, forceAbsolute, std::move(expr));
}

std::unique_ptr<Operation> Parser::handleIndirect(LineReader& reader, Instruction& ins, SourcePos insPos)
{
  auto expr = parseExpression(reader);
  SourcePos indexPos;
  auto index = optionalIndex(reader, &indexPos);
  if (index == IndexRegister::Y)
    throwSourceError(indexPos, "Indexed indirect addressing is only valid with the X register");
  reader.expectPunctuator(')');
  auto postIndex = optionalIndex(reader, &indexPos);
  if (postIndex != IndexRegister::None)
  {
    if (index != IndexRegister::None)
      throwSourceError(indexPos, "Indirect addressing modes cannot be combined");
    if (postIndex == IndexRegister::X)
      throwSourceError(indexPos, "Indirect indexed addressing is only valid with the Y register");
    index = postIndex;
  }
  return std::make_unique<IndirectOperation>(insPos, ins, index, std::move(expr));
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

std::unique_ptr<Expression> Parser::parseExpression(LineReader& reader, bool optional)
{
  // Expressions are evaluated strictly left to right, with no operator precedence,
  // in order to match the behavior of the original assembler.
  auto root = parseOperand(reader, optional);
  if (! root)
    return nullptr;
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

std::unique_ptr<ExprNode> Parser::parseOperand(LineReader& reader, bool optional)
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

      case '+':
      case '-':
      {
        int count = 1;
        Token extra;
        while (count < 3 && (extra = reader.nextToken()).type == TokenType::Punctuator && extra.punctuator == token.punctuator)
          ++ count;
        if (count < 3)
          reader.unget(extra);
        auto direction = token.punctuator == '-' ? BranchDirection::Backward : BranchDirection::Forward;
        return std::make_unique<ExprTemporarySymbol>(token.pos, direction, count);
      }

      default:
        throwSourceError(token.pos, "Unexpected character ('%c')", token.punctuator);
    }
  }
  if (optional)
    return nullptr;
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

IndexRegister Parser::optionalIndex(LineReader& reader, SourcePos *pos)
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
    {
      if (pos)
        *pos = token.pos;
      return IndexRegister::X;
    }
    if (index == "y")
    {
      if (pos)
        *pos = token.pos;
      return IndexRegister::Y;
    }
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
