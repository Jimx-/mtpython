#ifndef _VALUE_H_
#define _VALUE_H_

namespace mtpython {
namespace type {

	class Value {
	public:
		virtual TypeTag get_type() { return TY_NOTYPE; }
	};
}
}

#endif /* _VALUE_H_ */
