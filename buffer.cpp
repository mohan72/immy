/*
 * BUFFER.CPP - Implementation of Buffer Class with processing of user commands
 */

#include <cstddef>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ncurses.h>
#include "keydefs.h"
#include "buffer.h"

buffer::buffer (std::string fname) {
    filename = fname;
    std::ifstream file_stream(filename);
    if (file_stream) {
        std::stringstream buffer;
        text = (buffer << file_stream.rdbuf(), buffer.str());
        if (text.empty() || text.back() != '\n') {
            text.push_back('\n');
            dirty = true;
        }
    } else {
        text = "\n";
        dirty = true;
    }

    //Change Tabs to spaces if not already
    const std::string spaces(TAB_WIDTH, ' ');
    size_t pos = 0;
    while ((pos = text.find('\t', pos)) != std::string::npos) {
        text.replace(pos, 1, spaces);
        pos += spaces.length();
    }
    reflow_text();
    set_display_mask();
}

constexpr uint32_t buffer::hash(std::string_view str) {
    uint32_t hash_ = 2166136261U; // FNV offset basis
    for (char c : str) {
        hash_ ^= static_cast<uint8_t>(c);
        hash_ *= 16777619U;       // FNV prime
    }
    return hash_;
}

void buffer::set_display_mask() {
    mask_topline = topline;
    displaymask.clear();
    for (size_t i = 0; i < line_count(); i++) { displaymask.push_back({hash(text.substr(vline[i].start, vline[i].length).c_str())}); }
}

// Custom Word Wrap Tokenizer (Greedy Reflow Algorithm)
void buffer::reflow_text() {
    vline.clear();

    if (text.empty()) {
        vline.push_back({0, 0});
        return;
    }

    size_t idx = 0;
    size_t total_len = text.length();
    size_t current_row_start = 0;
    size_t current_row_len = 0;

    while (idx < total_len) {
        size_t token_start = idx;

        while (idx < total_len && text[idx] != ' ' && text[idx] != '\n') { idx++; }
        while (idx < total_len && text[idx] == ' ') { idx++; }
        if (idx < total_len && text[idx] == '\n') { idx++; }

        size_t token_len = idx - token_start;

        if (token_len > static_cast<size_t>(LINE_WIDTH)) {
            if (current_row_len > 0) {
                vline.push_back({current_row_start, current_row_len});
                current_row_start = token_start;
                current_row_len = 0;
            }

            size_t broken_idx = token_start;
            while (broken_idx < idx) {
                size_t chunk = std::min(std::min(static_cast<size_t>(LINE_WIDTH), idx - broken_idx), total_len - broken_idx);
                vline.push_back({broken_idx, chunk});
                broken_idx += chunk;
            }
            current_row_start = idx;
            continue;
        }

        if (current_row_len + token_len <= static_cast<size_t>(LINE_WIDTH)) {
            current_row_len += token_len;
        } else {
            vline.push_back({current_row_start, current_row_len});
            current_row_start = token_start;
            current_row_len = token_len;
        }

        if (idx > 0 && text[idx - 1] == '\n') {
            vline.push_back({current_row_start, current_row_len});
            current_row_start = idx;
            current_row_len = 0;
        }
    }

    if (current_row_len > 0 || current_row_start == total_len) { vline.push_back({current_row_start, current_row_len}); }

    if (!vline.empty()) {
        const auto& last_row = vline.back();
        if (last_row.length == static_cast<size_t>(LINE_WIDTH) &&
            last_row.start + last_row.length == total_len &&
            text.back() != '\n') {
            vline.push_back({total_len, 0});
        }
    }
}

void buffer::display_all() {
    int y = 0;
    size_t cline = topline;
    bkgd(COLOR_PAIR(BUFFER_COLOR));    // Clean BG
    clear();
    wattron(stdscr, BUFFER_COLOR);
    while(true) {
        if (cline >= line_count()) { break; }
        if (y > BOTTOM) { break; }
        if (cline < line_count()) { mvprintw(y, 0, "%s", text.substr(vline[cline].start, vline[cline].length).c_str()); }
        cline++;
        y++;
    }
    wattroff(stdscr, BUFFER_COLOR);
    refresh();
}

