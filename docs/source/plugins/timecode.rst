.. _plugins-timecode:

Timecode Recorder Plugin
========================

The timecode recorder plugin logs time codes to a text file in conjunction with
creating an input recording file to assist people creating gameplay videos.  The
time code log file is *only* created when making an input recording.  The time
code log file has the same name as the input recording file with the extension
**.timecode** appended.  Use the :ref:`record <mame-commandline-record>` and
:ref:`input_directory <mame-commandline-inputdirectory>` options to create an
input recording and specify the location for the output files.

By default, the plugin records a time code when you press the **F12** key on the
keyboard while not pressing either **Shift** or **Alt** key.  You can change
this setting in the options menu for the plugin (choose **Plugin Options** from
the main menu during emulation, and then choose **Timecode Recorder**).

Settings for the plugin are stored in JSON format in the file **plugin.cfg** in
the **timecode** folder inside your plugin data folder (see the
:ref:`homepath option <mame-commandline-homepath>`).
