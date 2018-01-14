.. _debugger-annotation-list:

Code Annotation Debugger Commands
=================================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-comadd` -- adds a comment to the disassembled code at given address
| :ref:`debugger-command-comdelete` -- removes a comment from the given address
| :ref:`debugger-command-comsave` -- save the current comments to a file
| :ref:`debugger-command-comlist` -- print currently available comments from file
| :ref:`debugger-command-commit` -- gives a bulk comadd then comsave command


 .. _debugger-command-comadd:

comadd
------

|  **comadd[//] <address>,<comment>**
|
| Adds a string <comment> to the disassembled code at <address>. The shortcut for this command is simply '//'
|
| Examples:
|
|  comadd 0, hello world.
|
| Adds the comment 'hello world.' to the code at address 0x0
|
|  // 10, undocumented opcode!
|
| Adds the comment 'undocumented opcode!' to the code at address 0x10


 .. _debugger-command-comdelete:

comdelete
---------

|  **comdelete**
|
| Deletes the comment at the specified memory offset. The comment which is deleted is in the currently active memory bank.
|
| Examples:
|
|  comdelete 10
|
| Deletes the comment at code address 0x10 (using the current memory bank settings)


 .. _debugger-command-comsave:

comsave
-------

|  **comsave**
|
| Saves the working comments to the driver's XML comment file.
|
| Examples:
|
|  comsave
|
| Saves the comments to the driver's comment file


 .. _debugger-command-comlist:

comlist
-------

|  **comlist**
|
| Prints the currently available comment file in human readable form in debugger output window.
|
| Examples:
|
|  comlist
|
| Shows currently available comments.


 .. _debugger-command-commit:

commit
------

|  **commit[/*] <address>,<comment>**
|
| Adds a string <comment> to the disassembled code at <address> then saves to file. Basically same as comadd + comsave via a single line.
| The shortcut for this command is simply \'\/\*\'
|
| Examples:
|
|  commit 0, hello world.
|
| Adds the comment 'hello world.' to the code at address 0x0
|
|  /* 10, undocumented opcode!
|
| Adds the comment 'undocumented opcode!' to the code at address 0x10

