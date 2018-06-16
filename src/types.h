#ifndef _INCLUDED_CASSM_TYPES_H
#define _INCLUDED_CASSM_TYPES_H

#include <string>
#include <cstdint>

namespace cassm
{

using Address = uint16_t;

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

// ----------------------------------------------------------------------------
//      BranchDirection
// ----------------------------------------------------------------------------

enum class BranchDirection
{
  Forward,
  Backward
};

std::string toString(BranchDirection direction) noexcept;

// ----------------------------------------------------------------------------
//      StringEncoding
// ----------------------------------------------------------------------------

enum class StringEncoding
{
  Petscii,
  Screen
};

std::string toString(StringEncoding encoding) noexcept;

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
  operator bool() const noexcept { return has_; }

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

}
#endif
