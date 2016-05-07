SET(LIBMTPYTHON_SRCLIST
		parse/token.cpp
		parse/diagnostics.cpp
		parse/scanner.cpp
		parse/parser.cpp
		parse/symtable.cpp
		parse/code_builder.cpp
		parse/codegen.cpp
		tree/nodes.cpp
		utils/file_helper.cpp
		utils/source_buffer.cpp
		objects/obj_space.cpp
		objects/base_object.cpp
		objects/bltin_exceptions.cpp
		objects/std/bool_object.cpp
		objects/std/bytearray_object.cpp
		objects/std/dict_object.cpp
		objects/std/int_object.cpp
		objects/std/iter_object.cpp
		objects/std/list_object.cpp
		objects/std/none_object.cpp
		objects/std/object_object.cpp
		objects/std/unicode_object.cpp
		objects/std/tuple_object.cpp
		objects/std/type_object.cpp
		objects/std/set_object.cpp
		objects/std/obj_space_std.cpp
		interpreter/arguments.cpp
		interpreter/pycode.cpp
		interpreter/compiler.cpp
		interpreter/typedef.cpp
		interpreter/gateway.cpp
		interpreter/function.cpp
		interpreter/pyframe.cpp
		interpreter/module.cpp
		interpreter/descriptor.cpp
		interpreter/cell.cpp
		interpreter/generator.cpp
		modules/_io/iomodule.cpp
		modules/_io/iobase.cpp
		modules/_io/bufferedio.cpp
		modules/_io/fileio.cpp
		modules/_io/textio.cpp
		modules/_weakref/weakrefmodule.cpp
		modules/builtins/bltinmodule.cpp
		modules/posix/posixmodule.cpp
		modules/sys/sysmodule.cpp
		vm/initpath.cpp
		vm/thread_context.cpp
		vm/vm.cpp)

FILE(GLOB_RECURSE LIBMTPYTHON_HDRLIST ../include/*.h)

SOURCE_GROUP("Header Files" FILES ${LIBMTPYTHON_HDRLIST})

SET(LIBRARIES )

ADD_LIBRARY(libmtpython ${LIBMTPYTHON_SRCLIST} ${LIBMTPYTHON_HDRLIST})

SET_TARGET_PROPERTIES(libmtpython PROPERTIES OUTPUT_NAME "mtpython") 

TARGET_LINK_LIBRARIES(libmtpython ${LIBRARIES})
INSTALL(TARGETS libmtpython ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
