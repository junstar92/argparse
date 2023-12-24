#include <iostream>
#include <argparse/argparse.h>

int main()
{
    {
    auto parser = argparse::ArgumentParser("02_help_argument");
    parser.add_argument("--foo")
    .set_help("foo help");

    // cmd: ./02_help_argument -h
    std::cout << "cmd > ./02_help_argument -h\n";
    int argc = 2;
    char const* argv[] = {
        "02_help_argument",
        "-h"
    };

    // auto args = parser.parse_args(argc, argv);
    // print help message, and then program exits
    parser.print_help(); // instead of calling parse_args, call print_help() to avoid program termination
    }
    {
    // Occasionally, it may be useful to disable the addition of this help option.
    // This can be achieved by passing false as constructor's argument.
    std::cout << "\n\n ------ after disable help option:\n";
    auto parser = argparse::ArgumentParser("02_help_argument", false);
    parser.add_argument("--foo")
    .set_help("foo help");
    parser.print_help();
    }
    return 0;
}