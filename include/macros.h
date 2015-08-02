#ifndef _MACRO_H_
#define _MACRO_H_

#include <cstddef>

#define SAFE_DELETE(p) do { if (p) delete p; p = NULL; } while(0)

#endif /* _MACRO_H_ */
