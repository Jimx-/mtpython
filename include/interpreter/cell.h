#ifndef _INTERPRETER_CELL_H_
#define _INTERPRETER_CELL_H_

#include "objects/base_object.h"
#include "objects/obj_space.h"
#include "interpreter/typedef.h"

namespace mtpython {
namespace interpreter {

class Cell : public objects::M_BaseObject {
private:
    objects::M_BaseObject* value;
public:
    Cell(objects::M_BaseObject* value=nullptr) : value(value) { }
    Typedef* get_typedef();

    objects::M_BaseObject* get() { return value; }
    void set(objects::M_BaseObject* v) { value = v; }
};

}
}

#endif
