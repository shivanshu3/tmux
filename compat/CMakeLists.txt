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

