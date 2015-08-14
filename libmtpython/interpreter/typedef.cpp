#include "interpreter/typedef.h"

using namespace mtpython::interpreter;
using namespace mtpython::objects;

Typedef::Typedef(std::string& name, std::unordered_map<std::string, M_BaseObject*>& dict) : name(name)
{
	add_entries(dict);
}

void Typedef::add_entries(std::unordered_map<std::string, M_BaseObject*>& entries)
{
	for (auto it = entries.begin(); it != entries.end(); it++) {
		this->dict[it->first] = it->second;
	}
}
