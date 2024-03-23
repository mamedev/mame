# Linenoise

## What's different in this fork?

- Win32 console
- full utf8 support (what about utf8 on windows)
- insert control characters
- now with multiline

## How do I include linenoise line editing support in my application?

From the Makefile:

  linenoise-ship.c simplifies delivery of linenoise support

  simple copy linenoise-ship.c to linenoise.c in your application, and also linenoise.h

  * If you want win32 support, also copy linenoise-win32.c
  * If you never want to support utf-8, you can omit utf8.h and utf8.c

To enable utf-8 support, define USE_UTF8

## Where do I get it?

Get it here: [https://github.com/msteveb/linenoise](https://github.com/msteveb/linenoise)

## Key bindings

This version supports the following key bindings:

    ctrl-j, Enter     Return the current line as the result
    ctrl-a, Home      Go to the start of the line
    ctrl-e, End       Go to the end of the line
    ctrl-u            Delete to beginning of line
    ctrl-k            Delete to end of line
    ctrl-y            Insert previously deleted chars at cursor
    ctrl-l            Clear screen
    ctrl-c            Quit
    ctrl-z            Exit to background (Unix only)
    ctrl-h, Backspace Delete char to left of cursor
    ctrl-d            With empty line -  return
    ctrl-d, Del       Delete char to right of cursor
    meta-b            Move word left
    meta-f            Move word right
    ctrl-w            Delete word to left
    ctrl-t            Transpose char and cursor and char to left of cursor, then move right
    ctrl-v            Insert next char as control character
    ctrl-b, Left      Move one char left
    ctrl-f, Right     Move one char right
    ctrl-p, Up        Move to previous history line
    ctrl-n, Down      Move to next history line
    Page-Up           Move to start of history
    Page-Down         Move to end of history
    Tab               Tab complete
    ctrl-r            Begin reverse incremental search

In reverse incremental search:

    Normal char       Add char to incremental search word
    ctrl-h, Backspace Remove last char from incremental search word
    ctrl-p, Up        Move to previous match
    ctrl-n, Down      Move to next match
    ctrl-g, ctrl-c    Return to normal mode with empty line
    Any other key     Return to normal mode with the current line and process the key

--------------------------------------------------------

## Original README below

Can a line editing library be 20k lines of code?

A minimal, zero-config, BSD licensed, readline replacement.

News: linenoise now includes minimal completion support, thanks to Pieter Noordhuis (@pnoordhuis).

News: linenoise is now part of [Android](http://android.git.kernel.org/?p=platform/system/core.git;a=tree;f=liblinenoise;h=56450eaed7f783760e5e6a5993ef75cde2e29dea;hb=HEAD Android)!

## Can a line editing library be 20k lines of code?

Line editing with some support for history is a really important feature for command line utilities. Instead of retyping almost the same stuff again and again it's just much better to hit the up arrow and edit on syntax errors, or in order to try a slightly different command. But apparently code dealing with terminals is some sort of Black Magic: readline is 30k lines of code, libedit 20k. Is it reasonable to link small utilities to huge libraries just to get a minimal support for line editing?

So what usually happens is either:

 * Large programs with configure scripts disabling line editing if readline is not present in the system, or not supporting it at all since readline is GPL licensed and libedit (the BSD clone) is not as known and available as readline is (Real world example of this problem: Tclsh).
 * Smaller programs not using a configure script not supporting line editing at all (A problem we had with Redis-cli for instance).
 
The result is a pollution of binaries without line editing support.

So I spent more or less two hours doing a reality check resulting in this little library: is it *really* needed for a line editing library to be 20k lines of code? Apparently not, it is possibe to get a very small, zero configuration, trivial to embed library, that solves the problem. Smaller programs will just include this, supporing line editing out of the box. Larger programs may use this little library or just checking with configure if readline/libedit is available and resorting to linenoise if not.

## Terminals, in 2010.

Apparently almost every terminal you can happen to use today has some kind of support for VT100 alike escape sequences. So I tried to write a lib using just very basic VT100 features. The resulting library appears to work everywhere I tried to use it.

Since it's so young I guess there are a few bugs, or the lib may not compile or work with some operating system, but it's a matter of a few weeks and eventually we'll get it right, and there will be no excuses for not shipping command line tools without built-in line editing support.

The library is currently less than 2000 lines of code. In order to use it in your project just look at the *example.c* file in the source distribution, it is trivial. Linenoise is BSD code, so you can use both in free software and commercial software.

## Tested with...

 * Linux text only console ($TERM = linux)
 * Linux KDE terminal application ($TERM = xterm)
 * Linux xterm ($TERM = xterm)
 * Mac OS X iTerm ($TERM = xterm)
 * Mac OS X default Terminal.app ($TERM = xterm)
 * OpenBSD 4.5 through an OSX Terminal.app ($TERM = screen)
 * IBM AIX 6.1
 * FreeBSD xterm ($TERM = xterm)

Please test it everywhere you can and report back!

## Let's push this forward!

Please fork it and add something interesting and send me a pull request. What's especially interesting are fixes, new key bindings, completion.

Send feedbacks to antirez at gmail
