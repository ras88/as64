#ifndef _INCLUDED_CASSM_BUFFER_H
#define _INCLUDED_CASSM_BUFFER_H

#include <vector>
#include <cstdint>
#include "types.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      CodeBuffer
// ----------------------------------------------------------------------------

class CodeBuffer
{
public:
  void writeByte(int offset, uint8_t value);
  void writeWord(int offset, uint16_t value);
  void fill(int offset, size_t count, uint8_t value = 0);

private:
  std::vector<uint8_t> data_;
};

// ----------------------------------------------------------------------------
//      CodeWriter
// ----------------------------------------------------------------------------

class CodeWriter
{
public:
  CodeWriter(CodeBuffer& buffer) noexcept;

  int offset() const noexcept { return offset_; }

  Address pc() const noexcept { return pc_; }
  void pc(Address addr) noexcept { pc_ = addr; }

  void byte(uint8_t value) { buffer_.writeByte(offset_, value); ++ offset_; ++ pc_; }
  void word(uint16_t value) { buffer_.writeWord(offset_, value); offset_ += 2; pc_ += 2; }
  void fill(size_t count, uint8_t value = 0) { buffer_.fill(offset_, count, value); offset_ += count; pc_ += count; }

private:
  CodeBuffer& buffer_;
  int offset_;
  Address pc_;
};

}
#endif
