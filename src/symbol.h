#ifndef _INCLUDED_CASSM_SYMBOL_H
#define _INCLUDED_CASSM_SYMBOL_H

#include <vector>
#include <unordered_map>
#include <utility>
#include <ostream>
#include "types.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      SymbolTable
// ----------------------------------------------------------------------------

class SymbolTable
{
public:
  SymbolTable();

  // Returns false if a symbol already exists with the given label.
  bool set(const Label& label, Address addr) noexcept;
  bool set(const std::pair<Label, Address>& symbol) noexcept { return set(symbol.first, symbol.second); }

  bool exists(const std::string& name) const noexcept { return symbols_.find(name) != std::end(symbols_); }
  Maybe<Address> get(const std::string& name) const noexcept;
  Maybe<Address> get(Address addr, int labelDelta) const noexcept;

  void write(std::ostream& s) const noexcept;

private:
  struct Symbol
  {
    Address address;
    int serialNum;
  };

  struct Temporary
  {
    LabelType type;
    Address addr;
  };

  std::unordered_map<std::string, Symbol> symbols_;
  std::vector<Temporary> temps_;
  int nextSerialNum_;
};

}
#endif
