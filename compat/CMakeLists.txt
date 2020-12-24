add_library(tmux_compat STATIC
    base64.c
    closefrom.c
    fdforkpty.c
    fgetln.c
    freezero.c
    getdtablecount.c
    getopt.c
    getprogname.c
    imsg-buffer.c
    imsg.c
    recallocarray.c
    setproctitle.c
    strlcat.c
    strlcpy.c
    strtonum.c
    unvis.c
    vis.c
)

if (TMUX_USE_CXX OR MSVC)
	get_target_property(SOURCE_FILES_C_TMUX_COMPAT tmux_compat SOURCES)
	set_source_files_properties(${SOURCE_FILES_C_TMUX_COMPAT} PROPERTIES LANGUAGE CXX)
endif()

