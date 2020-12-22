cmake_minimum_required(VERSION 3.13.4)
project(tmux)

add_compile_definitions(TMUX_VERSION="4.0-win32")
add_compile_definitions(HAVE_STRNLEN)

if (MSVC)
    add_compile_options(/WX)
else()
    add_compile_options(-Werror)
endif()

if (MSVC)
    set(CURSES_LIB_NAME "pdcurses")
else()
    set(CURSES_LIB_NAME "ncurses")
endif()

find_library(CURSES_LIB ${CURSES_LIB_NAME} REQUIRED)
find_path(CURSES_PATH curses.h REQUIRED)
add_compile_definitions(HAVE_NCURSES_H)

message("Curses lib found: ${CURSES_LIB}")
message("Curses header found: ${CURSES_PATH}")

find_library(EVENT_LIB event REQUIRED)
find_path(EVENT_PATH event.h REQUIRED)
find_path(EVENT_CONFIG_PATH event2/event-config.h REQUIRED)

message("Event lib found: ${EVENT_LIB}")
message("Event header found: ${EVENT_PATH}")
message("Event-config header found: ${EVENT_CONFIG_PATH}")

if (MSVC)
    find_library(WINSOCK2_LIB Ws2_32 REQUIRED)
    message("WinSock2 lib found: ${WINSOCK2_LIB}")
else()
    find_library(UTIL_LIB util REQUIRED)
    find_library(M_LIB m REQUIRED)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CURSES_PATH})
include_directories(${EVENT_PATH})
include_directories(${EVENT_CONFIG_PATH})

if (MSVC)
    include_directories(${CMAKE_SOURCE_DIR}/win32_headers)
endif()

add_subdirectory(compat)

set(CMD_PARSE_C ${CMAKE_CURRENT_BINARY_DIR}/cmd_parse_generated.c)

add_custom_command(
    OUTPUT ${CMD_PARSE_C}
    COMMAND yacc ${CMAKE_SOURCE_DIR}/cmd-parse.y
    COMMAND mv y.tab.c ${CMD_PARSE_C}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${CMAKE_SOURCE_DIR}/cmd-parse.y
)

add_library(tmux_misc STATIC)
if (MSVC)
    target_sources(tmux_misc PUBLIC osdep-windows.c)
else()
    target_sources(tmux_misc PUBLIC osdep-linux.c)
endif()

add_executable(tmux
    ${CMD_PARSE_C}
    alerts.c
    arguments.c
    attributes.c
    cfg.c
    client.c
    cmd-attach-session.c
    cmd-bind-key.c
    cmd-break-pane.c
    cmd-capture-pane.c
    cmd-choose-tree.c
    cmd-command-prompt.c
    cmd-confirm-before.c
    cmd-copy-mode.c
    cmd-detach-client.c
    cmd-display-menu.c
    cmd-display-message.c
    cmd-display-panes.c
    cmd-find-window.c
    cmd-find.c
    cmd-if-shell.c
    cmd-join-pane.c
    cmd-kill-pane.c
    cmd-kill-server.c
    cmd-kill-session.c
    cmd-kill-window.c
    cmd-list-buffers.c
    cmd-list-clients.c
    cmd-list-keys.c
    cmd-list-panes.c
    cmd-list-sessions.c
    cmd-list-windows.c
    cmd-load-buffer.c
    cmd-lock-server.c
    cmd-move-window.c
    cmd-new-session.c
    cmd-new-window.c
    cmd-paste-buffer.c
    cmd-pipe-pane.c
    cmd-queue.c
    cmd-refresh-client.c
    cmd-rename-session.c
    cmd-rename-window.c
    cmd-resize-pane.c
    cmd-resize-window.c
    cmd-respawn-pane.c
    cmd-respawn-window.c
    cmd-rotate-window.c
    cmd-run-shell.c
    cmd-save-buffer.c
    cmd-select-layout.c
    cmd-select-pane.c
    cmd-select-window.c
    cmd-send-keys.c
    cmd-set-buffer.c
    cmd-set-environment.c
    cmd-set-option.c
    cmd-show-environment.c
    cmd-show-messages.c
    cmd-show-options.c
    cmd-source-file.c
    cmd-split-window.c
    cmd-swap-pane.c
    cmd-swap-window.c
    cmd-switch-client.c
    cmd-unbind-key.c
    cmd-wait-for.c
    cmd.c
    colour.c
    control-notify.c
    control.c
    environ.c
    file.c
    format-draw.c
    format.c
    grid-view.c
    grid.c
    input-keys.c
    input.c
    job.c
    key-bindings.c
    key-string.c
    layout-custom.c
    layout-set.c
    layout.c
    log.c
    menu.c
    mode-tree.c
    names.c
    notify.c
    options-table.c
    options.c
    paste.c
    popup.c
    proc.c
    regsub.c
    resize.c
    screen-redraw.c
    screen-write.c
    screen.c
    server-client.c
    server-fn.c
    server.c
    session.c
    spawn.c
    status.c
    style.c
    tmux.c
    tty-acs.c
    tty-features.c
    tty-keys.c
    tty-term.c
    tty.c
    utf8.c
    window-buffer.c
    window-client.c
    window-clock.c
    window-copy.c
    window-customize.c
    window-tree.c
    window.c
    xmalloc.c
)

target_link_libraries(tmux tmux_compat)
target_link_libraries(tmux tmux_misc)
target_link_libraries(tmux ${CURSES_LIB})
target_link_libraries(tmux ${EVENT_LIB})

if (MSVC)
    target_link_libraries(tmux ${WINSOCK2_LIB})
else()
    target_link_libraries(tmux ${UTIL_LIB})
    target_link_libraries(tmux ${M_LIB})
endif()
