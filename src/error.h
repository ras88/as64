#ifndef _INCLUDED_AS64_ERROR_H
#define _INCLUDED_AS64_ERROR_H

#include <exception>
#include <string>

namespace as64
{

// ----------------------------------------------------------------------------
//      Error
// ----------------------------------------------------------------------------

class Error : public std::exception
{
public:
  virtual std::string message() const noexcept { return what(); }
  virtual std::string format() const noexcept { return message(); }
};

// ----------------------------------------------------------------------------
//      GeneralError
// ----------------------------------------------------------------------------

class GeneralError : public Error
{
};

// ----------------------------------------------------------------------------
//      SystemError
// ----------------------------------------------------------------------------

class SystemError : public GeneralError
{
public:
  SystemError(const std::string& path, int code = errno) noexcept
    : path_(path), code_(code) { }
  SystemError(int code = errno) noexcept
    : code_(code) { }
  SystemError(const SystemError& other) = default;
  virtual ~SystemError() noexcept = default;
  SystemError& operator=(const SystemError& other) = default;

  const char *what() const noexcept override { return "System Error"; }
  std::string message() const noexcept override;
  std::string format() const noexcept override;

  std::string path() const noexcept { return path_; }
  int code() const noexcept { return code_; }

private:
  std::string path_;
  int code_;
};

}
#endif
