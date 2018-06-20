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
#include <vector>
#include <algorithm>
#include "define.h"
#include "context.h"
#include "ast.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      DefinitionPass
// ----------------------------------------------------------------------------

class DefinitionPass : public StatementVisitor
{
public:
  DefinitionPass(Context& context);

  void run();

  void visit(SymbolDefinition& node) override;
  void visit(ProgramCounterAssignment& node) override;
  void visit(ImpliedOperation& node) override;
  void visit(ImmediateOperation& node) override;
  void visit(AccumulatorOperation& node) override;
  void visit(DirectOperation& node) override;
  void visit(IndirectOperation& node) override;
  void visit(BranchOperation& node) override;
  void visit(OriginDirective& node) override;
  void visit(BufferDirective& node) override;
  void visit(OffsetBeginDirective& node) override;
  void visit(OffsetEndDirective& node) override;
  void visit(ByteDirective& node) override;
  void visit(WordDirective& node) override;
  void visit(StringDirective& node) override;
  void visit(IfDirective& node) override;
  void visit(IfdefDirective& node) override;
  void visit(ElseDirective& node) override;
  void visit(EndifDirective& node) override;
  void visit(EndDirective& node) override;

  bool before(Statement& node) override;
  bool uncaught(SourceError& err) override;

private:
  struct Conditional
  {
    Statement *node;
    bool value;
  };

  void processLabel(Statement& node);
  void setLabel(Statement& node, Address value);
  void updateSkipFlag();
  void advance(SourcePos pos, ByteLength count);

  Context& context_;
  std::vector<Address> offsetStack_;
  bool skipping_;
  bool ended_;
  std::vector<Conditional> conditionalStack_;
};

DefinitionPass::DefinitionPass(Context& context)
  : context_(context), skipping_(false), ended_(false)
{
}

void DefinitionPass::run()
{
  context_.statements.accept(*this);

  for (const auto& cond: conditionalStack_)
    context_.messages.add(Severity::Error, cond.node->pos(), "Missing corresponding .ife");
}

bool DefinitionPass::before(Statement& node)
{
  node.setPc(context_.pc);
  if (ended_ || (skipping_ && ! node.isConditional()))
  {
    node.skip();
    return false;
  }
  return true;
}

void DefinitionPass::visit(SymbolDefinition& node)
{
  setLabel(node, node.expr().eval(context_));
}

void DefinitionPass::visit(ProgramCounterAssignment& node)
{
  context_.pc = node.expr().eval(context_);
}

void DefinitionPass::visit(ImpliedOperation& node)
{
  processLabel(node);

  auto length = node.instruction().encodeImplied(nullptr);
  if (! length.hasValue())
    throwSourceError(node.pos(), "Instruction '%s' does not support implied addressing", node.instruction().name().c_str());

  advance(node.pos(), *length);
}

void DefinitionPass::visit(ImmediateOperation& node)
{
  processLabel(node);

  auto length = node.instruction().encodeImmediate(nullptr, 0);
  if (! length.hasValue())
    throwSourceError(node.pos(), "Instruction '%s' does not support immediate addressing", node.instruction().name().c_str());

  advance(node.pos(), *length);
}

void DefinitionPass::visit(AccumulatorOperation& node)
{
  processLabel(node);

  auto length = node.instruction().encodeAccumulator(nullptr);
  if (! length.hasValue())
    throwSourceError(node.pos(), "Instruction '%s' does not support accumulator addressing", node.instruction().name().c_str());

  advance(node.pos(), *length);
}

void DefinitionPass::visit(DirectOperation& node)
{
  processLabel(node);

  // This type of addressing can result in either a 2 or 3 byte instruction. To figure out
  // whether the zero-page variation can be used, attempt to evaluate the expression.
  // If evaluation fails, force absolute mode for all future passes. (Zero-page addressing
  // requires all symbols referenced by the expression to be previously defined.)
  auto addr = node.expr().tryEval(context_);
  if (! addr.hasValue())
    node.setForceAbsolute(true);
  auto length = node.instruction().encodeDirect(nullptr, addr.value(0), node.index(), node.forceAbsolute());
  if (! length.hasValue())
  {
    if (node.index() != IndexRegister::None)
      throwSourceError(node.pos(), "Instruction '%s' does not support indexed addressing via %s",
                       node.instruction().name().c_str(), toString(node.index()).c_str());
    throwSourceError(node.pos(), "Instruction '%s' does not support direct addressing", node.instruction().name().c_str());
  }

  advance(node.pos(), *length);
}

