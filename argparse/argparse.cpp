#include "argparse.hpp"
#include <iostream>
#include <stdexcept>

namespace next {

Arguments parse_arguments(int argc, char** argv)
{
    if (argc != 6) {
        std::cerr << "Usage: next <initial.txt> <dt> <dump_interval> <vtk|vtu>\n";
        std::exit(1);
    }

    Arguments args;
    args.input_file = argv[2];
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
