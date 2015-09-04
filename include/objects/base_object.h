#ifndef _BASE_OBJECT_H_
#define _BASE_OBJECT_H_

#include "exceptions.h"
#include <string>

namespace mtpython {

namespace interpreter {
class Typedef;
}

namespace objects {

class ObjSpace;

/* Base of all MTPython objects */
class M_BaseObject {
public:
	virtual M_BaseObject* get_class(ObjSpace* space);
	virtual interpreter::Typedef* get_typedef() { throw NotImplementedException("Abstract"); }

	virtual M_BaseObject* bind_space(ObjSpace* space) { return this; }

	M_BaseObject* unique_id(ObjSpace* space);

	virtual M_BaseObject* get_dict(ObjSpace* space) { return nullptr; }
	M_BaseObject* get(ObjSpace* space, const std::string& attr);
	virtual M_BaseObject* get_dict_value(ObjSpace* space, const std::string& attr);
	virtual bool set_dict_value(ObjSpace* space,const std::string& attr, M_BaseObject* value);
	virtual bool del_dict_value(ObjSpace* space, const std::string& attr);

	virtual bool i_is(ObjSpace* space, M_BaseObject* other) { throw NotImplementedException("Abstract"); }

	virtual int to_int(ObjSpace* space, bool allow_conversion) { throw NotImplementedException("Abstract"); }
	virtual M_BaseObject* get_repr(ObjSpace* space, const std::string& info);
	virtual std::string to_string(ObjSpace* space) { throw NotImplementedException("Abstract"); }

	void lock() {}
	void unlock() {}

	virtual void dbg_print() { }	/* for debug purpose */
};

}
}

#endif
