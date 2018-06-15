#ifndef _INCLUDED_CASSM_CONTEXT_H
#define _INCLUDED_CASSM_CONTEXT_H

#include "source.h"
#include "ast.h"
#include "message.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      Context
// ----------------------------------------------------------------------------

class Context
{
public:
  Context();
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  SourceStream& source() { return source_; }
  StatementList& statements() { return statements_; }
  MessageList& messages() { return messages_; }

private:
  SourceStream source_;
  StatementList statements_;
  MessageList messages_;
};

}
#endif
