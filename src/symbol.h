#ifndef _INCLUDED_CASSM_SYMBOL_H
#define _INCLUDED_CASSM_SYMBOL_H

#include <vector>
#include <unordered_map>
#include "types.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      SymbolTable
// ----------------------------------------------------------------------------

class SymbolTable
{
public:
  // Returns false if a symbol already exists with the given label.
  bool set(const Label& label, Address addr) noexcept;

  Maybe<Address> get(const std::string& name) const noexcept;
  Maybe<Address> get(Address addr, int labelDelta) const noexcept;

private:
  struct Temporary
  {
    LabelType type;
    Address addr;
  };

  std::unordered_map<std::string, Address> symbols_;
  std::vector<Temporary> temps_;
};

}
#endif
