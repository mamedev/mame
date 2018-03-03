.. _debugger-cheats-list:

Cheat Debugger Commands
=======================


You can also type **help <command>** for further details on each command in the MAME Debugger interface.

| :ref:`debugger-command-cheatinit` -- initialize the cheat search to the selected memory area
| :ref:`debugger-command-cheatrange` -- add to the cheat search the selected memory area
| :ref:`debugger-command-cheatnext` -- continue cheat search comparing with the last value
| :ref:`debugger-command-cheatnextf` -- continue cheat search comparing with the first value
| :ref:`debugger-command-cheatlist` -- show the list of cheat search matches or save them to <filename>
| :ref:`debugger-command-cheatundo` -- undo the last cheat search (state only)

 .. _debugger-command-cheatinit:

cheatinit
---------

|  **cheatinit [<sign><width><swap>,[<address>,<length>[,<cpu>]]]**
|
| The cheatinit command initializes the cheat search to the selected memory area.
|
| If no parameter is specified the cheat search is initialized to all changeable memory of the main CPU.
|
| <sign> can be s(signed) or u(unsigned)
| <width> can be b(8 bit), w(16 bit), d(32 bit) or q(64 bit)
| <swap> append s for swapped search
|
| Examples:
|
|  cheatinit ub,0x1000,0x10
|
| Initialize the cheat search from 0x1000 to 0x1010 of the first CPU.
|
|  cheatinit sw,0x2000,0x1000,1
|
| Initialize the cheat search with width of 2 byte in signed mode from 0x2000 to 0x3000 of the second CPU.
|
|  cheatinit uds,0x0000,0x1000
|
| Initialize the cheat search with width of 4 byte swapped from 0x0000 to 0x1000.
|
| Back to :ref:`debugger-cheats-list`


 .. _debugger-command-cheatrange:

cheatrange
----------

|  **cheatrange <address>,<length>**
|
| The cheatrange command adds the selected memory area to the cheat search.
|
| Before using cheatrange it is necessary to initialize the cheat search with cheatinit.
|
| Examples:
|
|  cheatrange 0x1000,0x10
|
| Add the bytes from 0x1000 to 0x1010 to the cheat search.
|
| Back to :ref:`debugger-cheats-list`


 .. _debugger-command-cheatnext:

cheatnext
---------

|  **cheatnext <condition>[,<comparisonvalue>]**
|
| The cheatnext command will make comparisons with the last search matches.
|
| Possible <condition>:
|
|  **all**
|
| No <comparisonvalue> needed.
|
| Use to update the last value without changing the current matches.
|
|  **equal [eq]**
|
| Without <comparisonvalue> search for all bytes that are equal to the last search.
| With <comparisonvalue> search for all bytes that are equal to the <comparisonvalue>.
|
|  **notequal [ne]**
|
| Without <comparisonvalue> search for all bytes that are not equal to the last search.
| With <comparisonvalue> search for all bytes that are not equal to the <comparisonvalue>.
|
|  **decrease [de, +]**
|
| Without <comparisonvalue> search for all bytes that have decreased since the last search.
| With <comparisonvalue> search for all bytes that have decreased by the <comparisonvalue> since the last search.
|
|  **increase [in, -]**
|
| Without <comparisonvalue> search for all bytes that have increased since the last search.
| With <comparisonvalue> search for all bytes that have increased by the <comparisonvalue> since the last search.
|
|  **decreaseorequal [deeq]**
|
| No <comparisonvalue> needed.
|
| Search for all bytes that have decreased or have same value since the last search.
|
|  **increaseorequal [ineq]**
|
| No <comparisonvalue> needed.
|
| Search for all bytes that have decreased or have same value since the last search.
|
|  **smallerof [lt]**
|
| Without <comparisonvalue> this condition is invalid
| With <comparisonvalue> search for all bytes that are smaller than the <comparisonvalue>.
|
|  **greaterof [gt]**
|
| Without <comparisonvalue> this condition is invalid
| With <comparisonvalue> search for all bytes that are larger than the <comparisonvalue>.
|
|  **changedby [ch, ~]**
|
| Without <comparisonvalue> this condition is invalid
| With <comparisonvalue> search for all bytes that have changed by the <comparisonvalue> since the last search.
|
|
| Examples:
|
|  cheatnext increase
|
| Search for all bytes that have increased since the last search.
|
|  cheatnext decrease, 1
|
| Search for all bytes that have decreased by 1 since the last search.
|
| Back to :ref:`debugger-cheats-list`


 .. _debugger-command-cheatnextf:

cheatnextf
----------

|  **cheatnextf <condition>[,<comparisonvalue>]**
|
| The cheatnextf command will make comparisons with the initial search.
|
| Possible <condition>:
|
|  **all**
|
| No <comparisonvalue> needed.
|
| Use to update the last value without changing the current matches.
|
|  **equal [eq]**
|
| Without <comparisonvalue> search for all bytes that are equal to the initial search.
| With <comparisonvalue> search for all bytes that are equal to the <comparisonvalue>.
|
|  **notequal [ne]**
|
| Without <comparisonvalue> search for all bytes that are not equal to the initial search.
| With <comparisonvalue> search for all bytes that are not equal to the <comparisonvalue>.
|
|  **decrease [de, +]**
|
| Without <comparisonvalue> search for all bytes that have decreased since the initial search.
| With <comparisonvalue> search for all bytes that have decreased by the <comparisonvalue> since the initial search.
|
|  **increase [in, -]**
|
| Without <comparisonvalue> search for all bytes that have increased since the initial search.
|
| With <comparisonvalue> search for all bytes that have increased by the <comparisonvalue> since the initial search.
|
|  **decreaseorequal [deeq]**
|
| No <comparisonvalue> needed.
|
| Search for all bytes that have decreased or have same value since the initial search.
|
|  **increaseorequal [ineq]**
|
| No <comparisonvalue> needed.
|
| Search for all bytes that have decreased or have same value since the initial search.
|
|  **smallerof [lt]**
|
| Without <comparisonvalue> this condition is invalid.
| With <comparisonvalue> search for all bytes that are smaller than the <comparisonvalue>.
|
|  **greaterof [gt]**
|
| Without <comparisonvalue> this condition is invalid.
| With <comparisonvalue> search for all bytes that are larger than the <comparisonvalue>.
|
|  **changedby [ch, ~]**
|
| Without <comparisonvalue> this condition is invalid
| With <comparisonvalue> search for all bytes that have changed by the <comparisonvalue> since the initial search.
|
|
| Examples:
|
|  cheatnextf increase
|
| Search for all bytes that have increased since the initial search.
|
|  cheatnextf decrease, 1
|
| Search for all bytes that have decreased by 1 since the initial search.
|
| Back to :ref:`debugger-cheats-list`


 .. _debugger-command-cheatlist:

cheatlist
---------

|  **cheatlist [<filename>]**
|
| Without <filename> show the list of matches in the debug console.
| With <filename> save the list of matches in basic XML format to <filename>.
|
| Examples:
|
|  cheatlist
|
| Show the current matches in the debug console.
|
|  cheatlist cheat.txt
|
| Save the current matches in XML format to cheat.txt.
|
| Back to :ref:`debugger-cheats-list`


 .. _debugger-command-cheatundo:

cheatundo
---------

|  **cheatundo**
|
| Undo the results of the last search.
|
| The undo command has no effect on the last value.
|
|
| Examples:
|
|  cheatundo
|
| Undo the last search (state only).
|
| Back to :ref:`debugger-cheats-list`

