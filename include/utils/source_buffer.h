#ifndef _CHAR_BUFFER_H_
#define _CHAR_BUFFER_H_

#include "parse/compile_info.h"
#include <vector>
#include <string>

namespace mtpython {
namespace utils {

/* Source Buffer */
/* Used to manage source files, etc. */
class SourceBuffer {
private:
    std::string fname;
    std::vector<char> buf;
    std::vector<int> line_offset;
    int pos;
    mtpython::parse::SourceType src_type;

public:
    /* create an empty buffer */
    SourceBuffer()
        : pos(0), fname("<no file>"),
          src_type(mtpython::parse::SourceType::ST_ERROR){};
    /* create a buffer containing the file */
    SourceBuffer(const std::string& source,
                 mtpython::parse::SourceType src_type);
    SourceBuffer(std::vector<char>::iterator first,
                 std::vector<char>::iterator last);

    int load_source(const std::string& source);
    /* load a file */
    int load_file(const std::string& filename);
    /* read a char from the buffer */
    char read();

    mtpython::parse::SourceType get_src_type();

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

} // namespace utils
} // namespace mtpython

#endif
