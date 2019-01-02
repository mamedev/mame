Multiple Configuration Files
============================

MAME has a very powerful configuration file system that can allow you to tweak settings on a per-game, per-system, or even per-monitor type basis, but requires careful thought about how you arrange your configs.

.. _advanced-multi-CFG:

Order of Config Loading
-----------------------

1. The command line is parsed first, and any settings passed that way *will take priority over anything in an INI file*.
2. **mame.ini** (or other platform INI; e.g. **mess.ini**) is parsed twice.
    The first pass may change various path settings, so the second pass is done to see if there is a valid config file at that new location (and if so, change settings using that file)
3. **debug.ini** if in debug mode.
    This is an advanced config file, most people won't need to use it or be concerned by it.
4. System-specific INI files where appropriate (e.g. **neogeo_noslot.ini** or **cps2.ini**)
    As an example, Street Fighter Alpha is a CPS2 game, and so **cps2.ini** would be loaded here.
5. Monitor orientation INI file (either **horizont.ini** or **vertical.ini**)
    Pac-Man, for one example, is a vertical monitor setup, so it would load **vertical.ini**. Street Fighter Alpha is a horizontal game, so it loads **horizont.ini**.
6. System-type INI files (**arcade.ini**, **console.ini**, **computer.ini**, or **othersys.ini**)
    Both Pac-Man and Street Fighter Alpha are arcade games, so **arcade.ini** would be loaded here. Atari 2600 would load **console.ini**.
7. Screen-type INI file  (**vector.ini** for vector games, **raster.ini** for raster games, **lcd.ini** for LCD games)
    Pac-Man and Street Fighter Alpha are raster, so **raster.ini** gets loaded here. Tempest is a vector monitor game, and **vector.ini** would be loaded here.
8. Source INI files.
    This is an advanced config file, most people won't need to use it and it can be safely ignored.
    MAME will attempt to load **source/sourcefile.ini** and **sourcefile.ini**, where sourcefile is the actual filename of the source code file.
    *mame -listsource <game>* will show the source file for a given game.

    For instance, Banpresto's Sailor Moon, Atlus's Dodonpachi, and Nihon System's Dangun Feveron all share a large amount of hardware and are grouped into the **cave.c** file, meaning they all parse **source/cave.ini**
9. Parent INI file.
    For example, if running Pac-Man, which is a clone of Puck-Man, it'd be **puckman.ini**
10. Driver INI file.
    Using our previous example of Pac-Man, this would be **pacman.ini**.


Examples of Config Loading Order
--------------------------------

1. Alcon, which is the US clone of Slap Fight. (*mame alcon*)
    Command line, mame.ini, vertical.ini, arcade.ini, raster.ini, slapfght.ini, and lastly alcon.ini (*remember command line parameters take precedence over all else!*)

2. Super Street Fighter 2 Turbo (*mame ssf2t*)
    Command line, mame.ini, horizont.ini, arcade.ini, raster.ini, cps2.ini, and lastly ssf2t.ini (*remember command line parameters take precedence over all else!*)


Tricks to Make Life Easier
--------------------------

Some users may have a wall-mounted or otherwise rotatable monitor, and may wish to actually play vertical games with the rotated display. The easiest way to accomplish this is to put your rotation modifiers into **vertical.ini**, where they will only affect vertical games.

[todo: more practical examples]
