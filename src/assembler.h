#ifndef _INCLUDED_CASSM_ASSEMBLER_H
#define _INCLUDED_CASSM_ASSEMBLER_H

#include <string>
#include "source.h"
#include "buffer.h"
#include "instruction.h"
#include "table.h"

namespace cassm
{

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
  void handleOrg(LineReader& reader);
  void handleBuf(LineReader& reader);
  int byteExpression(LineReader& reader, ByteSelector selector);
  int evalExpression(LineReader& reader);
  ByteSelector optionalByteSelector(LineReader& reader);
  IndexRegister optionalIndex(LineReader& reader);

  SourceStream input_;
  CodeBuffer code_;
  CodeWriter writer_;
  SymbolTable<uint16_t> symbols_;

  using DirectiveHandler = void (Assembler::*)(LineReader& reader);
  static SymbolTable<DirectiveHandler> directives_;
};

}
#endif
