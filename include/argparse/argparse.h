#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <list>
#include <vector>
#include <optional>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <type_traits>
#include <utility>
#include <iterator>
#include <functional>
#include <limits>
#include <regex>
#include <iomanip>

#if defined(__linux__) || defined(__APPLE__)
#include <sys/ioctl.h>
#else
#include <windows.h>
#endif

namespace argparse {

class ArgumentParser;
class Argument;
using ArgumentRef = std::reference_wrapper<Argument>;
using ArgumentCRef = std::reference_wrapper<Argument const>;

// ==============================================================================================================
// Utility
// ==============================================================================================================
namespace utils {

template<typename CharT>
using tstring = std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>>;

template<typename CharT>
inline tstring<CharT>
to_upper(tstring<CharT> text)
{
    std::transform(std::begin(text), std::end(text), std::begin(text), ::toupper);
    return text;
}

template<typename CharT>
inline tstring<CharT>
trim(tstring<CharT> const& text)
{
    auto first{text.find_first_not_of(' ')};
    auto last{text.find_last_not_of(' ')};
    return text.substr(first, (last - first + 1));
}

inline std::string
join(std::vector<std::string> const& text, std::string const& chars)
{
    std::string ret;
    size_t const size = text.size();
    for (size_t i = 0; i < size; ++i) {
        ret += text[i];
        if (i < size - 1) {
            ret += chars;
        }
    }
    return ret;
}

template<typename T>
struct str_to_numeric;
template<>
struct str_to_numeric<short> {
    template<typename... Args>
    short operator()(Args&&... args) {
        return static_cast<short>(std::stoi(std::forward<Args...>(args)...));
    }
};
template<>
struct str_to_numeric<int> {
    template<typename... Args>
    int operator()(Args&&... args) {
        return std::stoi(std::forward<Args...>(args)...);
    }
};
template<>
struct str_to_numeric<long> {
    template<typename... Args>
    long operator()(Args&&... args) {
        return std::stol(std::forward<Args...>(args)...);
    }
};
template<>
struct str_to_numeric<long long> {
    template<typename... Args>
    long long operator()(Args&&... args) {
        return std::stoll(std::forward<Args...>(args)...);
    }
};
template<>
struct str_to_numeric<unsigned short> {
    template<typename... Args>
    unsigned short operator()(Args&&... args) {
        return static_cast<unsigned short>(std::stoul(std::forward<Args...>(args)...));
    }
};
template<>
struct str_to_numeric<unsigned int> {
    template<typename... Args>
    unsigned int operator()(Args&&... args) {
        return static_cast<unsigned int>(std::stoul(std::forward<Args...>(args)...));
    }
};
template<>
struct str_to_numeric<unsigned long> {
    template<typename... Args>
    unsigned long operator()(Args&&... args) {
        return std::stoul(std::forward<Args...>(args)...);
    }
};
template<>
struct str_to_numeric<unsigned long long> {
    template<typename... Args>
    unsigned long long operator()(Args&&... args) {
        return std::stoull(std::forward<Args...>(args)...);
    }
};
template<>
struct str_to_numeric<float> {
    template<typename... Args>
    float operator()(Args&&... args) {
        return std::stof(std::forward<Args...>(args)...);
    }
};
template<>
struct str_to_numeric<double> {
    template<typename... Args>
    double operator()(Args&&... args) {
        return std::stod(std::forward<Args...>(args)...);
    }
};
template<>
struct str_to_numeric<long double> {
    template<typename... Args>
    long double operator()(Args&&... args) {
        return std::stold(std::forward<Args...>(args)...);
    }
};

} // namespace argparse::utils

namespace traits {

// check if type is convertible to std::string
template<typename T>
struct is_convertible_to_str {
    static constexpr bool value = std::is_convertible_v<T, std::string>;
};

// check if all types are convertible to std::string
template<typename... Ts>
struct all_convertible_to_str {
    static constexpr bool value = (is_convertible_to_str<Ts>::value && ... && true);
};

// check if some types are convertible to std::string
template<typename... Ts, size_t... Is>
constexpr bool some_convertible_to_str(std::tuple<Ts...> const& tuple, std::index_sequence<Is...>) {
    return (is_convertible_to_str<decltype(std::get<Is>(tuple))>::value && ... && true);
}

// get last type in variadic template parameters
template<typename... Ts>
struct last_type {};
template<typename T>
struct last_type<T> {
    using type = T;
};
template<typename T0, typename T1, typename... Ts>
struct last_type<T0, T1, Ts...> : last_type<T1, Ts...> {};
template<typename... Ts>
using last_type_t = typename last_type<Ts...>::type;

// check if a type has initialize member function
template<typename, typename = std::void_t<>>
struct has_initialize : std::false_type {};
template<typename T>
struct has_initialize<T, std::void_t<decltype(&T::initialize)>> : std::true_type {};

// check if a type has get_action member function
template<typename, typename = std::void_t<>>
struct has_get_action : std::false_type {};
template<typename T>
struct has_get_action<T, std::void_t<decltype(&T::get_action)>> : std::true_type {};

// check if a type has get_valid member function
template<typename, typename = std::void_t<>>
struct has_get_valid : std::false_type {};
template<typename T>
struct has_get_valid<T, std::void_t<decltype(&T::get_valid)>> : std::true_type {};

// check if a type is Action class type
template<typename T>
struct is_action_class {
    static constexpr bool value = has_initialize<T>::value && has_get_action<T>::value && has_get_valid<T>::value;
};
template<typename T>
constexpr bool is_action_class_v = is_action_class<T>::value;

// check if it is container type
template<typename, typename = std::void_t<>>
struct is_container : std::false_type {};
template<>
struct is_container<std::string> : std::false_type {};
template<>
struct is_container<std::string_view> : std::false_type {};
template<typename T>
struct is_container<T, std::void_t<typename T::value_type,
                                   typename T::reference,
                                   typename T::const_reference,
                                   typename T::iterator,
                                   typename T::const_iterator,
                                   typename T::difference_type,
                                   typename T::size_type>> : std::true_type {};
template<typename T>
constexpr bool is_container_v = is_container<T>::value;

} // namespace argparse::traits

// ==============================================================================================================
// Namespace
// ==============================================================================================================

class Namespace
{
private:
    std::unordered_map<std::string, std::vector<std::string>> data_;
    inline static std::string const true_value = "true";
    inline static std::string const false_value = "false";

public:
    Namespace() {}
    Namespace(Namespace const& other) : data_{other.data_} {}
    Namespace(Namespace&& other) noexcept : data_{std::move(other.data_)} {}

    bool find(std::string const& key) const {
        return data_.find(key) != data_.end();
    }

    void set_value(std::string const& key, std::string const& value) {
        auto& arg_values = data_[key];
        if (arg_values.empty()) {
            arg_values.push_back(value);
        }
        else {
            arg_values[0] = value;
        }
    }

    void set_values(std::string const& key, std::vector<std::string> const& values) {
        data_[key] = values;
    }

    void append_value(std::string const& key, std::vector<std::string> const& values) {
        auto& arg_values = data_[key];
        if (arg_values.empty()) {
            arg_values = values;
        }
        else {
            for (auto const& value : values) {
                arg_values.push_back(value);
            }
        }
    }

    auto const& get_values() const {
        return data_;
    }

    template<typename T = std::string>
    auto get(std::string const& key) const {
        auto const& values = data_.at(key);
        if constexpr (traits::is_container_v<T>) {
            if constexpr (std::is_same_v<std::decay_t<typename T::value_type>, std::string>) {
                return values;
            }
            else if constexpr (std::is_same_v<std::decay_t<typename T::value_type>, bool>) {
                T ret;
                std::transform(values.cbegin(), values.cend(), std::back_inserter(ret), [&](std::string const& value) {
                    if (value == true_value) {
                        return true;
                    }
                    else if (value == false_value) {
                        return false;
                    }
                    else {
                        throw std::invalid_argument(value + " is not convertible to bool type");
                    }
                });
                return ret;
            }
            else {
                T ret;
                std::transform(values.cbegin(), values.cend(), std::back_inserter(ret), utils::str_to_numeric<typename T::value_type>());
                return ret;
            }
        }
        else {
            if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
                return values[0];
            }
            else if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
                auto const& value = values[0];
                if (value == true_value) {
                    return true;
                }
                else if (value == false_value) {
                    return false;
                }
                else {
                    throw std::invalid_argument(value + " is not convertible to bool type");
                }
            }
            else {
                return utils::str_to_numeric<T>()(values[0]);
            }
        }
    }

    std::vector<std::string>& operator[](std::string const& key) {
        if (data_.find(key) == data_.end()) {
            data_[key] = std::vector<std::string>{};
        }
        return data_[key];
    }

    std::vector<std::string> const& operator[](std::string const& key) const {
        if (data_.find(key) == data_.end()) {
            throw std::out_of_range(key + " is not found");
        }
        return data_.at(key);
    }

    friend std::ostream& operator<<(std::ostream& os, Namespace const& args) {
        os << "Namespace(";
        size_t i = 0, size = args.data_.size();
        for (auto const& pair : args.data_) {
            os << pair.first << '=';
            if (pair.second.empty()) {
                os << "None";
            }
            else {
                if (pair.second.size() == 1) {
                    os << pair.second[0];
                }
                else {
                    os << '[';
                    for (size_t j = 0, size_j = pair.second.size(); j < size_j; ++j) {
                        os << pair.second[j];
                        if (j < size_j - 1) {
                            os << ", ";
                        }
                    }
                    os << ']';
                }
            }
            if (i < size - 1) {
                os << ", ";
            }
            ++i;
        }
        os << ")";
        return os;
    }
};

// ==============================================================================================================
// Exception for Argument
// ==============================================================================================================
class argument_error : public std::invalid_argument {
public:
    explicit argument_error(Argument const& argument, std::string const& msg);
    explicit argument_error(Argument const& argument, const char* msg);

    ~argument_error() noexcept override {}

    const char* what() const noexcept override;

private:
    std::string msg_;
};

// ==============================================================================================================
// Action
// ==============================================================================================================

using Action = std::function<void(ArgumentParser*, Namespace&, Argument const&, std::vector<std::string> const&)>;
using Validation = std::function<void(Argument const&)>;

