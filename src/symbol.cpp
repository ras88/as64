#include <iostream>
#include <algorithm>
#include "symbol.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      SymbolTable
// ----------------------------------------------------------------------------

bool SymbolTable::set(const Label& label, Address addr) noexcept
{
  switch (label.type())
  {
    case LabelType::Symbolic:
    {
      auto i = symbols_.find(label.name());
      if (i != std::end(symbols_))
        return false;
      symbols_[label.name()] = addr;
      return true;
    }

    case LabelType::Temporary:
    case LabelType::TemporaryForward:
    case LabelType::TemporaryBackward:
      if (temps_.empty() || addr > temps_.back().addr)
        temps_.push_back({ label.type(), addr });
      else
      {
        Temporary entry{ label.type(), addr };
        auto i = std::lower_bound(std::begin(temps_), std::end(temps_), entry, [](const auto& a, const auto& b)
        {
          return a.addr < b.addr;
        });
        if (i->addr != addr)
          temps_.insert(i, entry);
      }
      return true;

    default:
      return true;
  }
}

Maybe<Address> SymbolTable::get(const std::string& name) const noexcept
{
  auto i = symbols_.find(name);
  if (i != std::end(symbols_))
    return i->second;
  return nullptr;
}

Maybe<Address> SymbolTable::get(Address addr, int labelDelta) const noexcept
{
  if (labelDelta == 0)
    return nullptr;

  Temporary entry{ LabelType::Empty, addr };
  auto i = std::lower_bound(std::begin(temps_), std::end(temps_), entry, [](const auto& a, const auto& b)
  {
    return a.addr < b.addr;
  });

  if (labelDelta > 0)
  {
    if (i != std::end(temps_) && i->addr != addr)
      -- i;
    while (labelDelta && i != std::end(temps_))
    {
      ++ i;
      if (i->type == LabelType::Temporary || i->type == LabelType::TemporaryForward)
        -- labelDelta;
    }
    if (i != std::end(temps_))
      return i->addr;
    return nullptr;
  }

  // The delta is negative, so we're going backward here.
  while (labelDelta && i > std::begin(temps_))
  {
    -- i;
    if (i->type == LabelType::Temporary || i->type == LabelType::TemporaryBackward)
      ++ labelDelta;
  }
  if (labelDelta == 0)
    return i->addr;
  return nullptr;
}

}
