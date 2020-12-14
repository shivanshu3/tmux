cmake_minimum_required(VERSION 3.13.4)
project(tmux)

add_compile_definitions(TMUX_VERSION="4.0-win32")
add_compile_definitions(HAVE_NCURSES_H)

find_library(NCURSES_LIB ncurses REQUIRED)
find_path(NCURSES_PATH ncurses.h REQUIRED)

find_library(EVENT_LIB event REQUIRED)
find_path(EVENT_PATH event.h REQUIRED)

find_library(C_LIB c REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${NCURSES_PATH})
include_directories(${EVENT_PATH})

add_subdirectory(compat)

set(CMD_PARSE_C ${CMAKE_CURRENT_BINARY_DIR}/cmd_parse_generated.c)

add_custom_command(
    OUTPUT ${CMD_PARSE_C}
    COMMAND yacc -o ${CMD_PARSE_C} cmd-parse.y
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# NOTE: This will have to be changed for Windows
add_library(tmux_misc STATIC
    osdep-linux.c
)

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
    format.c
    format-draw.c
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

target_link_libraries(tmux ${NCURSES_LIB})
target_link_libraries(tmux ${EVENT_LIB})
target_link_libraries(tmux tmux_compat)
target_link_libraries(tmux tmux_misc)

