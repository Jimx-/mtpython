#include <cstdlib>
#include <deque>
#include <iostream>

#include "objects/obj_space.h"
#include "interpreter/module.h"
#include "cpython_version.h"
#include "utils/file_helper.h"

using namespace mtpython::vm;
using namespace mtpython::interpreter;
using namespace mtpython::objects;

/* Remove ".." and "." in the path */
static std::string canonical_path(const std::string& path)
{
	std::deque<std::string> stack;

	std::size_t start = 0;
	while (true) {
		std::size_t dot = path.find(mtpython::FileHelper::sep, start);
		std::size_t end;
		if (dot == std::string::npos) {
			end = path.size();

		} else {
			end = dot;
		}

		std::string part = path.substr(start, end - start);
		if (part == ".." && stack.size() > 0) stack.pop_back();
		else if (part != "." && part != "") stack.push_back(part);

		if (dot == std::string::npos) break;
		while (end < path.size() && path[end] == mtpython::FileHelper::sep) end++;

		start = end;
	}

	std::string final_path = "";
	for (auto& part : stack) {
#ifdef _WIN32_
		if (final_path.size() > 0) {
#endif
		final_path += mtpython::FileHelper::sep;
#ifdef _WIN32_
        }
#endif
        final_path += part;
    }

    return final_path;
}

/* Return full path of the executable */
static std::string find_executable(const std::string& executable)
{
    std::string exec_name = executable;
#ifdef _WIN32_
    if (exec_name.substr(exec_name.size() - 4, 4) != ".exe") exec_name += ".exe";
#endif
    if (exec_name.find(mtpython::FileHelper::sep) != std::string::npos) return exec_name;
    else {
        char* path_env;
#ifdef _WIN32_
        std::size_t _size;
        _dupenv_s(&path_env, &_size, "PATH");
        if (!path_env) return "";
#else
        path_env = getenv("PATH");
#endif
        if (path_env && *path_env) {
            char* start = (char*)path_env;
            char* end = start + 1;
            while (true) {
                while (*end != mtpython::FileHelper::pathsep && *end != '\0') end++;
                std::string dir(start, end - start);

                if (dir.back() != mtpython::FileHelper::sep) dir += mtpython::FileHelper::sep;
                dir += exec_name;
                if (mtpython::FileHelper::file_exists(dir)) {
                    exec_name = dir;
                    break;
                }

                if (*end == '\0') break;
                if (*end == ';') end++;
                start = end;
                end++;
            }
        }
    }

    return canonical_path(exec_name);
}

static std::string resolve_dir(const std::string& path)
{
    if (path.size() == 0) return path;

    std::string dirname = path;
    if (dirname.back() != mtpython::FileHelper::sep) dirname += mtpython::FileHelper::sep;
    dirname += "..";

    return canonical_path(dirname);
}

static std::string version_str = "";

static bool compute_stdlib_path(const std::string& prefix, std::vector<std::string>& paths)
{
    if (version_str == "") {
        version_str += CPYTHON_VERSION[0];
        version_str += '.';
        version_str += CPYTHON_VERSION[1];
    }

    std::string lib_python = prefix + mtpython::FileHelper::sep;
    lib_python += "lib-python";
    lib_python += mtpython::FileHelper::sep;
    lib_python += version_str;
    if (!mtpython::FileHelper::is_dir(lib_python)) return false;

    paths.push_back(canonical_path(lib_python));
    return true;
}

static bool find_stdlib(const std::string& executable, std::vector<std::string>& paths, std::string& prefix)
{
    std::string search = executable;
    while (true) {
        std::string dirname = resolve_dir(search);
        if (search == dirname) return false;

        if (compute_stdlib_path(dirname, paths)) {
            prefix.assign(dirname);
            break;
        } else
            paths.clear();

        search = dirname;
    }

    return true;
}

void PyVM::init_bootstrap_path(const std::string& executable)
{
    std::string exec_name = executable;
    std::string exec_path = find_executable(exec_name);
    std::vector<std::string> stdlib_paths;
    std::string prefix = "";
    if (!find_stdlib(exec_path, stdlib_paths, prefix)) {
        std::cerr << "error: cannot compute sys.path, using default" << std::endl;
        return;
    }

    std::vector<M_BaseObject*> wrapped_paths;
    for (auto& path : stdlib_paths)
        wrapped_paths.push_back(space->wrap_str(path));

    BuiltinModule* sys_mod = static_cast<BuiltinModule*>(space->get_sys());
    M_BaseObject* dict = sys_mod->get_dict(space);
    space->setitem_str(dict, "path", space->new_list(wrapped_paths));

    space->setitem_str(dict, "executable", space->wrap_str(executable));
    space->setitem_str(dict, "prefix", space->wrap_str(prefix));
    space->setitem_str(dict, "exec_prefix", space->wrap_str(prefix));
}
