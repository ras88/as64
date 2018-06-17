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
class StatementVisitor;
class Context;

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
  Statement(SourcePos pos) noexcept : Node(pos) { }
  Statement(SourcePos pos, const Label& label) noexcept : Node(pos), label_(label) { }

  const Label& label() const noexcept { return label_; }
  void setLabel(const Label& label) noexcept { label_ = label; }

  virtual void accept(StatementVisitor& visitor) = 0;

protected:
  void prefixLabel(std::ostream& s) const noexcept;

private:
  Label label_;
};

// ----------------------------------------------------------------------------
//      EmptyStatement
// ----------------------------------------------------------------------------

class EmptyStatement: public Statement
{
public:
  EmptyStatement(SourcePos pos) noexcept : Statement(pos) { }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;
};

// ----------------------------------------------------------------------------
//      SymbolDefinition
// ----------------------------------------------------------------------------

class SymbolDefinition: public Statement
{
public:
  SymbolDefinition(SourcePos pos, const Label& label, std::unique_ptr<Expression> expr) noexcept
    : Statement(pos, label), expr_(std::move(expr)) { }

  Expression& expr() const noexcept { return *expr_; }

  void accept(StatementVisitor& visitor) override;
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

  Expression& expr() const noexcept { return *expr_; }

  void accept(StatementVisitor& visitor) override;
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

  Instruction& instruction() const noexcept { return instruction_; }

private:
  Instruction& instruction_;
};

// ----------------------------------------------------------------------------
//      ImpliedOperation
// ----------------------------------------------------------------------------

class ImpliedOperation : public Operation
{
public:
  ImpliedOperation(SourcePos pos, Instruction& instruction) noexcept : Operation(pos, instruction) { }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;
};

// ----------------------------------------------------------------------------
//      ImmediateOperation
// ----------------------------------------------------------------------------

class ImmediateOperation : public Operation
{
public:
  ImmediateOperation(SourcePos pos, Instruction& instruction, ByteSelector selector, std::unique_ptr<Expression> expr) noexcept
    : Operation(pos, instruction), selector_(selector), expr_(std::move(expr)) { }

  ByteSelector selector() const noexcept { return selector_; }
  Expression& expr() const noexcept { return *expr_; }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  ByteSelector selector_;
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      AccumulatorOperation
// ----------------------------------------------------------------------------

class AccumulatorOperation : public Operation
{
public:
  AccumulatorOperation(SourcePos pos, Instruction& instruction) noexcept : Operation(pos, instruction) { }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;
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

  IndexRegister index() const noexcept { return index_; }
  bool forceAbsolute() const noexcept { return forceAbsolute_; }
  Expression& expr() const noexcept { return *expr_; }

  void setForceAbsolute(bool value) { forceAbsolute_ = value; }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  IndexRegister index_;
  bool forceAbsolute_;
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      IndirectOperation
// ----------------------------------------------------------------------------

class IndirectOperation : public Operation
{
public:
  IndirectOperation(SourcePos pos, Instruction& instruction, IndexRegister index, std::unique_ptr<Expression> expr) noexcept
    : Operation(pos, instruction), index_(index), expr_(std::move(expr)) { }

  IndexRegister index() const noexcept { return index_; }
  Expression& expr() const noexcept { return *expr_; }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  IndexRegister index_;
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

  Expression& expr() const noexcept { return *expr_; }

  void accept(StatementVisitor& visitor) override;
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
//      OriginDirective
// ----------------------------------------------------------------------------

class OriginDirective : public Directive
{
public:
  OriginDirective(SourcePos pos, std::unique_ptr<Expression> expr) noexcept : Directive(pos), expr_(std::move(expr)) { }

  Expression& expr() const noexcept { return *expr_; }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      BufferDirective
// ----------------------------------------------------------------------------

class BufferDirective : public Directive
{
public:
  BufferDirective(SourcePos pos, std::unique_ptr<Expression> expr) noexcept : Directive(pos), expr_(std::move(expr)) { }

  Expression& expr() const noexcept { return *expr_; }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      OffsetBeginDirective
// ----------------------------------------------------------------------------

class OffsetBeginDirective : public Directive
{
public:
  OffsetBeginDirective(SourcePos pos, std::unique_ptr<Expression> expr) noexcept : Directive(pos), expr_(std::move(expr)) { }

  Expression& expr() const noexcept { return *expr_; }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::unique_ptr<Expression> expr_;
};

// ----------------------------------------------------------------------------
//      OffsetEndDirective
// ----------------------------------------------------------------------------

class OffsetEndDirective : public Directive
{
public:
  OffsetEndDirective(SourcePos pos) noexcept : Directive(pos) { }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;
};

// ----------------------------------------------------------------------------
//      ObjectFileDirective
// ----------------------------------------------------------------------------

class ObjectFileDirective : public Directive
{
public:
  ObjectFileDirective(SourcePos pos, const std::string& filename) noexcept : Directive(pos), filename_(filename) { }

