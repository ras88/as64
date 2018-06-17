#ifndef _INCLUDED_CASSM_BUFFER_H
#define _INCLUDED_CASSM_BUFFER_H

#include <vector>
#include <cassert>
#include "types.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      CodeBuffer
// ----------------------------------------------------------------------------

class CodeBuffer
{
public:
  Address origin() const noexcept { return origin_; }
  void setOrigin(Address pc) noexcept { origin_ = pc; }
  std::string filename() const noexcept { return filename_; }
  void setFilename(const std::string& filename) noexcept { filename_ = filename; }

  void writeByte(Offset offset, Byte value) noexcept;
  void writeWord(Offset offset, Word value) noexcept;
  void fill(ByteLength offset, ByteLength count, Byte value = 0) noexcept;

private:
  Address origin_;
  std::string filename_;
  std::vector<Byte> data_;
};

// ----------------------------------------------------------------------------
//      CodeWriter
// ----------------------------------------------------------------------------

class CodeWriter
{
public:
  CodeWriter() noexcept;
  CodeWriter(CodeBuffer& buffer) noexcept;

  Offset offset() const noexcept { return offset_; }

  void byte(Byte value) noexcept;
  void word(Word value) noexcept;
  void fill(ByteLength count, Byte value = 0) noexcept;

private:
  CodeBuffer *buffer_;
  Offset offset_;
};

inline void CodeWriter::byte(Byte value) noexcept
{
  assert(buffer_ != nullptr);
  buffer_->writeByte(offset_, value);
  ++ offset_;
}

inline void CodeWriter::word(Word value) noexcept
{
  assert(buffer_ != nullptr);
  buffer_->writeWord(offset_, value);
  offset_ += 2;
}

inline void CodeWriter::fill(ByteLength count, Byte value) noexcept
{
  assert(buffer_ != nullptr);
  buffer_->fill(offset_, count, value);
  offset_ += count;
}

}
#endif
