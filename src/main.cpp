#include <iostream>
#include "error.h"
#include "parser.h"
#include "define.h"
#include "emit.h"
#include "context.h"

using namespace cassm;

int main(int argc, char **argv)
{
  if (argc < 2)
    return -1;

  try
  {
    Context context;
    parseFile(context, argv[1]);

//    context.statements.dump(std::cout);
//    std::cout << std::endl;

    define(context);
    emit(context);

    if (context.messages.count())
      std::cerr << context.messages << std::endl;

    // TODO: don't write any buffers if any errors occurred
    for (const auto& buffer: context.buffers)
      buffer->save();

    return 0;
  }
  catch (Error& err)
  {
    std::cerr << "[Error] " << err.format() << std::endl;
    return -1;
  }
}
