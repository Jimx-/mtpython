#include "interpreter/typedef.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

Typedef::Typedef(const std::string& name,
                 const std::unordered_map<std::string, M_BaseObject*>& dict)
    : name(name)
{
    add_entries(dict);
}

Typedef::Typedef(const std::string& name, const std::vector<Typedef*>& bases,
                 const std::unordered_map<std::string, M_BaseObject*>& dict)
    : name(name), bases(bases)
{
    add_entries(dict);
}

void Typedef::add_entries(
    const std::unordered_map<std::string, M_BaseObject*>& entries)
{
    for (auto it = entries.begin(); it != entries.end(); it++) {
        this->dict[it->first] = it->second;
    }
}
