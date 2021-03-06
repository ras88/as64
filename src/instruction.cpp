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
#include "str.h"
#include "enum.h"
#include "table.h"
#include "buffer.h"
#include "table.h"
#include "instruction.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      IndexRegister
// ----------------------------------------------------------------------------

static EnumTags<IndexRegister> g_indexRegisterTags =
{
  { IndexRegister::None,          "None" },
  { IndexRegister::X,             "X" },
  { IndexRegister::Y,             "Y" }
};

std::string toString(IndexRegister index) noexcept
{
  return g_indexRegisterTags.fromValue(index);
}

// ----------------------------------------------------------------------------
//      AddrMode
// ----------------------------------------------------------------------------

AddrMode absoluteMode(IndexRegister index) noexcept
{
  switch (index)
  {
    case IndexRegister::None:
      return AddrMode::Absolute;

    case IndexRegister::X:
      return AddrMode::AbsoluteX;

    case IndexRegister::Y:
      return AddrMode::AbsoluteY;

    default:
      std::terminate();
  }
}

AddrMode zeroPageMode(IndexRegister index) noexcept
{
  switch (index)
  {
    case IndexRegister::None:
      return AddrMode::ZeroPage;

    case IndexRegister::X:
      return AddrMode::ZeroPageX;

    case IndexRegister::Y:
      return AddrMode::ZeroPageY;

    default:
      std::terminate();
  }
}

AddrMode indirectMode(IndexRegister index) noexcept
{
  switch (index)
  {
    case IndexRegister::None:
      return AddrMode::Indirect;

    case IndexRegister::X:
      return AddrMode::IndexedIndirect;

    case IndexRegister::Y:
      return AddrMode::IndirectIndexed;

    default:
      std::terminate();
  }
}

bool isZeroPage(AddrMode mode) noexcept
{
  return mode == AddrMode::ZeroPage || mode == AddrMode::ZeroPageX || mode == AddrMode::ZeroPageY ||
         mode == AddrMode::IndexedIndirect || mode == AddrMode::IndirectIndexed;
}

// ----------------------------------------------------------------------------
//      Instruction Table
// ----------------------------------------------------------------------------

struct InstructionDef
{
  const char *name;
  OpcodeArray opcodes;
};

constexpr Opcode ____ = -1;

static InstructionDef g_table[] =
{
  // Opcode   Accum   Immed   Imply   Rel     Abs     AbsX    AbsY    zp      zp,x    zp,y    Indir   (a, x)  (a),y
  { "adc",    ____,   0x69,   ____,   ____,   0x6d,   0x7d,   0x79,   0x65,   0x75,   ____,   ____,   0x61,   0x71  },
  { "and",    ____,   0x29,   ____,   ____,   0x2d,   0x3d,   0x39,   0x25,   0x35,   ____,   ____,   0x21,   0x31  },
  { "asl",    0x0a,   ____,   ____,   ____,   0x0e,   0x1e,   ____,   0x06,   0x16,   ____,   ____,   ____,   ____  },
  { "bcc",    ____,   ____,   ____,   0x90,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "bcs",    ____,   ____,   ____,   0xb0,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "beq",    ____,   ____,   ____,   0xf0,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "bit",    ____,   ____,   ____,   ____,   0x2c,   ____,   ____,   0x24,   ____,   ____,   ____,   ____,   ____  },
  { "bmi",    ____,   ____,   ____,   0x30,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "bne",    ____,   ____,   ____,   0xd0,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "bpl",    ____,   ____,   ____,   0x10,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "brk",    ____,   ____,   0x00,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "bvc",    ____,   ____,   ____,   0x50,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "bvs",    ____,   ____,   ____,   0x70,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "clc",    ____,   ____,   0x18,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "cld",    ____,   ____,   0xd8,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "cli",    ____,   ____,   0x58,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "clv",    ____,   ____,   0xb8,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "cmp",    ____,   0xc9,   ____,   ____,   0xcd,   0xdd,   0xd9,   0xc5,   0xd5,   ____,   ____,   0xc1,   0xd1  },
  { "cpx",    ____,   0xe0,   ____,   ____,   0xec,   ____,   ____,   0xe4,   ____,   ____,   ____,   ____,   ____  },
  { "cpy",    ____,   0xc0,   ____,   ____,   0xcc,   ____,   ____,   0xc4,   ____,   ____,   ____,   ____,   ____  },
  { "dec",    ____,   ____,   ____,   ____,   0xce,   0xde,   ____,   0xc6,   0xd6,   ____,   ____,   ____,   ____  },
  { "dex",    ____,   ____,   0xca,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "dey",    ____,   ____,   0x88,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "eor",    ____,   0x49,   ____,   ____,   0x4d,   0x5d,   0x59,   0x45,   0x55,   ____,   ____,   0x41,   0x51  },
  { "inc",    ____,   ____,   ____,   ____,   0xee,   0xfe,   ____,   0xe6,   0xf6,   ____,   ____,   ____,   ____  },
  { "inx",    ____,   ____,   0xe8,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "iny",    ____,   ____,   0xc8,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "jmp",    ____,   ____,   ____,   ____,   0x4c,   ____,   ____,   ____,   ____,   ____,   0x6c,   ____,   ____  },
  { "jsr",    ____,   ____,   ____,   ____,   0x20,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "lda",    ____,   0xa9,   ____,   ____,   0xad,   0xbd,   0xb9,   0xa5,   0xb5,   ____,   ____,   0xa1,   0xb1  },
  { "ldx",    ____,   0xa2,   ____,   ____,   0xae,   ____,   0xbe,   0xa6,   ____,   0xb6,   ____,   ____,   ____  },
  { "ldy",    ____,   0xa0,   ____,   ____,   0xac,   0xbc,   ____,   0xa4,   0xb4,   ____,   ____,   ____,   ____  },
  { "lsr",    0x4a,   ____,   ____,   ____,   0x4e,   0x53,   ____,   0x46,   0x56,   ____,   ____,   ____,   ____  },
  { "nop",    ____,   ____,   0xea,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "ora",    ____,   0x09,   ____,   ____,   0x0d,   0x1d,   0x19,   0x05,   0x15,   ____,   ____,   0x01,   0x11  },
  { "pha",    ____,   ____,   0x48,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "php",    ____,   ____,   0x08,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "pla",    ____,   ____,   0x68,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "plp",    ____,   ____,   0x28,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "rol",    0x2a,   ____,   ____,   ____,   0x2e,   0x3e,   ____,   0x26,   0x36,   ____,   ____,   ____,   ____  },
  { "ror",    0x6a,   ____,   ____,   ____,   0x6e,   0x7e,   ____,   0x66,   0x76,   ____,   ____,   ____,   ____  },
  { "rti",    ____,   ____,   0x40,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "rts",    ____,   ____,   0x60,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "sbc",    ____,   0xe9,   ____,   ____,   0xed,   0xfd,   0xf9,   0xe5,   0xf5,   ____,   ____,   0xe1,   0xf1  },
  { "sec",    ____,   ____,   0x38,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "sed",    ____,   ____,   0xf8,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "sei",    ____,   ____,   0x78,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "sta",    ____,   ____,   ____,   ____,   0x8d,   0x9d,   0x99,   0x85,   0x95,   ____,   ____,   0x81,   0x91  },
  { "stx",    ____,   ____,   ____,   ____,   0x8e,   ____,   ____,   0x86,   ____,   0x96,   ____,   ____,   ____  },
  { "sty",    ____,   ____,   ____,   ____,   0x8c,   ____,   ____,   0x84,   0x94,   ____,   ____,   ____,   ____  },
  { "tax",    ____,   ____,   0xaa,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "tay",    ____,   ____,   0xa8,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "tsx",    ____,   ____,   0xba,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "txa",    ____,   ____,   0x8a,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "txs",    ____,   ____,   0x9a,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  },
  { "tya",    ____,   ____,   0x98,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____,   ____  }
};