namespace actions {

class StoreAction {
public:
    void initialize(Argument& argument);
    Action get_action();
    Validation get_valid();
};

class StoreConstAction {
public:
    void initialize(Argument& argument);
    Action get_action();
    Validation get_valid();
};

class StoreTrueAction {
public:
    void initialize(Argument& argument);
    Action get_action();
    Validation get_valid();
};

class StoreFalseAction {
public:
    void initialize(Argument& argument);
    Action get_action();
    Validation get_valid();
};

class AppendAction {
public:
    void initialize(Argument& argument);
    Action get_action();
    Validation get_valid();
};

class AppendConstAction {
public:
    void initialize(Argument& argument);
    Action get_action();
    Validation get_valid();
};

class CountAction {
public:
    void initialize(Argument& argument);
    Action get_action();
    Validation get_valid();
};

class HelpAction {
public:
    void initialize(Argument& argument);
    Action get_action();
    Validation get_valid();
};

} // namespace argparse::actions

// ==============================================================================================================
// NArgs
// ==============================================================================================================

class NArgs
{
public:
    enum class Type {
        none = 0,
        integer = 1,
        optional = 2,       /* '?' */
        zero_or_more = 3,   /* '*' */
        one_or_more = 4,    /* '+' */
        remainder = 5,
        parser = 6,
        suppress = 7
    };

private:
    Type type_;
    size_t nargs_;

public:
    NArgs() : type_{Type::none}, nargs_{0} {}

    explicit NArgs(int n) {
        nargs_ = static_cast<size_t>(n);
        type_ = Type::integer;
    }

    explicit NArgs(char c) {
        if (c == '?') {
            type_ = Type::optional;
        }
        else if (c == '*') {
            type_ = Type::zero_or_more;
        }
        else if (c == '+') {
            type_ = Type::one_or_more;
        }
        else {
            throw std::invalid_argument("invalid nargs value");
        }
    }

    void set_type_parser() {
        type_ = Type::parser;
        nargs_ = {};
    }

    Type get_type() const {
        return type_;
    }

    size_t get_nargs() const {
        return nargs_;
    }

    std::string get_nargs_pattern(bool optional) const {
        std::string nargs_pattern;
        switch (type_)
        {
        case NArgs::Type::none:
            // the default (none) is assumed to be a single argument
            nargs_pattern = "(-*A-*)";
            break;
        case NArgs::Type::optional:
            // allow zero or one argument
            nargs_pattern = "(-*A?-*)";
            break;
        case NArgs::Type::zero_or_more:
            // allow zero or moer arguments
            nargs_pattern = "(-*[A-]*)";
            break;
        case NArgs::Type::one_or_more:
            // allow one or more arguments
            nargs_pattern = "(-*A[A-]*)";
            break;
        case NArgs::Type::remainder:
            // allow any number of options or arguments
            nargs_pattern = "([-AO]*)";
            break;
        case NArgs::Type::parser:
            // allow one argument followed by any number of options or arguments
            nargs_pattern = "(-*A[-AO]*)";
            break;
        case NArgs::Type::suppress:
            // suppress action, like nargs = 0
            nargs_pattern = "(-*-*)";
            break;
        case NArgs::Type::integer:
            // all other should be intergers
            if (nargs_ == static_cast<size_t>(0)) {
                nargs_pattern = "(-*-*)";
            }
            else {
                nargs_pattern = "(-*";
                for (size_t i = 0; i < nargs_ - 1; i++) {
                        nargs_pattern.append("A-*");
                }
                nargs_pattern.append("A-*)");
            }
            break;
        default:
            throw std::logic_error("invalid condition");
            break;
        }

        // if this is an optional action
        auto replace_all = [](std::string& str, std::string_view from, std::string_view to) {
            size_t pos = 0;
            while ((pos = str.find(from, pos)) != std::string::npos) {
                str.replace(pos, from.length(), to);
                pos += to.length();
                if (pos >= str.length()) break;
            }
        };
        if (optional) {
            replace_all(nargs_pattern, "-*", "");
            replace_all(nargs_pattern, "-", "");
        }

        return nargs_pattern;
    }
};

// ==============================================================================================================
// Argument
// ==============================================================================================================

enum class ArgumentType {
    undefined,
    positional_argument,
    optional_argument,
    subcommand_argument,
};

class Argument
{
    friend class ArgumentParser;
    friend class HelpFormatter;
    inline static const std::string true_value = "true";
    inline static const std::string false_value = "false";

private:
    inline static char prefix_char_{};

    std::vector<std::string> option_strings_;
    Action action_;
    Validation validation_;
    NArgs nargs_;
    bool required_{false};
    std::string help_{};
    std::string metavar_{};
    std::string dest_{};
    std::vector<std::string> choices_{};
    ArgumentType type_{ArgumentType::undefined};

    std::vector<std::string> const_value_{};
    std::vector<std::string> default_value_{};

    // data for subparser argument
    ArgumentParser* parent_{nullptr};
    std::vector<std::vector<std::string>> subparser_names;
    std::list<Argument> subarguments;

public:
    inline static std::string SUPPRESS = "===SUPPRESS===";

    Argument& set_nargs(int n) {
        nargs_ = NArgs(n);
        return *this;
    }
    Argument& set_nargs(char c) {
        nargs_ = NArgs(c);
        return *this;
    }
    NArgs const& get_nargs() const {
        return nargs_;
    }

    Argument& set_required(bool const required) {
        if (get_type() == ArgumentType::positional_argument) {
            throw std::logic_error("'required' is an invalid argument for positionals");
        }
        required_ = required;
        return *this;
    }
    bool get_required() const {
        return required_;
    }

    Argument& set_help(std::string_view help) {
        help_ = help;
        return *this;
    }
    std::string get_help() const {
        return help_;
    }

    Argument& set_metavar(std::string_view metavar) {
        metavar_ = metavar;
        return *this;
    }
    std::string get_metavar() const {
        return metavar_;
    }

    Argument& set_dest(std::string_view dest) {
        dest_ = dest;
        return *this;
    }
    std::string get_dest() const {
        return dest_;
    }

    Argument& set_choices(std::vector<std::string> const& choices) {
        choices_ = choices;
        return *this;
    }
    std::vector<std::string> const& get_choices() const {
        return choices_;
    }
    std::string get_choice_str() const {
        std::string choices;
        for (size_t i = 0, size = choices_.size(); i < size; ++i) {
            choices += '\'';
            choices += choices_[i];
            choices += '\'';
            if (i < size - 1) {
                choices += ", ";
            }
        }
        return choices;
    }

    ArgumentType get_type() const {
        return type_;
    }

    template<typename T>
    Argument& set_const(T const& const_value) {
        const_value_ = {std::to_string(const_value)};
        return *this;
    }
    template<>
    Argument& set_const(std::string const& const_value) {
        const_value_ = {const_value};
        return *this;
    }
    template<size_t SIZE>
    Argument& set_const(char const(&const_value)[SIZE]) {
        const_value_ = {const_value};
        return *this;
    }
    template<>
    Argument& set_const(char const(&const_value)[]) {
        const_value_ = {const_value};
        return *this;
    }
    template<>
    Argument& set_const(bool const& const_value) {
        if (const_value) {
            const_value_ = {true_value};
        }
        else {
            const_value_ = {false_value};
        }
        return *this;
    }
    std::vector<std::string> const& get_const() const {
        return const_value_;
    }

    template<typename T>
    Argument& set_default(T const& default_value) {
        default_value_ = {std::to_string(default_value)};
        return *this;
    }
    template<>
    Argument& set_default(std::string const& default_value) {
        default_value_ = {default_value};
        return *this;
    }
    template<size_t SIZE>
    Argument& set_default(char const(&default_value)[SIZE]) {
        default_value_ = {default_value};
        return *this;
    }
    template<>
    Argument& set_default(char const(&default_value)[]) {
        default_value_ = {default_value};
        return *this;
    }
    template<>
    Argument& set_default(bool const& default_value) {
        if (default_value) {
            default_value_ = {true_value};
        }
        else {
            default_value_ = {false_value};
        }
        return *this;
    }
    std::vector<std::string> const& get_default() const {
        return default_value_;
    }

    std::string get_argument_name() const {
        std::string ret;
        if (option_strings_.size()) {
            ret = utils::join(option_strings_, "/");
        }
        else if (dest_.length() && dest_ != Argument::SUPPRESS) {
            ret = dest_;
        }
        else if (choices_.size()) {
            ret = '{';
            ret += utils::join(choices_, ",");
            ret += '}';
        }
        
        return ret;
    }

    ArgumentParser& add_parser(std::string const& name, std::vector<std::string> const& alias = {}, std::string_view help = "");

    void initialize();

    void check_validation(Argument const& argument) const {
        validation_(argument);
    }

    bool operator==(Argument const& other) const {
        return std::addressof(*this) == std::addressof(other);
    }
    bool operator!=(Argument const& other) const {
        return !(*this == other);
    }

private:
    Argument() {}

    template<typename ActionType,
             typename... Args, size_t N = sizeof...(Args)>
    void __add_name_or_flags(ActionType&& action_obj, Args&&... args) {
        constexpr size_t num_of_flags = (traits::is_convertible_to_str<traits::last_type_t<Args...>>::value ? N : N - 1);
        static_assert(num_of_flags > 0, "at least one name or flags must be given to add a argument");
        
        // set name or flags and determine argument type
        const auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
        if constexpr (num_of_flags == (N - 1)) {
            if constexpr (!traits::some_convertible_to_str(tuple, std::make_index_sequence<N-1>{})) {
                throw std::invalid_argument("invalid name or flags: name or flags must be string type");
            }
            if constexpr (num_of_flags == 1) {
                if (__is_start_with_prefix_char(std::get<0>(tuple))) {
                    __add_option_strings_from_tuple(tuple, std::make_index_sequence<N-1>{});
                    __set_type(ArgumentType::optional_argument);
                }
                else {
                    __set_type(ArgumentType::positional_argument);
                }
            }
            else {
                __add_option_strings_from_tuple(tuple, std::make_index_sequence<N-1>{});
                __set_type(ArgumentType::optional_argument);
            }
        }
        else {
            if constexpr (!traits::all_convertible_to_str<Args...>::value) {
                throw std::invalid_argument("invalid name or flags: name or flags must be string type");
            }
            if constexpr (num_of_flags == 1) {
                if (__is_start_with_prefix_char(std::get<0>(tuple))) {
                    __add_option_strings_from_tuple(tuple, std::make_index_sequence<N>{});
                    __set_type(ArgumentType::optional_argument);
                }
                else {
                    __set_type(ArgumentType::positional_argument);
                }
            }
            else {
                __add_option_strings_from_tuple(tuple, std::make_index_sequence<N>{});
                __set_type(ArgumentType::optional_argument);
            }
        }

        // set action and initialize
        action_obj.initialize(*this);
        action_ = action_obj.get_action();
        validation_ = action_obj.get_valid();

        // check option strings validation
        if (type_ == ArgumentType::optional_argument) {
            int err_opt_str_idx = __check_option_strings_valid();
            if (err_opt_str_idx != -1) {
                throw std::invalid_argument("invalid option string '" + option_strings_[err_opt_str_idx] + "': must start with a character '-'");
            }
            // set dest for optional argument
            for (auto const& option_string : option_strings_) {
                if (option_string[1] == '-') {
                    set_dest(option_string.substr(option_string.find_first_not_of(prefix_char_)));
                    break;
                }
            }
            if (dest_.length() == 0) {
                set_dest(option_strings_[0].substr(option_strings_[0].find_first_not_of(prefix_char_)));
            }
            std::for_each(dest_.begin(), dest_.end(), [](char& c) {
                if (c == '-') {
                    c = '_';
                }
            });
        }

        // make positional arguments as required if at least one is always required
        if (type_ == ArgumentType::positional_argument) {
            if (nargs_.get_type() != NArgs::Type::optional && nargs_.get_type() != NArgs::Type::zero_or_more) {
                required_ = true;
            }

            // set dest for positional argument
            set_dest(std::get<0>(tuple));
        }
    }

