#include <iostream>
#include "error.h"
#include "parser.h"
#include "define.h"
#include "emit.h"
#include "lister.h"
#include "context.h"
#include "cmdline.h"

using namespace cassm;

static void usage()
{
  std::cout << "cassm [options] <file> ..." << std::endl;
  std::cout << "  -l              Write listing to standard output" << std::endl;
  std::cout << "  -o <file>       Specify output filename" << std::endl;
  std::cout << "  -O <path>       Specify output directory" << std::endl;
  std::cout << "  -r              Suppress load location from output file header" << std::endl;
  std::cout << "  -h              Show help text" << std::endl;
  std::cout << std::endl;
}

int main(int argc, char **argv)
{
  bool listingToStdout = false, suppressLoadLocation = false, showHelpText = false;
  std::string outputFilename, outputPath;
  auto inputFilenames = parseCommandLine(argc, argv,
  {
    { 'l',    false,      [&](const auto& value) { listingToStdout = true; } },
    { 'o',    true,       [&](const auto& value) { outputFilename = value; } },
    { 'O',    true,       [&](const auto& value) { outputPath = value; } },
    { 'r',    false,      [&](const auto& value) { suppressLoadLocation = true; } },
    { 'h',    false,      [&](const auto& value) { showHelpText = true; } }
  });

  if (showHelpText || inputFilenames.empty())
  {
    usage();
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
        if (buffer->filename().empty())
          buffer->setFilename(outputFilename);
        if (! buffer->filename().empty())
          buffer->save(outputPath, ! suppressLoadLocation);
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
