#include <iostream>
#include <argparse/argparse.h>

int main()
{
    // Normally, when you pass an invalid argument list to the parse_args() method of an ArgumentParser,
    // it will exit with error info.
    // If the user would like to catch errors manually,
    // the feature can be enabled by setting exit_on_error(the third argument) to false
    auto parser = argparse::ArgumentParser("03_exit_on_error", true, '-', false);
    parser.add_argument("--foo");

    // cmd: ./03_exit_on_error --foo
    std::cout << "cmd > ./03_exit_on_error --foo\n";
    int argc = 2;
    char const* argv[] = {
        "03_exit_on_error",
        "--foo"
    };

    try {
        auto args = parser.parse_args(argc, argv);
    }
    catch (std::exception& e) {
        std::cout << "catch exception\n";
        std::cout << e.what() << std::endl;
    }

    return 0;
}