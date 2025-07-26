#include "CompilerError.h"


std::ostream& operator<<(std::ostream& out, const CompilerError& ce)
{
	return out << ce.message;
}

bool operator==(const CompilerError& a, const CompilerError& b)
{
	return a.ec == b.ec && a.message == b.message;
}