    Argument& __set_action(Action action) {
        action_ = action;
        return *this;
    }

    Argument& __set_validation(Validation validation) {
        validation_ = validation;
        return *this;
    }

    void __set_type(ArgumentType const type) {
        type_ = type;
    }

    int __check_option_strings_valid() const {
        for (auto it = option_strings_.begin(); it != option_strings_.end(); ++it) {
            if (!__is_start_with_prefix_char(*it)) {
                return std::distance(option_strings_.begin(), it);
            }
        }
        return -1;
    }

    bool __is_start_with_prefix_char(std::string_view name_or_flag) const {
        return prefix_char_ == name_or_flag[0];
    }

    template<typename... Args, size_t... Is>
    void __add_option_strings_from_tuple(std::tuple<Args...> const& tuple, std::index_sequence<Is...>) {
        (option_strings_.push_back(std::get<Is>(tuple)), ...);
    }

    std::vector<std::string> __get_values(std::vector<std::string>& arg_strings) {
        std::vector<std::string> ret;
        // for everything but parser, remainder arguments, strip out first "--"
        if (nargs_.get_type() != NArgs::Type::parser && nargs_.get_type() != NArgs::Type::remainder) {
            if (arg_strings.size() > 0 && arg_strings[0] == "--") {
                arg_strings.erase(arg_strings.begin());
            }
        }
        // optional argument produces a default when not present
        if ((arg_strings.size() == 0) && (nargs_.get_type() == NArgs::Type::optional)) {
            if (type_ == ArgumentType::optional_argument) {
                if (const_value_.size()) {
                    ret = const_value_;
                }
            }
            else {
                if (default_value_.size()) {
                    ret = default_value_;
                }
            }
        }
        // when nargs='*' on a positional, if there were no command-line args,
        // use the default if it is anything other than none
        else if ((arg_strings.size() == 0) && (nargs_.get_type() == NArgs::Type::zero_or_more) && (type_ != ArgumentType::optional_argument)) {
            if (default_value_.size()) {
                ret = default_value_;
            }
            else {
                ret = arg_strings;
            }
        }
        // single argument or optional argument produces a single value
        else if ((arg_strings.size() == 1) && ((nargs_.get_type() == NArgs::Type::none) || (nargs_.get_type() == NArgs::Type::optional))) {
            ret.push_back(arg_strings[0]);
        }
        // remainder, parser
        else if ((nargs_.get_type() == NArgs::Type::remainder) || (nargs_.get_type() == NArgs::Type::parser)) {
            for (auto& args : arg_strings) {
                ret.push_back(args);
            }
        }
        // suppress argument does not put anything in the namespace
        else if (nargs_.get_type() == NArgs::Type::suppress) {
            ret.push_back(Argument::SUPPRESS);
        }
        // all other types of nargs produce a list
        else {
            for (auto& args : arg_strings) {
                ret.push_back(args);
            }
        }
        
        // check value if it has choice values
        if (choices_.size()) {
            for (auto const& value : ret) {
                if (value.length() && std::find(choices_.cbegin(), choices_.cend(), value) == choices_.cend()) {
                    throw argument_error(*this, "invalid choice: " + value + " (choose from " + get_choice_str() + ")");
                }
                // if a type of this argument is subparser type, the first value is only checked
                if (nargs_.get_type() == NArgs::Type::parser) break;
            }
        }

        return ret;
    }
};

struct ArgumentHash {
    size_t operator()(Argument const& argument) const {
        return std::hash<size_t>()((size_t)&argument);
    }
};

struct ArgumentRefHash {
    size_t operator()(ArgumentRef const& argument) const {
        return ArgumentHash()(argument.get());
    }
};

struct ArgumentRefEqual {
    size_t operator()(ArgumentRef const& x, ArgumentRef const& y) const {
        return x.get() == y.get();
    }
};

struct ArgumentCRefHash {
    size_t operator()(ArgumentCRef const& argument) const {
        return ArgumentHash()(argument.get());
    }
};

struct ArgumentCRefEqual {
    size_t operator()(ArgumentCRef const& x, ArgumentCRef const& y) const {
        return x.get() == y.get();
    }
};

// ==============================================================================================================
// ArgumentGroup & MutuallyExclusiveGroup
// ==============================================================================================================
class ArgumentGroup
{
    friend class ArgumentParser;
    friend class HelpFormatter;

private:
    ArgumentParser& parser_;
    std::string title_;
    std::string description_;
    std::list<ArgumentRef> group_arguments_;

public:
    ArgumentGroup(ArgumentParser& parser, std::string_view title = "", std::string_view description = "")
    : parser_{parser}, title_{title}, description_{description} {}
    ArgumentGroup(ArgumentGroup const& other)
    : parser_{other.parser_}, title_{other.title_}, description_{other.description_}, group_arguments_{other.group_arguments_} {}
    ArgumentGroup(ArgumentGroup&& other) noexcept
    : parser_{other.parser_}, title_{std::move(other.title_)}, description_{std::move(other.description_)}, group_arguments_{std::move(other.group_arguments_)} {}

    template<typename... Args>
    Argument& add_argument(Args&&... args);
    Argument& add_argument(Argument& argument);
};

class MutuallyExclusiveGroup
{
    friend class ArgumentParser;
    friend class HelpFormatter;
private:
    ArgumentParser& parser_;
    bool required_;
    std::list<ArgumentRef> group_arguments_;

public:
    MutuallyExclusiveGroup(ArgumentParser& parser, bool const required = false)
    : parser_{parser}, required_{required} {}
    MutuallyExclusiveGroup(MutuallyExclusiveGroup const& other)
    : parser_{other.parser_}, required_{other.required_}, group_arguments_{other.group_arguments_} {}
    MutuallyExclusiveGroup(MutuallyExclusiveGroup&& other) noexcept
    : parser_{other.parser_}, required_{other.required_}, group_arguments_{std::move(other.group_arguments_)} {}

    template<typename... Args>
    Argument& add_argument(Args&&... args);
    Argument& add_argument(Argument& argument);
};

// ==============================================================================================================
// HelpFormatter
// ==============================================================================================================
class HelpFormatter
{
    struct Section
    {
        HelpFormatter* formatter_;
        std::shared_ptr<Section> parent_;
        std::string heading_;
        
        Section() : formatter_{nullptr}, parent_{nullptr}, heading_{} {}
        Section(HelpFormatter* formatter)
        : formatter_{formatter}, parent_{nullptr}, heading_{""} {}
        Section(HelpFormatter* formatter, std::shared_ptr<Section>& parent, std::string_view heading = "")
        : formatter_{formatter}, parent_{parent}, heading_{heading} {}
    };

private:
    std::string prog_{};
    size_t width_{};
    size_t const indent_increment_{};
    size_t max_help_position_{};
    size_t current_indent_{};
    size_t argument_max_length_{};

public:
    HelpFormatter(std::string_view prog, size_t const width = 0, size_t const indent_increment = 2, size_t const max_help_position = 24)
    : prog_{prog}, width_{width}, indent_increment_{indent_increment}, max_help_position_{max_help_position}
    , current_indent_{0} {
        if (width_ == 0) {
            width_ = __get_terminal_size() - 2;
        }
        max_help_position_ = std::min(max_help_position_, std::max(width_ - 20, indent_increment_ * 2));
    }

    size_t get_current_indent() const {
        return current_indent_;
    }

    std::string start_section(std::string_view heading) {
        std::string ret;
        // add the heading if the section was non-empty
        if (heading != Argument::SUPPRESS && heading.length()) {
            for (size_t i = 0; i < current_indent_; ++i) ret += ' ';
            ret += heading;
            ret += ":\n";
        }

        __indent();

        return ret;
    }

    void end_section() {
        __dedent();
    }

    std::string add_text(std::string_view text) const {
        if (text.length() != 0 && text != Argument::SUPPRESS) {
            std::string ret;
            const size_t text_width = std::max<size_t>(width_ - current_indent_, 11);
            const size_t len = text.length();
            std::string indent;
            for (size_t i = 0; i < current_indent_; ++i) indent += ' ';
            if (current_indent_ + len <= text_width) {
                ret += indent;
                ret += text;
            }
            else {
                const size_t remain = 10;
                size_t i = 0;
                while (i < len) {
                    ret += indent;
                    ret += text.substr(i, remain);
                    if (i + remain < len) {
                        ret += '\n';
                    }
                    i += remain;
                }
            }

            return ret;
        }

        return {};
    }

