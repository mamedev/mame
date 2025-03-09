Other tools included with MAME
==============================


ledutil.exe/ledutil.sh
----------------------

On Microsoft Windows, ledutil.exe can take control of your keyboard LEDs to mirror those that were present on some early arcade games (e.g. Asteroids)

Start **ledutil.exe** from the command line to enable LED handling. Run **ledutil.exe -kill** to stop the handler.

On SDLMAME platforms such as Mac OS X and Linux, **ledutil.sh** can be used. Use **ledutil.sh -a** to have it automatically close when you exit SDLMAME.


Developer-focused tools included with MAME
==========================================


pngcmp
------

This tool is used in regression testing to compare PNG screenshot results with the runtest.cmd script found in the source archive. This script works only on Microsoft Windows.


nltool
------

Discrete component conversion tool.


nlwav
-----

Discrete component conversion and testing tool.


jedutil
-------

PAL/PLA/PLD/GAL dump handling tool. It can convert between the industry-standard JED format and MAME's proprietary packed binary format and it can show logic equations for the types of devices it knows the internal logic of.


ldresample
----------

This tool recompresses video data for laserdisc and VHS dumps.


ldverify
--------

This tool is used for comparing laserdisc or VHS CHD images with the source AVI.


romcmp
------

This tool is used to perform basic data comparisons and integrity checks on binary dumps. With the -h switch, it can also be used to calculate hash functions.


.. _othertools_srcdbgdump:

srcdbgdump
----------

This tool may be used to view the contents of
:ref:`MAME Debugging Information Files <srcdbg_mdi>`.  The only command-line
parameter it accepts is the name of the MAME Debugging Information file to
dump.  For example:

``srcdbgdump c:\path\to\mysymbols.mdi``
    Prints the contents of the specified MDI file as ASCII text to the console.
``srcdbgdump c:\path\to\mysymbols.mdi > c:\path\to\mysymbols.txt``
	Dump the contents into a text file.


unidasm
-------

Universal disassembler for many of the architectures supported in MAME.
