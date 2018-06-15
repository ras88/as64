#ifndef _INCLUDED_CASSM_AST_H
#define _INCLUDED_CASSM_AST_H

#include <string>
#include <memory>
#include <vector>
#include <ostream>
#include "types.h"
#include "source.h"
#include "table.h"
#include "instruction.h"

namespace cassm
{

class Node;
class Expression;
class ExprNode;

// ----------------------------------------------------------------------------
//      Node
// ----------------------------------------------------------------------------

class Node
{
public:
  Node(SourcePos pos) noexcept : pos_(pos) { }
  virtual ~Node() noexcept { }

  SourcePos pos() const noexcept { return pos_; }

  virtual void dump(std::ostream& s, int level = 0) const noexcept = 0;

protected:
  void indent(std::ostream& s, int level) const noexcept;

private:
  SourcePos pos_;
};

// ----------------------------------------------------------------------------
//      Statement
// ----------------------------------------------------------------------------

class Statement: public Node
{
public:
  Statement(SourcePos pos, const std::string& label = "") noexcept : Node(pos), label_(label) { }

  bool hasLabel() const noexcept { return ! label_.empty(); }
  std::string label() const noexcept { return label_; }
  void setLabel(const std::string& label) noexcept { label_ = label; }

protected:
  void prefixLabel(std::ostream& s) const noexcept;

private:
  std::string label_;
};

// ----------------------------------------------------------------------------
//      SymbolDefinition
// ----------------------------------------------------------------------------

class SymbolDefinition: public Statement
{
public:
  SymbolDefinition(SourcePos pos, const std::string& label, std::unique_ptr<Expression> expr) noexcept
    : Statement(pos, label), expr_(std::move(expr)) { }

  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      ProgramCounterAssignment
// ----------------------------------------------------------------------------

class ProgramCounterAssignment: public Statement
{
public:
  ProgramCounterAssignment(SourcePos pos, std::unique_ptr<Expression> expr) noexcept : Statement(pos), expr_(std::move(expr)) { }

  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      Operation
// ----------------------------------------------------------------------------

class Operation : public Statement
{
public:
  Operation(SourcePos pos, Instruction& instruction) noexcept : Statement(pos), instruction_(instruction) { }

  Instruction& instruction() const { return instruction_; }

private:
  Instruction& instruction_;
};

// ----------------------------------------------------------------------------
//      ImmediateOperation
// ----------------------------------------------------------------------------

class ImmediateOperation : public Operation
{
public:
  ImmediateOperation(SourcePos pos, Instruction& instruction, ByteSelector selector, std::unique_ptr<Expression> expr) noexcept
    : Operation(pos, instruction), selector_(selector), expr_(std::move(expr)) { }

  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  ByteSelector selector_;
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      DirectOperation
// ----------------------------------------------------------------------------

class DirectOperation : public Operation
{
public:
  DirectOperation(SourcePos pos, Instruction& instruction, IndexRegister index, bool forceAbsolute,
                  std::unique_ptr<Expression> expr) noexcept
    : Operation(pos, instruction), index_(index), forceAbsolute_(forceAbsolute), expr_(std::move(expr)) { }

  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  IndexRegister index_;
  bool forceAbsolute_;
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      BranchOperation
// ----------------------------------------------------------------------------

class BranchOperation : public Operation
{
public:
  BranchOperation(SourcePos pos, Instruction& instruction, std::unique_ptr<Expression> expr) noexcept
    : Operation(pos, instruction), expr_(std::move(expr)) { }

  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      Directive
// ----------------------------------------------------------------------------

class Directive : public Statement
{
public:
  Directive(SourcePos pos) noexcept : Statement(pos) { }
};

// ----------------------------------------------------------------------------
//      StatementList
// ----------------------------------------------------------------------------

class StatementList: public Node
{
public:
  StatementList(SourcePos pos) noexcept : Node(pos) { }

  void add(std::unique_ptr<Statement> statement) noexcept;

  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::vector<std::unique_ptr<Statement>> statements_;
};

// ----------------------------------------------------------------------------
//      Expression
// ----------------------------------------------------------------------------

class Expression : public Node
{
public:
  Expression(SourcePos pos, std::unique_ptr<ExprNode> root) : Node(pos), root_(std::move(root)) { }

  // If throwUndefined is false, this method returns -1 if the expression cannot
  // be resolved due to one or more undefined symbols. Any other error condition
  // always throws a SourceError.
  int eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined);

  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::unique_ptr<ExprNode> root_;
};

// ----------------------------------------------------------------------------
//      ExprNode
// ----------------------------------------------------------------------------

class ExprNode : public Node
{
public:
  ExprNode(SourcePos pos) : Node(pos) { }

  virtual int value() const noexcept { return -1; }
  virtual int checkValue() const { return value(); }
  virtual std::unique_ptr<ExprNode> eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined) { return nullptr; }
};

// ----------------------------------------------------------------------------
//      ExprConstant
// ----------------------------------------------------------------------------

class ExprConstant : public ExprNode
{
public:
  ExprConstant(SourcePos pos, int value) : ExprNode(pos), value_(value) { }

  int value() const noexcept override { return value_; }
  int checkValue() const override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  int value_;
};

// ----------------------------------------------------------------------------
//      ExprSymbol
// ----------------------------------------------------------------------------

class ExprSymbol : public ExprNode
{
public:
  ExprSymbol(SourcePos pos, const std::string& name) : ExprNode(pos), name_(name) { }

  std::unique_ptr<ExprNode> eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  std::string name_;
};

// ----------------------------------------------------------------------------
//      ExprProgramCounter
// ----------------------------------------------------------------------------

class ExprProgramCounter : public ExprNode
{
public:
  ExprProgramCounter(SourcePos pos) : ExprNode(pos) { }

  std::unique_ptr<ExprNode> eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;
};

// ----------------------------------------------------------------------------
//      ExprOperator
// ----------------------------------------------------------------------------

class ExprOperator : public ExprNode
{
public:
  using Handler = int(*)(int, int);

  ExprOperator(SourcePos pos, std::unique_ptr<ExprNode> left, std::unique_ptr<ExprNode> right, char op, Handler handler)
    : ExprNode(pos), left_(std::move(left)), right_(std::move(right)), op_(op), handler_(handler) { }

  std::unique_ptr<ExprNode> eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  std::unique_ptr<ExprNode> left_;
  std::unique_ptr<ExprNode> right_;
  char op_;
  Handler handler_;
};

}
#endif