void buffer::display_changes() {
    if (mask_topline != topline) {
        display_all();
        set_display_mask();
        return;
    }
    size_t vl;  //visual line index for display
    size_t lc = line_count();
    wattron(stdscr, BUFFER_COLOR);
    for (int i = 0; i <= BOTTOM; i++) {  //go through every screen line
        vl = topline + i;
        if (vl < lc) {
            if (vl >= displaymask.size() || displaymask[vl] != hash(text.substr(vline[vl].start, vline[vl].length).c_str())) {    //check if line has to be updated
                //update - draw clear line & display string
                move(i, 0);
                clrtoeol();
                mvprintw(i, 0, "%s", text.substr(vline[vl].start, vline[vl].length).c_str());
            }
        } else {
            move(i, 0);
            clrtoeol();
        }
    }
    wattroff(stdscr, BUFFER_COLOR);
    set_display_mask();
}

size_t buffer::cursor_line(size_t idx) {
    if (idx >= text.length() || vline.empty()) { return 0; }

    for (size_t line = 0; line < line_count(); line++) {
        if ((idx >= vline[line].start) && (idx < (vline[line].start + vline[line].length))) { return line; }
    }
    return 0;
}

size_t buffer::cursor_col(size_t idx) {
    if (idx >= text.length() || vline.empty()) { return 0; }

    for (size_t line = 0; line < line_count(); line++) {
        if ((idx >= vline[line].start) && (idx < (vline[line].start + vline[line].length))) { return idx - vline[line].start; }
    }
    return 0;
}

void buffer::save_buffer() {
    if (std::ofstream out{filename}) {
        out << text;
    } else {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
        exit(1);
    }
}

size_t buffer::find_word_end(size_t idx) {
    size_t cpos = idx;
    while (true) {
        while (text[cpos] != ' ' && text[cpos] != '\n') { cpos++; }
        while (text[cpos] == ' ') { cpos++; }
        break;
    }
    return cpos;
}

size_t buffer::find_prev_word(size_t idx) {
    size_t cpos = idx;
    if (cpos > 0 && text[cpos-1] == ' ') { cpos--; }
    while (true) {
        while (cpos > 0 && text[cpos] == ' ') { cpos--; }
        while (cpos > 0 && text[cpos] != ' ' && text[cpos] != '\n') { cpos--; }
        if (text[cpos] == ' ' || text[cpos] == '\n') { cpos++; }
        break;
    }
    return cpos;
}

void buffer::position_cursor() {
    if (cursor_row(idx_) < 0) {
        topline += cursor_row(idx_);
    } else if (cursor_row(idx_) > BOTTOM) {
        topline += (cursor_row(idx_) - BOTTOM);
    }
    display_changes();
    update_status();
    move(cursor_row(idx_), cursor_col(idx_));
    refresh();
}

void buffer::update_status() {
    wattron(stdscr, A_DIM);
    mvaddstr(BOTTOM+1, 0, "");
    clrtoeol();
    size_t l_no = cursor_line(idx_) + 1;    //Use common count
    size_t col = cursor_col(idx_) + 1;  //Use common count
    std::string status_message = " R:[" + std::to_string(l_no) + "] C:[" + std::to_string(col) + "] ";
    status_message += "CHAR:[" + std::to_string(static_cast<int>(text[idx_])) + "] ";
    status_message += "CHARS:[" + std::to_string(text.length()) + "] [F1 - Help] ";
    if (message_to_display != "") {
        status_message += "MSG:[" + message_to_display + "]";
        message_to_display = "";
    } else {
        status_message += "[" + (filename + (dirty ? "*" : "")) + "]";
    }
    mvprintw(BOTTOM+1, 0, "%s", status_message.c_str());
    wattroff(stdscr, A_DIM);
}

