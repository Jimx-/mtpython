#ifndef _STD_STR_OBJECT_H_
#define _STD_STR_OBJECT_H_

#include <string>
#include "objects/base_object.h"

namespace mtpython {
namespace objects {

#define M_STDUNICODEOBJECT(obj) (static_cast<M_StdUnicodeObject*>(obj))

class M_StdUnicodeObject : public M_BaseObject {
private:
    std::string value;

public:
    M_StdUnicodeObject(const std::string& s);

    bool i_is(ObjSpace* space, M_BaseObject* other);

    static M_BaseObject* __new__(mtpython::vm::ThreadContext* context,
                                 mtpython::objects::M_BaseObject* str_type,
                                 mtpython::objects::M_BaseObject* value);
    static M_BaseObject* __iter__(mtpython::vm::ThreadContext* context,
                                  M_BaseObject* self);
    static M_BaseObject* __repr__(mtpython::vm::ThreadContext* context,
                                  mtpython::objects::M_BaseObject* self);
    static M_BaseObject* __str__(mtpython::vm::ThreadContext* context,
                                 mtpython::objects::M_BaseObject* self);
    static M_BaseObject* __hash__(mtpython::vm::ThreadContext* context,
                                  mtpython::objects::M_BaseObject* self);
    static M_BaseObject* __eq__(mtpython::vm::ThreadContext* context,
                                mtpython::objects::M_BaseObject* self,
                                mtpython::objects::M_BaseObject* other);

    static interpreter::Typedef* _str_typedef();
    interpreter::Typedef* get_typedef();

    std::string to_string(ObjSpace* space) { return value; }

    void dbg_print();
};

} // namespace objects
} // namespace mtpython

#endif /* _STD_STR_OBJECT_H_ */
