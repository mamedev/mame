// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "emu.h"
#include "machine/fddebug.h"

void fd1094_init_debugging(running_machine &machine, const char *cpureg, const char *keyreg, const char *statreg, void (*changed)(running_machine &))
{
}

#if 0

/***************************************************************************

    fddebug.c

    FD1094 decryption helper routines.

****************************************************************************

    When searching for new keys, here are some common sequences in the
    System 16B games that are useful.

    IRQ4 handler entry points:

        common sequence 1:
            MOVE        SR,(A7)             40D7
            MOVE.B      #$23,(A7)           1EBC 0023
            MOVEM.L     D0-D7/A0-A6,-(A7)   48E7 FFFE

        common sequence 2:
            MOVEM.L     D0-D7/A0-A6,-(A7)   48E7 FFFE

        common sequence 3:
            BRA.W       <previous sequence> 6000 xxxx

    IRQ4 handler exit points:

        common sequence (often appears twice nearby):
            MOVE        (A7)+,D0-D7/A0-A6   4CDF 7FFF
            RTE                             4E73

    Entry points:

        common sequence 1:
            LEA         <stack>.L,A7        4FF9 xxxx xxxx
            MOVE        #$2700,SR           46FC 2700
            CMPI.L      #$00xxffff,D0       0C80 00xx FFFF
            MOVEQ       #0,D0
            MOVE.L      D0,D1               2200
            MOVE.L      D0,D2               2400
            MOVE.L      D0,D3               2600
            MOVE.L      D0,D4               2800
            MOVE.L      D0,D5               2A00
            MOVE.L      D0,D6               2C00
            MOVE.L      D0,D7               2E00

        common sequence 2:
            LEA         <stack>.W,A7        4FF8 xxxx
            MOVE        #$2700,SR           46FC 2700
            CMPI.L      #$00xxffff,D0       0C80 00xx FFFF
            MOVEQ       #0,D0
            MOVE.L      D0,D1               2200
            MOVE.L      D0,D2               2400
            MOVE.L      D0,D3               2600
            MOVE.L      D0,D4               2800
            MOVE.L      D0,D5               2A00
            MOVE.L      D0,D6               2C00
            MOVE.L      D0,D7               2E00

        common sequence 3:
            LEA         <stack>.W,A7        4FF8 xxxx
            MOVE        #$2700,SR           46FC 2700
            MOVEQ       #0,D0
            MOVE.L      D0,D1               2200
            MOVE.L      D0,D2               2400
            MOVE.L      D0,D3               2600
            MOVE.L      D0,D4               2800
            MOVE.L      D0,D5               2A00
            MOVE.L      D0,D6               2C00
            MOVE.L      D0,D7               2E00

        common sequence 4:
            BRA.W       <previous sequence> 6000 xxxx

****************************************************************************

    These constraints worked for finding exctleag's seed:

        fdcset 0410,4ff9
        fdcset 0412,0000
        fdcset 0414,0000
        fdcset 0416,46fc
        fdcset 0418,2700
        fdcset 041a,0c80
        fdcset 041c,0000,ff00
        fdcset 041e,ffff

        //fdcset 0f9e,40d7,ffff,irq
        fdcset 0fa0,1ebc,ffff,irq
        fdcset 0fa2,0023,ffff,irq
        //fdcset 0fa4,48e7,ffff,irq
        fdcset 0fa6,fffe,ffff,irq
        fdcset 0fa8,13f8,ffff,irq
        fdcset 0fac,00c4,ffff,irq
        fdcset 0fae,0001,ffff,irq

        //fdcset 1060,4cdf,ffff,irq
        fdcset 1062,7fff,ffff,irq
        //fdcset 1064,4e73,ffff,irq
        //fdcset 1070,4cdf,ffff,irq
        fdcset 1072,7fff,ffff,irq
        //fdcset 1074,4e73,ffff,irq

***************************************************************************/

#include "emu.h"
#include "machine/fd1094.h"
#include "cpu/m68000/m68000.h"

#include "debug/debugcmd.h"
#include "debug/debugcon.h"
#include "debug/debugcpu.h"
#include "debug/debugvw.h"
#include "machine/fddebug.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define KEY_SIZE            8192
#define MAX_CONSTRAINTS     100
#define MAX_SEARCH_DEPTH    10000

/* status byte breakdown */
#define STATE_MASK          0xff00
#define HIBITS_MASK         0x00c0
#define SEARCH_MASK         0x0020
#define STATUS_MASK         0x001f

/* possible status values */
#define STATUS_UNVISITED    0x00
#define STATUS_LOCKED       0x01
#define STATUS_NOCHANGE     0x02
#define STATUS_GUESS        0x03

/* sizes for the opcode table */
#define SIZE_BYTE           1       /* single byte */
#define SIZE_WORD           2       /* single word */
#define SIZE_LONG           3       /* single long */
#define SIZE_BIT            4       /* single byte, limited to bit sizes (0-7) */
#define SIZE_MASK           7

/* operand sizes */
#define OF_SIZEMASK         (SIZE_MASK << 0)
#define OF_BYTE             (SIZE_BYTE << 0)    /* byte size operation */
#define OF_WORD             (SIZE_WORD << 0)    /* word size operation */
#define OF_LONG             (SIZE_LONG << 0)    /* long size operation */

/* immediate sizes */
#define OF_ISIZEMASK        (SIZE_MASK << 3)
#define OF_IMMB             (SIZE_BYTE << 3)    /* immediate byte follows */
#define OF_IMMW             (SIZE_WORD << 3)    /* immediate word follows */
#define OF_IMML             (SIZE_LONG << 3)    /* immediate long follows */
#define OF_IMMBIT           (SIZE_BIT << 3)     /* immediate byte follows */

/* other opcode flags */
#define OF_EASRC            0x00000040          /* standard EA is source */
#define OF_EADST            0x00000080          /* standard EA is destination */
#define OF_EADREG           0x00000100          /* EA with data register is allowed */
#define OF_EAAREG           0x00000200          /* EA with address register is allowed */
#define OF_EAA              0x00000400          /* EA with (An) is allowed */
#define OF_EAPLUS           0x00000800          /* EA with (An)+ is allowed */
#define OF_EAMINUS          0x00001000          /* EA with -(An) is allowed */
#define OF_EADISP           0x00002000          /* EA with (D,An) displacement is allowed */
#define OF_EAABS            0x00004000          /* EA with absolute (both word and long) is allowed */
#define OF_EAIMM            0x00008000          /* EA with immediate is allowed */
#define OF_EAPCR            0x00010000          /* EA with PC-relative addressing is allowed */
#define OF_RARE             0x00080000          /* opcode is not commonly used */
#define OF_BRANCH           0x00100000          /* opcode represents a branch */
#define OF_JMP              0x00200000          /* opcode represents a jmp/jsr */
#define OF_MOVE             0x00400000          /* opcode has MOVE semantics */
#define OF_LENMASK          0xf0000000          /* opcode length mask */
#define OF_INVALID          0xffffffff          /* invalid opcode */



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* a single possible instruction decoding */
struct fd1094_possibility
{
	offs_t      basepc;             /* starting PC of the possibility */
	int         length;             /* number of words */
	UINT8       instrbuffer[10];    /* instruction data for disassembler */
	UINT8       keybuffer[10];      /* array of key values to produce the instruction data */
	UINT8       iffy;               /* is this an iffy possibility? */
	char        dasm[256];          /* disassembly */
};

