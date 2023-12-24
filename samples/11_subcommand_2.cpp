#include <iostream>
#include <argparse/argparse.h>

int main()
{
    // create the top-level parser
    auto parser = argparse::ArgumentParser("11_subcommand_2");

    auto& subparser = parser.add_subparsers("subcommands", "valid subcommands")
    .set_help("additional help");
    subparser.add_parser("foo");
    subparser.add_parser("bar");

    // cmd: ./11_subcommand_2 -h
    std::cout << "cmd > ./11_subcommand_2 -h\n";
    int argc = 2;
    char const* argv[] = {
        "11_subcommand_2",
        "-h"
    };

    auto args = parser.parse_args(argc, argv);

    return 0;
}