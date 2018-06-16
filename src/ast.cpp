#include <iostream>
#include "enum.h"
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
  if (hasLabel())
    s << '(' << label() << ") ";
}

// ----------------------------------------------------------------------------
//      EmptyStatement
// ----------------------------------------------------------------------------

void EmptyStatement::accept(StatementVisitor& visitor) const
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

void SymbolDefinition::accept(StatementVisitor& visitor) const
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

void ProgramCounterAssignment::accept(StatementVisitor& visitor) const
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

void ImpliedOperation::accept(StatementVisitor& visitor) const
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

void ImmediateOperation::accept(StatementVisitor& visitor) const
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

void AccumulatorOperation::accept(StatementVisitor& visitor) const
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

void DirectOperation::accept(StatementVisitor& visitor) const
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

void IndirectOperation::accept(StatementVisitor& visitor) const
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

void BranchOperation::accept(StatementVisitor& visitor) const
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

void OriginDirective::accept(StatementVisitor& visitor) const
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

void BufferDirective::accept(StatementVisitor& visitor) const
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

void OffsetBeginDirective::accept(StatementVisitor& visitor) const
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

void OffsetEndDirective::accept(StatementVisitor& visitor) const
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

void ObjectFileDirective::accept(StatementVisitor& visitor) const
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

void ByteDirective::accept(StatementVisitor& visitor) const
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

void WordDirective::accept(StatementVisitor& visitor) const
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

void StringDirective::accept(StatementVisitor& visitor) const
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
    statement->accept(visitor);
}

void StatementList::dump(std::ostream& s, int level) const noexcept
{
  dumpList(s, statements_, level);
}

// ----------------------------------------------------------------------------
//      Expression
// ----------------------------------------------------------------------------

int Expression::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  auto root = root_->eval(pc, symbols, throwUndefined);
  if (root)
    root_ = std::move(root);
  return root_->checkValue();
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

int ExprConstant::checkValue() const
{
  int value = this->value();
  if (value < 0 || value > 0xffff)
    throwSourceError(pos(), "Invalid expression result (%d); expected a number between 0 and 65535", value);
  return value;
}

void ExprConstant::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Constant: " << value_;
}

// ----------------------------------------------------------------------------
//      ExprSymbol
// ----------------------------------------------------------------------------

std::unique_ptr<ExprNode> ExprSymbol::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  auto *value = symbols.get(name_);
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

std::unique_ptr<ExprNode> ExprTemporarySymbol::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  // TODO
  return std::make_unique<ExprConstant>(pos(), 0);
}

void ExprTemporarySymbol::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << (direction_ == BranchDirection::Backward ? "Backward Temporary Symbol" : "Forward Temporary Symbol");
  s << ": count=" << count_;
}

// ----------------------------------------------------------------------------
//      ExprProgramCounter
// ----------------------------------------------------------------------------

std::unique_ptr<ExprNode> ExprProgramCounter::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  return std::make_unique<ExprConstant>(pos(), pc);
}

void ExprProgramCounter::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Program Counter";
}

// ----------------------------------------------------------------------------
//      ExprOperator
// ----------------------------------------------------------------------------

std::unique_ptr<ExprNode> ExprOperator::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  auto left = left_->eval(pc, symbols, throwUndefined);
  auto right = right_->eval(pc, symbols, throwUndefined);
  if (left)
    left_ = std::move(left);
  if (right)
    right_ = std::move(right);

  auto a = left_->value(), b = right_->value();
  if (a != -1 && b != -1)
    return std::make_unique<ExprConstant>(pos(), handler_(a, b));

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
