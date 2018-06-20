#ifndef _INCLUDED_CASSM_CONTEXT_H
#define _INCLUDED_CASSM_CONTEXT_H

#include <memory>
#include <vector>
#include "source.h"
#include "ast.h"
#include "message.h"
#include "symbol.h"
#include "buffer.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      Context
// ----------------------------------------------------------------------------

struct Context
{
  Context() : pc(0) { }

  SourceStream source;
  StatementList statements;
  MessageList messages;
  SymbolTable symbols;
  std::vector<std::unique_ptr<CodeBuffer>> buffers;

  ProgramCounter pc;
};

}
#endif
