// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debughlp.c

    Debugger help engine.

*********************************************************************/

#include "emu.h"
#include "debughlp.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CONSOLE_HISTORY     (10000)
#define CONSOLE_LINE_CHARS  (100)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct help_item
{
	const char *        tag;
	const char *        help;
};



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/



/***************************************************************************
    TABLE OF HELP
***************************************************************************/

static const help_item static_help_list[] =
{
	{
		"",
		"\n"
		"MAME Debugger Help\n"
		"  help [<topic>] -- get help on a particular topic\n"
		"\n"
		"Topics:\n"
		"  General\n"
		"  Memory\n"
		"  Execution\n"
		"  Breakpoints\n"
		"  Watchpoints\n"
		"  Registerpoints\n"
		"  Expressions\n"
		"  Comments\n"
		"  Cheats\n"
		"  Image\n"
	},
	{
		"general",
		"\n"
		"General Debugger Help\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  help [<topic>] -- get help on a particular topic\n"
		"  do <expression> -- evaluates the given expression\n"
		"  symlist [<cpu>] -- lists registered symbols\n"
		"  softreset -- executes a soft reset\n"
		"  hardreset -- executes a hard reset\n"
		"  print <item>[,...] -- prints one or more <item>s to the console\n"
		"  printf <format>[,<item>[,...]] -- prints one or more <item>s to the console using <format>\n"
		"  logerror <format>[,<item>[,...]] -- outputs one or more <item>s to the error.log\n"
		"  tracelog <format>[,<item>[,...]] -- outputs one or more <item>s to the trace file using <format>\n"
		"  history [<cpu>,<length>] -- outputs a brief history of visited opcodes\n"
		"  trackpc [<bool>,<cpu>,<bool>] -- visually track visited opcodes [boolean to turn on and off, for the given cpu, clear]\n"
		"  trackmem [<bool>,<bool>] -- record which PC writes to each memory address [boolean to turn on and off, clear]\n"
		"  pcatmemp <address>[,<cpu>] -- query which PC wrote to a given program memory address for the current CPU\n"
		"  pcatmemd <address>[,<cpu>] -- query which PC wrote to a given data memory address for the current CPU\n"
		"  pcatmemi <address>[,<cpu>] -- query which PC wrote to a given I/O memory address for the current CPU\n"
		"                                (Note: you can also query this info by right clicking in a memory window\n"
		"  statesave[ss] <filename> -- save a state file for the current driver\n"
		"  stateload[sl] <filename> -- load a state file for the current driver\n"
		"  snap [<filename>] -- save a screen snapshot.\n"
		"  source <filename> -- reads commands from <filename> and executes them one by one\n"
		"  quit -- exits MAME and the debugger\n"
	},
	{
		"memory",
		"\n"
		"Memory Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  dasm <filename>,<address>,<length>[,<opcodes>[,<cpu>]] -- disassemble to the given file\n"
		"  f[ind] <address>,<length>[,<data>[,...]] -- search program memory for data\n"
		"  f[ind]d <address>,<length>[,<data>[,...]] -- search data memory for data\n"
		"  f[ind]i <address>,<length>[,<data>[,...]] -- search I/O memory for data\n"
		"  dump <filename>,<address>,<length>[,<size>[,<ascii>[,<cpu>]]] -- dump program memory as text\n"
		"  dumpd <filename>,<address>,<length>[,<size>[,<ascii>[,<cpu>]]] -- dump data memory as text\n"
		"  dumpi <filename>,<address>,<length>[,<size>[,<ascii>[,<cpu>]]] -- dump I/O memory as text\n"
		"  save <filename>,<address>,<length>[,<cpu>] -- save binary program memory to the given file\n"
		"  saved <filename>,<address>,<length>[,<cpu>] -- save binary data memory to the given file\n"
		"  savei <filename>,<address>,<length>[,<cpu>] -- save binary I/O memory to the given file\n"
		"  load <filename>,<address>,<length>[,<cpu>] -- load binary program memory from the given file\n"
		"  loadd <filename>,<address>,<length>[,<cpu>] -- load binary data memory from the given file\n"
		"  loadi <filename>,<address>,<length>[,<cpu>] -- load binary I/O memory from the given file\n"
		"  map <address> -- map logical program address to physical address and bank\n"
		"  mapd <address> -- map logical data address to physical address and bank\n"
		"  mapi <address> -- map logical I/O address to physical address and bank\n"
		"  memdump [<filename>] -- dump the current memory map to <filename>\n"
	},
	{
		"execution",
		"\n"
		"Execution Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  s[tep] [<count>=1] -- single steps for <count> instructions (F11)\n"
		"  o[ver] [<count>=1] -- single steps over <count> instructions (F10)\n"
		"  out -- single steps until the current subroutine/exception handler is exited (Shift-F11)\n"
		"  g[o] [<address>] -- resumes execution, sets temp breakpoint at <address> (F5)\n"
		"  gi[nt] [<irqline>] -- resumes execution, setting temp breakpoint if <irqline> is taken (F7)\n"
		"  gt[ime] <milliseconds> -- resumes execution until the given delay has elapsed\n"
		"  gv[blank] -- resumes execution, setting temp breakpoint on the next VBLANK (F8)\n"
		"  n[ext] -- executes until the next CPU switch (F6)\n"
		"  focus <cpu> -- focuses debugger only on <cpu>\n"
		"  ignore [<cpu>[,<cpu>[,...]]] -- stops debugging on <cpu>\n"
		"  observe [<cpu>[,<cpu>[,...]]] -- resumes debugging on <cpu>\n"
		"  trace {<filename>|OFF}[,<cpu>[,<action>]] -- trace the given CPU to a file (defaults to active CPU)\n"
		"  traceover {<filename>|OFF}[,<cpu>[,<action>]] -- trace the given CPU to a file, but skip subroutines (defaults to active CPU)\n"
		"  traceflush -- flushes all open trace files\n"
	},
	{
		"breakpoints",
		"\n"
		"Breakpoint Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  bp[set] <address>[,<condition>[,<action>]] -- sets breakpoint at <address>\n"
		"  bpclear [<bpnum>] -- clears a given breakpoint or all if no <bpnum> specified\n"
		"  bpdisable [<bpnum>] -- disables a given breakpoint or all if no <bpnum> specified\n"
		"  bpenable [<bpnum>] -- enables a given breakpoint or all if no <bpnum> specified\n"
		"  bplist -- lists all the breakpoints\n"
	},
	{
		"watchpoints",
		"\n"
		"Watchpoint Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  wp[set] <address>,<length>,<type>[,<condition>[,<action>]] -- sets program space watchpoint\n"
		"  wpd[set] <address>,<length>,<type>[,<condition>[,<action>]] -- sets data space watchpoint\n"
		"  wpi[set] <address>,<length>,<type>[,<condition>[,<action>]] -- sets I/O space watchpoint\n"
		"  wpclear [<wpnum>] -- clears a given watchpoint or all if no <wpnum> specified\n"
		"  wpdisable [<wpnum>] -- disables a given watchpoint or all if no <wpnum> specified\n"
		"  wpenable [<wpnum>] -- enables a given watchpoint or all if no <wpnum> specified\n"
		"  wplist -- lists all the watchpoints\n"
		"  hotspot [<cpu>,[<depth>[,<hits>]]] -- attempt to find hotspots\n"
	},
	{
		"registerpoints",
		"\n"
		"Registerpoint Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  rp[set] {<condition>}[,<action>] -- sets a registerpoint to trigger on <condition>\n"
		"  rpclear [<rpnum>] -- clears a given registerpoint or all if no <rpnum> specified\n"
		"  rpdisable [<rpnum>] -- disabled a given registerpoint or all if no <rpnum> specified\n"
		"  rpenable [<rpnum>]  -- enables a given registerpoint or all if no <rpnum> specified\n"
		"  rplist -- lists all the registerpoints\n"
	},
	{
		"expressions",
		"\n"
		"Expressions can be used anywhere a numeric parameter is expected. The syntax for expressions is "
		"very close to standard C-style syntax with full operator ordering and parentheses. There are a "
		"few operators missing (notably the trinary ? : operator), and a few new ones (memory accessors). "
		"The table below lists all the operators in their order, highest precedence operators first.\n"
		"\n"
		"  ( ) : standard parentheses\n"
		"  ++ -- : postfix increment/decrement\n"
		"  ++ -- ~ ! - + b@ w@ d@ q@ : prefix inc/dec, binary NOT, logical NOT, unary +/-, memory access\n"
		"  * / % : multiply, divide, modulus\n"
		"  + - : add, subtract\n"
		"  << >> : shift left/right\n"
		"  < <= > >= : less than, less than or equal, greater than, greater than or equal\n"
		"  == != : equal, not equal\n"
		"  & : binary AND\n"
		"  ^ : binary XOR\n"
		"  | : binary OR\n"
		"  && : logical AND\n"
		"  || : logical OR\n"
		"  = *= /= %= += -= <<= >>= &= |= ^= : assignment\n"
		"  , : separate terms, function parameters\n"
		"\n"
		"These are the differences from C behaviors. First, All math is performed on full 64-bit unsigned "
		"values, so things like a < 0 won't work as expected. Second, the logical operators && and || do "
		"not have short-circuit properties -- both halves are always evaluated. Finally, the new memory "
		"operators work like this: b@<addr> refers to the byte read from <addr>. Similarly, w@ refers to "
		"a word in memory, d@ refers to a dword in memory, and q@ refers to a qword in memory. The memory "
		"operators can be used as both lvalues and rvalues, so you can write b@100 = ff to store a byte "
		"in memory. By default these operators read from the program memory space, but you can override "
		"that by prefixing them with a 'd' or an 'i'. So dw@300 refers to data memory word at address "
		"300 and id@400 refers to an I/O memory dword at address 400.\n"
	},
	{
		"comments",
		"\n"
		"Code annotation commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  comadd[//] <address>,<comment> -- adds a comment to the disassembled code at given address\n"
		"  comdelete <address> -- removes a comment from the given address\n"
		"  comsave -- save the current comments to a file\n"
		"\n"
	},
	{
		"cheats",
		"\n"
		"Cheat Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  cheatinit [<address>,<length>[,<cpu>]] -- initialize the cheat search to the selected memory area\n"
		"  cheatrange <address>,<length> -- add to the cheat search the selected memory area\n"
		"  cheatnext <condition>[,<comparisonvalue>] -- continue cheat search comparing with the last value\n"
		"  cheatnextf <condition>[,<comparisonvalue>] -- continue cheat search comparing with the first value\n"
		"  cheatlist [<filename>] -- show the list of cheat search matches or save them to <filename>\n"
		"  cheatundo -- undo the last cheat search (state only)\n"
	},
	{
		"image",
		"\n"
		"Image Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  images -- lists all image devices and mounted files\n"
		"  mount <device>,<filename> -- mounts file to named device\n"
		"  unmount <device> -- unmounts file from named device\n"
	},
	{
		"do",
		"\n"
		"  do <expression>\n"
		"\n"
		"The do command simply evaluates the given <expression>. This is typically used to set or modify "
		"variables.\n"
		"\n"
		"Examples:\n"
		"\n"
		"do pc = 0\n"
		"  Sets the register 'pc' to 0.\n"
	},
	{
		"symlist",
		"\n"
		"  symlist [<cpu>]\n"
		"\n"
		"Lists registered symbols. If <cpu> is not specified, then symbols in the global symbol table are "
		"displayed; otherwise, the symbols for <cpu>'s specific CPU are displayed. Symbols are listed "
		"alphabetically. Read-only symbols are flagged with an asterisk.\n"
		"\n"
		"Examples:\n"
		"\n"
		"symlist\n"
		"  Displays the global symbol table.\n"
		"\n"
		"symlist 2\n"
		"  Displays the symbols specific to CPU #2.\n"
	},
	{
		"softreset",
		"\n"
		"  softreset\n"
		"\n"
		"Executes a soft reset.\n"
		"\n"
		"Examples:\n"
		"\n"
		"softreset\n"
		"  Executes a soft reset.\n"
	},
	{
		"hardreset",
		"\n"
		"  hardreset\n"
		"\n"
		"Executes a hard reset.\n"
		"\n"
		"Examples:\n"
		"\n"
		"hardreset\n"
		"  Executes a hard reset.\n"
	},
	{
		"print",
		"\n"
		"  print <item>[,...]\n"
		"\n"
		"The print command prints the results of one or more expressions to the debugger console as hexadecimal "
		"values.\n"
		"\n"
		"Examples:\n"
		"\n"
		"print pc\n"
		"  Prints the value of 'pc' to the console as a hex number.\n"
		"\n"
		"print a,b,a+b\n"
		"  Prints a, b, and the value of a+b to the console as hex numbers.\n"
	},
	{
		"printf",
		"\n"
		"  printf <format>[,<item>[,...]]\n"
		"\n"
		"The printf command performs a C-style printf to the debugger console. Only a very limited set of "
		"formatting options are available:\n"
		"\n"
		"  %[0][<n>]d -- prints <item> as a decimal value with optional digit count and zero-fill\n"
		"  %[0][<n>]x -- prints <item> as a hexadecimal value with optional digit count and zero-fill\n"
		"\n"
		"All remaining formatting options are ignored. Use %% together to output a % character. Multiple "
		"lines can be printed by embedding a \\n in the text.\n"
		"\n"
		"Examples:\n"
		"\n"
		"printf \"PC=%04X\",pc\n"
		"  Prints PC=<pcval> where <pcval> is displayed in hexadecimal with 4 digits with zero-fill.\n"
		"\n"
		"printf \"A=%d, B=%d\\nC=%d\",a,b,a+b\n"
		"  Prints A=<aval>, B=<bval> on one line, and C=<a+bval> on a second line.\n"
	},
	{
		"logerror",
		"\n"
		"  logerror <format>[,<item>[,...]]\n"
		"\n"
		"The logerror command performs a C-style printf to the error log. Only a very limited set of "
		"formatting options are available:\n"
		"\n"
		"  %[0][<n>]d -- logs <item> as a decimal value with optional digit count and zero-fill\n"
		"  %[0][<n>]x -- logs <item> as a hexadecimal value with optional digit count and zero-fill\n"
		"\n"
		"All remaining formatting options are ignored. Use %% together to output a % character. Multiple "
		"lines can be printed by embedding a \\n in the text.\n"
		"\n"
		"Examples:\n"
		"\n"
		"logerror \"PC=%04X\",pc\n"
		"  Logs PC=<pcval> where <pcval> is displayed in hexadecimal with 4 digits with zero-fill.\n"
		"\n"
		"logerror \"A=%d, B=%d\\nC=%d\",a,b,a+b\n"
		"  Logs A=<aval>, B=<bval> on one line, and C=<a+bval> on a second line.\n"
	},
	{
		"tracelog",
		"\n"
		"  tracelog <format>[,<item>[,...]]\n"
		"\n"
		"The tracelog command performs a C-style printf and routes the output to the currently open trace "
		"file (see the 'trace' command for details). If no file is currently open, tracelog does nothing. "
		"Only a very limited set of formatting options are available. See the 'printf' help for details.\n"
		"\n"
		"Examples:\n"
		"\n"
		"tracelog \"PC=%04X\",pc\n"
		"  Outputs PC=<pcval> where <pcval> is displayed in hexadecimal with 4 digits with zero-fill.\n"
		"\n"
		"printf \"A=%d, B=%d\\nC=%d\",a,b,a+b\n"
		"  Outputs A=<aval>, B=<bval> on one line, and C=<a+bval> on a second line.\n"
	},
	{
		"trackpc",
		"\n"
		"  trackpc [<bool>,<cpu>,<bool>]\n"
		"\n"
		"The trackpc command displays which program counters have already been visited in all disassembler "
		"windows. The first boolean argument toggles the process on and off.  The second argument is a "
		"cpu selector; if no cpu is specified, the current cpu is automatically selected.  The third argument "
		"is a boolean denoting if the existing data should be cleared or not.\n"
		"\n"
		"Examples:\n"
		"\n"
		"trackpc 1\n"
		"  Begin tracking the current cpu's pc.\n"
		"\n"
		"trackpc 1, 0, 1\n"
		"  Continue tracking pc on cpu 0, but clear existing track info.\n"
	},
	{
		"trackmem",
		"\n"
		"  trackmem [<bool>,<cpu>,<bool>]\n"
		"\n"
		"The trackmem command logs the PC at each time a memory address is written to.  "
		"The first boolean argument toggles the process on and off.  The second argument is a cpu "
		"selector; if no cpu is specified, the current cpu is automatically selected. The third argument "
		" is a boolean denoting if the existing data should be cleared or not.  Please refer to the "
		"pcatmem command for information on how to retrieve this data.  Also, right clicking in "
		"a memory window will display the logged PC for the given address.\n"
		"\n"
		"Examples:\n"
		"\n"
		"trackmem\n"
		"  Begin tracking the current CPU's pc.\n"
		"\n"
		"trackmem 1, 0, 1\n"
		"  Continue tracking memory writes on cpu 0, but clear existing track info.\n"
	},
	{
		"pcatmem",
		"\n"
		"  pcatmem(p/d/i) <address>[,<cpu>]\n"
		"\n"
		"The pcatmem command returns which PC wrote to a given memory address for the current CPU. "
		"The first argument is the requested address.  The second argument is a cpu selector; if no "
		"cpu is specified, the current cpu is automatically selected.  Right clicking in a memory window "
		"will also display the logged PC for the given address.\n"
		"\n"
		"Examples:\n"
		"\n"
		"pcatmem 400000\n"
		"  Print which PC wrote this CPU's memory location 0x400000.\n"
	},
	{
		"statesave[ss]",
		"\n"
		"  statesave[ss] <filename>\n"
		"\n"
		"The statesave command creates a save state at this exact moment in time. "
		"The given state file gets written to the standard state directory (sta), and gets .sta to it - "
		"no file extension necessary.  All output for this command is currently echoed into the "
		"running machine window.\n"
		"\n"
		"Examples:\n"
		"\n"
		"statesave foo\n"
		"  Writes file 'foo.sta' in the default state save directory.\n"
	},
	{
		"stateload[sl]",
		"\n"
		"  stateload[ss] <filename>\n"
		"\n"
		"The stateload command retrieves a save state from disk. "
		"The given state file gets read from the standard state directory (sta), and gets .sta to it - "
		"no file extension necessary.  All output for this command is currently echoed into the "
		"running machine window.  Previous memory and PC tracking statistics are cleared.\n"
		"\n"
		"Examples:\n"
		"\n"
		"stateload foo\n"
		"  Reads file 'foo.sta' from the default state save directory.\n"
	},
	{
		"snap",
		"\n"
		"  snap [[<filename>], <scrnum>]\n"
		"\n"
		"The snap command takes a snapshot of the current video display and saves it to the configured "
		"snapshot directory. If <filename> is specified explicitly, a single screenshot for <scrnum> is "
		"saved under the requested filename. If <filename> is omitted, all screens are saved using the "
		"same default rules as the \"save snapshot\" key in MAME proper.\n"
		"\n"
		"Examples:\n"
		"\n"
		"snap\n"
		"  Takes a snapshot of the current video screen and saves to the next non-conflicting filename "
		"  in the configured snapshot directory.\n"
		"\n"
		"snap shinobi\n"
		"  Takes a snapshot of the current video screen and saves it as 'shinobi.png' in the configured "
		"  snapshot directory.\n"
	},
	{
		"source",
		"\n"
		"  source <filename>\n"
		"\n"
		"The source command reads in a set of debugger commands from a file and executes them one by "
		"one, similar to a batch file.\n"
		"\n"
		"Examples:\n"
		"\n"
		"source break_and_trace.cmd\n"
		"  Reads in debugger commands from break_and_trace.cmd and executes them.\n"
	},
	{
		"quit",
		"\n"
		"  quit\n"
		"\n"
		"The quit command exits MAME immediately.\n"
	},
	{
		"dasm",
		"\n"
		"  dasm <filename>,<address>,<length>[,<opcodes>[,<cpu>]]\n"
		"\n"
		"The dasm command disassembles program memory to the file specified in the <filename> parameter. "
		"<address> indicates the address of the start of disassembly, and <length> indicates how much "
		"memory to disassemble. The range <address> through <address>+<length>-1 inclusive will be "
		"output to the file. By default, the raw opcode data is output with each line. The optional "
		"<opcodes> parameter can be used to enable (1) or disable(0) this feature. Finally, you can "
		"disassemble code from another CPU by specifying the <cpu> parameter.\n"
		"\n"
		"Examples:\n"
		"\n"
		"dasm venture.asm,0,10000\n"
		"  Disassembles addresses 0-ffff in the current CPU, including raw opcode data, to the "
		"file 'venture.asm'.\n"
		"\n"
		"dasm harddriv.asm,3000,1000,0,2\n"
		"  Disassembles addresses 3000-3fff from CPU #2, with no raw opcode data, to the file "
		"'harddriv.asm'.\n"
	},
	{
		"find",
		"\n"
		"  f[ind][{d|i}] <address>,<length>[,<data>[,...]]\n"
		"\n"
		"The find/findd/findi commands search through memory for the specified sequence of data. "
		"'find' will search program space memory, while 'findd' will search data space memory "
		"and 'findi' will search I/O space memory. <address> indicates the address to begin searching, "
		"and <length> indicates how much memory to search. <data> can either be a quoted string "
		"or a numeric value or expression or the wildcard character '?'. Strings by default imply a "
		"byte-sized search; non-string data is searched by default in the native word size of the CPU. "
		"To override the search size for non-strings, you can prefix the value with b. to force byte- "
		"sized search, w. for word-sized search, d. for dword-sized, and q. for qword-sized. Overrides "
		"are remembered, so if you want to search for a series of words, you need only to prefix the "
		"first value with a w. Note also that you can intermix sizes in order to perform more complex "
		"searches. The entire range <address> through <address>+<length>-1  inclusive will be searched "
		"for the sequence, and all occurrences will be displayed.\n"
		"\n"
		"Examples:\n"
		"\n"
		"find 0,10000,\"HIGH SCORE\",0\n"
		"  Searches the address range 0-ffff in the current CPU for the string \"HIGH SCORE\" followed "
		"by a 0 byte.\n"
		"\n"
		"findd 3000,1000,w.abcd,4567\n"
		"  Searches the data memory address range 3000-3fff for the word-sized value abcd followed by "
		"the word-sized value 4567.\n"
		"\n"
		"find 0,8000,\"AAR\",d.0,\"BEN\",w.0\n"
		"  Searches the address range 0000-7fff for the string \"AAR\" followed by a dword-sized 0 "
		"followed by the string \"BEN\", followed by a word-sized 0.\n"
	},
	{
		"dump",
		"\n"
		"  dump[{d|i}] <filename>,<address>,<length>[,<size>[,<ascii>[,<cpu>]]]\n"
		"\n"
		"The dump/dumpd/dumpi commands dump memory to the text file specified in the <filename> "
		"parameter. 'dump' will dump program space memory, while 'dumpd' will dump data space memory "
		"and 'dumpi' will dump I/O space memory. <address> indicates the address of the start of dumping, "
		"and <length> indicates how much memory to dump. The range <address> through <address>+<length>-1 "
		"inclusive will be output to the file. By default, the data will be output in byte format, unless "
		"the underlying address space is word/dword/qword-only. You can override this by specifying the "
		"<size> parameter, which can be used to group the data in 1, 2, 4 or 8-byte chunks. The optional "
		"<ascii> parameter can be used to enable (1) or disable (0) the output of ASCII characters to the "
		"right of each line; by default, this is enabled. Finally, you can dump memory from another CPU "
		"by specifying the <cpu> parameter.\n"
		"\n"
		"Examples:\n"
		"\n"
		"dump venture.dmp,0,10000\n"
		"  Dumps addresses 0-ffff in the current CPU in 1-byte chunks, including ASCII data, to "
		"the file 'venture.dmp'.\n"
		"\n"
		"dumpd harddriv.dmp,3000,1000,4,0,3\n"
		"  Dumps data memory addresses 3000-3fff from CPU #3 in 4-byte chunks, with no ASCII data, "
		"to the file 'harddriv.dmp'.\n"
	},
	{
		"save",
		"\n"
		"  save[{d|i}] <filename>,<address>,<length>[,<cpu>]\n"
		"\n"
		"The save/saved/savei commands save raw memory to the binary file specified in the <filename> "
		"parameter. 'save' will save program space memory, while 'saved' will save data space memory "
		"and 'savei' will save I/O space memory. <address> indicates the address of the start of saving, "
		"and <length> indicates how much memory to save. The range <address> through <address>+<length>-1 "
		"inclusive will be output to the file. You can also save memory from another CPU by specifying the "
		"<cpu> parameter.\n"
		"\n"
		"Examples:\n"
		"\n"
		"save venture.bin,0,10000\n"
		"  Saves addresses 0-ffff in the current CPU to the binary file 'venture.bin'.\n"
		"\n"
		"saved harddriv.bin,3000,1000,3\n"
		"  Saves data memory addresses 3000-3fff from CPU #3 to the binary file 'harddriv.bin'.\n"
	},
	{
		"load",
		"\n"
		"  load[{d|i}] <filename>,<address>,<length>[,<cpu>]\n"
		"\n"
		"The load/loadd/loadi commands load raw memory from the binary file specified in the <filename> "
		"parameter. 'load' will load program space memory, while 'loadd' will load data space memory "
		"and 'loadi' will load I/O space memory. <address> indicates the address of the start of saving, "
		"and <length> indicates how much memory to load. The range <address> through <address>+<length>-1 "
		"inclusive will be read in from the file. If you specify <length> = 0 or a length greater than the "
		"total length of the file it will load the entire contents of the file and no more. You can also load "
		"memory from another CPU by specifying the <cpu> parameter.\n"
		"NOTE: This will only actually write memory that is possible to overwrite in the Memory Window\n"
		"\n"
		"Examples:\n"
		"\n"
		"load venture.bin,0,10000\n"
		"  Loads addresses 0-ffff in the current CPU from the binary file 'venture.bin'.\n"
		"\n"
		"loadd harddriv.bin,3000,1000,3\n"
		"  Loads data memory addresses 3000-3fff from CPU #3 from the binary file 'harddriv.bin'.\n"
	},
	{
		"step",
		"\n"
		"  s[tep] [<count>=1]\n"
		"\n"
		"The step command single steps one or more instructions in the currently executing CPU. By "
		"default, step executes one instruction each time it is issued. You can also tell step to step "
		"multiple instructions by including the optional <count> parameter.\n"
		"\n"
		"Examples:\n"
		"\n"
		"s\n"
		"  Steps forward one instruction on the current CPU.\n"
		"\n"
		"step 4\n"
		"  Steps forward four instructions on the current CPU.\n"
	},
	{
		"over",
		"\n"
		"  o[ver] [<count>=1]\n"
		"\n"
		"The over command single steps \"over\" one or more instructions in the currently executing CPU, "
		"stepping over subroutine calls and exception handler traps and counting them as a single "
		"instruction. Note that when stepping over a subroutine call, code may execute on other CPUs "
		"before the subroutine call completes. By default, over executes one instruction each time it is "
		"issued. You can also tell step to step multiple instructions by including the optional <count> "
		"parameter.\n"
		"\n"
		"Note that the step over functionality may not be implemented on all CPU types. If it is not "
		"implemented, then 'over' will behave exactly like 'step'.\n"
		"\n"
		"Examples:\n"
		"\n"
		"o\n"
		"  Steps forward over one instruction on the current CPU.\n"
		"\n"
		"over 4\n"
		"  Steps forward over four instructions on the current CPU.\n"
	},
	{
		"out",
		"\n"
		"  out\n"
		"\n"
		"The out command single steps until it encounters a return from subroutine or return from "
		"exception instruction. Note that because it detects return from exception conditions, if you "
		"attempt to step out of a subroutine and an interrupt/exception occurs before you hit the end, "
		"then you may stop prematurely at the end of the exception handler.\n"
		"\n"
		"Note that the step out functionality may not be implemented on all CPU types. If it is not "
		"implemented, then 'out' will behave exactly like 'step'.\n"
		"\n"
		"Examples:\n"
		"\n"
		"out\n"
		"  Steps until the current subroutine or exception handler returns.\n"
	},
	{
		"go",
		"\n"
		"  g[o] [<address>]\n"
		"\n"
		"The go command resumes execution of the current code. Control will not be returned to the "
		"debugger until a breakpoint or watchpoint is hit, or until you manually break in using the "
		"assigned key. The go command takes an optional <address> parameter which is a temporary "
		"unconditional breakpoint that is set before executing, and automatically removed when hit. "
		"\n"
		"Examples:\n"
		"\n"
		"g\n"
		"  Resume execution until the next break/watchpoint or until a manual break.\n"
		"\n"
		"g 1234\n"
		"  Resume execution, stopping at address 1234 unless something else stops us first.\n"
	},
	{
		"gvblank",
		"\n"
		"  gv[blank]\n"
		"\n"
		"The gvblank command resumes execution of the current code. Control will not be returned to "
		"the debugger until a breakpoint or watchpoint is hit, or until the next VBLANK occurs in the "
		"emulator.\n"
		"\n"
		"Examples:\n"
		"\n"
		"gv\n"
		"  Resume execution until the next break/watchpoint or until the next VBLANK.\n"
	},
	{
		"gint",
		"\n"
		"  gi[nt] [<irqline>]\n"
		"\n"
		"The gint command resumes execution of the current code. Control will not be returned to the "
		"debugger until a breakpoint or watchpoint is hit, or until an IRQ is asserted and acknowledged "
		"on the current CPU. You can specify <irqline> if you wish to stop execution only on a particular "
		"IRQ line being asserted and acknowledged. If <irqline> is omitted, then any IRQ line will stop "
		"execution.\n"
		"\n"
		"Examples:\n"
		"\n"
		"gi\n"
		"  Resume execution until the next break/watchpoint or until any IRQ is asserted and acknowledged "
		"on the current CPU.\n"
		"\n"
		"gint 4\n"
		"  Resume execution until the next break/watchpoint or until IRQ line 4 is asserted and "
		"acknowledged on the current CPU.\n"
	},
	{
		"gtime",
		"\n"
		"  gt[ime] <milliseconds>\n"
		"\n"
		"The gtime command resumes execution of the current code. Control will not be returned to the "
		"debugger until a specified delay has elapsed. The delay is in milliseconds.\n"
		"\n"
		"Example:\n"
		"\n"
		"gtime #10000\n"
		"  Resume execution for ten seconds\n"
	},
	{
		"next",
		"\n"
		"  n[ext]\n"
		"\n"
		"The next command resumes execution and continues executing until the next time a different "
		"CPU is scheduled. Note that if you have used 'ignore' to ignore certain CPUs, you will not "
		"stop until a non-'ignore'd CPU is scheduled.\n"
	},
	{
		"focus",
		"\n"
		"  focus <cpu>\n"
		"\n"
		"Sets the debugger focus exclusively to the given <cpu>. This is equivalent to specifying "
		"'ignore' on all other CPUs.\n"
		"\n"
		"Examples:\n"
		"\n"
		"focus 1\n"
		"  Focus exclusively CPU #1 while ignoring all other CPUs when using the debugger.\n"
	},
	{
		"ignore",
		"\n"
		"  ignore [<cpu>[,<cpu>[,...]]]\n"
		"\n"
		"Ignores the specified <cpu> in the debugger. This means that you won't ever see execution "
		"on that CPU, nor will you be able to set breakpoints on that CPU. To undo this change use "
		"the 'observe' command. You can specify multiple <cpu>s in a single command. Note also that "
		"you are not permitted to ignore all CPUs; at least one must be active at all times.\n"
		"\n"
		"Examples:\n"
		"\n"
		"ignore 1\n"
		"  Ignore CPU #1 when using the debugger.\n"
		"\n"
		"ignore 2,3,4\n"
		"  Ignore CPU #2, #3 and #4 when using the debugger.\n"
		"\n"
		"ignore\n"
		"  List the CPUs that are currently ignored.\n"
	},
	{
		"observe",
		"\n"
		"  observe [<cpu>[,<cpu>[,...]]]\n"
		"\n"
		"Re-enables interaction with the specified <cpu> in the debugger. This command undoes the "
		"effects of the 'ignore' command. You can specify multiple <cpu>s in a single command.\n"
		"\n"
		"Examples:\n"
		"\n"
		"observe 1\n"
		"  Stop ignoring CPU #1 when using the debugger.\n"
		"\n"
		"observe 2,3,4\n"
		"  Stop ignoring CPU #2, #3 and #4 when using the debugger.\n"
		"\n"
		"observe\n"
		"  List the CPUs that are currently observed.\n"
	},
	{
		"trace",
		"\n"
		"  trace {<filename>|OFF}[,<cpu>[,<action>]]\n"
		"\n"
		"Starts or stops tracing of the execution of the specified <cpu>. If <cpu> is omitted, "
		"the currently active CPU is specified. When enabling tracing, specify the filename in the "
		"<filename> parameter. To disable tracing, substitute the keyword 'off' for <filename>. If you "
		"wish to log additional information on each trace, you can append an <action> parameter which "
		"is a command that is executed before each trace is logged. Generally, this is used to include "
		"a 'tracelog' command. Note that you may need to embed the action within braces { } in order "
		"to prevent commas and semicolons from being interpreted as applying to the trace command "
		"itself.\n"
		"\n"
		"Examples:\n"
		"\n"
		"trace dribling.tr,0\n"
		"  Begin tracing the execution of CPU #0, logging output to dribling.tr.\n"
		"\n"
		"trace joust.tr\n"
		"  Begin tracing the currently active CPU, logging output to joust.tr.\n"
		"\n"
		"trace >>pigskin.tr\n"
		"  Begin tracing the currently active CPU, appending log output to pigskin.tr.\n"
		"\n"
		"trace off,0\n"
		"  Turn off tracing on CPU #0.\n"
		"\n"
		"trace asteroid.tr,0,{tracelog \"A=%02X \",a}\n"
		"  Begin tracing the execution of CPU #0, logging output to asteroid.tr. Before each line, "
		"output A=<aval> to the tracelog.\n"
	},
	{
		"traceover",
		"\n"
		"  traceover {<filename>|OFF}[,<cpu>[,<action>]]\n"
		"\n"
		"Starts or stops tracing of the execution of the specified <cpu>. When tracing reaches "
		"a subroutine or call, tracing will skip over the subroutine. The same algorithm is used as is "
		"used in the step over command. This means that traceover will not work properly when calls "
		"are recusive or the return address is not immediately following the call instruction. If "
		"<cpu> is omitted, the currently active CPU is specified. When enabling tracing, specify the "
		"filename in the <filename> parameter. To disable tracing, substitute the keyword 'off' for "
		"<filename>. If you wish to log additional information on each trace, you can append an <action> "
		"parameter which is a command that is executed before each trace is logged. Generally, this is "
		"used to include a 'tracelog' command. Note that you may need to embed the action within braces "
		"{ } in order to prevent commas and semicolons from being interpreted as applying to the trace "
		"command itself.\n"
		"\n"
		"Examples:\n"
		"\n"
		"traceover dribling.tr,0\n"
		"  Begin tracing the execution of CPU #0, logging output to dribling.tr.\n"
		"\n"
		"traceover joust.tr\n"
		"  Begin tracing the currently active CPU, logging output to joust.tr.\n"
		"\n"
		"traceover off,0\n"
		"  Turn off tracing on CPU #0.\n"
		"\n"
		"traceover asteroid.tr,0,{tracelog \"A=%02X \",a}\n"
		"  Begin tracing the execution of CPU #0, logging output to asteroid.tr. Before each line, "
		"output A=<aval> to the tracelog.\n"
	},
	{
		"traceflush",
		"\n"
		"  traceflush\n"
		"\n"
		"Flushes all open trace files.\n"
	},
	{
		"bpset",
		"\n"
		"  bp[set] <address>[,<condition>[,<action>]]\n"
		"\n"
		"Sets a new execution breakpoint at the specified <address>. The optional <condition> "
		"parameter lets you specify an expression that will be evaluated each time the breakpoint is "
		"hit. If the result of the expression is true (non-zero), the breakpoint will actually halt "
		"execution; otherwise, execution will continue with no notification. The optional <action> "
		"parameter provides a command that is executed whenever the breakpoint is hit and the "
		"<condition> is true. Note that you may need to embed the action within braces { } in order "
		"to prevent commas and semicolons from being interpreted as applying to the bpset command "
		"itself. Each breakpoint that is set is assigned an index which can be used in other "
		"breakpoint commands to reference this breakpoint.\n"
		"\n"
		"Examples:\n"
		"\n"
		"bp 1234\n"
		"  Set a breakpoint that will halt execution whenever the PC is equal to 1234.\n"
		"\n"
		"bp 23456,a0 == 0 && a1 == 0\n"
		"  Set a breakpoint that will halt execution whenever the PC is equal to 23456 AND the "
		"expression (a0 == 0 && a1 == 0) is true.\n"
		"\n"
		"bp 3456,1,{printf \"A0=%08X\\n\",a0; g}\n"
		"  Set a breakpoint that will halt execution whenever the PC is equal to 3456. When "
		"this happens, print A0=<a0val> and continue executing.\n"
		"\n"
		"bp 45678,a0==100,{a0 = ff; g}\n"
		"  Set a breakpoint that will halt execution whenever the PC is equal to 45678 AND the "
		"expression (a0 == 100) is true. When that happens, set a0 to ff and resume execution.\n"
		"\n"
		"temp0 = 0; bp 567890,++temp0 >= 10\n"
		"  Set a breakpoint that will halt execution whenever the PC is equal to 567890 AND the "
		"expression (++temp0 >= 10) is true. This effectively breaks only after the breakpoint "
		"has been hit 16 times.\n"
	},
	{
		"bpclear",
		"\n"
		"  bpclear [<bpnum>]\n"
		"\n"
		"The bpclear command clears a breakpoint. If <bpnum> is specified, only the requested "
		"breakpoint is cleared, otherwise all breakpoints are cleared.\n"
		"\n"
		"Examples:\n"
		"\n"
		"bpclear 3\n"
		"  Clear breakpoint index 3.\n"
		"\n"
		"bpclear\n"
		"  Clear all breakpoints.\n"
	},
	{
		"bpdisable",
		"\n"
		"  bpdisable [<bpnum>]\n"
		"\n"
		"The bpdisable command disables a breakpoint. If <bpnum> is specified, only the requested "
		"breakpoint is disabled, otherwise all breakpoints are disabled. Note that disabling a "
		"breakpoint does not delete it, it just temporarily marks the breakpoint as inactive.\n"
		"\n"
		"Examples:\n"
		"\n"
		"bpdisable 3\n"
		"  Disable breakpoint index 3.\n"
		"\n"
		"bpdisable\n"
		"  Disable all breakpoints.\n"
	},
	{
		"bpenable",
		"\n"
		"  bpenable [<bpnum>]\n"
		"\n"
		"The bpenable command enables a breakpoint. If <bpnum> is specified, only the requested "
		"breakpoint is enabled, otherwise all breakpoints are enabled.\n"
		"\n"
		"Examples:\n"
		"\n"
		"bpenable 3\n"
		"  Enable breakpoint index 3.\n"
		"\n"
		"bpenable\n"
		"  Enable all breakpoints.\n"
	},
	{
		"bplist",
		"\n"
		"  bplist\n"
		"\n"
		"The bplist command lists all the current breakpoints, along with their index and any "
		"conditions or actions attached to them.\n"
	},
	{
		"wpset",
		"\n"
		"  wp[{d|i}][set] <address>,<length>,<type>[,<condition>[,<action>]]\n"
		"\n"
		"Sets a new watchpoint starting at the specified <address> and extending for <length>. The "
		"inclusive range of the watchpoint is <address> through <address> + <length> - 1. The 'wpset' "
		"command sets a watchpoint on program memory; the 'wpdset' command sets a watchpoint on data "
		"memory; and the 'wpiset' sets a watchpoint on I/O memory. The <type> parameter specifies "
		"which sort of accesses to trap on. It can be one of three values: 'r' for a read watchpoint "
		"'w' for a write watchpoint, and 'rw' for a read/write watchpoint.\n"
		"\n"
		"The optional <condition> parameter lets you specify an expression that will be evaluated each "
		"time the watchpoint is hit. If the result of the expression is true (non-zero), the watchpoint "
		"will actually halt execution; otherwise, execution will continue with no notification. The "
		"optional <action> parameter provides a command that is executed whenever the watchpoint is hit "
		"and the <condition> is true. Note that you may need to embed the action within braces { } in "
		"order to prevent commas and semicolons from being interpreted as applying to the wpset command "
		"itself. Each watchpoint that is set is assigned an index which can be used in other "
		"watchpoint commands to reference this watchpoint.\n"
		"\n"
		"In order to help <condition> expressions, two variables are available. For all watchpoints, "
		"the variable 'wpaddr' is set to the address that actually triggered the watchpoint. For write "
		"watchpoints, the variable 'wpdata' is set to the data that is being written.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wp 1234,6,rw\n"
		"  Set a watchpoint that will halt execution whenever a read or write occurs in the address "
		"range 1234-1239 inclusive.\n"
		"\n"
		"wp 23456,a,w,wpdata == 1\n"
		"  Set a watchpoint that will halt execution whenever a write occurs in the address range "
		"23456-2345f AND the data written is equal to 1.\n"
		"\n"
		"wp 3456,20,r,1,{printf \"Read @ %08X\\n\",wpaddr; g}\n"
		"  Set a watchpoint that will halt execution whenever a read occurs in the address range "
		"3456-3475. When this happens, print Read @ <wpaddr> and continue executing.\n"
		"\n"
		"temp0 = 0; wp 45678,1,w,wpdata==f0,{temp0++; g}\n"
		"  Set a watchpoint that will halt execution whenever a write occurs to the address 45678 AND "
		"the value being written is equal to f0. When that happens, increment the variable temp0 and "
		"resume execution.\n"
	},
	{
		"wpclear",
		"\n"
		"  wpclear [<wpnum>]\n"
		"\n"
		"The wpclear command clears a watchpoint. If <wpnum> is specified, only the requested "
		"watchpoint is cleared, otherwise all watchpoints are cleared.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wpclear 3\n"
		"  Clear watchpoint index 3.\n"
		"\n"
		"wpclear\n"
		"  Clear all watchpoints.\n"
	},
	{
		"wpdisable",
		"\n"
		"  wpdisable [<wpnum>]\n"
		"\n"
		"The wpdisable command disables a watchpoint. If <wpnum> is specified, only the requested "
		"watchpoint is disabled, otherwise all watchpoints are disabled. Note that disabling a "
		"watchpoint does not delete it, it just temporarily marks the watchpoint as inactive.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wpdisable 3\n"
		"  Disable watchpoint index 3.\n"
		"\n"
		"wpdisable\n"
		"  Disable all watchpoints.\n"
	},
	{
		"wpenable",
		"\n"
		"  wpenable [<wpnum>]\n"
		"\n"
		"The wpenable command enables a watchpoint. If <wpnum> is specified, only the requested "
		"watchpoint is enabled, otherwise all watchpoints are enabled.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wpenable 3\n"
		"  Enable watchpoint index 3.\n"
		"\n"
		"wpenable\n"
		"  Enable all watchpoints.\n"
	},
	{
		"wplist",
		"\n"
		"  wplist\n"
		"\n"
		"The wplist command lists all the current watchpoints, along with their index and any "
		"conditions or actions attached to them.\n"
	},
	{
		"hotspot",
		"\n"
		"  hotspot [<cpu>,[<depth>[,<hits>]]]\n"
		"\n"
		"The hotspot command attempts to help locate hotspots in the code where speedup opportunities "
		"might be present. <cpu>, which defaults to the currently active CPU, specified which "
		"processor's memory to track. <depth>, which defaults to 64, controls the depth of the search "
		"buffer. The search buffer tracks the last <depth> memory reads from unique PCs. The <hits> "
		"parameter, which defaults to 250, specifies the minimum number of hits to report.\n"
		"\n"
		"The basic theory of operation is like this: each memory read is trapped by the debugger and "
		"logged in the search buffer according to the address which was read and the PC that executed "
		"the opcode. If the search buffer already contains a matching entry, that entry's count is "
		"incremented and the entry is moved to the top of the list. If the search buffer does not "
		"contain a matching entry, the entry from the bottom of the list is removed, and a new entry "
		"is created at the top with an initial count of 1. Entries which fall off the bottom are "
		"examined and if their count is larger than <hits>, they are reported to the debugger "
		"console.\n"
		"\n"
		"Examples:\n"
		"\n"
		"hotspot 0,10\n"
		"  Looks for hotspots on CPU 0 using a search buffer of 16 entries, reporting any entries which "
		"end up with 250 or more hits.\n"
		"\n"
		"hotspot 1,40,#1000\n"
		"  Looks for hotspots on CPU 1 using a search buffer of 64 entries, reporting any entries which "
		"end up with 1000 or more hits.\n"
	},
	{
		"rpset",
		"\n"
		"  rp[set] {<condition>}[,<action>]]\n"
		"\n"
		"Sets a new registerpoint which will be triggered when <condition> is met. The condition must "
		"be specified between curly braces to prevent the condition from being evaluated as an "
		"assignment.\n"
		"\n"
		"The optional <action> parameter provides a command that is executed whenever the registerpoint "
		"is hit. Note that you may need to embed the action within braces { } in "
		"order to prevent commas and semicolons from being interpreted as applying to the rpset command "
		"itself. Each registerpoint that is set is assigned an index which can be used in other "
		"registerpoint commands to reference this registerpoint.\n"
		"\n"
		"Examples:\n"
		"\n"
		"rp {PC==0150}\n"
		"  Set a registerpoint that will halt execution whenever the PC register equals 0x150.\n"
		"\n"
		"temp0=0; rp {PC==0150},{temp0++; g}\n"
		"  Set a registerpoint that will increment the variable temp0 whenever the PC register "
		"equals 0x0150.\n"
		"\n"
		"rp {temp0==5}\n"
		"  Set a registerpoint that will halt execution whenever the temp0 variable equals 5.\n"
	},
	{
		"rpclear",
		"\n"
		"  rpclear [<rpnum>]\n"
		"\n"
		"The rpclear command clears a registerpoint. If <rpnum> is specified, only the requested "
		"registerpoint is cleared, otherwise all registerpoints are cleared.\n"
		"\n"
		"Examples:\n"
		"\n"
		"rpclear 3\n"
		"  Clear registerpoint index 3.\n"
		"\n"
		"rpclear\n"
		"  Clear all registerpoints.\n"
	},
	{
		"rpdisable",
		"\n"
		"  rpdisable [<rpnum>]\n"
		"\n"
		"The rpdisable command disables a registerpoint. If <rpnum> is specified, only the requested "
		"registerpoint is disabled, otherwise all registerpoints are disabled. Note that disabling a "
		"registerpoint does not delete it, it just temporarily marks the registerpoint as inactive.\n"
		"\n"
		"Examples:\n"
		"\n"
		"rpdisable 3\n"
		"  Disable registerpoint index 3.\n"
		"\n"
		"rpdisable\n"
		"  Disable all registerpoints.\n"
	},
	{
		"rpenable",
		"\n"
		"  rpenable [<rpnum>]\n"
		"\n"
		"The rpenable command enables a registerpoint. If <rpnum> is specified, only the requested "
		"registerpoint is enabled, otherwise all registerpoints are enabled.\n"
		"\n"
		"Examples:\n"
		"\n"
		"rpenable 3\n"
		"  Enable registerpoint index 3.\n"
		"\n"
		"rpenable\n"
		"  Enable all registerpoints.\n"
	},
	{
		"rplist",
		"\n"
		"  rplist\n"
		"\n"
		"The rplist command lists all the current registerpoints, along with their index and any "
		"actions attached to them.\n"
	},
	{
		"map",
		"\n"
		"  map[{d|i}] <address>\n"
		"\n"
		"The map/mapd/mapi commands map a logical address in memory to the correct physical address, as "
		"well as specifying the bank. 'map' will map program space memory, while 'mapd' will map data space "
		"memory and 'mapi' will map I/O space memory.\n"
		"\n"
		"Example:\n"
		"\n"
		"map 152d0\n"
		"  Gives physical address and bank for logical address 152d0 in program memory\n"
	},
	{
		"memdump",
		"\n"
		"  memdump [<filename>]\n"
		"\n"
		"Dumps the current memory map to <filename>. If <filename> is omitted, then dumps to memdump.log"
		"\n"
		"Examples:\n"
		"\n"
		"memdump mylog.log\n"
		"  Dumps memory to mylog.log.\n"
		"\n"
		"memdump\n"
		"  Dumps memory to memdump.log.\n"
	},
	{
		"comadd",
		"\n"
		"  comadd[//] <address>,<comment>\n"
		"\n"
		"Adds a string <comment> to the disassembled code at <address>. The shortcut for this command is simply "
		"'//'\n"
		"\n"
		"Examples:\n"
		"\n"
		"comadd 0, hello world.\n"
		"  Adds the comment 'hello world.' to the code at address 0x0\n"
		"\n"
		"// 10, undocumented opcode!\n"
		"  Adds the comment 'undocumented opcode!' to the code at address 0x10\n"
		"\n"
	},
	{
		"comsave",
		"\n"
		"  comsave\n"
		"\n"
		"Saves the working comments to the driver's XML comment file.\n"
		"\n"
		"Examples:\n"
		"\n"
		"comsave\n"
		"  Saves the comments to the driver's comment file\n"
		"\n"
	},
	{
		"comdelete",
		"\n"
		"  comdelete\n"
		"\n"
		"Deletes the comment at the specified memory offset. "
		"The comment which is deleted is in the currently active memory bank.\n"
		"\n"
		"Examples:\n"
		"\n"
		"comdelete 10\n"
		"  Deletes the comment at code address 0x10 (using the current memory bank settings)\n"
		"\n"
	},
	{
		"cheatinit",
		"\n"
		"  cheatinit [<sign><width><swap>,[<address>,<length>[,<cpu>]]]\n"
		"\n"
		"The cheatinit command initializes the cheat search to the selected memory area.\n"
		"If no parameter is specified the cheat search is initialized to all changeable memory of the main cpu.\n"
		"<sign> can be s(signed) or u(unsigned)\n"
		"<width> can be b(8 bit), w(16 bit), d(32 bit) or q(64 bit)\n"
		"<swap> append s for swapped search\n"
		"\n"
		"Examples:\n"
		"\n"
		"cheatinit ub,0x1000,0x10\n"
		"  Initialize the cheat search from 0x1000 to 0x1010 of the first CPU.\n"
		"\n"
		"cheatinit sw,0x2000,0x1000,1\n"
		"  Initialize the cheat search with width of 2 byte in signed mode from 0x2000 to 0x3000 of the second CPU.\n"
		"\n"
		"cheatinit uds,0x0000,0x1000\n"
		"  Initialize the cheat search with width of 4 byte swapped from 0x0000 to 0x1000.\n"
	},
	{
		"cheatrange",
		"\n"
		"  cheatrange <address>,<length>\n"
		"\n"
		"The cheatrange command adds the selected memory area to the cheat search.\n"
		"Before using cheatrange it is necessary to initialize the cheat search with cheatinit.\n"
		"\n"
		"Examples:\n"
		"\n"
		"cheatrange 0x1000,0x10\n"
		"  Add the bytes from 0x1000 to 0x1010 to the cheat search.\n"
	},
	{
		"cheatnext",
		"\n"
		"  cheatnext <condition>[,<comparisonvalue>]\n"
		"\n"
		"The cheatnext command will make comparisons with the last search matches.\n"
		"Possible <condition>:\n"
		"  all\n"
		"   no <comparisonvalue> needed.\n"
		"   use to update the last value without changing the current matches.\n"
		"  equal [eq]\n"
		"   without <comparisonvalue> search for all bytes that are equal to the last search.\n"
		"   with <comparisonvalue> search for all bytes that are equal to the <comparisonvalue>.\n"
		"  notequal [ne]\n"
		"   without <comparisonvalue> search for all bytes that are not equal to the last search.\n"
		"   with <comparisonvalue> search for all bytes that are not equal to the <comparisonvalue>.\n"
		"  decrease [de, +]\n"
		"   without <comparisonvalue> search for all bytes that have decreased since the last search.\n"
		"   with <comparisonvalue> search for all bytes that have decreased by the <comparisonvalue> since the last search.\n"
		"  increase [in, -]\n"
		"   without <comparisonvalue> search for all bytes that have increased since the last search.\n"
		"   with <comparisonvalue> search for all bytes that have increased by the <comparisonvalue> since the last search.\n"
		"  decreaseorequal [deeq]\n"
		"   no <comparisonvalue> needed.\n"
		"   search for all bytes that have decreased or have same value since the last search.\n"
		"  increaseorequal [ineq]\n"
		"   no <comparisonvalue> needed.\n"
		"   search for all bytes that have decreased or have same value since the last search.\n"
		"  smallerof [lt]\n"
		"   without <comparisonvalue> this condition is invalid\n"
		"   with <comparisonvalue> search for all bytes that are smaller than the <comparisonvalue>.\n"
		"  greaterof [gt]\n"
		"   without <comparisonvalue> this condition is invalid\n"
		"   with <comparisonvalue> search for all bytes that are larger than the <comparisonvalue>.\n"
		"  changedby [ch, ~]\n"
		"   without <comparisonvalue> this condition is invalid\n"
		"   with <comparisonvalue> search for all bytes that have changed by the <comparisonvalue> since the last search.\n"
		"\n"
		"Examples:\n"
		"\n"
		"cheatnext increase\n"
		"  search for all bytes that have increased since the last search.\n"
		"\n"
		"cheatnext decrease, 1\n"
		"  search for all bytes that have decreased by 1 since the last search.\n"
	},
	{
		"cheatnextf",
		"\n"
		"  cheatnextf <condition>[,<comparisonvalue>]\n"
		"\n"
		"The cheatnextf command will make comparisons with the initial search.\n"
		"Possible <condition>:\n"
		"  all\n"
		"   no <comparisonvalue> needed.\n"
		"   use to update the last value without changing the current matches.\n"
		"  equal [eq]\n"
		"   without <comparisonvalue> search for all bytes that are equal to the initial search.\n"
		"   with <comparisonvalue> search for all bytes that are equal to the <comparisonvalue>.\n"
		"  notequal [ne]\n"
		"   without <comparisonvalue> search for all bytes that are not equal to the initial search.\n"
		"   with <comparisonvalue> search for all bytes that are not equal to the <comparisonvalue>.\n"
		"  decrease [de, +]\n"
		"   without <comparisonvalue> search for all bytes that have decreased since the initial search.\n"
		"   with <comparisonvalue> search for all bytes that have decreased by the <comparisonvalue> since the initial search.\n"
		"  increase [in, -]\n"
		"   without <comparisonvalue> search for all bytes that have increased since the initial search.\n"
		"   with <comparisonvalue> search for all bytes that have increased by the <comparisonvalue> since the initial search.\n"
		"  decreaseorequal [deeq]\n"
		"   no <comparisonvalue> needed.\n"
		"   search for all bytes that have decreased or have same value since the initial search.\n"
		"  increaseorequal [ineq]\n"
		"   no <comparisonvalue> needed.\n"
		"   search for all bytes that have decreased or have same value since the initial search.\n"
		"  smallerof [lt]\n"
		"   without <comparisonvalue> this condition is invalid.\n"
		"   with <comparisonvalue> search for all bytes that are smaller than the <comparisonvalue>.\n"
		"  greaterof [gt]\n"
		"   without <comparisonvalue> this condition is invalid.\n"
		"   with <comparisonvalue> search for all bytes that are larger than the <comparisonvalue>.\n"
		"  changedby [ch, ~]\n"
		"   without <comparisonvalue> this condition is invalid\n"
		"   with <comparisonvalue> search for all bytes that have changed by the <comparisonvalue> since the initial search.\n"
		"\n"
		"Examples:\n"
		"\n"
		"cheatnextf increase\n"
		"  search for all bytes that have increased since the initial search.\n"
		"\n"
		"cheatnextf decrease, 1\n"
		"  search for all bytes that have decreased by 1 since the initial search.\n"
	},
	{
		"cheatlist",
		"\n"
		"  cheatlist [<filename>]\n"
		"\n"
		"Without <filename> show the list of matches in the debug console.\n"
		"With <filename> save the list of matches in basic xml format to <filename>.\n"
		"\n"
		"Examples:\n"
		"\n"
		"cheatlist\n"
		"  Show the current matches in the debug console.\n"
		"cheatlist cheat.txt\n"
		"  Save the current matches to cheat.txt.\n"
	},
	{
		"cheatundo",
		"\n"
		"  cheatundo\n"
		"\n"
		"Undo the results of the last search.\n"
		"The undo command has no effect on the last value.\n"
		"\n"
		"Examples:\n"
		"\n"
		"cheatundo\n"
		"  Undo the last search (state only).\n"
	},
	{
		"images",
		"\n"
		"  images\n"
		"\n"
		"Used to display list of available image devices.\n"
		"\n"
		"Examples:\n"
		"\n"
		"images\n"
		"  Show list of devices and mounted files for current driver.\n"
	},
	{
		"mount",
		"\n"
		"  mount <device>,<filename>\n"
		"\n"
		"Mount <filename> to image <device>.\n"
		"<filename> can be softlist item or full path to file.\n"
		"\n"
		"Examples:\n"
		"\n"
		"mount cart,aladdin\n"
		"  Mounts softlist item alladin on cart device.\n"
	},
	{
		"unmount",
		"\n"
		"  unmount <device>\n"
		"\n"
		"Unmounts file from image <device>.\n"
		"\n"
		"Examples:\n"
		"\n"
		"unmount cart\n"
		"  Unmounts any file mounted on device named cart.\n"
	}
};