/* an entry in the opcode table */
struct optable_entry
{
	UINT32          flags;          /* per-opcode flags */
	const char *    string;         /* identifying string */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* array of PCs not to stop at */
static UINT8 *              ignorepc;
static UINT8                ignore_all;

/* array of information about each opcode */
static optable_entry *      optable;

/* buffer for undoing operations */
static UINT8 *              undobuff;

/* array of possible instruction decodings */
static fd1094_possibility   posslist[4*4*4*4*4];
static int                  posscount;

/* array of possible seeds */
static UINT32 *             possible_seed;

/* array of constraints */
static fd1094_constraint    constraints[MAX_CONSTRAINTS];
static int                  constcount;

/* stack of search addresses */
static UINT32               searchstack[MAX_SEARCH_DEPTH];
static int                  searchsp;

/* current key generation parameters */
static UINT32               fd1094_global;
static UINT32               fd1094_seed;
static UINT8                keydirty;

/* pointers to our data */
static UINT16 *             coderegion;
static UINT32               coderegion_words;
static UINT8 *              keyregion;
static UINT16 *             keystatus;
static UINT32               keystatus_words;

/* key changed callback */
static void                 (*key_changed)(running_machine &);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void set_default_key_params(running_machine &machine);
static void load_overlay_file(running_machine &machine);
static void save_overlay_file(running_machine &machine);

static int instruction_hook(device_t &device, offs_t curpc);

static void execute_fdsave(running_machine &machine, int ref, int params, const char **param);
static void execute_fdoutput(running_machine &machine, int ref, int params, const char **param);
static void execute_fdseed(running_machine &machine, int ref, int params, const char **param);
static void execute_fdlockguess(running_machine &machine, int ref, int params, const char **param);
static void execute_fdeliminate(running_machine &machine, int ref, int params, const char **param);
static void execute_fdunlock(running_machine &machine, int ref, int params, const char **param);
static void execute_fdignore(running_machine &machine, int ref, int params, const char **param);
static void execute_fdundo(running_machine &machine, int ref, int params, const char **param);
static void execute_fdstatus(running_machine &machine, int ref, int params, const char **param);
static void execute_fdstate(running_machine &machine, int ref, int params, const char **param);
static void execute_fdpc(running_machine &machine, int ref, int params, const char **param);
static void execute_fdsearch(running_machine &machine, int ref, int params, const char **param);
static void execute_fddasm(running_machine &machine, int ref, int params, const char **param);
static void execute_fdcset(running_machine &machine, int ref, int params, const char **param);
static void execute_fdclist(running_machine &machine, int ref, int params, const char **param);
static void execute_fdcsearch(running_machine &machine, int ref, int params, const char **param);

static fd1094_possibility *try_all_possibilities(address_space &space, int basepc, int offset, int length, UINT8 *instrbuffer, UINT8 *keybuffer, fd1094_possibility *possdata);
static void tag_possibility(running_machine &machine, fd1094_possibility *possdata, UINT8 status);

static void perform_constrained_search(running_machine &machine);
static UINT32 find_global_key_matches(UINT32 startwith, UINT16 *output);
static int find_constraint_sequence(UINT32 global, int quick);
static int does_key_work_for_constraints(const UINT16 *base, UINT8 *key);
static UINT32 reconstruct_base_seed(int keybaseaddr, UINT32 startseed);

static void build_optable(running_machine &machine);
static int validate_ea(address_space &space, UINT32 pc, UINT8 modereg, const UINT8 *parambase, UINT32 flags);
static int validate_opcode(address_space &space, UINT32 pc, const UINT8 *opdata, int maxwords);




/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-----------------------------------------------
    addr_to_keyaddr - given an address,
    return the address in the key that will be
    used to decrypt it
-----------------------------------------------*/

static inline int addr_to_keyaddr(offs_t address)
{
	/* for address xx0000-xx0006 (but only if >= 000008), use key xx2000-xx2006 */
	if ((address & 0x0ffc) == 0 && address >= 4)
		return (address & 0x1fff) | 0x1000;
	else
		return address & 0x1fff;
}


/*-----------------------------------------------
    mask_for_keyaddr - given a key address,
    return a mask indicating which bits should
    always be 1
-----------------------------------------------*/

static inline UINT8 mask_for_keyaddr(offs_t address)
{
	/* the first half of the key always has bit 0x80 set; the second half 0x40 */
	/* however, the values at 0000-0003 and 1000-1003 don't follow this rule */
	if ((address & 0x0ffc) == 0)
		return 0x00;
	else if ((address & 0x1000) == 0)
		return 0x80;
	else
		return 0x40;
}


/*-----------------------------------------------
    advance_seed - advance the PRNG seed by
    the specified number of steps
-----------------------------------------------*/

static inline UINT32 advance_seed(UINT32 seed, int count)
{
	/* iterate over the seed for 'count' reps */
	while (count--)
	{
		seed = seed * 0x29;
		seed += seed << 16;
	}
	return seed;
}


/*-----------------------------------------------
    key_value_from_seed - extract the key value
    from a seed and apply the given mask
-----------------------------------------------*/

static inline UINT8 key_value_from_seed(UINT32 seed, UINT8 mask)
{
	/* put bits 16-21 of the seed in the low 6 bits and OR with the mask */
	return ((~seed >> 16) & 0x3f) | mask;
}


/*-----------------------------------------------
    generate_key_bytes - generate a sequence of
    consecutive key bytes, starting with the
    given seed
-----------------------------------------------*/

static inline void generate_key_bytes(UINT8 *dest, UINT32 keyoffs, UINT32 count, UINT32 seed)
{
	int bytenum;

	/* generate 'count' bytes of a key */
	for (bytenum = 0; bytenum < count; bytenum++)
	{
		UINT32 keyaddr = (keyoffs + bytenum) & 0x1fff;
		UINT8 mask = mask_for_keyaddr(keyaddr);

		/* advance the seed first, then store the derived value */
		seed = advance_seed(seed, 1);
		dest[keyaddr] = key_value_from_seed(seed, mask);
	}
}


/*-----------------------------------------------
    get_opcode_length - return the length of
    an opcode based on the opcode
-----------------------------------------------*/

static inline UINT8 get_opcode_length(UINT16 opcode)
{
	/* return the length from the table */
	return optable[opcode].flags >> 28;
}


/*-----------------------------------------------
    set_constraint - set the values of a
    constraint
-----------------------------------------------*/

static inline void set_constraint(fd1094_constraint *constraint, UINT32 pc, UINT16 state, UINT16 value, UINT16 mask)
{
	constraint->pc = pc;
	constraint->state = state;
	constraint->value = value & mask;
	constraint->mask = mask;
}

/*-----------------------------------------------
    print_possibilities - print possibilities
    for a given address
-----------------------------------------------*/

static inline void print_possibilities(running_machine &machine)
{
	int i;

	debug_console_printf(machine, "Possibilities @ %06X:\n", posslist[0].basepc);
	for (i = 0; i < posscount; i++)
		debug_console_printf(machine, " %c%2x: %s\n", posslist[i].iffy ? ' ' : '*', i, posslist[i].dasm);
}


/*-----------------------------------------------
    pc_is_valid - is a given PC value valid?
    0=no, 1=yes, 2=unlikely
-----------------------------------------------*/

static inline int pc_is_valid(address_space &space, UINT32 pc, UINT32 flags)
{
	/* if we're odd or out of range, fail */
	if ((pc & 1) == 1)
		return 0;
	if (pc & 0xff000000)
		return 0;
	if (space.direct().read_ptr(pc) == NULL)
		return 0;
	return 1;
}


/*-----------------------------------------------
    addr_is_valid - is a given address value
    valid? 0=no, 1=yes, 2=unlikely
-----------------------------------------------*/

static inline int addr_is_valid(address_space &space, UINT32 addr, UINT32 flags)
{
	/* if this a JMP, the address is a PC */
	if (flags & OF_JMP)
		return pc_is_valid(space, addr, flags);

	/* if we're odd or out of range, fail */
	if ((flags & OF_SIZEMASK) != OF_BYTE && (addr & 1) == 1)
		return 0;
	if ((addr & 0xff000000) != 0 && (addr & 0xff000000) != 0xff000000)
		return 0;

	/* if we're invalid, fail */
	if (strcmp(const_cast<address_space &>(space)->get_handler_string(ROW_READ, addr), "segaic16_memory_mapper_lsb_r") == 0)
		return 2;

	return 1;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-----------------------------------------------
    fd1094_init_debugging - set up debugging
-----------------------------------------------*/

void fd1094_init_debugging(running_machine &machine, const char *cpureg, const char *keyreg, const char *statreg, void (*changed)(running_machine &))
{
	/* set the key changed callback */
	key_changed = changed;

	/* set up the regions */
	coderegion = (UINT16 *)machine.root_device().memregion(cpureg)->base();
	coderegion_words = machine.root_device().memregion(cpureg)->bytes() / 2;
	keyregion = (UINT8 *)machine.root_device().memregion(keyreg)->base();
	keystatus = (UINT16 *)machine.root_device().memregion(statreg)->base();
	keystatus_words = machine.root_device().memregion(statreg)->bytes() / 2;
	assert(coderegion_words == keystatus_words);

	/* allocate memory for the ignore table */
	ignorepc = auto_alloc_array_clear(machine, UINT8, 1 << 23);

	/* allocate memory for the undo buffer */
	undobuff = std::make_unique<UINT8[]>(keystatus_words * 2);
	memcpy(undobuff, keystatus, keystatus_words * 2);

	/* allocate memory for the possible seeds array */
	possible_seed = std::make_unique<UINT32[]>(65536);

	/* build the opcode table */
	build_optable(machine);

	/* set up default constraints */
	constcount = 0;
	set_constraint(&constraints[constcount++], 0x000000, FD1094_STATE_RESET, 0x0000, 0xffff);
	set_constraint(&constraints[constcount++], 0x000002, FD1094_STATE_RESET, 0x0000, 0xffff);
	set_constraint(&constraints[constcount++], 0x000004, FD1094_STATE_RESET, 0x0000, 0xffff);
	set_constraint(&constraints[constcount++], 0x000006, FD1094_STATE_RESET, 0x0000, 0xc001);

	/* determine the key parameters */
	set_default_key_params(machine);

	/* read the key overlay file */
	load_overlay_file(machine);

	/* add some commands */
	debug_console_register_command(machine, "fdsave", CMDFLAG_NONE, 0, 0, 0, execute_fdsave);
	debug_console_register_command(machine, "fdoutput", CMDFLAG_NONE, 0, 1, 1, execute_fdoutput);
	debug_console_register_command(machine, "fdseed", CMDFLAG_NONE, 0, 2, 2, execute_fdseed);
	debug_console_register_command(machine, "fdguess", CMDFLAG_NONE, STATUS_GUESS, 1, 1, execute_fdlockguess);
	debug_console_register_command(machine, "fdlock", CMDFLAG_NONE, STATUS_LOCKED, 1, 1, execute_fdlockguess);
	debug_console_register_command(machine, "fdeliminate", CMDFLAG_NONE, 0, 1, 10, execute_fdeliminate);
	debug_console_register_command(machine, "fdunlock", CMDFLAG_NONE, 0, 1, 1, execute_fdunlock);
	debug_console_register_command(machine, "fdignore", CMDFLAG_NONE, 0, 0, 1, execute_fdignore);
	debug_console_register_command(machine, "fdundo", CMDFLAG_NONE, 0, 0, 0, execute_fdundo);
	debug_console_register_command(machine, "fdstatus", CMDFLAG_NONE, 0, 0, 0, execute_fdstatus);
	debug_console_register_command(machine, "fdstate", CMDFLAG_NONE, 0, 0, 1, execute_fdstate);
	debug_console_register_command(machine, "fdpc", CMDFLAG_NONE, 0, 0, 1, execute_fdpc);
	debug_console_register_command(machine, "fdsearch", CMDFLAG_NONE, 0, 0, 0, execute_fdsearch);
	debug_console_register_command(machine, "fddasm", CMDFLAG_NONE, 0, 1, 1, execute_fddasm);
	debug_console_register_command(machine, "fdcset", CMDFLAG_NONE, 0, 2, 4, execute_fdcset);
	debug_console_register_command(machine, "fdclist", CMDFLAG_NONE, 0, 0, 0, execute_fdclist);
	debug_console_register_command(machine, "fdcsearch", CMDFLAG_NONE, 0, 0, 0, execute_fdcsearch);

	/* set up the instruction hook */
	machine.device("maincpu")->debug()->set_instruction_hook(instruction_hook);

	/* regenerate the key */
	if (keydirty)
		fd1094_regenerate_key(machine);
}


/*-----------------------------------------------
    set_default_key_params - based on the game
    name, set some defaults
-----------------------------------------------*/

static void set_default_key_params(running_machine &machine)
{
	static const struct
	{
		const char *    gamename;
		UINT32          global;
		UINT32          seed;
	} default_keys[] =
	{
		{ "altbeastj1", 0xFCAFF9F9, 0x177AC6 },
		{ "bullet",   0x12A8F9EC, 0x1B1FC3 },
	};
	int keynum;

	/* look for a matching game and set the key appropriately */
	for (keynum = 0; keynum < ARRAY_LENGTH(default_keys); keynum++)
		if (strcmp(machine.system().name, default_keys[keynum].gamename) == 0)
		{
			fd1094_global = default_keys[keynum].global;
			fd1094_seed = default_keys[keynum].seed;
			keydirty = TRUE;
			break;
		}
}


/*-----------------------------------------------
    load_overlay_file - load the key overlay
    file
-----------------------------------------------*/

static void load_overlay_file(running_machine &machine)
{
	int pcaddr;

	/* determine the filename and open the file */
	emu_file file(OPEN_FLAG_READ);
	file_error filerr = file.open(machine.system().name, ".kov");
	if (filerr == FILERR_NONE)
	{
		file.read(keystatus, keystatus_words * 2);

		/* convert from big-endian */
		for (pcaddr = 0; pcaddr < keystatus_words; pcaddr++)
			keystatus[pcaddr] = BIG_ENDIANIZE_INT16(keystatus[pcaddr]) & ~SEARCH_MASK;
	}

	/* mark the key dirty */
	keydirty = TRUE;
}


/*-----------------------------------------------
    save_overlay_file - save the key overlay
    file
-----------------------------------------------*/

static void save_overlay_file(running_machine &machine)
{
	int pcaddr;

	/* determin the filename and open the file */
	emu_file file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
	file_error filerr = file.open(machine.system().name, ".kov");
	if (filerr == FILERR_NONE)
	{
		/* convert to big-endian */
		for (pcaddr = 0; pcaddr < keystatus_words; pcaddr++)
			keystatus[pcaddr] = BIG_ENDIANIZE_INT16(keystatus[pcaddr]);

		/* write the data */
		file.write(keystatus, keystatus_words * 2);

		/* convert from big-endian */
		for (pcaddr = 0; pcaddr < keystatus_words; pcaddr++)
			keystatus[pcaddr] = BIG_ENDIANIZE_INT16(keystatus[pcaddr]);
	}
}


/*-----------------------------------------------
    fd1094_regenerate_key - regenerate the key
    based on the raw parameters and the overlay
    data
-----------------------------------------------*/

void fd1094_regenerate_key(running_machine &machine)
{
	int reps = keystatus_words / KEY_SIZE;
	int keyaddr, repnum;

	/* store the global key in the first 4 bytes */
	keyregion[0] = fd1094_global >> 24;
	keyregion[1] = fd1094_global >> 16;
	keyregion[2] = fd1094_global >> 8;
	keyregion[3] = fd1094_global >> 0;

	/* then generate the remaining 8188 bytes */
	generate_key_bytes(keyregion, 4, 8192 - 4, fd1094_seed);

	/* apply the overlay */
	for (keyaddr = 4; keyaddr < KEY_SIZE; keyaddr++)
	{
		keyregion[keyaddr] |= keystatus[keyaddr] & HIBITS_MASK;

		/* if we're locked, propogate that info to all our reps */
		if ((keystatus[keyaddr] & STATUS_MASK) == STATUS_LOCKED)
			for (repnum = 1; repnum < reps; repnum++)
			{
				keystatus[repnum * KEY_SIZE + keyaddr] = (keystatus[repnum * KEY_SIZE + keyaddr] & ~STATUS_MASK) | STATUS_LOCKED;
				if ((keyaddr & 0x1ffc) == 0x1000)
					keystatus[repnum * KEY_SIZE + keyaddr - 0x1000] = (keystatus[repnum * KEY_SIZE + keyaddr - 0x1000] & ~STATUS_MASK) | STATUS_LOCKED;
			}
	}

	/* update the key with the current fd1094 manager */
	if (key_changed != NULL)
		(*key_changed)(machine);

	/* force all memory and disassembly views to update */
	machine.debug_view().update_all(DVT_MEMORY);
	machine.debug_view().update_all(DVT_DISASSEMBLY);

	/* reset keydirty */
	keydirty = FALSE;
}


/*-----------------------------------------------
    instruction_hook - per-instruction hook
-----------------------------------------------*/

static int instruction_hook(device_t &device, offs_t curpc)
{
	int curfdstate = fd1094_set_state(keyregion, -1);
	UINT8 instrbuffer[10], keybuffer[5];
	int i, keystat;

	/* quick exit if we're ignoring */
	if (ignore_all || ignorepc[curpc/2])
		return 0;

	/* quick exit if we're already locked */
	keystat = keystatus[curpc/2] & STATUS_MASK;
	keystatus[curpc/2] = (keystatus[curpc/2] & ~STATE_MASK) | (curfdstate << 8);
	if (keystat == STATUS_LOCKED || keystat == STATUS_NOCHANGE)
	{
		UINT16 opcode = fd1094_decode(curpc/2, coderegion[curpc/2], keyregion, 0);
		int length = get_opcode_length(opcode);
		for (i = 1; i < length; i++)
		{
			keystat = keystatus[curpc/2 + i] & STATUS_MASK;
			if (keystat != STATUS_LOCKED && keystat != STATUS_NOCHANGE)
				break;
		}
		if (i == length)
		{
			for (i = 1; i < length; i++)
				keystatus[curpc/2 + i] = (keystatus[curpc/2 + i] & ~STATE_MASK) | (curfdstate << 8);
			return 0;
		}
	}

	/* try all possible decodings at the current pc */
	posscount = try_all_possibilities(device.memory().space(AS_PROGRAM), curpc, 0, 0, instrbuffer, keybuffer, posslist) - posslist;
	if (keydirty)
		fd1094_regenerate_key(device.machine());

	/* if we only ended up with one possibility, mark that one as good */
	if (posscount == 1)
	{
		tag_possibility(device.machine(), &posslist[0], STATUS_LOCKED);
		fd1094_regenerate_key(device.machine());
		return 0;
	}

	/* print possibilities and break */
	print_possibilities(device.machine());
	return 1;
}


/*-----------------------------------------------
    execute_fdsave - handle the 'fdsave' command
-----------------------------------------------*/

static void execute_fdsave(running_machine &machine, int ref, int params, const char **param)
{
	save_overlay_file(machine);
	debug_console_printf(machine, "File saved\n");
}


/*-----------------------------------------------
    execute_fdoutput - output the current key
    to a file
-----------------------------------------------*/

static void execute_fdoutput(running_machine &machine, int ref, int params, const char **param)
{
	/* make sure we're up-to-date */
	if (keydirty)
		fd1094_regenerate_key(machine);

	/* determin the filename and open the file */
	emu_file file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
	file_error filerr = file.open(param[0]);
	if (filerr == FILERR_NONE)
		file.write(keyregion, KEY_SIZE);

	debug_console_printf(machine, "File '%s' saved\n", param[0]);
}


/*-----------------------------------------------
    execute_fdseed - handle the 'fdseed' command
-----------------------------------------------*/

static void execute_fdseed(running_machine &machine, int ref, int params, const char **param)
{
	UINT64 num1, num2;

	/* extract the parameters */
	if (!debug_command_parameter_number(machine, param[0], &num1))
		return;
	if (!debug_command_parameter_number(machine, param[1], &num2))
		return;

	/* set the global and seed, and then regenerate the key */
	fd1094_global = num1;
	fd1094_seed = num2;

	/* clear out our buffer */
	memset(keystatus, 0, keystatus_words * sizeof(keystatus[0]));

	/* regenerate the key and reset the 68000 */
	fd1094_regenerate_key(machine);
}


/*-----------------------------------------------
    execute_fdlockguess - handle the 'fdlock'
    and 'fdguess' commands
-----------------------------------------------*/

static void execute_fdlockguess(running_machine &machine, int ref, int params, const char **param)
{
	UINT64 num1;

	/* extract the parameter */
	if (!debug_command_parameter_number(machine, param[0], &num1))
		return;

	/* make sure it is within range of our recent possibilities */
	if (num1 >= posscount)
	{
		debug_console_printf(machine, "Possibility of out range (%x max)\n", posscount);
		return;
	}

	/* create an undo buffer */
	memcpy(undobuff, keystatus, keystatus_words * 2);

	/* tag this possibility as indicated by the ref parameter, and then regenerate the key */
	tag_possibility(machine, &posslist[num1], ref);
	fd1094_regenerate_key(machine);
}


/*-----------------------------------------------
    execute_fdeliminate - handle the
    'fdeliminate' command
-----------------------------------------------*/

static void execute_fdeliminate(running_machine &machine, int ref, int params, const char **param)
{
	int pnum, posssrc, possdst;
	int plist[10];

	/* extract parameters */
	for (pnum = 0; pnum < params; pnum++)
	{
		UINT64 num1;

		/* extract the parameters */
		if (!debug_command_parameter_number(machine, param[pnum], &num1))
			return;

		/* make sure it is within range of our recent possibilities */
		if (num1 >= posscount)
		{
			debug_console_printf(machine, "Possibility %x of out range (%x max)\n", (int)num1, posscount);
			return;
		}

		/* set the entry */
		plist[pnum] = num1;
	}

	/* loop over parameters */
	for (posssrc = possdst = 0; posssrc < posscount; posssrc++)
	{
		/* is the current pnum in our list to delete? */
		for (pnum = 0; pnum < params; pnum++)
			if (plist[pnum] == posssrc)
				break;

		/* if not, copy to the dest */
		if (pnum == params)
			posslist[possdst++] = posslist[posssrc];
	}

	/* set the final count */
	posscount = possdst;

	/* reprint the possibilities */
	print_possibilities(machine);
}


/*-----------------------------------------------
    execute_fdunlock - handle the 'fdunlock'
    command
-----------------------------------------------*/

static void execute_fdunlock(running_machine &machine, int ref, int params, const char **param)
{
	device_t *cpu = debug_cpu_get_visible_cpu(machine);
	int reps = keystatus_words / KEY_SIZE;
	int keyaddr, repnum;
	UINT64 offset;

	/* support 0 or 1 parameters */
	if (params != 1 || !debug_command_parameter_number(machine, param[0], &offset))
		offset = cpu->safe_pc();
	keyaddr = addr_to_keyaddr(offset / 2);

	/* toggle the ignore PC status */
	debug_console_printf(machine, "Unlocking PC %06X\n", (int)offset);

	/* iterate over all reps and unlock them */
	for (repnum = 0; repnum < reps; repnum++)
	{
		UINT16 *dest = &keystatus[repnum * KEY_SIZE + keyaddr];
		if ((*dest & STATUS_MASK) == STATUS_LOCKED)
			*dest &= ~STATUS_MASK & ~HIBITS_MASK;

		/* unlock the duplicate key bytes as well */
		if ((keyaddr & 0x1ffc) == 0x1000)
		{
			dest = &keystatus[repnum * KEY_SIZE + keyaddr - 0x1000];
			if ((*dest & STATUS_MASK) == STATUS_LOCKED)
				*dest &= ~STATUS_MASK & ~HIBITS_MASK;
		}
	}
}


/*-----------------------------------------------
    execute_fdignore - handle the 'fdignore'
    command
-----------------------------------------------*/

static void execute_fdignore(running_machine &machine, int ref, int params, const char **param)
{
	device_t *cpu = debug_cpu_get_visible_cpu(machine);
	UINT64 offset;

	/* support 0 or 1 parameters */
	if (params == 1 && strcmp(param[0], "all") == 0)
	{
		ignore_all = TRUE;
		debug_console_printf(machine, "Ignoring all unknown opcodes\n");
		return;
	}
	if (params != 1 || !debug_command_parameter_number(machine, param[0], &offset))
		offset = cpu->safe_pc();
	offset /= 2;

	/* toggle the ignore PC status */
	ignorepc[offset] = !ignorepc[offset];
	if (ignorepc[offset])
		debug_console_printf(machine, "Ignoring address %06X\n", (int)offset * 2);
	else
		debug_console_printf(machine, "No longer ignoring address %06X\n", (int)offset * 2);

	/* if no parameter given, implicitly run as well */
	if (params == 0)
		debug_cpu_get_visible_cpu(machine)->debug()->go();
}


/*-----------------------------------------------
    execute_fdundo - handle the 'fdundo'
    command
-----------------------------------------------*/

static void execute_fdundo(running_machine &machine, int ref, int params, const char **param)
{
	/* copy the undobuffer back and regenerate the key */
	memcpy(keystatus, undobuff, keystatus_words * 2);
	fd1094_regenerate_key(machine);
	debug_console_printf(machine, "Undid last change\n");
}


/*-----------------------------------------------
    execute_fdstatus - handle the 'fdstatus'
    command
-----------------------------------------------*/

static void execute_fdstatus(running_machine &machine, int ref, int params, const char **param)
{
	int numreps = keystatus_words / KEY_SIZE;
	int locked = 4, nomatter = 0, guesses = 0;
	int keyaddr;

	/* count how many locked keys we have */
	for (keyaddr = 4; keyaddr < KEY_SIZE; keyaddr++)
	{
		int count[STATUS_MASK + 1] = { 0 };
		int repnum;

		for (repnum = 0; repnum < numreps; repnum++)
			count[keystatus[repnum * KEY_SIZE + keyaddr] & STATUS_MASK]++;
		if (count[STATUS_LOCKED] > 0)
			locked++;
		else if (count[STATUS_GUESS] > 0)
			guesses++;
		else
			nomatter++;
	}
	debug_console_printf(machine, "%4d/%4d keys locked (%d%%)\n", locked, KEY_SIZE, locked * 100 / KEY_SIZE);
	debug_console_printf(machine, "%4d/%4d keys guessed (%d%%)\n", guesses, KEY_SIZE, guesses * 100 / KEY_SIZE);
	debug_console_printf(machine, "%4d/%4d keys don't matter (%d%%)\n", nomatter, KEY_SIZE, nomatter * 100 / KEY_SIZE);
}


/*-----------------------------------------------
    execute_fdstate - handle the 'fdstate'
    command
-----------------------------------------------*/

static void execute_fdstate(running_machine &machine, int ref, int params, const char **param)
{
	UINT64 newstate;

	/* set the new state if we got a parameter */
	if (params > 0)
	{
		if (!debug_command_parameter_number(machine, param[0], &newstate))
			return;
		fd1094_set_state(keyregion, newstate);
		fd1094_regenerate_key(machine);
		machine.debug_view().update_all(DVT_MEMORY);
		machine.debug_view().update_all(DVT_DISASSEMBLY);
	}

	/* 0 parameters displays the current state */
	debug_console_printf(machine, "FD1094 state = %X\n", fd1094_set_state(keyregion, -1));
}


/*-----------------------------------------------
    execute_fdpc - handle the 'fdpc'
    command
-----------------------------------------------*/

static void execute_fdpc(running_machine &machine, int ref, int params, const char **param)
{
	device_t *cpu = debug_cpu_get_visible_cpu(machine);
	UINT64 newpc;

	/* support 0 or 1 parameters */
	if (!debug_command_parameter_number(machine, param[0], &newpc))
		newpc = cpu->safe_pc();

	/* set the new PC */
	cpu->state().set_pc(newpc);

	/* recompute around that */
	instruction_hook(*cpu, newpc);
}


/*-----------------------------------------------
    execute_fdsearch - handle the 'fdsearch'
    command
-----------------------------------------------*/

static void execute_fdsearch(running_machine &machine, int ref, int params, const char **param)
{
	address_space &space = debug_cpu_get_visible_cpu(machine)->memory().space(AS_PROGRAM);
	int pc = space.device().safe_pc();
	int length, first = TRUE;
	UINT8 instrdata[2];
	UINT16 decoded;

	/* if we don't match, reset the stack */
	if (searchsp == 0 || searchstack[searchsp-1] != pc)
	{
		int pcaddr;
		debug_console_printf(machine, "Starting new search at PC=%06X\n", pc);
		searchsp = 0;
		for (pcaddr = 0; pcaddr < coderegion_words; pcaddr++)
			keystatus[pcaddr] &= ~SEARCH_MASK;
	}
	else
	{
		debug_console_printf(machine, "Resuming search at PC=%06X\n", pc);
		searchsp--;
	}

	/* loop while we don't need to break */
	while (1)
	{
		int newpc;

		/* for each PC after the first, do some extra work */
		if (!first)
		{
			/* if we've hit this PC already, stop and back off */
			while ((keystatus[pc/2] & SEARCH_MASK) != 0 && searchsp > 0)
				pc = searchstack[--searchsp];
			if ((keystatus[pc/2] & SEARCH_MASK) != 0)
			{
				debug_console_printf(machine, "Search stack exhausted\n");
				break;
			}

			/* set this as our current PC and run the instruction hook */
			space.device().state().set_pc(pc);
			if (instruction_hook(space.device(), pc))
				break;
		}
		keystatus[pc/2] |= SEARCH_MASK;
		first = FALSE;

		/* decode the first word */
		decoded = fd1094_decode(pc/2, coderegion[pc/2], keyregion, 0);
		instrdata[0] = decoded >> 8;
		instrdata[1] = decoded;

		/* get the opcode */
		length = validate_opcode(space, pc, instrdata, 1);
		if (length < 0)
			length = -length;
		if (length == 0)
		{
			debug_console_printf(machine, "Invalid opcode; unable to advance\n");
			break;
		}

		/* advance to the new PC */
		newpc = pc + length * 2;

		/* handle branches */
		if (optable[decoded].flags & OF_BRANCH)
		{
			int deltapc = (INT8)decoded;
			int targetpc;

			/* extract the delta PC */
			if ((optable[decoded].flags & OF_ISIZEMASK) == OF_IMMW)
				deltapc = (INT16)fd1094_decode((pc+2)/2, coderegion[(pc+2)/2], keyregion, 0);
			else if ((optable[decoded].flags & OF_ISIZEMASK) == OF_IMML)
				deltapc = (INT32)(fd1094_decode((pc+2)/2, coderegion[(pc+2)/2], keyregion, 0) << 16) + fd1094_decode((pc+4)/2, coderegion[(pc+4)/2], keyregion, 0);

			/* for everything but unconditional branches, push the target on the stack; else just go there */
			targetpc = (pc + 2 + deltapc) & 0xffffff;
			if ((decoded & 0xff00) != 0x6000)
				searchstack[searchsp++] = targetpc;
			else
				newpc = targetpc;
		}

		/* handle jumps */
		if (optable[decoded].flags & OF_JMP)
		{
			int targetpc;

			/* if we're not an absolute address, skip it */
			if ((decoded & 0x3e) != 0x38)
				continue;

			/* determine the target PC */
			if ((decoded & 0x3f) == 0x38)
				targetpc = (INT16)fd1094_decode((pc+2)/2, coderegion[(pc+2)/2], keyregion, 0);
			else
				targetpc = (INT32)(fd1094_decode((pc+2)/2, coderegion[(pc+2)/2], keyregion, 0) << 16) + fd1094_decode((pc+4)/2, coderegion[(pc+4)/2], keyregion, 0);

			/* for jsr's, add a stack entry to explore the destination; else just go there */
			if ((decoded & 0xffc0) == 0x4e80)
				searchstack[searchsp++] = targetpc;
			else
				newpc = targetpc;
		}

		/* if we hit RTS/RTE, stop here */
		if (decoded == 0x4e73 || decoded == 0x4e75)
			continue;

		/* set the new PC */
		pc = newpc;
	}

	/* push the current PC on the stack */
	searchstack[searchsp++] = pc;
}


/*-----------------------------------------------
    execute_fddasm - handle the 'fddasm'
    command
-----------------------------------------------*/

static void execute_fddasm(running_machine &machine, int ref, int params, const char **param)
{
	address_space &space = debug_cpu_get_visible_cpu(machine)->memory().space(AS_PROGRAM);
	int origstate = fd1094_set_state(keyregion, -1);
	const char *filename;
	int skipped = FALSE;
	UINT32 pcaddr;

	/* extract the parameters */
	filename = param[0];

	/* open the file */
	emu_file file(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE);
	file_error filerr = file.open(filename);
	if (filerr != FILERR_NONE)
	{
		debug_console_printf(machine, "Unable to create file '%s'\n", filename);
		return;
	}

	/* now do the disassembly */
	for (pcaddr = 0; pcaddr < coderegion_words; )
	{
		UINT8 instrbuffer[10];
		int unknowns = FALSE;
		int length, pcoffs;
		char disasm[256];
		UINT16 decoded;
		int pnum;

		/* if we haven't visited this word, go to the next */
		if ((keystatus[pcaddr] & STATE_MASK) == 0)
		{
			pcaddr++;
			skipped = TRUE;
			continue;
		}

		/* get the opcode */
		fd1094_set_state(keyregion, FD1094_STATE_RESET | (keystatus[pcaddr] >> 8));
		decoded = fd1094_decode(pcaddr, coderegion[pcaddr], keyregion, 0);
		length = optable[decoded].flags >> 28;
		if (optable[decoded].flags == OF_INVALID)
			length = 1;

		/* decode the remaining words */
		instrbuffer[0] = decoded >> 8;
		instrbuffer[1] = decoded;
		for (pcoffs = 1; pcoffs < length; pcoffs++)
		{
			if ((keystatus[pcaddr + pcoffs] & STATUS_MASK) == STATUS_UNVISITED)
			{
				pcaddr++;
				skipped = TRUE;
				continue;
			}
			decoded = fd1094_decode(pcaddr + pcoffs, coderegion[pcaddr + pcoffs], keyregion, 0);
			instrbuffer[pcoffs*2+0] = decoded >> 8;
			instrbuffer[pcoffs*2+1] = decoded;
		}

		/* disassemble the instruction */
		m68k_disassemble_raw(disasm, pcaddr * 2, instrbuffer, instrbuffer, M68K_CPU_TYPE_68000);

		/* print the line */
		if (skipped)
			file.printf("\n");
		skipped = FALSE;
		file.printf(" %02X %06X:", keystatus[pcaddr] >> 8, pcaddr * 2);
		for (pcoffs = 0; pcoffs < 5; pcoffs++)
		{
			if (pcoffs < length)
			{
				static const char statchar[] = "? =?";
				int keystat = keystatus[pcaddr + pcoffs] & STATUS_MASK;
				if (keystat != STATUS_LOCKED && keystat != STATUS_NOCHANGE)
					unknowns = TRUE;
				file.printf(" %02X%02X%c", instrbuffer[pcoffs*2+0], instrbuffer[pcoffs*2+1], statchar[keystat]);
			}
			else
				file.printf("      ");
		}
		file.printf("%s\n", disasm);

		/* if we have unknowns, display them as well */
		if (unknowns > 0)
		{
			UINT8 keybuffer[5];
			int posscount = try_all_possibilities(space, pcaddr * 2, 0, 0, instrbuffer, keybuffer, posslist) - posslist;
			for (pnum = 0; pnum < posscount; pnum++)
				if (strcmp(disasm, posslist[pnum].dasm) != 0)
				{
					file.printf("          :");
					for (pcoffs = 0; pcoffs < 5; pcoffs++)
						if (pcoffs < posslist[pnum].length)
							file.printf(" %02X%02X ", posslist[pnum].instrbuffer[pcoffs*2+0], posslist[pnum].instrbuffer[pcoffs*2+1]);
						else
							file.printf("      ");
					file.printf("%s\n", posslist[pnum].dasm);
				}
		}

		/* advance */
		pcaddr += length;
	}

	/* close the file */
	fd1094_set_state(keyregion, origstate);
}


/*-----------------------------------------------
    execute_fdcset - handle the 'fdcset'
    command
-----------------------------------------------*/

static void execute_fdcset(running_machine &machine, int ref, int params, const char **param)
{
	UINT64 pc, value, mask = 0xffff, state = FD1094_STATE_RESET;
	int cnum;

	/* extract the parameters */
	if (!debug_command_parameter_number(machine, param[0], &pc))
		return;
	if (!debug_command_parameter_number(machine, param[1], &value))
		return;
	if (params >= 3 && !debug_command_parameter_number(machine, param[2], &mask))
		return;
	if (params >= 4)
	{
		if (strcmp(param[3], "irq") == 0)
			state = FD1094_STATE_IRQ;
		else if (!debug_command_parameter_number(machine, param[3], &state))
			return;
	}

	/* validate parameters */
	if ((pc & 1) != 0 || pc > 0xffffff)
	{
		debug_console_printf(machine, "Invalid PC specified (%08X)\n", (UINT32)pc);
		return;
	}

	/* look for a match and remove any matching constraints */
	for (cnum = 0; cnum < constcount; cnum++)
	{
		/* insert ahead of later constraints */
		if (constraints[cnum].pc > pc)
		{
			memmove(&constraints[cnum + 1], &constraints[cnum], (constcount - cnum) * sizeof(constraints[0]));
			break;
		}

		/* replace matching constraints */
		else if (constraints[cnum].pc == pc)
			break;
	}

	/* set the new constraint and increase the count */
	if (cnum >= constcount || constraints[cnum].pc != pc)
		constcount++;
	set_constraint(&constraints[cnum], pc, state, value, mask);

	/* explain what we did */
	debug_console_printf(machine, "Set new constraint at PC=%06X, state=%03X: decrypted & %04X == %04X\n",
			(int)pc, (int)state, (int)mask, (int)value);
}


/*-----------------------------------------------
    execute_fdclist - handle the 'fdclist'
    command
-----------------------------------------------*/

static void execute_fdclist(running_machine &machine, int ref, int params, const char **param)
{
	int cnum;

	/* loop over constraints and print them */
	for (cnum = 0; cnum < constcount; cnum++)
	{
		fd1094_constraint *constraint = &constraints[cnum];
		debug_console_printf(machine, "  PC=%06X, state=%03X: decrypted & %04X == %04X\n",
				constraint->pc, constraint->state, constraint->mask, constraint->value);
	}
}


/*-----------------------------------------------
    execute_fdcsearch - handle the 'fdcsearch'
    command
-----------------------------------------------*/

static void execute_fdcsearch(running_machine &machine, int ref, int params, const char **param)
{
//  debug_console_printf(machine, "Searching for possible global keys....\n");
	perform_constrained_search(machine);
}


/*-----------------------------------------------
    try_all_possibilities - recursively try
    all possible values of the high bits of the
    key at the given address for the specified
    length
-----------------------------------------------*/

static fd1094_possibility *try_all_possibilities(address_space &space, int basepc, int offset, int length, UINT8 *instrbuffer, UINT8 *keybuffer, fd1094_possibility *possdata)
{
	UINT8 keymask, keystat;
	UINT16 possvalue[4];
	UINT8 posskey[4];
	int numposs = 0;
	int decoded;
	int keyaddr;
	int pcaddr;
	int hibit;
	int i;

	/* get the key address and mask */
	pcaddr = basepc/2 + offset;
	keyaddr = addr_to_keyaddr(pcaddr);
	keymask = mask_for_keyaddr(keyaddr);
	keystat = keystatus[pcaddr] & STATUS_MASK;

	/* if the status is 1 (locked) or 2 (doesn't matter), just take the current value */
	if (keystat == STATUS_LOCKED || keystat == STATUS_NOCHANGE)
	{
		posskey[numposs] = keyregion[keyaddr];
		possvalue[numposs++] = fd1094_decode(pcaddr, coderegion[pcaddr], keyregion, 0);
	}

	/* otherwise, iterate over high bits */
	else
	{
		/* remember the original key and iterate over high bits */
		UINT8 origkey = keyregion[keyaddr];
		for (hibit = 0x00; hibit < 0x100; hibit += 0x40)
			if ((hibit & keymask) == keymask)
			{
				/* set the key and decode this word */
				keyregion[keyaddr] = (origkey & ~HIBITS_MASK) | hibit;
				decoded = fd1094_decode(pcaddr, coderegion[pcaddr], keyregion, 0);

				/* see if we already got that value */
				for (i = 0; i < numposs; i++)
					if ((UINT16)decoded == possvalue[i])
						break;

				/* if not, add it to the list */
				if (i == numposs)
				{
					posskey[numposs] = keyregion[keyaddr];
					possvalue[numposs++] = decoded;
				}
			}

		/* restore the original key */
		keyregion[keyaddr] = origkey;

		/* if there was only one possibility, then mark it as "doesn't matter" */
		if (numposs == 1)
		{
			keystatus[pcaddr] = (keystatus[pcaddr] & ~STATUS_MASK) | STATUS_NOCHANGE;
			keydirty = TRUE;
		}
	}

	/* now iterate over our possible values */
	for (i = 0; i < numposs; i++)
	{
		/* set the instruction buffer */
		instrbuffer[offset*2 + 0] = possvalue[i] >> 8;
		instrbuffer[offset*2 + 1] = possvalue[i];
		keybuffer[offset] = posskey[i];

		/* if our length is 0, we need to do a quick dasm to see how long our length is */
		if (offset == 0)
		{
			/* first make sure we are a valid instruction */
			if ((possvalue[i] & 0xf000) == 0xa000 || (possvalue[i] & 0xf000) == 0xf000)
				continue;
			length = validate_opcode(space, basepc, instrbuffer, 1);
			if (length == 0)
				continue;
			if (length < 0)
				length = -length;
		}

		/* if we're not at our target length, recursively call ourselves */
		if (offset < length - 1)
			possdata = try_all_possibilities(space, basepc, offset + 1, length, instrbuffer, keybuffer, possdata);

		/* otherwise, output what we have */
		else
		{
			int tlen, inoffs;

			/* do the disassembly, and make sure we don't get an invalid result */
			m68k_disassemble_raw(possdata->dasm, basepc, instrbuffer, instrbuffer, M68K_CPU_TYPE_68000);

			/* validate the opcode */
			tlen = validate_opcode(space, basepc, instrbuffer, length);
			if (tlen == 0)
			{
				printf("Eliminated: %s [", possdata->dasm);
				for (inoffs = 0; inoffs < length; inoffs++)
					printf("%04X ", (instrbuffer[inoffs*2+0] << 8) | instrbuffer[inoffs*2+1]);
				printf("]\n");
				continue;
			}

			/* copy the rest of the data and increment the pointer */
			possdata->basepc = basepc;
			possdata->length = (tlen < 0) ? -tlen : tlen;
			possdata->iffy = (tlen < 0);
			memcpy(possdata->instrbuffer, instrbuffer, sizeof(possdata->instrbuffer));
			memcpy(possdata->keybuffer, keybuffer, sizeof(possdata->keybuffer));
			possdata++;
		}
	}

	return possdata;
}


/*-----------------------------------------------
    tag_possibility - tag a given possibility
    with the specified status
-----------------------------------------------*/

static void tag_possibility(running_machine &machine, fd1094_possibility *possdata, UINT8 status)
{
	int curfdstate = fd1094_set_state(keyregion, -1);
	int nomatter = 0, locked = 0, guessed = 0;
	int reps = keystatus_words / KEY_SIZE;
	UINT8 newstat[5];
	int pcoffs;

	/* determine the new status for each word */
	for (pcoffs = 0; pcoffs < possdata->length; pcoffs++)
	{
		int pnum;

		/* default to setting the requested status */
		newstat[pcoffs] = status;

		/* see if the current word was the same across all possibilities */
		for (pnum = 0; pnum < posscount; pnum++)
			if (posslist[pnum].instrbuffer[pcoffs*2+0] != possdata->instrbuffer[pcoffs*2+0] ||
				posslist[pnum].instrbuffer[pcoffs*2+1] != possdata->instrbuffer[pcoffs*2+1])
				break;

		/* if so, lock, don't guess */
		if (pnum == posscount)
			newstat[pcoffs] = STATUS_LOCKED;
	}

	/* iterate over words in the opcode */
	for (pcoffs = 0; pcoffs < possdata->length; pcoffs++)
	{
		int pcaddr = possdata->basepc/2 + pcoffs;
		int keyaddr = addr_to_keyaddr(pcaddr);
		int keystat = keystatus[pcaddr] & STATUS_MASK;
		int repnum;

		/* if the status doesn't match and isn't "no change", then set the status */
		if (keystat != STATUS_NOCHANGE)
		{
			keystatus[keyaddr] = (keystatus[keyaddr] & ~HIBITS_MASK) | (possdata->keybuffer[pcoffs] & HIBITS_MASK);
			keystatus[pcaddr] = (keystatus[pcaddr] & ~STATE_MASK & ~STATUS_MASK) | (curfdstate << 8) | newstat[pcoffs];
			keydirty = TRUE;
		}
		else
			keystatus[pcaddr] = (keystatus[pcaddr] & ~STATE_MASK) | (curfdstate << 8);

		/* if we're now locked, propogate across all reps */
		keystat = keystatus[pcaddr] & STATUS_MASK;
		if (keystat == STATUS_LOCKED)
			for (repnum = 0; repnum < reps; repnum++)
			{
				keystatus[repnum * KEY_SIZE + keyaddr] = (keystatus[repnum * KEY_SIZE + keyaddr] & ~STATUS_MASK) | STATUS_LOCKED;
				if ((keyaddr & 0x1ffc) == 0x1000)
					keystatus[repnum * KEY_SIZE + keyaddr - 0x1000] = (keystatus[repnum * KEY_SIZE + keyaddr - 0x1000] & ~STATUS_MASK) | STATUS_LOCKED;
			}

		/* update the final key status */
		if (keystat == STATUS_LOCKED)
			locked++;
		else if (keystat == STATUS_GUESS)
			guessed++;
		else if (keystat == STATUS_NOCHANGE)
			nomatter++;
	}

	debug_console_printf(machine, "PC=%06X: locked %d, guessed %d, nochange %d\n", possdata->basepc, locked, guessed, nomatter);
}


/*-----------------------------------------------
    perform_constrained_search - look for
    the next global key that will match the
    given sequence/mask pair
-----------------------------------------------*/

static void perform_constrained_search(running_machine &machine)
{
	UINT32 global;

	/* ensure our first 4 constraints are what we expect */
	assert(constraints[0].pc == 0x000000);
	assert(constraints[1].pc == 0x000002);
	assert(constraints[2].pc == 0x000004);
	assert(constraints[3].pc == 0x000006);

	/* start with a 0 global key and brute force from there */
	global = 0;

	/* loop until we run out of possibilities */
	while (1)
	{
		UINT16 output[4];
		int numseeds;

		/* look for the next global key match */
		global = find_global_key_matches(global + 1, output);
		if (global == 0)
			break;
//      debug_console_printf(machine, "Checking global key %08X (PC=%06X)....\n", global, (output[2] << 16) | output[3]);

		/* use the IRQ handler to find more possibilities */
		numseeds = find_constraint_sequence(global, FALSE);
		if (numseeds > 0)
		{
			int i;
			for (i = 0; i < numseeds; i++)
				debug_console_printf(machine, "  Possible: global=%08X seed=%06X pc=%04X\n", global, possible_seed[i], output[3]);
		}
	}
}


/*-----------------------------------------------
    find_global_key_matches - look for
    the next global key that will match the
    given sequence/mask pair
-----------------------------------------------*/

static UINT32 find_global_key_matches(UINT32 startwith, UINT16 *output)
{
	int key0, key1, key2, key3;
	UINT8 key[4];

	/* iterate over the first key byte, allowing all possible values */
	for (key0 = (startwith >> 24) & 0xff; key0 < 256; key0++)
	{
		/* set the key and reset the fd1094 */
		key[0] = key0;
		startwith &= 0x00ffffff;
		fd1094_set_state(key, FD1094_STATE_RESET);

		/* if we match, iterate over the second key byte */
		output[0] = fd1094_decode(0x000000, coderegion[0], key, TRUE);
		if ((output[0] & constraints[0].mask) == constraints[0].value)

			/* iterate over the second key byte, limiting the scope to known valid keys */
			for (key1 = (startwith >> 16) & 0xff; key1 < 256; key1++)
				if ((key1 & 0xf8) == 0xa8 || (key1 & 0xf8) == 0xf8)
				{
					/* set the key and reset the fd1094 */
					key[1] = key1;
					startwith &= 0x0000ffff;
					fd1094_set_state(key, FD1094_STATE_RESET);

					/* if we match, iterate over the third key byte */
					output[1] = fd1094_decode(0x000001, coderegion[1], key, TRUE);
					if ((output[1] & constraints[1].mask) == constraints[1].value)

						/* iterate over the third key byte, limiting the scope to known valid keys */
						for (key2 = (startwith >> 8) & 0xff; key2 < 256; key2++)
							if ((key2 & 0xc0) == 0xc0)
							{
								/* set the key and reset the fd1094 */
								key[2] = key2;
								startwith &= 0x000000ff;
								fd1094_set_state(key, FD1094_STATE_RESET);

								/* if we match, iterate over the fourth key byte */
								output[2] = fd1094_decode(0x000002, coderegion[2], key, TRUE);
								if ((output[2] & constraints[2].mask) == constraints[2].value)

									/* iterate over the fourth key byte, limiting the scope to known valid keys */
									for (key3 = (startwith >> 0) & 0xff; key3 < 256; key3++)
										if ((key3 & 0xc0) == 0xc0)
										{
											/* set the key and reset the fd1094 */
											key[3] = key3;
											startwith = 0;
											fd1094_set_state(key, FD1094_STATE_RESET);

											/* if we match, return the value */
											output[3] = fd1094_decode(0x000003, coderegion[3], key, TRUE);
											if ((output[3] & constraints[3].mask) == constraints[3].value)
												return (key0 << 24) | (key1 << 16) | (key2 << 8) | key3;
										}
							}
				}
	}
	return 0;
}


/*-----------------------------------------------
    find_constraint_sequence - look for a
    sequence of decoded words at the given
    address, and optionally verify that there
    are valid PRNG keys that could generate the
    results
-----------------------------------------------*/

static int find_constraint_sequence(UINT32 global, int quick)
{
	const fd1094_constraint *minkeyaddr = &constraints[4];
	const fd1094_constraint *maxkeyaddr = &constraints[4];
	const fd1094_constraint *curr;
	int keyvalue, keyaddr, keysneeded;
	int seedcount = 0;
	UINT16 decrypted;
	UINT8 key[8192];
	UINT8 keymask;
	offs_t pcaddr;

	/* if we don't have any extra constraints, we're good */
	if (constcount <= 4)
		return -1;

	/* set the global key */
	key[0] = global >> 24;
	key[1] = global >> 16;
	key[2] = global >> 8;
	key[3] = global >> 0;
	fd1094_set_state(key, -1);

	/* first see if it is even possible, regardless of PRNG */
	for (curr = &constraints[4]; curr < &constraints[constcount]; curr++)
	{
		/* get the key address and value for this offset */
		pcaddr = curr->pc / 2;
		keyaddr = addr_to_keyaddr(pcaddr);
		keymask = mask_for_keyaddr(keyaddr);

		/* track the minumum and maximum key addresses, but only for interesting combinations */
		if ((coderegion[pcaddr] & 0xe000) != 0x0000)
		{
			if (keyaddr < addr_to_keyaddr(minkeyaddr->pc / 2))
				minkeyaddr = curr;
			if (keyaddr > addr_to_keyaddr(maxkeyaddr->pc / 2))
				maxkeyaddr = curr;
		}

		/* set the state */
		fd1094_set_state(key, curr->state);

		/* brute force search this byte */
		for (keyvalue = 0; keyvalue < 256; keyvalue++)
			if ((keyvalue & keymask) == keymask)
			{
				/* see if this works */
				key[keyaddr] = keyvalue;
				decrypted = fd1094_decode(pcaddr, coderegion[pcaddr], key, FALSE);

				/* if we got a match, stop; we're done */
				if ((decrypted & curr->mask) == curr->value)
					break;
			}

		/* if we failed, we're done */
		if (keyvalue == 256)
			return 0;
	}

	/* if we're quick, that's all the checking we do */
	if (quick)
		return -1;

	/* determine how many keys we need to cover our whole range */
	keysneeded = addr_to_keyaddr(maxkeyaddr->pc / 2) + 1 - addr_to_keyaddr(minkeyaddr->pc / 2);

	/* now do the more thorough search */
	pcaddr = minkeyaddr->pc / 2;
	keyaddr = addr_to_keyaddr(pcaddr);
	keymask = mask_for_keyaddr(keyaddr);

	/* set the state */
	fd1094_set_state(key, minkeyaddr->state);

	/* brute force search the first byte key of the key */
	for (keyvalue = 0; keyvalue < 256; keyvalue++)
		if ((keyvalue & keymask) == keymask)
		{
			/* see if this works */
			key[keyaddr] = keyvalue;
			decrypted = fd1094_decode(pcaddr, coderegion[pcaddr], key, FALSE);

			/* if we got a match, then iterate over all possible PRNG sequences starting with this */
			if ((decrypted & minkeyaddr->mask) == minkeyaddr->value)
			{
				UINT32 seedlow;

//              debug_console_printf(machine, "Global %08X ... Looking for keys that generate a keyvalue of %02X at %04X\n",
//                      global, keyvalue, keyaddr);

				/* iterate over seed possibilities */
				for (seedlow = 0; seedlow < (1 << 16); seedlow++)
				{
					/* start with the known upper bits together with the 16 guessed lower bits */
					UINT32 seedstart = (~keyvalue << 16) | seedlow;

					/* generate data starting with this seed into the key */
					generate_key_bytes(key, keyaddr + 1, keysneeded - 1, seedstart);

					/* if the whole thing matched, record the match */
					if (does_key_work_for_constraints(coderegion, key))
					{
						seedstart = reconstruct_base_seed(keyaddr, seedstart);
						if ((seedstart & 0x3fffff) != 0)
							possible_seed[seedcount++] = seedstart;
					}
				}
			}
		}

	return seedcount;
}


/*-----------------------------------------------
    does_key_work_for_constraints - return true
    if the given key might work for a given set
    of constraints
-----------------------------------------------*/

static int does_key_work_for_constraints(const UINT16 *base, UINT8 *key)
{
	const fd1094_constraint *curr;
	UINT16 decrypted;

	/* iterate over the sequence */
	for (curr = &constraints[4]; curr < &constraints[constcount]; curr++)
	{
		offs_t pcaddr = curr->pc / 2;
		int keyaddr = addr_to_keyaddr(pcaddr);
		UINT8 keymask = mask_for_keyaddr(keyaddr);
		int hibits;

		/* set the state */
		fd1094_set_state(key, curr->state);

		/* iterate over high bits (1 per byte) */
		for (hibits = 0; hibits < 0x100; hibits += 0x40)
			if ((hibits & keymask) == keymask)
			{
				/* update the key bits */
				key[keyaddr] = (key[keyaddr] & ~0xc0) | hibits;

				/* decrypt using this key; stop if we get a match */
				decrypted = fd1094_decode(pcaddr, base[pcaddr], key, FALSE);
				if ((decrypted & curr->mask) == curr->value)
					break;
			}

		/* if we failed to match, we're done */
		if (hibits >= 0x100)
			return FALSE;
	}

	/* got a match on all entries */
	return TRUE;
}


/*-----------------------------------------------
    reconstruct_base_seed - given the seed
    value at a particular key address, return
    the seed that would be used to generate the
    first key value (at offset 4)
-----------------------------------------------*/

static UINT32 reconstruct_base_seed(int keybaseaddr, UINT32 startseed)
{
	UINT32 seed = startseed;
	UINT32 window[8192];
	int index = 0;

	/* keep generating, starting from the start seed until we re-generate the start seed */
	/* note that some sequences are smaller than the window, so we also have to ensure */
	/* that we generate at least one full window's worth of data */
	do
	{
		seed = seed * 0x29;
		seed += seed << 16;
		window[index++ % ARRAY_LENGTH(window)] = seed;
	} while (((startseed ^ seed) & 0x3fffff) != 0 || index < ARRAY_LENGTH(window));

	/* when we break, we have overshot */
	index--;

	/* back up to where we would have been at address 3 */
	index -= keybaseaddr - 3;
	if (index < 0)
		index += ARRAY_LENGTH(window);

	/* return the value from the window at that location */
	return window[index % ARRAY_LENGTH(window)] & 0x3fffff;
}


/*-----------------------------------------------
    Table of opcode parameters
-----------------------------------------------*/

#define ENTRY(a,b,c,d)      { #a, #b, c, d },

static const struct
{
	const char *    bitstring;
	const char *    eastring;
	UINT32          flags;
	const char *    instring;
} instr_table[] =
{
	ENTRY(1100...100000..., ........., OF_BYTE | OF_RARE, "ABCD Dn,Dm")
	ENTRY(1100...100001..., ........., OF_BYTE | OF_RARE, "ABCD -(An),-(Am)")
	ENTRY(1101...000......, d.A+-DBIP, OF_BYTE | OF_EASRC, "ADD.B <ea>,Dn")
	ENTRY(1101...001......, daA+-DBIP, OF_WORD | OF_EASRC, "ADD.W <ea>,Dn")
	ENTRY(1101...010......, daA+-DBIP, OF_LONG | OF_EASRC, "ADD.L <ea>,Dn")
	ENTRY(1101...011......, daA+-DBIP, OF_WORD | OF_EASRC, "ADDA.W <ea>,An")
	ENTRY(1101...100......, ..A+-DB.., OF_BYTE | OF_EADST, "ADD.B Dn,<ea>")
	ENTRY(1101...101......, ..A+-DB.., OF_WORD | OF_EADST, "ADD.W Dn,<ea>")
	ENTRY(1101...110......, ..A+-DB.., OF_LONG | OF_EADST, "ADD.L Dn,<ea>")
	ENTRY(1101...111......, daA+-DBIP, OF_LONG | OF_EASRC, "ADDA.L <ea>,An")
	ENTRY(0000011000......, d.A+-DB.., OF_BYTE | OF_EADST | OF_IMMB, "ADDI.B #x,<ea>")
	ENTRY(0000011001......, d.A+-DB.., OF_WORD | OF_EADST | OF_IMMW, "ADDI.W #x,<ea>")
	ENTRY(0000011010......, d.A+-DB.., OF_LONG | OF_EADST | OF_IMML, "ADDI.L #x,<ea>")
	ENTRY(0101...000......, d.A+-DB.., OF_BYTE | OF_EADST, "ADDQ.B #x,<ea>")
	ENTRY(0101...001......, daA+-DB.., OF_WORD | OF_EADST, "ADDQ.W #x,<ea>")
	ENTRY(0101...010......, daA+-DB.., OF_LONG | OF_EADST, "ADDQ.L #x,<ea>")
	ENTRY(1101...10000...., ........., OF_BYTE | OF_RARE, "ADDX.B")
	ENTRY(1101...10100...., ........., OF_WORD | OF_RARE, "ADDX.W")
	ENTRY(1101...11000...., ........., OF_LONG | OF_RARE, "ADDX.L")
	ENTRY(1100...000......, d.A+-DBIP, OF_BYTE | OF_EASRC, "AND.B <ea>,Dn")
	ENTRY(1100...001......, d.A+-DBIP, OF_WORD | OF_EASRC, "AND.W <ea>,Dn")
	ENTRY(1100...010......, d.A+-DBIP, OF_LONG | OF_EASRC, "AND.L <ea>,Dn")
	ENTRY(1100...100......, ..A+-DB.., OF_BYTE | OF_EADST, "AND.B Dn,<ea>")
	ENTRY(1100...101......, ..A+-DB.., OF_WORD | OF_EADST, "AND.W Dn,<ea>")
	ENTRY(1100...110......, ..A+-DB.., OF_LONG | OF_EADST, "AND.L Dn,<ea>")
	ENTRY(0000001000111100, ........., OF_BYTE | OF_IMMB | OF_RARE, "ANDI #x,CCR")
	ENTRY(0000001000......, d.A+-DB.., OF_BYTE | OF_EADST | OF_IMMB, "ANDI.B #x,<ea>")
	ENTRY(0000001001......, d.A+-DB.., OF_WORD | OF_EADST | OF_IMMW, "ANDI.W #x,<ea>")
	ENTRY(0000001010......, d.A+-DB.., OF_LONG | OF_EADST | OF_IMML, "ANDI.L #x,<ea>")
	ENTRY(1110....00.00..., ........., OF_BYTE, "ASL/ASR.B")
	ENTRY(1110....01.00..., ........., OF_WORD, "ASL/ASR.W")
	ENTRY(1110....10.00..., ........., OF_LONG, "ASL/ASR.L")
	ENTRY(1110000.11......, ..A+-DB.., OF_WORD | OF_EADST, "ASL/ASR.W <ea>")
	ENTRY(0110000000000000, ........., OF_WORD | OF_IMMW | OF_BRANCH, "BRA.W <dst>")
	ENTRY(01100000.......0, ........., OF_BYTE | OF_BRANCH, "BRA.B <dst>")
	ENTRY(0110000100000000, ........., OF_WORD | OF_IMMW | OF_BRANCH, "BSR.W <dst>")
	ENTRY(01100001.......0, ........., OF_BYTE | OF_BRANCH, "BSR.B <dst>")
	ENTRY(0110....00000000, ........., OF_WORD | OF_IMMW | OF_BRANCH, "Bcc.W <dst>")
	ENTRY(0110...........0, ........., OF_BYTE | OF_BRANCH, "Bcc.B <dst>")
	ENTRY(0000...101......, d.A+-DB.., OF_BYTE | OF_EADST, "BCHG Dn,<ea>")
	ENTRY(0000100001......, d.A+-DB.., OF_BYTE | OF_EADST | OF_IMMBIT, "BCHG #x,<ea>")
	ENTRY(0000...110......, d.A+-DB.., OF_BYTE | OF_EADST, "BCLR Dn,<ea>")
	ENTRY(0000100010......, d.A+-DB.., OF_BYTE | OF_EADST | OF_IMMBIT, "BCLR #x,<ea>")
	ENTRY(0000...111......, d.A+-DB.., OF_BYTE | OF_EADST, "BSET Dn,<ea>")
	ENTRY(0000100011......, d.A+-DB.., OF_BYTE | OF_EADST | OF_IMMBIT, "BSET #x,<ea>")
	ENTRY(0000...100......, d.A+-DBIP, OF_BYTE | OF_EADST, "BTST Dn,<ea>")
	ENTRY(0000100000......, d.A+-DB.P, OF_BYTE | OF_EADST | OF_IMMBIT, "BTST #x,<ea>")
	ENTRY(0100...110......, d.A+-DBIP, OF_WORD | OF_EADST | OF_RARE, "CHK.W <ea>,Dn")
	ENTRY(0100001000......, d.A+-DB.., OF_BYTE | OF_EADST, "CLR.B <ea>")
	ENTRY(0100001001......, d.A+-DB.., OF_WORD | OF_EADST, "CLR.W <ea>")
	ENTRY(0100001010......, d.A+-DB.., OF_LONG | OF_EADST, "CLR.L <ea>")
	ENTRY(1011...000......, d.A+-DBIP, OF_BYTE | OF_EASRC, "CMP.B <ea>,Dn")
	ENTRY(1011...001......, daA+-DBIP, OF_WORD | OF_EASRC, "CMP.W <ea>,Dn")
	ENTRY(1011...010......, daA+-DBIP, OF_LONG | OF_EASRC, "CMP.L <ea>,Dn")
	ENTRY(1011...011......, daA+-DBIP, OF_WORD | OF_EASRC, "CMPA.W <ea>,Dn")
	ENTRY(1011...111......, daA+-DBIP, OF_LONG | OF_EASRC, "CMPA.L <ea>,Dn")
	ENTRY(0000110000......, d.A+-DB.., OF_BYTE | OF_EASRC | OF_IMMB, "CMPI.B #x,<ea>")
	ENTRY(0000110001......, d.A+-DB.., OF_WORD | OF_EASRC | OF_IMMW, "CMPI.W #x,<ea>")
	ENTRY(0000110010......, d.A+-DB.., OF_LONG | OF_EASRC | OF_IMML, "CMPI.L #x,<ea>")
	ENTRY(1011...100001..., ........., OF_BYTE | OF_RARE, "CMPM.B")
	ENTRY(1011...101001..., ........., OF_WORD | OF_RARE, "CMPM.W")
	ENTRY(1011...110001..., ........., OF_LONG | OF_RARE, "CMPM.L")
	ENTRY(0101....11001..., ........., OF_WORD | OF_IMMW | OF_BRANCH, "DBcc.W <dst>")
	ENTRY(1000...111......, d.A+-DBIP, OF_WORD | OF_EASRC, "DIVS.W <ea>,Dn")
	ENTRY(1000...011......, d.A+-DBIP, OF_WORD | OF_EASRC, "DIVU.W <ea>,Dn")
	ENTRY(1011...100......, d.A+-DB.., OF_BYTE | OF_EADST, "EOR.B Dn,<ea>")
	ENTRY(1011...101......, d.A+-DB.., OF_WORD | OF_EADST, "EOR.W Dn,<ea>")
	ENTRY(1011...110......, d.A+-DB.., OF_LONG | OF_EADST, "EOR.L Dn,<ea>")
	ENTRY(0000101000111100, ........., OF_BYTE | OF_IMMB | OF_RARE, "EORI #x,CCR")
	ENTRY(0000101000......, d.A+-DB.., OF_BYTE | OF_EADST | OF_IMMB, "EORI.B #x,<ea>")
	ENTRY(0000101001......, d.A+-DB.., OF_WORD | OF_EADST | OF_IMMW, "EORI.W #x,<ea>")
	ENTRY(0000101010......, d.A+-DB.., OF_LONG | OF_EADST | OF_IMML, "EORI.L #x,<ea>")
	ENTRY(1100...101000..., ........., OF_LONG, "EXG Dn,Dn")
	ENTRY(1100...101001..., ........., OF_LONG, "EXG An,An")
	ENTRY(1100...110001..., ........., OF_LONG, "EXG Dn,An")
	ENTRY(0100100010000..., ........., OF_WORD, "EXT.W Dn")
	ENTRY(0100100011000..., ........., OF_WORD, "EXT.L Dn")
	ENTRY(0100111011......, ..A..DB.P, OF_WORD | OF_EASRC | OF_JMP, "JMP <ea>")
	ENTRY(0100111010......, ..A..DB.P, OF_WORD | OF_EASRC | OF_JMP, "JSR <ea>")
	ENTRY(0100...111......, ..A..DB.P, OF_BYTE | OF_EASRC, "LEA <ea>,An")
	ENTRY(0100111001010..., ........., OF_WORD | OF_IMMW | OF_RARE, "LINK An,#x")
	ENTRY(1110....00.01..., ........., OF_BYTE, "LSL/LSR.B Dn")
	ENTRY(1110....01.01..., ........., OF_WORD, "LSL/LSR.W Dn")
	ENTRY(1110....10.01..., ........., OF_LONG, "LSL/LSR.L Dn")
	ENTRY(1110001.11......, ..A+-DB.., OF_WORD | OF_EADST, "LSL/LSR.W <ea>")
	ENTRY(0001............, d.A+-DBIP, OF_BYTE | OF_EASRC | OF_MOVE, "MOVE.B <ea>,<ea>")
	ENTRY(0011............, daA+-DBIP, OF_WORD | OF_EASRC | OF_MOVE, "MOVE.W <ea>,<ea>")
	ENTRY(0010............, daA+-DBIP, OF_LONG | OF_EASRC | OF_MOVE, "MOVE.L <ea>,<ea>")
	ENTRY(0011...001......, daA+-DBIP, OF_WORD | OF_EASRC, "MOVEA.W <ea>,An")
	ENTRY(0010...001......, daA+-DBIP, OF_LONG | OF_EASRC, "MOVEA.L <ea>,An")
	ENTRY(0100010011......, d.A+-DBIP, OF_WORD | OF_EASRC | OF_RARE, "MOVE <ea>,CCR")
	ENTRY(0100000011......, d.A+-DB.., OF_WORD | OF_EADST | OF_RARE, "MOVE SR,<ea>")
	ENTRY(0100100010......, ..A.-DB.., OF_WORD | OF_EADST | OF_IMMW, "MOVEM.W <regs>,<ea>")
	ENTRY(0100100011......, ..A.-DB.., OF_LONG | OF_EADST | OF_IMMW, "MOVEM.L <regs>,<ea>")
	ENTRY(0100110010......, ..A+.DB.P, OF_WORD | OF_EASRC | OF_IMMW, "MOVEM.W <ea>,<regs>")
	ENTRY(0100110011......, ..A+.DB.P, OF_LONG | OF_EASRC | OF_IMMW, "MOVEM.L <ea>,<regs>")
	ENTRY(0000...100001..., ........., OF_WORD | OF_IMMW | OF_RARE, "MOVEP.W (d16,Ay),Dn")
	ENTRY(0000...101001..., ........., OF_LONG | OF_IMMW | OF_RARE, "MOVEP.L (d16,Ay),Dn")
	ENTRY(0000...110001..., ........., OF_WORD | OF_IMMW | OF_RARE, "MOVEP.W Dn,(d16,Ay)")
	ENTRY(0000...111001..., ........., OF_LONG | OF_IMMW | OF_RARE, "MOVEP.L Dn,(d16,Ay)")
	ENTRY(0111...0........, ........., OF_LONG, "MOVEQ #x,Dn")
	ENTRY(1100...111......, d.A+-DBIP, OF_WORD | OF_EASRC, "MULS.W <ea>,Dn")
	ENTRY(1100...011......, d.A+-DBIP, OF_WORD | OF_EASRC, "MULU.W <ea>,Dn")
	ENTRY(0100100000......, d.A+-DB.., OF_BYTE | OF_EADST | OF_RARE, "NBCD <ea>")
	ENTRY(0100010000......, d.A+-DB.., OF_BYTE | OF_EADST, "NEG.B <ea>")
	ENTRY(0100010001......, d.A+-DB.., OF_WORD | OF_EADST, "NEG.W <ea>")
	ENTRY(0100010010......, d.A+-DB.., OF_LONG | OF_EADST, "NEG.L <ea>")
	ENTRY(0100000000......, d.A+-DB.., OF_BYTE | OF_EADST | OF_RARE, "NEGX.B <ea>")
	ENTRY(0100000001......, d.A+-DB.., OF_WORD | OF_EADST | OF_RARE, "NEGX.W <ea>")
	ENTRY(0100000010......, d.A+-DB.., OF_LONG | OF_EADST | OF_RARE, "NEGX.L <ea>")
	ENTRY(0100111001110001, ........., 0, "NOP")
	ENTRY(0100011000......, d.A+-DB.., OF_BYTE | OF_EADST, "NOT.B <ea>")
	ENTRY(0100011001......, d.A+-DB.., OF_WORD | OF_EADST, "NOT.W <ea>")
	ENTRY(0100011010......, d.A+-DB.., OF_LONG | OF_EADST, "NOT.L <ea>")
	ENTRY(1000...000......, d.A+-DBIP, OF_BYTE | OF_EASRC, "OR.B <ea>,Dn")
	ENTRY(1000...001......, d.A+-DBIP, OF_WORD | OF_EASRC, "OR.W <ea>,Dn")
	ENTRY(1000...010......, d.A+-DBIP, OF_LONG | OF_EASRC, "OR.L <ea>,Dn")
	ENTRY(1000...100......, ..A+-DB.., OF_BYTE | OF_EADST, "OR.B Dn,<ea>")
	ENTRY(1000...101......, ..A+-DB.., OF_WORD | OF_EADST, "OR.W Dn,<ea>")
	ENTRY(1000...110......, ..A+-DB.., OF_LONG | OF_EADST, "OR.L Dn,<ea>")
	ENTRY(0000000000111100, ........., OF_BYTE | OF_IMMB | OF_RARE, "ORI #x,CCR")
	ENTRY(0000000000......, d.A+-DB.., OF_BYTE | OF_EADST | OF_IMMB, "ORI.B #x,<ea>")
	ENTRY(0000000001......, d.A+-DB.., OF_WORD | OF_EADST | OF_IMMW, "ORI.W #x,<ea>")
	ENTRY(0000000010......, d.A+-DB.., OF_LONG | OF_EADST | OF_IMML, "ORI.L #x,<ea>")
	ENTRY(0100100001......, ..A..DB.P, OF_BYTE | OF_EADST | OF_RARE, "PEA <ea>")
	ENTRY(1110....00.11..., ........., OF_BYTE, "ROL/ROR.B Dn")
	ENTRY(1110....01.11..., ........., OF_WORD, "ROL/ROR.W Dn")
	ENTRY(1110....10.11..., ........., OF_LONG, "ROL/ROR.L Dn")
	ENTRY(1110011.11......, ..A+-DB.., OF_WORD | OF_EADST, "ROL/ROR.W <ea>")
	ENTRY(1110....00.10..., ........., OF_BYTE | OF_RARE, "ROXL/ROXR.B Dn")
	ENTRY(1110....01.10..., ........., OF_WORD | OF_RARE, "ROXL/ROXR.W Dn")
	ENTRY(1110....10.10..., ........., OF_LONG | OF_RARE, "ROXL/ROXR.L Dn")
	ENTRY(1110010.11......, ..A+-DB.., OF_WORD | OF_EADST | OF_RARE, "ROXL/ROXR.W <ea>")
	ENTRY(0100111001110111, ........., OF_RARE, "RTR")
	ENTRY(0100111001110101, ........., OF_RARE, "RTS")
	ENTRY(1000...100000..., ........., OF_BYTE | OF_RARE, "SBCD Dn,Dm")
	ENTRY(1000...100001..., ........., OF_BYTE | OF_RARE, "SBCD -(An),-(Am)")
	ENTRY(0101....11......, d.A+-DB.., OF_BYTE | OF_EADST | OF_RARE, "Scc <ea>")
	ENTRY(1001...000......, d.A+-DBIP, OF_BYTE | OF_EASRC, "SUB.B <ea>,Dn")
	ENTRY(1001...001......, daA+-DBIP, OF_WORD | OF_EASRC, "SUB.W <ea>,Dn")
	ENTRY(1001...010......, daA+-DBIP, OF_LONG | OF_EASRC, "SUB.L <ea>,Dn")
	ENTRY(1001...011......, daA+-DBIP, OF_WORD | OF_EASRC, "SUBA.W <ea>,An")
	ENTRY(1001...100......, ..A+-DB.., OF_BYTE | OF_EADST, "SUB.B Dn,<ea>")
	ENTRY(1001...101......, ..A+-DB.., OF_WORD | OF_EADST, "SUB.W Dn,<ea>")
	ENTRY(1001...110......, ..A+-DB.., OF_LONG | OF_EADST, "SUB.L Dn,<ea>")
	ENTRY(1001...111......, daA+-DBIP, OF_LONG | OF_EASRC, "SUBA.L <ea>,An")
	ENTRY(0000010000......, d.A+-DB.., OF_BYTE | OF_EADST | OF_IMMB, "SUBI.B #x,<ea>")
	ENTRY(0000010001......, d.A+-DB.., OF_WORD | OF_EADST | OF_IMMW, "SUBI.W #x,<ea>")
	ENTRY(0000010010......, d.A+-DB.., OF_LONG | OF_EADST | OF_IMML, "SUBI.L #x,<ea>")
	ENTRY(0101...100......, d.A+-DB.., OF_BYTE | OF_EADST, "SUBQ.B #x,<ea>")
	ENTRY(0101...101......, daA+-DB.., OF_WORD | OF_EADST, "SUBQ.W #x,<ea>")
	ENTRY(0101...110......, daA+-DB.., OF_LONG | OF_EADST, "SUBQ.L #x,<ea>")
	ENTRY(1001...10000...., ........., OF_BYTE | OF_RARE, "SUBX.B")
	ENTRY(1001...10100...., ........., OF_WORD | OF_RARE, "SUBX.W")
	ENTRY(1001...11000...., ........., OF_LONG | OF_RARE, "SUBX.L")
	ENTRY(0100100001000..., ........., OF_LONG | OF_RARE, "SWAP Dn")
	ENTRY(0100101011......, d.A+-DB.., OF_BYTE | OF_EASRC | OF_RARE, "TAS <ea>")
	ENTRY(010011100100...., ........., OF_RARE, "TRAP #x")
	ENTRY(0100111001110110, ........., OF_RARE, "TRAPV")
	ENTRY(0100101000......, d.A+-DB.., OF_BYTE | OF_EASRC, "TST.B <ea>")
	ENTRY(0100101001......, d.A+-DB.., OF_WORD | OF_EASRC, "TST.W <ea>")
	ENTRY(0100101010......, d.A+-DB.., OF_LONG | OF_EASRC, "TST.L <ea>")
	ENTRY(0100111001011..., ........., OF_RARE, "UNLK")
	ENTRY(0000001001111100, ........., OF_WORD | OF_IMMW | OF_RARE, "ANDI #x,SR")
	ENTRY(0000101001111100, ........., OF_WORD | OF_IMMW | OF_RARE, "EORI #x,SR")
	ENTRY(0100000011......, d.A+-DB.., OF_WORD | OF_EADST | OF_RARE, "MOVE SR,<ea>")
	ENTRY(0100011011......, d.A+-DBIP, OF_WORD | OF_EASRC | OF_RARE, "MOVE <ea>,SR")
	ENTRY(010011100110...., ........., OF_LONG | OF_RARE, "MOVE USP")
	ENTRY(0000000001111100, ........., OF_WORD | OF_IMMW | OF_RARE, "ORI #x,SR")
	ENTRY(0100111001110000, ........., OF_RARE, "RESET")
	ENTRY(0100111001110011, ........., OF_RARE, "RTE")
	ENTRY(0100111001110010, ........., OF_WORD | OF_IMMW | OF_RARE, "STOP #x")
};


/*-----------------------------------------------
    build_optable - build up the opcode table
-----------------------------------------------*/

static void build_optable(running_machine &machine)
{
	int opnum, inum;

	/* allocate and initialize the opcode table */
	optable = auto_alloc_array(machine, optable_entry, 65536);
	for (opnum = 0; opnum < 65536; opnum++)
	{
		optable[opnum].flags = OF_INVALID;
		optable[opnum].string = NULL;
	}

	/* now iterate over entries in our intruction table */
	for (inum = 0; inum < ARRAY_LENGTH(instr_table); inum++)
	{
		const char *bitstring = instr_table[inum].bitstring;
		const char *eastring = instr_table[inum].eastring;
		const char *instring = instr_table[inum].instring;
		UINT32 flags = instr_table[inum].flags;
		UINT8 ea_allowed[64], ea2_allowed[64];
		int bitnum, step, eanum, ea2num;
		UINT16 mask = 0, value = 0;

		/* build up the mask and value from the bitstring */
		for (bitnum = 0; bitnum < 16; bitnum++)
		{
			assert(bitstring[bitnum] == '0' || bitstring[bitnum] == '1' || bitstring[bitnum] == '.');
			mask <<= 1;
			value <<= 1;
			if (bitstring[bitnum] != '.')
			{
				mask |= 1;
				value |= (bitstring[bitnum] == '1');
			}
		}

		/* if we have an EA, fill in the EA bits */
		memset(ea_allowed, 0, sizeof(ea_allowed));
		if (flags & (OF_EASRC | OF_EADST))
		{
			assert((mask & 0x003f) == 0);
			assert(eastring[0] == 'd' || eastring[0] == '.');
			if (eastring[0] == 'd') memset(&ea_allowed[0x00], 1, 8);
			assert(eastring[1] == 'a' || eastring[1] == '.');
			if (eastring[1] == 'a') memset(&ea_allowed[0x08], 1, 8);
			assert(eastring[2] == 'A' || eastring[2] == '.');
			if (eastring[2] == 'A') memset(&ea_allowed[0x10], 1, 8);
			assert(eastring[3] == '+' || eastring[3] == '.');
			if (eastring[3] == '+') memset(&ea_allowed[0x18], 1, 8);
			assert(eastring[4] == '-' || eastring[4] == '.');
			if (eastring[4] == '-') memset(&ea_allowed[0x20], 1, 8);
			assert(eastring[5] == 'D' || eastring[5] == '.');
			if (eastring[5] == 'D') memset(&ea_allowed[0x28], 1, 16);
			assert(eastring[6] == 'B' || eastring[6] == '.');
			if (eastring[6] == 'B') memset(&ea_allowed[0x38], 1, 2);
			assert(eastring[7] == 'I' || eastring[7] == '.');
			if (eastring[7] == 'I') ea_allowed[0x3c] = 1;
			assert(eastring[8] == 'P' || eastring[8] == '.');
			if (eastring[8] == 'P') memset(&ea_allowed[0x3a], 1, 2);
			step = 0x40;
		}
		else
		{
			assert(strcmp(eastring, ".........") == 0);
			ea_allowed[0] = 1;
			step = 1;
		}

		/* if we're a move instruction, fill in the EA2 bits */
		memset(ea2_allowed, 0, sizeof(ea2_allowed));
		if (flags & OF_MOVE)
		{
			assert((mask & 0x0fc0) == 0);
			memset(&ea2_allowed[0x00], 1, 8);
			memset(&ea2_allowed[0x10], 1, 42);
			step = 0x1000;
		}
		else
			ea2_allowed[0] = 1;

		/* iterate over allowed EAs and fill in the opcode entries */
		for (ea2num = 0; ea2num < 64; ea2num++)
			if (ea2_allowed[ea2num])
				for (eanum = 0; eanum < 64; eanum++)
					if (ea_allowed[eanum])
					{
						UINT16 eabits = ((ea2num & 0x38) << 3) | ((ea2num & 0x07) << 9) | eanum;

						/* iterate over opcode entries */
						for (opnum = 0; opnum <= mask; opnum += step)
							if ((opnum & mask) == value)
							{
								int length = 1;

								/* skip if we've already populated */
								if (optable[opnum | eabits].flags != OF_INVALID)
									continue;

								/* determine the length of the opcode */
								if (flags & OF_ISIZEMASK)
									length += ((flags & OF_ISIZEMASK) == OF_IMML) ? 2 : 1;
								if ((eanum >= 0x28 && eanum <= 0x38) || eanum == 0x3a || eanum == 0x3b)
									length += 1;
								else if (eanum == 0x39)
									length += 2;
								else if (eanum == 0x3c)
									length += ((flags & OF_SIZEMASK) == OF_LONG) ? 2 : 1;
								if ((ea2num >= 0x28 && ea2num <= 0x38) || ea2num == 0x3a || ea2num == 0x3b)
									length += 1;
								else if (ea2num == 0x39)
									length += 2;
								else if (ea2num == 0x3c)
									length += ((flags & OF_SIZEMASK) == OF_LONG) ? 2 : 1;

								/* make sure we match the disassembler */
								#ifdef MAME_DEBUG
								{
									char dummybuffer[40];
									UINT8 instrbuffer[10];
									instrbuffer[0] = (opnum | eabits) >> 8;
									instrbuffer[1] = (opnum | eabits);
									dummybuffer[0] = 0;
									assert(length == (m68k_disassemble_raw(dummybuffer, 0, instrbuffer, instrbuffer, M68K_CPU_TYPE_68000) & 0xff) / 2);
								}
								#endif

								/* set the value of the entry in the table */
								optable[opnum | eabits].flags = flags | (length << 28);
								optable[opnum | eabits].string = instring;
							}
					}
	}
}


/*-----------------------------------------------
    validate_ea - determine whether an EA is
    valid or not, and return the length
-----------------------------------------------*/

static int validate_ea(address_space &space, UINT32 pc, UINT8 modereg, const UINT8 *parambase, UINT32 flags)
{
	UINT32 addr;
	int valid;

	/* switch off of the mode */
	switch ((modereg >> 3) & 7)
	{
		case 0:     /* Dn -- always good */
		case 1:     /* An -- always good */
		case 2:     /* (An) -- always good */
		case 3:     /* (An)+ -- always good */
		case 4:     /* -(An) -- always good */
			return 0;

		case 5:     /* (d16,An) -- always good, but odd displacements are a warning for word/long */
			if ((flags & OF_SIZEMASK) != OF_BYTE && (parambase[1] & 1) == 1)
				return -1;
			return 1;

		case 6:     /* (d8,An,Xn)  -- always good, but odd displacements are a warning for word/long */
			/* also look for invalid extension words */
			if ((parambase[0] & 7) != 0)
				return 1000;
			if ((flags & OF_SIZEMASK) != OF_BYTE && (parambase[1] & 1) == 1)
				return -1;
			return 1;

		case 7:
			switch (modereg & 7)
			{
				case 0: /* (xxx).W -- make sure it is not odd for word/long */
					addr = (INT16)((parambase[0] << 8) | parambase[1]);
					valid = addr_is_valid(space, addr & 0xffffff, flags);
					return (valid == 0) ? 1000 : (valid == 2) ? -1 : 1;

				case 1: /* (xxx).L -- make sure it is not odd for word/long, and make sure upper byte of addr is 0 */
					valid = addr_is_valid(space, (parambase[0] << 24) | (parambase[1] << 16) | (parambase[2] << 8) | parambase[3], flags);
					return (valid == 0) ? 1000 : (valid == 2) ? -2 : 2;

				case 2: /* (d16,PC) -- make sure it is not odd for word/long */
					valid = addr_is_valid(space, pc + (INT16)((parambase[0] << 8) | parambase[1]), flags);
					return (valid == 0) ? 1000 : (valid == 2) ? -1 : 1;

				case 3: /* (d8,PC,Xn) -- odd displacements are a warning for word/long */
					if ((parambase[0] & 7) != 0)
						return 1000;
					if ((flags & OF_SIZEMASK) != OF_BYTE && (parambase[1] & 1) == 1)
						return -1;
					return 1;

				case 4: /* immediate -- check high byte if byte-sized */
					if ((flags & OF_SIZEMASK) == OF_BYTE && parambase[0] != 0)
						return 1000;
					return ((flags & OF_SIZEMASK) == SIZE_LONG) ? 2 : 1;
			}
			break;
	}

	/* should never get here */
	assert(FALSE);
	return 0;
}


/*-----------------------------------------------
    validate_opcode - validate an opcode up to
    the length specified
-----------------------------------------------*/

static int validate_opcode(address_space &space, UINT32 pc, const UINT8 *opdata, int maxwords)
{
	UINT32 immvalue = 0;
	int iffy = FALSE;
	int offset = 0;
	UINT16 opcode;
	UINT32 flags;
	int oplength;

	assert(maxwords >= 1);

	/* extract the opcode and look it up in our table */
	opcode = (opdata[offset*2+0] << 8) | opdata[offset*2+1];
	flags = optable[opcode].flags;
	oplength = flags >> 28;

	/* weed out invalid opcodes immediately */
	offset++;
	if (flags == OF_INVALID)
		return 0;
	iffy = ((flags & OF_RARE) != 0);

	/* if we're done, or if we don't have enough words, stop now */
	if (offset == oplength || maxwords < oplength)
		return iffy ? -oplength : oplength;

	/* if the opcode has an immediate, process that */
	if (flags & OF_ISIZEMASK)
	{
		int neededwords = ((flags & OF_ISIZEMASK) == OF_IMML) ? 2 : 1;

		/* extract the immediate value */
		immvalue = (opdata[offset*2+0] << 8) | opdata[offset*2+1];
		if ((flags & OF_ISIZEMASK) == OF_IMML)
			immvalue = (immvalue << 16) | (opdata[offset*2+2] << 8) | opdata[offset*2+3];

		/* if it's a byte immediate, ensure the upper bits are 0 (except for -1) */
		if ((flags & OF_ISIZEMASK) == OF_IMMB && immvalue > 0xff && immvalue != 0xffff)
			return 0;

		/* if it's a bit immediate, ensure all but the lower 3 bits are 0 */
		if ((flags & OF_ISIZEMASK) == OF_IMMBIT)
		{
			/* registers can do up to 32 bits */
			if ((opcode & 0x3f) < 8)
			{
				if (immvalue > 31)
					return 0;
			}

			/* memory operands can do up to 8 bits */
			else
			{
				if (immvalue > 7)
					return 0;
			}
		}

		/* advance past the immedate */
		offset += neededwords;
	}

	/* if we're a branch, validate the immediate value */
	if (flags & OF_BRANCH)
	{
		int valid;

		/* first adjust the PC based on the size of the branch */
		pc += 2;
		if ((flags & OF_SIZEMASK) == OF_BYTE)
			pc += (INT8)opcode;
		else if ((flags & OF_SIZEMASK) == OF_WORD)
			pc += (INT16)immvalue;
		else
			pc += immvalue;

		/* if we're odd or out of range, fail */
		valid = pc_is_valid(space, pc, flags);
		if (valid == 0)
			return 0;
		if (valid == 2)
			iffy = TRUE;
	}

	/* process the EA, if present */
	if (flags & (OF_EASRC | OF_EADST))
	{
		int modereg = opcode & 0x003f;
		int ealen = validate_ea(space, pc + offset*2, modereg, &opdata[offset*2], flags);

		/* if the ea was invalid, forward that result */
		if (ealen == 1000)
			return 0;

		/* if the ea was iffy, indicate that */
		if (ealen < 0)
		{
			ealen = -ealen;
			iffy = TRUE;
		}

		/* advance past the ea */
		offset += ealen;
	}

	/* process the move EA, if present */
	if (flags & OF_MOVE)
	{
		int modereg = ((opcode & 0x01c0) >> 3) | ((opcode & 0x0e00) >> 9);
		int ealen = validate_ea(space, pc + offset*2, modereg, &opdata[offset*2], flags);

		/* if the ea was invalid, forward that result */
		if (ealen == 1000)
			return 0;

		/* if the ea was iffy, indicate that */
		if (ealen < 0)
		{
			ealen = -ealen;
			iffy = TRUE;
		}

		/* advance past the ea */
		offset += ealen;
	}

	/* at this point we should be at the end */
	assert(offset == oplength);
	return iffy ? -oplength : oplength;
}

#endif
