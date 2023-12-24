#include <iostream>
#include <argparse/argparse.h>

int main()
{
    auto parser = argparse::ArgumentParser();
    parser.set_abbrev(true); // true is default value
    parser.add_argument("--foobar", argparse::actions::StoreTrueAction());
    parser.add_argument("--foonley", argparse::actions::StoreFalseAction());

    // cmd: ./01_allow_abbrev --foon
    std::cout << "cmd > ./01_allow_abbrev --foon\n";
    int argc = 2;
    char const* argv[] = {
        "01_allow_abbrev",
        "--foon"
    };

    auto args = parser.parse_args(argc, argv);

    std::cout << args << std::endl;

    return 0;
}