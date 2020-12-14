include_directories(../)

add_library(compat STATIC
    strtonum.c
    strlcat.c
    strlcpy.c
)

