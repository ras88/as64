#ifndef _INCLUDED_CASSM_ASSEMBLER_H
#define _INCLUDED_CASSM_ASSEMBLER_H

#include <string>
#include "source.h"
#include "buffer.h"
#include "table.h"

namespace cassm
{

class Instruction;

// ----------------------------------------------------------------------------
//      Assembler
// ----------------------------------------------------------------------------

class Assembler
{
public:
  Assembler();

  void file(const std::string& filename);

  void assemble();

private:
  enum class ByteSelector
  {
    Unspecified,
    Low,
    High
  };

  void handleLine(LineReader& reader);
  void handleInstruction(LineReader& reader, Instruction& ins);
  void handleImmediate(LineReader& reader, Instruction& ins);
  void handleDirect(LineReader& reader, Instruction& ins, bool forceAbsolute);
  void handleIndirect(LineReader& reader, Instruction& ins);
  void handleRelative(LineReader& reader, Instruction& ins);
  void handleDirective(LineReader& reader);
  int byteExpression(LineReader& reader, ByteSelector selector);
  int evalExpression(LineReader& reader);
  int evalOperand(LineReader& reader);
  ByteSelector optionalByteSelector(LineReader& reader);
  [[noreturn]] void throwError(const char *format, ...);

  SourceStream input_;
  CodeBuffer code_;
  CodeWriter writer_;
  SymbolTable<uint16_t> symbols_;
};

}
#endif
