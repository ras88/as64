#ifndef _INCLUDED_AS64_CMDLINE_H
#define _INCLUDED_AS64_CMDLINE_H

#include <string>
#include <vector>
#include <functional>
#include <initializer_list>

namespace as64
{

// ----------------------------------------------------------------------------
//      Command Line Parser
// ----------------------------------------------------------------------------

using OptionHandler = std::function<void (const std::string& value)>;

struct Option
{
  char name;
  bool hasArg;
  OptionHandler handler;
};

std::vector<std::string> parseCommandLine(int argc, char **argv, std::initializer_list<Option> options);

}
#endif
