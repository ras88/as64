#include <iostream>
#include "str.h"
#include "error.h"
#include "table.h"
#include "parser.h"
#include "context.h"

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
  std::unique_ptr<Statement> handleDirective(LineReader& reader);
  std::unique_ptr<Statement> handleOrg(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleOff(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleOfe(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleBuf(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleByte(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleWord(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleAsc(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleScr(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleSeq(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleObj(LineReader& reader, SourcePos pos);
  std::unique_ptr<Statement> handleUnsupported(LineReader& reader, SourcePos pos);
  std::unique_ptr<Expression> parseExpression(LineReader& reader, bool optional = false);
  std::unique_ptr<ExprNode> parseOperand(LineReader& reader, bool optional = false);
  ByteSelector optionalByteSelector(LineReader& reader);
  IndexRegister optionalIndex(LineReader& reader, SourcePos *pos = nullptr);

  Context& context_;

  using DirectiveHandler = std::unique_ptr<Statement> (Parser::*)(LineReader& reader, SourcePos pos);
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
  context_.source.includeFile(filename);
}

void Parser::parse()
{
  Line *line;
  while((line = context_.source.nextLine()) != nullptr)
  {
    try
    {
      LineReader reader(*line);
      context_.statements.add(handleStatement(reader));
      
      // TODO: possibly support ':' for multiple statements on a single line
      auto token = reader.nextToken();
      if (token.type != TokenType::End)
        throwSourceError(token.pos, "Unexpected character");
    }
    catch (SourceError& err)
    {
      context_.messages.add(Severity::Error, err.pos(), err.message());
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

  return std::make_unique<EmptyStatement>(first.pos);
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

std::unique_ptr<Statement> Parser::handleDirective(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Identifier)
    throwSourceError(token.pos, "Expected a directive name");

  auto *handler = directives_.get(toLowerCase(token.text));
  if (! handler)
    throwSourceError(token.pos, "Unknown directive '%s'", token.text.c_str());

  return (this->**handler)(reader, token.pos);
}

std::unique_ptr<Statement> Parser::handleOrg(LineReader& reader, SourcePos pos)
{
  return std::make_unique<OriginDirective>(pos, parseExpression(reader));
}

std::unique_ptr<Statement> Parser::handleOff(LineReader& reader, SourcePos pos)
{
  return std::make_unique<OffsetBeginDirective>(pos, parseExpression(reader));
}

std::unique_ptr<Statement> Parser::handleOfe(LineReader& reader, SourcePos pos)
{
  return std::make_unique<OffsetEndDirective>(pos);
}

std::unique_ptr<Statement> Parser::handleBuf(LineReader& reader, SourcePos pos)
{
  return std::make_unique<BufferDirective>(pos, parseExpression(reader));
}

std::unique_ptr<Statement> Parser::handleByte(LineReader& reader, SourcePos pos)
{
  auto selector = optionalByteSelector(reader);
  std::vector<std::unique_ptr<Expression>> args;
  Token token;
  do
  {
    args.push_back(parseExpression(reader));
  }
  while (reader.optionalPunctuator(','));
  return std::make_unique<ByteDirective>(pos, selector, std::move(args));
}

std::unique_ptr<Statement> Parser::handleWord(LineReader& reader, SourcePos pos)
{
  std::vector<std::unique_ptr<Expression>> args;
  Token token;
  do
  {
    args.push_back(parseExpression(reader));
  }
  while (reader.optionalPunctuator(','));
  return std::make_unique<WordDirective>(pos, std::move(args));
}

std::unique_ptr<Statement> Parser::handleAsc(LineReader& reader, SourcePos pos)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Literal)
    throwSourceError(token.pos, "Expected a quoted string");
  return std::make_unique<StringDirective>(pos, StringEncoding::Petscii, token.text);
}

std::unique_ptr<Statement> Parser::handleScr(LineReader& reader, SourcePos pos)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Literal)
    throwSourceError(token.pos, "Expected a quoted string");
  return std::make_unique<StringDirective>(pos, StringEncoding::Screen, token.text);
}

std::unique_ptr<Statement> Parser::handleSeq(LineReader& reader, SourcePos pos)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Literal)
    throwSourceError(token.pos, "Expected a quoted filename");
  try
  {
    context_.source.includeFile(token.text);
    return std::make_unique<EmptyStatement>(pos);
  }
  catch (SystemError& err)
  {
    throwSourceError(token.pos, "%s", err.message().c_str());
  }
}

std::unique_ptr<Statement> Parser::handleObj(LineReader& reader, SourcePos pos)
{
  auto token = reader.nextToken();
  if (token.type != TokenType::Literal)
    throwSourceError(token.pos, "Expected a quoted filename");
  return std::make_unique<ObjectFileDirective>(pos, token.text);
}

std::unique_ptr<Statement> Parser::handleUnsupported(LineReader& reader, SourcePos pos)
{
  Token token;
  do
  {
    token = reader.nextToken();
  }
  while (token.type != TokenType::End);                 // TODO: adjust logic if ':' is supported
  context_.messages.warning(pos, "Ignored unsupported statement");
  return std::make_unique<EmptyStatement>(pos);
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
  table.emplace("off",      &Parser::handleOff);
  table.emplace("ofe",      &Parser::handleOfe);
  table.emplace("buf",      &Parser::handleBuf);
  table.emplace("byte",     &Parser::handleByte);
  table.emplace("word",     &Parser::handleWord);
  table.emplace("asc",      &Parser::handleAsc);
  table.emplace("scr",      &Parser::handleScr);
  table.emplace("seq",      &Parser::handleSeq);
  table.emplace("obj",      &Parser::handleObj);
  table.emplace("dvi",      &Parser::handleUnsupported);
  table.emplace("dvo",      &Parser::handleUnsupported);
  table.emplace("burst",    &Parser::handleUnsupported);
  table.emplace("mem",      &Parser::handleUnsupported);
  table.emplace("dis",      &Parser::handleUnsupported);
  table.emplace("out",      &Parser::handleUnsupported);
  table.emplace("bas",      &Parser::handleUnsupported);
  table.emplace("link",     &Parser::handleUnsupported);
  table.emplace("loop",     &Parser::handleUnsupported);
  table.emplace("file",     &Parser::handleUnsupported);
  table.emplace("lst",      &Parser::handleUnsupported);
  table.emplace("top",      &Parser::handleUnsupported);
  table.emplace("sst",      &Parser::handleUnsupported);
  table.emplace("psu",      &Parser::handleUnsupported);
  table.emplace("fas",      &Parser::handleUnsupported);
});

}