  std::string filename() const noexcept { return filename_; }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::string filename_;
};

// ----------------------------------------------------------------------------
//      ByteDirective
// ----------------------------------------------------------------------------

class ByteDirective : public Directive
{
public:
  ByteDirective(SourcePos pos, ByteSelector selector, std::vector<std::unique_ptr<Expression>> args) noexcept
    : Directive(pos), selector_(selector), args_(std::move(args)) { }

  ByteSelector selector() const noexcept { return selector_; }
  ByteLength byteLength() const noexcept { return args_.size(); }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  ByteSelector selector_;
  std::vector<std::unique_ptr<Expression>> args_;
};

// ----------------------------------------------------------------------------
//      WordDirective
// ----------------------------------------------------------------------------

class WordDirective : public Directive
{
public:
  WordDirective(SourcePos pos, std::vector<std::unique_ptr<Expression>> args) noexcept
    : Directive(pos), args_(std::move(args)) { }

  ByteLength byteLength() const noexcept { return args_.size() * 2; }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  std::vector<std::unique_ptr<Expression>> args_;
};

// ----------------------------------------------------------------------------
//      StringDirective
// ----------------------------------------------------------------------------

class StringDirective : public Directive
{
public:
  StringDirective(SourcePos pos, StringEncoding encoding, const std::string& str) noexcept
    : Directive(pos), encoding_(encoding), str_(str) { }

  ByteLength byteLength() const noexcept { return str_.length(); }

  void accept(StatementVisitor& visitor) override;
  void dump(std::ostream& s, int level = 0) const noexcept override;

private:
  StringEncoding encoding_;
  std::string str_;
};

// ----------------------------------------------------------------------------
//      StatementVisitor
// ----------------------------------------------------------------------------

class StatementVisitor
{
public:
  virtual void visit(SymbolDefinition& node) { }
  virtual void visit(ProgramCounterAssignment& node) { }
  virtual void visit(ImpliedOperation& node) { }
  virtual void visit(ImmediateOperation& node) { }
  virtual void visit(AccumulatorOperation& node) { }
  virtual void visit(DirectOperation& node) { }
  virtual void visit(IndirectOperation& node) { }
  virtual void visit(BranchOperation& node) { }
  virtual void visit(OriginDirective& node) { }
  virtual void visit(BufferDirective& node) { }
  virtual void visit(OffsetBeginDirective& node) { }
  virtual void visit(OffsetEndDirective& node) { }
  virtual void visit(ObjectFileDirective& node) { }
  virtual void visit(ByteDirective& node) { }
  virtual void visit(WordDirective& node) { }
  virtual void visit(StringDirective& node) { }

  virtual void uncaught(SourceError& err) { }
};

// ----------------------------------------------------------------------------
//      StatementList
// ----------------------------------------------------------------------------

class StatementList
{
public:
  void add(std::unique_ptr<Statement> statement) noexcept;

  void accept(StatementVisitor& visitor) const;
  void dump(std::ostream& s, int level = 0) const noexcept;;

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

  Maybe<Address> tryEval(Context& context);
  Address eval(Context& context);

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

  virtual Maybe<Address> value() const noexcept { return nullptr; }
  virtual std::unique_ptr<ExprNode> eval(Context& context, bool throwUndefined) { return nullptr; }
};

// ----------------------------------------------------------------------------
//      ExprConstant
// ----------------------------------------------------------------------------

class ExprConstant : public ExprNode
{
public:
  ExprConstant(SourcePos pos, int value) : ExprNode(pos), value_(value) { }

  Maybe<Address> value() const noexcept override { return value_; }
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

  std::unique_ptr<ExprNode> eval(Context& context, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  std::string name_;
};

// ----------------------------------------------------------------------------
//      ExprTemporarySymbol
// ----------------------------------------------------------------------------

class ExprTemporarySymbol : public ExprNode
{
public:
  ExprTemporarySymbol(SourcePos pos, int labelDelta)
    : ExprNode(pos), labelDelta_(labelDelta) { }

  std::unique_ptr<ExprNode> eval(Context& context, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  int labelDelta_;
};

// ----------------------------------------------------------------------------
//      ExprProgramCounter
// ----------------------------------------------------------------------------

class ExprProgramCounter : public ExprNode
{
public:
  ExprProgramCounter(SourcePos pos) : ExprNode(pos) { }

  std::unique_ptr<ExprNode> eval(Context& context, bool throwUndefined) override;
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

  std::unique_ptr<ExprNode> eval(Context& context, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  std::unique_ptr<ExprNode> left_;
  std::unique_ptr<ExprNode> right_;
  char op_;
  Handler handler_;
};

}
#endif
