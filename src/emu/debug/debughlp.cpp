// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    debughlp.cpp

    Debugger help engine.

*********************************************************************/

#include "emu.h"
#include "debughlp.h"

#include "corestr.h"

#include <cstdio>
#include <iterator>
#include <map>



namespace {

/***************************************************************************
    TABLE OF HELP
***************************************************************************/

struct help_item
{
	char const *tag;
	char const *help;
};

const help_item f_static_help_list[] =
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
		"  helpcustom -- get help on any custom commands registered by devices\n"
		"  do <expression> -- evaluates the given expression\n"
		"  symlist [<CPU>] -- lists registered symbols\n"
		"  softreset -- executes a soft reset\n"
		"  hardreset -- executes a hard reset\n"
		"  print <item>[,...] -- prints one or more <item>s to the console\n"
		"  printf <format>[,<item>[,...]] -- prints one or more <item>s to the console using <format>\n"
		"  logerror <format>[,<item>[,...]] -- outputs one or more <item>s to the error.log\n"
		"  tracelog <format>[,<item>[,...]] -- outputs one or more <item>s to the trace file using <format>\n"
		"  tracesym <item>[,...]] -- outputs one or more <item>s to the trace file\n"
		"  history [<CPU>,[<length>]] -- outputs a brief history of visited opcodes\n"
		"  trackpc [<bool>,[<CPU>,[<bool>]]] -- visually track visited opcodes [boolean to turn on and off, for CPU, clear]\n"
		"  trackmem [<bool>,[<CPU>,[<bool>]]] -- record which PC writes to each memory address [boolean to turn on and off, for CPU, clear]\n"
		"  pcatmem <address>[:<space>] -- query which PC wrote to a given memory address\n"
		"  pcatmemd <address>[:<space>] -- query which PC wrote to a given data memory address\n"
		"  pcatmemi <address>[:<space>] -- query which PC wrote to a given I/O memory address\n"
		"  pcatmemo <address>[:<space>] -- query which PC wrote to a given opcode memory address\n"
		"                                (Note: you can also query this info by right-clicking in a memory window)\n"
		"  rewind[rw] -- go back in time by loading the most recent rewind state"
		"  statesave[ss] <filename> -- save a state file for the current driver\n"
		"  stateload[sl] <filename> -- load a state file for the current driver\n"
		"  snap [<filename>] -- save a screen snapshot.\n"
		"  source <filename> -- reads commands from <filename> and executes them one by one\n"
		"  cls -- clears the console text buffer\n"
		"  quit -- exits MAME and the debugger\n"
	},
	{
		"memory",
		"\n"
		"Memory Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  dasm <filename>,<address>,<length>[,<opcodes>[,<CPU>]] -- disassemble to the given file\n"
		"  f[ind] <address>,<length>[,<data>[,...]] -- search memory for data\n"
		"  f[ind]d <address>,<length>[,<data>[,...]] -- search data memory for data\n"
		"  f[ind]i <address>,<length>[,<data>[,...]] -- search I/O memory for data\n"
		"  fill <address>,<length>[,<data>[,...]] -- fill memory with data\n"
		"  filld <address>[:<space>],<length>[,<data>[,...]] -- fill data memory with data\n"
		"  filli <address>[:<space>],<length>[,<data>[,...][ -- fill I/O memory with data\n"
		"  fillo <address>[:<space>],<length>[,<data>[,...][ -- fill opcode memory with data\n"
		"  dump <filename>,<address>[:<space>],<length>[,<group>[,<ascii>[,<rowsize>]]] -- dump memory as text\n"
		"  dumpd <filename>,<address>[:<space>],<length>[,<group>[,<ascii>[,<rowsize>]]] -- dump data memory as text\n"
		"  dumpi <filename>,<address>[:<space>],<length>[,<group>[,<ascii>[,<rowsize>]]] -- dump I/O memory as text\n"
		"  dumpo <filename>,<address>[:<space>],<length>[,<group>[,<ascii>[,<rowsize>]]] -- dump opcodes memory as text\n"
		"  strdump <filename>,<address>[:<space>],<length>[,<term>] -- dump ASCII strings from memory\n"
		"  strdumpd <filename>,<address>[:<space>],<length>[,<term>] -- dump ASCII strings from data memory\n"
		"  strdumpi <filename>,<address>[:<space>],<length>[,<term>] -- dump ASCII strings from I/O memory\n"
		"  strdumpo <filename>,<address>[:<space>],<length>[,<term>] -- dump ASCII strings from opcodes memory\n"
		"  save <filename>,<address>[:<space>],<length> -- save binary memory to the given file\n"
		"  saved <filename>,<address>[:<space>],<length> -- save binary data memory to the given file\n"
		"  savei <filename>,<address>[:<space>],<length> -- save binary I/O memory to the given file\n"
		"  saveo <filename>,<address>[:<space>],<length> -- save binary opcode memory to the given file\n"
		"  saver <filename>,<address>[:<space>],<length>,<region> -- save binary memory region to the given file\n"
		"  load <filename>,<address>[:<space>][,<length>] -- load binary memory from the given file\n"
		"  loadd <filename>,<address>[:<space>][,<length>] -- load binary data memory from the given file\n"
		"  loadi <filename>,<address>[:<space>][,<length>] -- load binary I/O memory from the given file\n"
		"  loado <filename>,<address>[:<space>][,<length>] -- load binary opcode memory from the given file\n"
		"  loadr <filename>,<address>[:<space>],<length>,<region> -- load binary memory region from the given file\n"
		"  map <address>[:<space>] -- map logical address to physical address and bank\n"
		"  mapd <address>[:<space>] -- map logical data address to physical address and bank\n"
		"  mapi <address>[:<space>] -- map logical I/O address to physical address and bank\n"
		"  mapo <address>[:<space>] -- map logical opcode address to physical address and bank\n"
		"  memdump [<filename>,[<root>]] -- dump current memory maps to <filename>\n"
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
		"  ge[x] [<exception>[,<condition>]] -- resumes execution, setting temp breakpoint if <exception> is raised\n"
		"  gi[nt] [<irqline>] -- resumes execution, setting temp breakpoint if <irqline> is taken (F7)\n"
		"  gt[ime] <milliseconds> -- resumes execution until the given delay has elapsed\n"
		"  gv[blank] -- resumes execution, setting temp breakpoint on the next VBLANK (F8)\n"
		"  n[ext] -- executes until the next CPU switch (F6)\n"
		"  focus <CPU> -- focuses debugger only on <CPU>\n"
		"  ignore [<CPU>[,<CPU>[,...]]] -- stops debugging on <CPU>\n"
		"  observe [<CPU>[,<CPU>[,...]]] -- resumes debugging on <CPU>\n"
		"  suspend [<CPU>[,<CPU>[,...]]] -- suspends execution on <CPU>\n"
		"  resume [<CPU>[,<CPU>[,...]]] -- resumes execution on <CPU>\n"
		"  cpulist -- list all CPUs\n"
		"  trace {<filename>|OFF}[,<CPU>[,<detectloops>[,<action>]]] -- trace the given CPU to a file (defaults to active CPU)\n"
		"  traceover {<filename>|OFF}[,<CPU>[,<detectloops>[,<action>]]] -- trace the given CPU to a file, but skip subroutines (defaults to active CPU)\n"
		"  traceflush -- flushes all open trace files\n"
	},
	{
		"breakpoints",
		"\n"
		"Breakpoint Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  bp[set] <address>[:<CPU>][,<condition>[,<action>]] -- sets breakpoint at <address>\n"
		"  bpclear [<bpnum>] -- clears a given breakpoint or all if no <bpnum> specified\n"
		"  bpdisable [<bpnum>] -- disables a given breakpoint or all if no <bpnum> specified\n"
		"  bpenable [<bpnum>] -- enables a given breakpoint or all if no <bpnum> specified\n"
		"  bplist [<CPU>] -- lists all the breakpoints\n"
	},
	{
		"watchpoints",
		"\n"
		"Watchpoint Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  wp[set] <address>[:<space>],<length>,<type>[,<condition>[,<action>]] -- sets watchpoint\n"
		"  wpd[set] <address>[:<space>],<length>,<type>[,<condition>[,<action>]] -- sets data space watchpoint\n"
		"  wpi[set] <address>[:<space>],<length>,<type>[,<condition>[,<action>]] -- sets I/O space watchpoint\n"
		"  wpo[set] <address>[:<space>],<length>,<type>[,<condition>[,<action>]] -- sets opcode space watchpoint\n"
		"  wpclear [<wpnum>] -- clears a given watchpoint or all if no <wpnum> specified\n"
		"  wpdisable [<wpnum>] -- disables a given watchpoint or all if no <wpnum> specified\n"
		"  wpenable [<wpnum>] -- enables a given watchpoint or all if no <wpnum> specified\n"
		"  wplist [<CPU>] -- lists all the watchpoints\n"
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
		"These are the differences from C behaviors:\n"
		"\n"
		"First, all math is performed on full 64-bit unsigned values, so things like a < 0 won't work "
		"as expected.\n"
		"Second, the logical operators && and || do not have short-circuit properties -- both halves are "
		"always evaluated.\n"
		"Finally, the new memory operators work like this:\n"
		"b@<addr> refers to the byte at <addr> while suppressing side effects.\n"
		"Similarly, w@ and w! refer to a word in memory, d@ and d! refer to a dword in memory, and "
		"q@ and q! refer to a qword in memory.\n"
		"The memory operators can be used as both lvalues and rvalues, so you can write b@100 = ff to "
		"store a byte in memory. By default these operators read from the program memory space, but you "
		"can override that by prefixing them with a 'd' or an 'i'.\n"
		"As such, dw@300 refers to data memory word at address 300 and id@400 refers to an I/O memory "
		"dword at address 400.\n"

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
		"  comlist -- print currently available comments from file\n"
		"  commit[/*] <address>,<comment> -- gives a bulk comadd then comsave command\n"
		"\n"
	},
	{
		"cheats",
		"\n"
		"Cheat Commands\n"
		"Type help <command> for further details on each command\n"
		"\n"
		"  cheatinit [<sign><width>[<swap>],[<address>,<length>[,<CPU>]]] -- initialize the cheat search to the selected memory area\n"
		"  cheatrange <address>,<length> -- add to the cheat search the selected memory area\n"
		"  cheatnext <condition>[,<comparisonvalue>] -- continue cheat search comparing with the previous value\n"
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
		"  images -- lists all image devices and mounted mounted media\n"
		"  mount <instance>,<filename> -- mounts file to specified device\n"
		"  unmount <instance> -- unmounts media from specified device\n"
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
		"  symlist [<CPU>]\n"
		"\n"
		"Lists registered symbols. If <CPU> is not specified, then symbols in the global symbol table are "
		"displayed; otherwise, the symbols for <CPU>'s specific CPU are displayed. Symbols are listed "
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
		"tracelog \"A=%d, B=%d\\nC=%d\",a,b,a+b\n"
		"  Outputs A=<aval>, B=<bval> on one line, and C=<a+bval> on a second line.\n"
	},
	{
		"tracesym",
		"\n"
		"  tracesym <item>[,...]\n"
		"\n"
		"The tracesym command prints the specified symbols and routes the output to the currently open trace "
		"file (see the 'trace' command for details). If no file is currently open, tracesym does nothing. "
		"\n"
		"Examples:\n"
		"\n"
		"tracesym pc\n"
		"  Outputs PC=<pcval> where <pcval> is displayed in the default format.\n"
	},
	{
		"history",
		"\n"
		"  history [<CPU>,[<length>]]\n"
		"\n"
		"The history command displays recently visited PC addresses, and the disassembly of the "
		"instructions at those addresses.  If present, the first argument is a CPU selector "
		"(either a tag or a CPU number); if no CPU is specified, the current CPU is assumed.  "
		"The second argument, if present, limits the maximum number of addresses shown.  "
		"Addresses are shown in order from least to most recently visited.\n"
		"\n"
		"Examples:\n"
		"\n"
		"history ,5\n"
		"  Displays up to five most recently visited PC addresses for the current CPU.\n"
		"\n"
		"history 3\n"
		"  Displays recently visited PC addresses for CPU 3.\n"
		"\n"
		"history audiocpu,1\n"
		"  Displays the most recently visited PC addresses for the CPU ':audiocpu'.\n"
	},
	{
		"trackpc",
		"\n"
		"  trackpc [<bool>,[<CPU>,[<bool>]]]\n"
		"\n"
		"The trackpc command displays which program counters have already been visited in all "
		"disassembler views.  The first Boolean argument toggles the process on and off.  The "
		"second argument is a CPU selector (either a tag or a debugger CPU number); if no CPU is "
		"specified, the current CPU is assumed.  The third argument is a Boolean indicating "
		"whether the existing data should be cleared.\n"
		"\n"
		"Examples:\n"
		"\n"
		"trackpc 1\n"
		"  Begin tracking the current CPU's pc.\n"
		"\n"
		"trackpc 1, 0, 1\n"
		"  Continue tracking pc on CPU 0, but clear existing track info.\n"
	},
	{
		"trackmem",
		"\n"
		"  trackmem [<bool>,[<CPU>,[<bool>]]]\n"
		"\n"
		"The trackmem command logs the PC at each time a memory address is written to.  "
		"The first Boolean argument toggles the process on and off.  The second argument is a CPU "
		"selector (either a tag or a debugger CPU number); if no CPU is specified, the current CPU "
		"is assumed.  The third argument is a Boolean indicating whether the existing data should "
		"be cleared.  Please refer to the 'pcatmem' command for information on how to retrieve this "
		"data.  Also, right-clicking in a memory view will display the logged PC for the given "
		"address.\n"
		"\n"
		"Examples:\n"
		"\n"
		"trackmem\n"
		"  Begin tracking memory writes for the current CPU.\n"
		"\n"
		"trackmem 1, 0, 1\n"
		"  Continue tracking memory writes on CPU 0, but clear existing tracking data.\n"
	},
	{
		"pcatmem",
		"\n"
		"  pcatmem[{d|i|o}] <address>[:<space>]\n"
		"\n"
		"The pcatmem command returns which PC value at the time the specified address was most "
		"recently written.  The argument is the requested address, optionally followed by a colon "
		"and a CPU and/or address space.  The CPU may be specified as a tag or debugger CPU number; "
		"if no CPU is specified, the CPU currently visible in the debugger is assumed.  If an "
		"address space is not specified, the command suffix sets the address space: 'pcatmem' "
		"defaults to the first space exposed by the device, 'pcatmemd' defaults to the data space, "
		"'pcatmemi' defaults to the I/O space, and 'pcatmemo' defaults to the opcodes space.\n"
		"\n"
		"Right-clicking in a memory view will also display the logged PC for the given address.\n"
		"\n"
		"Examples:\n"
		"\n"
		"pcatmem 400000\n"
		"  Print which PC wrote to this CPU's program space at location 0x400000.\n"
		"\n"
		"pcatmem 3bc:io\n"
		"  Print which PC wrote this CPU's memory io space at location 0x3bc.\n"
		"\n"
		"pcatmem 1400:audiocpu\n"
		"  Print which PC wrote the CPU :audiocpu's memory program space at location 0x1400.\n"
	},
	{ "pcatmemd", "#pcatmem" },
	{ "pcatmemi", "#pcatmem" },
	{ "pcatmemo", "#pcatmem" },
	{
		"rewind[rw]",
		"\n"
		"  rewind[rw]"
		"\n"
		"The rewind command loads the most recent RAM-based state.  Rewind states, when enabled, are "
		"saved when \"step\", \"over\", or \"out\" command gets executed, storing the machine state as "
		"of the moment before actually stepping.  Consecutively loading rewind states can work like "
		"reverse execution.  Depending on which steps forward were taken previously, the behavior can "
		"be similar to GDB's \"reverse-stepi\" or \"reverse-next\".  All output for this command is "
		"currently echoed into the running machine window.  Previous memory and PC tracking statistics "
		"are cleared, actual reverse execution does not occur.\n"
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
		"  stateload[sl] <filename>\n"
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
		"  snap [<filename>[,<scrnum>]]\n"
		"\n"
		"Takes a snapshot of the emulated video display and saves it to the configured snapshot "
		"directory.  If a filename is specified, a single screenshot for the specified screen is "
		"saved using the specified filename (or the first emulated screen in the system if a screen "
		"is not specified).  If a file name is not specified, the configured snapshot view and file "
		"name pattern are used.\n"
		"\n"
		"If a file name is specified, the .png extension is automatically appended.  The screen "
		"number is specified as a zero-based index.\n"
		"\n"
		"Examples:\n"
		"\n"
		"snap\n"
		"  Takes a snapshot using the configured snapshot view and file name options.\n"
		"\n"
		"snap shinobi\n"
		"  Takes a snapshot of the first emulated video screen and saves it as 'shinobi.png' in the "
		"  configured snapshot directory.\n"
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
		"  dasm <filename>,<address>,<length>[,<opcodes>[,<CPU>]]\n"
		"\n"
		"The dasm command disassembles program memory to the file specified in the <filename> parameter. "
		"<address> indicates the address of the start of disassembly, and <length> indicates how much "
		"memory to disassemble. The range <address> through <address>+<length>-1 inclusive will be "
		"output to the file. By default, the raw opcode data is output with each line. The optional "
		"<opcodes> parameter can be used to enable (1) or disable(0) this feature. Finally, you can "
		"disassemble code from another CPU by specifying the <CPU> parameter.\n"
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
		"  f[ind][{d|i|o}] <address>[:<space>],<length>[,<data>[,...]]\n"
		"\n"
		"The find commands search through memory for the specified sequence of data.  The <address> "
		"is the address to begin searching from, optionally followed by a device and/or address "
		"space; the <length> specifies how much memory to search.  The device may be specified as a "
		"tag or a debugger CPU number; if no device is specified, the CPU currently visible in the "
		"debugger is assumed.  If an address space is not specified, the command suffix sets the "
		"address space: 'find' defaults to the first address space exposed by the device, 'findd' "
		"defaults to the data space, 'findi' defaults to the I/O space, and 'findo' defaults to the "
		"opcodes space.\n"
		"\n"
		"The <data> can either be a quoted string or a numeric value or expression or the wildcard "
		"character '?'.  By default, strings imply a byte-sized search; by default non-string data "
		"is searched using the native word size of the address space. To override the search size "
		"for non-string data, you can prefix values with b. to force byte-sized search, w. for "
		"word-sized search, d. for dword-sized search, and q. for qword-sized search.  Overrides "
		"propagate to subsequent values, so if you want to search for a sequence of words, you need "
		"only prefix the first value with a w.  Also note that you can intermix sizes to perform "
		"more complex searches.  The entire range <address> through <address>+<length>-1, "
		"inclusive, will be searched for the sequence, and all occurrences will be displayed.\n"
		"\n"
		"Examples:\n"
		"\n"
		"find 0,10000,\"HIGH SCORE\",0\n"
		"  Searches the address range 0-ffff in the current CPU for the string \"HIGH SCORE\" "
		"followed by a 0 byte.\n"
		"\n"
		"find 300:tms9918a,100,w.abcd,4567\n"
		"  Searches the address range 300-3ff in the first address space exposed by the device "
		"':tms9918a' for the word-sized value abcd followed by the word-sized value 4567.\n"
		"\n"
		"find 0,8000,\"AAR\",d.0,\"BEN\",w.0\n"
		"  Searches the address range 0000-7fff for the string \"AAR\" followed by a dword-sized 0 "
		"followed by the string \"BEN\", followed by a word-sized 0.\n"
	},
	{ "findd", "#find" },
	{ "findi", "#find" },
	{ "findo", "#find" },
	{
		"fill",
		"\n"
		"  fill[{d|i|o}] <address>[:<space>],<length>[,<data>[,...]]\n"
		"\n"
		"The fill commands overwrite a block of memory with copies of the supplied data sequence.  "
		"The <address> specifies the address to begin writing at, optionally followed by a device "
		"and/or address space; the <length> specifies how much memory to fill.  The device may be "
		"specified as a tag or a debugger CPU number; if no device is specified, the CPU currently "
		"visible in the debugger is assumed.  If an address space is not specified, the command "
		"suffix sets the address space: 'fill' defaults to the first address space exposed by the "
		"device, 'filld' defaults to the data space, 'filli' defaults to the I/O space, and 'fillo' "
		"defaults to the opcodes space.\n"
		"\n"
		"The <data> can either be a quoted string or a numeric value or expression.  By default, "
		"non-string data is written using the native word size of the address space. To override "
		"the data size for non-string data, you can prefix values with b. to force byte-sized fill, "
		"w. for word-sized fill, d. for dword-sized fill, and q. for qword-sized fill. Overrides "
		"propagate to subsequent values, so if you want to fill with a series of words, you need "
		"only prefix the first value with a w.  Also note that you can intermix sizes to perform "
		"more complex fills. The fill operation may be truncated if a page fault occurs or if part "
		"of the sequence or string would fall beyond <address>+<length>-1.\n"
	},
	{ "filld", "#fill" },
	{ "filli", "#fill" },
	{ "fillo", "#fill" },
	{
		"dump",
		"\n"
		"  dump[{d|i|o}] <filename>,<address>[:<space>],<length>[,<group>[,<ascii>[,<rowsize>]]]\n"
		"\n"
		"The dump commands dump memory to the text file specified in the <filename> parameter.  The "
		"<address> specifies the address to start dumping from, optionally followed by a device "
		"and/or address space; the <length> specifies how much memory to dump.  The device may be "
		"specified as a tag or a debugger CPU number; if no device is specified, the CPU currently "
		"visible in the debugger is assumed.  If an address space is not specified, the command "
		"suffix sets the address space: 'dump' defaults to the first address space exposed by the "
		"device, 'dumpd' defaults to the data space, 'dumpi' defaults to the I/O space, and 'dumpo' "
		"defaults to the opcodes space.\n"
		"\n"
		"The range <address> through <address>+<length>-1, inclusive, will be output to the file.  "
		"By default, the data will be output using the native word size of the address space.  You "
		"can override this by specifying the <group> parameter, which can be used to group the data "
		"in 1-, 2-, 4- or 8-byte chunks.  The optional <ascii> parameter is a Boolean value used to "
		"enable or disable output of ASCII characters on the right of each line (enabled by "
		"default).  The optional <rowsize> parameter specifies the amount of data on each line in "
		"address units (defaults to 16 bytes).\n"
		"\n"
		"Examples:\n"
		"\n"
		"dump venture.dmp,0,10000\n"
		"  Dumps addresses 0-ffff in the current CPU in 1-byte chunks, including ASCII data, to "
		"the file 'venture.dmp'.\n"
		"\n"
		"dumpd harddriv.dmp,3000:3,1000,4,0\n"
		"  Dumps data memory addresses 3000-3fff from CPU #3 in 4-byte chunks, with no ASCII data, "
		"to the file 'harddriv.dmp'.\n"
		"\n"
		"dump vram.dmp,0:sms_vdp:videoram,4000,1,0,8\n"
		"  Dumps 'videoram' space addresses 0000-3fff from the device ':sms_vdp' in 1-byte chunks, "
		"with no ASCII data, and 8 bytes per line, to the file 'vram.dmp'.\n"
	},
	{ "dumpd", "#dump" },
	{ "dumpi", "#dump" },
	{ "dumpo", "#dump" },
	{
		"strdump",
		"\n"
		"  strdump[{d|i|o}] <filename>,<address>[:<space>],<length>[,<term>]\n"
		"\n"
		"The strdump commands dump memory to the text file specified in the <filename> parameter.  "
		"The <address> specifies the address to start dumping from, optionally followed by a device "
		"and/or address space; the <length> specifies how much memory to dump.  The device may be "
		"specified as a tag or a debugger CPU number; if no device is specified, the CPU currently "
		"visible in the debugger is assumed.  If an address space is not specified, the command "
		"suffix sets the address space: 'strdump' defaults to the first address space exposed by "
		"the device, 'strdumpd' defaults to the data space, 'strdumpi' defaults to the I/O space, "
		"and 'strdumpo' defaults to the opcodes space.\n"
		"\n"
		"By default, the data will be interpreted as a series of NUL-terminated strings, the dump "
		"will have one string per line, and C-style escapes will be used for non-ASCII characters. "
		"The optional <term> parameter can be used to specify a different string terminator "
		"character.\n"
	},
	{ "strdumpd", "#strdump" },
	{ "strdumpi", "#strdump" },
	{ "strdumpo", "#strdump" },
	{
		"save",
		"\n"
		"  save[{d|i|o}] <filename>,<address>[:<space>],<length>\n"
		"\n"
		"The save commands save raw memory to the binary file specified in the <filename> "
		"parameter.  The <address> specifies the address to start saving from, optionally followed "
		"by a device and/or address space; the <length> specifies how much memory to save.  The "
		"device may be specified as a tag or a debugger CPU number; if no device is specified, the "
		"CPU currently visible in the debugger is assumed.  If an address space is not specified, "
		"the command suffix sets the address space: 'save' defaults to the first address space "
		"exposed by the device, 'saved' defaults to the data space, 'savei' defaults to the I/O "
		"space, and 'saveo' defaults to the opcodes space.\n"
		"\n"
		"The range <address> through <address>+<length>-1, inclusive, will be output to the file.\n"
		"\n"
		"Examples:\n"
		"\n"
		"save venture.bin,0,10000\n"
		"  Saves addresses 0-ffff in the current CPU to the binary file 'venture.bin'.\n"
		"\n"
		"saved harddriv.bin,3000:3,1000\n"
		"  Saves data memory addresses 3000-3fff from CPU #3 to the binary file 'harddriv.bin'.\n"
		"\n"
		"save vram.bin,0:sms_vdp:videoram,4000\n"
		"  Saves 'videoram' space addresses 0000-3fff from the device ':sms_vdp' to the binary file "
		"'vram.bin'.\n"
	},
	{ "saved", "#save" },
	{ "savei", "#save" },
	{ "saveo", "#save" },
	{
		"saver",
		"\n"
		"  saver <filename>,<address>,<length>,<region>\n"
		"\n"
		"The saver command saves the raw content of memory region <region> to the binary file "
		"specified in the <filename> parameter.  The <address> specifies the address to start "
		"saving from, and the <length> specifies how much memory to save.  The range <address> "
		"through <address>+<length>-1, inclusive, will be output to the file.\n"
		"\n"
		"Examples:\n"
		"\n"
		"saver data.bin,200,100,:monitor\n"
		"  Saves ':monitor' region addresses 200-2ff to the binary file 'data.bin'.\n"
		"\n"
		"saver cpurom.bin,1000,400,.\n"
		"  Saves addresses 1000-13ff from the memory region with the same tag as the visible CPU to "
		"the binary file 'cpurom.bin'.\n"
	},
	{
		"load",
		"\n"
		"  load[{d|i|o}] <filename>,<address>[:<space>][,<length>]\n"
		"\n"
		"The load commands load raw memory from the binary file specified in the <filename> "
		"parameter.  The <address> specifies the address to start loading to, optionally followed "
		"by a device and/or address space; the <length> specifies how much memory to load.  The "
		"device may be specified as a tag or a debugger CPU number; if no device is specified, the "
		"CPU currently visible in the debugger is assumed.  If an address space is not specified, "
		"the command suffix sets the address space: 'load' defaults to the first address space "
		"exposed by the device, 'loadd' defaults to the data space, 'loadi' defaults to the I/O "
		"space, and 'loado' defaults to the opcodes space.\n"
		"\n"
		"The range <address> through <address>+<length>-1, inclusive, will be read in from the "
		"file.  If the <length> is omitted, if it is zero, or if it is greater than the total "
		"length of the file, the entire contents of the file will be loaded but no more.\n"
		"\n"
		"NOTE: this has the same effect as writing to the address space from a debugger memory "
		"view, or using memory accessors in debugger expressions.  Read-only memory will not be "
		"overwritten, and writing to I/O addresses may have effects beyond setting register "
		"values.\n"
		"\n"
		"Examples:\n"
		"\n"
		"load venture.bin,0,10000\n"
		"  Loads addresses 0-ffff in the current CPU from the binary file 'venture.bin'.\n"
		"\n"
		"loadd harddriv.bin,3000,1000,3\n"
		"  Loads data memory addresses 3000-3fff from CPU #3 from the binary file 'harddriv.bin'.\n"
		"\n"
		"load vram.bin,0:sms_vdp:videoram\n"
		"  Loads 'videoram' space starting at address 0000 from the device ':sms_vdp' with the "
		"entire content of the binary file 'vram.bin'.\n"
	},
	{ "loadd", "#load" },
	{ "loadi", "#load" },
	{ "loado", "#load" },
	{
		"loadr",
		"\n"
		"  loadr <filename>,<address>,<length>,<region>\n"
		"\n"
		"The loadr command loads raw memory in the memory region <region> from the binary file "
		"specified by the <filename> parameter.  The <address> specifies the address to start "
		"loading to, and the <length> specifies how much memory to load.  The range <address> "
		"through <address>+<length>-1, inclusive, will be read in from the file.  If the <length> "
		"is zero, or is greater than the total length of the file, the entire contents of the file "
		"will be loaded but no more.\n"
		"\n"
		"Examples:\n"
		"\n"
		"loadr data.bin,200,100,:monitor\n"
		"  Loads ':monitor' region addresses 200-2ff from the binary file 'data.bin'.\n"
		"\n"
		"loadr cpurom.bin,1000,400,.\n"
		"  Loads addresses 1000-13ff in the memory region with the same tag as the visible CPU from "
		"the binary file 'cpurom.bin'.\n"
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
		"Example:\n"
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
		"gex",
		"\n"
		"  ge[x] [<exception>,[<condition>]]\n"
		"\n"
		"The gex command resumes execution of the current code.  Control will not be returned to "
		"the debugger until a breakpoint or watchpoint is hit, or until an exception condition "
		"is raised on the current CPU.  You can specify <exception> if you wish to stop execution "
		"only on a particular exception condition occurring.  If <exception> is omitted, then any "
		"exception condition will stop execution.  The optional <condition> parameter lets you "
		"specify an expression that will be evaluated each time the specified exception condition "
		"is raised.  If the result of the expression is true (non-zero), the exception will halt "
		"execution; otherwise, execution will continue with no notification.\n"
		"\n"
		"Examples:\n"
		"\n"
		"gex\n"
		"  Resume execution until the next break/watchpoint or until any exception condition is "
		"raised on the current CPU.\n"
		"\n"
		"ge 2\n"
		"  Resume execution until the next break/watchpoint or until exception condition 2 is "
		"raised on the current CPU.\n"
	},
	{
		"gint",
		"\n"
		"  gi[nt] [<irqline>]\n"
		"\n"
		"The gint command resumes execution of the current code.  Control will not be returned to "
		"the debugger until a breakpoint or watchpoint is hit, or until an IRQ is asserted and "
		"acknowledged on the current CPU.  You can specify <irqline> if you wish to stop execution "
		"only on a particular IRQ line being asserted and acknowledged.  If <irqline> is omitted, "
		"then any IRQ line will stop execution.\n"
		"\n"
		"Examples:\n"
		"\n"
		"gi\n"
		"  Resume execution until the next break/watchpoint or until any IRQ is asserted and "
		"acknowledged on the current CPU.\n"
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
		"gvblank",
		"\n"
		"  gv[blank]\n"
		"\n"
		"The gvblank command resumes execution of the current code. Control will not be returned to "
		"the debugger until a breakpoint or watchpoint is hit, or until the beginning of the "
		" vertical blanking interval for an emulated screen.\n"
		"\n"
		"Example:\n"
		"\n"
		"gv\n"
		"  Resume execution until the next break/watchpoint or until the next VBLANK.\n"
	},
	{
		"next",
		"\n"
		"  n[ext]\n"
		"\n"
		"The next command resumes execution and continues executing until the next time a different "
		"CPU is scheduled. Note that if you have used 'focus' or 'ignore' to ignore certain CPUs, "
		"execution will continue until a non-'ignore'd CPU is scheduled.\n"
		"\n"
		"Example:\n"
		"\n"
		"n\n"
		"  Resume execution, stopping when a different CPU that is not ignored is scheduled.\n"
	},
	{
		"focus",
		"\n"
		"  focus <CPU>\n"
		"\n"
		"Sets the debugger focus exclusively to the given <CPU>. This is equivalent to specifying "
		"'ignore' on all other CPUs.\n"
		"\n"
		"Examples:\n"
		"\n"
		"focus 1\n"
		"  Focus exclusively CPU #1 while ignoring all other CPUs when using the debugger.\n"
		"\n"
		"focus audiopcb:melodycpu\n"
		"  Focus exclusively on the CPU ':audiopcb:melodycpu'.\n"
	},
	{
		"ignore",
		"\n"
		"  ignore [<CPU>[,<CPU>[,...]]]\n"
		"\n"
		"Ignores the specified CPUs in the debugger.  CPUs can be specified by tag or debugger CPU "
		"number.  The debugger never shows execution for ignored CPUs, and breakpoints or "
		"watchpoints on ignored CPUs have no effect.  If no CPUs are specified, currently ignored "
		"CPUs will be listed.  Use the 'observe' command to stop ignoring a CPU.  Note that you "
		"cannot ignore all CPUs; at least CPU must be observed at all times.\n"
		"\n"
		"Examples:\n"
		"\n"
		"ignore audiocpu\n"
		"  Ignore the CPU ':audiocpu' when using the debugger.\n"
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
		"  observe [<CPU>[,<CPU>[,...]]]\n"
		"\n"
		"Re-enables interaction with the specified <CPU> in the debugger. This command undoes the "
		"effects of the 'ignore' command. You can specify multiple <CPU>s in a single command.\n"
		"\n"
		"Examples:\n"
		"\n"
		"observe audiocpu\n"
		"  Stop ignoring the CPU ':audiocpu' when using the debugger.\n"
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
		"  trace {<filename>|off}[,<CPU>[,[noloop|logerror][,<action>]]]\n"
		"\n"
		"Starts or stops tracing of the execution of the specified <CPU>, or the currently visible "
		"CPU if no CPU is specified.  To enable tracing, specify the trace log file name in the "
		"<filename> parameter.  To disable tracing, use the keyword 'off' for <filename> "
		"parameter.  If the **<filename>** begins with two right angle brackets (>>), it is treated "
		"as a directive to open the file for appending rather than overwriting.\n"
		"\n"
		"The optional third parameter is a flags field.  The supported flags are 'noloop' and "
		"'logerror'.  Multiple flags must be separated by | (pipe) characters.  By default, loops "
		"are detected and condensed to a single line.  If the 'noloop' flag is specified, loops "
		"will not be detected and every instruction will be logged as executed.  If the 'logerror' "
		"flag is specified, error log output will be included in the trace log.\n"
		"\n"
		"The optional <action> parameter is a debugger command to execute before each trace message "
		"is logged.  Generally, this will include a 'tracelog' or 'tracesym' command to include "
		"additional information in the trace log.  Note that you may need to embed the action "
		"within braces { } in order to prevent commas and semicolons from being interpreted as "
		"applying to the trace command itself.\n"
		"\n"
		"Examples:\n"
		"\n"
		"trace joust.tr\n"
		"  Begin tracing the execution of the currently visible CPU, logging output to joust.tr.\n"
		"\n"
		"trace dribling.tr,maincpu\n"
		"  Begin tracing the execution of the CPU ':maincpu', logging output to dribling.tr.\n"
		"\n"
		"trace starswep.tr,,noloop\n"
		"  Begin tracing the execution of the currently visible CPU, logging output to starswep.tr, "
		"with loop detection disabled.\n"
		"\n"
		"trace starswep.tr,1,logerror\n"
		"  Begin tracing the execution of CPU #1, logging output (along with logerror output) to "
		"starswep.tr.\n"
		"\n"
		"trace starswep.tr,0,logerror|noloop\n"
		"  Begin tracing the execution of CPU #0, logging output (along with logerror output) to "
		"starswep.tr, with loop detection disabled.\n"
		"\n"
		"trace >>pigskin.tr\n"
		"  Begin tracing execution of the currently visible CPU, appending log output to "
		"pigskin.tr.\n"
		"\n"
		"trace off,0\n"
		"  Turn off tracing on CPU #0.\n"
		"\n"
		"trace asteroid.tr,,,{tracelog \"A=%02X \",a}\n"
		"  Begin tracing the execution of the currently visible CPU, logging output to asteroid.tr. "
		"Before each line, output A=<aval> to the trace log.\n"
	},
	{
		"traceover",
		"\n"
		"  traceover {<filename>|off}[,<CPU>[,[noloop|logerror][,<action>]]]\n"
		"\n"
		"Starts or stops tracing for execution of the specified **<CPU>**, or the currently visible "
		"CPU if no CPU is specified.  When a subroutine call is encountered, tracing will skip over "
		"the subroutine.  The same algorithm is used as is used in the step over command.  It will "
		"not work properly with recursive functions, or if the return address does not immediately "
		"follow the call instruction.\n"
		"\n"
		"This command accepts the same parameters as the 'trace' command.  Please refer to the "
		"corresponding section for a detailed description of options and more examples.\n"
		"\n"
		"Examples:\n"
		"\n"
		"traceover joust.tr\n"
		"  Begin tracing the execution of the currently visible CPU, logging output to joust.tr.\n"
		"\n"
		"traceover dribling.tr,maincpu\n"
		"  Begin tracing the execution of the CPU ':maincpu', logging output to dribling.tr.\n"
		"\n"
		"traceover starswep.tr,,noloop\n"
		"  Begin tracing the execution of the currently visible CPU, logging output to starswep.tr, "
		"with loop detection disabled.\n"
		"\n"
		"traceover off,0\n"
		"  Turn off tracing on CPU #0.\n"
		"\n"
		"traceover asteroid.tr,,,{tracelog \"A=%02X \",a}\n"
		"  Begin tracing the execution of the currently visible CPU, logging output to "
		"asteroid.tr.  Before each line, output A=<aval> to the trace log.\n"
	},
	{
		"traceflush",
		"\n"
		"  traceflush\n"
		"\n"
		"Flushes all open trace log files to disk.\n"
		"\n"
		"Example:\n"
		"\n"
		"traceflush\n"
		"  Flush trace log files.\n"
	},
	{
		"bpset",
		"\n"
		"  bp[set] <address>[:<CPU>][,<condition>[,<action>]]\n"
		"\n"
		"Sets a new execution breakpoint at the specified <address>.  The <address> may optionally "
		"be followed by a colon and a tag or debugger CPU number to specify a CPU explicitly.  If "
		"no CPU is specified, the CPU currently visible in the debugger is assumed.  The optional "
		"<condition> parameter lets you specify an expression that will be evaluated each time the "
		"breakpoint is hit.  If the result of the expression is true (non-zero), the breakpoint "
		"will halt execution; otherwise, execution will continue with no notification.  The "
		"optional <action> parameter provides a command that is executed whenever the breakpoint is "
		"hit and the <condition> is true.  Note that you may need to embed the action within braces "
		"{ } in order to prevent commas and semicolons from being interpreted as applying to the "
		"'bpset' command itself.\n"
		"\n"
		"Each breakpoint that is set is assigned an index which can be used to refer to it in other "
		"breakpoint commands.\n"
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
		"bp 3456:audiocpu,1,{ printf \"A0=%08X\\n\",a0 ; g }\n"
		"  Set a breakpoint on the CPU ':audiocpu' that will halt execution whenever the PC is "
		"equal to 3456.  When this happens, print A0=<a0val> and continue executing.\n"
		"\n"
		"bp 45678:2,a0==100,{ a0 = ff ; g }\n"
		"  Set a breakpoint on CPU #2 that will halt execution whenever the PC is equal to 45678 "
		"and the expression (a0 == 100) is true.  When that happens, set a0 to ff and resume "
		"execution.\n"
		"\n"
		"temp0 = 0 ; bp 567890,++temp0 >= 10\n"
		"  Set a breakpoint that will halt execution whenever the PC is equal to 567890 and the "
		"expression (++temp0 >= 10) is true.  This effectively breaks only after the breakpoint "
		"has been hit 16 times.\n"
	},
	{
		"bpclear",
		"\n"
		"  bpclear [<bpnum>]\n"
		"\n"
		"The bpclear command clears a breakpoint.  If <bpnum> is specified, only the requested "
		"breakpoint is cleared; otherwise all breakpoints are cleared.\n"
		"\n"
		"Examples:\n"
		"\n"
		"bpclear 3\n"
		"  Clear the breakpoint with index 3.\n"
		"\n"
		"bpclear\n"
		"  Clear all breakpoints.\n"
	},
	{
		"bpdisable",
		"\n"
		"  bpdisable [<bpnum>]\n"
		"\n"
		"The bpdisable command disables a breakpoint.  If <bpnum> is specified, only the requested "
		"breakpoint is disabled; otherwise all breakpoints are disabled.  Note that disabling a "
		"breakpoint does not delete it, it just temporarily marks the breakpoint as inactive.\n"
		"\n"
		"Examples:\n"
		"\n"
		"bpdisable 3\n"
		"  Disable the breakpoint with index 3.\n"
		"\n"
		"bpdisable\n"
		"  Disable all breakpoints.\n"
	},
	{
		"bpenable",
		"\n"
		"  bpenable [<bpnum>]\n"
		"\n"
		"The bpenable command enables a breakpoint.  If <bpnum> is specified, only the requested "
		"breakpoint is enabled; otherwise all breakpoints are enabled.\n"
		"\n"
		"Examples:\n"
		"\n"
		"bpenable 3\n"
		"  Enable the breakpoint with index 3.\n"
		"\n"
		"bpenable\n"
		"  Enable all breakpoints.\n"
	},
	{
		"bplist",
		"\n"
		"  bplist [<CPU>]\n"
		"\n"
		"The bplist list current breakpoints, along with their indices and any associated "
		"conditions or actions.  If no <CPU> is specified, breakpoints for all CPUs in the system "
		"will be listed; if a <CPU> is specified, only breakpoints for that CPU will be listed.  "
		"The <CPU> can be specified by tag or by debugger CPU number.\n"
		"\n"
		"Examples:\n"
		"\n"
		"bplist\n"
		"  List all breakpoints.\n"
		"\n"
		"bplist .\n"
		"  List all breakpoints for the visible CPU.\n"
		"\n"
		"bplist maincpu\n"
		"  List all breakpoints for the CPU ':maincpu'.\n"
	},
	{
		"wpset",
		"\n"
		"  wp[{d|i|o}][set] <address>[:<space>],<length>,<type>[,<condition>[,<action>]]\n"
		"\n"
		"Sets a new watchpoint starting at the specified <address> and extending for <length>.  The "
		"inclusive range of the watchpoint is <address> through <address>+<length>-1.  The "
		"<address> may optionally be followed by a CPU and/or address space.  The CPU may be "
		"specified as a tag or a debugger CPU number; if no CPU is specified, the CPU currently "
		"visible in the debugger is assumed.  If an address space is not specified, the command "
		"suffix sets the address space: 'wpset' defaults to the first address space exposed by the "
		"CPU, 'wpdset' defaults to the data space, 'wpiset' defaults to the I/O space, and 'wposet' "
		"defaults to the opcodes space.  The <type> parameter specifies the access types to trap "
		"on - it can be one of three values: 'r' for read accesses, 'w' for write accesses, or 'rw' "
		"for both read and write accesses.\n"
		"\n"
		"The optional <condition> parameter lets you specify an expression that will be evaluated "
		"each time the watchpoint is triggered.  If the result of the expression is true "
		"(non-zero), the watchpoint will halt execution; otherwise, execution will continue with no "
		"notification.  The optional <action> parameter provides a command that is executed "
		"whenever the watchpoint is triggered and the <condition> is true. Note that you may need "
		"to embed the action within braces { } in order to prevent commas and semicolons from being "
		"interpreted as applying to the wpset command itself.\n"
		"\n"
		"Each watchpoint that is set is assigned an index which can be used to refer to it in other "
		"watchpoint commands\n"
		"\n"
		"To make <condition> expressions more useful, two variables are available: for all "
		"watchpoints, the variable 'wpaddr' is set to the access address that triggered the "
		"watchpoint; for write watchpoints, the variable 'wpdata' is set to the data being "
		"written.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wp 1234,6,rw\n"
		"  Set a watchpoint that will halt execution whenever a read or write occurs in the address "
		"range 1234-1239 inclusive.\n"
		"\n"
		"wp 23456:data,a,w,wpdata == 1\n"
		"  Set a watchpoint that will halt execution whenever a write occurs in the address range "
		"23456-2345f of the data space and the data written is equal to 1.\n"
		"\n"
		"wp 3456:maincpu,20,r,1,{ printf \"Read @ %08X\\n\",wpaddr ; g }\n"
		"  Set a watchpoint on the CPU ':maincpu' that will halt execution whenever a read occurs "
		"in the address range 3456-3475.  When this happens, print Read @ <wpaddr> and continue "
		"execution.\n"
		"\n"
		"temp0 = 0 ; wp 45678,1,w,wpdata==f0,{ temp0++ ; g }\n"
		"  Set a watchpoint that will halt execution whenever a write occurs to the address 45678 "
		"and the value being written is equal to f0.  When that happens, increment the variable "
		"temp0 and continue execution.\n"
	},
	{ "wpdset", "#wpset" },
	{ "wpiset", "#wpset" },
	{ "wposet", "#wpset" },
	{
		"wpclear",
		"\n"
		"  wpclear [<wpnum>]\n"
		"\n"
		"The wpclear command clears a watchpoint.  If <wpnum> is specified, only the requested "
		"watchpoint is cleared; otherwise all watchpoints are cleared.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wpclear 3\n"
		"  Clear the watchpoint with index 3.\n"
		"\n"
		"wpclear\n"
		"  Clear all watchpoints.\n"
	},
	{
		"wpdisable",
		"\n"
		"  wpdisable [<wpnum>]\n"
		"\n"
		"The wpdisable command disables a watchpoint.  If <wpnum> is specified, only the requested "
		"watchpoint is disabled; otherwise all watchpoints are disabled.  Note that disabling a "
		"watchpoint does not delete it, it just temporarily marks the watchpoint as inactive.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wpdisable 3\n"
		"  Disable the watchpoint with index 3.\n"
		"\n"
		"wpdisable\n"
		"  Disable all watchpoints.\n"
	},
	{
		"wpenable",
		"\n"
		"  wpenable [<wpnum>]\n"
		"\n"
		"The wpenable command enables a watchpoint.  If <wpnum> is specified, only the requested "
		"watchpoint is enabled; otherwise all watchpoints are enabled.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wpenable 3\n"
		"  Enable the watchpoint with index 3.\n"
		"\n"
		"wpenable\n"
		"  Enable all watchpoints.\n"
	},
	{
		"wplist",
		"\n"
		"  wplist [<CPU>]\n"
		"\n"
		"The wplist list current watchpoints, along with their indices and any associated "
		"conditions or actions.  If no <CPU> is specified, watchpoints for all CPUs in the system "
		"will be listed; if a <CPU> is specified, only watchpoints for that CPU will be listed.  "
		"The <CPU> can be specified by tag or by debugger CPU number.\n"
		"\n"
		"Examples:\n"
		"\n"
		"wplist\n"
		"  List all watchpoints.\n"
		"\n"
		"wplist .\n"
		"  List all watchpoints for the visible CPU.\n"
		"\n"
		"wplist maincpu\n"
		"  List all watchpoints for the CPU ':maincpu'.\n"
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
		"  map[{d|i|o}] <address>[:<space>]\n"
		"\n"
		"The map commands map a logical memory address to the corresponding physical address, as "
		"well as reporting  the handler name.  The address may optionally be followed by a colon "
		"and device and/or address space.  The device may be specified as a tag or a debugger CPU "
		"number; if no device is specified, the CPU currently visible in the debugger is assumed.  "
		"If an address space is not specified, the command suffix sets the address space: 'map' "
		"defaults to the first address space exposed by the device, 'mapd' defaults to the data "
		"space, 'mapi' defaults to the I/O space, and 'mapo' defaults to the opcodes space.\n"
		"\n"
		"Examples:\n"
		"\n"
		"map 152d0\n"
		"  Gives physical address and handler name for logical address 152d0 in program memory for "
		"the currently visible CPU.\n"
		"\n"
		"map 107:sms_vdp\n"
		"  Gives physical address and handler name for logical address 107 in the first address "
		"space for the device ':sms_vdp'.\n"
	},
	{ "mapd", "#map" },
	{ "mapi", "#map" },
	{ "mapo", "#map" },
	{
		"memdump",
		"\n"
		"  memdump [<filename>,[<device>]]\n"
		"\n"
		"Dumps the current memory maps to the file specified by <filename>, or memdump.log if "
		"omitted.  If <device> is specified, only memory maps for the part of the device tree "
		"rooted at this device will be dumped.  Devices may be specified using tags or CPU "
		"numbers.\n"
		"\n"
		"Examples:\n"
		"\n"
		"memdump mylog.log\n"
		"  Dumps memory maps for all devices in the system to the file mylog.log.\n"
		"\n"
		"memdump\n"
		"  Dumps memory maps for all devices in the system to the file memdump.log.\n"
		"\n"
		"memdump audiomaps.log,audiopcb\n"
		"  Dumps memory maps for device ':audiopcb' and all its child devices to the file "
		"audiomaps.log.\n"
		"\n"
		"memdump mylog.log,1\n"
		"  Dumps memory maps for the CPU 1 and all its child devices to the file mylog.log.\n"
	},
	{
		"comlist",
		"\n"
		"  comlist\n"
		"\n"
		"Prints the currently available comment file in human readable form in debugger output window."
		"\n"
		"Examples:\n"
		"\n"
		"comlist\n"
		"  Shows currently available comments.\n"
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
	{ "//", "#comadd" },
	{
		"commit",
		"\n"
		"  commit[/*] <address>,<comment>\n"
		"\n"
		"Adds a string <comment> to the disassembled code at <address> then saves to file. Basically same as comadd + comsave via a single line.\n"
		"The shortcut for this command is simply '/*'\n"
		"\n"
		"Examples:\n"
		"\n"
		"commit 0, hello world.\n"
		"  Adds the comment 'hello world.' to the code at address 0x0 and saves comments\n"
		"\n"
		"/* 10, undocumented opcode!\n"
		"  Adds the comment 'undocumented opcode!' to the code at address 0x10 and saves comments\n"
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
		"  cheatinit [<sign><width>[<swap>],[<address>,<length>[,<CPU>]]]\n"
		"\n"
		"The cheatinit command initializes the cheat search to the selected memory area.\n"
		"If no parameter is specified the cheat search is initialized to all changeable memory of the main CPU.\n"
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
		"The cheatnext command will make comparisons with the previous search matches.\n"
		"Possible <condition>:\n"
		"  all\n"
		"   no <comparisonvalue> needed.\n"
		"   use to update the previous value without changing the current matches.\n"
		"  equal [eq]\n"
		"   without <comparisonvalue> search for all bytes that are equal to the previous search.\n"
		"   with <comparisonvalue> search for all bytes that are equal to the <comparisonvalue>.\n"
		"  notequal [ne]\n"
		"   without <comparisonvalue> search for all bytes that are not equal to the previous search.\n"
		"   with <comparisonvalue> search for all bytes that are not equal to the <comparisonvalue>.\n"
		"  decrease [de, +]\n"
		"   without <comparisonvalue> search for all bytes that have decreased since the previous search.\n"
		"   with <comparisonvalue> search for all bytes that have decreased by the <comparisonvalue> since the previous search.\n"
		"  increase [in, -]\n"
		"   without <comparisonvalue> search for all bytes that have increased since the previous search.\n"
		"   with <comparisonvalue> search for all bytes that have increased by the <comparisonvalue> since the previous search.\n"
		"  decreaseorequal [deeq]\n"
		"   no <comparisonvalue> needed.\n"
		"   search for all bytes that have decreased or have same value since the previous search.\n"
		"  increaseorequal [ineq]\n"
		"   no <comparisonvalue> needed.\n"
		"   search for all bytes that have decreased or have same value since the previous search.\n"
		"  smallerof [lt]\n"
		"   without <comparisonvalue> this condition is invalid\n"
		"   with <comparisonvalue> search for all bytes that are smaller than the <comparisonvalue>.\n"
		"  greaterof [gt]\n"
		"   without <comparisonvalue> this condition is invalid\n"
		"   with <comparisonvalue> search for all bytes that are larger than the <comparisonvalue>.\n"
		"  changedby [ch, ~]\n"
		"   without <comparisonvalue> this condition is invalid\n"
		"   with <comparisonvalue> search for all bytes that have changed by the <comparisonvalue> since the previous search.\n"
		"\n"
		"Examples:\n"
		"\n"
		"cheatnext increase\n"
		"  search for all bytes that have increased since the previous search.\n"
		"\n"
		"cheatnext decrease, 1\n"
		"  search for all bytes that have decreased by 1 since the previous search.\n"
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
		"   use to update the previous value without changing the current matches.\n"
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
		"Lists the instance names for media images devices in the system and the currently mounted "
		"media images, if any.  Brief instance names, as allowed for command line media options, "
		"are listed.\n"
		"\n"
		"Examples:\n"
		"\n"
		"images\n"
		"  Lists image device instance names and mounted media.\n"
	},
	{
		"mount",
		"\n"
		"  mount <instance>,<filename>\n"
		"\n"
		"Mounts a file on a media device.  The device may be specified by its instance name or "
		"brief instance name, as allowed for command line media options.\n"
		"\n"
		"Some media devices allow software list items to be mounted using this command by supplying "
		"the short name of the software list item in place of a filename for the <filename> "
		"parameter.\n"
		"\n"
		"Examples:\n"
		"\n"
		"mount flop1,os1xutls.td0\n"
		"  Mount the file 'os1xutls.td0' on the media device with instance name 'flop1'.\n"
		"mount cart,10yard\n"
		"  Mount the software list item with short name '10yard' on the media device with instance "
		"name 'cart'.\n"
	},
	{
		"unmount",
		"\n"
		"  unmount <instance>\n"
		"\n"
		"Unmounts the mounted media image (if any) from a device.  The device may be specified by "
		"its instance name or brief instance name, as allowed for command line media options.\n"
		"\n"
		"Examples:\n"
		"\n"
		"unmount cart\n"
		"  Unmounts any media image mounted on the device with instance name 'cart'.\n"
	}
};



