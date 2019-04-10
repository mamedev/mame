MAME Path Handling
==================

MAME has a specific order it uses when checking for user files such as ROM sets and cheat files.


Order of Path Loading
---------------------

Let's use an example of the cheat file for AfterBurner 2 for Sega Genesis/MegaDrive (aburner2 in the megadrive softlist), and your cheatpath is set to "cheat" (as per the default) -- this is how MAME will search for that cheat file:

1. ``cheat/megadriv/aburner2.xml``
2. ``cheat/megadriv.zip`` -> ``aburner2.xml``
   Notice that it checks for a .ZIP file first before a .7Z file.
3. ``cheat/megadriv.zip`` -> ``<arbitrary path>/aburner2.xml``
   It will look for the first (if any) ``aburner2.xml`` file it can find inside that zip, no matter what the path is.
4. ``cheat.zip`` -> ``megadriv/aburner2.xml``
   Now it is specifically looking for the file and folder combination, but inside the cheat.zip file.
5. ``cheat.zip`` -> ``<arbitrary path>/megadriv/aburner2.xml``
   Like before, except looking for the first (if any) ``aburner2.xml`` inside a ``megadriv`` folder inside the zip.
6. ``cheat/megadriv.7z`` -> ``aburner2.xml``
   Now we start checking 7ZIP files.
7. ``cheat/megadriv.7z`` -> ``<arbitrary path>/aburner2.xml``
8. ``cheat.7z`` -> ``megadriv/aburner2.xml``
9. ``cheat.7z`` -> ``<arbitrary path>/megadriv/aburner2.xml``
   Similar to zip, except now 7ZIP files.


[todo: ROM set loading is slightly more complicated, adding CRC. Get that documented in the next day or two.]
