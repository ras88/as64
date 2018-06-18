#include <iostream>
#include "buffer.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      CodeBuffer
// ----------------------------------------------------------------------------

void CodeBuffer::writeByte(Offset offset, Byte value) noexcept
{
  if (offset >= data_.size())
    data_.resize(offset + 1);
  data_[offset] = value;
}

void CodeBuffer::writeWord(Offset offset, Word value) noexcept
{
  if (offset + 1 >= data_.size())
    data_.resize(offset + 2);
  data_[offset] = value;
  data_[offset] = value >> 8;
}

void CodeBuffer::fill(Offset offset, ByteLength count, Byte value) noexcept
{
  auto endOffset = offset + count;
  if (endOffset > data_.size())
    data_.resize(endOffset);
  std::fill(&data_[offset], &data_[endOffset], value);
}

// ----------------------------------------------------------------------------
//      CodeWriter
// ----------------------------------------------------------------------------

CodeWriter::CodeWriter(CodeBuffer *buffer) noexcept
  : buffer_(buffer), offset_(0)
{
}

void CodeWriter::attach(CodeBuffer *buffer) noexcept
{
  buffer_ = buffer;
  offset_ = 0;
}

}
