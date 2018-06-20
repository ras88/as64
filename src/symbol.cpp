// Copyright (c) 2018 Robert A. Stoerrle
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
// OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <iostream>
#include <algorithm>
#include <cstdio>
#include "str.h"
#include "symbol.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      SymbolTable
// ----------------------------------------------------------------------------

SymbolTable::SymbolTable()
  : nextSerialNum_(0)
{
}

bool SymbolTable::set(const Label& label, Address addr) noexcept
{
  switch (label.type())
  {
    case LabelType::Symbolic:
    {
      auto i = symbols_.find(label.name());
      if (i != std::end(symbols_))
        return false;
      symbols_[label.name()] = { addr, nextSerialNum_ ++ };
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
    return i->second.address;
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

void SymbolTable::write(std::ostream& s) const noexcept
{
  // Sort the symbols into original declaration order.
  std::vector<std::pair<std::string, Symbol>> entries;
  entries.reserve(symbols_.size());
  size_t longestName = 0;
  for (const auto& symbol: symbols_)
  {
    if (symbol.first.length() > longestName)
      longestName = symbol.first.length();
    entries.push_back({ symbol.first, symbol.second });
  }
  std::sort(std::begin(entries), std::end(entries), [](const auto& a, const auto& b)
  {
    return a.second.serialNum < b.second.serialNum;
  });

  if (longestName % 2)
    ++ longestName;
  longestName += 2;
  for (const auto& entry: entries)
  {
    char addrText[16];
    std::snprintf(addrText, sizeof(addrText), "%04x", entry.second.address);
    s << padRight(entry.first, longestName) << "= $" << addrText << std::endl;
  }
}

}