/***************************************************************************
    CODE
***************************************************************************/

const char *debug_get_help(const char *tag)
{
	static char ambig_message[1024];
	const help_item *found = nullptr;
	int i, msglen, foundcount = 0;
	int taglen = (int)strlen(tag);
	char tagcopy[256];

	/* make a lowercase copy of the tag */
	for (i = 0; i <= taglen; i++)
		tagcopy[i] = tolower((UINT8)tag[i]);

	/* find a match */
	for (i = 0; i < ARRAY_LENGTH(static_help_list); i++)
		if (!strncmp(static_help_list[i].tag, tagcopy, taglen))
		{
			foundcount++;
			found = &static_help_list[i];
			if (strlen(found->tag) == taglen)
			{
				foundcount = 1;
				break;
			}
		}

	/* only a single match makes sense */
	if (foundcount == 1)
		return found->help;

	/* if not found, return the first entry */
	if (foundcount == 0)
		return static_help_list[0].help;

	/* otherwise, indicate ambiguous help */
	msglen = sprintf(ambig_message, "Ambiguous help request, did you mean:\n");
	for (i = 0; i < ARRAY_LENGTH(static_help_list); i++)
		if (!strncmp(static_help_list[i].tag, tagcopy, taglen))
			msglen += sprintf(&ambig_message[msglen], "  help %s?\n", static_help_list[i].tag);
	return ambig_message;
}
