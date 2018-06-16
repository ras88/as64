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

  void uncaught(SourceError& err) override;

private:
  void processLabel(const Statement& node);

  Context& context_;
};

DefinitionPass::DefinitionPass(Context& context)
  : context_(context)
{
}

void DefinitionPass::run()
{
  context_.statements.accept(*this);
}

void DefinitionPass::visit(const SymbolDefinition& node)
{
}

void DefinitionPass::visit(const ProgramCounterAssignment& node)
{
}

void DefinitionPass::visit(const ImpliedOperation& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const ImmediateOperation& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const AccumulatorOperation& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const DirectOperation& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const IndirectOperation& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const BranchOperation& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const OriginDirective& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const BufferDirective& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const OffsetBeginDirective& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const OffsetEndDirective& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const ObjectFileDirective& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const ByteDirective& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const WordDirective& node)
{
  processLabel(node);
}

void DefinitionPass::visit(const StringDirective& node)
{
  processLabel(node);
}

void DefinitionPass::uncaught(SourceError& err)
{
  context_.messages.add(Severity::Error, err.pos(), err.message());
}

void DefinitionPass::processLabel(const Statement& node)
{
  // TODO: remember not to add '+', etc. to symbol table!

}

void define(Context& context)
{
  DefinitionPass pass(context);
  pass.run();
}

}
