SET(LIBMTPYTHON_SRCLIST
		parse/token.cpp
		parse/diagnostics.cpp
		parse/scanner.cpp
		parse/parser.cpp
		parse/symtable.cpp
		parse/code_builder.cpp
		parse/codegen.cpp
		tree/nodes.cpp
		utils/source_buffer.cpp
		objects/obj_space.cpp
		objects/std/int_object.cpp
		objects/std/object_object.cpp
		objects/std/type_object.cpp
		objects/std/obj_space_std.cpp
		interpreter/pycode.cpp
		interpreter/compiler.cpp
		interpreter/typedef.cpp
		interpreter/gateway.cpp
		interpreter/function.cpp
		vm/thread_context.cpp
		vm/vm.cpp)

FILE(GLOB_RECURSE LIBMTPYTHON_HDRLIST ../include/*.h)

SOURCE_GROUP("Header Files" FILES ${LIBMTPYTHON_HDRLIST})

SET(LIBRARIES )

ADD_LIBRARY(libmtpython ${LIBMTPYTHON_SRCLIST} ${LIBMTPYTHON_HDRLIST})

SET_TARGET_PROPERTIES(libmtpython PROPERTIES OUTPUT_NAME "mtpython") 

TARGET_LINK_LIBRARIES(libmtpython ${LIBRARIES})
INSTALL(TARGETS libmtpython ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
