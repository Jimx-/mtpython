#ifndef _TYPE_INT_H_
#define _TYPE_INT_H_

namespace mtpython {
namespace type {

	class TypeInt {

	};

	class ValueInt {
	private:
		int val;
	public:
		virtual TypeTag get_type() { return TY_INT; }
		int get_value() { return val; }
	};
}
}

#endif /* _TYPE_INT_H_ */
