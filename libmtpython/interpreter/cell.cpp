#include "interpreter/cell.h"

using namespace mtpython::objects;
using namespace mtpython::interpreter;

static Typedef Cell_typedef("cell", {

});

Typedef* Cell::get_typedef()
{
    return &Cell_typedef;
}
