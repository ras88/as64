#ifndef _INCLUDED_CASSM_BUFFER_H
#define _INCLUDED_CASSM_BUFFER_H

#include <vector>
#include <ostream>
#include <cassert>
#include "types.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      CodeBuffer
// ----------------------------------------------------------------------------

class CodeBuffer
{
public:
  CodeBuffer();

  Address origin() const noexcept { return origin_; }
  void setOrigin(Address pc) noexcept { origin_ = pc; }
  std::string filename() const noexcept { return filename_; }
  void setFilename(const std::string& filename) noexcept { filename_ = filename; }

  bool isEmpty() const noexcept { return data_.empty(); }
  ByteLength size() const noexcept { return data_.size(); }
  Byte operator[](Offset offset) const noexcept { return data_[offset]; }

  void writeByte(Offset offset, Byte value) noexcept;
  void writeWord(Offset offset, Word value) noexcept;
  void fill(ByteLength offset, ByteLength count, Byte value = 0) noexcept;

  void write(std::ostream& c, bool withOriginPrefix = true) const noexcept;
  void save(const std::string& pathPrefix = "", bool withOriginPrefix = true) const;

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
  CodeWriter(CodeBuffer *buffer = nullptr) noexcept;

  Offset offset() const noexcept { return offset_; }
  CodeBuffer *buffer() const noexcept { return buffer_; }
  void attach(CodeBuffer *buffer) noexcept;

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

// ----------------------------------------------------------------------------
//      CodeRange
// ----------------------------------------------------------------------------

class CodeRange
{
public:
  CodeRange(CodeBuffer *buffer = nullptr, Offset start = 0, Offset end = 0) noexcept
    : buffer_(buffer), start_(start), end_(end) { }
  CodeRange(const CodeRange& other) noexcept = default;
  CodeRange& operator=(const CodeRange& other) = default;

  bool isValid() const noexcept { return buffer_; }
  CodeBuffer *buffer() const noexcept { return buffer_; }
  Offset start() const noexcept { return start_; }
  Offset end() const noexcept { return end_; }
  ByteLength length() const noexcept { return end_ - start_; }
  std::string filename() const noexcept { return buffer_ ? buffer_->filename() : ""; }

  Byte operator[](Offset offset) const noexcept { return (*buffer_)[offset + start_]; }

  std::string toString() const noexcept;
  friend std::ostream& operator<<(std::ostream& s, const CodeRange& pos) noexcept;

private:
  CodeBuffer *buffer_;
  Offset start_;
  Offset end_;
};

}
#endif
