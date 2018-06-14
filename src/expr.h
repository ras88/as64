#ifndef _INCLUDED_CASSM_EXPR_H
#define _INCLUDED_CASSM_EXPR_H

#include <string>
#include <ostream>
#include <memory>
#include "table.h"
#include "source.h"
#include "buffer.h"

namespace cassm
{

class ExprNode;
using OwnedExprNode = std::unique_ptr<ExprNode>;

// ----------------------------------------------------------------------------
//      Expression
// ----------------------------------------------------------------------------

class Expression
{
public:
  Expression(LineReader& reader);
  ~Expression();

  // If throwUndefined is false, this method returns -1 if the expression cannot
  // be resolved due to one or more undefined symbols. Any other error condition
  // always throws a SourceError.
  int eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined);

  void dump(std::ostream& s, int level = 0);

private:
  OwnedExprNode parseOperand(LineReader& reader);

  OwnedExprNode root_;
};

}
#endif
