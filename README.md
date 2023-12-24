# argparse

C++ (python-argparse-like) Argument Parser

## Requirement

- C++17

## A Simple Example

Here's a simple example.

```c++
#include <iostream>
#include <argparse/argparse.h>

int main(int argc, char** argv)
{
    auto parser = argparse::ArgumentParser();

    parser.add_argument("square")
    .set_help("display a square of a given number");
    parser.add_argument("-v", "--verbose", argparse::actions::StoreTrueAction())
    .set_help("increase output verbosity");

    auto args = parser.parse_args(argc, argv);
    int square = args.get<int>("square");
    
    if (args.get<bool>("verbose")) {
        std::cout << "the square of " << square << " equals " << square * square << std::endl;
    }
    else {
        std::cout << square * square << std::endl;
    }

    return 0;
}
```

Running this code after compiling:
```
$ ./program 4
16
$ ./program 4 --verbose
the square of 4 equals 16
$ ./program --verbose 4
the square of 4 equals 16
$ ./program -h
usage: program [-h] [-v] square

positional arguments:
  square         display a square of a given number

options:
  -h, --help     show this help message and exit
  -v, --verbose  increase output verbosity
```

- The `add_argument(...)` method of `ArgumentParser` adds command-line options the program will receive: The argument `square` is a positional argument and the argument `-v`(`--version`) is an optional argument.
- The `add_argument(...)` returns a reference to the added `argparse::Argument` object. The returned reference allows for the setting of various argument's properties.
- The `parse_args(argc, argv)` method enables reading command-line arguments and returns a map object(`argparse::Namespace`) that stores the given arguments and their values as key-value pairs.
- All argument values are stored as strings. The stored values can be accessed through the `get<T>("key")` method of the returned map object.