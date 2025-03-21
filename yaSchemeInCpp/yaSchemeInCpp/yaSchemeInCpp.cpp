// yaSchemeInCpp.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <cstdio>
#include <tchar.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <variant>
#include <optional>
#include <cstdint>

using namespace std;

// return given number as a string
[[nodiscard]] string str(int64_t n) {
    ostringstream os;
    os << n;
    return os.str();
}

// return true iff given character is '0'..'9'
[[nodiscard]] bool isdig(char c) {
    return isdigit(static_cast<unsigned char>(c)) != 0;
}

////////////////////// cell

enum class cell_type { Symbol, Number, List, Proc, Lambda };

//class CCELL {
//    cell_type tag;
//    std::variant<int, float> value;
//};

struct environment; // forward declaration; cell and environment reference each other

// a variant that can hold any kind of lisp value
struct cell {
    using proc_type = cell(*)(const vector<cell>&);
    using iter = vector<cell>::const_iterator;
    using map_type = map<string, cell>;

    cell_type type;
    string val;
    vector<cell> list;
    proc_type proc;
    environment *  env;

    explicit cell(cell_type type = cell_type::Symbol) : type(type) {}

    cell(cell_type type, const  string& val) : type(type), val(val) {}

    explicit cell(proc_type proc) : type(cell_type::Proc), proc(proc) {}
};

using cells = vector<cell>;
using cellit = cells::const_iterator;

inline const cell false_sym(cell_type::Symbol, "#f");
inline const cell true_sym(cell_type::Symbol, "#t"); // anything that isn't false_sym is true
inline const cell nil(cell_type::Symbol, "nil");


////////////////////// environment

// a dictionary that (a) associates symbols with cells, and
// (b) can chain to an "outer" dictionary
struct environment {
    explicit environment(environment* outer = nullptr)
        : outer_(outer) {
    }

    environment(const cells& parms, const cells& args, environment* outer)
        : outer_(outer)
    {
        auto a = args.begin();
        for (auto p = parms.begin(); p != parms.end(); ++p)
            env_[p->val] = *a++;
    }

    // map a variable name onto a cell
    using map_type = map<string, cell>;

    // return a reference to the innermost environment where 'var' appears
    [[nodiscard]] map_type& find(const  string& var) {
        if (auto it = env_.find(string(var)); it != env_.end())
            return env_; // the symbol exists in this environment
        if (outer_)
            return outer_->find(var); // attempt to find the symbol in some "outer" env
        cout << "unbound symbol '" << var << "'\n";
        exit(1);
    }

    // return a reference to the cell associated with the given symbol 'var'
    [[nodiscard]] cell& operator[](const  string& var) {
        return env_[string(var)];
    }

private:
    map_type env_; // inner symbol->cell mapping
    environment* outer_; // next adjacent outer env, or nullopt if there are no further environments
};


////////////////////// built-in primitive procedures

[[nodiscard]] cell proc_add(const cells& c) {
    int64_t n(stoll(c[0].val));
    for (auto i = c.begin() + 1; i != c.end(); ++i)
        n += stoll(i->val);
    return cell(cell_type::Number, str(n));
}

[[nodiscard]] cell proc_sub(const cells& c) {
    int64_t n(stoll(c[0].val));
    for (auto i = c.begin() + 1; i != c.end(); ++i)
        n -= stoll(i->val);
    return cell(cell_type::Number, str(n));
}

[[nodiscard]] cell proc_mul(const cells& c) {
    int64_t n(1);
    for (const auto& i : c)
        n *= stoll(i.val);
    return cell(cell_type::Number, str(n));
}

[[nodiscard]] cell proc_div(const cells& c) {
    int64_t n(stoll(c[0].val));
    for (auto i = c.begin() + 1; i != c.end(); ++i)
        n /= stoll(i->val);
    return cell(cell_type::Number, str(n));
}

[[nodiscard]] cell proc_greater(const cells& c) {
    int64_t n(stoll(c[0].val));
    for (auto i = c.begin() + 1; i != c.end(); ++i)
        if (n <= stoll(i->val))
            return false_sym;
    return true_sym;
}

[[nodiscard]] cell proc_less(const cells& c) {
    int64_t n(stoll(c[0].val));
    for (auto i = c.begin() + 1; i != c.end(); ++i)
        if (n >= stoll(i->val))
            return false_sym;
    return true_sym;
}

[[nodiscard]] cell proc_less_equal(const cells& c) {
    int64_t n(stoll(c[0].val));
    for (auto i = c.begin() + 1; i != c.end(); ++i)
        if (n > stoll(i->val))
            return false_sym;
    return true_sym;
}

[[nodiscard]] cell proc_length(const cells& c) { return cell(cell_type::Number, str(c[0].list.size())); }
[[nodiscard]] cell proc_nullp(const cells& c) { return c[0].list.empty() ? true_sym : false_sym; }
[[nodiscard]] cell proc_car(const cells& c) { return c[0].list[0]; }

[[nodiscard]] cell proc_cdr(const cells& c) {
    if (c[0].list.size() < 2)
        return nil;
    cell result(c[0]);
    result.list.erase(result.list.begin());
    return result;
}

[[nodiscard]] cell proc_append(const cells& c) {
    cell result(cell_type::List);
    result.list = c[0].list;
    result.list.insert(result.list.end(), c[1].list.begin(), c[1].list.end());
    return result;
}

