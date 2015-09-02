#ifndef _INTERPRETER_ERROR_H_
#define _INTERPRETER_ERROR_H_

#include "objects/obj_space.h"
#include "exceptions.h"
#include "objects/bltin_exceptions.h"
#include <string>
#include <memory>

namespace mtpython {
namespace interpreter {

/* Interpreter level error to be sent to app level */
class InterpError : public std::runtime_error {
private:
	objects::M_BaseObject* type;
	objects::M_BaseObject* value;
public:
	InterpError(objects::M_BaseObject* type, objects::M_BaseObject* value) : std::runtime_error("Interpreter exception"), type(type), value(value) { }

	objects::M_BaseObject* get_type() const { return type; }
	objects::M_BaseObject* get_value() const { return value; }

	template<typename ... Args>
	static InterpError format(objects::ObjSpace* space, objects::M_BaseObject* type, const char* format, Args ... args)
	{
		size_t size = snprintf(nullptr, 0, format, args ...) + 1;
		std::unique_ptr<char[]> buf(new char[size]);
		snprintf(buf.get(), size, format, args ...);
		return InterpError(type, space->wrap_str(std::string(buf.get(), buf.get() + size - 1)));
	}

	bool match(objects::ObjSpace* space, objects::M_BaseObject* match_type)
	{
		return space->match_exception(type, match_type);
	}
};

}
}

#endif
