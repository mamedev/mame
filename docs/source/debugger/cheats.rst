.. _debugger-cheats-list:

Cheat Debugger Commands
=======================

:ref:`debugger-command-cheatinit`
    initialize the cheat search to the selected memory area
:ref:`debugger-command-cheatrange`
    add selected memory area to cheat search
:ref:`debugger-command-cheatnext`
    filter cheat candidates by comparing to previous values
:ref:`debugger-command-cheatnextf`
    filter cheat candidates by comparing to initial value
:ref:`debugger-command-cheatlist`
    show the list of cheat search matches or save them to a file
:ref:`debugger-command-cheatundo`
    undo the last cheat search (state only)

The debugger includes basic cheat search functionality, which works
by saving the contents of memory, and then filtering locations according
to how the values change.

We’ll demonstrate use of the cheat search functionality to make an
infinite lives cheat for Raiden (``raiden``):

* Start the game with the debugger active.  Allow the game to run,
  insert a coin, and start a game, then break into the debugger.
* Ensure the main CPU is visible, and start a search for 8-bit unsigned
  values using the
  :ref:`cheatinit command <debugger-command-cheatinit>`::

      >cheatinit ub
      36928 cheat locations initialized for NEC V30 ':maincpu' program space
* Allow the game to run, lose a life and break into the debugger.
* Use the :ref:`cheatnext command <debugger-command-cheatnext>` to
  filter locations that have decreased by 1::

      >cheatnext -,1
      12 cheats found
* Allow the game to run, lose another life, break into the
  debugger, and filter locations that have decreased by 1 again::

      >cheatnext -,1
      Address=0B85 Start=03 Current=01
      1 cheats found
* Use the :ref:`cheatlist command <debugger-command-cheatlist>` to save
  the cheat candidate to a file::

      >cheatlist raiden-p1-lives.xml
* The file now contains an XML fragment with cheat to set the candidate
  location to the initial value:

  .. code-block:: XML

      <cheat desc="Possibility 1: 00B85 (01)">
        <script state="run">
          <action>:maincpu.pb@0x00B85=0x03</action>
        </script>
      </cheat>


.. _debugger-command-cheatinit:

cheatinit
---------

**cheatinit [[<sign>[<width>[<swap>]]],[<address>,<length>[,<space>]]]**

Initialize the cheat search to writable RAM areas in the specified
address space.  May be abbreviated to ``ci``.

The first argument specifies the data format to search for.  The
**<sign>** may be **u** for unsigned or **s** for signed, the
**<width>** may be **b** for 8-bit (byte), **w** for 16-bit (word),
**d** for 32-bit (double word), or **q** for 64-bit (quadruple word);
**<swap>** may be **s** for reversed byte order.  If the first argument
is omitted or empty, the data format from the previous cheat search is
used, or unsigned 8-bit format if this is the first cheat search.

The **<address>** specifies the address to start searching from, and the
**<length>** specifies how much memory to search.  If specified,
writable RAM in the range **<address>** through
**<address>+<length>-1**, inclusive, will be searched; otherwise, all
writable RAM in the address space will be searched.

See :ref:`debugger-devicespec` for details on specifying address spaces.
If the address space is not specified, it defaults to the first address
space exposed by the visible CPU.

Examples:

``cheatinit ub,0x1000,0x10``
    Initialize the cheat search for unsigned 8-bit values in addresses
    0x1000-0x100f in the program space of the visible CPU.
``cheatinit sw,0x2000,0x1000,1``
    Initialize the cheat search for signed 16-bit values in addresses
    0x2000-0x2fff in the program space of the second CPU in the system
    (zero-based index).
``cheatinit uds,0x0000,0x1000``
    Initialize the cheat search for unsigned 64-bit values with reverse
    byte order in addresses 0x0000-0x0fff in the program space of the
    visible CPU.

Back to :ref:`debugger-cheats-list`


.. _debugger-command-cheatrange:

cheatrange
----------

**cheatrange <address>,<length>**

Add writable RAM areas to the cheat search.  May be abbreviated to
``cr``.  Before using this command, the
:ref:`cheatinit command <debugger-command-cheatinit>` must be used to
initialize the cheat search and set the address space and data format.

The **<address>** specifies the address to start searching from, and the
**<length>** specifies how much memory to search.  Writable RAM in the
range **<address>** through **<address>+<length>-1**, inclusive, will be
added to the areas to search.

Examples:

``cheatrange 0x1000,0x10``
    Add addresses 0x1000-0x100f to the areas to search for cheats.

Back to :ref:`debugger-cheats-list`


.. _debugger-command-cheatnext:

cheatnext
---------

**cheatnext <condition>[,<comparisonvalue>]**

Filter candidates by comparing to the previous search values.  If five
or fewer candidates remain, they will be shown in the debugger console.
May be abbreviated to ``cn``.

Possible **<condition>** arguments:

``all``
    Use to update the last value without changing the current matches
    (the **<comparisonvalue>** is not used).
``equal`` (``eq``)
    Without **<comparisonvalue>**, search for values that are equal to
    the previous search; with **<comparisonvalue>**, search for values
    that are equal to the **<comparisonvalue>**.
