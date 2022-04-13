.. _debugger-annotation-list:

Code Annotation Debugger Commands
=================================

:ref:`debugger-command-comadd`
    adds a comment to the disassembled code at given address
:ref:`debugger-command-comdelete`
    removes a comment from the given address
:ref:`debugger-command-comsave`
    save the current comments to file
:ref:`debugger-command-comlist`
    list comments stored in the comment file for the system
:ref:`debugger-command-commit`
    combined ``comadd`` and ``comsave`` command


.. _debugger-command-comadd:

comadd
------

**comadd <address>,<comment>**

Sets the specified comment for the specified address in the disassembled
code for the visible CPU.  This command may be abbreviated to ``//``.

Examples:

``comadd 0,hello world.``
    Adds the comment “hello world.” to the code at address 0.
``// 10,undocumented opcode!``
    Adds the comment “undocumented opcode!” to the code at address 10.


.. _debugger-command-comdelete:

comdelete
---------

**comdelete**

Deletes the comment at the specified address for the visible CPU.

Examples:

``comdelete 10``
    Deletes the comment at code address 10 for the visible CPU.


.. _debugger-command-comsave:

comsave
-------

**comsave**

Saves the current comments to the XML comment file for the emulated
system.  This file will be loaded by the debugger the next time the
system is run with debugging enabled.  The directory for saving these
files is set using the
:ref:`comment_directory <mame-commandline-commentdirectory>` option.

Examples:

``comsave``
    Saves the current comments to the comment file for the system.


.. _debugger-command-comlist:

comlist
-------

**comlist**

Reads the comments stored in the XML comment file for the emulated
system and prints them to the debugger console.  This command does not
affect the comments for the current session, it reads the file directly.
The directory for these files is set using the
:ref:`comment_directory <mame-commandline-commentdirectory>` option.

Examples:

comlist
    Shows comments stored in the comment file for the system.


.. _debugger-command-commit:

commit
------

**commit <address>,<comment>**

Sets the specified comment for the specified address in the disassembled
code for the visible CPU, and saves comments to the file for the current
emulated system (equivalent to :ref:`debugger-command-comadd` followed
by :ref:`debugger-command-comsave`).  This command may be abbreviated to
``/*``.

Examples:

``commit 0,hello world.``
    Adds the comment “hello world.” to the code at address 0 for the
    visible CPU and saves comments.
``/* 10,undocumented opcode!``
    Adds the comment “undocumented opcode!” to the code at address 10
    for the visible CPU and saves comments.
