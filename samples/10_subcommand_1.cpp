#include <iostream>
#include <argparse/argparse.h>

int main()
{
    // create the top-level parser
    auto parser = argparse::ArgumentParser("10_subcommand_1");
    parser.add_argument("--foo", argparse::actions::StoreTrueAction())
    .set_help("foo help");
    auto& subparser = parser.add_subparsers()
    .set_help("sub-command help");

    // create the parser for the "a" command
    auto& parser_a = subparser.add_parser("a", {}, "a help");
    parser_a.add_argument("bar")
    .set_help("bar help");

    // create the parser for the "b" command
    auto& parser_b = subparser.add_parser("b", {}, "b help");
    parser_b.add_argument("--baz")
    .set_choices({"X", "Y", "Z"})
    .set_help("baz help");

    {
    // cmd: ./10_subcommand_1 a 12
    std::cout << "cmd > ./10_subcommand_1 a 12\n";
    int argc = 3;
    char const* argv[] = {
        "10_subcommand_1",
        "a", "12"
    };

    auto args = parser.parse_args(argc, argv);
    std::cout << args << "\n\n";
    }

    {
    // cmd: ./10_subcommand_1 --foo b --baz Z
    std::cout << "cmd > ./10_subcommand_1 --foo b --baz Z\n";
    int argc = 5;
    char const* argv[] = {
        "10_subcommand_1",
        "--foo", "b",
        "--baz", "Z"
    };

    auto args = parser.parse_args(argc, argv);
    std::cout << args << "\n\n";
    }

    {
    // cmd: ./10_subcommand_1 a -h
    std::cout << "cmd > ./10_subcommand_1 a -h\n";
    int argc = 3;
    char const* argv[] = {
        "10_subcommand_1",
        "a", "-h"
    };

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
    }

    return 0;
}