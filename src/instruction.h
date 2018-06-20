#ifndef _INCLUDED_CASSM_INSTRUCTION_H
#define _INCLUDED_CASSM_INSTRUCTION_H

#include <string>
#include <array>
#include <cstdint>

namespace as64
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

std::string toString(IndexRegister index) noexcept;

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
AddrMode indirectMode(IndexRegister index) noexcept;
bool isZeroPage(AddrMode mode) noexcept;

// ----------------------------------------------------------------------------
//      Opcode
// ----------------------------------------------------------------------------

using Opcode = int16_t;

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
  bool isImplied() const noexcept { return isValid(opcode(AddrMode::Implied)); }

  Maybe<ByteLength> encodeImplied(CodeWriter *writer) const noexcept;
  Maybe<ByteLength> encodeAccumulator(CodeWriter *writer) const noexcept;
  Maybe<ByteLength> encodeImmediate(CodeWriter *writer, Byte value) const noexcept;
  Maybe<ByteLength> encodeDirect(CodeWriter *writer, Address addr, IndexRegister index, bool forceAbsolute = false) const noexcept;
  Maybe<ByteLength> encodeIndirect(CodeWriter *writer, Address addr, IndexRegister index) const noexcept;
  Maybe<ByteLength> encodeRelative(CodeWriter *writer, SByte delta) const noexcept;
  Maybe<ByteLength> encodeRelative(CodeWriter *writer, Address from, Address to) const noexcept;

private:
  std::string name_;
  OpcodeArray opcodes_;
};

Instruction *instructionNamed(const std::string& name) noexcept;

}
#endif
