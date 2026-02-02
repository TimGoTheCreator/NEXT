#pragma once
#include <string>

namespace next {

struct Arguments {
    std::string input_file;
    double dt;
    double dump_interval;
    bool use_vtu;
};

Arguments parse_arguments(int argc, char** argv);

}
