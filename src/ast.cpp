#include <iostream>
#include "error.h"
#include "ast.h"

namespace cassm
{

constexpr int MaxHeaderWidth = 20;

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
//      SymbolDefinition
// ----------------------------------------------------------------------------

void SymbolDefinition::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Define: " << label() << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      ProgramCounterAssignment
// ----------------------------------------------------------------------------

void ProgramCounterAssignment::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Set Program Counter:" << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      ImmediateOperation
// ----------------------------------------------------------------------------

void ImmediateOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Immediate Mode Instruction: " << instruction().name();
  switch (selector_)
  {
    case ByteSelector::Low:
      s << " [LSB]";
      break;

    case ByteSelector::High:
      s << " [MSB]";
      break;

    default:
      break;
  }
  s << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      DirectOperation
// ----------------------------------------------------------------------------

void DirectOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Direct Mode Instruction: " << instruction().name();
  switch (index_)
  {
    case IndexRegister::X:
      s << " [,X]";
      break;

    case IndexRegister::Y:
      s << " [,Y]";
      break;

    default:
      break;
  }
  if (forceAbsolute_)
    s << " [Force Absolute]";
  s << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      BranchOperation
// ----------------------------------------------------------------------------

void BranchOperation::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Branch Instruction: " << instruction().name() << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      StatementList
// ----------------------------------------------------------------------------

void StatementList::add(std::unique_ptr<Statement> statement) noexcept
{
  statements_.push_back(std::move(statement));
}

void StatementList::dump(std::ostream& s, int level) const noexcept
{
  for (size_t index = 0; index < statements_.size(); ++ index)
  {
    if (index)
      s << std::endl;
    statements_[index]->dump(s, level);
  }
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
