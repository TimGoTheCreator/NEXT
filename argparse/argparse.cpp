#include "argparse.hpp"
#include <iostream>
#include <stdexcept>

namespace next {

Arguments parse_arguments(int argc, char** argv)
{
    if (argc != 6) {
        std::cerr << "Usage: next <input.txt> <threads> <dt> <dump_interval> <vtk|vtu>\n";
        std::exit(1);
    }

    Arguments args;

    args.input_file = argv[1];

    args.threads = std::stoi(argv[2]);
    if (args.threads < 1) {
        std::cerr << "Thread count must be >= 1\n";
        std::exit(1);
    }

    args.dt = std::stod(argv[3]);
    args.dump_interval = std::stod(argv[4]);

    std::string fmt = argv[5];
    if (fmt == "vtu") args.use_vtu = true;
    else if (fmt == "vtk") args.use_vtu = false;
    else {
        std::cerr << "Format must be 'vtk' or 'vtu'\n";
        std::exit(1);
    }

    return args;
}

}
