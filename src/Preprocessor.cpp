#include "../include/Common.h"
#include "../include/ErrorHandler.h"
#include "../include/Tokenizer.h"
#include <filesystem>
#include <fstream>
#define ucharcast(x) static_cast<unsigned char>(x)

namespace casmlang {
namespace fs = std::filesystem;
void Tokenizer::prcs_process() {
    if (current() != '#')
        panic("Cannot preprocess line that does not start with '#'");
    advance();

    std::string command = "";
    while (!eof() && current() != ' ') {
        command += current();
        advance();
    }
    advance(); // skip space

    if (command == "stdlib") {
        prcs_process_include();
    }
}

void Tokenizer::prcs_process_include() {
    fs::path stdlib = CASMLANG_STDLIB_PATH;

    std::string arg;
    while (!eof() && current() != '\n') {
        arg += current();
        advance();
    }

    // remove whitespaces around it
    while (!arg.empty() && std::isspace(ucharcast(arg.front())))
        arg.erase(arg.begin());

    while (!arg.empty() && std::isspace(ucharcast(arg.back())))
        arg.pop_back();

    fs::path linkfile = stdlib / (arg + ".casm");

    if (!fs::exists(linkfile)) {
        panic("Preprocessor: couldn't find file \"" + arg + ".casm\" in standard library at line " +
              std::to_string(row));
    }

    std::ifstream file(linkfile);
    if (!file) {
        panic("Preprocessor: Found file \"" + arg +
              ".casm\" in standard library but couldn't open it; check permissions");
    }

    // read the included file.
    std::string included((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Make sure the included file cannot accidentally merge its last
    // token with the token following the #stdlib directive.
    if (!included.empty() && included.back() != '\n')
        included += '\n';

    /*
     * cursor currently points at the newline after:
     *
     *     #stdlib filename
     *
     * Insert the file immediately before that newline.
     *
     * We intentionally do NOT advance() here. The next iteration of
     * tokenize() will process the first character of the included file.
     */
    code.insert(cursor, included);
}
} // namespace casmlang