    std::string add_usage(std::string_view usage, std::list<Argument> const& arguments, std::list<MutuallyExclusiveGroup> const& groups, std::string_view prefix = "") const {
        if (usage != Argument::SUPPRESS) {
            std::string prefix_, usage_;
            if (prefix == std::string{}) {
                prefix_ = "usage: ";
            }

            // if usage is specific, use that
            if (usage != std::string{}) {
                usage_ = usage;
            }
            // if no optionals or positionals are available, usage is just prog
            else if (usage == std::string{} && arguments.size() == 0) {
                usage_ = prog_;
            }
            // if optionals and positionals are available, calculate usage
            else if (usage == std::string{}) {
                // split optionals from positionals
                std::vector<ArgumentCRef> all_arguments_;
                std::vector<ArgumentCRef> optional_arguments_;
                std::vector<ArgumentCRef> positional_arguments_;
                for (auto& argument : arguments) {
                    if (argument.get_type() == ArgumentType::optional_argument) {
                        all_arguments_.push_back(argument);
                        optional_arguments_.push_back(argument);
                    }
                }
                for (auto& argument : arguments) {
                    if (argument.get_type() != ArgumentType::optional_argument) {
                        all_arguments_.push_back(argument);
                        positional_arguments_.push_back(argument);
                    }
                }
                // build full usage string
                usage_ = prog_ + ' ' + __format_arguments_usage(all_arguments_, groups);

                // wrap the usage parts if it's too long
                const size_t text_width = width_ - current_indent_;
                if (prefix_.length() + usage_.length() > text_width) {
                    std::string opt_usage = __format_arguments_usage(optional_arguments_, groups);
                    std::string pos_usage = __format_arguments_usage(positional_arguments_, groups);
                    std::vector<std::string> opt_parts, pos_parts;

                    std::regex reg_pattern(R"==(\(.*?\)+(?=\s|$)|\[.*?\]+(?=\s|$)|\S+)==");
                    std::smatch match;
                    while (std::regex_search(opt_usage, match, reg_pattern)) {
                        opt_parts.push_back(match.str(0));
                        opt_usage = match.suffix().str();
                    }
                    while (std::regex_search(pos_usage, match, reg_pattern)) {
                        pos_parts.push_back(match.str(0));
                        pos_usage = match.suffix().str();
                    }

                    std::vector<std::string> lines;
                    // if prog is short, follow it with optionals or positionals
                    if ((prefix_.length() + prog_.length()) <= static_cast<size_t>(0.75 * text_width)) {
                        std::string indent;
                        for (size_t i = 0; i < prefix_.length() + prog_.length() + 1; ++i) indent += ' ';
                    }
                    // if prog is long, put it on its own line
                    else {
                        std::string indent;
                        for (size_t i = 0; i < prefix_.length(); ++i) indent += ' ';
                    }
                }
            }

            std::string ret = prefix_ + usage_ + "\n";
            return ret;
        }

        return {};
    }

    std::string add_arguments(std::list<ArgumentRef> const& arguments) {
        // formatting argument
        std::string ret;
        for (auto const& it : arguments) {
            ret += __format_argument(it.get());
        }

        return ret;
    }

    void set_argument_max_length(std::list<Argument> const& arguments) {
        __indent();
        // set argument maximum length
        for (auto const& argument : arguments) {
            if (argument.get_help() != Argument::SUPPRESS) {
                std::vector<std::string> invocations;
                // find all invocations
                invocations.push_back(__format_argument_invocations(argument));
                if (argument.get_type() == ArgumentType::subcommand_argument) {
                    for (auto const& subargument : argument.subarguments) {
                        invocations.push_back(__format_argument_invocations(subargument));
                    }
                }

                // update the maximum item length
                const size_t invocation_length = std::max_element(
                    invocations.begin(), invocations.end(), [](std::string const& x, std::string const& y) {
                        return x.length() < y.length();
                    })->length();
                argument_max_length_ = std::max(argument_max_length_, invocation_length + current_indent_);
            }
        }
        __dedent();
    }

private:
    size_t __get_terminal_size() const {
        #if defined(__linux__) || defined(__APPLE__)
            struct winsize w;
            ioctl(fileno(stdout), TIOCGWINSZ, &w);
            return static_cast<size_t>(w.ws_col);
        #elif defined(_WIN32)
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
            width = (int)(csbi.srWindow.Right-csbi.srWindow.Left+1);
            height = (int)(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
        #endif
    }

    void __indent() {
        current_indent_ += indent_increment_;
    }

    void __dedent() {
        current_indent_ -= indent_increment_;
    }

    std::string __format_arguments_usage(std::vector<ArgumentCRef> const& arguments, std::list<MutuallyExclusiveGroup> const& groups) const {
        std::unordered_set<ArgumentCRef,
                           ArgumentCRefHash, ArgumentCRefEqual> group_arguments;
        std::map<int, std::string> inserts;
        for (auto const& group : groups) {
            if (group.group_arguments_.size() == 0) {
                throw std::logic_error("empty group");
            }

            int start = -1;
            for (int i = 0; i < arguments.size(); ++i) {
                if (group.group_arguments_.begin()->get() == arguments[i].get()) {
                    start = i;
                    break;
                }
            }
            if (start == -1) continue;

            int end = start + group.group_arguments_.size();
            if (end > arguments.size()) continue;

            bool same = true;
            auto group_arg_it = group.group_arguments_.begin();
            for (int i = start; i < end; ++i, ++group_arg_it) {
                if (group_arg_it->get() != arguments[i].get()) {
                    same = false;
                    break;
                }
            }

            if (same) {
                int suppressed_arguments_count = 0;
                for (auto& group_arg : group.group_arguments_) {
                    group_arguments.insert(group_arg);
                    if (group_arg.get().get_help() == Argument::SUPPRESS) {
                        ++suppressed_arguments_count;
                    }
                }

                int exposed_arguments_count = group.group_arguments_.size() - suppressed_arguments_count;
                if (!group.required_) {
                    if (inserts.find(start) != inserts.end()) {
                        inserts[start] += " [";
                    }
                    else {
                        inserts.insert_or_assign(start, "[");
                    }
                    if (inserts.find(end) != inserts.end()) {
                        inserts[end] += "]";
                    }
                    else {
                        inserts.insert_or_assign(end, "]");
                    }
                }
                else if (exposed_arguments_count > 1) {
                    if (inserts.find(start) != inserts.end()) {
                        inserts[start] += " (";
                    }
                    else {
                        inserts.insert_or_assign(start, "(");
                    }
                    if (inserts.find(end) != inserts.end()) {
                        inserts[end] += ")";
                    }
                    else {
                        inserts.insert_or_assign(end, ")");
                    }
                }
                for (int i = start + 1; i < end; ++i) {
                    inserts.insert_or_assign(i, "|");
                }
            }
        }

        // collect all arguments format strings
        std::vector<std::string> parts;
        for (int i = 0; i < arguments.size(); ++i) {
            auto& ref_argument = arguments[i];
            auto& argument = arguments[i].get();
            // suppressed arguments are marked with None
            // remove | seperators for suppressed arguments
            if (argument.get_help() == Argument::SUPPRESS) {
                parts.push_back(std::string{});
            }
            else if (argument.get_type() != ArgumentType::optional_argument) {
                // produce all arg strings
                std::string part = __format_args(argument, __get_default_metavar_for_positional(argument));

                // if it's in a group, strip the outer []
                if (group_arguments.find(ref_argument) != group_arguments.end()) {
                    if (part[0] == '[' && part[part.length() - 1] == ']') {
                        part = part.substr(1, part.length() - 2);
                    }
                }
                parts.push_back(part);
            }
            else {
                std::string part;
                // produce the first way to invoke the option in brackets
                std::string const& option_string = argument.option_strings_[0];

                // if the optional doesn't take a value, format is: -s or --long
                if (argument.get_nargs().get_type() == NArgs::Type::integer && argument.get_nargs().get_nargs() == 0) {
                    part = option_string;
                }
                // if the optional takes a value, format is: -s ARGS or --long ARGS
                else {
                    std::string args_string = __format_args(argument, __get_default_metavar_for_optional(argument));
                    part = option_string + ' ' + args_string;
                }

                // make it look optional if it's not required or in a group
                if (!argument.get_required() && (group_arguments.find(ref_argument) == group_arguments.end())) {
                    part = '[' + part + ']';
                }
                parts.push_back(part);
            }
        }
        
        // insert things at the necessary indices
        for (auto p = inserts.rbegin(); p != inserts.rend(); ++p) {
            parts.insert(parts.begin() + p->first, p->second);
        }

        // join all the items with spaces
        std::string ret = utils::join(parts, " ");

        // clean up separators for mutually exclusive groups
        ret = std::regex_replace(ret, std::regex(R"==(([\[(]) )=="), "$1");
        ret = std::regex_replace(ret, std::regex(R"==( ([\])]))=="), "$1");
        ret = std::regex_replace(ret, std::regex(R"==([\[(] *[\])])=="), "");
        ret = utils::trim(ret);

        return ret;
    }

    std::string __get_default_metavar_for_positional(Argument const& argument) const {
        return argument.get_dest();
    }

    std::string __get_default_metavar_for_optional(Argument const& argument) const {
        return utils::to_upper(argument.get_dest());
    }

    std::string __metavar_formatter(Argument const& argument, std::string const& default_metavar) const {
        std::string metavar;
        if (argument.get_metavar() != std::string{}) {
            metavar = argument.get_metavar();
        }
        else if (argument.get_choices().size()) {
            auto const& choices = argument.get_choices();
            metavar = '{';
            metavar += utils::join(choices, ",");
            metavar += '}';
        }
        else {
            metavar = default_metavar;
        }

        return metavar;
    }

    std::string __format_args(Argument const& argument, std::string const& default_metavar) const {
        std::string metavar = __metavar_formatter(argument, default_metavar);

        std::ostringstream oss;
        if (argument.get_nargs().get_type() == NArgs::Type::none) {
            oss << metavar;
        }
        else if (argument.get_nargs().get_type() == NArgs::Type::optional) {
            oss << '[' << metavar << ']';
        }
        else if (argument.get_nargs().get_type() == NArgs::Type::zero_or_more) {
            oss << '[' << metavar << " ...]";
        }
        else if (argument.get_nargs().get_type() == NArgs::Type::one_or_more) {
            oss << metavar << " [" << metavar << " ...]";
        }
        else if (argument.get_nargs().get_type() == NArgs::Type::remainder) {
            oss << "...";
        }
        else if (argument.get_nargs().get_type() == NArgs::Type::parser) {
            oss << metavar << " ...";
        }
        else if (argument.get_nargs().get_type() == NArgs::Type::suppress) {
            oss << "";
        }
        else {
            size_t const nargs = argument.get_nargs().get_nargs();
            for (size_t i = 0; i < nargs; ++i) {
                oss << metavar;
                if (i < nargs - 1) {
                    oss << ' ';
                }
            }
        }
        return oss.str();
    }

    std::string __format_argument_invocations(Argument const& argument) const {
        if (argument.option_strings_.size() == 0) {
            return __metavar_formatter(argument, argument.get_dest());
        }
        else {
            std::vector<std::string> parts;

            // if the optional doesn't take a value, format is: -s, --long
            if (argument.get_nargs().get_type() == NArgs::Type::integer && argument.get_nargs().get_nargs() == 0) {
                parts = argument.option_strings_;
            }
            // if the optional take a value, format is: -s ARGS, --long ARGS
            else {
                const std::string args_string = __format_args(argument, __get_default_metavar_for_optional(argument));
                for (auto const& option_string : argument.option_strings_) {
                    parts.push_back(option_string + ' ' + args_string);
                }
            }

            return utils::join(parts, ", ");
        }
    }

    std::string __format_argument(Argument const& argument) {
        if (argument.get_help() == Argument::SUPPRESS) {
            return std::string{};
        }
        // determine the required width and the entry label
        const size_t help_position = std::min(argument_max_length_ + 2, max_help_position_);
        const size_t help_width = std::max<size_t>(width_ - help_position, 11);
        const size_t argument_width = help_position - current_indent_ - 2;
        std::string argument_header = __format_argument_invocations(argument);

        size_t indent_first;
        std::ostringstream oss;
        for (size_t i = 0; i < current_indent_; ++i) oss << ' ';
        // no help: start on same line and add a final newline
        if (argument.get_help() == std::string{}) {
            oss << argument_header << '\n';
        }
        // short argument name: start on the same line and pad two spaces
        else if (argument_header.length() <= argument_width) {
            oss << std::setw(argument_width) << std::left << argument_header << "  ";
            indent_first = 0;
        }
        // long argument name: start on the next line
        else {
            oss << argument_header << '\n';
            indent_first = help_position;
        }
        
        std::vector<std::string> parts{oss.str()};
        // if there was help for the argument, add lines of help text
        if (argument.get_help().length()) {
            std::string const help_text = argument.get_help();
            std::vector<std::string> help_lines;
            for (size_t i = 0; i < help_text.length(); i += help_width) {
                help_lines.push_back(help_text.substr(i, help_width));
            }
            for (auto const& line : help_lines) {
                std::string part;
                for (size_t i = 0; i < indent_first; ++i) part += ' ';
                part += line;
                part += '\n';
                parts.push_back(part);
                indent_first = help_position;
            }
        }
        // or add a newline if the decription doesn't end with one
        else if (parts[0][parts[0].length() - 1] != '\n') {
            parts.push_back("\n");
        }

        // if there are any sub-arguments, add their help as well
        if (argument.get_type() == ArgumentType::subcommand_argument) {            
            __indent();
            for (auto const& subargument : argument.subarguments) {
                parts.push_back(__format_argument(subargument));
            }
            __dedent();
        }

        oss.str("");
        oss.clear();
        for (size_t i = 0, size = parts.size(); i < size; ++i) {
            oss << parts[i];
        }

        return oss.str();
    }
};


// ==============================================================================================================
// ArgumentParser
// ==============================================================================================================
class ArgumentParser
{
    friend class Argument;
    friend class ArgumentGroup;
    friend class MutuallyExclusiveGroup;

