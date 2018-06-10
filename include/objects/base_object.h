#ifndef _BASE_OBJECT_H_
#define _BASE_OBJECT_H_

#include <vector>
#include <string>
#include "exceptions.h"

namespace mtpython {

namespace interpreter {
class Typedef;
}

namespace vm {
class ThreadContext;
}

namespace objects {

class ObjSpace;

/* Base of all MTPython objects */
class M_BaseObject {
public:
	virtual ~M_BaseObject() { }

	void* operator new(size_t size, vm::ThreadContext* context);

	virtual M_BaseObject* get_class(ObjSpace* space);
	virtual void set_class(ObjSpace* space, M_BaseObject* type) { throw NotImplementedException("set_class()"); }
	virtual interpreter::Typedef* get_typedef() { throw NotImplementedException("get_typedef()"); }

	virtual M_BaseObject* bind_space(ObjSpace* space) { return this; }

	M_BaseObject* unique_id(ObjSpace* space);

	virtual M_BaseObject* get_dict(ObjSpace* space) { return nullptr; }
	M_BaseObject* get(ObjSpace* space, const std::string& attr);
	virtual M_BaseObject* get_dict_value(ObjSpace* space, const std::string& attr);
	virtual bool set_dict_value(ObjSpace* space,const std::string& attr, M_BaseObject* value);
	virtual bool del_dict_value(ObjSpace* space, const std::string& attr);

	virtual bool i_is(ObjSpace* space, M_BaseObject* other);

	virtual int to_int(ObjSpace* space, bool allow_conversion) { throw NotImplementedException("to_int()"); }
	virtual M_BaseObject* get_repr(ObjSpace* space, const std::string& info);
	virtual std::string to_string(ObjSpace* space) { throw NotImplementedException("to_string()"); }

	virtual void unpack_iterable(ObjSpace* space, std::vector<M_BaseObject*>& list) { throw NotImplementedException("unpack_iterable()"); }

	void lock() {}
	void unlock() {}

	virtual void dbg_print() { }	/* for debug purpose */
};

class ScopedObjectLock {
private:
	M_BaseObject* obj;
public:
	ScopedObjectLock(M_BaseObject* _obj) : obj(_obj) { obj->lock(); }
	~ScopedObjectLock() { obj->unlock(); }
};

}
}

#endif
