#ifndef _INCLUDED_CASSM_ASSEMBLER_H
#define _INCLUDED_CASSM_ASSEMBLER_H

#include <string>
#include "source.h"

namespace cassm
{

class Instruction;

// ----------------------------------------------------------------------------
//      Assembler
// ----------------------------------------------------------------------------

class Assembler
{
public:
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
  int byteExpression(LineReader& reader, ByteSelector selector);
  int evalExpression(LineReader& reader);
  int evalOperand(LineReader& reader);
  ByteSelector optionalByteSelector(LineReader& reader);

  SourceStream input_;
};

}
#endif
