# argparse

C++ Argument Parser (like python argparse)

## Table Of Contents

- [Requirement](#requirement)
- [A Simple Example](#a-simple-example)
- [ArgumentParser](#argumentparser)
  - [prog\_name](#prog_name)
  - [help](#help)
  - [prefix\_char](#prefix_char)
  - [exit\_on\_error](#exit_on_error)
  - [allow\_abbrev](#allow_abbrev)
- [Argument](#argument)
  - [action](#action)
  - [nargs](#nargs)
  - [const](#const)
  - [default](#default)
  - [choices](#choices)
  - [required](#required)
  - [help](#help-1)
  - [metavar](#metavar)
  - [dest](#dest)
- [The parse\_args() method](#the-parse_args-method)
  - [Option Value Syntax](#option-value-syntax)
  - [Invalid Arguments](#invalid-arguments)
  - [Arguments Containing -](#arguments-containing--)
  - [Argument Abbreviations (prefix matching)](#argument-abbreviations-prefix-matching)
  - [The Namespace Object](#the-namespace-object)
- [Sub-commands](#sub-commands)
- [Argument Groups](#argument-groups)
- [Mutually Exclusive Groups](#mutually-exclusive-groups)

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
- The `add_argument(...)` returns a reference to the added `Argument` object. The returned reference allows for the setting of various argument's properties.
- The `parse_args(argc, argv)` method enables reading command-line arguments and returns a map object(`Namespace`) that stores the given arguments and their values as key-value pairs.
- All argument values are stored as strings. The stored values can be accessed through the `get<T>("key")` method of the returned map object.

## ArgumentParser

```c++
namespace argparse {
    class ArgumentParser(std::string_view const prog_name = "", bool const help = true, char const prefix_char = '-', bool const exit_on_error_ = true)
}
```

- `prog_name` - The name of the program (default: the first command-line argument)
- `help` - Add a `-h/--help` option to the parser (default: `true`)
- `prefix_char` - The character that prefix optional arguments (default: `-`)
- `exit_on_error` - Determines whether or not ArgumentParser exits with error info when an error occurs (default: `true`)

### prog_name

By default, `ArgumentParser` object use the first command-line argument to determine how to display the name of the program in help messages. Default value is set when parsing command-line arguments. For example, consider a executable named `program` with the following code:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser();
    parser.add_argument("--foo").set_help("foo help");

    auto args = parser.parse_args(argc, argv);
}
```
The help for this program will display `program` as the program name (regardless of where the program was invoked from):
```
$ ./program --help
usage: program [-h] [--foo FOO]

options:
  -h, --help  show this help message and exit
  --foo FOO   foo help
```

To change this default behavior, another value can be suppplied using the `prog_name` argument to `ArgumentParser`:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("myprogram");
    parser.add_argument("--foo").set_help("foo help");

    auto args = parser.parse_args(argc, argv);
}
```
```
$ ./program --help
usage: myprogram [-h] [--foo FOO]

options:
  -h, --help  show this help message and exit
  --foo FOO   foo help
```

### help

By default, `ArgumentParser` objects add an option which simple displays the parser's help message. For example, consider a file named `program` containing the following code:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser();
    parser.add_argument("--foo").set_help("foo help");

    auto args = parser.parse_args(argc, argv);
}
```
If `-h` or `--help` is supplied at the command line, the `ArgumentParser` help will be printed:
```
$ ./program --help
usage: program [-h] [--foo FOO]

options:
  -h, --help  show this help message and exit
  --foo FOO   foo help
```

Occasionally, it may be usefule to disable the addition of this help option. This can be achieved by passing `false` as the `help` argument to the `ArgumentParser` constructor:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program", false);
    parser.add_argument("--foo").set_help("foo help");
    parser.print_help();
}
```
```
$ ./program
usage: program [--foo FOO]

options:
  --foo FOO  foo help
```

The help option is typically `-h/--help`. The execption to this is if the `prefix_char` is specified to another character, in which case `-h` and `--help` are not valid options.

> refer to [02_help_argument.cpp](/samples/02_help_argument.cpp)

### prefix_char

Most command-line options will use `-` as the prefix, e.g. `-f/--foo`. Parsers that need to support different or additional prefix characters, e.g. for options like `+f` or `/foo`, may specify them using the `prefix_char` argument to the `ArgumentParser` constructor:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program", true, '+');
    parser.add_argument("+f");
    parser.add_argument("++bar");
    
    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program +f X ++bar Y
Namespace(bar=Y, f=X)
```

### exit_on_error

Normally, when you pass an invalid argument list to the `parse_args(...)` method of an `ArgumentParser`, it will exit with error info.

If the user would like to catch errors manually, the feature can be enabled by setting `exit_on_error` argument to `false`:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program", true, '-', false);
    parser.add_argument("--foo");
    
    try {
        auto args = parser.parse_args(argc, argv);
    }
    catch (std::exception& e) {
        std::cout << "catch exception\n";
        std::cout << e.what() << std::endl;
    }
}
```
```
$ ./program --foo
catch exception
argument --foo: expected one argument
```

> refer to [03_exit_on_error.cpp](/samples/03_exit_on_error.cpp)

### allow_abbrev

Normally, when you pass an argument list to the `parse_args()` method of an `ArgumentParser`, it recognizes abbreviations of long options.

This feature can be disabled by calling `set_abbrev()` with `false`.
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.set_abbrev(false);
    parser.add_argument("--foobar", argparse::actions::StoreTrueAction());
    parser.add_argument("--foonley", argparse::actions::StoreFalseAction());
    
    auto args = parser.parse_args(argc, argv);
}
```
```
$ ./program --foon
usage: program [-h] [--foobar] [--foonley]
[ARGPARSE ERROR] unrecognized arguments: --foon
```

> refer to [01_allow_abbrev.cpp](/samples/01_allow_abbrev.cpp)

## Argument

```c++
namespace argparse {
    template<typename... Args>
    Argument& ArgumentParser::add_argument(Args&&... args);
}
```

To add a new argument (`Argument`), call the `add_argument(...)` method. This method must know whether an optional argument, or a positional argument is expected. The arguments passed to `add_argument()` must therefore be either a series of flags, or a simple argument name.

For example, an optional argument could be created like:
```c++
parser.add_argument("-f", "--foo");
```

while a positional argument could be created like:
```c++
parser.add_argument("bar");
```

When `parse_args()` is called, optional arguments will be identified by the `-` prefix, and the remaining arguments will be assumed to be positional:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("-f", "--foo");
    parser.add_argument("bar");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program BAR
Namespace(bar=BAR)
$ ./program BAR --foo FOO
Namespace(foo=FOO, bar=BAR)
$ ./program --foo FOO
usage: program [-h] [-f FOO] bar
[ARGPARSE ERROR] the following arguments are required: bar
```

### action

`Argument` objects is associated with actions. These actions can do just about anything with the command-line arguments associated with them, though most actions simple add an attirbute to the object returned by `parse_args()`. The action specifies how the command-line arguments should be handled. An action can be set by passing the `Action` class as the last argument to the `add_argument()` method. The supplied actions are:

- `StoreAction` - This just stores the argument's value. This is the default action.
- `StoreConstAction` - This stores the specified value by `set_const()` method. Default const value is empty. The `StoreConstAction` action is most commonly used with optional arguments that specify some sort of flag.
- `StoreTrueAction` and `StoreFalseAction` - These are special cases of `StoreConstAction` used for storing the values `true` and `false` respectively. In addition, they create default values of `false` and `true` respectively.
- `AppendAction` - This stores a list, and appends each argument value to the list. It is useful to allow an option to be specified multiple times. If the default value is non-empty, the default elements will be present in the parsed value for the option, with any values from the command line appended after those default values.
- `AppendConstAction` - This stores a list, and appends the value specified by the `set_const` method to the list. `AppendConstAction` is typically useful when multiple arguments need to store constants to the same list.
- `CountAction` - This counts the number of times a keyword argument occurs. This is useful for increasing verbosity levels.
- `HelpAction` - This prints a complete help message for all the options in the current parser and then exits. By default a help action is automatically added to the parser.

If you need an action other than the above actions, you can define and use a custom action. A custom action requires 3 methods - `initialize()`, `get_action()`, `get_valid()`:
```c++
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
```

> for examples, refer to [05_actions.cpp](/samples/05_actions.cpp) and [06_custom_action.cpp](/samples/06_custom_action.cpp)

### nargs

`Argument` objects usually associate a single command-line argument with a single action to be taken. The `nargs` associates a different number of command-line arguments with a single action. The `nargs` can be set by `set_nargs()` method. The supported values are:

- `N` (an integer) - `N` arguments from the command line will be gathered together into a list.
- `?` (optional) - One argument will be consumed from the command line if possible, and produced as a single item. If no command-line argument is present, the value that is set by `set_default` method will be produced. <br>Note that for optional arguments, there is an additional case - the option string is present but not followed by a command-line argument. In this case the value from `const` will be produced.
- `*` (zero or more) - All command-line arguments present are gathered into a list. <br>Note that it generally doesn't make much sense to have more than one positional argument with `nargs='*'`, but multiple optional arguments with `nargs='*'` is possible.
- `+` (one or more) - Just like `*`, all command-line arguments present are gathered into a list. Additionally, an error message will be gathered if there wasn't one command-line argument present.

If the `nargs` is not set from `set_nargs` method, the number of arguments consumed is determined by the `action`. Generally this means a single command-line argument will be consumed and a single item will be produced.

> refer to [07_nargs.cpp](/samples/07_nargs.cpp)

### const

The `const_value` of `Argument` is used to hold constant values that are not read from the command line but are required for the various actions. The `const_value` can be set by calling `set_const()`. The two most common uses of it are:

- When `add_argument()` is called with `StoreConstAction()` or `AppendConstAction()`. These actions add the `const_value` to one of the attirbutes of the object returned by `parse_args()`. See [05_actions.cpp](/samples/05_actions.cpp) for examples. If `const_value` is not set from `set_const()`, it will receive no values (a default value in the object returned by `parse_args()` is initialized to `None`).
- When `add_argument()` is called with option strings (like `-f` or `--foo`) and `nargs='?'`. This creates an optional argument that can be followed by zero or one command-line arguments.

### default

All optional arguments and some positional arguments may be omitted at the command line. The `default_value` of `Argument` specifies what value should be used if the command-line argument is not present. The `default_value` can be set by calling `set_default()`. For optional arguments, the `default_value` is used when the option string was not present at the command line:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("--foo")
    .set_default(42); // or passing "42"

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program --foo 2
Namespace(foo=2)
$ ./program
Namespace(foo=42)
```

For positional arguments with `nargs` equal to `?` or `*`, the `default_value` is used when no command-line argument was present:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("foo")
    .set_nargs('?')
    .set_default(42);

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program a
Namespace(foo=a)
$ ./program
Namespace(foo=42)
```

Passing `argparse::Argument::SUPPRESS` to `set_default()` causes no attribute to be added if the command-line argument was not present:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("--foo")
    .set_default(argparse::Argument::SUPPRESS);

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program
Namespace()
$ ./program --foo 1
Namespace(foo=1)
```

### choices

Some command-line arguments should be selected from a restricted set of values. These can be handled by passing a sequence of strings as argument to `set_choices()` method. When the command line is paresed, argument values will be checked, and an error message will be displayed if the argument was not one of the acceptable values:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("move")
    .set_choices({"rock", "paper", "scissors"});

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program rock
Namespace(move=rock)
$ ./program fire
usage: program [-h] {rock,paper,scissors}
[ARGPARSE ERROR] argument move: invalid choice: 'fire' (choose from 'rock', 'paper', 'scissors')
```

> refer to [08_choices.cpp](/samples/08_choices.cpp)

### required

In general, the `ArgumentParser` assumes that flags like `-f` and `--bar` indicate optional arguments, which can always be omitted at the command line. To make an option required, `true` can be specified by calling `set_required()`:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("--foo")
    .set_required(true);

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program --foo BAR
Namespace(foo=BAR)
$ ./program
usage: program [-h] --foo FOO
[ARGPARSE ERROR] the following arguments are required: --foo
```

As the example shows, if an option is marked as `required`, `parse_args()` will report an error if that option is not present at the command line.

### help

The `help` is a string containing a brief description of the argument. When a user requests help (usually by using `-h` or `--help` at the command line), these `help` description will be displayed with each argument. The `help` description can be set by `set_help()` method:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("frobble");
    parser.add_argument("--foo", argparse::actions::StoreTrueAction())
    .set_help("foo the bars before frobbling");
    parser.add_argument("bar")
    .set_nargs('+')
    .set_help("one of the bars to be frobbled");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./frobble -h
usage: frobble [-h] [--foo] bar [bar ...]

positional arguments:
  bar         one of the bars to be frobbled

options:
  -h, --help  show this help message and exit
  --foo       foo the bars before frobbling
```

`ArgumentParser` supports silencing the help entry for certain options, by setting the `help` to `argparse::Argument::SUPPRESS`:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("frobble");
    parser.add_argument("--foo", argparse::actions::StoreTrueAction())
    .set_help(argparse::Argument::SUPPRESS);

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./frobble -h
usage: frobble [-h]

options:
  -h, --help  show this help message and exit
```

### metavar

When `ArgumentParser` generates help messages, it needs some way to refer to each expected argument. By default, `ArgumentParser` objects use the `dest` value as the "name" of each object. By default, for positional arguments, the `dest` value is used directly, and for optional arguments, the `dest` value is uppercased. So, a single positional argument with `dest="bar"` will be referred to as `bar`. A single optional argument `--foo` that should be followed by a single command-line argument will be referred to as `FOO`. An example:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("--foo")
    .set_metavar("YYY");
    parser.add_argument("bar")
    .set_metavar("XXX");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program X --foo Y
Namespace(foo=Y, bar=X)
$ ./program -h
usage: program [-h] [--foo YYY] XXX

positional arguments:
  XXX

options:
  -h, --help  show this help message and exit
  --foo YYY
```

Note that `metavar` only changes the _displayed_ name - the name of the attribute on the object returned by `parse_args()` is still determined by the `dest` value.

> Not supported that different values of `nargs` cause the `metavar` to be used multiple times.
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("x")
    .set_nargs(2);
    parser.add_argument("--foo")
    .set_nargs(2)
    .set_metavar("bar");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program -h
usage: program [-h] [--foo bar bar] x x

positional arguments:
  x

options:
  -h, --help     show this help message and exit
  --foo bar bar
```

### dest

Most actions add some value as an attribute of the object returned by `parse_args()`. The name of this attribute is determined by the `dest` value of `Argument`. For positional arguments, `dest` is normally supplied as the first argument to `add_argument()`:

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("bar");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program XXX
Namespace(bar=XXX)
```

For optional arguments, the value of `dest` is normally inferred from the option strings. `ArgumentParser` generates the value of `dest` by taking the first long option string and stripping away the initial `--` string. If no long option strings were supplied, `dest` will be derived from the first short option string by stripping the initial `-` character. Any internal `-` characters will be converted to `_` characters to make sure the string is a valid attribute name. The example below illustrate this behavior:

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("-f", "--foo-bar", "--foo");
    parser.add_argument("-x", "-y");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program -f 1 -x 2
Namespace(x=2, foo_bar=1)
$ ./program --foo 1 -y 2
Namespace(x=2, foo_bar=1)
```

`dest` allows a custom attribute name by calling `set_dest()` method:

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("--foo")
    .set_dest("bar");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program --foo XXX
Namespace(bar=XXX)
```

## The parse_args() method

```c++
namespace argparse {
    Namespace ArgumentParser::parse_args(int argc, char const *const *const argv);
}
```

`parse_args()` converts argument strings to objects and assign them as attributes of the returned object (`Namespace` object).

### Option Value Syntax

The `parse_args()` method supports several ways of specifying the value of an option (if it takes one). In the simplest case, the option and its value are passed as two separate arguments:

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("-x");
    parser.add_argument("--foo");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program -x X
Namespace(x=X)
$ ./program --foo FOO
Namespace(foo=FOO)
```

The option and value can also be passed as a single command-line argument, using `=` to separate them:
```
$ ./program --foo=FOO -x=X
Namespace(foo=FOO, x=X)
```

For short options (options only one character long), the option and its value can be concatenated:
```
$ ./program -xX
Namespace(x=X)
```

Several short options can be joined together, using only a single `-` prefix, as long as only the last option (or none of them) requires a value:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("-x", argparse::actions::StoreTrueAction());
    parser.add_argument("-y", argparse::actions::StoreTrueAction());
    parser.add_argument("-z");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program -xyzZ
Namespace(z=Z, y=true, x=true)
```

### Invalid Arguments

While parsing the command line, `parse_args()` checks for a variety of errors, including ambigous options, invalid options, wrong number of positional arguments, etc. When it encounters such an error, it exits and prints the error along with a usage message:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("--foo");
    parser.add_argument("bar")
    .set_nargs('?');

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```bash
# invalid option
$ ./program --bar
usage: program [-h] [--foo FOO] [bar]
[ARGPARSE ERROR] unrecognized arguments: --bar
# wrong number of arguments
$ ./program spam badger
usage: program [-h] [--foo FOO] [bar]
[ARGPARSE ERROR] unrecognized arguments: badger
```

### Arguments Containing -

The `parse_args()` method attempts to give errors whenever the user has clearly made a mistake, but some situations are inherently ambigous. For example, the command-line argument `-1` could either be an attemp to specify an option or an attemp to provide a positional argument. The `parse_args()` method is cautious here: positional arguments may only begin with `-` if they look like negative numbers and there are no options in the parser that look like negative numbers:

**Ex1**:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("-x");
    parser.add_argument("foo")
    .set_nargs('?');

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```bash
# no negative number options, so -1 is a positional argument
$ ./program -x -1
Namespace(x=-1)
# no negative number options, so -1 and -5 are positional arguments
$ ./program -x -1 -5
Namespace(foo=-5, x=-1)
```

**Ex2**:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("-1")
    .set_dest("one");
    parser.add_argument("foo")
    .set_nargs('?');

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```bash
# negative number options present, so -1 is an option
$ ./program -1 X
Namespace(one=X)
# negative number options present, so -2 is an option
$ ./program -2
usage: program [-h] [-1 ONE] [foo]
[ARGPARSE ERROR] unrecognized arguments: -2
# negative number options present, so both -1s are options
$ ./program -1 -1
usage: program [-h] [-1 ONE] [foo]
[ARGPARSE ERROR] argument -1: expected one argument
```

If you have positional arguments that must begin with `-` and don't look like negative numbers, you can insert the pseudo-argument `--` which tells `parse_args()` that everything after that is a positonal argument:
```
$ ./program -- -f
Namespace(foo=-f)
```

### Argument Abbreviations (prefix matching)

The `parse_args()` method by default allows long options to be abbreviated to a prefix, if the abbreviation is unambiguous (the prefix matches a unique option):
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("-bacon");
    parser.add_argument("-badger");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program -bac MMM
Namespace(bacon=MMM)
$ ./program -bad WOOD
Namespace(badger=WOOD)
$ ./program -ba BA
usage: program [-h] [-bacon BACON] [-badger BADGER]
[ARGPARSE ERROR] ambiguous option: -ba could match -bacon, -badger
```

An error is produced for arguments that could produce more than one options. This feature can be disabled by setting [`allow_abbrev`](#allow_abbrev) to `false`.

### The Namespace Object

```c++
namespace argparse {
    class Namespace;
}
```

This class is deliberately simple for storing values on the attributes. Internally, the values are stored as strings in the `map` object. You can get the value stored in `Namespace` object by the parser using `get<T>(key)` method of `Namespace` object.

If you want to check whether the key-value exists before reading the value, you can call `has(key)` method which returns boolean value.

```c++
auto args = parser.parse_args(argc, argv);

int foo_value = args.get<int>("foo");
bool bar_value = args.get<bool>("bar");
if (args.has("foo_bar")) {
    std::vector<int> value_list = args.get<std::vector<int>>("foo_bar");
}
```

> **Beyond `Namespace`:** <br>
> All values are stored as a sequence of strings. If you specify template parameter as a single type like `get<int>(key)`, which returns the first value in the sequence of strings as specified type. If the desired return type is not a string, additional conversion is performed internally. When a container type is passed the template parameter like `get<std::vector<int>>(key)` or `get<std::list<float>>(key)`, it returns a sequence of values as desired container type.

## Sub-commands

Many programs split up their functionality into a number of sub-commands, for example, the `git` program cna invoke sub-commands like `git checkout`, `git clone`, and `git commit`. Splitting up functionality this way can be a particularly good idea when a program performs several different functions which require different kinds of command-line arguments. `ArgumentParser` supports the creation of such sub-commands with the `add_subparsers()` method. The `add_subparsers()` method is normally called with no arguments and returns a `Argument` object with special action. This object can call a additional method, `add_parser()`, which takes a command name and so on, and returns an `ArgumentParser` object that can be modified as usual.

> **Note**: Currently, `ArgumentParser` constructor arguments are not supported when calling `add_parser()`.

```c++
namespace argparse {
    Argument& ArgumentParser::add_subparsers(std::string_view title = "", std::string_view description = "");

    ArgumentParser& Argument::add_parser(std::string const& name, std::vector<std::string> const& aliases, std::string_view help);
}
```

Description of parameters <br>
For `add_subparsers()`:

- `title` - title for the sub-parser group in help output: by default "subcommands" if description is provided, otherwise uses title for positional arguments
- `description` - description for the sub-parser group in help output

For `add_parser()`:

- `name` - command of the subparser like `checkout` of `git` program
- `aliases` - aliases of the subcommand
- `help` - help message for the subparser command

Example usage:

```c++
int main(int argc, char** argv) {
    // create the top-level parser
    auto parser = argparse::ArgumentParser("program");
    parser.add_argument("--foo", argparse::actions::StoreTrueAction())
    .set_help("foo help");
    auto& subparsers = parser.add_subparsers();
    subparsers.set_help("sub-command help");
    
    // create the parser for the 'a' command
    auto& parser_a = subparsers.add_parser("a", {}, "a help");
    parser_a.add_argument("bar").set_help("bar help");

    // create the parser for the 'b' command
    auto& parser_b = subparsers.add_parser("b", {}, "b help");
    parser_b.add_argument("--baz").set_choices({"X", "Y", "Z"}).set_help("baz help");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program a 12
Namespace(bar=12, foo=false)
$ ./program --foo b --baz Z
Namespace(baz=Z, foo=true)
```

Note that the object returned by `parse_args()` will only contain attributes for the main parser and the subparer that was selected by the command line (and not any other subparsers). So in the example above, when the `a` command is specified, only the `foo` and `bar` attributes are present, and when the `b` command is specified, only the `foo` and `baz` attributes are present.

Similarly, when a help message is requested from a subparser, only the help for that particular parser will be printed. The help message will not include parent parser or sibling parser messages. (A help message for each subparser command, however, can be given by calling `set_help()` as above.)

```
$ ./program --help
usage: program [-h] [--foo] {a,b} ...

positional arguments:
  {a,b}       sub-command help
    a         a help
    b         b help

options:
  -h, --help  show this help message and exit
  --foo       foo help

$ ./program a --help
usage: program a [-h] bar

positional arguments:
  bar         bar help

options:
  -h, --help  show this help message and exit

$ ./program b --help
usage: program b [-h] [--baz {X,Y,Z}]

options:
  -h, --help     show this help message and exit
  --baz {X,Y,Z}  baz help
```

The `add_subparsers()` method also supports `title` and `description` arguments. When either is present, the subparser's commands will appear in their own group in the help output. For example:

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    auto& subparser = parser.add_subparsers("subcommands", "valid subcommands");
    subparser.set_help("additional help");

    subparser.add_parser("foo");
    subparser.add_parser("bar");

    auto args = parser.parse_args(argc, argv);
}
```
```
$ ./program -h
usage: program [-h] {foo,bar} ...

options:
  -h, --help  show this help message and exit

subcommands:
  valid subcommands

  {foo,bar}   additional help
```

Furthermore, `add_parser()` supports an additional `aliases` argument, which allows multiple strings to refer to the same subparser. This example, aliases `co` as a shorthand for `checkout`:

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    auto& subparser = parser.add_subparsers();

    auto& checkout = subparser.add_parser("chekcout", {"co"});
    checkout.add_argument("foo");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program co bar
Namespace(foo=bar)
```

If it is necessary to check the name of the subparser that was invoked, the `dest` of subparser will work:

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    auto& subparser = parser.add_subparsers();
    subparser.set_dest("subparser_name");

    auto& subparser1 = subparser.add_parser("1");
    subparser1.add_argument("-x");
    auto& subparser2 = subparser.add_parser("2");
    subparser2.add_argument("y");

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program 2 frobble
Namespace(y=frobble, subparser_name=2)
```

## Argument Groups

```c++
namespace argparse {
    ArgumentGroup& ArgumentParser::add_argument_group(std::string_view title, std::string_view description);
}
```

By default, `ArgumentParser` groups command-line arguments into 'positional arguments' and 'options' when displaying help messages. When there is a better conceptual grouping of arguments than this default one, appropriate groups can be created using the `add_argument_group()` method:

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program", false);
    auto& group = parser.add_argument_group("group");
    group.add_argument("--foo").set_help("foo help");
    group.add_argument("bar").set_help("bar help");

    parser.print_help();
}
```
```
$ ./program
usage: program [--foo FOO] bar

group:
  --foo FOO  foo help
  bar        bar help
```

The `add_argument_group()` method returns an argument group object which has an `add_argument()` method just like a regular `ArgumentParser`. When an argument is added to the group, the parser treats is just like a normal argument, but displays the argument in a separate group for help messages. The `add_argument_group()` method accepts `title` and `description` arguments which can be used to customize this display:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program", false);
    auto& group1 = parser.add_argument_group("group1", "group1 description");
    group1.add_argument("foo").set_help("foo help");
    auto& group2 = parser.add_argument_group("group2", "group2 description");
    group2.add_argument("--bar").set_help("bar help");

    parser.print_help();
}
```
```
$ ./program
usage: program [--bar BAR] foo

group1:
  group1 description

  foo        foo help

group2:
  group2 description

  --bar BAR  bar help
```

Note that any arguments not in your user-defined groups will end up back in the usual 'positional arguments' and 'options' sections.

## Mutually Exclusive Groups

```c++
namespace argparse {
    MutuallyExclusiveGroup& ArgumentParser::add_mutually_exclusive_group(bool const required);
}
```

Create a mutually exclusive group. It will make sure that only one of the arguments in the mutually exclusive group was present on the command line.

```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    auto& group = parser.add_mutually_exclusive_group();
    group.add_argument("--foo", argparse::actions::StoreTrueAction());
    group.add_argument("--bar", argparse::actions::StoreFalseAction());

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program --foo
Namespace(bar=true, foo=true)
$ ./program --bar
Namespace(bar=false, foo=false)
$ ./program --foo --bar
[ARGPARSE ERROR] argument --bar: not allowed with argument --foo
```

The `add_mutually_exclusive_group()` method also accepts a `required` argument, to indicate that at least one of the mutually exclusive is required:
```c++
int main(int argc, char** argv) {
    auto parser = argparse::ArgumentParser("program");
    auto& group = parser.add_mutually_exclusive_group(true);
    group.add_argument("--foo", argparse::actions::StoreTrueAction());
    group.add_argument("--bar", argparse::actions::StoreFalseAction());

    auto args = parser.parse_args(argc, argv);
    std::cout << args << std::endl;
}
```
```
$ ./program
[ARGPARSE ERROR] one of the arguments --foo --bar is required
```

> **Note**: Some functionality needs to be updated.