#include "cmdline.h"

namespace cassm
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
