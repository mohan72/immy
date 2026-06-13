#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <fstream>

#define BOTTOM (LINES-2)
#define RIGHT (COLS-1)
#define LINE_WIDTH (COLS)
#define TAB_WIDTH       4
#define BUFFER_COLOR    1
#define POPUP_COLOR     2

struct line {
    size_t start;
    size_t length;
};

struct dmask {
    size_t start;
    size_t length;
    bool altered;
};

class buffer {
    std::string text;
    std::string filename;
    std::vector<line> vline;
    std::string message_to_display;

    size_t topline;
    size_t idx_;
    bool dirty;

    public:
        buffer(std::string fname);
        ~buffer();
        std::string get_text() { return text; }
        std::vector<line> get_vline() { return vline; }
        size_t line_count() { return vline.size()-1; }
        size_t char_count() { return text.length(); }
        void reflow_text();
        void display_line(size_t row, size_t line_no);
        void display_all();
        size_t cursor_line(size_t idx);
        size_t cursor_col(size_t idx);
        int cursor_row(size_t idx);
        size_t find_word_end(size_t idx);
        size_t find_prev_word(size_t idx);
        void position_cursor();
        void update_status();
        void message(std::string msg);
        void process_commands();
        void save_buffer();
};

#endif
