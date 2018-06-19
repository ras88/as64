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

  void before(Statement& node) override;
  void after(Statement& node) override;
  bool uncaught(SourceError& err) override;

private:
  void willEmit();
  void invalidInstruction();
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

void CodeGenerationPass::before(Statement& node)
{
  context_.pc = node.pc();
  start_ = writer_.offset();
}

void CodeGenerationPass::after(Statement& node)
{
  node.setRange({ writer_.buffer(), start_, writer_.offset() });
}

void CodeGenerationPass::visit(ProgramCounterAssignment& node)
{
  willEmit();
  auto addr = node.expr().eval(context_);
  if (! writer_.buffer()->isEmpty() && addr < context_.pc)
    throwSourceError(node.pos(), "Invalid program counter assignment (address $%04x < pc $%04x)", addr, context_.pc);
  writer_.fill(addr - context_.pc);
}

void CodeGenerationPass::visit(ImpliedOperation& node)
{
  willEmit();
  if (! node.instruction().encodeImplied(&writer_))
    invalidInstruction();
}

void CodeGenerationPass::visit(ImmediateOperation& node)
{
  willEmit();
  auto value = select(node.selector(), node.expr().eval(context_));
  if (! value)
    throwSourceError(node.pos(), "Expected a value between 0 and 255; got %d", node.expr().eval(context_));
  if (! node.instruction().encodeImmediate(&writer_, *value))
    invalidInstruction();
}

void CodeGenerationPass::visit(AccumulatorOperation& node)
{
  willEmit();
  if (! node.instruction().encodeAccumulator(&writer_))
    invalidInstruction();
}

void CodeGenerationPass::visit(DirectOperation& node)
{
  willEmit();
  auto addr = node.expr().eval(context_);
  if (! node.instruction().encodeDirect(&writer_, addr, node.index(), node.forceAbsolute()))
    invalidInstruction();
}

void CodeGenerationPass::visit(IndirectOperation& node)
{
  willEmit();
  auto addr = node.expr().eval(context_);
  if ( ! node.instruction().encodeIndirect(&writer_, addr, node.index()))
    invalidInstruction();
}

void CodeGenerationPass::visit(BranchOperation& node)
{
  willEmit();
  auto addr = node.expr().eval(context_);
  if (! node.instruction().encodeRelative(&writer_, context_.pc, addr))
    throwSourceError(node.pos(), "Branch out of range");
}

void CodeGenerationPass::visit(BufferDirective& node)
{
  willEmit();
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
  willEmit();
  for (const auto& expr: node)
  {
    auto value = select(node.selector(), expr->eval(context_));
    if (! value)
      throwSourceError(expr->pos(), "Expected a value between 0 and 255; got %d", expr->eval(context_));
    writer_.byte(*value);
  }
}

void CodeGenerationPass::visit(WordDirective& node)
{
  willEmit();
  for (const auto& expr: node)
    writer_.word(expr->eval(context_));
}

void CodeGenerationPass::visit(StringDirective& node)
{
  willEmit();
  auto str = encode(node.encoding(), node.str());
  for (const auto& c: str)
    writer_.byte(c);
}

void CodeGenerationPass::willEmit()
{
  if (writer_.buffer()->isEmpty())
    writer_.buffer()->setOrigin(context_.pc);
}

bool CodeGenerationPass::uncaught(SourceError& err)
{
  context_.messages.add(Severity::Error, err.pos(), err.message(), err.isFatal());
  return ! err.isFatal();
}

void CodeGenerationPass::invalidInstruction()
{
  // TODO: terminate due to internal error
  std::cerr << "Invalid instruction" << std::endl;
  std::terminate();
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
