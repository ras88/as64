#include <iostream>
#include <fstream>
#include <sstream>
#include "error.h"
#include "buffer.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      CodeBuffer
// ----------------------------------------------------------------------------

CodeBuffer::CodeBuffer()
  : origin_(0)
{
}

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
  data_[offset+1] = value >> 8;
}

void CodeBuffer::fill(Offset offset, ByteLength count, Byte value) noexcept
{
  auto endOffset = offset + count;
  if (endOffset > data_.size())
    data_.resize(endOffset);
  std::fill(&data_[offset], &data_[endOffset], value);
}

void CodeBuffer::write(std::ostream& s, bool withOriginPrefix) const noexcept
{
  if (withOriginPrefix)
  {
    char c1 = origin_, c2 = origin_ >> 8;
    s.write(&c1, 1);
    s.write(&c2, 1);
  }
  s.write(reinterpret_cast<const char *>(&data_[0]), data_.size());
}

void CodeBuffer::save(const std::string& pathPrefix, bool withOriginPrefix) const
{
  std::string filename = filename_;
  if (! pathPrefix.empty())
  {
    if (pathPrefix[pathPrefix.length() - 1] != PATH_SEPARATOR)
      filename = PATH_SEPARATOR + filename;
    filename = pathPrefix + filename;
  }
  std::ofstream s(filename);
  if (! s.is_open())
    throw SystemError(filename);

  write(s, withOriginPrefix);

  s.close();
  if (s.bad())
    throw SystemError(filename);
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

// ----------------------------------------------------------------------------
//      CodeRange
// ----------------------------------------------------------------------------

std::string CodeRange::toString() const noexcept
{
  std::stringstream s;
  s << *this;
  return s.str();
}

std::ostream& operator<<(std::ostream& s, const CodeRange& pos) noexcept
{
  if (pos.buffer_)
  {
    auto filename = pos.buffer_->filename();
    if (! filename.empty())
      s << filename << ':';
    char buf[32];
    sprintf(buf, "%04x-%04x", pos.start_, pos.end_);
    s << buf;
  }
  return s;
}

}
