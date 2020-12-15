add_library(tmux_compat STATIC
    strtonum.c
    strlcat.c
    strlcpy.c
    getopt.c
    closefrom.c
    base64.c
    fgetln.c
    getprogname.c
    fdforkpty.c
    imsg.c
    imsg-buffer.c
    vis.c
    unvis.c
    getdtablecount.c
    recallocarray.c
    setproctitle.c
)

