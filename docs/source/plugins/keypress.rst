.. _plugins-keypress:

Keypress Display Plugin
========================

The keypress plugin shows the names of all currently pressed input items on
screen using MAME's pop-up message overlay.  It is useful for gameplay
recording and tracing player actions, as well as diagnosing input mapping
issues.

Enable the plugin using the :ref:`plugin option <mame-commandline-plugin>` on
the command line:

.. code-block:: bash

    mame <system> -plugin keypress

While the plugin is active, any pressed keys, buttons, or other input items
will be displayed as a space-separated list at the bottom of the screen.  The
message clears automatically when all inputs are released.
