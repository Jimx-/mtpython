#include "modules/_collections/collectionsmodule.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"

namespace mtpython {
namespace modules {

class M_Deque : public objects::M_BaseObject {
public:
    M_Deque() {}

    static interpreter::Typedef* _deque_typedef()
    {
        static interpreter::Typedef deque_typedef("deque", {});
        return &deque_typedef;
    }

    interpreter::Typedef* get_typedef() { return _deque_typedef(); }
};

class M_DefaultDict : public objects::M_BaseObject {
public:
    M_DefaultDict() {}

    static interpreter::Typedef* _defaultdict_typedef()
    {
        static interpreter::Typedef defaultdict_typedef("defaultdict", {});
        return &defaultdict_typedef;
    }

    interpreter::Typedef* get_typedef() { return _defaultdict_typedef(); }
};

CollectionsModule::CollectionsModule(mtpython::objects::ObjSpace* space,
                                     M_BaseObject* name)
    : BuiltinModule(space, name)
{
    add_def("deque", space->get_typeobject(M_Deque::_deque_typedef()));
    add_def("defaultdict",
            space->get_typeobject(M_DefaultDict::_defaultdict_typedef()));
}

} // namespace modules
} // namespace mtpython
