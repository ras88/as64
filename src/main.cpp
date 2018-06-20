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

#include <iostream>
#include "error.h"
#include "parser.h"
#include "define.h"
#include "emit.h"
#include "lister.h"
#include "context.h"
#include "cmdline.h"

using namespace as64;

constexpr const char *g_version = "v1.0.0";

static void usage()
{
  std::cout << "as64 [options] <file> ..." << std::endl;
  std::cout << "  -l                  Write listing to standard output" << std::endl;
  std::cout << "  -o <file>           Specify output filename" << std::endl;
  std::cout << "  -O <path>           Specify output directory" << std::endl;
  std::cout << "  -D <name[=value]>   Add an entry to the symbol table (value defaults to 0)" << std::endl;
  std::cout << "  -s                  Write the symbol table to standard output" << std::endl;
  std::cout << "  -r                  Suppress load location from output file header" << std::endl;
  std::cout << "  -A                  Write AST to standard output and then exit" << std::endl;
  std::cout << "  -h                  Show help text" << std::endl;
  std::cout << "  -v                  Show version number" << std::endl;
  std::cout << std::endl;
}

static std::pair<Label, Address> parseDefinition(const std::string& text)
{
  auto pos = text.find_first_of('=');
  if (pos == std::string::npos)
    return { text, 0 };

  return { text.substr(0, pos), stoi(text.substr(pos + 1), 0) };
}

int main(int argc, char **argv)
{
  bool listingToStdout = false, suppressLoadLocation = false, showHelpText = false, astToStdout = false;
  bool symbolsToStdout = false, showVersion = false;
  std::string outputFilename, outputPath;
  Context context;
  auto inputFilenames = parseCommandLine(argc, argv,
  {
    { 'h',    false,      [&](const auto& value) { showHelpText = true; } },
    { 'v',    false,      [&](const auto& value) { showVersion = true; } },
    { 'l',    false,      [&](const auto& value) { listingToStdout = true; } },
    { 'o',    true,       [&](const auto& value) { outputFilename = value; } },
    { 'O',    true,       [&](const auto& value) { outputPath = value; } },
    { 'r',    false,      [&](const auto& value) { suppressLoadLocation = true; } },
    { 'A',    false,      [&](const auto& value) { astToStdout = true; } },
    { 'D',    true,       [&](const auto& value) { context.symbols.set(parseDefinition(value)); } },
    { 's',    false,      [&](const auto& value) { symbolsToStdout = true; } }
  });

  if (showVersion)
  {
    std::cout << g_version << std::endl;
    return 0;
  }

  if (showHelpText || inputFilenames.empty())
  {
    usage();
    return 0;
  }

  try
  {
    parseFiles(context, inputFilenames);

    if (astToStdout)
    {
      context.statements.dump(std::cout);
      std::cout << std::endl;
      return 0;
    }

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
      if (symbolsToStdout)
        context.symbols.write(std::cout);
    }

    return context.messages.errorCount() ? -1 : 0;
  }
  catch (Error& err)
  {
    std::cerr << "[Error] " << err.format() << std::endl;
    return -1;
  }
}
