set(STANDALONE true)

list(APPEND CPUS Z80)

list(APPEND MACHINES Z80DAISY)

macro(standalone _projectname)
	add_executable(${_projectname})

	target_sources(${_projectname} PRIVATE
		${MAME_DIR}/src/zexall/main.cpp
		${MAME_DIR}/src/zexall/zexall.cpp
		${MAME_DIR}/src/zexall/zexall.h
		${MAME_DIR}/src/zexall/interface.h
	)

	strip_executable(${_projectname})
	minimal_symbols(${_projectname})
endmacro()