static Table<Instruction>& instructions() noexcept
{
  static Table<Instruction> instance([](auto& table)
  {
    for (const auto& def: g_table)
      table.emplace(def.name, def.name, def.opcodes);
  });

  return instance;
}

// ----------------------------------------------------------------------------
//      Instruction
// ----------------------------------------------------------------------------

Instruction::Instruction(const std::string& name, OpcodeArray opcodes) noexcept
  : name_(name), opcodes_(opcodes)
{
}

Maybe<ByteLength> Instruction::encodeImplied(CodeWriter *writer) const noexcept
{
  auto op = opcode(AddrMode::Implied);
  if (! isValid(op))
    return nullptr;
  if (writer)
    writer->byte(op);
  return 1;
}

Maybe<ByteLength> Instruction::encodeAccumulator(CodeWriter *writer) const noexcept
{
  auto op = opcode(AddrMode::Accumulator);
  if (! isValid(op))
    return nullptr;
  if (writer)
    writer->byte(op);
  return 1;
}

Maybe<ByteLength> Instruction::encodeImmediate(CodeWriter *writer, Byte value) const noexcept
{
  auto op = opcode(AddrMode::Immediate);
  if (! isValid(op))
    return nullptr;
  if (writer)
  {
    writer->byte(op);
    writer->byte(value);
  }
  return 2;
}

Maybe<ByteLength> Instruction::encodeDirect(CodeWriter *writer, Address addr,
                                            IndexRegister index, bool forceAbsolute) const noexcept
{
  if (addr < 0x100 && ! forceAbsolute)
  {
    auto op = opcode(zeroPageMode(index));
    if (isValid(op))
    {
      if (writer)
      {
        writer->byte(op);
        writer->byte(addr);
      }
      return 2;
    }
  }
  auto op = opcode(absoluteMode(index));
  if (isValid(op))
  {
    if (writer)
    {
      writer->byte(op);
      writer->word(addr);
    }
    return 3;
  }

  return nullptr;
}

Maybe<ByteLength> Instruction::encodeIndirect(CodeWriter *writer, Address addr, IndexRegister index) const noexcept
{
  auto mode = indirectMode(index);
  auto op = opcode(mode);
  if (! isValid(op))
    return nullptr;

  if (mode == AddrMode::Indirect)
  {
    if (writer)
    {
      writer->byte(op);
      writer->word(addr);
    }
    return 3;
  }

  if (addr > 0xff)
    return nullptr;
  if (writer)
  {
    writer->byte(op);
    writer->byte(addr);
  }
  return 2;
}

Maybe<ByteLength> Instruction::encodeRelative(CodeWriter *writer, SByte delta) const noexcept
{
  auto op = opcode(AddrMode::Relative);
  if (! isValid(op))
    return nullptr;

  if (writer)
  {
    writer->byte(op);
    writer->byte(delta);
  }

  return 2;
}

Maybe<ByteLength> Instruction::encodeRelative(CodeWriter *writer, Address from, Address to) const noexcept
{
  auto delta = static_cast<int>(to) - (static_cast<int>(from) + 2);
  if (delta > 127 || delta < -128)
    return nullptr;

  return encodeRelative(writer, delta);
}

Instruction *instructionNamed(const std::string& name) noexcept
{
  return instructions().get(toLowerCase(name));
}

}
