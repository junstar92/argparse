#include <iostream>
#include <argparse/argparse.h>

int main()
{
    {
        /**
         * - N (integer)
         * N arguments from the command line will be gathered together into a list.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--foo")
        .set_nargs(2);
        parser.add_argument("bar")
        .set_nargs(1);
        // cmd: ./07_nargs c --foo a b
        std::cout << "cmd > ./07_nargs c --foo a b\n";
        int argc = 5;
        char const* argv[] = {
            "07_nargs",
            "c",
            "--foo", "a", "b"
        };

        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
    }
    {
        /**
         * - '?' (optional)
         * One argument will be consumed from the command line if possible, and produced as a single item.
         * If no command-line argument is present, the value set by set_default method will be produced.
         * Note that for optional arguments, there is an additional case
         * - the option string is present but not followed by a command-line argument.
         *   In this case the value from const will be produced.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--foo")
        .set_nargs('?')
        .set_const("c")
        .set_default("d");
        parser.add_argument("bar")
        .set_nargs('?')
        .set_default("d");
        // cmd: ./07_nargs XX --foo YY
        {
        std::cout << "cmd > ./07_nargs XX --foo YY\n";
        int argc = 4;
        char const* argv[] = {
            "07_nargs",
            "XX",
            "--foo", "YY"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
        }
        {
        // cmd: ./07_nargs XX --foo
        std::cout << "cmd > ./07_nargs XX --foo\n";
        int argc = 3;
        char const* argv[] = {
            "07_nargs",
            "XX",
            "--foo"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
        }
        {
        // cmd: ./07_nargs
        std::cout << "cmd > ./07_nargs\n";
        int argc = 1;
        char const* argv[] = {
            "07_nargs"
        };
        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
        }
    }
    {
        /**
         * - '*' (zero or more)
         * All command-line arguments present are gathered into a list.
         * Note that it generally doesn't make much sense to have more than one positional argument with nargs '*',
         * but multiple optional arguments with nargs '*' is possible.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("--foo")
        .set_nargs('*');
        parser.add_argument("--bar")
        .set_nargs('*');
        parser.add_argument("baz")
        .set_nargs('*');
        // cmd: ./07_nargs a b --foo x y --bar 1 2
        std::cout << "cmd > ./07_nargs a b --foo x y --bar 1 2\n";
        int argc = 9;
        char const* argv[] = {
            "07_nargs",
            "a", "b",
            "--foo", "x", "y",
            "--bar", "1", "2"
        };

        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
    }
    {
        /**
         * - '+' (one or more)
         * Just like '*', all command-line arguments present are gathered into a list.
         * Additionally, an error message will be generated if there wasn't one command-line argument present.
         */
        auto parser = argparse::ArgumentParser();
        parser.add_argument("foo")
        .set_nargs('+');
        {
        // cmd: ./07_nargs a b
        std::cout << "cmd > ./07_nargs a b\n";
        int argc = 3;
        char const* argv[] = {
            "07_nargs",
            "a", "b"
        };

        auto args = parser.parse_args(argc, argv);
        std::cout << args << "\n\n";
        }
        {
        // cmd: ./07_nargs
        std::cout << "cmd > ./07_nargs\n";
        int argc = 1;
        char const* argv[] = {
            "07_nargs"
        };

        // it generates an error.
        auto args = parser.parse_args(argc, argv);
        std::cout << args << std::endl;
        }
    }

    return 0;
}