void buffer::process_commands() {
    size_t command;
    bool done = false;
    bool save_override = false;
    curs_set(1);

    while (!done) {
        command = getch();
        if (save_override && command != CTRL_Q) { save_override = false; }
        switch (command) {
            case CTRL_Q:
                if (dirty && !save_override) {
                    save_override = true;
                    message_to_display = "Exit again to ignore changes.";
                    position_cursor();
                } else {
                    done = true;
                }
                break;
            case KEY_RIGHT:
                idx_ = std::min(idx_+1, text.length()-1);
                position_cursor();
                lastcol = cursor_col(idx_);
                break;
            case KEY_SRIGHT:
                if (text[idx_] == '\n' && idx_ < text.length()-1) { //Move cursor just once if sitting on a Newline
                    idx_++;
                } else {
                    idx_ = find_word_end(idx_);
                }
                position_cursor();  //Required in case scroll happened
                lastcol = cursor_col(idx_);
                break;
            case KEY_LEFT:
                if (idx_ > 0) { idx_--; }
                position_cursor();
                lastcol = cursor_col(idx_);
                break;
            case KEY_SLEFT:
                if (text[idx_] == '\n' && idx_ > 0) {
                    idx_--;
                } else if (idx_ > 0 && text[idx_-1] == '\n') {
                    idx_--;
                } else {
                    idx_ = find_prev_word(idx_);
                }
                position_cursor();  //Required in case scroll happened
                lastcol = cursor_col(idx_);
                break;
            case KEY_HOME:
                idx_ -= cursor_col(idx_);
                position_cursor();
                lastcol = cursor_col(idx_);
                break;
            case KEY_END:
                idx_ += (vline[cursor_line(idx_)].length-1 - cursor_col(idx_));
                position_cursor();
                lastcol = cursor_col(idx_);
                break;
            case KEY_DOWN: {
                if (cursor_line(idx_) < (line_count()-1)) {
                    idx_ = vline[cursor_line(idx_)+1].start;
                } else {
                    idx_ = text.length()-1;
                }
                position_cursor();
                idx_ = std::min(idx_ + lastcol, idx_ + (vline[cursor_line(idx_)].length-1 - cursor_col(idx_)));
                position_cursor();
                break;
            }
            case KEY_UP:
                if (cursor_line(idx_) > 0) {
                    idx_ = vline[cursor_line(idx_)-1].start;
                    position_cursor();
                    idx_ = std::min(idx_ + lastcol, idx_ + (vline[cursor_line(idx_)].length-1 - cursor_col(idx_)));
                    position_cursor();
                }
                break;
            case KEY_DC:
                if (idx_ != text.length()-1) {
                    text.erase(idx_, 1);
                    dirty = true;
                    reflow_text();
                    position_cursor();
                    lastcol = cursor_col(idx_);
                }
                break;
            case KEY_BACKSPACE:
            case BKSPACE_A:
            case BKSPACE_B:
                if (idx_ > 0) {
                    idx_--;
                    text.erase(idx_, 1);
                    dirty = true;
                    reflow_text();
                    position_cursor();
                    lastcol = cursor_col(idx_);
                }
                break;
            case ENTER:
                text.insert(idx_, 1, '\n');
                idx_++;
                dirty = true;
                reflow_text();
                position_cursor();
                lastcol = cursor_col(idx_);
                break;
            case CTRL_W: {   //Delete to end of word
                text.erase(idx_, (find_word_end(idx_) - idx_));
                dirty = true;
                reflow_text();
                position_cursor();
                lastcol = cursor_col(idx_);
                break;
            }
            case CTRL_K: {
                size_t cl = cursor_line(idx_);
                text.erase(vline[cl].start, vline[cl].length - 1);
                idx_ = vline[cl].start;
                if (cl < line_count()-1) { text.erase(idx_, 1); }
                dirty = true;
                reflow_text();
                position_cursor();
                lastcol = cursor_col(idx_);
                break;
            }
            case TAB: {
                const std::string spaces(TAB_WIDTH, ' ');
                text.insert(idx_, spaces);
                dirty = true;
                idx_ += TAB_WIDTH;
                reflow_text();
                position_cursor();
                lastcol = cursor_col(idx_);
                break;
            }
            case CTRL_S:
                save_buffer();
                dirty = false;
                position_cursor();
                break;
            case KEY_F(1):
                display_help();
                position_cursor();
                break;
            default:
                if (command >= 32 && command <= 126) {
                    text.insert(idx_, 1, command);
                    dirty = true;
                    idx_++;
                    reflow_text();
                    position_cursor();
                    lastcol = cursor_col(idx_);
                }
                break;
        }
    }
}

void buffer::display_help() {
    WINDOW *popup = newwin(9, 44, (LINES - 9) / 2, (COLS - 44) / 2);
    box(popup, 0, 0); // Draw a border around the window

    mvwprintw(popup, 1, 2, "             immy v0.95");
    mvwprintw(popup, 3, 2, "CTRL+Q - Quit     | SHIFT+LT - Prev Word");
    mvwprintw(popup, 4, 2, "CTRL+S - Save     | SHIFT+RT - Next Word");
    mvwprintw(popup, 5, 2, "CTRL+W - Del Word | CTRL+K   - Del Line");
    mvwprintw(popup, 7, 2, "            Press any key to continue...");
    wrefresh(popup);

    wgetch(popup); // Wait for user to press any key
    werase(popup);      // Clear the popup window content
    wrefresh(popup);    // Force removal from the terminal display
    delwin(popup);      // Deallocate the popup window memory
    touchwin(stdscr);   // Mark the main screen as changed so ncurses redraws it entirely
}
