#ifndef _INCLUDED_CASSM_CONTEXT_H
#define _INCLUDED_CASSM_CONTEXT_H

#include "source.h"
#include "ast.h"
#include "message.h"
#include "table.h"

namespace cassm
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
  SymbolTable<Address> symbols;

  Address pc;
};

}
#endif