/***************************************************************************
    HELP_MANAGER
***************************************************************************/

class help_manager
{
private:
	using help_map = std::map<std::string_view, char const *>;

	help_map m_help_list;
	help_item const *m_uncached_help = std::begin(f_static_help_list);

	help_manager() = default;

public:
	char const *find(std::string_view tag)
	{
		// find a cached exact match if possible
		std::string const lower = strmakelower(tag);
		auto const found = m_help_list.find(lower);
		if (m_help_list.end() != found)
			return found->second;

		// cache more entries while searching for an exact match
		while (std::end(f_static_help_list) != m_uncached_help)
		{
			help_map::iterator ins;
			if (*m_uncached_help->help == '#')
			{
				auto const xref = m_help_list.find(&m_uncached_help->help[1]);
				assert(m_help_list.end() != xref);
				ins = m_help_list.emplace(m_uncached_help->tag, xref->second).first;
			}
			else
			{
				ins = m_help_list.emplace(m_uncached_help->tag, m_uncached_help->help).first;
			}
			++m_uncached_help;
			if (lower == ins->first)
				return ins->second;
		}

		// find a partial match
		auto candidate = m_help_list.lower_bound(lower);
		if ((m_help_list.end() != candidate) && (candidate->first.substr(0, lower.length()) == lower))
		{
			// if only one partial match, take it
			auto const next = std::next(candidate);
			if ((m_help_list.end() == next) || (next->first.substr(0, lower.length()) != lower))
				return candidate->second;

			// TODO: pointers to static strings are bad, mmmkay?
			static char ambig_message[1024];
			int msglen = std::sprintf(ambig_message, "Ambiguous help request, did you mean:\n");
			do
			{
				msglen += std::sprintf(&ambig_message[msglen], "  help %.*s?\n", int(candidate->first.length()), &candidate->first[0]);
				++candidate;
			}
			while ((m_help_list.end() != candidate) && (candidate->first.substr(0, lower.length()) == lower));
			return ambig_message;
		}

		// take the first help entry if no matches at all
		return f_static_help_list[0].help;
	}

	static help_manager &instance()
	{
		static help_manager s_instance;
		return s_instance;
	}
};

} // anonymous namespace



/***************************************************************************
    PUBLIC INTERFACE
***************************************************************************/

const char *debug_get_help(std::string_view tag)
{
	return help_manager::instance().find(tag);
}
