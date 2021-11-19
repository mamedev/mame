.. _plugins:

Plugins
=======

.. contents:: :local:


.. _plugins-intro:

Introduction
------------

MAME supports plugins that can provide additional functionality.  Plugins
have been written to communicate with external programs, play games
automatically, display internal game structures like hitboxes, provide alternate
user interfaces, and automatically test emulation.  See :ref:`luaengine` for
more information about MAME’s Lua API.


.. _plugins-using:

Using plugins
-------------

To enable plugins, you need to turn on the
:ref:`plugins option <mame-commandline-plugins>`, and make sure the
:ref:`pluginspath option <mame-commandline-pluginspath>` includes the folder
where your plugins are stored.  You can set the plugins option in an INI file
or on the command line.  You can set the pluginspath option by selecting
**Configure Options** from the system selection menu, then selecting
**Configure Directories**, and then selecting **Plugins** (you can also set it
in an INI file or on the command line).

Many plugins need to store settings and/or data.  The
:ref:`homepath option <mame-commandline-homepath>` sets the folder where plugins
should save data (defaults to the working directory).  You can change this by
selecting **Configure Options** from the system selection menu, thens selecting
**Configure Directories**, and then selecting **Plugin Data**.

To turn individual plugins on or off, first make sure plugins are enabled, then
select **Configure Options** from the system selection menu, and then select
**Plugins**.  You will need to completely exit MAME and start it again for
changes to the enabled plugins to take effect.  You can also use the
:ref:`plugin option <mame-commandline-plugin>` on the command line, or change
the settings in the **plugin.ini** file.

If an enabled plugin needs additional configuration, or if it needs to show
information, a **Plugin Options** item will appear in the main menu (accessed by
pressing **Tab** during emulation by default).


.. _plugins-included:

Included plugins
----------------

MAME includes several plugins providing useful functionality, and serving as
sample code that you can use as a starting point when writing your own plugins.

.. toctree::
    :titlesonly:

    autofire
    console
    data
    discord
    dummy
    gdbstub
    hiscore
    inputmacro
    layout
    timecode
    timer