    using ArgumentRefIter = std::list<ArgumentRef>::iterator;
    using SubParserIter = std::list<ArgumentParser>::iterator;

private:
    std::string prog_name_;
    std::string usage_;
    std::string description_;
    std::string epilog_;
    char prefix_char_;
    bool allow_abbrev_;
    bool help_;
    bool exit_on_error_;

    std::list<Argument> args_list_;
    std::unordered_map<std::string_view, ArgumentRefIter> optional_args_map_;
    ArgumentRefIter const end_iterator;

    std::list<ArgumentGroup> argument_groups_;
    ArgumentGroup& positional_groups_;
    ArgumentGroup& optional_groups_;
    std::list<MutuallyExclusiveGroup> mutually_exclusive_groups_;

    std::optional<std::reference_wrapper<ArgumentGroup>> subparser_;
    std::list<ArgumentParser> subparsers_list_;
    std::map<std::string, SubParserIter> subparsers_map_;

    bool has_negative_number_options_{false};

public:
    explicit ArgumentParser(std::string_view const prog_name = "", bool const help = true, char const prefix_char = '-', bool const exit_on_error = true)
    : prog_name_{prog_name}, usage_{}, description_{}, epilog_{}
    , prefix_char_{prefix_char}, allow_abbrev_{true}, help_{help}, exit_on_error_{exit_on_error}
    , args_list_{}, optional_args_map_{}, end_iterator{}, argument_groups_{}
    , positional_groups_{add_argument_group("positional arguments")}
    , optional_groups_{add_argument_group("options")}
    , mutually_exclusive_groups_{}
    , subparser_{std::nullopt}, subparsers_map_{}
    , has_negative_number_options_{false} {
        Argument::prefix_char_ = prefix_char_;
        if (help_) {
            __add_help_argument();
        }
    }

    ArgumentParser& set_usage(std::string_view usage) {
        usage_ = usage;
        return *this;
    }
    std::string get_usage() const {
        return usage_;
    }

    ArgumentParser& set_description(std::string_view description) {
        description_ = description;
        return *this;
    }
    std::string get_description() const {
        return description_;
    }

    ArgumentParser& set_epilog(std::string_view epilog) {
        epilog_ = epilog;
        return *this;
    }
    std::string get_epilog() const {
        return epilog_;
    }

    ArgumentParser& set_abbrev(bool const abbrev) {
        allow_abbrev_ = abbrev;
        return *this;
    }

    auto& get_subparser_map() {
        return subparsers_map_;
    }

    template<typename... Args>
    Argument& add_argument(Args&&... args) {
        Argument& ret = __add_argument(std::forward<Args>(args)...);

        if (ret.get_type() == ArgumentType::positional_argument) {
            positional_groups_.add_argument(ret);
        }
        else if (ret.get_type() == ArgumentType::optional_argument) {
            // add optional argument to arguments map
            __add_optional_argument(ret);
        }
        else {
            throw argument_error(ret, "invalid argument type");
        }

        return ret;
    }

    ArgumentGroup& add_argument_group(std::string_view title = "", std::string_view description = "") {
        auto& ret = argument_groups_.emplace_back(*this, title, description);
        return ret;
    }

    MutuallyExclusiveGroup& add_mutually_exclusive_group(bool const required = false) {
        auto& ret = mutually_exclusive_groups_.emplace_back(*this, required);
        return ret;
    }

    Argument& add_subparsers(std::string_view title = "", std::string_view description = "") {
        if (subparser_.has_value()) {
            __error(prog_name_ + " cannot have multiple subparser arguments");
        }

        if (title.length() || description.length()) {
            if (title.length()) {
                subparser_ = add_argument_group(title, description);
            }
            else {
                subparser_ = add_argument_group("subcommands", description);
            }
        }
        else {
            subparser_ = positional_groups_;
        }

        Argument argument;
        argument.set_dest(Argument::SUPPRESS)
        .set_required(false)
        .__set_action(
            [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
                std::string const parser_name = values[0];
                std::vector<std::string> arg_strings(values.size() - 1);
                for (size_t i = 0; i < arg_strings.size(); ++i) {
                    arg_strings[i] = values[i + 1];
                }
                // set the parser name if requested
                if (argument.get_dest() != Argument::SUPPRESS) {
                    args.set_value(argument.get_dest(), parser_name);
                }
                // select the subparser
                auto& subparser_map = parser->get_subparser_map();
                if (subparser_map.find(parser_name) == subparser_map.end()) {
                    throw argument_error(argument, "unknown parser " + parser_name + " (choices: " + argument.get_choice_str() + ")");
                }
                auto& subparser = subparser_map[parser_name];

                auto sub_args = subparser->parse_args(arg_strings);
                auto const& sub_args_values = sub_args.get_values();
                for (auto& pair : sub_args_values) {
                    args[pair.first] = pair.second;
                }
            }
        )
        .__set_validation(
            [](Argument const&) {
                // no valid conditionvalidation_
            }
        );
        argument.nargs_.set_type_parser();
        argument.__set_type(ArgumentType::subcommand_argument);
        argument.parent_ = this;

        args_list_.push_back(argument);
        subparser_.value().get().add_argument(args_list_.back());

        return args_list_.back();
    }

    Namespace parse_args(int argc, char const *const *const argv) {
        // set program name if no name exists
        if (!prog_name_.length()) {
            prog_name_ = std::string(argv[0]);
            int last_slash_or_back_slash_idx = prog_name_.find_last_of('/');
            if (last_slash_or_back_slash_idx == std::string::npos) {
                last_slash_or_back_slash_idx = prog_name_.find_last_of('\\');
            }
            if (last_slash_or_back_slash_idx != std::string::npos) {
                prog_name_ = prog_name_.substr(last_slash_or_back_slash_idx + 1);
            }
        }
        // parsing argument list
        std::vector<std::string> arg_strings;
        for (int i = 1; i < argc; ++i) {
            arg_strings.push_back(std::string(argv[i]));
        }

        return parse_args(arg_strings);
    }

    Namespace parse_args(std::vector<std::string> const& arg_strings) {
        Namespace ret;
        auto extras = parse_known_args(ret, arg_strings);

        if (extras.size()) {
            std::ostringstream oss;
            oss << "unrecognized arguments: ";
            oss << utils::join(extras, " ");
            __error(oss.str());
        }

        return ret;
    }

    std::vector<std::string> parse_known_args(Namespace& ret, std::vector<std::string> const& arg_strings) {
        // add default values that aren't present
        for (auto& arg : args_list_) {
            arg.initialize();
            if (arg.get_dest() != Argument::SUPPRESS) {
                auto const& default_values = arg.get_default();
                if (default_values.size()) {
                    if (default_values.size() > 1 || default_values[0] != Argument::SUPPRESS) {
                        ret.set_values(arg.get_dest(), default_values);
                    }
                }
            }
        }

        if (exit_on_error_) {
            try {
                return __parse_known_args(ret, arg_strings);
            }
            catch (std::exception const& e) {
                __error(e.what());
            }
        }
        else {
            return __parse_known_args(ret, arg_strings);
        }
    }

    void print_help() const {
        std::cout << __format_help();
    }

    void print_usage() const {
        std::cout << __format_usage();
    }

    void exit(std::string_view message = "", int const status = 0) const {
        __exit(message, status);
    }

private:
    template<typename... Args>
    Argument& __add_argument(Args&&... args) {
        // create argument
        Argument argument;
        constexpr size_t N = sizeof...(Args);
        constexpr size_t num_of_flags = (traits::is_convertible_to_str<traits::last_type_t<Args...>>::value ? N : N - 1);

        if constexpr (num_of_flags == (N - 1)) {
            static_assert(traits::is_action_class_v<traits::last_type_t<Args...>>, "action object must have initialize(), get_action(), and get_valid() member functions");
            
            const auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
            argument.__add_name_or_flags(std::get<N-1>(tuple), std::forward<Args>(args)...);
        }
        else {
            argument.__add_name_or_flags(actions::StoreAction(), std::forward<Args>(args)...);
        }

        // check conflict
        __check_conflict(argument);
        
        // add to argument list
        args_list_.push_back(argument);

        // set the flag if any option strings look like negative numbers
        for (std::string const& option_string : argument.option_strings_) {
            if (__match_negative_number(option_string)) {
                has_negative_number_options_ = true;
            }
        }

        return args_list_.back();
    }

