#ifndef _INCLUDED_CASSM_PARSER_H
#define _INCLUDED_CASSM_PARSER_H

#include <string>

namespace cassm
{

class Context;

void parseFile(Context& context, const std::string& filename);


}
#endif
