Configuring MAME
================

.. contents:: :local:

Getting Started: A Quick Preface
--------------------------------

Once you have MAME installed, the next step is to configure it. There are
several ways to do this, and each will be covered in turn.

If you are on Windows, the MAME executable will be called ``mame.exe``.

If you are on Linux or MacOS, the MAME executable will be called ``mame``.


Initial Setup: Creating mame.ini From Command Line on Windows
-------------------------------------------------------------

First, you will need to *cd* to the directory where you installed MAME into.
If, for instance, you have MAME installed in ``C:\Users\Public\MAME`` you will
need to type ``cd C:\Users\Public\MAME`` into the command prompt.

Then you have MAME create the config file by typing ``mame
-createconfig``.  MAME will then create the ``mame.ini`` file in the
MAME installation folder.  This file contains the default
configuration settings for a new MAME installation.


Initial Setup: Creating mame.ini From Command Line on Linux or MacOS
--------------------------------------------------------------------

The steps for Linux and MacOS are similar to those of Windows. If you
installed MAME using the package manager that came from a Linux distro, you will
type ``mame -createconfig`` into your terminal of choice.

If you have compiled from source or downloaded a binary package of MAME,
you will ``cd`` into the directory you put the MAME files into.

For instance, ``cd /home/myusername/mame``

Then you will type ``./mame -createconfig`` into
your terminal of choice.

You can then need to edit the ``mame.ini`` file in your favorite text editor,
e.g. *Notepad* on Windows or *vi* on Linux/MacOS, or you can change settings
from inside of MAME.


Initial Setup: Graphical Setup
------------------------------

This is the easiest way to get started. Start MAME by opening the MAME
icon in the location where you installed it. This will be ``mame.exe``
on Windows, ``mame`` on Linux and macOS.

Once MAME has started, you can either use your mouse to click on the
**Configure Options** menu selection at the bottom center of your screen,
or you can switch panes to the bottom one (default key is Tab), then press
the menu accept button (default key is Return/Enter) to go into the
Configuration menu.

Choose **Save Configuration** to create the ``mame.ini`` file with default
settings. From here, you can either continue to configure things from the
graphical user interface or edit the ``mame.ini`` file in your favorite
text editor.


Configuring MAME
----------------