    void __add_optional_argument(Argument& argument) {
        optional_groups_.group_arguments_.push_back(argument);
        auto it = --optional_groups_.group_arguments_.end();
        for (std::string_view option_string : argument.option_strings_) {
            optional_args_map_.insert_or_assign(option_string, it);
        }
    }

    void __add_optional_argument_map(ArgumentRefIter it) {
        for (std::string_view option_string : it->get().option_strings_) {
            optional_args_map_.insert_or_assign(option_string, it);
        }
    }

    void __add_help_argument() {
        std::string short_arg = std::string(1, prefix_char_) + std::string("h");
        std::string long_arg = std::string(1, prefix_char_) + std::string(1, prefix_char_) + "help";
        add_argument(short_arg, long_arg, actions::HelpAction())
        .set_help("show this help message and exit");
    }

    std::vector<std::string> __parse_known_args(Namespace& ret, std::vector<std::string> const& arg_strings) {
        // mutually exclusive arguments to the other arguments
        std::unordered_map<ArgumentRef, std::list<ArgumentRef>, ArgumentRefHash, ArgumentRefEqual> argument_conflicts;
        for (auto& mutex_group : mutually_exclusive_groups_) {
            auto& group_arguments = mutex_group.group_arguments_;
            for (auto& it : group_arguments) {
                auto& conflict = argument_conflicts[it.get()];
                for (auto& other : group_arguments) {
                    if (it.get() != other.get()) {
                        conflict.push_back(other);
                    }
                }
            }
        }

        // find all option indices
        std::unordered_map<int, std::tuple<std::string, ArgumentRefIter, std::string>> arg_string_indices; // [arg_string, argument object, explicit_arg_value]
        std::string arg_string_pattern;
        
        // and determine the arg_string_pattern
        // which has an 'O' if there is an option at index,
        // an 'A' if there is an argument, or a '-' if there is a '--'
        for (size_t i = 0; i < arg_strings.size(); ++i) {
            if (arg_strings[i] == "--") {
                // all args after -- are non-options (positional arguments)
                arg_string_pattern.push_back('-');
                while (++i < arg_strings.size()) {
                    arg_string_pattern.push_back('A');
                }
            }
            else {
                // otherwise, add the arg to the arg strings and note the index if it was an option
                if (__parse_optional(arg_string_indices, arg_strings[i], i)) {
                    arg_string_pattern.push_back('O');
                }
                else {
                    arg_string_pattern.push_back('A');
                }
            }
        }

        auto positional_arguments = __get_positional_args();
        ArgumentRefIter positionals = positional_arguments.begin();
        std::unordered_set<ArgumentRef, ArgumentRefHash, ArgumentRefEqual> seen_args;
        std::unordered_set<ArgumentRef, ArgumentRefHash, ArgumentRefEqual> seen_non_default_args;

        // consume positionals and optionals alternately, untill we have passed the last option string
        std::vector<std::string> extras;
        int start_index = 0;
        int max_option_string_indices = [&arg_string_indices]() {
            int ret = -1;
            for (auto const& m : arg_string_indices) {
                ret = std::max(ret, m.first);
            }
            return ret;
        }();

        while (start_index <= max_option_string_indices) {
            int next_option_string_index = [&arg_string_indices](int start_index) {
                int ret = std::numeric_limits<int>::max();
                for (auto const& m : arg_string_indices) {
                    if (m.first >= start_index) {
                        ret = std::min(ret, m.first);
                    }
                }
                return ret;
            }(start_index);
            // consume any positionals precdeding the next option
            if (start_index != next_option_string_index) {
                int positionals_end_index = __consume_positionals(ret,
                                                                  positionals,
                                                                  positional_arguments.end(),
                                                                  start_index,
                                                                  seen_args,
                                                                  seen_non_default_args,
                                                                  argument_conflicts,
                                                                  arg_strings, arg_string_pattern);

                // only try to parse the next optional if we didn't consume
                // the option string during the positionals parsing
                if (positionals_end_index > start_index) {
                    start_index = positionals_end_index;
                    continue;
                }
                else {
                    start_index = positionals_end_index;
                }
            }
            // if we consumed all the positionals we could and we're not
            // at the index of an option string, there extra arguments
            auto is_not_contain_opt_args = [&arg_string_indices](int start_index) {
                for (auto& m : arg_string_indices) {
                    if (start_index == m.first) {
                        return false;
                    }
                }
                return true;
            };
            if (is_not_contain_opt_args(start_index)) {
                for (size_t i = start_index; i < next_option_string_index; ++i) {
                    extras.push_back(arg_strings[i]);
                }
                start_index = next_option_string_index;
            }

            // consume the next optional and any arguments for it
            start_index = __consume_optionals(ret,
                                              arg_string_indices,
                                              extras,
                                              start_index,
                                              seen_args,
                                              seen_non_default_args,
                                              argument_conflicts,
                                              arg_strings, arg_string_pattern);
        }

        // consume any positional following the last optional
        auto stop_index = __consume_positionals(ret,
                                                positionals,
                                                positional_arguments.end(),
                                                start_index,
                                                seen_args,
                                                seen_non_default_args,
                                                argument_conflicts,
                                                arg_strings, arg_string_pattern);

        // extra arguments when we didn't consume all the argument strings
        for (size_t i = stop_index; i < arg_strings.size(); ++i) {
            extras.push_back(arg_strings[i]);
        }

        // make sure all required arguments were present
        std::vector<std::string> required_args;
        for (auto& arg : args_list_) {
            bool seen = false;
            for (auto& seen_arg : seen_args) {
                if (arg == seen_arg) {
                    seen = true;
                    break;
                }
            }

            if (!seen && arg.get_required()) {
                if (arg.get_required()) {
                    required_args.push_back(arg.get_argument_name());
                }
                // else {
                //     // [TODO]: convert action default now instead of doing it ...
                //     if (arg.get_default().has_value()) {
                //         auto& value = ret[arg.get_dest()];
                //         if (value.size() && arg.get_default().value() == value[0]) {

                //         }
                //     }
                // }
            }
        }
        if (required_args.size()) {
            std::ostringstream oss;
            oss << "the following arguments are required: ";
            oss << utils::join(required_args, ", ");
            throw std::invalid_argument(oss.str());
        }

        // make sure all required groups had one option present
        for (auto const& group : mutually_exclusive_groups_) {
            if (group.required_) {
                bool used = false;
                std::vector<std::string> names;
                for (auto const& argument : group.group_arguments_) {
                    if (argument.get().get_help() != Argument::SUPPRESS) {
                        names.push_back(argument.get().get_argument_name());
                    }
                    if (seen_non_default_args.find(argument) != seen_non_default_args.end()) {
                        used = true;
                    }
                }
                // if no arguments were used, report the error
                if (!used) {
                    std::string name = utils::join(names, " ");
                    throw std::logic_error("one of the arguments " + name + " is required");
                }
            }
        }

        return extras;
    }

    bool __parse_optional(std::unordered_map<int, std::tuple<std::string, ArgumentRefIter, std::string>>& arg_string_indices,
                          std::string_view arg_string,
                          int const idx) {
        // if it's an empty string, it was meant to be a positional
        if (arg_string.length() == 0) return false;
        // if it doesn't start with a prefix, it was meant to be a positional
        if (arg_string[0] != prefix_char_) return false;

        // if the option string is present in the parser, it was meant to be a optional
        for (auto& arg_map : optional_args_map_) {
            if (arg_string == arg_map.first) {
                arg_string_indices[idx] = std::make_tuple(arg_string, arg_map.second, std::string(""));
                return true;
            }
        }
        // if it's just a single character, it was meant to be a positional
        if (arg_string.length() == 1) return false;

        // if the option string before the "=" is present, it was meant to be a optional
        auto assign_idx = arg_string.find('=');
        if (assign_idx != std::string::npos) {
            auto opt_string = arg_string.substr(0, assign_idx);
            auto explicit_arg = arg_string.substr(assign_idx + 1);

            for (auto& arg_map : optional_args_map_) {
                if (opt_string == arg_map.first) {
                    arg_string_indices[idx] = std::make_tuple(opt_string, arg_map.second, explicit_arg);
                    return true;
                }
            }
        }

        // search through all possible prefixes of the option string and all arguments
        // in the parser for possible interpretations
        std::vector<std::tuple<std::string, ArgumentRefIter, std::string>> option_tuples;
        // option strings starting with two prefix character are only split at the '='
        if (arg_string.length() >= 2 && arg_string[0] == prefix_char_ && arg_string[1] == prefix_char_) {
            if (allow_abbrev_) {
                std::string option_prefix, explicit_arg;
                if (auto pos = arg_string.find('='); pos != std::string::npos) {
                    option_prefix = arg_string.substr(0, pos);
                    explicit_arg = arg_string.substr(pos + 1);
                }
                else {
                    option_prefix = arg_string;
                }
                for (auto& opt_pair : optional_args_map_) {
                    if (opt_pair.first.rfind(option_prefix) == 0) {
                        option_tuples.emplace_back(opt_pair.first, opt_pair.second, explicit_arg);
                    }
                }
            }
        }
        // single character options can be concatenated with their arguments but multiple character
        // options always have to have their argument separate
        else if (arg_string.length() >= 2 && arg_string[0] == prefix_char_ && arg_string[1] != prefix_char_) {
            auto short_option_prefix = arg_string.substr(0, 2);
            auto short_explicit_arg = arg_string.substr(2);

            for (auto& opt_pair : optional_args_map_) {
                if (opt_pair.first == short_option_prefix) {
                    option_tuples.emplace_back(opt_pair.first, opt_pair.second, short_explicit_arg);
                }
                else if (opt_pair.first.rfind(arg_string) == 0) {
                    option_tuples.emplace_back(opt_pair.first, opt_pair.second, "");
                }
            }
        }
        else {
            throw std::invalid_argument("unexpected option string: " + std::string{arg_string});
        }
        // if multiple arguments match, the option string was ambigous
        if (option_tuples.size() > 1) {
            std::ostringstream oss;
            oss << "ambiguous option: " << arg_string << " could match ";
            for (size_t i = 0, size = option_tuples.size(); i < size; ++i) {
                oss << std::get<0>(option_tuples[i]);
                if (i < size - 1) {
                    oss << ", ";
                }
            }
            throw std::invalid_argument(oss.str());
        }
        else if (option_tuples.size() == 1) {
            arg_string_indices[idx] = option_tuples[0];
            return true;
        }

        // if it was not found as an option, but it looks like a negative number, it was meant to be a positional
        // unless there are negative-number-like options
        if (__match_negative_number(std::string(arg_string))) {
            if (!has_negative_number_options_) {
                return false;
            }
        }

        // if it contains a space, it was meant to be a positional
        if (arg_string.find(' ') != std::string::npos) {
            return false;
        }

        // it was meant to be an optional but there is no such option
        // in this parser (though it might be a valid option in a subparser)
        arg_string_indices[idx] = std::make_tuple(std::string(""), end_iterator, std::string(""));
        return true;
    };

