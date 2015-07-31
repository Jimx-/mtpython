SET(MTPYTHON_SRCLIST
		parse/token.cpp
		parse/diagnostics.cpp
		parse/scanner.cpp
		parse/parser.cpp
		tree/nodes.cpp
		utils/source_buffer.cpp)

SET(LIBRARIES )

ADD_LIBRARY(libmtpython ${MTPYTHON_SRCLIST})

SET_TARGET_PROPERTIES(libmtpython PROPERTIES OUTPUT_NAME "mtpython") 

TARGET_LINK_LIBRARIES(libmtpython ${LIBRARIES})
INSTALL(TARGETS libmtpython ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
