#include <iostream>
#include "error.h"
#include "parser.h"
#include "define.h"
#include "emit.h"
#include "lister.h"
#include "context.h"

using namespace cassm;

static void showHelp()
{
  std::cout << "cassm [options] <file> ..." << std::endl;
  std::cout << "  -l              Write listing to standard output" << std::endl;
  std::cout << "  -o <file>       Specify output filename" << std::endl;
  std::cout << std::endl;
}

int main(int argc, char **argv)
{
  std::vector<std::string> inputFilenames;
  bool listingToStdout = false;
  std::string listingFilename, outputFilename;
  for (int index = 1; index < argc; ++ index)
  {
    const char *p = argv[index];
    if (*p == '-')
    {
      ++ p;
      switch (*p)
      {
        case 'l':
          listingToStdout = true;
          break;

        case 'o':
          // TODO
          break;
      }
    }
    else
      inputFilenames.push_back(p);
  }

  if (inputFilenames.empty())
  {
    showHelp();
    return 0;
  }

  try
  {
    Context context;
    parseFiles(context, inputFilenames);

    define(context);
    if (! context.messages.hasFatalError())
      emit(context);

    if (context.messages.count())
      std::cerr << context.messages << std::endl;

    if (context.messages.errorCount() == 0)
    {
      for (const auto& buffer: context.buffers)
      {
        if (! buffer->filename().empty())
          buffer->save();
      }
      if (listingToStdout)
        list(std::cout, context);
    }

    return context.messages.errorCount() ? -1 : 0;
  }
  catch (Error& err)
  {
    std::cerr << "[Error] " << err.format() << std::endl;
    return -1;
  }
}
