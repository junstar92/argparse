#include <iostream>
#include <argparse/argparse.h>

int main()
{
    /**
     * Some command-line arguments should be selected from a restricted set of values.
     * These can be handled by passing a sequence of strings as argument to set_choices() method.
     * When the command line is parsed, argument values will be checked, and an error message will be
     * displayed if the argument was not one of the acceptable values.
     */
    auto parser = argparse::ArgumentParser();
    parser.add_argument("move")
    .set_choices({"rock", "paper", "scissors"});

    {
    // cmd: ./08_choices rock
    std::cout << "cmd > ./08_choices rock\n";
    int argc = 2;
    char const* argv[] = {
        "08_choices",
        "rock"
    };

    auto args = parser.parse_args(argc, argv);
    std::cout << args << "\n\n";
    }

    {
    // cmd: ./08_choices fire
    std::cout << "cmd > ./08_choices fire\n";
    int argc = 2;
    char const* argv[] = {
        "08_choices",
        "fire" // it generates an error message
    };

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
    }

    return 0;
}