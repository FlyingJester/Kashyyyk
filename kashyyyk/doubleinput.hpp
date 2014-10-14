#pragma once

#include <vector>

class Fl_Widget;

namespace Kashyyyk {

struct DoubleInput_Return {
    int value;
    const char *one;
    const char *two;
};

// A simple dialog that has two inputs instead of only one.
// Like a double fl_input!
struct DoubleInput_Return DoubleInput(const char *msg, const char *label1, const char *defstr1, const char *label2, const char *defstr2, std::vector<Fl_Widget *> *extra = nullptr, bool cancellable = true);

}
