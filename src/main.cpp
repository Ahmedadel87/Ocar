#include "../include/sysCall.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    std::string read_from = "../code.scl";
    std::string write_to = "../output.sh";

    if (argc == 2) {
        read_from = argv[1];
    } else if (argc > 2) {
        read_from = argv[1];
        write_to = argv[2];
    }

    std::ifstream fileStream(read_from);
    if (!fileStream.is_open()) {
        std::string message = "couldn't open file:";
        message.append(argv[1]);
        throw std::runtime_error(message);
    }
    std::ostringstream sstr;
    sstr << fileStream.rdbuf();

    std::ofstream output(write_to);
    if (!output.is_open()) {
        throw std::runtime_error("couldn't open output file lolze");
    }

    Compiler compiler;
    compiler.compile(sstr.str(), output);

    output.close();

    std::string cmd = "bash " + write_to;
    std::cout << "==OUTPUT==\n";
    int result = system(cmd.data());
    return result;
}
