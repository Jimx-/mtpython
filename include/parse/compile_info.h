#ifndef _PARSE_COMPILER_INFO_
#define _PARSE_COMPILER_INFO_

#include <string>

namespace mtpython {
namespace parse {

typedef enum {
	ST_ERROR, ST_FILE_INPUT,
} SourceType;

class CompileInfo {
private:
	std::string filename;
	SourceType type;
	int flags;
public:
	CompileInfo(std::string& filename, SourceType type, int flags) : filename(filename), type(type), flags(flags) { }

	std::string& get_filename() { return filename; }
	SourceType get_type() { return type; }
	int get_flags() { return flags; }
};

}
}

#endif /* _PARSE_COMPILER_INFO_ */