    int __match_argument(ArgumentRefIter it, std::string const& arg_strings_pattern) {
        // match the pattern for this argument to the argument strings
        bool optional = (it->get().get_type() == ArgumentType::optional_argument);
        std::string nargs_pattern_str = it->get().nargs_.get_nargs_pattern(optional);

        std::regex nargs_pattern(nargs_pattern_str);
        std::smatch match;
        if (!std::regex_search(arg_strings_pattern, match, nargs_pattern)) {
            // throw exception if we weren't able to find a match
            std::stringstream ss;
            const auto nargs_type = it->get().nargs_.get_type();
            if (nargs_type == NArgs::Type::none) {
                ss << "expected one argument";
            }
            else if (nargs_type == NArgs::Type::optional) {
                ss << "expected at most one argument";
            }
            else if (nargs_type == NArgs::Type::one_or_more) {
                ss << "expected at least one argument";
            }
            else {
                ss << "expected " << it->get().nargs_.get_nargs() << " argument(s)";
            }
            throw argument_error(it->get(), ss.str());
        }
        
        return match.str(1).length();
    }

    std::vector<int> __match_arguments_partial(ArgumentRefIter first, ArgumentRefIter last, std::string const& arg_strings_pattern) {
        // progressively shorten the positional arguments list by slicing off the
        // final arguments until we find a match
        std::vector<int> ret;
        int len = std::distance(first, last);

        for (size_t i = 0; i < len; ++i, --last) {
            std::string nargs_pattern_str;
            for (auto it = first; it != last; ++it) {
                nargs_pattern_str += it->get().nargs_.get_nargs_pattern(false);
            }
            std::regex nargs_pattern(nargs_pattern_str);
            std::smatch match;
            if (std::regex_search(arg_strings_pattern, match, nargs_pattern)) {
                ret.push_back(match.str(1).length());
                break;
            }
        }
        // return the list of argument string counts
        return ret;
    }

