#include "../include/IrPrinter.h"

namespace casmlang {
void IrPrinter::print_indent() {
    for (int i = 0; i < indent; i++) {
        stream << "    ";
    }
}
} // namespace casmlang