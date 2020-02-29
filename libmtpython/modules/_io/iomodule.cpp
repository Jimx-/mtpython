#include "modules/_io/iomodule.h"
#include "interpreter/arguments.h"
#include "interpreter/gateway.h"
#include "interpreter/pycode.h"
#include "interpreter/compiler.h"
#include "interpreter/error.h"
#include "interpreter/pyframe.h"
#include "interpreter/typedef.h"
#include "interpreter/descriptor.h"

#include "modules/_io/iobase.h"
#include "modules/_io/textio.h"
#include "modules/_io/fileio.h"
#include "modules/_io/bufferedio.h"

using namespace mtpython::modules;
using namespace mtpython::objects;
using namespace mtpython::interpreter;

static Typedef _IOBase_typedef("_io._IOBase", {});

static Typedef _RawIOBase_typedef("_io._RawIOBase", {&_IOBase_typedef}, {});

static Typedef* _fileio_typedef()
{
    static Typedef FileIO_typedef(
        "_io.FileIO", {&_RawIOBase_typedef},
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", M_FileIO::__new__)},
            {"__init__",
             new InterpFunctionWrapper("__init__", M_FileIO::__init__)},
            {"name",
             new GetSetDescriptor(M_FileIO::name_get, M_FileIO::name_set)},
        });
    return &FileIO_typedef;
}
Typedef* M_FileIO::get_typedef() { return _fileio_typedef(); }

static Typedef _BufferedIOBase_typedef("_io._BufferedIOBase",
                                       {&_IOBase_typedef}, {});
Typedef* M_BufferedIOBase::get_typedef() { return &_BufferedIOBase_typedef; }

static Typedef* _bufferedreader_typedef()
{
    static Typedef BufferedReader_typedef(
        "_io.BufferedReader", {&_BufferedIOBase_typedef},
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", M_BufferedReader::__new__)},
            {"__init__",
             new InterpFunctionWrapper("__init__", M_BufferedReader::__init__)},
            {"name", new GetSetDescriptor(M_BufferedReader::name_get)},
        });
    return &BufferedReader_typedef;
}
Typedef* M_BufferedReader::get_typedef() { return _bufferedreader_typedef(); }

static Typedef* _bufferwriter_typedef()
{
    static Typedef BufferedWriter_typedef(
        "_io.BufferedWriter", {&_BufferedIOBase_typedef},
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", M_BufferedWriter::__new__)},
            {"__init__",
             new InterpFunctionWrapper("__init__", M_BufferedWriter::__init__)},
            {"name", new GetSetDescriptor(M_BufferedWriter::name_get)},
        });
    return &BufferedWriter_typedef;
}
Typedef* M_BufferedWriter::get_typedef() { return _bufferwriter_typedef(); }

static Typedef _TextIOBase_typedef("_io._TextIOBase", {&_IOBase_typedef}, {});

static Typedef* _textiowrapper_typedef()
{
    static Typedef TextIOWrapper_typedef(
        "_io.TextIOWrapper", {&_TextIOBase_typedef},
        {
            {"__new__",
             new InterpFunctionWrapper("__new__", M_TextIOWrapper::__new__)},
            {"__init__",
             new InterpFunctionWrapper("__init__", M_TextIOWrapper::__init__)},
            {"__repr__",
             new InterpFunctionWrapper("__repr__", M_TextIOWrapper::__repr__)},
            {"name", new GetSetDescriptor(M_TextIOWrapper::name_get)},
            {"buffer", new GetSetDescriptor(M_TextIOWrapper::buffer_get)},
        });

    return &TextIOWrapper_typedef;
}

Typedef* M_TextIOWrapper::get_typedef() { return _textiowrapper_typedef(); }

static M_BaseObject* io_open(mtpython::vm::ThreadContext* context,
                             const Arguments& args)
{
    static Signature open_signature({"file", "mode", "buffering", "encoding",
                                     "errors", "newline", "closefd"});

    ObjSpace* space = context->get_space();

    std::vector<M_BaseObject*> scope;
    args.parse("open", nullptr, open_signature, scope,
               {space->wrap_str(context, "r"), space->wrap_int(context, -1),
                space->wrap_None(), space->wrap_None(), space->wrap_None(),
                space->new_bool(true)});
    M_BaseObject* file = scope[0];
    M_BaseObject* wrapped_mode = scope[1];
    std::string mode = space->unwrap_str(wrapped_mode);
    M_BaseObject* wrapped_buffering = scope[2];
    int buffering = space->unwrap_int(wrapped_buffering, false);
    M_BaseObject* wrapped_encoding = scope[3];
    M_BaseObject* wrapped_errors = scope[4];
    M_BaseObject* wrapped_newline = scope[5];
    M_BaseObject* wrapped_closefd = scope[6];
    bool closefd = space->is_true(wrapped_closefd);

    bool updating = false, appending = false, reading = false, writing = false,
         binary = false;
    for (char c : mode) {
        switch (c) {
        case 'r':
            reading = true;
            break;
        case 'w':
            writing = true;
            break;
        case 'a':
            appending = true;
            break;
        case '+':
            updating = true;
            break;
        case 'b':
            binary = true;
            break;
        default:
            throw InterpError::format(space, space->ValueError_type(),
                                      "invalid mode: %s", mode.c_str());
        }
    }

    M_BaseObject* raw =
        space->call_function(context, space->get_typeobject(_fileio_typedef()),
                             {file, wrapped_mode, wrapped_closefd});

    bool line_buffering = false;
    if (buffering < 0) buffering = DEFAULT_BUFFER_SIZE;

    M_BaseObject* buffer_cls;
    if (updating) {

    } else if (writing || appending) {
        buffer_cls = space->get_typeobject(_bufferwriter_typedef());
    } else {
        buffer_cls = space->get_typeobject(_bufferedreader_typedef());
    }

    M_BaseObject* buffer = space->call_function(
        context, buffer_cls, {raw, space->wrap_int(context, buffering)});

    M_BaseObject* wrapper;
    if (binary) {
        wrapper = buffer;
    } else {
        wrapper = space->call_function(
            context, space->get_typeobject(&_TextIOBase_typedef),
            {buffer, wrapped_encoding, wrapped_errors, wrapped_newline,
             space->new_bool(line_buffering)});
        space->setattr(wrapper, space->wrap_str(context, "mode"), wrapped_mode);
    }

    return wrapper;
}

IOModule::IOModule(ObjSpace* space, M_BaseObject* name)
    : BuiltinModule(space, name)
{
    add_def("DEFAULT_BUFFER_SIZE",
            space->wrap_int(space->current_thread(), DEFAULT_BUFFER_SIZE));

    add_def("_IOBase", space->get_typeobject(&_IOBase_typedef));
    add_def("_RawIOBase", space->get_typeobject(&_RawIOBase_typedef));
    add_def("_TextIOBase", space->get_typeobject(&_TextIOBase_typedef));
    add_def("_BufferedIOBase", space->get_typeobject(&_BufferedIOBase_typedef));

    add_def("BufferedReader", space->get_typeobject(_bufferedreader_typedef()));

    add_def("FileIO", space->get_typeobject(_fileio_typedef()));
    add_def("TextIOWrapper", space->get_typeobject(_textiowrapper_typedef()));

    add_def("open", new InterpFunctionWrapper("open", io_open));
}
