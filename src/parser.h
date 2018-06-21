#ifndef _INCLUDED_AS64_PARSER_H
#define _INCLUDED_AS64_PARSER_H

#include <string>
#include <vector>

namespace as64
{

class Context;

void parseFile(Context& context, const std::string& filename);
void parseFiles(Context& context, const std::vector<std::string>& filenames);

}
#endif
