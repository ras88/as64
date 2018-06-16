#include <iostream>
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

  void visit(const SymbolDefinition& node) override;
  void visit(const ProgramCounterAssignment& node) override;
  void visit(const ImpliedOperation& node) override;
  void visit(const ImmediateOperation& node) override;
  void visit(const AccumulatorOperation& node) override;
  void visit(const DirectOperation& node) override;
  void visit(const IndirectOperation& node) override;
  void visit(const BranchOperation& node) override;
  void visit(const OriginDirective& node) override;
  void visit(const BufferDirective& node) override;
  void visit(const OffsetBeginDirective& node) override;
  void visit(const OffsetEndDirective& node) override;
  void visit(const ObjectFileDirective& node) override;
  void visit(const ByteDirective& node) override;
  void visit(const WordDirective& node) override;
  void visit(const StringDirective& node) override;

private:
  Context& context_;
};

DefinitionPass::DefinitionPass(Context& context)
  : context_(context)
{
}

void DefinitionPass::run()
{
  context_.statements().accept(*this);
}

void DefinitionPass::visit(const SymbolDefinition& node)
{
}

void DefinitionPass::visit(const ProgramCounterAssignment& node)
{
}

void DefinitionPass::visit(const ImpliedOperation& node)
{
}

void DefinitionPass::visit(const ImmediateOperation& node)
{
}

void DefinitionPass::visit(const AccumulatorOperation& node)
{
}

void DefinitionPass::visit(const DirectOperation& node)
{
}

void DefinitionPass::visit(const IndirectOperation& node)
{
}

void DefinitionPass::visit(const BranchOperation& node)
{
}

void DefinitionPass::visit(const OriginDirective& node)
{
}

void DefinitionPass::visit(const BufferDirective& node)
{
}

void DefinitionPass::visit(const OffsetBeginDirective& node)
{
}

void DefinitionPass::visit(const OffsetEndDirective& node)
{
}

void DefinitionPass::visit(const ObjectFileDirective& node)
{
}

void DefinitionPass::visit(const ByteDirective& node)
{
}

void DefinitionPass::visit(const WordDirective& node)
{
}

void DefinitionPass::visit(const StringDirective& node)
{
}

void define(Context& context)
{
  DefinitionPass pass(context);
  pass.run();
}

}
