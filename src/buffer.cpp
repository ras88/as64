#include <iostream>
#include "buffer.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      CodeBuffer
// ----------------------------------------------------------------------------

void CodeBuffer::writeByte(int offset, uint8_t value)
{
  if (offset >= data_.size())
    data_.resize(offset + 1);
  data_[offset] = value;
}

void CodeBuffer::writeWord(int offset, uint16_t value)
{
  if (offset + 1 >= data_.size())
    data_.resize(offset + 2);
  data_[offset] = value;
  data_[offset] = value >> 8;
}

// ----------------------------------------------------------------------------
//      CodeWriter
// ----------------------------------------------------------------------------

CodeWriter::CodeWriter(CodeBuffer& buffer)
  : buffer_(buffer), offset_(0)
{
}

}
