/*
 * IMMY - Tiny ASCII plain text editor written within 500 lines of C++ Code
 */

#include <iostream>
#include <ncurses.h>
#include "buffer.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: test <filename>" << std::endl;
        exit(1);
    }

    initscr();
    keypad(stdscr, TRUE);   // Allow capturing special keys (like arrows, F1-F12)
    set_escdelay(25);
    use_default_colors();
    raw();
    noecho();
    curs_set(0);    // Hide the cursor
    start_color();
    init_pair(1, -1, -1);
    bkgd(COLOR_PAIR(1));

    std::string file_path = argv[1];
    buffer* buf = new buffer(file_path);

    buf->display_all();
    buf->position_cursor();
    buf->process_commands();

    endwin();
    return 0;
}
