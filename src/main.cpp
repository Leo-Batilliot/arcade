#include "Core.hpp"
#include "DLLoader.hpp"
#include <iostream>
#include <stdexcept>

static void printUsage(const char* argv0) {
    std::cerr << "Usage:   " << argv0 << " <path_to_graphical_library.so>\n"
              << "Example: " << argv0 << " ./lib/arcade_sfml.so\n";
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 84;
    }

    try {
        Arcade::DLLoader probe(argv[1]);
        if (!probe.hasSymbol("createDisplay")) {
            if (probe.hasSymbol("createGame"))
                std::cerr << "Error: '" << argv[1] << "' not a graphical library\n";
            else
                std::cerr << "Error: '" << argv[1] << "' is not a valid arcade library\n";
            return 84;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 84;
    }

    try {
        Arcade::Core core(argv[1]);
        core.run();
    } catch (const std::exception& e) {
        std::cerr << "[Fatal] " << e.what() << "\n";
        return 84;
    }

    return 0;
}
