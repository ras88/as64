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
    Parser parser;
    parser.file(argv[1]);
    parser.parse();

    return 0;
  }
  catch (Error& err)
  {
    std::cerr << "[Error] " << err.format() << std::endl;
    return -1;
  }
}
