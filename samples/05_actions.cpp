#include <iostream>
#include <argparse/argparse.h>

int main()
{
    {
        /**
         * - StoreAction
         * This just stores the argument's value (default action).
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--foo");
        // cmd: ./05_actions --foo 1
        std::cout << "cmd > ./05_actions --foo 1\n";
        int argc = 3;
        char const* argv[] = {
            "05_actions",
            "--foo", "1"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
    }
    {
        /**
         * - StoreConstAction
         * This stores the specified value by set_const_value method.
         * Default const value is empty. The `StoreConstAction` action is most commonly 
         * used with optional arguments that specify some sort of flag.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--foo", argparse::actions::StoreConstAction())
        .set_const(42);
        // cmd: ./05_actions --foo
        std::cout << "cmd > ./05_actions --foo\n";
        int argc = 2;
        char const* argv[] = {
            "05_actions",
            "--foo"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
    }
    {
        /**
         * - StoreTrueAction and StoreFalseAction
         * These are special cases of `StoreConstAction` used for storing the values true and false respectively.
         * In addition, they create default values of false and true respectively.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--foo", argparse::actions::StoreTrueAction());
        parser.add_argument("--bar", argparse::actions::StoreFalseAction());
        parser.add_argument("--baz", argparse::actions::StoreFalseAction());
        // cmd: ./05_actions --foo --bar
        std::cout << "cmd > ./05_actions --foo --bar\n";
        int argc = 3;
        char const* argv[] = {
            "05_actions",
            "--foo",
            "--bar"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
    }
    {
        /**
         * - AppendAction
         * This stores a list, and appends each argument value to the list.
         * It is useful to allow an option to be specified multiple times.
         * If the default value is non-empty, the default elements will be present in the parsed value for the option,
         * with any values from the command line appended after those default values.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--foo", argparse::actions::AppendAction());
        // cmd: ./05_actions --foo 1 --foo 2
        std::cout << "cmd > ./05_actions --foo 1 --foo 2\n";
        int argc = 5;
        char const* argv[] = {
            "05_actions",
            "--foo", "1",
            "--foo", "2"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
        std::cout << "foo values: ";
        auto foo = args.get<std::vector<int>>("foo");
        for (auto& val : foo) {
            std::cout << val << ' ';
        }
        std::cout << '\n';
    }
    {
        /**
         * - AppendConstAction
         * This stores a list, and appends the value specified by the const_value method to the list.
         * `AppendConstAction` is typically useful when multiple arguments need to store constants to the same list.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--str", argparse::actions::AppendConstAction())
        .set_dest("types")
        .set_const("str");
        parser.add_argument("--int", argparse::actions::AppendConstAction())
        .set_dest("types")
        .set_const("int");
        // cmd: ./05_actions --str --int
        std::cout << "cmd > ./05_actions --str --int\n";
        int argc = 3;
        char const* argv[] = {
            "05_actions",
            "--str",
            "--int"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
    }
    {
        /**
         * - CountAction
         * This counts the number of times a keyword argument occurs.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--verbose", "-v", argparse::actions::CountAction())
        .set_default(0); // Note: default value is set unless eplicitly set to 0.
        // cmd: ./05_actions -vvv
        std::cout << "cmd > ./05_actions -vvv\n";
        int argc = 2;
        char const* argv[] = {
            "05_actions",
            "-vvv",
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << std::endl;
    }

    return 0;
}