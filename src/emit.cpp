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
  void visit(OriginDirective& node) override;
  void visit(BufferDirective& node) override;
  void visit(OffsetBeginDirective& node) override;
  void visit(OffsetEndDirective& node) override;
  void visit(ObjectFileDirective& node) override;
  void visit(ByteDirective& node) override;
  void visit(WordDirective& node) override;
  void visit(StringDirective& node) override;

  void uncaught(SourceError& err) override;

private:
  void advance(SourcePos pos, Address count);
  void invalidInstruction();

  Context& context_;
  CodeWriter writer_;
};

CodeGenerationPass::CodeGenerationPass(Context& context)
  : context_(context)
{
  auto buffer = std::make_unique<CodeBuffer>();
  writer_.attach(buffer.get());
  context_.buffers.push_back(std::move(buffer));
}

void CodeGenerationPass::run()
{
  context_.pc = 0;
  context_.statements.accept(*this);
}

void CodeGenerationPass::visit(ProgramCounterAssignment& node)
{
  // TODO: emit zeros as necessary to reach the target address 
  context_.pc = node.expr().eval(context_);
}

void CodeGenerationPass::visit(ImpliedOperation& node)
{
  auto length = node.instruction().encodeImplied(&writer_);
  if (! length)
    invalidInstruction();
  advance(node.pos(), *length);
}

void CodeGenerationPass::visit(ImmediateOperation& node)
{
  auto value = select(node.selector(), node.expr().eval(context_));
  if (! value)
    throwSourceError(node.pos(), "Expected a value between 0 and 255; got %d", node.expr().eval(context_));
  auto length = node.instruction().encodeImmediate(&writer_, *value);
  if (! length)
    invalidInstruction();
  advance(node.pos(), *length);
}

void CodeGenerationPass::visit(AccumulatorOperation& node)
{
  auto length = node.instruction().encodeAccumulator(&writer_);
  if (! length)
    invalidInstruction();
  advance(node.pos(), *length);
}

void CodeGenerationPass::visit(DirectOperation& node)
{
  auto addr = node.expr().eval(context_);
  auto length = node.instruction().encodeDirect(&writer_, addr, node.index(), node.forceAbsolute());
  if (! length)
    invalidInstruction();
  advance(node.pos(), *length);
}

void CodeGenerationPass::visit(IndirectOperation& node)
{
  auto addr = node.expr().eval(context_);
  auto length = node.instruction().encodeIndirect(&writer_, addr, node.index());
  if (! length)
    invalidInstruction();
  advance(node.pos(), *length);
}

void CodeGenerationPass::visit(BranchOperation& node)
{
  auto addr = node.expr().eval(context_);
  auto length = node.instruction().encodeRelative(&writer_, context_.pc, addr);
  if (! length)
    throwSourceError(node.pos(), "Branch out of range");
  advance(node.pos(), *length);
}

void CodeGenerationPass::visit(OriginDirective& node)
{
  context_.pc = node.expr().eval(context_);
}

void CodeGenerationPass::visit(BufferDirective& node)
{
  // TODO
  advance(node.pos(), node.expr().eval(context_));
}

void CodeGenerationPass::visit(OffsetBeginDirective& node)
{
  // TODO
}

void CodeGenerationPass::visit(OffsetEndDirective& node)
{
  // TODO
}

void CodeGenerationPass::visit(ObjectFileDirective& node)
{
  // TODO
}

void CodeGenerationPass::visit(ByteDirective& node)
{
  // TODO
  advance(node.pos(), node.byteLength());
}

void CodeGenerationPass::visit(WordDirective& node)
{
  // TODO
  advance(node.pos(), node.byteLength());
}

void CodeGenerationPass::visit(StringDirective& node)
{
  // TODO
  advance(node.pos(), node.byteLength());
}

void CodeGenerationPass::uncaught(SourceError& err)
{
  context_.messages.add(Severity::Error, err.pos(), err.message());
}

void CodeGenerationPass::advance(SourcePos pos, Address count)
{
  // TODO: should probably be a fatal error
  if (count > 65536 - context_.pc)
    throwSourceError(pos, "16-bit address overflow");
  context_.pc += count;
}

void CodeGenerationPass::invalidInstruction()
{
  // TODO: terminate due to internal error
  std::cerr << "Invalid instruction" << std::endl;
  std::terminate();
}

void emit(Context& context)
{
  CodeGenerationPass pass(context);
  pass.run();
}

}
