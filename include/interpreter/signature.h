#ifndef _INTERPRETER_SIGNATURE_H_
#define _INTERPRETER_SIGNATURE_H_

#include "objects/base_object.h"
#include <vector>
#include <initializer_list>
#include <algorithm>

namespace mtpython {
namespace interpreter {

/* Code signature */
class Signature {
protected:
    std::vector<std::string> argnames;
    /* varargname can be: "" = no vararg, "*" = vararg without name, otherwise
     * the name of vararg */
    std::string varargname;
    std::string kwargname;
    std::vector<std::string> kwonlyargnames;

public:
    Signature() {}
    Signature(const std::initializer_list<std::string>& argnames)
        : argnames(argnames), varargname(""), kwargname("")
    {}
    Signature(const std::string& varargname, const std::string& kwargname)
        : varargname(varargname), kwargname(kwargname)
    {}
    Signature(const std::initializer_list<std::string>& argnames,
              const std::string& varargname, const std::string& kwargname,
              const std::initializer_list<std::string>& kwonlyargnames)
        : argnames(argnames), varargname(varargname), kwargname(kwargname),
          kwonlyargnames(kwonlyargnames)
    {}
    Signature(const std::vector<std::string>& argnames,
              const std::string& varargname, const std::string& kwargname,
              const std::vector<std::string>& kwonlyargnames)
        : argnames(argnames), varargname(varargname), kwargname(kwargname),
          kwonlyargnames(kwonlyargnames)
    {}

    bool has_vararg() { return (varargname != ""); }
    bool has_kwarg() { return (kwargname != ""); }

    int find_argname(const std::string& name)
    {
        std::vector<std::string>::iterator it =
            std::find(argnames.begin(), argnames.end(), name);
        if (it == argnames.end())
            return -1;
        else
            return it - argnames.begin();
    }

    std::string& get_varargname() { return varargname; }

    int get_nargs() { return argnames.size(); }
    int scope_len()
    {
        return argnames.size() + (has_kwarg() ? 1 : 0) + (has_vararg() ? 1 : 0);
    }
};

} // namespace interpreter
} // namespace mtpython

#endif /* _INTERPRETER_SIGNATURE_H_ */
