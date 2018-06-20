#ifndef _INCLUDED_CASSM_TYPES_H
#define _INCLUDED_CASSM_TYPES_H

#include <string>
#include <ostream>
#include <cstdint>

namespace as64
{

using Address = uint16_t;
using ProgramCounter = uint32_t;
using ByteLength = uint16_t;
using Byte = uint8_t;
using SByte = int8_t;
using Word = uint16_t;
using Offset = uint16_t;

#ifdef __GNUC__
#define CHECK_FORMAT(formatIndex, argIndex) __attribute__ (( format(printf, formatIndex, argIndex) ))
#else
#define CHECK_FORMAT(formatIndex, argIndex)
#endif

// ----------------------------------------------------------------------------
//      Maybe
// ----------------------------------------------------------------------------

template<typename T> class Maybe
{
public:
  Maybe() noexcept : value_(T()), has_(false) { }
  Maybe(T value) noexcept : value_(value), has_(true) { }
  Maybe(std::nullptr_t) noexcept : value_(T()), has_(false) { }
  Maybe(const Maybe&) noexcept = default;
  Maybe(Maybe&&) noexcept = default;
  Maybe& operator=(Maybe&&) noexcept = default;
  Maybe& operator=(const Maybe&) noexcept = default;
  Maybe& operator=(std::nullptr_t) noexcept { reset(); }
  Maybe& operator=(T value) noexcept;
  void reset() noexcept;

  T operator *() const noexcept { return value_; }
  T operator ->() const noexcept { return value_; }
  T value() const noexcept { return value_; }
  T value(T defaultValue) const noexcept { return has_ ? value_ : defaultValue; }
  bool hasValue() const noexcept { return has_; }

private:
  T value_;
  bool has_ : 1;
};

template<typename T> void Maybe<T>::reset() noexcept
{
  value_ = T();
  has_ = false;
}

template<typename T> Maybe<T>& Maybe<T>::operator=(T value) noexcept
{
  value_ = value_;
  has_ = true;
  return *this;
}

// ----------------------------------------------------------------------------
//      ByteSelector
// ----------------------------------------------------------------------------

enum class ByteSelector
{
  Unspecified,
  Low,
  High
};

std::string toString(ByteSelector selector) noexcept;
Maybe<Byte> select(ByteSelector selector, Word value) noexcept;

// ----------------------------------------------------------------------------
//      LabelType
// ----------------------------------------------------------------------------

enum class LabelType
{
  Empty,
  Symbolic,
  Temporary,
  TemporaryForward,
  TemporaryBackward
};

// ----------------------------------------------------------------------------
//      Label
// ----------------------------------------------------------------------------

class Label
{
public:
  Label(LabelType type = LabelType::Empty) noexcept : type_(type) { }
  Label(const std::string& name) noexcept : type_(LabelType::Symbolic), name_(name) { }

  LabelType type() const noexcept { return type_; }
  bool isEmpty() const noexcept { return type_ == LabelType::Empty; }
  bool isSymbolic() const noexcept { return type_ == LabelType::Symbolic; }
  bool isTemporary() const noexcept;

  const std::string& name() const noexcept { return name_; }

  friend std::ostream& operator<<(std::ostream& s, const Label& label);

private:
  LabelType type_;
  std::string name_;
};

inline bool Label::isTemporary() const noexcept
{
  return type_ == LabelType::Temporary || type_ == LabelType::TemporaryForward || type_ == LabelType::TemporaryBackward;
}

}
#endif
