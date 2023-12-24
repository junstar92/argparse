#include <iostream>
#include <argparse/argparse.h>

int main()
{
    auto parser = argparse::ArgumentParser();

    // an optional argument couble be created like:
    parser.add_argument("-f", "--foo");
    // an positional argument could be created like:
    parser.add_argument("bar");

    {
        // cmd: ./04_argument_name BAR
        std::cout << "cmd > ./04_argument_name BAR\n";
        int argc = 2;
        char const* argv[] = {
            "04_argument_name",
            "BAR"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
    }

    {
        // cmd: ./04_argument_name BAR --foo FOO
        std::cout << "cmd > ./04_argument_name BAR --foo FOO\n";
        int argc = 4;
        char const* argv[] = {
            "04_argument_name",
            "BAR",
            "--foo", "FOO"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
    }

    {
        // cmd: ./04_argument_name --foo FOO
        std::cout << "cmd > ./04_argument_name --foo FOO\n";
        int argc = 3;
        char const* argv[] = {
            "04_argument_name",
            "--foo", "FOO"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << std::endl;
    }

    return 0;
}