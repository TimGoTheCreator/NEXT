#include "argparse.hpp"
#include <iostream>
#include <stdexcept>

namespace next {

Arguments parse_arguments(int argc, char** argv)
{
    if (argc != 6) {
        std::cerr << "Usage: next <input.txt> <threads> <dt> <dump_interval> <vtk|vtu|hdf5>\n";
        std::exit(1);
    }

    Arguments args;

    args.input_file = argv[1];
    args.threads = std::stoi(argv[2]);
    args.dt = std::stod(argv[3]);
    args.dump_interval = std::stod(argv[4]);

    std::string fmt = argv[5];

    if (fmt == "vtk")
        args.format = OutputFormat::VTK;
    else if (fmt == "vtu")
        args.format = OutputFormat::VTU;
    else if (fmt == "hdf5")
        args.format = OutputFormat::HDF5;
    else {
        std::cerr << "Choose a file format: vtk, vtu, or hdf5\n";
        std::exit(1);
    }

    return args;
}

} // namespace next
