#include "utils/file_helper.h"

#ifdef _UNIX_
#include <sys/stat.h>

char mtpython::FileHelper::sep = '/';
char mtpython::FileHelper::pathsep = ':';

bool mtpython::FileHelper::file_exists(const std::string& name)
{
	struct stat sbuf;
	return (stat(name.c_str(), &sbuf) == 0);
}

bool mtpython::FileHelper::is_dir(const std::string& path)
{
	struct stat sbuf;
	if (stat(path.c_str(), &sbuf)) return false;

	return (sbuf.st_mode & S_IFDIR) ? true : false;
}
#else

#ifdef _WIN32_

#include <sys/stat.h>

char mtpython::FileHelper::sep = '\\';
char mtpython::FileHelper::pathsep = ';';

bool mtpython::FileHelper::file_exists(const std::string& name)
{
	struct _stat sbuf;
	return (_stat(name.c_str(), &sbuf) == 0);
}

bool mtpython::FileHelper::is_dir(const std::string& path)
{
	struct _stat sbuf;
	if (_stat(path.c_str(), &sbuf)) return false;

	return (sbuf.st_mode & S_IFDIR) ? true : false;
}

#else
#error "Unsupported platform"
#endif

#endif
