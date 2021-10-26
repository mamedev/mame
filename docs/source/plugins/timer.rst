Timer Plugin
============

The timer plugin records the total time spent emulating each combination of a
system and a software list item, as well as the number of times each combination
has been launched.  To see the statistics, bring up the main menu (press **Tab**
during emulation by default), choose **Plugin Options**, and then choose
**Timer**.

Note that this plugin records wall clock time, not emulated time.  That is, it
records the real time duration elapsed while the emulation is running, according
to the host OS.  This may be shorter than the elapsed emulated time if you turn
off throttling or use MAME’s “fast forward” feature, or it may be longer than
the elapsed emulated time if you pause the emulation of if the emulation is too
demanding to run at full speed.

The statistics are stored in the file **timer.db** in the **timer** folder
inside your plugin data folder (see the
:ref:`homepath option <mame-commandline-homepath>`).  The file is a SQLite3
database.