    int __consume_optionals(Namespace& ret,
                               std::unordered_map<int, std::tuple<std::string, ArgumentRefIter, std::string>>& arg_string_indices,
                               std::vector<std::string>& extras,
                               int start_index,
                               std::unordered_set<ArgumentRef, ArgumentRefHash, ArgumentRefEqual>& seen_args,
                               std::unordered_set<ArgumentRef, ArgumentRefHash, ArgumentRefEqual>& seen_non_default_args,
                               std::unordered_map<ArgumentRef, std::list<ArgumentRef>, ArgumentRefHash, ArgumentRefEqual> const& argument_conflicts,
                               std::vector<std::string> const& arg_strings,
                               std::string const& arg_string_pattern) {
        auto [arg_string, optional, explicit_arg] = arg_string_indices[start_index];
        auto stop_index = start_index;
        std::vector<std::pair<ArgumentRefIter, std::vector<std::string>>> arg_tuples;
        while (1) {
            if (optional == end_iterator) {
                // if we found no optional argument, skip it
                extras.push_back(arg_strings[start_index]);
                return start_index + 1;
            }

            if (explicit_arg.length()) {
                auto arg_count = __match_argument(optional, "A");

                if (arg_count == 0 && arg_string[1] != prefix_char_ && explicit_arg != std::string{}) {
                    // if the argument is a single-dash option and takes no arguments,
                    // try to parse more single-dash options out of the tail of the option string
                    arg_tuples.push_back(std::make_tuple(optional, std::vector<std::string>{}));
                    std::string option_string{arg_string[0]};
                    option_string += explicit_arg[0];

                    bool found = false;
                    for (auto& opt_pair : optional_args_map_) {
                        if (opt_pair.first == option_string) {
                            optional = opt_pair.second;
                            explicit_arg = explicit_arg.substr(1);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        throw argument_error(*optional, "ignored explicit argument " + explicit_arg);
                    }
                }
                else if (arg_count == 1) {
                    stop_index = start_index + 1;
                    std::vector<std::string> args{explicit_arg};
                    arg_tuples.push_back(std::make_pair(optional, args));
                    break;
                }
                else {
                    throw argument_error(*optional, "ignored explicit argument " + explicit_arg);
                }
            }
            else {
                // if there is no explicit argument
                start_index += 1;
                auto selected_patterns = arg_string_pattern.substr(start_index);
                auto arg_count = __match_argument(optional, selected_patterns);
                stop_index = start_index + arg_count;
                std::vector<std::string> args(arg_strings.begin() + start_index, arg_strings.begin() + stop_index);
                arg_tuples.push_back(std::make_pair(optional, args));
                break;
            }
        }

        if (arg_tuples.size()) {
            for (auto& [optional, args] : arg_tuples) {
                __take_argument(ret, seen_args, seen_non_default_args, argument_conflicts, optional, args);
            }
        }
        else {
            throw std::logic_error("invalid condition");
        }

        return stop_index;
    }

    size_t __consume_positionals(Namespace& ret,
                                 ArgumentRefIter& positionals, ArgumentRefIter last,
                                 size_t start_index,
                                 std::unordered_set<ArgumentRef, ArgumentRefHash, ArgumentRefEqual>& seen_args,
                                 std::unordered_set<ArgumentRef, ArgumentRefHash, ArgumentRefEqual>& seen_non_default_args,
                                 std::unordered_map<ArgumentRef, std::list<ArgumentRef>, ArgumentRefHash, ArgumentRefEqual> const& argument_conflicts,
                                 std::vector<std::string> const& arg_strings,
                                 std::string const& arg_string_pattern) {
        std::string selected_pattern = arg_string_pattern.substr(start_index);
        auto arg_counts = __match_arguments_partial(positionals, last, selected_pattern);

        // slice off the appropriate arg strings for each positional
        // and the positional and its args to the list
        for (size_t i = 0; i < arg_counts.size(); ++i, ++positionals) {
            std::vector<std::string> args(arg_strings.begin() + start_index, arg_strings.begin() + start_index + arg_counts[i]);
            start_index += arg_counts[i];
            __take_argument(ret, seen_args, seen_non_default_args, argument_conflicts, positionals, args);
        }

        return start_index;
    };

    void __take_argument(Namespace& ret,
                         std::unordered_set<ArgumentRef, ArgumentRefHash, ArgumentRefEqual>& seen_args,
                         std::unordered_set<ArgumentRef, ArgumentRefHash, ArgumentRefEqual>& seen_non_default_args,
                         std::unordered_map<ArgumentRef, std::list<ArgumentRef>, ArgumentRefHash, ArgumentRefEqual> const& argument_conflicts,
                         ArgumentRefIter const& arg_it,
                         std::vector<std::string>& args) {
        Argument& argument = arg_it->get();
        seen_args.insert(argument);
        auto argument_values = argument.__get_values(args);

        // [TODO] seen_non_default_arg logic (due to comparison between addresses of 2 objects)
        // error if this argument is not allowed with other previsouly seen arguments,
        // assuming that arguments that use the default value don't really count as "present"
        seen_non_default_args.insert(argument);
        if (argument_conflicts.find(argument) != argument_conflicts.end()) {
            auto const& conflicts_list = argument_conflicts.at(argument);
            for (auto const& conflict : conflicts_list) {
                for (auto const& seen_arg : seen_args) {
                    if (conflict.get() == seen_arg.get()) {
                        throw argument_error(argument, "not allowed with argument " + seen_arg.get().get_argument_name());
                    }
                }
            }
        }

        // take the argument if didn't receive a SUPPRESS value
        if (argument_values.size() == 1) {
            if (argument_values[0] == Argument::SUPPRESS) return;
        }
        argument.action_(this, ret, *arg_it, argument_values);
    }

    void __check_conflict(Argument const& argument) {
        std::vector<std::string_view> conflict_options;
        for (std::string_view option_string : argument.option_strings_) {
            if (optional_args_map_.find(option_string) != optional_args_map_.end()) {
                conflict_options.push_back(option_string);
            }
        }
        // resolve or error & exit if conflict exists
        if (!conflict_options.empty()) {
            std::stringstream ss;
            ss << "conflicting option string(s): ";
            for (size_t i = 0; i < conflict_options.size(); ++i) {
                ss << conflict_options[i];
                if (i < conflict_options.size() - 1) {
                    ss << ", ";
                }
            }
            throw argument_error(argument, ss.str());
        }
    }

    std::list<ArgumentRef> __get_positional_args() {
        std::list<ArgumentRef> ret;

        for (auto& argument : args_list_) {
            if (argument.option_strings_.size() == 0) {
                ret.push_back(argument);
            }
        }

        return ret;
    }

    bool __match_negative_number(std::string const& arg_string) {
        std::string negative_number_pattern_str = R"(^-\d+$|^-\d*\.\d+$)";
        std::regex pattern(negative_number_pattern_str);
        if (std::regex_match(arg_string, pattern)) {
            return true;
        }
        return false;
    }

    std::string __format_help() const {
        HelpFormatter formatter(prog_name_);
        // calculate argument max length
        formatter.set_argument_max_length(args_list_);

        std::ostringstream oss;
        // usage
        oss << formatter.add_usage(usage_, args_list_, mutually_exclusive_groups_);
        // description
        if (description_.length()) {
            oss << '\n' << formatter.add_text(description_) << '\n';
        }
        // positionals, optionals and user-defined groups
        for (auto const& group : argument_groups_) {
            std::string heading = formatter.start_section(group.title_);
            std::string description = formatter.add_text(group.description_);
            std::string arguments = formatter.add_arguments(group.group_arguments_);
            formatter.end_section();

            if (description.length() == 0 && arguments.length() == 0) {
                // do nothing
            }
            else {
                oss << '\n';
                if (heading.length()) {
                    oss << heading;
                }
                if (description.length()) {
                    oss << description << "\n\n";
                }
                if (arguments.length()) {
                    oss << arguments;
                }
            }
        }
        // epilog
        std::string epilog = formatter.add_text(epilog_);
        if (epilog.length()) {
            oss << '\n' << formatter.add_text(epilog_) << '\n';
        }

        return oss.str();
    }

    std::string __format_usage() const {
        HelpFormatter formatter(prog_name_);
        return formatter.add_usage(usage_, args_list_, mutually_exclusive_groups_);
    }

    [[noreturn]] void __error(std::string const& err_msg) const {
        std::cout << __format_usage();
        __exit("[ARGPARSE ERROR] " + err_msg, 2);
    }

    [[noreturn]] void __exit(std::string_view err_msg, int const status = 0) const {
        if (err_msg.length())
            std::cout << err_msg << std::endl;
        std::exit(status);
    }
};

// ==============================================================================================================
// Argument Definition
// ==============================================================================================================
ArgumentParser& Argument::add_parser(std::string const& name, std::vector<std::string> const& aliases, std::string_view help)
{
    if (!parent_ || (type_ != ArgumentType::subcommand_argument)) {
        throw argument_error(*this, "add_parser is not supported for this argument");
    }

    // check conflict
    auto conflict_name = std::find(choices_.cbegin(), choices_.cend(), name);
    if (conflict_name != choices_.end()) {
        throw argument_error(*this, "conflicting subparser: " + *conflict_name);
    }

    // create the parser and add it to the map
    std::string prog_name = parent_->prog_name_ + ' ' + name;
    auto subparser_it = parent_->subparsers_list_.emplace(parent_->subparsers_list_.end(), prog_name);
    parent_->subparsers_map_.insert_or_assign(name, subparser_it);
    // make parser available under aliases also
    for (auto const& alias : aliases) {
        if (std::find(choices_.cbegin(), choices_.cend(), alias) != choices_.cend()) {
            throw argument_error(*this, "conflicting subparser alias: " + alias);
        }
        parent_->subparsers_map_.insert_or_assign(alias, subparser_it);
    }

    // create a subargument to hold the choice help
    if (help.length()) {
        std::string metavar = name;
        if (aliases.size()) {
            metavar += " (";
            metavar += utils::join(aliases, ", ");
            metavar += ')';
        }
        Argument arg;
        arg.set_help(help)
        .set_dest(name)
        .set_metavar(metavar);
        subarguments.push_back(arg);
    }

    // update choices & add subcommand names
    std::vector<std::string> choices = get_choices();
    choices.push_back(name);
    subparser_names.push_back({name});
    for (auto const& alias : aliases) {
        choices.push_back(alias);
        subparser_names.back().push_back(alias);
    }
    set_choices(choices);

    return *subparser_it;
}

void Argument::initialize() {
    // mark positional arguments as required if at least one is always required
    if (type_ == ArgumentType::positional_argument) {
        if (nargs_.get_type() != NArgs::Type::optional && nargs_.get_type() != NArgs::Type::zero_or_more) {
            required_ = true;
        }
        if (nargs_.get_type() == NArgs::Type::zero_or_more) {
            if (!default_value_.size()) {
                required_ = true;
            }
        }
    }

    // if the argument is subparser, update prog_name
    if (parent_) {
        for (auto const& names : subparser_names) {
            auto& subparser = *parent_->subparsers_map_[names[0]];
            subparser.prog_name_ = parent_->prog_name_ + ' ' + names[0];
        }
    }

    // check validation
    validation_(*this);
}

// ==============================================================================================================
// ArgumentGroup & MutuallyExclusiveGroup Definition
// ==============================================================================================================
template<typename... Args>
Argument& ArgumentGroup::add_argument(Args&&... args)
{
    Argument& argument = parser_.__add_argument(std::forward<Args>(args)...);
    group_arguments_.push_back(argument);
    // add optional argument to arguments map
    parser_.__add_optional_argument_map(--group_arguments_.end());
    return argument;
}

Argument& ArgumentGroup::add_argument(Argument& argument)
{
    group_arguments_.push_back(argument);
    // add optional argument to arguments map
    parser_.__add_optional_argument_map(--group_arguments_.end());
    return argument;
}

template<typename... Args>
Argument& MutuallyExclusiveGroup::add_argument(Args&&... args)
{
    Argument& argument = parser_.__add_argument(std::forward<Args>(args)...);
    group_arguments_.push_back(argument);
    // add optional argument to arguments map
    parser_.__add_optional_argument(argument);
    return argument;
}

Argument& MutuallyExclusiveGroup::add_argument(Argument& argument)
{
    group_arguments_.push_back(argument);
    // add optional argument to arguments map
    parser_.__add_optional_argument(argument);
    return argument;
}

// ==============================================================================================================
// Action Definition
// ==============================================================================================================
namespace actions {

void StoreAction::initialize(Argument& argument)
{
    //argument.set_required(false);
}
Action StoreAction::get_action()
{
    return [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
        if (argument.get_nargs().get_type() == NArgs::Type::integer && argument.get_nargs().get_nargs() == static_cast<size_t>(0)) {
            throw std::logic_error("nargs for store actions must be != 0; if you have nothing to store, actions such a store true or store const may be more appropriate");
        }
        if (argument.get_const().size() && argument.get_nargs().get_type() != NArgs::Type::optional) {
            throw std::logic_error("nargs must be '?'(optional) to supply const");
        }

        auto const nargs_type = argument.get_nargs().get_type();
        if (nargs_type == NArgs::Type::none || nargs_type == NArgs::Type::optional) {
            args.set_value(argument.get_dest(), values[0]);
        }
        else {
            args[argument.get_dest()] = values;
        }
    };
}
Validation StoreAction::get_valid()
{
    return [](Argument const& argument) {
        auto const& nargs = argument.get_nargs();
        if (nargs.get_type() == NArgs::Type::integer && nargs.get_nargs() == 0) {
            throw std::logic_error("nargs for store actions must be != 0; if you have nothing to store, actions such as store true or false const may be more appropriate");
        }
        if (argument.get_const().size() && nargs.get_type() != NArgs::Type::optional) {
            throw std::logic_error("nargs must be '?'(optional) to supply const");
        }
    };
}

void StoreConstAction::initialize(Argument& argument)
{
    argument.set_required(false);
    argument.set_nargs(0);
}
Action StoreConstAction::get_action()
{
    return [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
        args.set_values(argument.get_dest(), argument.get_const());
    };
}
Validation StoreConstAction::get_valid()
{
    return [](Argument const& argument) {
        // no valid condition
    };
}

void StoreTrueAction::initialize(Argument& argument)
{
    argument.set_const(true);
    argument.set_default(false);
    argument.set_required(false);
    argument.set_nargs(0);
}
Action StoreTrueAction::get_action()
{
    return [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
        args.set_values(argument.get_dest(), argument.get_const());
    };
}
Validation StoreTrueAction::get_valid()
{
    return [](Argument const& argument) {
        // no valid conditions
    };
}

void StoreFalseAction::initialize(Argument& argument)
{
    argument.set_const(false);
    argument.set_default(true);
    argument.set_required(false);
    argument.set_nargs(0);
}
Action StoreFalseAction::get_action()
{
    return [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
        args.set_values(argument.get_dest(), argument.get_const());
    };
}
Validation StoreFalseAction::get_valid()
{
    return [](Argument const& argument) {
        // no valid conditions
    };
}

void AppendAction::initialize(Argument& argument)
{
    argument.set_required(false);
}
Action AppendAction::get_action()
{
    return [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
        auto& arg_values = args[argument.get_dest()];
        for (auto const& value : values) {
            arg_values.push_back(value);
        }
    };
}
Validation AppendAction::get_valid()
{
    return [](Argument const& argument) {
        auto const& nargs = argument.get_nargs();
        if (nargs.get_type() == NArgs::Type::integer && nargs.get_nargs() == 0) {
            throw std::logic_error("nargs for append actions must be != 0; if arg strings are not supplying the value to append, the append const action may be more appropriate");
        }
        if (argument.get_const().size() && nargs.get_type() != NArgs::Type::optional) {
            throw std::logic_error("nargs must be '?'(optional) to supply const");
        }
    };
}

void AppendConstAction::initialize(Argument& argument)
{
    argument.set_nargs(0);
    argument.set_required(false);
}
Action AppendConstAction::get_action()
{
    return [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
        auto const& const_values = argument.get_const();
        auto& arg_values = args[argument.get_dest()];
        
        if (const_values.size()) {
            for (auto const& value : const_values) {
                arg_values.push_back(value);
            }
        }
    };
}
Validation AppendConstAction::get_valid()
{
    return [](Argument const& argument) {
        // no valid conditions
    };
}

void CountAction::initialize(Argument& argument)
{
    argument.set_nargs(0);
    argument.set_required(false);
}
Action CountAction::get_action()
{
    return [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
        if (args[argument.get_dest()].empty()) {
            args[argument.get_dest()].push_back(std::to_string(1));
            return;
        }
        size_t arg_value = std::stoi(args[argument.get_dest()][0]);
        arg_value += 1;
        args[argument.get_dest()][0] = std::to_string(arg_value);
    };
}
Validation CountAction::get_valid()
{
    return [](Argument const& argument) {
        // no valid conditions
    };
}

void HelpAction::initialize(Argument& argument)
{
    argument.set_nargs(0);
    argument.set_default(Argument::SUPPRESS);
    argument.set_dest(Argument::SUPPRESS);
}
Action HelpAction::get_action()
{
    return [](ArgumentParser* parser, Namespace& args, Argument const& argument, std::vector<std::string> const& values) {
        parser->print_help();
        parser->exit();
    };
}
Validation HelpAction::get_valid()
{
    return [](Argument const& argument) {
        // no valid conditions
    };
}

} // namespace argparse::actions

// ==============================================================================================================
// Exception Definition
// ==============================================================================================================
argument_error::argument_error(Argument const& argument, std::string const& msg)
: invalid_argument("") {
    msg_ += "argument ";
    msg_ += argument.get_argument_name();
    msg_ += ": ";
    msg_ += msg;
}

argument_error::argument_error(Argument const& argument, const char* msg)
: invalid_argument("") {
    msg_ += "argument ";
    msg_ += argument.get_argument_name();
    msg_ += ": ";
    msg_ += msg;
}

const char* argument_error::what() const noexcept {
    return msg_.c_str();
}

}