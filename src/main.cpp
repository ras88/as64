#include <iostream>
#include "source.h"

using namespace cassm;

int main(int argc, char **argv)
{
  if (argc < 2)
    return -1;

  auto source = load(argv[1]);

  for (const auto& line: source)
  {
    std::cout << line.index() << ": " << line.text() << std::endl;

    LineReader reader(line);
    Token token;
    do
    {
      token = reader.nextToken();
      std::cout << "  " << token << std::endl;
    }
    while(token.type != TokenType::End);
  }

  return 0;
}
