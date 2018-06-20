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
#include "emit.h"
#include "context.h"
#include "ast.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      CodeGenerationPass
// ----------------------------------------------------------------------------

class CodeGenerationPass : public StatementVisitor
{
public:
  CodeGenerationPass(Context& context);

  void run();

  void visit(ProgramCounterAssignment& node) override;
  void visit(ImpliedOperation& node) override;
  void visit(ImmediateOperation& node) override;
  void visit(AccumulatorOperation& node) override;
  void visit(DirectOperation& node) override;
  void visit(IndirectOperation& node) override;
  void visit(BranchOperation& node) override;
  void visit(BufferDirective& node) override;
  void visit(ObjectFileDirective& node) override;
  void visit(ByteDirective& node) override;
  void visit(WordDirective& node) override;
  void visit(StringDirective& node) override;

  bool before(Statement& node) override;
  void after(Statement& node) override;
  bool uncaught(SourceError& err) override;

private:
  void invalidInstruction(SourcePos pos);
  void newBuffer();

  Context& context_;
  CodeWriter writer_;
  Offset start_;
};

CodeGenerationPass::CodeGenerationPass(Context& context)
  : context_(context)
{
  newBuffer();
}

void CodeGenerationPass::run()
{
  context_.statements.accept(*this);
}

bool CodeGenerationPass::before(Statement& node)
{
  context_.pc = node.pc();
  start_ = writer_.offset();
  if (writer_.buffer()->isEmpty())
    writer_.buffer()->setOrigin(node.pc());
  return ! node.isSkipped();
}

void CodeGenerationPass::after(Statement& node)
{
  node.setRange({ writer_.buffer(), start_, writer_.offset() });
}

void CodeGenerationPass::visit(ProgramCounterAssignment& node)
{
  auto addr = node.expr().eval(context_);
  if (! writer_.buffer()->isEmpty() && addr < context_.pc)
    throwSourceError(node.pos(), "Invalid program counter assignment (address $%04x < pc $%04x)", addr, context_.pc);
  writer_.fill(addr - context_.pc);
}

void CodeGenerationPass::visit(ImpliedOperation& node)
{
  if (! node.instruction().encodeImplied(&writer_).hasValue())
    invalidInstruction(node.pos());
}

void CodeGenerationPass::visit(ImmediateOperation& node)
{
  auto value = select(node.selector(), node.expr().eval(context_));
  if (! value.hasValue())
    throwSourceError(node.pos(), "Expected a value between 0 and 255; got %d", node.expr().eval(context_));
  if (! node.instruction().encodeImmediate(&writer_, *value).hasValue())
    invalidInstruction(node.pos());
}

void CodeGenerationPass::visit(AccumulatorOperation& node)
{
  if (! node.instruction().encodeAccumulator(&writer_).hasValue())
    invalidInstruction(node.pos());
}

void CodeGenerationPass::visit(DirectOperation& node)
{
  auto addr = node.expr().eval(context_);
  if (! node.instruction().encodeDirect(&writer_, addr, node.index(), node.forceAbsolute()).hasValue())
    invalidInstruction(node.pos());
}

void CodeGenerationPass::visit(IndirectOperation& node)
{
  auto addr = node.expr().eval(context_);
  if ( ! node.instruction().encodeIndirect(&writer_, addr, node.index()).hasValue())
    invalidInstruction(node.pos());
}

void CodeGenerationPass::visit(BranchOperation& node)
{
  auto addr = node.expr().eval(context_);
  if (! node.instruction().encodeRelative(&writer_, context_.pc, addr).hasValue())
    throwSourceError(node.pos(), "Branch out of range");
}

void CodeGenerationPass::visit(BufferDirective& node)
{
  writer_.fill(node.expr().eval(context_));
}

void CodeGenerationPass::visit(ObjectFileDirective& node)
{
  if (! writer_.buffer()->isEmpty())
    newBuffer();
  auto& buffer = *writer_.buffer();
  buffer.setFilename(node.filename());
}

void CodeGenerationPass::visit(ByteDirective& node)
{
  for (const auto& expr: node)
  {
    auto value = select(node.selector(), expr->eval(context_));
    if (! value.hasValue())
      throwSourceError(expr->pos(), "Expected a value between 0 and 255; got %d", expr->eval(context_));
    writer_.byte(*value);
  }
}

void CodeGenerationPass::visit(WordDirective& node)
{
  for (const auto& expr: node)
    writer_.word(expr->eval(context_));
}

void CodeGenerationPass::visit(StringDirective& node)
{
  auto str = encode(node.encoding(), node.str());
  for (const auto& c: str)
    writer_.byte(c);
}

bool CodeGenerationPass::uncaught(SourceError& err)
{
  context_.messages.add(Severity::Error, err.pos(), err.message(), err.isFatal());
  return ! err.isFatal();
}

void CodeGenerationPass::invalidInstruction(SourcePos pos)
{
  throwSourceError(pos, "INTERNAL ERROR! Failed to encode instruction");
}

void CodeGenerationPass::newBuffer()
{
  auto buffer = std::make_unique<CodeBuffer>();
  writer_.attach(buffer.get());
  context_.buffers.push_back(std::move(buffer));
}

void emit(Context& context)
{
  CodeGenerationPass pass(context);
  pass.run();
}

}
