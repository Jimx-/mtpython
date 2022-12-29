corrosion_import_crate(MANIFEST_PATH ${TOPDIR}/3rdparty/mmtk/mmtk/Cargo.toml)

set(LIBMTPYTHON_SRCLIST
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
    objects/std/bytes_object.cpp
    objects/std/memory_object.cpp
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
    modules/errno/errnomodule.cpp
    gc/garbage_collector_mmtk.cpp
    vm/initpath.cpp
    vm/native_thread.cpp
    vm/native_thread_posix.cpp
    vm/thread_context.cpp
    vm/vm.cpp)

set(LIBRARIES CONAN_PKG::spdlog
              mmtk_ruby)

add_library(libmtpython ${LIBMTPYTHON_SRCLIST})

set_target_properties(libmtpython PROPERTIES OUTPUT_NAME "mtpython")

target_link_libraries(libmtpython ${LIBRARIES})
install(TARGETS libmtpython ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
