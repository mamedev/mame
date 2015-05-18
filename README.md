
# **MAME** #

[![Join the chat at https://gitter.im/mamedev/mame](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mamedev/mame?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)


What is MAME?
=============

MAME stands for Multiple Arcade Machine Emulator.

MAME's purpose is to preserve decades of video-game history. As gaming technology continues to rush forward, MAME prevents these important "vintage" games from being lost and forgotten. This is achieved by documenting the hardware and how it functions. The source code to MAME serves as this documentation. The fact that the games are playable serves primarily to validate the accuracy of the documentation (how else can you prove that you have recreated the hardware faithfully?).


What is MESS?
=============

MESS (Multi Emulator Super System) is the sister project of MAME. MESS documents the hardware for a wide variety of (mostly vintage) computers, video game consoles, and calculators, as MAME does for arcade games.

The MESS and MAME projects live in the same source repository and share much of the same code, but are different build targets.


How to compile?
=============

If you're on a *nix system, it could be as easy as typing

```
make
```

for a MAME build, or

```
make TARGET=mess
```

for a MESS build (provided you have all the [prerequisites](http://forums.bannister.org/ubbthreads.php?ubb=showflat&Number=35138)).

For Windows users, we provide a ready-made [build environment](http://mamedev.org/tools/) based on MinGW-w64. [Visual Studio builds](http://wiki.mamedev.org/index.php?title=Building_MAME_using_Microsoft_Visual_Studio_compilers) are also possible.




Where can I find out more?
=============

* [Official MAME Development Team Site](http://mamedev.org/) (includes binary downloads for MAME and MESS, wiki, forums, and more)
* [Official MESS Wiki](http://www.mess.org/)
* [MAME Testers](http://mametesters.org/) (official bug tracker for MAME and MESS)


Contributing
=============

## Coding standard

MAME source code should be viewed and edited with your editor set to use four spaces per tab.  Tabs are used for initial indentation of lines, with one tab used per indentation level.  Spaces are used for other alignment within a line.

Try to follow these rules to help keep the code consistent and readable:
* Indent code one level for each level of scope
* Don't indent for things that don't change the level of scope (e.g. case labels)
* Outdent labels (including case labels) by one level
* Indent line continuations by a different amount to a level of scope so they can be visually distinguished
* Place the braces that open and close compound statements on their own lines to make them easier to visually balance (with the exception of compound statements on a single line)
* Don't visually separate parts of an if/else tree with comments or whitespace, compound may help tie things together
* If one branch of an if/else tree requires a compound statement, use compound statements for all branches
* Wrap macros in do/while to make them play nicer with surrounding code
* Use parentheses when using macro arguments to make semantics more predictable

Here's an example showing many of these things:
```c++
#define SOME_MACRO(a, b) do { something((a) * (b)); } while (0)

bool function(
        int     intparam,
        long    longparam)
{
    switch (intparam)
    {
    case val1:
    case val2:
        {
            bool scoped(initialisation());
            call_something(scoped);
        }
        break;
    case val3:
        unscoped_call(longparam);
        break;
    default:
        goto failure;
    }

    if (cond1) { trivial(); stuff(); }
    else { also(); trivial(); }

    if (cond2)
    {
        // The compound statements braces guide you to the next part of the tree
        something();
    }
    else if (cond3)
    {
        another_call();
    }
    else
    {
        the_default();
    }

    many_params(
            first(),
            second(),
            third(
                also(),
                has(),
                several()),
            fourth());

    return true;

failure:
    return false;
}
```
