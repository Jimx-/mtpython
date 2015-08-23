#include "utils/source_buffer.h"
#include "interpreter/compiler.h"
#include "vm/vm.h"
#include "exceptions.h"
#include <fstream>

#include "interpreter/module.h"

using namespace mtpython::vm;
using namespace mtpython::objects;
using namespace mtpython::parse;

PyVM::PyVM(ObjSpace* space) : main_thread(this, space)
{
	this->space = space;
	space->set_vm(this);
}

void PyVM::run_file(std::string& filename)
{
	std::ifstream file;
	file.open(filename);

	if (!file) {
		throw mtpython::FileNotFoundException(("unable to open file " + filename).c_str());
	}

	std::string source;
    file.seekg(0, std::ios::end);
    source.resize((unsigned int)file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&source[0], source.size());
    file.close();

    main_thread.get_compiler()->compile(source, filename, mtpython::parse::SourceType::ST_FILE_INPUT, 0)->exec_code(&main_thread, space->new_dict(), nullptr);

	while(1);
}
