#include <iostream>
#include <argparse/argparse.h>

class FooAction
{
public:
    void initialize(argparse::Argument& argument) {
        // do nothing
    }
    argparse::Action get_action() {
        return [](argparse::ArgumentParser* parser, argparse::Namespace& ret, argparse::Argument const& argument, std::vector<std::string> const& values) {
            std::cout << ret << ' ' << argument.get_dest() << " = " << values[0] << std::endl;
            ret[argument.get_dest()] = values;
        };
    }
    argparse::Validation get_valid() {
        return [](argparse::Argument const& argument) {
            if (argument.get_nargs().get_type() != argparse::NArgs::Type::none) {
                throw std::invalid_argument("nargs not allowed");
            }
        };
    }
};

int main()
{
    auto parser = argparse::ArgumentParser();
    parser.add_argument("--foo", FooAction());
    parser.add_argument("bar", FooAction());
    // cmd: ./06_custom_action 1 --foo 2
    std::cout << "cmd > ./06_custom_action 1 --foo 2\n";
    int argc = 4;
    char const* argv[] = {
        "06_custom_action",
        "1",
        "--foo", "2"
    };

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;

    return 0;
}