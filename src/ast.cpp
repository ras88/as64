// Copyright (c) 2018 Robert A. Stoerrle
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
// OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <iostream>
#include "enum.h"
#include "context.h"
#include "ast.h"

namespace as64
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
//      BitmapDirective
// ----------------------------------------------------------------------------

void BitmapDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void BitmapDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Bitmap: " << args_.size() << " byte(s)";
}

// ----------------------------------------------------------------------------
//      IfDirective
// ----------------------------------------------------------------------------

void IfDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void IfDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "If Directive" << std::endl;
  expr_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      IfdefDirective
// ----------------------------------------------------------------------------

void IfdefDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void IfdefDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Ifdef Directive: " << name_;
}

// ----------------------------------------------------------------------------
//      ElseDirective
// ----------------------------------------------------------------------------

void ElseDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void ElseDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Else Directive";
}

// ----------------------------------------------------------------------------
//      EndifDirective
// ----------------------------------------------------------------------------

void EndifDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void EndifDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "Endif Directive";
}

// ----------------------------------------------------------------------------
//      EndDirective
// ----------------------------------------------------------------------------

void EndDirective::accept(StatementVisitor& visitor)
{
  visitor.visit(*this);
}

void EndDirective::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  prefixLabel(s);
  s << "End Directive";
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
      if (visitor.before(*statement))
        statement->accept(visitor);
      visitor.after(*statement);
    }
    catch (SourceError& err)
    {
      if (! visitor.uncaught(err))
        break;;
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
  if (! value.hasValue())
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
  if (! value.hasValue())
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
  if (a.hasValue() && b.hasValue())
  {
    Address result;
    switch (op_)
    {
      case '+':
        result = *a + *b;
        break;

      case '-':
        result = *a - *b;
        break;

      case '*':
        result = *a * *b;
        break;

      case '/':
        if (*b == 0)
          throwSourceError(pos(), "Integer division by zero");
        result = *a / *b;
        break;

      default:
        throwSourceError(pos(), "Invalid expression operator '%c'", op_);
    }
    if (result < 0 || result > 0xffff)
      throwSourceError(pos(), "Invalid operation result (%d); expected a number between 0 and 65535", result);
    return std::make_unique<ExprConstant>(pos(), result);
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
