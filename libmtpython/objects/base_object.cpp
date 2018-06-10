#include <memory>

#include "objects/base_object.h"
#include "macros.h"
#include "interpreter/error.h"
#include "gc/heap.h"
#include "vm/vm.h"
#include <iostream>

using namespace mtpython::objects;

void* M_BaseObject::operator new(size_t size, mtpython::vm::ThreadContext* context)
{
	gc::Heap* heap = context->heap();

	M_BaseObject* obj;
	if (heap) {
        obj = (M_BaseObject*) context->heap()->allocate_mem(size);
        context->new_local_ref(obj);
        context->heap()->post_allocate_obj(obj);
	} else {
		obj = (M_BaseObject*) ::operator new(size);
	}

	return obj;
}

M_BaseObject* M_BaseObject::get_class(ObjSpace* space)
{
	return space->get_typeobject(get_typedef());
}

M_BaseObject* M_BaseObject::unique_id(ObjSpace* space)
{
	return space->wrap_int(space->current_thread(), reinterpret_cast<unsigned long>(this));
}

M_BaseObject* M_BaseObject::get(ObjSpace* space, const std::string& attr)
{
	M_BaseObject* obj = get_dict_value(space, attr);
	if (!obj) {
		throw mtpython::interpreter::InterpError(space->AttributeError_type(), space->wrap_str(space->current_thread(), attr));
	}

	return obj;
}

M_BaseObject* M_BaseObject::get_dict_value(ObjSpace* space, const std::string& attr)
{
	M_BaseObject* dict = get_dict(space);
	if (dict) {
		return space->finditem_str(dict, attr);
	}
	return nullptr;
}

bool M_BaseObject::set_dict_value(ObjSpace* space, const std::string& attr, M_BaseObject* value)
{
	M_BaseObject* dict = get_dict(space);

	if (dict) {
		space->setitem_str(dict, attr, value);
		return true;
	}

	return false;
}

bool M_BaseObject::del_dict_value(ObjSpace* space, const std::string& attr)
{
	M_BaseObject* dict = get_dict(space);

	if (dict) {
		M_BaseObject* wrapped_attr = space->wrap_str(space->current_thread(), attr);
		space->delitem(dict, wrapped_attr);
		return true;
	}

	return false;
}

bool mtpython::objects::M_BaseObject::i_is(ObjSpace * space, M_BaseObject * other)
{
	return this == other;
}

M_BaseObject* M_BaseObject::get_repr(ObjSpace* space, const std::string& info)
{
	M_BaseObject* id = space->id(this);
	unsigned long id_int = space->unwrap_int(id);
	/* max id: 0xffffffffffffffff */
	std::unique_ptr<char[]> buf(new char[20]);
	snprintf(buf.get(), 20, "0x%lx", id_int);
	std::string addr(buf.get());

	std::string repr_string = "<" + info + " at " + addr + ">";
	return space->wrap_str(space->current_thread(), repr_string);
}

