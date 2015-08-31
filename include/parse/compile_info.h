#ifndef _PARSE_COMPILER_INFO_
#define _PARSE_COMPILER_INFO_

#include <string>

namespace mtpython {
namespace parse {

typedef enum {
	ST_ERROR, ST_FILE_INPUT,
} SourceType;

#define PyCF_SOURCE_IS_UTF8  0x0100

class CompileInfo {
private:
	std::string filename;
	SourceType type;
	int flags;
	std::string encoding;
public:
	CompileInfo(const std::string& filename, SourceType type, int flags) : filename(filename), type(type), flags(flags), encoding("") { }

	std::string& get_filename() { return filename; }
	SourceType get_type() { return type; }
	int get_flags() { return flags; }

	void set_encoding(const std::string& encoding) { this->encoding = encoding; }
	const std::string& get_encoding() { return encoding; }
};

}
}

#endif /* _PARSE_COMPILER_INFO_ */
