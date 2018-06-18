#include <iostream>
#include "enum.h"
#include "context.h"
#include "ast.h"

namespace cassm
{

constexpr int MaxHeaderWidth = 28;

static EnumTags<ByteSelector> g_byteSelectorTags =
{
  { ByteSelector::Low,                " [LSB]" },
  { ByteSelector::High,               " [MSB]" }
};

static EnumTags<IndexRegister> g_indexRegisterTags =
{
  { IndexRegister::X,                 " [,X]" },
  { IndexRegister::Y,                 " [,Y]" }
};

template<typename T> static void dumpList(std::ostream& s, const std::vector<std::unique_ptr<T>>& items, int level)
{
  for (size_t index = 0; index < items.size(); ++ index)
  {
    if (index)
      s << std::endl;
    items[index]->dump(s, level);
  }
}

// ----------------------------------------------------------------------------
//      Node
// ----------------------------------------------------------------------------

void Node::indent(std::ostream& s, int level) const noexcept
{
  auto header = pos_.toString();
  if (header.length() > MaxHeaderWidth)
    header = header.substr(header.length() - MaxHeaderWidth);
  s << '[' << header << ']';
  level += MaxHeaderWidth + 1 - header.length();
  while (level > 0)
  {
    s << ' ';
    -- level;
  }
}

// ----------------------------------------------------------------------------
//      Statement
// ----------------------------------------------------------------------------

void Statement::prefixLabel(std::ostream& s) const noexcept
{
  if (! label().isEmpty())
    s << '(' << label() << ") ";
}

// ----------------------------------------------------------------------------
//      EmptyStatement
// ----------------------------------------------------------------------------

void EmptyStatement::accept(StatementVisitor& visitor)
{
}

void EmptyStatement::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Empty Statement";
}

// ----------------------------------------------------------------------------
//      SymbolDefinition
// ----------------------------------------------------------------------------

void SymbolDefinition::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void SymbolDefinition::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Define: " << label() << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      ProgramCounterAssignment
// ----------------------------------------------------------------------------

void ProgramCounterAssignment::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void ProgramCounterAssignment::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Set Program Counter:" << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      ImpliedOperation
// ----------------------------------------------------------------------------

void ImpliedOperation::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void ImpliedOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Implied Mode Instruction: " << instruction().name();
}

// ----------------------------------------------------------------------------
//      ImmediateOperation
// ----------------------------------------------------------------------------

void ImmediateOperation::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void ImmediateOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Immediate Mode Instruction: " << instruction().name() << g_byteSelectorTags.fromValue(selector_) << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      AccumulatorOperation
// ----------------------------------------------------------------------------

void AccumulatorOperation::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void AccumulatorOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Accumulator Mode Instruction: " << instruction().name();
}

// ----------------------------------------------------------------------------
//      DirectOperation
// ----------------------------------------------------------------------------

void DirectOperation::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void DirectOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Direct Mode Instruction: " << instruction().name() << g_indexRegisterTags.fromValue(index_);
  if (forceAbsolute_)
    s << " [Force Absolute]";
  s << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      IndirectOperation
// ----------------------------------------------------------------------------

void IndirectOperation::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void IndirectOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Indirect Mode Instruction: " << instruction().name() << g_indexRegisterTags.fromValue(index_) << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      BranchOperation
// ----------------------------------------------------------------------------

void BranchOperation::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void BranchOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Branch Instruction: " << instruction().name() << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      OriginDirective
// ----------------------------------------------------------------------------

void OriginDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void OriginDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Origin Directive" << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      BufferDirective
// ----------------------------------------------------------------------------

void BufferDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void BufferDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Buffer Directive" << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      OffsetBeginDirective
// ----------------------------------------------------------------------------

void OffsetBeginDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void OffsetBeginDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Offset Begin Directive" << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      OffsetEndDirective
// ----------------------------------------------------------------------------

void OffsetEndDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void OffsetEndDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Offset End Directive";
}

// ----------------------------------------------------------------------------
//      ObjectFileDirective
// ----------------------------------------------------------------------------

void ObjectFileDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void ObjectFileDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Object File Directive: \"" << filename_ << '"';
}

// ----------------------------------------------------------------------------
//      ByteDirective
// ----------------------------------------------------------------------------

void ByteDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void ByteDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << args_.size() << " byte(s)" << g_byteSelectorTags.fromValue(selector_) << ':' << std::endl;
  dumpList(s, args_, level + 2);
}

// ----------------------------------------------------------------------------
//      WordDirective
// ----------------------------------------------------------------------------

void WordDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void WordDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << args_.size() << " word(s):" << std::endl;
  dumpList(s, args_, level + 2);
}

// ----------------------------------------------------------------------------
//      StringDirective
// ----------------------------------------------------------------------------

void StringDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void StringDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s <<  str_.length() << " byte string [" << toString(encoding_) << ']' << std::endl;
  indent(s, level + 2);
  s << '"' << str_ << '"';
}

// ----------------------------------------------------------------------------
//      StatementList
// ----------------------------------------------------------------------------

void StatementList::add(std::unique_ptr<Statement> statement) noexcept
{
  statements_.push_back(std::move(statement));
}

void StatementList::accept(StatementVisitor& visitor) const
{
  for (const auto& statement: statements_)
  {
    try
    {
      visitor.before(*statement);
      statement->accept(visitor);
      visitor.after(*statement);
    }
    catch (SourceError& err)
    {
      visitor.uncaught(err);
    }
  }
}

void StatementList::dump(std::ostream& s, int level) const noexcept
{
  dumpList(s, statements_, level);
}

// ----------------------------------------------------------------------------
//      Expression
// ----------------------------------------------------------------------------

Maybe<Address> Expression::tryEval(Context& context)
{
  auto root = root_->eval(context, false);
  if (root)
    root_ = std::move(root);
  return root_->value();
}

Address Expression::eval(Context& context)
{
  auto root = root_->eval(context, true);
  if (root)
    root_ = std::move(root);
  return root_->value().value();
}

void Expression::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Expression" << std::endl;
  root_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      ExprConstant
// ----------------------------------------------------------------------------

void ExprConstant::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Constant: " << value_;
}

// ----------------------------------------------------------------------------
//      ExprSymbol
// ----------------------------------------------------------------------------

std::unique_ptr<ExprNode> ExprSymbol::eval(Context& context, bool throwUndefined)
{
  auto value = context.symbols.get(name_);
  if (! value)
  {
    if (! throwUndefined)
      return nullptr;
    throwSourceError(pos(), "Undefined symbol '%s'", name_.c_str());
  }
  return std::make_unique<ExprConstant>(pos(), *value);
}

void ExprSymbol::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Symbol: " << name_;
}

// ----------------------------------------------------------------------------
//      ExprTemporarySymbol
// ----------------------------------------------------------------------------

std::unique_ptr<ExprNode> ExprTemporarySymbol::eval(Context& context, bool throwUndefined)
{
  auto value = context.symbols.get(context.pc, labelDelta_);
  if (! value)
  {
    if (! throwUndefined)
      return nullptr;
    throwSourceError(pos(), "No applicable temporary branch symbol found");
  }
  return std::make_unique<ExprConstant>(pos(), *value);
}

void ExprTemporarySymbol::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Temporary Label Delta = " << labelDelta_;
}

// ----------------------------------------------------------------------------
//      ExprProgramCounter
// ----------------------------------------------------------------------------

std::unique_ptr<ExprNode> ExprProgramCounter::eval(Context& context, bool throwUndefined)
{
  return std::make_unique<ExprConstant>(pos(), context.pc);
}

void ExprProgramCounter::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Program Counter";
}

// ----------------------------------------------------------------------------
//      ExprOperator
// ----------------------------------------------------------------------------

std::unique_ptr<ExprNode> ExprOperator::eval(Context& context, bool throwUndefined)
{
  auto left = left_->eval(context, throwUndefined);
  auto right = right_->eval(context, throwUndefined);
  if (left)
    left_ = std::move(left);
  if (right)
    right_ = std::move(right);

  auto a = left_->value(), b = right_->value();
  if (a && b)
  {
    auto result = handler_(*a, *b);
    if (result < 0 || result > 0xffff)
      throwSourceError(pos(), "Invalid operation result (%d); expected a number between 0 and 65535", result);
    return std::make_unique<ExprConstant>(pos(), handler_(*a, *b));
  }

  return nullptr;
}

void ExprOperator::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Operator: " << op_ << std::endl;
  left_->dump(s, level + 2);
  s << std::endl;
  right_->dump(s, level + 2);
}

}
