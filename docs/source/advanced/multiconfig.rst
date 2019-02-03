Multiple Configuration Files
============================

MAME has a very powerful configuration file system that can allow you to tweak settings on a per-game, per-system, or even per-monitor type basis, but requires careful thought about how you arrange your configs.

.. _advanced-multi-CFG:

Order of Config Loading
-----------------------

1. The command line is parsed first, and any settings passed that way *will take
   precedence over anything in an INI file*.

2. ``mame.ini`` (or other platform INI; e.g. ``mess.ini``) is parsed twice.  The
   first pass may change various path settings, so the second pass is done to
   see if there is a valid configuration file at that new location (and if so,
   change settings using that file).

3. ``debug.ini`` if the debugger is enabled.  This is an advanced config file,
   most people won't need to use it or be concerned by it.

4. Screen orientation INI file (either ``horizont.ini`` or ``vertical.ini``).
   For example Pac-Man has a vertical screen, so it loads ``vertical.ini``,
   while Street Fighter Alpha uses a horizontal screen, so it loads
   ``horizont.ini``.

   Systems with no monitors, multiple monitors with different orientations, or
   monitors connected to slot devices will usually load ``horizont.ini``.

5. System type INI file (``arcade.ini``, ``console.ini``, ``computer.ini``, or
   ``othersys.ini``).  Both Pac-Man and Street Fighter Alpha are arcade games,
   so ``arcade.ini`` will be loaded here, while Atari 2600 will load
   ``console.ini`` as it is a home game console.

6. Monitor type INI file (``vector.ini`` for vector monitors, ``raster.ini`` for
   CRT raster monitors, or ``lcd.ini`` for LCD/EL/plasma matrix monitors).
   Pac-Man and Street Fighter Alpha use raster CRTs, so ``raster.ini`` is loaded
   here, while Tempest uses a vector monitor, so ``vector.ini`` is loaded here.

   For systems that have multiple monitor types, such as House Mannequin with
   its CRT raster monitor and dual LCD matrix monitors, the INI file relevant to
   the first monitor is used (``raster.ini`` in this case).  Systems without
   monitors or with other kinds of monitors will not load an INI file for this
   step.

7. Driver source file INI file.  MAME will attempt to load
   ``source/``\ *<sourcefile>*\ ``.ini`` where *<sourcefile>* is the base name
   of the source code file where the system driver is defined.  A system's
   source file can be found using **mame -listsource <pattern>** at the command
   line.

   For instance, Banpresto's Sailor Moon, Atlus's Dodonpachi, and Nihon System's
   Dangun Feveron all run on similar hardware and are defined in the
   ``cave.cpp`` source file, so they will all load ``source/cave.ini`` at this
   step.

8. BIOS set INI file (if applicable).  For example The Last Soldier uses the
   Neo-Geo MVS BIOS, so it will load ``neogeo.ini``.  Systems that don't use a
   BIOS set won't load an INI file for this step.

9. Parent system INI file.  For example The Last Soldier is a clone of The Last
   Blade / Bakumatsu Roman - Gekka no Kenshi, so it will load ``lastblad.ini``.
   Parent systems will not load an INI file for this step.

10. System INI file.  Using the previous example, The Last Soldier will load
    ``lastsold.ini``.


Examples of Config Loading Order
--------------------------------

* Brix, which is a clone of Zzyzzyxx. (**mame brix**)

  1. Command line
  2. ``mame.ini`` (global)
  3. (debugger not enabled, no extra INI file loaded)
  4. ``vertical.ini`` (screen orientation)
  5. ``arcade.ini`` (system type)
  6. ``raster.ini`` (monitor type)
  7. ``source/jack.ini`` (driver source file)
  8. (no BIOS set)
  9. ``zzyzzyxx.ini`` (parent system)
  10. ``brix.ini`` (system)

* Super Street Fighter 2 Turbo (**mame ssf2t**)

  1. Command line
  2. ``mame.ini`` (global)
  3. (debugger not enabled, no extra INI file loaded)
  4. ``horizont.ini`` (screen orientation)
  5. ``arcade.ini`` (system type)
  6. ``raster.ini`` (monitor type)
  7. ``source/cps2.ini`` (driver source file)
  8. (no BIOS set)
  9. (no parent system)
  10. ``ssf2t.ini`` (system)

* Final Arch (**mame finlarch**)

  1. Command line
  2. ``mame.ini`` (global)
  3. (debugger not enabled, no extra INI file loaded)
  4. ``horizont.ini`` (screen orientation)
  5. ``arcade.ini`` (system type)
  6. ``raster.ini`` (monitor type)
  7. ``source/stv.ini`` (driver source file)
  8. ``stvbios.ini`` (BIOS set)
  9. ``smleague.ini`` (parent system)
  10. ``finlarch.ini`` (system)

*Remember command line parameters take precedence over all else!*


Tricks to Make Life Easier
--------------------------

Some users may have a wall-mounted or otherwise rotatable monitor, and may wish
to actually play vertical games with the rotated display.  The easiest way to
accomplish this is to put your rotation modifiers into ``vertical.ini``, where
they will only affect vertical games.

[todo: more practical examples]
