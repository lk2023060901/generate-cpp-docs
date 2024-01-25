#pragma once

#include <map>

class ArgsParser;

namespace utils {
    ArgsParser parseCommandLine(int argc, char* argv[]);
}