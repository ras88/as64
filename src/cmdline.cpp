// Copyright (c) 2018 Robert A. Stoerrle
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
// OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "cmdline.h"

namespace as64
{

// ----------------------------------------------------------------------------
//      Command Line Parser
// ----------------------------------------------------------------------------

std::vector<std::string> parseCommandLine(int argc, char **argv, std::initializer_list<Option> options)
{
  std::vector<std::string> args;
  for (int index = 1; index < argc; ++ index)
  { 
    const char *p = argv[index];
    if (*p == '-')
    {
      ++ p;
      bool cont;
      do
      {
        cont = false;
        for (const auto& option: options)
        {
          if (*p == option.name)
          {
            ++ p;
            if (! option.hasArg)
            {
              option.handler("");
              cont = true;
            }
            else
            {
              if (*p)
                option.handler(p);
              else
              {
                ++ index;
                if (index < argc)
                  option.handler(argv[index]);
              }
            }
            break;
          }
        }
      }
      while (cont);
    }
    else
      args.push_back(p);
  }
  return args;
}

}