``notequal`` (``ne``)
    Without **<comparisonvalue>**, search for values that are not equal
    to the previous search; with **<comparisonvalue>**, search for
    values that are not equal to the **<comparisonvalue>**.
``decrease`` (``de``, ``-``)
    Without **<comparisonvalue>**, search for values that have decreased
    since the previous search; with **<comparisonvalue>**, search for
    values that have decreased by the **<comparisonvalue>** since the
    previous search.
``increase`` (``in``, ``+``)
    Without **<comparisonvalue>**, search for values that have increased
    since the previous search; with **<comparisonvalue>**, search for
    values that have increased by the **<comparisonvalue>** since the
    previous search.
``decreaseorequal`` (``deeq``)
    Search for values that have decreased or are unchanged since the
    previous search (the **<comparisonvalue>** is not used).
``increaseorequal`` (``ineq``)
    Search for values that have increased or are unchanged since the
    previous search (the **<comparisonvalue>** is not used).
``smallerof`` (``lt``, ``<``)
    Search for values that are less than the **<comparisonvalue>** (the
    **<comparisonvalue>** is required).
``greaterof`` (``gt``, ``>``)
    Search for values that are greater than the **<comparisonvalue>**
    (the **<comparisonvalue>** is required).
``changedby`` (``ch``, ``~``)
    Search for values that have changed by the **<comparisonvalue>**
    since the previous search (the **<comparisonvalue>** is required).

Examples:

``cheatnext increase``
    Search for all values that have increased since the previous search.
``cheatnext decrease,1``
    Search for all values that have decreased by 1 since the previous
    search.

Back to :ref:`debugger-cheats-list`


 .. _debugger-command-cheatnextf:

cheatnextf
----------

**cheatnextf <condition>[,<comparisonvalue>]**

Filter candidates by comparing to the initial search values.  If five or
fewer candidates remain, they will be shown in the debugger console.
May be abbreviated to ``cn``.

Possible **<condition>** arguments:

``all``
    Use to update the last value without changing the current matches
    (the **<comparisonvalue>** is not used).
``equal`` (``eq``)
    Without **<comparisonvalue>**, search for values that are equal to
    the initial search; with **<comparisonvalue>**, search for values
    that are equal to the **<comparisonvalue>**.
``notequal`` (``ne``)
    Without **<comparisonvalue>**, search for values that are not equal
    to the initial search; with **<comparisonvalue>**, search for values
    that are not equal to the **<comparisonvalue>**.
``decrease`` (``de``, ``-``)
    Without **<comparisonvalue>**, search for values that have decreased
    since the initial search; with **<comparisonvalue>**, search for
    values that have decreased by the **<comparisonvalue>** since the
    initial search.
``increase`` (``in``, ``+``)
    Without **<comparisonvalue>**, search for values that have increased
    since the initial search; with **<comparisonvalue>**, search for
    values that have increased by the **<comparisonvalue>** since the
    initial search.
``decreaseorequal`` (``deeq``)
    Search for values that have decreased or are unchanged since the
    initial search (the **<comparisonvalue>** is not used).
``increaseorequal`` (``ineq``)
    Search for values that have increased or are unchanged since the
    initial search (the **<comparisonvalue>** is not used).
``smallerof`` (``lt``, ``<``)
    Search for values that are less than the **<comparisonvalue>** (the
    **<comparisonvalue>** is required).
``greaterof`` (``gt``, ``>``)
    Search for values that are greater than the **<comparisonvalue>**
    (the **<comparisonvalue>** is required).
``changedby`` (``ch``, ``~``)
    Search for values that have changed by the **<comparisonvalue>**
    since the initial search (the **<comparisonvalue>** is required).

Examples:

``cheatnextf increase``
    Search for all values that have increased since the initial search.
``cheatnextf decrease,1``
    Search for all values that have decreased by 1 since the initial
    search.

Back to :ref:`debugger-cheats-list`


.. _debugger-command-cheatlist:

cheatlist
---------

**cheatlist [<filename>]**

Without **<filename>**, show the current cheat matches in the debugger
console; with **<filename>**, save the current cheat matches in basic
XML format to the specified file.  May be abbreviated to ``cl``.

Examples:

``cheatlist``
    Show the current matches in the console.
``cheatlist cheat.xml``
    Save the current matches to the file **cheat.xml** in XML format.

Back to :ref:`debugger-cheats-list`


.. _debugger-command-cheatundo:

cheatundo
---------

**cheatundo**

Undo filtering of cheat candidates by the most recent
:ref:`cheatnext <debugger-command-cheatnext>` or
:ref:`cheatnextf <debugger-command-cheatnextf>` command.  Note that the
previous values *are not* rolled back.  May be abbreviated to ``cu``.

Examples:

``cheatundo``
    Restore cheat candidates filtered out by the most recent
    :ref:`cheatnext <debugger-command-cheatnext>` or
    :ref:`cheatnextf <debugger-command-cheatnextf>` command.

Back to :ref:`debugger-cheats-list`