void DefinitionPass::visit(IndirectOperation& node)
{
  processLabel(node);

  auto length = node.instruction().encodeIndirect(nullptr, 0, node.index());
  if (! length.hasValue())
  {
    if (node.index() != IndexRegister::None)
      throwSourceError(node.pos(), "Instruction '%s' does not support indirect addressing via %s",
                       node.instruction().name().c_str(), toString(node.index()).c_str());
    throwSourceError(node.pos(), "Instruction '%s' does not support indirect addressing", node.instruction().name().c_str());
  }

  advance(node.pos(), *length);
}

void DefinitionPass::visit(BranchOperation& node)
{
  processLabel(node);

  auto length = node.instruction().encodeRelative(nullptr, 0);
  if (! length.hasValue())
    throwSourceError(node.pos(), "Instruction '%s' is not a branch instruction", node.instruction().name().c_str());

  advance(node.pos(), *length);
}

void DefinitionPass::visit(OriginDirective& node)
{
  processLabel(node);

  context_.pc = node.expr().eval(context_);
}

void DefinitionPass::visit(BufferDirective& node)
{
  processLabel(node);

  advance(node.pos(), node.expr().eval(context_));
}

void DefinitionPass::visit(OffsetBeginDirective& node)
{
  processLabel(node);

  offsetStack_.push_back(context_.pc);
  context_.pc = node.expr().eval(context_);
}

void DefinitionPass::visit(OffsetEndDirective& node)
{
  processLabel(node);

  if (offsetStack_.empty())
    throwSourceError(node.pos(), "Program counter is not offset");

  context_.pc = offsetStack_.back();
  offsetStack_.pop_back();
}

void DefinitionPass::visit(ByteDirective& node)
{
  processLabel(node);

  advance(node.pos(), node.byteLength());
}

void DefinitionPass::visit(WordDirective& node)
{
  processLabel(node);

  advance(node.pos(), node.byteLength());
}

void DefinitionPass::visit(StringDirective& node)
{
  processLabel(node);

  advance(node.pos(), node.byteLength());
}

void DefinitionPass::visit(IfDirective& node)
{
  conditionalStack_.push_back({ &node, node.expr().eval(context_) != 0 });
  updateSkipFlag();
}

void DefinitionPass::visit(IfdefDirective& node)
{
  conditionalStack_.push_back({ &node, context_.symbols.exists(node.name()) });
  updateSkipFlag();
}

void DefinitionPass::visit(ElseDirective& node)
{
  if (conditionalStack_.empty())
    throwSourceError(node.pos(), ".else without a corresponding .if or .ifdef");
  conditionalStack_.back().value = ! conditionalStack_.back().value;
  updateSkipFlag();
}

void DefinitionPass::visit(EndifDirective& node)
{
  if (conditionalStack_.empty())
    throwSourceError(node.pos(), ".ife without a corresponding .if or .ifdef");
  conditionalStack_.pop_back();
  updateSkipFlag();
}

void DefinitionPass::visit(EndDirective& node)
{
  ended_ = true;
}

bool DefinitionPass::uncaught(SourceError& err)
{
  context_.messages.add(Severity::Error, err.pos(), err.message(), err.isFatal());
  return ! err.isFatal();
}

void DefinitionPass::processLabel(Statement& node)
{
  setLabel(node, context_.pc);
}

void DefinitionPass::setLabel(Statement& node, Address value)
{
  if (! context_.symbols.set(node.label(), value))
    throwSourceError(node.pos(), "Symbol '%s' already exists", node.label().name().c_str());
}

void DefinitionPass::updateSkipFlag()
{
  skipping_ = std::find_if(std::begin(conditionalStack_), std::end(conditionalStack_),
                           [](const auto& cond) { return cond.value == false; }) != std::end(conditionalStack_);
}

void DefinitionPass::advance(SourcePos pos, ByteLength count)
{
  if (context_.pc + count > 65536)
    throwFatalSourceError(pos, "16-bit address overflow");
  context_.pc += count;

  // The original program counter continues to advance even when one or more offsets is in effect.
  for (auto& addr: offsetStack_)
  {
    if (count > 65536 - addr)
      throwFatalSourceError(pos, "16-bit address overflow");
    addr += count;
  }
}

void define(Context& context)
{
  DefinitionPass pass(context);
  pass.run();
}

}