[[nodiscard]] cell proc_cons(const cells& c) {
    cell result(cell_type::List);
    result.list.push_back(c[0]);
    result.list.insert(result.list.end(), c[1].list.begin(), c[1].list.end());
    return result;
}

[[nodiscard]] cell proc_list(const cells& c) {
    cell result(cell_type::List); result.list = c;
    return result;
}

// define the bare minimum set of primitives necessary to pass the unit tests
void add_globals(environment& env) {
    env["nil"] = nil;
    env["#f"] = false_sym;
    env["#t"] = true_sym;
    env["append"] = cell(&proc_append);
    env["car"] = cell(&proc_car);
    env["cdr"] = cell(&proc_cdr);
    env["cons"] = cell(&proc_cons);
    env["length"] = cell(&proc_length);
    env["list"] = cell(&proc_list);
    env["null?"] = cell(&proc_nullp);
    env["+"] = cell(&proc_add);
    env["-"] = cell(&proc_sub);
    env["*"] = cell(&proc_mul);
    env["/"] = cell(&proc_div);
    env[">"] = cell(&proc_greater);
    env["<"] = cell(&proc_less);
    env["<="] = cell(&proc_less_equal);
}


////////////////////// eval

 cell eval(cell x, environment* env) {
    if (x.type == cell_type::Symbol)
        return env->find(x.val)[x.val];
    if (x.type == cell_type::Number)
        return x;
    if (x.list.empty())
        return nil;
    if (x.list[0].type == cell_type::Symbol) {
        if (x.list[0].val == "quote")       // (quote exp)
            return x.list[1];
        if (x.list[0].val == "if") {        // (if test conseq [alt])
            auto test_result = eval(x.list[1], env);
            return eval(test_result.val == "#f" ?
                (x.list.size() < 4 ? nil : x.list[3]) : x.list[2], env);
        }
        if (x.list[0].val == "set!")        // (set! var exp)
            return env->find(x.list[1].val)[x.list[1].val] = eval(x.list[2], env);
        if (x.list[0].val == "define")      // (define var exp)
            return (*env)[x.list[1].val] = eval(x.list[2], env);
        if (x.list[0].val == "lambda") {    // (lambda (var*) exp)
            x.type = cell_type::Lambda;
            x.env = env;
            return x;
        }
        if (x.list[0].val == "begin") {     // (begin exp*)
            for (size_t i = 1; i < x.list.size() - 1; ++i)
                eval(x.list[i], env);
            return eval(x.list[x.list.size() - 1], env);
        }
    }
    // (proc exp*)
    cell proc(eval(x.list[0], env));
    cells exps;
    for (auto exp = x.list.begin() + 1; exp != x.list.end(); ++exp)
        exps.push_back(eval(*exp, env));
    if (proc.type == cell_type::Lambda) {
        return eval(/*body*/proc.list[2], new environment(/*parms*/proc.list[1].list, /*args*/exps, proc.env));
    }
    else if (proc.type == cell_type::Proc)
        return proc.proc(exps);
    else {
        cout << "not a function\n";
        exit(1);
    }
}


////////////////////// parse, read and user interaction

[[nodiscard]] list<string> tokenize(string str) {
    list<string> tokens;
    const char* s = str.c_str();
    while (*s) {
        while (*s == ' ')
            ++s;
        if (*s == '(' || *s == ')')
            tokens.push_back(*s++ == '(' ? "(" : ")");
        else {
            const char* t = s;
            while (*t && *t != ' ' && *t != '(' && *t != ')')
                ++t;
            tokens.push_back(string(s, t));
            s = t;
        }
    }
    return tokens;
}

[[nodiscard]] cell atom(const  string& token) {
    if (isdig(token[0]) || (token[0] == '-' && isdig(token[1])))
        return cell(cell_type::Number, token);
    return cell(cell_type::Symbol, token);
}

[[nodiscard]] cell read_from(list<string>& tokens) {
    if (tokens.empty()) {
        cout << "unexpected EOF while reading\n";
        exit(1);
    }
    string token = tokens.front();
    tokens.pop_front();
    if (token == "(") {
        cell c(cell_type::List);
        while (tokens.front() != ")")
            c.list.push_back(read_from(tokens));
        tokens.pop_front();
        return c;
    }
    else
        return atom(token);
}

[[nodiscard]] cell read(string s) {
    list<string> tokens = tokenize(s);
    if (tokens.empty())
        return nil;
    return read_from(tokens);
}

[[nodiscard]] string to_string(const cell& exp) {
    if (exp.type == cell_type::List) {
        string s("(");
        for (auto e = exp.list.begin(); e != exp.list.end(); ++e)
            s += to_string(*e) + ' ';
        if (s[s.size() - 1] == ' ')
            s.erase(s.size() - 1);
        return s + ')';
    }
    else if (exp.type == cell_type::Lambda)
        return "<Lambda>";
    else if (exp.type == cell_type::Proc)
        return "<Proc>";
    return exp.val;
}

// the default read-eval-print-loop
void repl(const  string& prompt, environment* env)
{
    for (;;) {
        cout << prompt;
        string line;  getline(cin, line);
        cout << to_string(eval(read(line), env)) << '\n';
    }
}

int main() {
    environment global_env;
    add_globals(global_env);
    repl(">>> ", &global_env);
    return 0;
}




// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
