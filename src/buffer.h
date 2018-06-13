#ifndef _INCLUDED_CASSM_BUFFER_H
#define _INCLUDED_CASSM_BUFFER_H

#include <vector>
#include <cstdint>

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

private:
  std::vector<uint8_t> data_;
};

// ----------------------------------------------------------------------------
//      CodeWriter
// ----------------------------------------------------------------------------

class CodeWriter
{
public:
  CodeWriter(CodeBuffer& buffer);

  void byte(uint8_t value) { buffer_.writeByte(offset_, value); ++ offset_; }
  void word(uint16_t value) { buffer_.writeWord(offset_, value); offset_ += 2; }

private:
  CodeBuffer& buffer_;
  int offset_;
};

}
#endif
