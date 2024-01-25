#include "Utils.h"
#include "ArgsParser.h"

namespace utils {
	ArgsParser parseCommandLine(int argc, char* argv[])
	{
		ArgsParser parser;

		if (!parser.parse(argc, argv)) {

		}

		return parser;
	}
}
