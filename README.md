# immy

Plain text editor with word wrap within 500 lines of code (490 LOC as of last count).

# Compile and install

Requires a C++ compiler like gcc or clang. Use the 'Makefile' to compile and install:

'make' to compiles the source

'sudo make install' to install it in compatible UNIX-based systems

'make fclean' to completely remove the compiled objects and executable

# History

I have been trying to write my own plain text editor for quite some time; many years in fact. My source code archive is littered with quite a few half-finished editors in C, C++, Java, and Python. In fact, one of the first things I do when I learn a language is to write a plain text editor for the CLI. I choose the CLI because it is one of the most challenging environments to work in programming if you want to target screen management. The standards are too many and the different operating systems expose different key commands. The challenge can be quite frustrating. The ncurses library works differently in capturing and reporting keys in different environments.


I have installed and studied, to an extent, many small editor projects available online. The 'kilo' text editor project is something I love because of the challenge it sets itself: to write a text editor in less than 1000 lines of code. It is an exemplary project that has inspired many forks and alternate takes.


The main reason I wanted to write my own text editor was that you had to run through a few hoops to get word wrap in popular console text editors. Nano didn't even have word wrap until version 2+. All I wanted was an editor with proper word wrap at word+blank boundaries from the get go. This reason, of course, no longer exists - popular text editors have caught up; but I wanted to finish my project in any case.


The biggest challenge in writing a text editor is managing the text buffer. There are whole books dedicated to the art of text buffer design. That is the scale of the problem. There are many elegant designs, and some clever ones too. It all boils down to the kind of constraints we are bound to: small memory, poor speed, and so on. I learned C++ programmming by reading 'Type and Learn C++' by Martin Rinehart. It is a wonderful book that works its way through developing a code editor (no word wrap). It uses a doubly-linked list for the text buffer as the objective is to teach pointers. Managing a doubly-linked list with a char* for each line of text is the epitome of risk in C programming. One move out of bound and your program is guaranteed to crash. The book also introduced me to the slick Borland <conio.h> console library for screen management. It was a non-standard library and is no longer available. Ncurses is cross-platform and has become the industry standard.


My initial attempts to write my own editor were focused on using a doubly-linked list and the ncurses library. I was successful to an extent and had a decent editor at hand. Managing dynamic word wrap was difficult and proving troublesome: I was finding it difficult to keep track of the cursor. I was also challenging myself to keep the linecount low to attempt to stay within a thousand lines of code. And then I hit two breakthroughs:

1. I used a std::string object from the C++ standard library to manage the buffer as a single continuous stream of characters instead of a list of strings for each visible line. std::string can grow and shrink dynamically as the library takes care of malloc and delete itself. I never have to worry about pointers and segfaults were a thing of the past.

2. I used a global char index to track the cursor. With no more worry to track the line, the cursor just had to be adjusted by one character most of the times.


These were possible due to a single reflow function which read the whole string and created a table of starting and ending indices for each line. It takes very little memory and is quite fast as it just works on the in-memory buffer. I can run this anytime I wanted to, and all I had to know was which character held the cursor.


There are two downsides to the approach: 1. The whole file is in a single std::string buffer. This is not suitable for memory-constrained systems. 2. Since I run the reflow and screen update functions liberally, there is an occasional flicker of the screen when I type fast. This will have to be addressed sometime in the future.


Result: My own tiny little text editor with word wrap and quite a few editing functions (move by word, by line, delete word etc.) within 500 lines of code. I am calling it 'immy' which is a 'flick', or a 'tiny amount' in Tamil.
