#include <iostream>
#include <argparse/argparse.h>

int main()
{
    auto parser = argparse::ArgumentParser("argparse");
    parser.set_description("What the program does")
    .set_epilog("Text at the bottom of help");
    parser.add_argument("filename");        // positional argument
    parser.add_argument("-c", "--count");   // option that takes a value
    parser.add_argument("-v", "--verbose", argparse::actions::StoreTrueAction()); // on/off flag

    // cmd: ./00_basic_usage file_name --count 15 --verbose
    std::cout << "cmd > ./00_basic_usage file_name --count 15 --verbose\n";
    int argc = 5;
    char const* argv[] = {
        "00_basic_usage", // executable file
        "file_name",
        "--count", "15",
        "--verbose"
    };

    auto args = parser.parse_args(argc, argv);
    
    std::cout << args << std::endl;
    std::cout << "filename:  " << args.get<std::string>("filename") << std::endl;
    std::cout << "--count:   " << args.get<int>("count") << std::endl;
    std::cout << "--verbose: " << std::boolalpha << args.get<bool>("verbose") << std::endl;

    return 0;
}