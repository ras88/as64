#ifndef _INCLUDED_CASSM_INSTRUCTION_H
#define _INCLUDED_CASSM_INSTRUCTION_H

#include <string>
#include <array>

namespace cassm
{

// ----------------------------------------------------------------------------
//      AddrMode
// ----------------------------------------------------------------------------

enum class AddrMode
{
  Accumulator,
  Immediate,
  Implied,
  Relative,
  Absolute,
  AbsoluteX,
  AbsoluteY,
  ZeroPage,
  ZeroPageX,
  ZeroPageY,
  Indirect,
  IndexedIndirect,
  IndirectIndexed,

  _End
};

constexpr size_t AddrModeCount = static_cast<size_t>(AddrMode::_End);

// ----------------------------------------------------------------------------
//      Opcode
// ----------------------------------------------------------------------------

using Opcode = int;

constexpr bool isValid(Opcode opcode) { return opcode >= 0; }

using OpcodeArray = std::array<Opcode, AddrModeCount>;

// ----------------------------------------------------------------------------
//      Instruction
// ----------------------------------------------------------------------------

class Instruction
{
public:
  Instruction(const std::string& name, OpcodeArray opcodes);

  std::string name() const { return name_; }
  Opcode opcode(AddrMode mode) const { return opcodes_[static_cast<int>(mode)]; }

private:
  std::string name_;
  OpcodeArray opcodes_;
};

Instruction *instructionNamed(const std::string& name);

}
#endif
