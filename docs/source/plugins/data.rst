.. _plugins-data:

Data Plugin
===========

The data plugin loads information from various external support files so it can
be displayed in MAME.  If the plugin is enabled, info is show in the **Infos**
tab of the right-hand pane on the system and software selection menus.  The info
viewer can be shown by clicking the toolbar button on the system and software
selection menus, or by choosing **External DAT View** from the main menu during
emulation (this menu item will not appear if the data plugin is not enabled, or
if no information is available for the emulated system).

To set the folders where the data plugin looks for supported files, choose
**Configure Options** on the system selection menu, then choose
**Configure Directories**, and then choose **DATs**.  You can also set the
``historypath`` option in your **ui.ini** file.

Loading large data files like **history.xml** can take quite a while, so please
be patient the first time you start MAME after updating or adding new data
files.

The following files are supported:

history.xml
    From Gaming-History (formerly Arcade-History)
mameinfo.dat
    From `MASH’s MAMEINFO <https://mashinfo.github.io/mameinfo/>`_
messinfo.dat
    From `progetto-SNAPS MESSINFO.dat
    <https://www.progettosnaps.net/messinfo/>`_
gameinit.dat
    From `progetto-SNAPS GameInit.dat
    <https://www.progettosnaps.net/gameinit/>`_
command.dat
    from `progetto-SNAPS Command.dat
    <https://www.progettosnaps.net/command/>`_
score3.htm
    `Top Scores <http://replay.marpirc.net/txt/scores3.htm>`_ from
    the `MAME Action Replay Page <http://replay.marpirc.net/>`_
Japanese mameinfo.dat and command.dat
    From `MAME E2J <https://e2j.net/downloads/>`_
sysinfo.dat
    From the defunct Progetto EMMA site
story.dat
    From the defunct MAMESCORE site

If you install `hi2txt <https://greatstoneex.github.io/hi2txt-doc/>`_, the data
plugin can also show high scores from non-volatile memory or saved by the
:ref:`hiscore support plugin <plugins-hiscore>` for supported games.

Note that you can only use a single file of each type at a time.  You cannot,
for example, use the English and Japanese **mameinfo.dat** files simultaneously.

The data plugin creates a **history.db** file in the **data** folder in the
plugin data folder (see the :ref:`homepath option <mame-commandline-homepath>`).
This file stores the information from the support files in a format suitable for
rapid loading.  It uses the SQLite3 database format.
