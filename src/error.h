#ifndef _INCLUDED_CASSM_ERROR_H
#define _INCLUDED_CASSM_ERROR_H

#include <exception>
#include <string>

namespace cassm
{

// ----------------------------------------------------------------------------
//      Error
// ----------------------------------------------------------------------------

class Error : public std::exception
{
public:
  virtual std::string message() const noexcept { return what(); } 
  virtual std::string path() const noexcept { return ""; }
  virtual int number() const noexcept { return 0; }
};

// ----------------------------------------------------------------------------
//      SystemError
// ----------------------------------------------------------------------------

class SystemError : public Error
{
public:
  SystemError(const std::string& path, int number = errno) noexcept
    : path_(path), number_(number) { }
  SystemError(int number = errno) noexcept
    : number_(number) { }
  SystemError(const SystemError& other) = default;
  virtual ~SystemError() noexcept = default;
  SystemError& operator=(const SystemError& other) = default;

  std::string path() const noexcept override { return path_; }
  int number() const noexcept override { return number_; }
  const char *what() const noexcept override { return "System Error"; }
  std::string message() const noexcept override;

private:
  std::string path_;
  int number_;
};

}
#endif
