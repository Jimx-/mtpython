#include <fstream>

#include "utils/source_buffer.h"
#include "interpreter/compiler.h"
#include "interpreter/pycode.h"
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

    compile_code(&main_thread, source, filename, "exec")->exec_code(&main_thread, space->new_dict(), nullptr);
}

mtpython::interpreter::Code* PyVM::compile_code(ThreadContext* context, const std::string &source,
												const std::string &filename, const std::string &mode)
{
	ObjSpace* space = context->get_space();
	mtpython::interpreter::Module* mod = dynamic_cast<mtpython::interpreter::Module*>(space->get_builtin());

	M_BaseObject* code_obj = mod->call(context, "compile", {space->wrap(source), space->wrap(filename), space->wrap(mode), space->wrap(0), space->wrap(0), space->wrap(0)});

	mtpython::interpreter::PyCode* code = dynamic_cast<mtpython::interpreter::PyCode*>(code_obj);
	return code;
}

void ThreadContext::enter(mtpython::interpreter::PyFrame* frame)
{
	frame_stack.push(frame);
}

void ThreadContext::leave(mtpython::interpreter::PyFrame* frame)
{
	frame_stack.pop();
}
