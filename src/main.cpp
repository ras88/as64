#include <iostream>
#include "error.h"
#include "parser.h"

using namespace cassm;

int main(int argc, char **argv)
{
  if (argc < 2)
    return -1;

  try
  {
    Context context;
    parseFile(context, argv[1]);

    context.statements().dump(std::cout);
    std::cout << std::endl;

    if (context.messages().count())
      std::cerr << context.messages() << std::endl;

    return 0;
  }
  catch (Error& err)
  {
    std::cerr << "[Error] " << err.format() << std::endl;
    return -1;
  }
}
