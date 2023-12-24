#include <iostream>
#include <argparse/argparse.h>

int main()
{
    /**
     * ArgumentParser adds some value as an attribute of the object retured by parse_args(...).
     * You can get the values from calling get method with a dest (equivalent a key).
     * The name of this attribute(dest or key) is determined by the set_dest method of Argument.
     * 
     * For positional arguments, dest is normally supplied as the first argument name to add_argument(...).
     * For optional arguments, the value of dest is normally inferred from the option strings. ArgumentParser
     * generates the value of dest by taking the first long option(like '--args') and stripping away the initial
     * '--' string. If no long option strings were supplied, dest will be derived from the first short option string
     * by stripping the initial '-' character. Any internal '-' characters will be converted to '_' characters
     * to make sure the string is a valid attribute name.
     */
    {
    auto parser = argparse::ArgumentParser("09_dest", '-', false);
    parser.add_argument("-f", "--foo-bar", "--foo"); // generates 'foo_bar' as dest value
    parser.add_argument("-x", "-y"); // generates 'x' as dest value

    // cmd: ./09_dest -f 1 -x 2
    std::cout << "cmd > ./09_dest -f 1 -x 2\n";
    int argc = 5;
    char const* argv[] = {
        "09_dest",
        "-f", "1",
        "-x", "2"
    };

    auto args = parser.parse_args(argc, argv);
    std::cout << "-f: " << args.get("foo_bar") << std::endl;
    std::cout << "-x: " << args.get("x") << std::endl;
    }

    {
    auto parser = argparse::ArgumentParser("09_dest", '-', false);
    parser.add_argument("--foo")
    .set_dest("bar"); // generates 'bar' as dest value

    // cmd: ./09_dest --foo
    std::cout << "cmd > ./09_dest --foo XXX\n";
    int argc = 3;
    char const* argv[] = {
        "09_dest",
        "--foo", "XXX"
    };

    auto args = parser.parse_args(argc, argv);
    std::cout << "--foo: " << args.get("bar") << std::endl;
    }

    return 0;
}