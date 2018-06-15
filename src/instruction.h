#ifndef _INCLUDED_CASSM_INSTRUCTION_H
#define _INCLUDED_CASSM_INSTRUCTION_H

#include <string>
#include <array>

namespace cassm
{

class CodeWriter;

// ----------------------------------------------------------------------------
//      IndexRegister
// ----------------------------------------------------------------------------

enum class IndexRegister
{
  None,
  X,
  Y
};

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

AddrMode absoluteMode(IndexRegister index) noexcept;
AddrMode zeroPageMode(IndexRegister index) noexcept;
bool isZeroPage(AddrMode mode) noexcept;

// ----------------------------------------------------------------------------
//      Opcode
// ----------------------------------------------------------------------------

using Opcode = int;

constexpr bool isValid(Opcode opcode) noexcept { return opcode >= 0; }

using OpcodeArray = std::array<Opcode, AddrModeCount>;

// ----------------------------------------------------------------------------
//      Instruction
// ----------------------------------------------------------------------------

class Instruction
{
public:
  Instruction(const std::string& name, OpcodeArray opcodes) noexcept ;

  std::string name() const noexcept { return name_; }
  bool supports(AddrMode mode) const noexcept { return isValid(opcode(mode)); }
  Opcode opcode(AddrMode mode) const noexcept { return opcodes_[static_cast<int>(mode)]; }
  bool isRelative() const noexcept { return isValid(opcode(AddrMode::Relative)); }

  // All of the encoding methods return the number of bytes written, or 0 to indicate that no
  // compatible addressing mode exists for the instruction.
  size_t encodeDirect(CodeWriter& writer, uint16_t addr, IndexRegister index, bool forceAbsolute = false) const noexcept;

private:
  std::string name_;
  OpcodeArray opcodes_;
};

Instruction *instructionNamed(const std::string& name) noexcept;

}
#endif
