.. _plugins-gdbstub:

GDB Stub Plugin
===============

The GDB stub plugin acts as a remote debugging server for the GNU debugger
(GDB).  This allows you to connect to MAME and debug supported systems using
GDB.  The plugin listens for connections on port 2159 on the IPv4 loopback
address (127.0.0.1).  Only Intel 80386 (i386) family processors are supported.

See the :ref:`debugger option <mame-commandline-debugger>` for another GDB
remote debugging implementation with support for more CPUs and configurable
listening ports.
