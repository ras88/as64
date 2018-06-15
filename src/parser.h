#ifndef _INCLUDED_CASSM_PARSER_H
#define _INCLUDED_CASSM_PARSER_H

#include <string>
#include <ostream>
#include "source.h"
#include "instruction.h"
#include "expr.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      ByteSelector
// ----------------------------------------------------------------------------

enum class ByteSelector
{
  Unspecified,
  Low,
  High
};

// ----------------------------------------------------------------------------
//      Statement
// ----------------------------------------------------------------------------

class Statement
{
public:
  Statement(const Line& line) : line_(line) { }
  virtual ~Statement() { }

  virtual void dump(std::ostream& s) const noexcept = 0;

private:
  Line line_;
};

// ----------------------------------------------------------------------------
//      LabelStatement
// ----------------------------------------------------------------------------

class LabelStatement : public Statement
{
public:
  LabelStatement(const Line& line, const std::string& name) : Statement(line), name_(name) { }

  void dump(std::ostream& s) const noexcept override;

private:
  std::string name_;
};

// ----------------------------------------------------------------------------
//      SymbolDefinitionStatement
// ----------------------------------------------------------------------------

class SymbolDefinitionStatement : public Statement
{
public:
  SymbolDefinitionStatement(const Line& line, const std::string& name, Expression expr)
    : Statement(line), name_(name), expr_(std::move(expr)) { }

  void dump(std::ostream& s) const noexcept override;

private:
  std::string name_;
  Expression expr_;
};

// ----------------------------------------------------------------------------
//      ProgramCounterAssignmentStatement
// ----------------------------------------------------------------------------

class ProgramCounterAssignmentStatement : public Statement
{
public:
  ProgramCounterAssignmentStatement(const Line& line, Expression expr) : Statement(line), expr_(std::move(expr)) { }

  void dump(std::ostream& s) const noexcept override;

private:
  Expression expr_;
};

// ----------------------------------------------------------------------------
//      InstructionStatement
// ----------------------------------------------------------------------------

class InstructionStatement : public Statement
{
public:
  InstructionStatement(const Line& line, Instruction& instruction) : Statement(line), instruction_(instruction) { }

  Instruction& instruction() const { return instruction_; }

private:
  Instruction& instruction_;
};

// ----------------------------------------------------------------------------
//      ImmediateInstructionStatement
// ----------------------------------------------------------------------------

class ImmediateInstructionStatement : public InstructionStatement
{
public:
  ImmediateInstructionStatement(const Line& line, Instruction& instruction, ByteSelector selector, Expression expr)
    : InstructionStatement(line, instruction), selector_(selector), expr_(std::move(expr)) { }

  void dump(std::ostream& s) const noexcept override;

private:
  ByteSelector selector_;
  Expression expr_;
};

// ----------------------------------------------------------------------------
//      Parser
// ----------------------------------------------------------------------------

class Parser
{
public:
  void file(const std::string& filename);

  void parse();

private:
  void handleLine(LineReader& reader);
  void handleInstruction(LineReader& reader, Instruction& ins);
  void handleImmediate(LineReader& reader, Instruction& ins);
  void handleDirect(LineReader& reader, Instruction& ins, bool forceAbsolute);
  void handleIndirect(LineReader& reader, Instruction& ins);
  void handleRelative(LineReader& reader, Instruction& ins);
  void handleDirective(LineReader& reader);
  void handleOrg(LineReader& reader);
  void handleBuf(LineReader& reader);
  ByteSelector optionalByteSelector(LineReader& reader);
  IndexRegister optionalIndex(LineReader& reader);
  void emit(std::unique_ptr<Statement> statement);

  SourceStream input_;
  std::vector<std::unique_ptr<Statement>> statements_;

  using DirectiveHandler = void (Parser::*)(LineReader& reader);
  static SymbolTable<DirectiveHandler> directives_;
};

}
#endif
