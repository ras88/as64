#include <iostream>
#include "error.h"
#include "assembler.h"

using namespace cassm;

int main(int argc, char **argv)
{
  if (argc < 2)
    return -1;

  try
  {
    Assembler assembler;
    assembler.file(argv[1]);
    assembler.assemble();

    return 0;
  }
  catch (Error& err)
  {
    std::cerr << err.message() << std::endl;
    return -1;
  }
}
