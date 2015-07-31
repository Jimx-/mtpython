#ifndef _TYPE_H_
#define _TYPE_H_

namespace mtpython {
namespace type {

	typedef enum {
		TY_NOTYPE, TY_INT,
	} TypeTag;

	class Type {
	public:
		virtual TypeTag get_type() { return TY_NOTYPE; }
	};
}
}

#endif /* _TYPE_H_ */
