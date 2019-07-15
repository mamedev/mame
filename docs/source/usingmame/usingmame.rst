Using MAME
----------

If you want to dive right in and skip the command line, there's a nice graphical way to use MAME without the need to download and set up a front end. Simply start MAME with no parameters, by doubleclicking the **mame.exe** file or running it directly from the command line. If you're looking to harness the full power of MAME, keep reading further.

On Macintosh OS X and \*nix-based platforms, please be sure to set your font up to match your locale before starting, otherwise you may not be able to read the text due to missing glyphs.

If you are a new MAME user, you could find this emulator a bit complex at first. Let's take a moment to talk about softlists, as they can simplify matters quite a bit. If the content you are trying to play is a documented entry on one of the MAME softlists, starting the content is as easy as

    **mame.exe** *<system>* *<software>*

For instance:

    **mame.exe nes metroidu**

will load the USA version of Metroid for the Nintendo Entertainment System.


Alternatively, you could start MAME with

	**mame.exe nes**
	
and choose the *software list* from the cartridge slot. From there, you could pick any softlist-compatible software you have in your roms folders. Please note that many older dumps of cartridges and discs may either be bad or require renaming to match up to the softlist in order to work in this way.


If you are loading an arcade board or other non-softlist content, things are only a little more complicated:


The basic usage, from command line, is

	**mame.exe** *<system>* *<media>* *<software>* *<options>*

where

* *<system>* is the shortname of the system you want to emulate (e.g. nes, c64, etc.)
* *<media>* is the switch for the media you want to load (if it's a cartridge, try **-cart** or **-cart1**; if it's a floppy disk, try **-flop** or **-flop1**; if it's a CD-ROM, try **-cdrom**)
* *<software>* is the program / game you want to load (and it can be given either as the fullpath to the file to load, or as the shortname of the file in our software lists)
* *<options>* is any additional command line option for controllers, video, sound, etc.

Remember that if you type a <system> name which does not correspond to any emulated system, MAME will suggest you some possible choices which are close to what you typed; and if you don't know which <media> switch are available, you can always launch

	**mame.exe** *<system>* **-listmedia**

If you don't know what *<options>* are available, there are a few things you can do. First of all, you can check the command line options section of this manual. You can also try one of the many :ref:`frontends` available for MAME.


Alternatively, you should keep in mind the following command line options, which might be very useful on occasion:


	**mame.exe -help**

tells what MAME is the basic structure of MAME launching options, i.e. as explained above.


	**mame.exe -showusage**

gives you the (quite long) list of available command line options for MAME. The main options are described, in the :ref:`universal-command-line` section of this manual.


	**mame.exe -showconfig**

gives you a (quite long) list of available configuration options for MAME. These configuration can always be modified at command line, or by editing them in mame.ini which is the main configuration file for MAME. You can find a description of some configuration options in the :ref:`universal-command-line` section of the manual (in most cases, each configuration option has a corresponding command line option to configure and modify it).


	**mame.exe -createconfig**

creates a brand new **mame.ini** file, with default configuration settings. Notice that mame.ini is basically a plain text file, hence you can open it with any text editor (e.g. Notepad, Emacs or TextEdit) and configure every option you need. However, no particular tweaks are needed to start, so you can basically leave most of the options unaltered.

If you execute **mame64 -createconfig** when you already have an existing mame.ini from a previous MAME version, MAME automatically updates the pre-existing mame.ini by copying changed options into it.


Once you are more confident with MAME options, you may want to configure a bit more your setup. In this case, keep in mind the order in which options are read; see :ref:`advanced-multi-CFG` for details.
