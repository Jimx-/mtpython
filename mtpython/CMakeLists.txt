set(MTPYTHON_SRCLIST
		main.cpp)

set(LIBRARIES libmtpython)

add_executable(mtpython ${MTPYTHON_SRCLIST})

target_link_libraries(mtpython ${LIBRARIES})
install(TARGETS mtpython RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
