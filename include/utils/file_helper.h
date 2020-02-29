#ifndef _FILE_HELPER_H_
#define _FILE_HELPER_H_

#include <string>

namespace mtpython {

class FileHelper {
public:
    static char sep;
    static char pathsep;

    static bool file_exists(const std::string& name);
    static bool is_dir(const std::string& path);
};

} // namespace mtpython

#endif /* _FILE_HELPER_H_ */
