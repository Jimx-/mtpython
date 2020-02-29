#ifndef _INTERPRETER_TYPEDEF_H_
#define _INTERPRETER_TYPEDEF_H_

#include <string>
#include <unordered_map>
#include <vector>
#include "objects/base_object.h"

namespace mtpython {
namespace interpreter {

/* Object space independent typedef */
class Typedef {
private:
    std::string name;
    std::vector<Typedef*> bases;
    std::unordered_map<std::string, objects::M_BaseObject*> dict;

public:
    Typedef(
        const std::string& name,
        const std::unordered_map<std::string, objects::M_BaseObject*>& dict);
    Typedef(
        const std::string& name, const std::vector<Typedef*>& bases,
        const std::unordered_map<std::string, objects::M_BaseObject*>& dict);

    std::string& get_name() { return name; }
    std::vector<Typedef*>& get_bases() { return bases; }
    std::unordered_map<std::string, objects::M_BaseObject*>& get_dict()
    {
        return dict;
    }
    void add_entries(
        const std::unordered_map<std::string, objects::M_BaseObject*>& dict);
};

} // namespace interpreter
} // namespace mtpython

#endif /* _INTERPRETER_TYPEDEF_H_ */
