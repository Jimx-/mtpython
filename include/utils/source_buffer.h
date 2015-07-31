#ifndef _CHAR_BUFFER_H_
#define _CHAR_BUFFER_H_

#include <vector>
#include <string>

namespace mtpython {
namespace utils {

typedef enum {
	ST_ERROR, ST_FILE_INPUT,
} SourceType;

/* Source Buffer */
/* Used to manage source files, etc. */
class SourceBuffer {
private:
	std::string fname;
	std::vector<char> buf;
	std::vector<int> line_offset;
    int pos;
    SourceType src_type;
	
public:
	/* create an empty buffer */
	SourceBuffer() : pos(0), fname("<no file>"), src_type(ST_ERROR) {};
	/* create a buffer containing the file */
	SourceBuffer(const std::string& filename);
	SourceBuffer(std::vector<char>::iterator first, std::vector<char>::iterator last);
	
	/* load a file */
	int load_file(const std::string& filename);
	/* read a char from the buffer */
	char read();

	SourceType get_src_type();

	/* get & set position */
	int tell_pos();
	void seek(int pos);

	/* true iff the whole buffer has been read */
	bool eof();
	/* how many chars in this buffer */
	int size();

	/* begin & end iterators */
	std::vector<char>::iterator begin();
	std::vector<char>::iterator end();

	std::string get_filename() { return fname; }

	/* get #line_nr line */
	std::string get_line(int line_nr);

	/* dump the buffer */
	void dump();
};

}
}

#endif
