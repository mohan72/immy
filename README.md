# immy

Plain text editor with dynamic word-wrap within 500 lines of code (479 LOC as of last count).

# Compile and install

Requires a C++ compiler like gcc or clang. Use the 'Makefile' to compile and install:

'make' to compiles the source

'sudo make install' to install it in compatible UNIX-based systems

'make fclean' to completely remove the compiled objects and executable

# History

I have been trying to write my own plain text editor for many years. My source code archive is littered with half-finished editors in C, C++, Java, and Python, all abandoned at various stages of development. One of the first things I do when I learn a new language, almost as a ritual, is to start writing a plain text editor for the CLI. I choose the CLI because it is one of the most challenging environments to work with for screen management. The standards are too many and operating systems add their own quirks: without additional setup MacOS doesn't report the Backspace key as a special key; Linux requires the UNICODE -lncursesw lib to be linked instead of the standard -lncurses lib even to capture and display non-wide characters correctly; the ncurses library reports different keys in different environments, etc.


I have installed, tried and studied, to a certain extent, many small editor projects available online. I love the 'kilo' text editor due to its self-imposed constraint: to write a usable plain text editor in less than 1000 lines of code. Kilo is one of the inspirations for the 'immy' project.


The main reason I started writing my own text editor was that I had to read through stack overflow to setup proper word-wrap in popular text editors (nano and vim). Nano didn't even have word wrap until version 2+. All I wanted was an editor with word-wrap at word+blank boundaries from the get go.


The biggest challenge and design decision in developing a text editor is managing the text buffer. There are whole books dedicated to the art of text buffer design - such is the scale of the problem. There are many elegant designs; some more clever than the others. You have to be a pointers* champion to deal with most of the well-known solutions. I learned C++ programmming by reading 'Type and Learn C++' by Martin Rinehart. It is a wonderful book that works its way by developing a plain text editor (no word wrap). It uses a doubly-linked list for the text buffer. Managing a doubly-linked list with a char* for each line of text, I would say, is the epitome of risk in C programming. One move out of bound and your program segfaults. The book also introduced me to the slick <conio.h> console library from Borland, a non-standard library that is not available in modern compilers. Ncurses is cross-platform and is now the industry standard for console-based screen management.


My initial attempts to write a plain-text editor used a doubly-linked list and the ncurses library. I was successful to an extent and even had a decent editor at hand. Managing dynamic word-wrap was difficult and proved problematic: It was difficult to keep track of the cursor. I was also challenging myself to keep the linecount low in order to stay within 1000 lines of code. And then I hit two breakthroughs:

1. I used a std::string object from the C++ standard library as the text buffer instead of a linked-list of char* for each visible line. std::string can grow and shrink dynamically, as the library takes care of malloc and delete. I never had to worry about pointers, and segfaults were a thing of the past.

2. I used a global index to track the cursor as a specific character in the buffer. With no more necessity to track the specific line where the cursor was located, screen management became a breeze. This was possible due to the implementation of a single 'reflow' function that read the whole buffer string and created a vector of starting positions and length of each line. It takes a tiny amount of memory and is quite fast working through the in-memory buffer.


There are two downsides to the approach: 1. The whole file is in a single std::string buffer. This is not suitable for memory-constrained systems. 2. Since I run the reflow and screen update functions liberally, there is an occasional flicker of the screen when I type fast. This has now been completely eradicated as the display routine only updates screen lines that have changed.


Result: My own tiny little text editor with dynamic word-wrap and a small set of useful editing features (move by word, by line, delete word etc.), all within 500 lines of code. I am calling it 'immy' which is a 'flick', or a 'tiny bit' in Tamil.
