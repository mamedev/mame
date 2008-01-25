/***************************************************************************

    memory.c

    Functions which handle the CPU memory access.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    Caveats:

    * If your driver executes an opcode which crosses a bank-switched
    boundary, it will pull the wrong data out of memory. Although not
    a common case, you may need to revert to memcpy to work around this.
    See machine/tnzs.c for an example.

    To do:

    - Add local banks for RAM/ROM to reduce pressure on banking
    - Always mirror everything out to 32 bits so we don't have to mask the address?
    - Add the ability to start with another memory map and modify it
    - Add fourth memory space for encrypted opcodes
    - Automatically mirror program space into data space if no data space
    - Get rid of opcode/data separation by using address spaces?
    - Add support for internal addressing (maybe just accessors - see TMS3202x)
    - Evaluate min/max opcode ranges and do we include a check in cpu_readop?

****************************************************************************

    Address map fields and restrictions:

    AM_RANGE(start, end)
        Specifies a range of consecutive addresses beginning with 'start' and
        ending with 'end' inclusive. An address hits in this bucket if the
        'address' >= 'start' and 'address' <= 'end'.

    AM_SPACE(match, mask)
        Specifies at the bit level (closer to real hardware) how to determine
        if an address matches a given bit pattern. An address hits in this
        bucket if 'address' & 'mask' == 'match'

    AM_MASK(mask)
        Specifies a mask for the addresses in the current bucket. This mask
        is applied after a positive hit in the bucket specified by AM_RANGE
        or AM_SPACE, and is computed before accessing the RAM or calling
        through to the read/write handler. If you use AM_SPACE, the mask
        is implicitly set equal to the logical NOT of the mask specified in
        the AM_SPACE macro. If you use AM_MIRROR, below, the mask is ANDed
        implicitly with the logical NOT of the mirror. The mask specified
        by this macro is ANDed against any implicit masks.

    AM_MIRROR(mirror)
        Specifies mirror addresses for the given bucket. The current bucket
        is mapped repeatedly according to the mirror mask, once where each
        mirror bit is 0, and once where it is 1. For example, a 'mirror'
        value of 0x14000 would map the bucket at 0x00000, 0x04000, 0x10000,
        and 0x14000.

    AM_READ(read)
        Specifies the read handler for this bucket. All reads will pass
        through the given callback handler. Special static values representing
        RAM, ROM, or BANKs are also allowed here.

    AM_WRITE(write)
        Specifies the write handler for this bucket. All writes will pass
        through the given callback handler. Special static values representing
        RAM, ROM, or BANKs are also allowed here.

    AM_REGION(region, offs)
        Only useful if AM_READ/WRITE point to RAM, ROM, or BANK memory. By
        default, memory is allocated to back each bucket. By specifying
        AM_REGION, you can tell the memory system to point the base of the
        memory backing this bucket to a given memory 'region' at the
        specified 'offs'.

    AM_SHARE(index)
        Similar to AM_REGION, this specifies that the memory backing the
        current bucket is shared with other buckets. The first bucket to
        specify the share 'index' will use its memory as backing for all
        future buckets that specify AM_SHARE with the same 'index'.

    AM_BASE(base)
        Specifies a pointer to a pointer to the base of the memory backing
        the current bucket.

    AM_SIZE(size)
        Specifies a pointer to a size_t variable which will be filled in
        with the size, in bytes, of the current bucket.

***************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "deprecat.h"
#ifdef MAME_DEBUG
#include "debug/debugcpu.h"
#endif


#define MEM_DUMP		(0)
#define VERBOSE			(0)
#define ALLOW_ONLY_AUTO_MALLOC_BANKS	0


#define VPRINTF(x)	do { if (VERBOSE) mame_printf_debug x; } while (0)



/***************************************************************************

    Basic theory of memory handling:

    An address with up to 32 bits is passed to a memory handler. First,
    an address mask is applied to the address, removing unused bits.

    Next, the address is broken into two halves, an upper half and a
    lower half. The number of bits in each half can be controlled via
    macros in memory.h, but they default to the upper 18 bits and the
    lower 14 bits. The upper half is then used as an index into the
    base_lookup table.

    If the value pulled from the table is within the range 192-255, then
    the lower half of the address is needed to resolve the final handler.
    The value from the table (192-255) is combined with the lower address
    bits to form an index into a subtable.

    Table values in the range 0-63 are reserved for internal handling
    (such as RAM, ROM, NOP, and banking). Table values between 64 and 192
    are assigned dynamically at startup.

***************************************************************************/

/* macros for the profiler */
#define MEMREADSTART()			do { profiler_mark(PROFILER_MEMREAD); } while (0)
#define MEMREADEND(ret)			do { profiler_mark(PROFILER_END); return ret; } while (0)
#define MEMWRITESTART()			do { profiler_mark(PROFILER_MEMWRITE); } while (0)
#define MEMWRITEEND(ret)		do { (ret); profiler_mark(PROFILER_END); return; } while (0)

/* helper macros */
#define HANDLER_IS_RAM(h)		((FPTR)(h) == STATIC_RAM)
#define HANDLER_IS_ROM(h)		((FPTR)(h) == STATIC_ROM)
#define HANDLER_IS_NOP(h)		((FPTR)(h) == STATIC_NOP)
#define HANDLER_IS_BANK(h)		((FPTR)(h) >= STATIC_BANK1 && (FPTR)(h) <= STATIC_BANKMAX)
#define HANDLER_IS_STATIC(h)	((FPTR)(h) < STATIC_COUNT)

#define HANDLER_TO_BANK(h)		((UINT32)(FPTR)(h))
#define BANK_TO_HANDLER(b)		((genf *)(FPTR)(b))

#define SPACE_SHIFT(s,a)		(((s)->ashift < 0) ? ((a) << -(s)->ashift) : ((a) >> (s)->ashift))
#define SPACE_SHIFT_END(s,a)	(((s)->ashift < 0) ? (((a) << -(s)->ashift) | ((1 << -(s)->ashift) - 1)) : ((a) >> (s)->ashift))
#define INV_SPACE_SHIFT(s,a)	(((s)->ashift < 0) ? ((a) >> -(s)->ashift) : ((a) << (s)->ashift))

#define SUBTABLE_PTR(tabledata, entry) (&(tabledata)->table[(1 << LEVEL1_BITS) + (((entry) - SUBTABLE_BASE) << LEVEL2_BITS)])

#ifdef MAME_DEBUG
#define DEBUG_HOOK_READ(a,b,c) if (debug_hook_read) (*debug_hook_read)(a, b, c)
#define DEBUG_HOOK_WRITE(a,b,c,d) if (debug_hook_write) (*debug_hook_write)(a, b, c, d)
#else
#define DEBUG_HOOK_READ(a,b,c)
#define DEBUG_HOOK_WRITE(a,b,c,d)
#endif


/*-------------------------------------------------
    TYPE DEFINITIONS
-------------------------------------------------*/

typedef struct _memory_block memory_block;
struct _memory_block
{
	UINT8					cpunum;					/* which CPU are we associated with? */
	UINT8					spacenum;				/* which address space are we associated with? */
	UINT8					isallocated;			/* did we allocate this ourselves? */
	offs_t 					start, end;				/* start/end or match/mask for verifying a match */
    UINT8 *					data;					/* pointer to the data for this block */
};

typedef struct _bank_data bank_data;
struct _bank_data
{
	UINT8 					used;					/* is this bank used? */
	UINT8 					dynamic;				/* is this bank allocated dynamically? */
	UINT8 					cpunum;					/* the CPU it is used for */
	UINT8 					spacenum;				/* the address space it is used for */
	UINT8 					read;					/* is this bank used for reads? */
	UINT8 					write;					/* is this bank used for writes? */
	offs_t 					base;					/* the base offset */
	offs_t 					end;					/* the end offset */
	UINT16					curentry;				/* current entry */
	void *					entry[MAX_BANK_ENTRIES];/* array of entries for this bank */
	void *					entryd[MAX_BANK_ENTRIES];/* array of decrypted entries for this bank */
};

typedef union _rwhandlers rwhandlers;
union _rwhandlers
{
	genf *					generic;				/* generic handler void */
	read_handlers			read;					/* read handlers */
	write_handlers			write;					/* write handlers */
};

/* In memory.h: typedef struct _handler_data handler_data */
struct _handler_data
{
	rwhandlers				handler;				/* function pointer for handler */
	offs_t					offset;					/* base offset for handler */
	offs_t					top;					/* maximum offset for handler */
	offs_t					mask;					/* mask against the final address */
	const char *			name;					/* name of the handler */
};

typedef struct _subtable_data subtable_data;
struct _subtable_data
{
	UINT8					checksum_valid;			/* is the checksum valid */
	UINT32					checksum;				/* checksum over all the bytes */
	UINT32					usecount;				/* number of times this has been used */
};

typedef struct _table_data table_data;
struct _table_data
{
	UINT8 *					table;					/* pointer to base of table */
	UINT8 					subtable_alloc;			/* number of subtables allocated */
	subtable_data			subtable[SUBTABLE_COUNT]; /* info about each subtable */
	handler_data			handlers[ENTRY_COUNT];	/* array of user-installed handlers */
};

typedef struct _addrspace_data addrspace_data;
struct _addrspace_data
{
	UINT8					cpunum;					/* CPU index */
	UINT8					spacenum;				/* address space index */
	INT8					ashift;					/* address shift */
	UINT8					abits;					/* address bits */
	UINT8 					dbits;					/* data bits */
	offs_t					rawmask;				/* raw address mask, before adjusting to bytes */
	offs_t					mask;					/* address mask */
	UINT64					unmap;					/* unmapped value */
	table_data				read;					/* memory read lookup table */
	table_data				write;					/* memory write lookup table */
	const data_accessors *		accessors;				/* pointer to the memory accessors */
	address_map *			map;					/* original memory map */
	address_map *			adjmap;					/* adjusted memory map */
};

typedef struct _cpu_data cpu_data;
struct _cpu_data
{
	opbase_handler 			opbase;					/* opcode base handler */

	void *					op_ram;					/* dynamic RAM base pointer */
	void *					op_rom;					/* dynamic ROM base pointer */
	offs_t					op_mask;				/* dynamic ROM address mask */
	offs_t					op_mem_min;				/* dynamic ROM/RAM min */
	offs_t					op_mem_max;				/* dynamic ROM/RAM max */
	UINT8		 			opcode_entry;			/* opcode base handler */

	UINT8					spacemask;				/* mask of which address spaces are used */
	addrspace_data		 	space[ADDRESS_SPACES];	/* info about each address space */
};


/*-------------------------------------------------
    GLOBAL VARIABLES
-------------------------------------------------*/

UINT8 *						opcode_base;					/* opcode base */
UINT8 *						opcode_arg_base;				/* opcode argument base */
offs_t						opcode_mask;					/* mask to apply to the opcode address */
offs_t						opcode_memory_min;				/* opcode memory minimum */
offs_t						opcode_memory_max;				/* opcode memory maximum */
UINT8		 				opcode_entry;					/* opcode readmem entry */

address_space				active_address_space[ADDRESS_SPACES];/* address space data */

static UINT8 *				bank_ptr[STATIC_COUNT];			/* array of bank pointers */
static UINT8 *				bankd_ptr[STATIC_COUNT];		/* array of decrypted bank pointers */
static void *				shared_ptr[MAX_SHARED_POINTERS];/* array of shared pointers */

static memory_block 		memory_block_list[MAX_MEMORY_BLOCKS];/* array of memory blocks we are tracking */
static int 					memory_block_count = 0;			/* number of memory_block[] entries used */

static int					cur_context;					/* current CPU context */

static opbase_handler		opbasefunc;						/* opcode base override */

static int					debugger_access;				/* treat accesses as coming from the debugger */
static int					log_unmap[ADDRESS_SPACES];		/* log unmapped memory accesses */

static cpu_data				cpudata[MAX_CPU];				/* data gathered for each CPU */
static bank_data 			bankdata[STATIC_COUNT];			/* data gathered for each bank */

#ifdef MAME_DEBUG
static debug_hook_read_ptr	debug_hook_read;				/* pointer to debugger callback for memory reads */
static debug_hook_write_ptr	debug_hook_write;				/* pointer to debugger callback for memory writes */
#endif

static const data_accessors memory_accessors[ADDRESS_SPACES][4][2] =
{
	/* program accessors */
	{
		{
			{ program_read_byte_8, NULL, NULL, NULL, program_write_byte_8, NULL, NULL, NULL },
			{ program_read_byte_8, NULL, NULL, NULL, program_write_byte_8, NULL, NULL, NULL }
		},
		{
			{ program_read_byte_16le, program_read_word_16le, NULL, NULL, program_write_byte_16le, program_write_word_16le, NULL, NULL },
			{ program_read_byte_16be, program_read_word_16be, NULL, NULL, program_write_byte_16be, program_write_word_16be, NULL, NULL }
		},
		{
			{ program_read_byte_32le, program_read_word_32le, program_read_dword_32le, NULL, program_write_byte_32le, program_write_word_32le, program_write_dword_32le, NULL },
			{ program_read_byte_32be, program_read_word_32be, program_read_dword_32be, NULL, program_write_byte_32be, program_write_word_32be, program_write_dword_32be, NULL }
		},
		{
			{ program_read_byte_64le, program_read_word_64le, program_read_dword_64le, program_read_qword_64le, program_write_byte_64le, program_write_word_64le, program_write_dword_64le, program_write_qword_64le },
			{ program_read_byte_64be, program_read_word_64be, program_read_dword_64be, program_read_qword_64be, program_write_byte_64be, program_write_word_64be, program_write_dword_64be, program_write_qword_64be }
		}
	},

	/* data accessors */
	{
		{
			{ data_read_byte_8, NULL, NULL, NULL, data_write_byte_8, NULL, NULL, NULL },
			{ data_read_byte_8, NULL, NULL, NULL, data_write_byte_8, NULL, NULL, NULL }
		},
		{
			{ data_read_byte_16le, data_read_word_16le, NULL, NULL, data_write_byte_16le, data_write_word_16le, NULL, NULL },
			{ data_read_byte_16be, data_read_word_16be, NULL, NULL, data_write_byte_16be, data_write_word_16be, NULL, NULL }
		},
		{
			{ data_read_byte_32le, data_read_word_32le, data_read_dword_32le, NULL, data_write_byte_32le, data_write_word_32le, data_write_dword_32le, NULL },
			{ data_read_byte_32be, data_read_word_32be, data_read_dword_32be, NULL, data_write_byte_32be, data_write_word_32be, data_write_dword_32be, NULL }
		},
		{
			{ data_read_byte_64le, data_read_word_64le, data_read_dword_64le, data_read_qword_64le, data_write_byte_64le, data_write_word_64le, data_write_dword_64le, data_write_qword_64le },
			{ data_read_byte_64be, data_read_word_64be, data_read_dword_64be, data_read_qword_64be, data_write_byte_64be, data_write_word_64be, data_write_dword_64be, data_write_qword_64be }
		}
	},

	/* I/O accessors */
	{
		{
			{ io_read_byte_8, NULL, NULL, NULL, io_write_byte_8, NULL, NULL, NULL },
			{ io_read_byte_8, NULL, NULL, NULL, io_write_byte_8, NULL, NULL, NULL }
		},
		{
			{ io_read_byte_16le, io_read_word_16le, NULL, NULL, io_write_byte_16le, io_write_word_16le, NULL, NULL },
			{ io_read_byte_16be, io_read_word_16be, NULL, NULL, io_write_byte_16be, io_write_word_16be, NULL, NULL }
		},
		{
			{ io_read_byte_32le, io_read_word_32le, io_read_dword_32le, NULL, io_write_byte_32le, io_write_word_32le, io_write_dword_32le, NULL },
			{ io_read_byte_32be, io_read_word_32be, io_read_dword_32be, NULL, io_write_byte_32be, io_write_word_32be, io_write_dword_32be, NULL }
		},
		{
			{ io_read_byte_64le, io_read_word_64le, io_read_dword_64le, io_read_qword_64le, io_write_byte_64le, io_write_word_64le, io_write_dword_64le, io_write_qword_64le },
			{ io_read_byte_64be, io_read_word_64be, io_read_dword_64be, io_read_qword_64be, io_write_byte_64be, io_write_word_64be, io_write_dword_64be, io_write_qword_64be }
		}
	},
};

const char *const address_space_names[ADDRESS_SPACES] = { "program", "data", "I/O" };


/*-------------------------------------------------
    INLINE FUNCTIONS
-------------------------------------------------*/

INLINE void force_opbase_update(void)
{
	opcode_entry = 0xff;
	memory_set_opbase(activecpu_get_physical_pc_byte());
}


/*-------------------------------------------------
    FUNCTION PROTOTYPES
-------------------------------------------------*/

static void init_cpudata(void);
static void init_addrspace(UINT8 cpunum, UINT8 spacenum);
static void preflight_memory(void);
static void populate_memory(void);
static void install_mem_handler_private(addrspace_data *space, int iswrite, int databits, int ismatchmask, offs_t start, offs_t end, offs_t mask, offs_t mirror, genf *handler, int isfixed, const char *handler_name);
static void install_mem_handler(addrspace_data *space, int iswrite, int databits, int ismatchmask, offs_t start, offs_t end, offs_t mask, offs_t mirror, genf *handler, int isfixed, const char *handler_name);
static genf *assign_dynamic_bank(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mirror, int isfixed, int ismasked);
static UINT8 get_handler_index(handler_data *table, genf *handler, const char *handler_name, offs_t start, offs_t end, offs_t mask);
static void populate_table_range(addrspace_data *space, int iswrite, offs_t start, offs_t stop, UINT8 handler);
static void populate_table_match(addrspace_data *space, int iswrite, offs_t matchval, offs_t matchmask, UINT8 handler);
static UINT8 allocate_subtable(table_data *tabledata);
static void reallocate_subtable(table_data *tabledata, UINT8 subentry);
static int merge_subtables(table_data *tabledata);
static void release_subtable(table_data *tabledata, UINT8 subentry);
static UINT8 *open_subtable(table_data *tabledata, offs_t l1index);
static void close_subtable(table_data *tabledata, offs_t l1index);
static void allocate_memory(void);
static void *allocate_memory_block(int cpunum, int spacenum, offs_t start, offs_t end, void *memory);
static void register_for_save(int cpunum, int spacenum, offs_t start, void *base, size_t numbytes);
static address_map *assign_intersecting_blocks(addrspace_data *space, offs_t start, offs_t end, UINT8 *base);
static void find_memory(void);
static void *memory_find_base(int cpunum, int spacenum, int readwrite, offs_t offset);
static genf *get_static_handler(int databits, int readorwrite, int spacenum, int which);
static void memory_exit(running_machine *machine);

static void mem_dump(void)
{
	FILE *file;

	if (MEM_DUMP)
	{
		file = fopen("memdump.log", "w");
		if (file)
		{
			memory_dump(file);
			fclose(file);
		}
	}
}



/*-------------------------------------------------
    memory_init - initialize the memory system
-------------------------------------------------*/

void memory_init(running_machine *machine)
{
	int i;

	for (i = 0; i < ADDRESS_SPACES; i++)
		log_unmap[i] = 1;

	/* no current context to start */
	cur_context = -1;

	/* reset the shared pointers and bank pointers */
	memset(shared_ptr, 0, sizeof(shared_ptr));
	memset(bank_ptr, 0, sizeof(bank_ptr));
	memset(bankd_ptr, 0, sizeof(bankd_ptr));

	/* reset our hardcoded and allocated pointer tracking */
	memset(memory_block_list, 0, sizeof(memory_block_list));
	memory_block_count = 0;

	/* init the CPUs */
	init_cpudata();
	add_exit_callback(machine, memory_exit);

	/* preflight the memory handlers and check banks */
	preflight_memory();

	/* then fill in the tables */
	populate_memory();

	/* allocate any necessary memory */
	allocate_memory();

	/* find all the allocated pointers */
	find_memory();

	/* dump the final memory configuration */
	mem_dump();
}


/*-------------------------------------------------
    memory_exit - free memory
-------------------------------------------------*/

static void memory_exit(running_machine *machine)
{
	int cpunum, spacenum;

	/* free all the tables */
	for (cpunum = 0; cpunum < MAX_CPU; cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			if (cpudata[cpunum].space[spacenum].read.table)
				free(cpudata[cpunum].space[spacenum].read.table);
			if (cpudata[cpunum].space[spacenum].write.table)
				free(cpudata[cpunum].space[spacenum].write.table);
		}
}


/*-------------------------------------------------
    memory_set_context - set the memory context
-------------------------------------------------*/

void memory_set_context(int activecpu)
{
	/* remember dynamic RAM/ROM */
	if (cur_context != -1)
	{
		cpudata[cur_context].op_ram = opcode_arg_base;
		cpudata[cur_context].op_rom = opcode_base;
		cpudata[cur_context].op_mask = opcode_mask;
		cpudata[cur_context].op_mem_min = opcode_memory_min;
		cpudata[cur_context].op_mem_max = opcode_memory_max;
		cpudata[cur_context].opcode_entry = opcode_entry;
	}
	cur_context = activecpu;

	opcode_arg_base = cpudata[activecpu].op_ram;
	opcode_base = cpudata[activecpu].op_rom;
	opcode_mask = cpudata[activecpu].op_mask;
	opcode_memory_min = cpudata[activecpu].op_mem_min;
	opcode_memory_max = cpudata[activecpu].op_mem_max;
	opcode_entry = cpudata[activecpu].opcode_entry;

	/* program address space */
	active_address_space[ADDRESS_SPACE_PROGRAM].addrmask = cpudata[activecpu].space[ADDRESS_SPACE_PROGRAM].mask;
	active_address_space[ADDRESS_SPACE_PROGRAM].readlookup = cpudata[activecpu].space[ADDRESS_SPACE_PROGRAM].read.table;
	active_address_space[ADDRESS_SPACE_PROGRAM].writelookup = cpudata[activecpu].space[ADDRESS_SPACE_PROGRAM].write.table;
	active_address_space[ADDRESS_SPACE_PROGRAM].readhandlers = cpudata[activecpu].space[ADDRESS_SPACE_PROGRAM].read.handlers;
	active_address_space[ADDRESS_SPACE_PROGRAM].writehandlers = cpudata[activecpu].space[ADDRESS_SPACE_PROGRAM].write.handlers;
	active_address_space[ADDRESS_SPACE_PROGRAM].accessors = cpudata[activecpu].space[ADDRESS_SPACE_PROGRAM].accessors;

	/* data address space */
	if (cpudata[activecpu].spacemask & (1 << ADDRESS_SPACE_DATA))
	{
		active_address_space[ADDRESS_SPACE_DATA].addrmask = cpudata[activecpu].space[ADDRESS_SPACE_DATA].mask;
		active_address_space[ADDRESS_SPACE_DATA].readlookup = cpudata[activecpu].space[ADDRESS_SPACE_DATA].read.table;
		active_address_space[ADDRESS_SPACE_DATA].writelookup = cpudata[activecpu].space[ADDRESS_SPACE_DATA].write.table;
		active_address_space[ADDRESS_SPACE_DATA].readhandlers = cpudata[activecpu].space[ADDRESS_SPACE_DATA].read.handlers;
		active_address_space[ADDRESS_SPACE_DATA].writehandlers = cpudata[activecpu].space[ADDRESS_SPACE_DATA].write.handlers;
		active_address_space[ADDRESS_SPACE_DATA].accessors = cpudata[activecpu].space[ADDRESS_SPACE_DATA].accessors;
	}

	/* I/O address space */
	if (cpudata[activecpu].spacemask & (1 << ADDRESS_SPACE_IO))
	{
		active_address_space[ADDRESS_SPACE_IO].addrmask = cpudata[activecpu].space[ADDRESS_SPACE_IO].mask;
		active_address_space[ADDRESS_SPACE_IO].readlookup = cpudata[activecpu].space[ADDRESS_SPACE_IO].read.table;
		active_address_space[ADDRESS_SPACE_IO].writelookup = cpudata[activecpu].space[ADDRESS_SPACE_IO].write.table;
		active_address_space[ADDRESS_SPACE_IO].readhandlers = cpudata[activecpu].space[ADDRESS_SPACE_IO].read.handlers;
		active_address_space[ADDRESS_SPACE_IO].writehandlers = cpudata[activecpu].space[ADDRESS_SPACE_IO].write.handlers;
		active_address_space[ADDRESS_SPACE_IO].accessors = cpudata[activecpu].space[ADDRESS_SPACE_IO].accessors;
	}

	opbasefunc = cpudata[activecpu].opbase;

#ifdef MAME_DEBUG
	if (activecpu != -1)
		debug_get_memory_hooks(activecpu, &debug_hook_read, &debug_hook_write);
	else
	{
		debug_hook_read = NULL;
		debug_hook_write = NULL;
	}
#endif
}


/*-------------------------------------------------
    memory_get_map - return a pointer to a CPU's
    memory map
-------------------------------------------------*/

const address_map *memory_get_map(int cpunum, int spacenum)
{
	return cpudata[cpunum].space[spacenum].map;
}


/*-------------------------------------------------
    memory_set_opbase_handler - change op-code
    memory base
-------------------------------------------------*/

opbase_handler memory_set_opbase_handler(int cpunum, opbase_handler function)
{
	opbase_handler old = cpudata[cpunum].opbase;
	cpudata[cpunum].opbase = function;
	if (cpunum == cpu_getactivecpu())
		opbasefunc = function;
	return old;
}


/*-------------------------------------------------
    memory_set_opbase - generic opcode base changer
-------------------------------------------------*/

void memory_set_opbase(offs_t pc)
{
	const address_space *space = &active_address_space[ADDRESS_SPACE_PROGRAM];
	UINT8 *base = NULL, *based = NULL;
	const handler_data *handlers;
	UINT8 entry;

	/* allow overrides */
	if (opbasefunc != NULL)
	{
		pc = (*opbasefunc)(pc);
		if (pc == ~0)
			return;
	}

	/* perform the lookup */
	pc &= space->addrmask;
	entry = space->readlookup[LEVEL1_INDEX(pc)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry,pc)];

	/* keep track of current entry */
	opcode_entry = entry;

	/* if we don't map to a bank, see if there are any banks we can map to */
	if (entry < STATIC_BANK1 || entry >= STATIC_RAM)
	{
		/* loop over banks and find a match */
		for (entry = 1; entry < STATIC_COUNT; entry++)
		{
			bank_data *bdata = &bankdata[entry];
			if (bdata->used && bdata->cpunum == cur_context && bdata->spacenum == ADDRESS_SPACE_PROGRAM &&
				bdata->base < pc && bdata->end > pc)
				break;
		}

		/* if nothing was found, leave everything alone */
		if (entry == STATIC_COUNT)
		{
			logerror("cpu #%d (PC=%08X): warning - op-code execute on mapped I/O\n",
						cpu_getactivecpu(), activecpu_get_pc());
			return;
		}
	}

	/* if no decrypted opcodes, point to the same base */
	base = bank_ptr[entry];
	based = bankd_ptr[entry];
	if (based == NULL)
		based = base;

	/* compute the adjusted base */
	handlers = &active_address_space[ADDRESS_SPACE_PROGRAM].readhandlers[entry];
	opcode_mask = handlers->mask;
	opcode_arg_base = base - (handlers->offset & opcode_mask);
	opcode_base = based - (handlers->offset & opcode_mask);
	opcode_memory_min = handlers->offset;
	opcode_memory_max = handlers->top;
}


/*-------------------------------------------------
    memory_set_decrypted_region - sets the
    decrypted region for the given CPU
-------------------------------------------------*/

void memory_set_decrypted_region(int cpunum, offs_t start, offs_t end, void *base)
{
	int banknum, found = FALSE;

	/* loop over banks looking for a match */
	for (banknum = 0; banknum < STATIC_COUNT; banknum++)
	{
		bank_data *bdata = &bankdata[banknum];
		if (bdata->used && bdata->cpunum == cpunum && bdata->spacenum == ADDRESS_SPACE_PROGRAM && bdata->read)
		{
			if (bdata->base >= start && bdata->end <= end)
			{
				bankd_ptr[banknum] = (UINT8 *)base + bdata->base - start;
				found = TRUE;

				/* if this is live, adjust now */
				if (cpu_getactivecpu() >= 0 && cpunum == cur_context && opcode_entry == banknum)
					force_opbase_update();
			}
			else if (bdata->base < end && bdata->end > start)
				fatalerror("memory_set_decrypted_region found straddled region %08X-%08X for CPU %d", start, end, cpunum);
		}
	}

	if (!found)
		fatalerror("memory_set_decrypted_region unable to find matching region %08X-%08X for CPU %d", start, end, cpunum);
}


/*-------------------------------------------------
    memory_get_read_ptr - return a pointer to the
    base of RAM associated with the given CPU
    and offset
-------------------------------------------------*/

void *memory_get_read_ptr(int cpunum, int spacenum, offs_t offset)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	UINT8 entry;

	/* perform the lookup */
	offset &= space->mask;
	entry = space->read.table[LEVEL1_INDEX(offset)];
	if (entry >= SUBTABLE_BASE)
		entry = space->read.table[LEVEL2_INDEX(entry, offset)];

	/* 8-bit case: RAM/ROM */
	if (entry >= STATIC_RAM)
		return NULL;
	offset = (offset - space->read.handlers[entry].offset) & space->read.handlers[entry].mask;
	return &bank_ptr[entry][offset];
}


/*-------------------------------------------------
    memory_get_write_ptr - return a pointer to the
    base of RAM associated with the given CPU
    and offset
-------------------------------------------------*/

void *memory_get_write_ptr(int cpunum, int spacenum, offs_t offset)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	UINT8 entry;

	/* perform the lookup */
	offset &= space->mask;
	entry = space->write.table[LEVEL1_INDEX(offset)];
	if (entry >= SUBTABLE_BASE)
		entry = space->write.table[LEVEL2_INDEX(entry, offset)];

	/* 8-bit case: RAM/ROM */
	if (entry >= STATIC_RAM)
		return NULL;
	offset = (offset - space->write.handlers[entry].offset) & space->write.handlers[entry].mask;
	return &bank_ptr[entry][offset];
}


/*-------------------------------------------------
    memory_get_op_ptr - return a pointer to the
    base of opcode RAM associated with the given
    CPU and offset
-------------------------------------------------*/

void *memory_get_op_ptr(int cpunum, offs_t offset, int arg)
{
	addrspace_data *space = &cpudata[cpunum].space[ADDRESS_SPACE_PROGRAM];
	void *ptr = NULL;
	UINT8 entry;

	/* if there is a custom mapper, use that */
	if (cpudata[cpunum].opbase != NULL)
	{
		/* need to save opcode info */
		UINT8 *saved_opcode_base = opcode_base;
		UINT8 *saved_opcode_arg_base = opcode_arg_base;
		offs_t saved_opcode_mask = opcode_mask;
		offs_t saved_opcode_memory_min = opcode_memory_min;
		offs_t saved_opcode_memory_max = opcode_memory_max;
		UINT8 saved_opcode_entry = opcode_entry;

		/* query the handler */
		offs_t new_offset = (*cpudata[cpunum].opbase)(offset);

		/* if it returns ~0, we use whatever data the handler set */
		if (new_offset == ~0)
			ptr = arg ? &opcode_arg_base[offset] : &opcode_base[offset];

		/* otherwise, we use the new offset in the generic case below */
		else
			offset = new_offset;

		/* restore opcode info */
		opcode_base = saved_opcode_base;
		opcode_arg_base = saved_opcode_arg_base;
		opcode_mask = saved_opcode_mask;
		opcode_memory_min = saved_opcode_memory_min;
		opcode_memory_max = saved_opcode_memory_max;
		opcode_entry = saved_opcode_entry;

		/* if we got our pointer, we're done */
		if (ptr != NULL)
			return ptr;
	}

	/* perform the lookup */
	offset &= space->mask;
	entry = space->read.table[LEVEL1_INDEX(offset)];
	if (entry >= SUBTABLE_BASE)
		entry = space->read.table[LEVEL2_INDEX(entry, offset)];

	/* if a non-RAM area, return NULL */
	if (entry >= STATIC_RAM)
		return NULL;

	/* adjust the offset */
	offset = (offset - space->read.handlers[entry].offset) & space->read.handlers[entry].mask;
	return (!arg && bankd_ptr[entry]) ? &bankd_ptr[entry][offset] : &bank_ptr[entry][offset];
}


/*-------------------------------------------------
    memory_configure_bank - configure the
    addresses for a bank
-------------------------------------------------*/

void memory_configure_bank(int banknum, int startentry, int numentries, void *base, offs_t stride)
{
	int entrynum;

	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bankdata[banknum].used)
		fatalerror("memory_configure_bank called with invalid bank %d", banknum);
	if (bankdata[banknum].dynamic)
		fatalerror("memory_configure_bank called with dynamic bank %d", banknum);
	if (startentry < 0 || startentry + numentries > MAX_BANK_ENTRIES)
		fatalerror("memory_configure_bank called with out-of-range entries %d-%d", startentry, startentry + numentries - 1);
	if (!base)
		fatalerror("memory_configure_bank called NULL base");

	/* fill in the requested bank entries */
	for (entrynum = startentry; entrynum < startentry + numentries; entrynum++)
		bankdata[banknum].entry[entrynum] = (UINT8 *)base + (entrynum - startentry) * stride;
}



/*-------------------------------------------------
    memory_configure_bank_decrypted - configure
    the decrypted addresses for a bank
-------------------------------------------------*/

void memory_configure_bank_decrypted(int banknum, int startentry, int numentries, void *base, offs_t stride)
{
	int entrynum;

	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bankdata[banknum].used)
		fatalerror("memory_configure_bank called with invalid bank %d", banknum);
	if (bankdata[banknum].dynamic)
		fatalerror("memory_configure_bank called with dynamic bank %d", banknum);
	if (startentry < 0 || startentry + numentries > MAX_BANK_ENTRIES)
		fatalerror("memory_configure_bank called with out-of-range entries %d-%d", startentry, startentry + numentries - 1);
	if (!base)
		fatalerror("memory_configure_bank_decrypted called NULL base");

	/* fill in the requested bank entries */
	for (entrynum = startentry; entrynum < startentry + numentries; entrynum++)
		bankdata[banknum].entryd[entrynum] = (UINT8 *)base + (entrynum - startentry) * stride;
}



/*-------------------------------------------------
    memory_set_bank - set the base of a bank
-------------------------------------------------*/

void memory_set_bank(int banknum, int entrynum)
{
	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bankdata[banknum].used)
		fatalerror("memory_set_bank called with invalid bank %d", banknum);
	if (bankdata[banknum].dynamic)
		fatalerror("memory_set_bank called with dynamic bank %d", banknum);
	if (entrynum < 0 || entrynum > MAX_BANK_ENTRIES)
		fatalerror("memory_set_bank called with out-of-range entry %d", entrynum);
	if (!bankdata[banknum].entry[entrynum])
		fatalerror("memory_set_bank called for bank %d with invalid bank entry %d", banknum, entrynum);

	/* set the base */
	bankdata[banknum].curentry = entrynum;
	bank_ptr[banknum] = bankdata[banknum].entry[entrynum];
	bankd_ptr[banknum] = bankdata[banknum].entryd[entrynum];

	/* if we're executing out of this bank, adjust the opbase pointer */
	if (opcode_entry == banknum && cpu_getactivecpu() >= 0)
		force_opbase_update();
}



/*-------------------------------------------------
    memory_get_bank - return the currently
    selected bank
-------------------------------------------------*/

int memory_get_bank(int banknum)
{
	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bankdata[banknum].used)
		fatalerror("memory_get_bank called with invalid bank %d", banknum);
	if (bankdata[banknum].dynamic)
		fatalerror("memory_get_bank called with dynamic bank %d", banknum);
	return bankdata[banknum].curentry;
}



/*-------------------------------------------------
    memory_set_bankptr - set the base of a bank
-------------------------------------------------*/

void memory_set_bankptr(int banknum, void *base)
{
	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bankdata[banknum].used)
		fatalerror("memory_set_bankptr called with invalid bank %d", banknum);
	if (bankdata[banknum].dynamic)
		fatalerror("memory_set_bankptr called with dynamic bank %d", banknum);
	if (base == NULL)
		fatalerror("memory_set_bankptr called NULL base");
	if (ALLOW_ONLY_AUTO_MALLOC_BANKS)
		validate_auto_malloc_memory(base, bankdata[banknum].end - bankdata[banknum].base + 1);

	/* set the base */
	bank_ptr[banknum] = base;

	/* if we're executing out of this bank, adjust the opbase pointer */
	if (opcode_entry == banknum && cpu_getactivecpu() >= 0)
		force_opbase_update();
}


/*-------------------------------------------------
    memory_set_debugger_access - set debugger access
-------------------------------------------------*/

void memory_set_debugger_access(int debugger)
{
	debugger_access = debugger;
}


/*-------------------------------------------------
    memory_set_log_unmap - sets whether unmapped
    memory accesses should be logged or not
-------------------------------------------------*/

void memory_set_log_unmap(int spacenum, int log)
{
	log_unmap[spacenum] = log;
}


/*-------------------------------------------------
    memory_get_log_unmap - gets whether unmapped
    memory accesses should be logged or not
-------------------------------------------------*/

int memory_get_log_unmap(int spacenum)
{
	return log_unmap[spacenum];
}


/*-------------------------------------------------
    memory_install_readX_handler - install dynamic
    read handler for X-bit case
-------------------------------------------------*/

void *_memory_install_read_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, FPTR handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (handler < 0 || handler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_read_handler()");
	install_mem_handler(space, 0, space->dbits, 0, start, end, mask, mirror, (genf *)(FPTR)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}

UINT8 *_memory_install_read8_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 8, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}

UINT16 *_memory_install_read16_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 16, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}

UINT32 *_memory_install_read32_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read32_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 32, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}

UINT64 *_memory_install_read64_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read64_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 64, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}


/*-------------------------------------------------
    memory_install_writeX_handler - install dynamic
    write handler for X-bit case
-------------------------------------------------*/

void *_memory_install_write_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, FPTR handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (handler < 0 || handler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_write_handler()");
	install_mem_handler(space, 1, space->dbits, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, start));
}

UINT8 *_memory_install_write8_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, write8_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 1, 8, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, start));
}

UINT16 *_memory_install_write16_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, write16_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 1, 16, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, start));
}

UINT32 *_memory_install_write32_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, write32_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 1, 32, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, start));
}

UINT64 *_memory_install_write64_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, write64_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 1, 64, 0, start, end, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, start));
}


/*-------------------------------------------------
    memory_install_readwriteX_handler - install
    dynamic read and write handlers for X-bit case
-------------------------------------------------*/

void *_memory_install_readwrite_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, FPTR rhandler, FPTR whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler < 0 || rhandler >= STATIC_COUNT || whandler < 0 || whandler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_readwrite_handler()");
	install_mem_handler(space, 0, space->dbits, 0, start, end, mask, mirror, (genf *)(FPTR)rhandler, 0, rhandler_name);
	install_mem_handler(space, 1, space->dbits, 0, start, end, mask, mirror, (genf *)(FPTR)whandler, 0, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}

UINT8 *_memory_install_readwrite8_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_handler rhandler, write8_handler whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 8, 0, start, end, mask, mirror, (genf *)rhandler, 0, rhandler_name);
	install_mem_handler(space, 1, 8, 0, start, end, mask, mirror, (genf *)whandler, 0, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}

UINT16 *_memory_install_readwrite16_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_handler rhandler, write16_handler whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 16, 0, start, end, mask, mirror, (genf *)rhandler, 0, rhandler_name);
	install_mem_handler(space, 1, 16, 0, start, end, mask, mirror, (genf *)whandler, 0, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}

UINT32 *_memory_install_readwrite32_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read32_handler rhandler, write32_handler whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 32, 0, start, end, mask, mirror, (genf *)rhandler, 0, rhandler_name);
	install_mem_handler(space, 1, 32, 0, start, end, mask, mirror, (genf *)whandler, 0, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}

UINT64 *_memory_install_readwrite64_handler(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read64_handler rhandler, write64_handler whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 64, 0, start, end, mask, mirror, (genf *)rhandler, 0, rhandler_name);
	install_mem_handler(space, 1, 64, 0, start, end, mask, mirror, (genf *)whandler, 0, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, start));
}


/*-------------------------------------------------
    memory_install_readX_matchmask_handler -
    install dynamic match/mask read handler for
    X-bit case
-------------------------------------------------*/

void *_memory_install_read_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, FPTR handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (handler < 0 || handler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_read_matchmask_handler()");
	install_mem_handler(space, 0, space->dbits, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, matchval));
}

UINT8 *_memory_install_read8_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, read8_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 8, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, matchval));
}

UINT16 *_memory_install_read16_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, read16_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 16, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, matchval));
}

UINT32 *_memory_install_read32_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, read32_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 32, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, matchval));
}

UINT64 *_memory_install_read64_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, read64_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 0, 64, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 0, SPACE_SHIFT(space, matchval));
}


/*-------------------------------------------------
    memory_install_writeX_matchmask_handler -
    install dynamic match/mask write handler for
    X-bit case
-------------------------------------------------*/

void *_memory_install_write_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, FPTR handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (handler < 0 || handler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_write_matchmask_handler()");
	install_mem_handler(space, 1, space->dbits, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, matchval));
}

UINT8 *_memory_install_write8_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, write8_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 1, 8, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, matchval));
}

UINT16 *_memory_install_write16_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, write16_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 1, 16, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, matchval));
}

UINT32 *_memory_install_write32_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, write32_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 1, 32, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, matchval));
}

UINT64 *_memory_install_write64_matchmask_handler(int cpunum, int spacenum, offs_t matchval, offs_t maskval, offs_t mask, offs_t mirror, write64_handler handler, const char *handler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	install_mem_handler(space, 1, 64, 1, matchval, maskval, mask, mirror, (genf *)handler, 0, handler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, 1, SPACE_SHIFT(space, matchval));
}


/*-------------------------------------------------
    construct_address_map - build address map
-------------------------------------------------*/

void construct_address_map(address_map *map, const machine_config *drv, int cpunum, int spacenum)
{
	int cputype = drv->cpu[cpunum].type;
	construct_map_t internal_map = (construct_map_t)cputype_get_info_fct(cputype, CPUINFO_PTR_INTERNAL_MEMORY_MAP + spacenum);

	map->flags = AM_FLAGS_END;

	/* start by constructing the internal CPU map */
	if (internal_map)
		map = (*internal_map)(map);

	/* construct the standard map */
	if (drv->cpu[cpunum].construct_map[spacenum][0])
		map = (*drv->cpu[cpunum].construct_map[spacenum][0])(map);
	if (drv->cpu[cpunum].construct_map[spacenum][1])
		map = (*drv->cpu[cpunum].construct_map[spacenum][1])(map);
}


/*-------------------------------------------------
    init_cpudata - initialize the cpudata
    structure for each CPU
-------------------------------------------------*/

static void init_cpudata(void)
{
	int cpunum, spacenum;

	/* zap the cpudata structure */
	memset(&cpudata, 0, sizeof(cpudata));

	/* loop over CPUs */
	for (cpunum = 0; cpunum < MAX_CPU && Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
	{
		/* set the RAM/ROM base */
		cpudata[cpunum].op_ram = cpudata[cpunum].op_rom = memory_region(REGION_CPU1 + cpunum);
		cpudata[cpunum].op_mem_max = memory_region_length(REGION_CPU1 + cpunum);
		cpudata[cpunum].op_mem_min = 0;
		cpudata[cpunum].opcode_entry = STATIC_UNMAP;
		cpudata[cpunum].opbase = NULL;

		/* TODO: make this dynamic */
		cpudata[cpunum].spacemask = 0;
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			init_addrspace(cpunum, spacenum);
		cpudata[cpunum].op_mask = cpudata[cpunum].space[ADDRESS_SPACE_PROGRAM].mask;
	}
}


/*-------------------------------------------------
    adjust_addresses - adjust addresses for a
    given address space in a standard fashion
-------------------------------------------------*/

INLINE void adjust_addresses(addrspace_data *space, int ismatchmask, offs_t *start, offs_t *end, offs_t *mask, offs_t *mirror)
{
	/* adjust start/end/mask values */
	if (*mask == 0)
		*mask = space->rawmask & ~*mirror;
	else
		*mask &= space->rawmask;
	*start &= ~*mirror & space->rawmask;
	*end &= ~*mirror & space->rawmask;

	/* adjust to byte values */
	*mask = SPACE_SHIFT(space, *mask);
	*start = SPACE_SHIFT(space, *start);
	*end = ismatchmask ? SPACE_SHIFT(space, *end) : SPACE_SHIFT_END(space, *end);
	*mirror = SPACE_SHIFT(space, *mirror);
}


/*-------------------------------------------------
    init_addrspace - initialize the address space
    data structure
-------------------------------------------------*/

static void init_addrspace(UINT8 cpunum, UINT8 spacenum)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	cpu_type cputype = Machine->drv->cpu[cpunum].type;
	int abits = cputype_addrbus_width(cputype, spacenum);
	int dbits = cputype_databus_width(cputype, spacenum);
	int accessorindex = (dbits == 8) ? 0 : (dbits == 16) ? 1 : (dbits == 32) ? 2 : 3;
	int entrynum;

	/* determine the address and data bits */
	space->cpunum = cpunum;
	space->spacenum = spacenum;
	space->ashift = cputype_addrbus_shift(cputype, spacenum);
	space->abits = abits - space->ashift;
	space->dbits = dbits;
	space->rawmask = 0xffffffffUL >> (32 - abits);
	space->mask = SPACE_SHIFT_END(space, space->rawmask);
	space->accessors = &memory_accessors[spacenum][accessorindex][cputype_endianness(cputype) == CPU_IS_LE ? 0 : 1];
	space->map = NULL;
	space->adjmap = NULL;

	/* if there's nothing here, just punt */
	if (space->abits == 0)
		return;
	cpudata[cpunum].spacemask |= 1 << spacenum;

	/* construct the combined memory map */
	{
		/* allocate and clear memory for 2 copies of the map */
		address_map *map = auto_malloc(sizeof(space->map[0]) * MAX_ADDRESS_MAP_SIZE * 4);
		memset(map, 0, sizeof(space->map[0]) * MAX_ADDRESS_MAP_SIZE * 4);

		/* make pointers to the standard and adjusted maps */
		space->map = map;
		space->adjmap = &map[MAX_ADDRESS_MAP_SIZE * 2];

		construct_address_map(map, Machine->drv, cpunum, spacenum);

		/* convert implicit ROM entries to map to the memory region */
		if (spacenum == ADDRESS_SPACE_PROGRAM && memory_region(REGION_CPU1 + cpunum))
			for (map = space->map; !IS_AMENTRY_END(map); map++)
				if (!IS_AMENTRY_EXTENDED(map) && HANDLER_IS_ROM(map->read.handler) && !map->region)
				{
					offs_t end = SPACE_SHIFT_END(space, map->end);

					/* make sure they fit within the memory region before doing so, however */
					if (end < memory_region_length(REGION_CPU1 + cpunum))
					{
						map->region = REGION_CPU1 + cpunum;
						map->region_offs = SPACE_SHIFT(space, map->start);
					}
				}

		/* convert region-relative entries to their memory pointers */
		for (map = space->map; !IS_AMENTRY_END(map); map++)
			if (map->region)
				map->memory = memory_region(map->region) + map->region_offs;

		/* make the adjusted map */
		memcpy(space->adjmap, space->map, sizeof(space->map[0]) * MAX_ADDRESS_MAP_SIZE * 2);
		for (map = space->adjmap; !IS_AMENTRY_END(map); map++)
			if (!IS_AMENTRY_EXTENDED(map))
				adjust_addresses(space, IS_AMENTRY_MATCH_MASK(map), &map->start, &map->end, &map->mask, &map->mirror);

		/* validate adjusted addresses against implicit regions */
		for (map = space->adjmap; !IS_AMENTRY_END(map); map++)
			if (map->region && map->share == 0 && !map->base)
			{
				UINT8 *base = memory_region(map->region);
				offs_t length = memory_region_length(map->region);

				/* validate the region */
				if (!base)
					fatalerror("Error: CPU %d space %d memory map entry %X-%X references non-existant region %d", cpunum, spacenum, map->start, map->end, map->region);
				if (map->region_offs + (map->end - map->start + 1) > length)
					fatalerror("Error: CPU %d space %d memory map entry %X-%X extends beyond region %d size (%X)", cpunum, spacenum, map->start, map->end, map->region, length);
			}
	}

	/* init the static handlers */
	memset(space->read.handlers, 0, sizeof(space->read.handlers));
	memset(space->write.handlers, 0, sizeof(space->write.handlers));
	for (entrynum = 0; entrynum < ENTRY_COUNT; entrynum++)
	{
		space->read.handlers[entrynum].handler.generic = get_static_handler(dbits, 0, spacenum, entrynum);
		space->read.handlers[entrynum].mask = space->mask;
		space->write.handlers[entrynum].handler.generic = get_static_handler(dbits, 1, spacenum, entrynum);
		space->write.handlers[entrynum].mask = space->mask;
	}

	/* allocate memory */
	space->read.table = malloc_or_die(1 << LEVEL1_BITS);
	space->write.table = malloc_or_die(1 << LEVEL1_BITS);

	/* initialize everything to unmapped */
	memset(space->read.table, STATIC_UNMAP, 1 << LEVEL1_BITS);
	memset(space->write.table, STATIC_UNMAP, 1 << LEVEL1_BITS);
}


/*-------------------------------------------------
    preflight_memory - verify the memory structs
    and track which banks are referenced
-------------------------------------------------*/

static void preflight_memory(void)
{
	int cpunum, spacenum, entrynum;

	/* zap the bank data */
	memset(&bankdata, 0, sizeof(bankdata));

	/* loop over CPUs */
	for (cpunum = 0; cpunum < MAX_CPU && Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].spacemask & (1 << spacenum))
			{
				addrspace_data *space = &cpudata[cpunum].space[spacenum];
				const address_map *map;

				/* scan the adjusted map */
				for (map = space->adjmap; map && !IS_AMENTRY_END(map); map++)
				{
					/* look for extended flags */
					if (IS_AMENTRY_EXTENDED(map))
					{
						UINT32 flags = AM_EXTENDED_FLAGS(map);
						UINT32 val;

						/* if we specify an address space, make sure it matches the current space */
						if (flags & AMEF_SPECIFIES_SPACE)
						{
							val = (flags & AMEF_SPACE_MASK) >> AMEF_SPACE_SHIFT;
							if (val != spacenum)
								fatalerror("cpu #%d has address space %d handlers in place of address space %d handlers!", cpunum, val, spacenum);
						}

						/* if we specify an databus width, make sure it matches the current address space's */
						if (flags & AMEF_SPECIFIES_DBITS)
						{
							val = (flags & AMEF_DBITS_MASK) >> AMEF_DBITS_SHIFT;
							val = (val + 1) * 8;
							if (val != space->dbits)
								fatalerror("cpu #%d uses wrong %d-bit handlers for address space %d (should be %d-bit)!", cpunum, val, spacenum, space->dbits);
						}

						/* if we specify an addressbus width, adjust the mask */
						if (flags & AMEF_SPECIFIES_ABITS)
						{
							space->rawmask = 0xffffffffUL >> (32 - ((flags & AMEF_ABITS_MASK) >> AMEF_ABITS_SHIFT));
							space->mask = SPACE_SHIFT_END(space, space->rawmask);
						}

						/* if we specify an unmap value, set it */
						if (flags & AMEF_SPECIFIES_UNMAP)
							space->unmap = ((flags & AMEF_UNMAP_MASK) == 0) ? (UINT64)0 : (UINT64)-1;
					}

					/* otherwise, just track banks and hardcoded memory pointers */
					else
					{
						int bank = -1;

						/* look for a bank handler in eithe read or write */
						if (HANDLER_IS_BANK(map->read.handler))
							bank = HANDLER_TO_BANK(map->read.handler);
						else if (HANDLER_IS_BANK(map->write.handler))
							bank = HANDLER_TO_BANK(map->write.handler);

						/* if we got one, add the data */
						if (bank >= 1 && bank <= MAX_EXPLICIT_BANKS)
						{
							bank_data *bdata = &bankdata[bank];

							/* wire up state saving for the entry the first time we see it */
							if (!bdata->used)
								state_save_register_item("memory", bank, bdata->curentry);

							bdata->used = TRUE;
							bdata->dynamic = FALSE;
							bdata->cpunum = cpunum;
							bdata->spacenum = spacenum;
							if (bank == HANDLER_TO_BANK(map->read.handler))
								bdata->read = TRUE;
							if (bank == HANDLER_TO_BANK(map->write.handler))
								bdata->write = TRUE;
							bdata->base = map->start;
							bdata->end = map->end;
							bdata->curentry = MAX_BANK_ENTRIES;
						}
					}
				}

				/* now loop over all the handlers and enforce the address mask (which may have changed) */
				for (entrynum = 0; entrynum < ENTRY_COUNT; entrynum++)
				{
					space->read.handlers[entrynum].mask &= space->mask;
					space->write.handlers[entrynum].mask &= space->mask;
				}
			}
}


/*-------------------------------------------------
    populate_memory - populate the memory mapping
    tables with entries
-------------------------------------------------*/

static void populate_memory(void)
{
	int cpunum, spacenum;

	/* loop over CPUs and address spaces */
	for (cpunum = 0; cpunum < MAX_CPU && Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].spacemask & (1 << spacenum))
			{
				addrspace_data *space = &cpudata[cpunum].space[spacenum];
				const address_map *map;

				/* install the handlers, using the original, unadjusted memory map */
				if (space->map != NULL)
				{
					/* first find the end */
					for (map = space->map; !IS_AMENTRY_END(map); map++) ;

					/* then work backwards, populating the address map */
					for (map--; map >= space->map; map--)
						if (!IS_AMENTRY_EXTENDED(map))
						{
							int ismatchmask = ((map->flags & AM_FLAGS_MATCH_MASK) != 0);
							int isfixed = (map->memory != NULL) || (map->share != 0);
							if (map->read.handler != NULL)
								install_mem_handler_private(space, 0, space->dbits, ismatchmask, map->start, map->end, map->mask, map->mirror, map->read.handler, isfixed, map->read_name);
							if (map->write.handler != NULL)
								install_mem_handler_private(space, 1, space->dbits, ismatchmask, map->start, map->end, map->mask, map->mirror, map->write.handler, isfixed, map->write_name);
						}
				}
			}
}


/*-------------------------------------------------
    install_mem_handler_private - wrapper for
    install_mem_handler which is used at
    initialization time and converts RAM/ROM
    banks to dynamically assigned banks
-------------------------------------------------*/

static void install_mem_handler_private(addrspace_data *space, int iswrite, int databits, int ismatchmask, offs_t start, offs_t end, offs_t mask, offs_t mirror, genf *handler, int isfixed, const char *handler_name)
{
	/* translate ROM to RAM/UNMAP here */
	if (HANDLER_IS_ROM(handler))
		handler = iswrite ? (genf *)STATIC_UNMAP : (genf *)MRA8_RAM;

	/* assign banks for RAM/ROM areas */
	if (HANDLER_IS_RAM(handler))
	{
		int ismasked = (mask != 0);
		offs_t temp_start = start;
		offs_t temp_end = end;
		offs_t temp_mask = mask;
		offs_t temp_mirror = mirror;

		/* adjust the incoming addresses (temporarily) */
		adjust_addresses(space, ismatchmask, &temp_start, &temp_end, &temp_mask, &temp_mirror);

		/* assign a bank to the adjusted addresses */
		handler = (genf *)assign_dynamic_bank(space->cpunum, space->spacenum, temp_start, temp_end, temp_mirror, isfixed, ismasked);
		if (!bank_ptr[HANDLER_TO_BANK(handler)])
			bank_ptr[HANDLER_TO_BANK(handler)] = memory_find_base(space->cpunum, space->spacenum, iswrite, temp_start);
	}

	/* then do a normal installation */
	install_mem_handler(space, iswrite, databits, ismatchmask, start, end, mask, mirror, handler, isfixed, handler_name);
}


/*-------------------------------------------------
    install_mem_handler - installs a handler for
    memory operations
-------------------------------------------------*/

static void install_mem_handler(addrspace_data *space, int iswrite, int databits, int ismatchmask, offs_t start, offs_t end, offs_t mask, offs_t mirror, genf *handler, int isfixed, const char *handler_name)
{
	offs_t lmirrorbit[LEVEL2_BITS], lmirrorbits, hmirrorbit[32 - LEVEL2_BITS], hmirrorbits, lmirrorcount, hmirrorcount;
	table_data *tabledata = iswrite ? &space->write : &space->read;
	UINT8 idx, prev_entry = STATIC_INVALID;
	int cur_index, prev_index = 0;
	int i;

	/* sanity check */
	if (HANDLER_IS_ROM(handler) || HANDLER_IS_RAM(handler))
		fatalerror("fatal: install_mem_handler called with ROM or RAM after initialization");
	if (space->dbits != databits)
		fatalerror("fatal: install_mem_handler called with a %d-bit handler for a %d-bit address space", databits, space->dbits);
	if (start > end)
		fatalerror("fatal: install_mem_handler called with start greater than end");

	/* if we're installing a new bank, make sure we mark it */
	if (HANDLER_IS_BANK(handler))
	{
		bank_data *bdata = &bankdata[HANDLER_TO_BANK(handler)];

		/* if this is the first time we've seen this bank, create a new entry */
		if (!bdata->used)
		{
			bdata->used = TRUE;
			bdata->dynamic = FALSE;
			bdata->cpunum = space->cpunum;
			bdata->spacenum = space->spacenum;
			bdata->base = start;
			bdata->end = end;
			bdata->curentry = MAX_BANK_ENTRIES;

			/* if we're allowed to, wire up state saving for the entry */
			if (state_save_registration_allowed())
				state_save_register_item("memory", HANDLER_TO_BANK(handler), bdata->curentry);

			VPRINTF(("Allocated new bank %d\n", HANDLER_TO_BANK(handler)));
		}
	}

	/* adjust the incoming addresses */
	adjust_addresses(space, ismatchmask, &start, &end, &mask, &mirror);

	/* if this ended up a bank handler, tag it for reads or writes */
	if (HANDLER_IS_BANK(handler))
	{
		bank_data *bdata = &bankdata[HANDLER_TO_BANK(handler)];

		/* track whether reads or writes are going here */
		if (!iswrite)
			bdata->read = TRUE;
		else
			bdata->write = TRUE;
	}

	/* determine the mirror bits */
	hmirrorbits = lmirrorbits = 0;
	for (i = 0; i < LEVEL2_BITS; i++)
		if (mirror & (1 << i))
			lmirrorbit[lmirrorbits++] = 1 << i;
	for (i = LEVEL2_BITS; i < 32; i++)
		if (mirror & (1 << i))
			hmirrorbit[hmirrorbits++] = 1 << i;

	/* get the final handler index */
	idx = get_handler_index(tabledata->handlers, handler, handler_name, start, end, mask);

	/* loop over mirrors in the level 2 table */
	for (hmirrorcount = 0; hmirrorcount < (1 << hmirrorbits); hmirrorcount++)
	{
		/* compute the base of this mirror */
		offs_t hmirrorbase = 0;
		for (i = 0; i < hmirrorbits; i++)
			if (hmirrorcount & (1 << i))
				hmirrorbase |= hmirrorbit[i];

		/* if this is not our first time through, and the level 2 entry matches the previous
           level 2 entry, just do a quick map and get out; note that this only works for entries
           which don't span multiple level 1 table entries */
		cur_index = LEVEL1_INDEX(start + hmirrorbase);
		if (cur_index == LEVEL1_INDEX(end + hmirrorbase))
		{
			if (hmirrorcount != 0 && prev_entry == tabledata->table[cur_index])
			{
				VPRINTF(("Quick mapping subtable at %08X to match subtable at %08X\n", cur_index << LEVEL2_BITS, prev_index << LEVEL2_BITS));

				/* release the subtable if the old value was a subtable */
				if (tabledata->table[cur_index] >= SUBTABLE_BASE)
					release_subtable(tabledata, tabledata->table[cur_index]);

				/* reallocate the subtable if the new value is a subtable */
				if (tabledata->table[prev_index] >= SUBTABLE_BASE)
					reallocate_subtable(tabledata, tabledata->table[prev_index]);

				/* set the new value and short-circuit the mapping step */
				tabledata->table[cur_index] = tabledata->table[prev_index];
				continue;
			}
			prev_index = cur_index;
			prev_entry = tabledata->table[cur_index];
		}

		/* loop over mirrors in the level 1 table */
		for (lmirrorcount = 0; lmirrorcount < (1 << lmirrorbits); lmirrorcount++)
		{
			/* compute the base of this mirror */
			offs_t lmirrorbase = hmirrorbase;
			for (i = 0; i < lmirrorbits; i++)
				if (lmirrorcount & (1 << i))
					lmirrorbase |= lmirrorbit[i];

			/* populate the tables */
			if (!ismatchmask)
				populate_table_range(space, iswrite, start + lmirrorbase, end + lmirrorbase, idx);
			else
				populate_table_match(space, iswrite, start + lmirrorbase, end + lmirrorbase, idx);
		}
	}

	/* if this is being installed to a live CPU, update the context */
	if (space->cpunum == cur_context)
		memory_set_context(cur_context);
}


/*-------------------------------------------------
    assign_dynamic_bank - finds a free or exact
    matching bank
-------------------------------------------------*/

static genf *assign_dynamic_bank(int cpunum, int spacenum, offs_t start, offs_t end, offs_t mirror, int isfixed, int ismasked)
{
	int bank;

	/* loop over banks, searching for an exact match or an empty */
	for (bank = MAX_BANKS; bank >= 1; bank--)
		if (!bankdata[bank].used || (bankdata[bank].dynamic && bankdata[bank].cpunum == cpunum && bankdata[bank].spacenum == spacenum && bankdata[bank].base == start))
		{
			bankdata[bank].used = TRUE;
			bankdata[bank].dynamic = TRUE;
			bankdata[bank].cpunum = cpunum;
			bankdata[bank].spacenum = spacenum;
			bankdata[bank].base = start;
			bankdata[bank].end = end;
			VPRINTF(("Assigned bank %d to %d,%d,%08X\n", bank, cpunum, spacenum, start));
			return BANK_TO_HANDLER(bank);
		}

	/* if we got here, we failed */
	fatalerror("cpu #%d: ran out of banks for RAM/ROM regions!", cpunum);
	return NULL;
}


/*-------------------------------------------------
    get_handler_index - finds the index of a
    handler, or allocates a new one as necessary
-------------------------------------------------*/

static UINT8 get_handler_index(handler_data *table, genf *handler, const char *handler_name, offs_t start, offs_t end, offs_t mask)
{
	int i;

	start &= mask;

	/* all static handlers are hardcoded */
	if (HANDLER_IS_STATIC(handler))
	{
		i = (FPTR)handler;
		if (HANDLER_IS_BANK(handler))
		{
			table[i].offset = start;
			table[i].top = end;
			table[i].mask = mask;
			table[i].name = handler_name;
		}
		return i;
	}

	/* otherwise, we have to search */
	for (i = STATIC_COUNT; i < SUBTABLE_BASE; i++)
	{
		if (table[i].handler.generic == NULL)
		{
			table[i].handler.generic = handler;
			table[i].offset = start;
			table[i].top = end;
			table[i].mask = mask;
			table[i].name = handler_name;
			return i;
		}
		if (table[i].handler.generic == handler && table[i].offset == start && table[i].mask == mask)
			return i;
	}
	return 0;
}


/*-------------------------------------------------
    populate_table_range - assign a memory handler
    to a range of addresses
-------------------------------------------------*/

static void populate_table_range(addrspace_data *space, int iswrite, offs_t start, offs_t stop, UINT8 handler)
{
	table_data *tabledata = iswrite ? &space->write : &space->read;
	offs_t l2mask = (1 << LEVEL2_BITS) - 1;
	offs_t l1start = start >> LEVEL2_BITS;
	offs_t l2start = start & l2mask;
	offs_t l1stop = stop >> LEVEL2_BITS;
	offs_t l2stop = stop & l2mask;
	offs_t l1index;

	/* sanity check */
	if (start > stop)
		return;

	/* handle the starting edge if it's not on a block boundary */
	if (l2start != 0)
	{
		UINT8 *subtable = open_subtable(tabledata, l1start);

		/* if the start and stop end within the same block, handle that */
		if (l1start == l1stop)
		{
			memset(&subtable[l2start], handler, l2stop - l2start + 1);
			close_subtable(tabledata, l1start);
			return;
		}

		/* otherwise, fill until the end */
		memset(&subtable[l2start], handler, (1 << LEVEL2_BITS) - l2start);
		close_subtable(tabledata, l1start);
		if (l1start != (offs_t)~0) l1start++;
	}

	/* handle the trailing edge if it's not on a block boundary */
	if (l2stop != l2mask)
	{
		UINT8 *subtable = open_subtable(tabledata, l1stop);

		/* fill from the beginning */
		memset(&subtable[0], handler, l2stop + 1);
		close_subtable(tabledata, l1stop);

		/* if the start and stop end within the same block, handle that */
		if (l1start == l1stop)
			return;
		if (l1stop != 0) l1stop--;
	}

	/* now fill in the middle tables */
	for (l1index = l1start; l1index <= l1stop; l1index++)
	{
		/* if we have a subtable here, release it */
		if (tabledata->table[l1index] >= SUBTABLE_BASE)
			release_subtable(tabledata, tabledata->table[l1index]);
		tabledata->table[l1index] = handler;
	}
}


/*-------------------------------------------------
    populate_table_match - assign a memory handler
    to a range of addresses
-------------------------------------------------*/

static void populate_table_match(addrspace_data *space, int iswrite, offs_t matchval, offs_t matchmask, UINT8 handler)
{
	table_data *tabledata = iswrite ? &space->write : &space->read;
	int lowermask, lowermatch;
	int uppermask, uppermatch;
	int l1index, l2index;

	/* clear out any ignored bits in the matchval */
	matchval &= matchmask;

	/* compute the lower half of the match/mask pair */
	lowermask = matchmask & ((1<<LEVEL2_BITS)-1);
	lowermatch = matchval & ((1<<LEVEL2_BITS)-1);

	/* compute the upper half of the match/mask pair */
	uppermask = matchmask >> LEVEL2_BITS;
	uppermatch = matchval >> LEVEL2_BITS;

	/* if the lower bits of the mask are all 0, we can work exclusively at the top level */
	if (lowermask == 0)
	{
		/* loop over top level matches */
		for (l1index = 0; l1index <= (space->mask >> LEVEL2_BITS); l1index++)
			if ((l1index & uppermatch) == uppermask)
			{
				/* if we have a subtable here, release it */
				if (tabledata->table[l1index] >= SUBTABLE_BASE)
					release_subtable(tabledata, tabledata->table[l1index]);
				tabledata->table[l1index] = handler;
			}
	}

	/* okay, we need to work at both levels */
	else
	{
		/* loop over top level matches */
		for (l1index = 0; l1index <= (space->mask >> LEVEL2_BITS); l1index++)
			if ((l1index & uppermatch) == uppermask)
			{
				UINT8 *subtable = open_subtable(tabledata, l1index);

				/* now loop over lower level matches */
				for (l2index = 0; l2index < (1 << LEVEL2_BITS); l2index++)
					if ((l2index & lowermask) == lowermatch)
						subtable[l2index] = handler;
				close_subtable(tabledata, l1index);
			}
	}
}


/*-------------------------------------------------
    allocate_subtable - allocate a fresh subtable
    and set its usecount to 1
-------------------------------------------------*/

static UINT8 allocate_subtable(table_data *tabledata)
{
	/* loop */
	while (1)
	{
		UINT8 subindex;

		/* find a subtable with a usecount of 0 */
		for (subindex = 0; subindex < SUBTABLE_COUNT; subindex++)
			if (tabledata->subtable[subindex].usecount == 0)
			{
				/* if this is past our allocation budget, allocate some more */
				if (subindex >= tabledata->subtable_alloc)
				{
					tabledata->subtable_alloc += SUBTABLE_ALLOC;
					tabledata->table = realloc(tabledata->table, (1 << LEVEL1_BITS) + (tabledata->subtable_alloc << LEVEL2_BITS));
					if (!tabledata->table)
						fatalerror("error: ran out of memory allocating memory subtable");
				}

				/* bump the usecount and return */
				tabledata->subtable[subindex].usecount++;
				return subindex + SUBTABLE_BASE;
			}

		/* merge any subtables we can */
		if (!merge_subtables(tabledata))
			fatalerror("Ran out of subtables!");
	}

	/* hopefully this never happens */
	return 0;
}


/*-------------------------------------------------
    reallocate_subtable - increment the usecount on
    a subtable
-------------------------------------------------*/

static void reallocate_subtable(table_data *tabledata, UINT8 subentry)
{
	UINT8 subindex = subentry - SUBTABLE_BASE;

	/* sanity check */
	if (tabledata->subtable[subindex].usecount <= 0)
		fatalerror("Called reallocate_subtable on a table with a usecount of 0");

	/* increment the usecount */
	tabledata->subtable[subindex].usecount++;
}


/*-------------------------------------------------
    merge_subtables - merge any duplicate
    subtables
-------------------------------------------------*/

static int merge_subtables(table_data *tabledata)
{
	int merged = 0;
	UINT8 subindex;

	VPRINTF(("Merging subtables....\n"));

	/* okay, we failed; update all the checksums and merge tables */
	for (subindex = 0; subindex < SUBTABLE_COUNT; subindex++)
		if (!tabledata->subtable[subindex].checksum_valid && tabledata->subtable[subindex].usecount != 0)
		{
			UINT32 *subtable = (UINT32 *)SUBTABLE_PTR(tabledata, subindex + SUBTABLE_BASE);
			UINT32 checksum = 0;
			int l2index;

			/* update the checksum */
			for (l2index = 0; l2index < (1 << LEVEL2_BITS)/4; l2index++)
				checksum += subtable[l2index];
			tabledata->subtable[subindex].checksum = checksum;
			tabledata->subtable[subindex].checksum_valid = 1;
		}

	/* see if there's a matching checksum */
	for (subindex = 0; subindex < SUBTABLE_COUNT; subindex++)
		if (tabledata->subtable[subindex].usecount != 0)
		{
			UINT8 *subtable = SUBTABLE_PTR(tabledata, subindex + SUBTABLE_BASE);
			UINT32 checksum = tabledata->subtable[subindex].checksum;
			UINT8 sumindex;

			for (sumindex = subindex + 1; sumindex < SUBTABLE_COUNT; sumindex++)
				if (tabledata->subtable[sumindex].usecount != 0 &&
					tabledata->subtable[sumindex].checksum == checksum &&
					!memcmp(subtable, SUBTABLE_PTR(tabledata, sumindex + SUBTABLE_BASE), 1 << LEVEL2_BITS))
				{
					int l1index;

					VPRINTF(("Merging subtable %d and %d....\n", subindex, sumindex));

					/* find all the entries in the L1 tables that pointed to the old one, and point them to the merged table */
					for (l1index = 0; l1index <= (0xffffffffUL >> LEVEL2_BITS); l1index++)
						if (tabledata->table[l1index] == sumindex + SUBTABLE_BASE)
						{
							release_subtable(tabledata, sumindex + SUBTABLE_BASE);
							reallocate_subtable(tabledata, subindex + SUBTABLE_BASE);
							tabledata->table[l1index] = subindex + SUBTABLE_BASE;
							merged++;
						}
				}
		}

	return merged;
}


/*-------------------------------------------------
    release_subtable - decrement the usecount on
    a subtable and free it if we're done
-------------------------------------------------*/

static void release_subtable(table_data *tabledata, UINT8 subentry)
{
	UINT8 subindex = subentry - SUBTABLE_BASE;

	/* sanity check */
	if (tabledata->subtable[subindex].usecount <= 0)
		fatalerror("Called release_subtable on a table with a usecount of 0");

	/* decrement the usecount and clear the checksum if we're at 0 */
	tabledata->subtable[subindex].usecount--;
	if (tabledata->subtable[subindex].usecount == 0)
		tabledata->subtable[subindex].checksum = 0;
}


/*-------------------------------------------------
    open_subtable - gain access to a subtable for
    modification
-------------------------------------------------*/

static UINT8 *open_subtable(table_data *tabledata, offs_t l1index)
{
	UINT8 subentry = tabledata->table[l1index];

	/* if we don't have a subtable yet, allocate a new one */
	if (subentry < SUBTABLE_BASE)
	{
		UINT8 newentry = allocate_subtable(tabledata);
		memset(SUBTABLE_PTR(tabledata, newentry), subentry, 1 << LEVEL2_BITS);
		tabledata->table[l1index] = newentry;
		tabledata->subtable[newentry - SUBTABLE_BASE].checksum = (subentry + (subentry << 8) + (subentry << 16) + (subentry << 24)) * ((1 << LEVEL2_BITS)/4);
		subentry = newentry;
	}

	/* if we're sharing this subtable, we also need to allocate a fresh copy */
	else if (tabledata->subtable[subentry - SUBTABLE_BASE].usecount > 1)
	{
		UINT8 newentry = allocate_subtable(tabledata);

		/* allocate may cause some additional merging -- look up the subentry again */
		/* when we're done; it should still require a split */
		subentry = tabledata->table[l1index];
		assert(subentry >= SUBTABLE_BASE);
		assert(tabledata->subtable[subentry - SUBTABLE_BASE].usecount > 1);

		memcpy(SUBTABLE_PTR(tabledata, newentry), SUBTABLE_PTR(tabledata, subentry), 1 << LEVEL2_BITS);
		release_subtable(tabledata, subentry);
		tabledata->table[l1index] = newentry;
		tabledata->subtable[newentry - SUBTABLE_BASE].checksum = tabledata->subtable[subentry - SUBTABLE_BASE].checksum;
		subentry = newentry;
	}

	/* mark the table dirty */
	tabledata->subtable[subentry - SUBTABLE_BASE].checksum_valid = 0;

	/* return the pointer to the subtable */
	return SUBTABLE_PTR(tabledata, subentry);
}


/*-------------------------------------------------
    close_subtable - stop access to a subtable
-------------------------------------------------*/

static void close_subtable(table_data *tabledata, offs_t l1index)
{
	/* defer any merging until we run out of tables */
}


/*-------------------------------------------------
    Return whether a given memory map entry implies
    the need of allocating and registering memory
-------------------------------------------------*/

static int amentry_needs_backing_store(int cpunum, int spacenum, const address_map *map)
{
	FPTR handler;

	if (IS_AMENTRY_EXTENDED(map))
		return 0;
	if (map->base)
		return 1;

	handler = (FPTR)map->write.handler;
	if (handler >= 0 && handler < STATIC_COUNT)
	{
		if (handler != STATIC_INVALID &&
			handler != STATIC_ROM &&
			handler != STATIC_NOP &&
			handler != STATIC_UNMAP)
			return 1;
	}

	handler = (FPTR)map->read.handler;
	if (handler >= 0 && handler < STATIC_COUNT)
	{
		if (handler != STATIC_INVALID &&
			(handler < STATIC_BANK1 || handler > STATIC_BANK1 + MAX_BANKS - 1) &&
			(handler != STATIC_ROM || spacenum != ADDRESS_SPACE_PROGRAM || map->start >= memory_region_length(REGION_CPU1 + cpunum)) &&
			handler != STATIC_NOP &&
			handler != STATIC_UNMAP)
			return 1;
	}

	return 0;
}


/*-------------------------------------------------
    allocate_memory - allocate memory for
    CPU address spaces
-------------------------------------------------*/

static void allocate_memory(void)
{
	int cpunum, spacenum;

	/* loop over all CPUs and memory spaces */
	for (cpunum = 0; cpunum < MAX_CPU && Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].spacemask & (1 << spacenum))
			{
				addrspace_data *space = &cpudata[cpunum].space[spacenum];
				address_map *map, *unassigned = NULL;
				int start_count = memory_block_count;
				int i;

				/* make a first pass over the memory map and track blocks with hardcoded pointers */
				/* we do this to make sure they are found by memory_find_base first */
				for (map = space->adjmap; map && !IS_AMENTRY_END(map); map++)
					if (!IS_AMENTRY_EXTENDED(map) && map->memory != NULL)
					{
						if (!IS_AMENTRY_MATCH_MASK(map))
							allocate_memory_block(cpunum, spacenum, map->start, map->end, map->memory);
						else
							allocate_memory_block(cpunum, spacenum, map->start, map->start + map->mask, map->memory);
					}

				/* loop over all blocks just allocated and assign pointers from them */
				for (i = start_count; i < memory_block_count; i++)
					unassigned = assign_intersecting_blocks(space, memory_block_list[i].start, memory_block_list[i].end, memory_block_list[i].data);

				/* if we don't have an unassigned pointer yet, try to find one */
				if (!unassigned)
					unassigned = assign_intersecting_blocks(space, ~0, 0, NULL);

				/* loop until we've assigned all memory in this space */
				while (unassigned)
				{
					offs_t curstart, curend;
					int changed;
					void *block;

					/* work in MEMORY_BLOCK_SIZE-sized chunks */
					curstart = unassigned->start / MEMORY_BLOCK_SIZE;
					if (!IS_AMENTRY_MATCH_MASK(unassigned))
						curend = unassigned->end / MEMORY_BLOCK_SIZE;
					else
						curend = (unassigned->start + unassigned->mask) / MEMORY_BLOCK_SIZE;

					/* loop while we keep finding unassigned blocks in neighboring MEMORY_BLOCK_SIZE chunks */
					do
					{
						changed = 0;

						/* scan for unmapped blocks in the adjusted map */
						for (map = space->adjmap; map && !IS_AMENTRY_END(map); map++)
							if (!IS_AMENTRY_EXTENDED(map) && map->memory == NULL && map != unassigned && amentry_needs_backing_store(cpunum, spacenum, map))
							{
								offs_t blockstart, blockend;

								/* get block start/end blocks for this block */
								blockstart = map->start / MEMORY_BLOCK_SIZE;
								if (!IS_AMENTRY_MATCH_MASK(map))
									blockend = map->end / MEMORY_BLOCK_SIZE;
								else
									blockend = (map->start + map->mask) / MEMORY_BLOCK_SIZE;

								/* if we intersect or are adjacent, adjust the start/end */
								if (blockstart <= curend + 1 && blockend >= curstart - 1)
								{
									if (blockstart < curstart)
										curstart = blockstart, changed = 1;
									if (blockend > curend)
										curend = blockend, changed = 1;
								}
							}
					} while (changed);

					/* we now have a block to allocate; do it */
					curstart = curstart * MEMORY_BLOCK_SIZE;
					curend = curend * MEMORY_BLOCK_SIZE + (MEMORY_BLOCK_SIZE - 1);
					block = allocate_memory_block(cpunum, spacenum, curstart, curend, NULL);

					/* assign memory that intersected the new block */
					unassigned = assign_intersecting_blocks(space, curstart, curend, block);
				}
			}
}


/*-------------------------------------------------
    allocate_memory_block - allocate a single
    memory block of data
-------------------------------------------------*/

static void *allocate_memory_block(int cpunum, int spacenum, offs_t start, offs_t end, void *memory)
{
	memory_block *block = &memory_block_list[memory_block_count];
	int allocatemem = (memory == NULL);
	int region;

	VPRINTF(("allocate_memory_block(%d,%d,%08X,%08X,%p)\n", cpunum, spacenum, start, end, memory));

	/* if we weren't passed a memory block, allocate one and clear it to zero */
	if (allocatemem)
	{
		memory = auto_malloc(end - start + 1);
		memset(memory, 0, end - start + 1);
	}

	/* register for saving, but only if we're not part of a memory region */
	for (region = 0; region < MAX_MEMORY_REGIONS; region++)
	{
		UINT8 *region_base = memory_region(region);
		UINT32 region_length = memory_region_length(region);
		if (region_base != NULL && region_length != 0 && (UINT8 *)memory >= region_base && ((UINT8 *)memory + (end - start + 1)) < region_base + region_length)
		{
			VPRINTF(("skipping save of this memory block as it is covered by a memory region\n"));
			break;
		}
	}
	if (region == MAX_MEMORY_REGIONS)
		register_for_save(cpunum, spacenum, start, memory, end - start + 1);

	/* fill in the tracking block */
	block->cpunum = cpunum;
	block->spacenum = spacenum;
	block->isallocated = allocatemem;
	block->start = start;
	block->end = end;
	block->data = memory;
	memory_block_count++;
	return memory;
}


/*-------------------------------------------------
    register_for_save - register a block of
    memory for save states
-------------------------------------------------*/

static void register_for_save(int cpunum, int spacenum, offs_t start, void *base, size_t numbytes)
{
	int bytes_per_element = cpudata[cpunum].space[spacenum].dbits/8;
	char name[256];

	sprintf(name, "%d.%08x-%08x", spacenum, start, (int)(start + numbytes - 1));
	state_save_register_memory("memory", cpunum, name, base, bytes_per_element, (UINT32)numbytes / bytes_per_element);
}


/*-------------------------------------------------
    assign_intersecting_blocks - find all
    intersecting blocks and assign their pointers
-------------------------------------------------*/

static address_map *assign_intersecting_blocks(addrspace_data *space, offs_t start, offs_t end, UINT8 *base)
{
	address_map *map, *unassigned = NULL;

	/* loop over the adjusted map and assign memory to any blocks we can */
	for (map = space->adjmap; map && !IS_AMENTRY_END(map); map++)
		if (!IS_AMENTRY_EXTENDED(map))
		{
			/* if we haven't assigned this block yet, do it against the last block */
			if (map->memory == NULL)
			{
				/* inherit shared pointers first */
				if (map->share && shared_ptr[map->share])
				{
					map->memory = shared_ptr[map->share];
	 				VPRINTF(("memory range %08X-%08X -> shared_ptr[%d] [%p]\n", map->start, map->end, map->share, map->memory));
	 			}

				/* otherwise, look for a match in this block */
				else
				{
					if (!IS_AMENTRY_MATCH_MASK(map))
					{
						if (map->start >= start && map->end <= end)
						{
							map->memory = base + (map->start - start);
	 						VPRINTF(("memory range %08X-%08X -> found in block from %08X-%08X [%p]\n", map->start, map->end, start, end, map->memory));
	 					}
					}
					else
					{
						if (map->start >= start && map->start + map->mask <= end)
						{
							map->memory = base + (map->start - start);
	 						VPRINTF(("memory range %08X-%08X -> found in block from %08X-%08X [%p]\n", map->start, map->end, start, end, map->memory));
	 					}
					}
				}
			}

			/* if we're the first match on a shared pointer, assign it now */
			if (map->memory != NULL && map->share && !shared_ptr[map->share])
				shared_ptr[map->share] = map->memory;

			/* keep track of the first unassigned entry */
			if (map->memory == NULL && !unassigned && amentry_needs_backing_store(space->cpunum, space->spacenum, map))
				unassigned = map;
		}

	return unassigned;
}


/*-------------------------------------------------
    reattach_banks - reconnect banks after a load
-------------------------------------------------*/

static void reattach_banks(void)
{
	int banknum;

	/* once this is done, find the starting bases for the banks */
	for (banknum = 1; banknum <= MAX_BANKS; banknum++)
		if (bankdata[banknum].used && !bankdata[banknum].dynamic)
		{
			/* if this entry has a changed entry, set the appropriate pointer */
			if (bankdata[banknum].curentry != MAX_BANK_ENTRIES)
				bank_ptr[banknum] = bankdata[banknum].entry[bankdata[banknum].curentry];
		}
}


/*-------------------------------------------------
    find_memory - find all the requested pointers
    into the final allocated memory
-------------------------------------------------*/

static void find_memory(void)
{
	int cpunum, spacenum, banknum;

	/* loop over CPUs and address spaces */
	for (cpunum = 0; cpunum < MAX_CPU && Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].spacemask & (1 << spacenum))
			{
				addrspace_data *space = &cpudata[cpunum].space[spacenum];
				const address_map *map;

				/* fill in base/size entries, and handle shared memory */
				for (map = space->adjmap; map && !IS_AMENTRY_END(map); map++)
					if (!IS_AMENTRY_EXTENDED(map))
					{
						/* assign base/size values */
						if (map->base != NULL)
							*map->base = map->memory;
						if (map->size)
						{
							if (!IS_AMENTRY_MATCH_MASK(map))
								*map->size = map->end - map->start + 1;
							else
								*map->size = map->mask + 1;
						}
					}
			}

	/* once this is done, find the starting bases for the banks */
	for (banknum = 1; banknum <= MAX_BANKS; banknum++)
		if (bankdata[banknum].used)
		{
			address_map *map;

			/* set the initial bank pointer */
			for (map = cpudata[bankdata[banknum].cpunum].space[bankdata[banknum].spacenum].adjmap; map && !IS_AMENTRY_END(map); map++)
				if (!IS_AMENTRY_EXTENDED(map) && map->start == bankdata[banknum].base)
				{
					bank_ptr[banknum] = map->memory;
	 				VPRINTF(("assigned bank %d pointer to memory from range %08X-%08X [%p]\n", banknum, map->start, map->end, map->memory));
					break;
				}

			/* if the entry was set ahead of time, override the automatically found pointer */
			if (!bankdata[banknum].dynamic && bankdata[banknum].curentry != MAX_BANK_ENTRIES)
				bank_ptr[banknum] = bankdata[banknum].entry[bankdata[banknum].curentry];
		}

	/* request a callback to fix up the banks when done */
	state_save_register_func_postload(reattach_banks);
}


/*-------------------------------------------------
    memory_find_base - return a pointer to the
    base of RAM associated with the given CPU
    and offset
-------------------------------------------------*/

static void *memory_find_base(int cpunum, int spacenum, int readwrite, offs_t offset)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	address_map *map;
	memory_block *block;
	int blocknum;

	VPRINTF(("memory_find_base(%d,%d,%d,%08X) -> ", cpunum, spacenum, readwrite, offset));

	/* look in the adjusted map */
	for (map = space->adjmap; map && !IS_AMENTRY_END(map); map++)
		if (!IS_AMENTRY_EXTENDED(map))
		{
			offs_t maskoffs = offset & map->mask;
			if (!IS_AMENTRY_MATCH_MASK(map))
			{
				if (maskoffs >= map->start && maskoffs <= map->end)
				{
					VPRINTF(("found in entry %08X-%08X [%p]\n", map->start, map->end, (UINT8 *)map->memory + (maskoffs - map->start)));
					return (UINT8 *)map->memory + (maskoffs - map->start);
				}
			}
			else
			{
				if ((maskoffs & map->end) == map->start)
				{
					VPRINTF(("found in entry %08X-%08X [%p]\n", map->start, map->end, (UINT8 *)map->memory + (maskoffs - map->start)));
					return (UINT8 *)map->memory + (maskoffs - map->start);
				}
			}
		}

	/* if not found there, look in the allocated blocks */
	for (blocknum = 0, block = memory_block_list; blocknum < memory_block_count; blocknum++, block++)
		if (block->cpunum == cpunum && block->spacenum == spacenum && block->start <= offset && block->end > offset)
		{
			VPRINTF(("found in allocated memory block %08X-%08X [%p]\n", block->start, block->end, block->data + (offset - block->start)));
			return block->data + offset - block->start;
		}

	VPRINTF(("did not find\n"));
	return NULL;
}


/*-------------------------------------------------
    PERFORM_LOOKUP - common lookup procedure
-------------------------------------------------*/

#define PERFORM_LOOKUP(lookup,space,extraand)											\
	/* perform lookup */																\
	address &= space.addrmask & extraand;												\
	entry = space.lookup[LEVEL1_INDEX(address)];										\
	if (entry >= SUBTABLE_BASE)															\
		entry = space.lookup[LEVEL2_INDEX(entry,address)];								\


/*-------------------------------------------------
    READBYTE - generic byte-sized read handler
-------------------------------------------------*/

#define READBYTE8(name,spacenum)														\
UINT8 name(offs_t original_address)														\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~0);						\
	DEBUG_HOOK_READ(spacenum, 1, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM) 															\
		MEMREADEND(bank_ptr[entry][address]);											\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handler8)(address));\
	return 0;																			\
}																						\

#define READBYTE(name,spacenum,xormacro,handlertype,ignorebits,shiftbytes,masktype)		\
UINT8 name(offs_t original_address)														\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~0);						\
	DEBUG_HOOK_READ(spacenum, 1, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMREADEND(bank_ptr[entry][xormacro(address)]);									\
																						\
	/* fall back to the handler */														\
	else																				\
	{																					\
		int shift = 8 * (shiftbytes);													\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handlertype)(address >> (ignorebits), ~((masktype)0xff << shift)) >> shift);\
	}																					\
	return 0;																			\
}																						\

#define READBYTE16BE(name,space)	READBYTE(name,space,BYTE_XOR_BE, handler16,1,~address & 1,UINT16)
#define READBYTE16LE(name,space)	READBYTE(name,space,BYTE_XOR_LE, handler16,1, address & 1,UINT16)
#define READBYTE32BE(name,space)	READBYTE(name,space,BYTE4_XOR_BE,handler32,2,~address & 3,UINT32)
#define READBYTE32LE(name,space)	READBYTE(name,space,BYTE4_XOR_LE,handler32,2, address & 3,UINT32)
#define READBYTE64BE(name,space)	READBYTE(name,space,BYTE8_XOR_BE,handler64,3,~address & 7,UINT64)
#define READBYTE64LE(name,space)	READBYTE(name,space,BYTE8_XOR_LE,handler64,3, address & 7,UINT64)


/*-------------------------------------------------
    READWORD - generic word-sized read handler
    (16-bit, 32-bit and 64-bit aligned only!)
-------------------------------------------------*/

#define READWORD16(name,spacenum)														\
UINT16 name(offs_t original_address)													\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~1);						\
	DEBUG_HOOK_READ(spacenum, 2, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMREADEND(*(UINT16 *)&bank_ptr[entry][address]);								\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handler16)(address >> 1,0));\
	return 0;																			\
}																						\

#define READWORD(name,spacenum,xormacro,handlertype,ignorebits,shiftbytes,masktype)		\
UINT16 name(offs_t original_address)													\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~1);						\
	DEBUG_HOOK_READ(spacenum, 2, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMREADEND(*(UINT16 *)&bank_ptr[entry][xormacro(address)]);						\
																						\
	/* fall back to the handler */														\
	else																				\
	{																					\
		int shift = 8 * (shiftbytes);													\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handlertype)(address >> (ignorebits), ~((masktype)0xffff << shift)) >> shift);\
	}																					\
	return 0;																			\
}																						\

#define READWORD32BE(name,space)	READWORD(name,space,WORD_XOR_BE, handler32,2,~address & 2,UINT32)
#define READWORD32LE(name,space)	READWORD(name,space,WORD_XOR_LE, handler32,2, address & 2,UINT32)
#define READWORD64BE(name,space)	READWORD(name,space,WORD2_XOR_BE,handler64,3,~address & 6,UINT64)
#define READWORD64LE(name,space)	READWORD(name,space,WORD2_XOR_LE,handler64,3, address & 6,UINT64)


/*-------------------------------------------------
    READDWORD - generic dword-sized read handler
    (32-bit and 64-bit aligned only!)
-------------------------------------------------*/

#define READDWORD32(name,spacenum)														\
UINT32 name(offs_t original_address)													\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~3);						\
	DEBUG_HOOK_READ(spacenum, 4, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMREADEND(*(UINT32 *)&bank_ptr[entry][address]);								\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handler32)(address >> 2,0));\
	return 0;																			\
}																						\

#define READMASKED32(name,spacenum)														\
UINT32 name(offs_t original_address, UINT32 mem_mask)									\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~3);						\
	DEBUG_HOOK_READ(spacenum, 4, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMREADEND(*(UINT32 *)&bank_ptr[entry][address]);								\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handler32)(address >> 2, mem_mask));\
	return 0;																			\
}																						\

#define READDWORD(name,spacenum,xormacro,handlertype,ignorebits,shiftbytes,masktype)	\
UINT32 name(offs_t original_address)													\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~3);						\
	DEBUG_HOOK_READ(spacenum, 4, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMREADEND(*(UINT32 *)&bank_ptr[entry][xormacro(address)]);						\
																						\
	/* fall back to the handler */														\
	else																				\
	{																					\
		int shift = 8 * (shiftbytes);													\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handlertype)(address >> (ignorebits), ~((masktype)0xffffffff << shift)) >> shift);\
	}																					\
	return 0;																			\
}																						\

#define READDWORD64BE(name,space)	READDWORD(name,space,DWORD_XOR_BE,handler64,3,~address & 4,UINT64)
#define READDWORD64LE(name,space)	READDWORD(name,space,DWORD_XOR_LE,handler64,3, address & 4,UINT64)


/*-------------------------------------------------
    READQWORD - generic qword-sized read handler
    (64-bit aligned only!)
-------------------------------------------------*/

#define READQWORD64(name,spacenum)														\
UINT64 name(offs_t original_address)													\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~7);						\
	DEBUG_HOOK_READ(spacenum, 8, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMREADEND(*(UINT64 *)&bank_ptr[entry][address]);								\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handler64)(address >> 3,0));\
	return 0;																			\
}																						\

#define READMASKED64(name,spacenum)														\
UINT64 name(offs_t original_address, UINT64 mem_mask)									\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMREADSTART();																		\
	PERFORM_LOOKUP(readlookup,active_address_space[spacenum],~7);						\
	DEBUG_HOOK_READ(spacenum, 8, address);												\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].readhandlers[entry].offset) & active_address_space[spacenum].readhandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMREADEND(*(UINT64 *)&bank_ptr[entry][address]);								\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMREADEND((*active_address_space[spacenum].readhandlers[entry].handler.read.handler64)(address >> 3, mem_mask));\
	return 0;																			\
}																						\


/*-------------------------------------------------
    WRITEBYTE - generic byte-sized write handler
-------------------------------------------------*/

#define WRITEBYTE8(name,spacenum)														\
void name(offs_t original_address, UINT8 data)											\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~0);						\
	DEBUG_HOOK_WRITE(spacenum, 1, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMWRITEEND(bank_ptr[entry][address] = data);									\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handler8)(address, data));\
}																						\

#define WRITEBYTE(name,spacenum,xormacro,handlertype,ignorebits,shiftbytes,masktype)	\
void name(offs_t original_address, UINT8 data)											\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~0);						\
	DEBUG_HOOK_WRITE(spacenum, 1, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMWRITEEND(bank_ptr[entry][xormacro(address)] = data);							\
																						\
	/* fall back to the handler */														\
	else																				\
	{																					\
		int shift = 8 * (shiftbytes);													\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handlertype)(address >> (ignorebits), (masktype)data << shift, ~((masktype)0xff << shift)));\
	}																					\
}																						\

#define WRITEBYTE16BE(name,space)	WRITEBYTE(name,space,BYTE_XOR_BE, handler16,1,~address & 1,UINT16)
#define WRITEBYTE16LE(name,space)	WRITEBYTE(name,space,BYTE_XOR_LE, handler16,1, address & 1,UINT16)
#define WRITEBYTE32BE(name,space)	WRITEBYTE(name,space,BYTE4_XOR_BE,handler32,2,~address & 3,UINT32)
#define WRITEBYTE32LE(name,space)	WRITEBYTE(name,space,BYTE4_XOR_LE,handler32,2, address & 3,UINT32)
#define WRITEBYTE64BE(name,space)	WRITEBYTE(name,space,BYTE8_XOR_BE,handler64,3,~address & 7,UINT64)
#define WRITEBYTE64LE(name,space)	WRITEBYTE(name,space,BYTE8_XOR_LE,handler64,3, address & 7,UINT64)


/*-------------------------------------------------
    WRITEWORD - generic word-sized write handler
    (16-bit, 32-bit and 64-bit aligned only!)
-------------------------------------------------*/

#define WRITEWORD16(name,spacenum)														\
void name(offs_t original_address, UINT16 data)											\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~1);						\
	DEBUG_HOOK_WRITE(spacenum, 2, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMWRITEEND(*(UINT16 *)&bank_ptr[entry][address] = data);						\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handler16)(address >> 1, data, 0));\
}																						\

#define WRITEWORD(name,spacenum,xormacro,handlertype,ignorebits,shiftbytes,masktype)	\
void name(offs_t original_address, UINT16 data)											\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~1);						\
	DEBUG_HOOK_WRITE(spacenum, 2, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMWRITEEND(*(UINT16 *)&bank_ptr[entry][xormacro(address)] = data);				\
																						\
	/* fall back to the handler */														\
	else																				\
	{																					\
		int shift = 8 * (shiftbytes);													\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handlertype)(address >> (ignorebits), (masktype)data << shift, ~((masktype)0xffff << shift)));\
	}																					\
}																						\

#define WRITEWORD32BE(name,space)	WRITEWORD(name,space,WORD_XOR_BE, handler32,2,~address & 2,UINT32)
#define WRITEWORD32LE(name,space)	WRITEWORD(name,space,WORD_XOR_LE, handler32,2, address & 2,UINT32)
#define WRITEWORD64BE(name,space)	WRITEWORD(name,space,WORD2_XOR_BE,handler64,3,~address & 6,UINT64)
#define WRITEWORD64LE(name,space)	WRITEWORD(name,space,WORD2_XOR_LE,handler64,3, address & 6,UINT64)


/*-------------------------------------------------
    WRITEDWORD - dword-sized write handler
    (32-bit and 64-bit aligned only!)
-------------------------------------------------*/

#define WRITEDWORD32(name,spacenum)														\
void name(offs_t original_address, UINT32 data)											\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~3);						\
	DEBUG_HOOK_WRITE(spacenum, 4, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMWRITEEND(*(UINT32 *)&bank_ptr[entry][address] = data);						\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handler32)(address >> 2, data, 0));\
}																						\

#define WRITEMASKED32(name,spacenum)													\
void name(offs_t original_address, UINT32 data, UINT32 mem_mask)						\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~3);						\
	DEBUG_HOOK_WRITE(spacenum, 4, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
	{																					\
		UINT32 *dest = (UINT32 *)&bank_ptr[entry][address];								\
		MEMWRITEEND(*dest = (*dest & mem_mask) | (data & ~mem_mask));					\
	}																					\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handler32)(address >> 2, data, mem_mask));\
}																						\

#define WRITEDWORD(name,spacenum,xormacro,handlertype,ignorebits,shiftbytes,masktype)	\
void name(offs_t original_address, UINT32 data)											\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~3);						\
	DEBUG_HOOK_WRITE(spacenum, 4, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMWRITEEND(*(UINT32 *)&bank_ptr[entry][xormacro(address)] = data);				\
																						\
	/* fall back to the handler */														\
	else																				\
	{																					\
		int shift = 8 * (shiftbytes);													\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handlertype)(address >> (ignorebits), (masktype)data << shift, ~((masktype)0xffffffff << shift)));\
	}																					\
}																						\

#define WRITEDWORD64BE(name,space)	WRITEDWORD(name,space,DWORD_XOR_BE,handler64,3,~address & 4,UINT64)
#define WRITEDWORD64LE(name,space)	WRITEDWORD(name,space,DWORD_XOR_LE,handler64,3, address & 4,UINT64)


/*-------------------------------------------------
    WRITEQWORD - qword-sized write handler
    (64-bit aligned only!)
-------------------------------------------------*/

#define WRITEQWORD64(name,spacenum)														\
void name(offs_t original_address, UINT64 data)											\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~7);						\
	DEBUG_HOOK_WRITE(spacenum, 8, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
		MEMWRITEEND(*(UINT64 *)&bank_ptr[entry][address] = data);						\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handler64)(address >> 3, data, 0));\
}																						\

#define WRITEMASKED64(name,spacenum)													\
void name(offs_t original_address, UINT64 data, UINT64 mem_mask)						\
{																						\
	offs_t address = original_address;													\
	UINT32 entry;																		\
	MEMWRITESTART();																	\
	PERFORM_LOOKUP(writelookup,active_address_space[spacenum],~7);						\
	DEBUG_HOOK_WRITE(spacenum, 8, address, data);										\
																						\
	/* handle banks inline */															\
	address = (address - active_address_space[spacenum].writehandlers[entry].offset) & active_address_space[spacenum].writehandlers[entry].mask;\
	if (entry < STATIC_RAM)																\
	{																					\
		UINT64 *dest = (UINT64 *)&bank_ptr[entry][address];								\
		MEMWRITEEND(*dest = (*dest & mem_mask) | (data & ~mem_mask));					\
	}																					\
																						\
	/* fall back to the handler */														\
	else																				\
		MEMWRITEEND((*active_address_space[spacenum].writehandlers[entry].handler.write.handler64)(address >> 3, data, mem_mask));\
}																						\


/*-------------------------------------------------
    Program memory handlers
-------------------------------------------------*/

     READBYTE8(program_read_byte_8,      ADDRESS_SPACE_PROGRAM)
    WRITEBYTE8(program_write_byte_8,     ADDRESS_SPACE_PROGRAM)

  READBYTE16BE(program_read_byte_16be,   ADDRESS_SPACE_PROGRAM)
    READWORD16(program_read_word_16be,   ADDRESS_SPACE_PROGRAM)
 WRITEBYTE16BE(program_write_byte_16be,  ADDRESS_SPACE_PROGRAM)
   WRITEWORD16(program_write_word_16be,  ADDRESS_SPACE_PROGRAM)

  READBYTE16LE(program_read_byte_16le,   ADDRESS_SPACE_PROGRAM)
    READWORD16(program_read_word_16le,   ADDRESS_SPACE_PROGRAM)
 WRITEBYTE16LE(program_write_byte_16le,  ADDRESS_SPACE_PROGRAM)
   WRITEWORD16(program_write_word_16le,  ADDRESS_SPACE_PROGRAM)

  READBYTE32BE(program_read_byte_32be,   ADDRESS_SPACE_PROGRAM)
  READWORD32BE(program_read_word_32be,   ADDRESS_SPACE_PROGRAM)
   READDWORD32(program_read_dword_32be,  ADDRESS_SPACE_PROGRAM)
  READMASKED32(program_read_masked_32be, ADDRESS_SPACE_PROGRAM)
 WRITEBYTE32BE(program_write_byte_32be,  ADDRESS_SPACE_PROGRAM)
 WRITEWORD32BE(program_write_word_32be,  ADDRESS_SPACE_PROGRAM)
  WRITEDWORD32(program_write_dword_32be, ADDRESS_SPACE_PROGRAM)
 WRITEMASKED32(program_write_masked_32be,ADDRESS_SPACE_PROGRAM)

  READBYTE32LE(program_read_byte_32le,   ADDRESS_SPACE_PROGRAM)
  READWORD32LE(program_read_word_32le,   ADDRESS_SPACE_PROGRAM)
   READDWORD32(program_read_dword_32le,  ADDRESS_SPACE_PROGRAM)
  READMASKED32(program_read_masked_32le, ADDRESS_SPACE_PROGRAM)
 WRITEBYTE32LE(program_write_byte_32le,  ADDRESS_SPACE_PROGRAM)
 WRITEWORD32LE(program_write_word_32le,  ADDRESS_SPACE_PROGRAM)
  WRITEDWORD32(program_write_dword_32le, ADDRESS_SPACE_PROGRAM)
 WRITEMASKED32(program_write_masked_32le,ADDRESS_SPACE_PROGRAM)

  READBYTE64BE(program_read_byte_64be,   ADDRESS_SPACE_PROGRAM)
  READWORD64BE(program_read_word_64be,   ADDRESS_SPACE_PROGRAM)
 READDWORD64BE(program_read_dword_64be,  ADDRESS_SPACE_PROGRAM)
   READQWORD64(program_read_qword_64be,  ADDRESS_SPACE_PROGRAM)
  READMASKED64(program_read_masked_64be, ADDRESS_SPACE_PROGRAM)
 WRITEBYTE64BE(program_write_byte_64be,  ADDRESS_SPACE_PROGRAM)
 WRITEWORD64BE(program_write_word_64be,  ADDRESS_SPACE_PROGRAM)
WRITEDWORD64BE(program_write_dword_64be, ADDRESS_SPACE_PROGRAM)
  WRITEQWORD64(program_write_qword_64be, ADDRESS_SPACE_PROGRAM)
 WRITEMASKED64(program_write_masked_64be,ADDRESS_SPACE_PROGRAM)

  READBYTE64LE(program_read_byte_64le,   ADDRESS_SPACE_PROGRAM)
  READWORD64LE(program_read_word_64le,   ADDRESS_SPACE_PROGRAM)
 READDWORD64LE(program_read_dword_64le,  ADDRESS_SPACE_PROGRAM)
   READQWORD64(program_read_qword_64le,  ADDRESS_SPACE_PROGRAM)
  READMASKED64(program_read_masked_64le, ADDRESS_SPACE_PROGRAM)
 WRITEBYTE64LE(program_write_byte_64le,  ADDRESS_SPACE_PROGRAM)
 WRITEWORD64LE(program_write_word_64le,  ADDRESS_SPACE_PROGRAM)
WRITEDWORD64LE(program_write_dword_64le, ADDRESS_SPACE_PROGRAM)
  WRITEQWORD64(program_write_qword_64le, ADDRESS_SPACE_PROGRAM)
 WRITEMASKED64(program_write_masked_64le,ADDRESS_SPACE_PROGRAM)


/*-------------------------------------------------
    Data memory handlers
-------------------------------------------------*/

     READBYTE8(data_read_byte_8,      ADDRESS_SPACE_DATA)
    WRITEBYTE8(data_write_byte_8,     ADDRESS_SPACE_DATA)

  READBYTE16BE(data_read_byte_16be,   ADDRESS_SPACE_DATA)
    READWORD16(data_read_word_16be,   ADDRESS_SPACE_DATA)
 WRITEBYTE16BE(data_write_byte_16be,  ADDRESS_SPACE_DATA)
   WRITEWORD16(data_write_word_16be,  ADDRESS_SPACE_DATA)

  READBYTE16LE(data_read_byte_16le,   ADDRESS_SPACE_DATA)
    READWORD16(data_read_word_16le,   ADDRESS_SPACE_DATA)
 WRITEBYTE16LE(data_write_byte_16le,  ADDRESS_SPACE_DATA)
   WRITEWORD16(data_write_word_16le,  ADDRESS_SPACE_DATA)

  READBYTE32BE(data_read_byte_32be,   ADDRESS_SPACE_DATA)
  READWORD32BE(data_read_word_32be,   ADDRESS_SPACE_DATA)
   READDWORD32(data_read_dword_32be,  ADDRESS_SPACE_DATA)
  READMASKED32(data_read_masked_32be, ADDRESS_SPACE_DATA)
 WRITEBYTE32BE(data_write_byte_32be,  ADDRESS_SPACE_DATA)
 WRITEWORD32BE(data_write_word_32be,  ADDRESS_SPACE_DATA)
  WRITEDWORD32(data_write_dword_32be, ADDRESS_SPACE_DATA)
 WRITEMASKED32(data_write_masked_32be,ADDRESS_SPACE_DATA)

  READBYTE32LE(data_read_byte_32le,   ADDRESS_SPACE_DATA)
  READWORD32LE(data_read_word_32le,   ADDRESS_SPACE_DATA)
   READDWORD32(data_read_dword_32le,  ADDRESS_SPACE_DATA)
  READMASKED32(data_read_masked_32le, ADDRESS_SPACE_DATA)
 WRITEBYTE32LE(data_write_byte_32le,  ADDRESS_SPACE_DATA)
 WRITEWORD32LE(data_write_word_32le,  ADDRESS_SPACE_DATA)
  WRITEDWORD32(data_write_dword_32le, ADDRESS_SPACE_DATA)
 WRITEMASKED32(data_write_masked_32le,ADDRESS_SPACE_DATA)

  READBYTE64BE(data_read_byte_64be,   ADDRESS_SPACE_DATA)
  READWORD64BE(data_read_word_64be,   ADDRESS_SPACE_DATA)
 READDWORD64BE(data_read_dword_64be,  ADDRESS_SPACE_DATA)
   READQWORD64(data_read_qword_64be,  ADDRESS_SPACE_DATA)
  READMASKED64(data_read_masked_64be, ADDRESS_SPACE_DATA)
 WRITEBYTE64BE(data_write_byte_64be,  ADDRESS_SPACE_DATA)
 WRITEWORD64BE(data_write_word_64be,  ADDRESS_SPACE_DATA)
WRITEDWORD64BE(data_write_dword_64be, ADDRESS_SPACE_DATA)
  WRITEQWORD64(data_write_qword_64be, ADDRESS_SPACE_DATA)
 WRITEMASKED64(data_write_masked_64be,ADDRESS_SPACE_DATA)

  READBYTE64LE(data_read_byte_64le,   ADDRESS_SPACE_DATA)
  READWORD64LE(data_read_word_64le,   ADDRESS_SPACE_DATA)
 READDWORD64LE(data_read_dword_64le,  ADDRESS_SPACE_DATA)
   READQWORD64(data_read_qword_64le,  ADDRESS_SPACE_DATA)
  READMASKED64(data_read_masked_64le, ADDRESS_SPACE_DATA)
 WRITEBYTE64LE(data_write_byte_64le,  ADDRESS_SPACE_DATA)
 WRITEWORD64LE(data_write_word_64le,  ADDRESS_SPACE_DATA)
WRITEDWORD64LE(data_write_dword_64le, ADDRESS_SPACE_DATA)
  WRITEQWORD64(data_write_qword_64le, ADDRESS_SPACE_DATA)
 WRITEMASKED64(data_write_masked_64le,ADDRESS_SPACE_DATA)


/*-------------------------------------------------
    I/O memory handlers
-------------------------------------------------*/

     READBYTE8(io_read_byte_8,      ADDRESS_SPACE_IO)
    WRITEBYTE8(io_write_byte_8,     ADDRESS_SPACE_IO)

  READBYTE16BE(io_read_byte_16be,   ADDRESS_SPACE_IO)
    READWORD16(io_read_word_16be,   ADDRESS_SPACE_IO)
 WRITEBYTE16BE(io_write_byte_16be,  ADDRESS_SPACE_IO)
   WRITEWORD16(io_write_word_16be,  ADDRESS_SPACE_IO)

  READBYTE16LE(io_read_byte_16le,   ADDRESS_SPACE_IO)
    READWORD16(io_read_word_16le,   ADDRESS_SPACE_IO)
 WRITEBYTE16LE(io_write_byte_16le,  ADDRESS_SPACE_IO)
   WRITEWORD16(io_write_word_16le,  ADDRESS_SPACE_IO)

  READBYTE32BE(io_read_byte_32be,   ADDRESS_SPACE_IO)
  READWORD32BE(io_read_word_32be,   ADDRESS_SPACE_IO)
   READDWORD32(io_read_dword_32be,  ADDRESS_SPACE_IO)
  READMASKED32(io_read_masked_32be, ADDRESS_SPACE_IO)
 WRITEBYTE32BE(io_write_byte_32be,  ADDRESS_SPACE_IO)
 WRITEWORD32BE(io_write_word_32be,  ADDRESS_SPACE_IO)
  WRITEDWORD32(io_write_dword_32be, ADDRESS_SPACE_IO)
 WRITEMASKED32(io_write_masked_32be,ADDRESS_SPACE_IO)

  READBYTE32LE(io_read_byte_32le,   ADDRESS_SPACE_IO)
  READWORD32LE(io_read_word_32le,   ADDRESS_SPACE_IO)
   READDWORD32(io_read_dword_32le,  ADDRESS_SPACE_IO)
  READMASKED32(io_read_masked_32le, ADDRESS_SPACE_IO)
 WRITEBYTE32LE(io_write_byte_32le,  ADDRESS_SPACE_IO)
 WRITEWORD32LE(io_write_word_32le,  ADDRESS_SPACE_IO)
  WRITEDWORD32(io_write_dword_32le, ADDRESS_SPACE_IO)
 WRITEMASKED32(io_write_masked_32le,ADDRESS_SPACE_IO)

  READBYTE64BE(io_read_byte_64be,   ADDRESS_SPACE_IO)
  READWORD64BE(io_read_word_64be,   ADDRESS_SPACE_IO)
 READDWORD64BE(io_read_dword_64be,  ADDRESS_SPACE_IO)
   READQWORD64(io_read_qword_64be,  ADDRESS_SPACE_IO)
  READMASKED64(io_read_masked_64be, ADDRESS_SPACE_IO)
 WRITEBYTE64BE(io_write_byte_64be,  ADDRESS_SPACE_IO)
 WRITEWORD64BE(io_write_word_64be,  ADDRESS_SPACE_IO)
WRITEDWORD64BE(io_write_dword_64be, ADDRESS_SPACE_IO)
  WRITEQWORD64(io_write_qword_64be, ADDRESS_SPACE_IO)
 WRITEMASKED64(io_write_masked_64be,ADDRESS_SPACE_IO)

  READBYTE64LE(io_read_byte_64le,   ADDRESS_SPACE_IO)
  READWORD64LE(io_read_word_64le,   ADDRESS_SPACE_IO)
 READDWORD64LE(io_read_dword_64le,  ADDRESS_SPACE_IO)
   READQWORD64(io_read_qword_64le,  ADDRESS_SPACE_IO)
  READMASKED64(io_read_masked_64le, ADDRESS_SPACE_IO)
 WRITEBYTE64LE(io_write_byte_64le,  ADDRESS_SPACE_IO)
 WRITEWORD64LE(io_write_word_64le,  ADDRESS_SPACE_IO)
WRITEDWORD64LE(io_write_dword_64le, ADDRESS_SPACE_IO)
  WRITEQWORD64(io_write_qword_64le, ADDRESS_SPACE_IO)
 WRITEMASKED64(io_write_masked_64le,ADDRESS_SPACE_IO)


/*-------------------------------------------------
    safe opcode reading
-------------------------------------------------*/

UINT8 cpu_readop_safe(offs_t offset)
{
	activecpu_set_opbase(offset);
	return cpu_readop_unsafe(offset);
}

UINT16 cpu_readop16_safe(offs_t offset)
{
	activecpu_set_opbase(offset);
	return cpu_readop16_unsafe(offset);
}

UINT32 cpu_readop32_safe(offs_t offset)
{
	activecpu_set_opbase(offset);
	return cpu_readop32_unsafe(offset);
}

UINT64 cpu_readop64_safe(offs_t offset)
{
	activecpu_set_opbase(offset);
	return cpu_readop64_unsafe(offset);
}

UINT8 cpu_readop_arg_safe(offs_t offset)
{
	activecpu_set_opbase(offset);
	return cpu_readop_arg_unsafe(offset);
}

UINT16 cpu_readop_arg16_safe(offs_t offset)
{
	activecpu_set_opbase(offset);
	return cpu_readop_arg16_unsafe(offset);
}

UINT32 cpu_readop_arg32_safe(offs_t offset)
{
	activecpu_set_opbase(offset);
	return cpu_readop_arg32_unsafe(offset);
}

UINT64 cpu_readop_arg64_safe(offs_t offset)
{
	activecpu_set_opbase(offset);
	return cpu_readop_arg64_unsafe(offset);
}


/*-------------------------------------------------
    unmapped memory handlers
-------------------------------------------------*/

static READ8_HANDLER( mrh8_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory byte read from %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap;
}
static READ16_HANDLER( mrh16_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory word read from %08X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*2), mem_mask ^ 0xffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap;
}
static READ32_HANDLER( mrh32_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory dword read from %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*4), mem_mask ^ 0xffffffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap;
}
static READ64_HANDLER( mrh64_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory qword read from %08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*8), (int)(mem_mask >> 32) ^ 0xffffffff, (int)(mem_mask & 0xffffffff) ^ 0xffffffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap;
}

static WRITE8_HANDLER( mwh8_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory byte write to %08X = %02X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset), data);
}
static WRITE16_HANDLER( mwh16_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory word write to %08X = %04X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*2), data, mem_mask ^ 0xffff);
}
static WRITE32_HANDLER( mwh32_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory dword write to %08X = %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*4), data, mem_mask ^ 0xffffffff);
}
static WRITE64_HANDLER( mwh64_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory qword write to %08X = %08X%08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*8), (int)(data >> 32), (int)(data & 0xffffffff), (int)(mem_mask >> 32) ^ 0xffffffff, (int)(mem_mask & 0xffffffff) ^ 0xffffffff);
}

static READ8_HANDLER( mrh8_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory byte read from %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap;
}
static READ16_HANDLER( mrh16_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory word read from %08X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*2), mem_mask ^ 0xffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap;
}
static READ32_HANDLER( mrh32_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory dword read from %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*4), mem_mask ^ 0xffffffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap;
}
static READ64_HANDLER( mrh64_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory qword read from %08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*8), (int)(mem_mask >> 32) ^ 0xffffffff, (int)(mem_mask & 0xffffffff) ^ 0xffffffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap;
}

static WRITE8_HANDLER( mwh8_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory byte write to %08X = %02X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset), data);
}
static WRITE16_HANDLER( mwh16_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory word write to %08X = %04X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*2), data, mem_mask ^ 0xffff);
}
static WRITE32_HANDLER( mwh32_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory dword write to %08X = %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*4), data, mem_mask ^ 0xffffffff);
}
static WRITE64_HANDLER( mwh64_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory qword write to %08X = %08X%08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*8), (int)(data >> 32), (int)(data & 0xffffffff), (int)(mem_mask >> 32) ^ 0xffffffff, (int)(mem_mask & 0xffffffff) ^ 0xffffffff);
}

static READ8_HANDLER( mrh8_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O byte read from %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap;
}
static READ16_HANDLER( mrh16_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O word read from %08X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*2), mem_mask ^ 0xffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap;
}
static READ32_HANDLER( mrh32_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O dword read from %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*4), mem_mask ^ 0xffffffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap;
}
static READ64_HANDLER( mrh64_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O qword read from %08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*8), (int)(mem_mask >> 32) ^ 0xffffffff, (int)(mem_mask & 0xffffffff) ^ 0xffffffff);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap;
}

static WRITE8_HANDLER( mwh8_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O byte write to %08X = %02X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset), data);
}
static WRITE16_HANDLER( mwh16_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O word write to %08X = %04X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*2), data, mem_mask ^ 0xffff);
}
static WRITE32_HANDLER( mwh32_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O dword write to %08X = %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*4), data, mem_mask ^ 0xffffffff);
}
static WRITE64_HANDLER( mwh64_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O qword write to %08X = %08X%08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), INV_SPACE_SHIFT(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*8), (int)(data >> 32), (int)(data & 0xffffffff), (int)(mem_mask >> 32) ^ 0xffffffff, (int)(mem_mask & 0xffffffff) ^ 0xffffffff);
}


/*-------------------------------------------------
    no-op memory handlers
-------------------------------------------------*/

static READ8_HANDLER( mrh8_nop_program )   { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap; }
static READ16_HANDLER( mrh16_nop_program ) { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap; }
static READ32_HANDLER( mrh32_nop_program ) { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap; }
static READ64_HANDLER( mrh64_nop_program ) { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap; }

static READ8_HANDLER( mrh8_nop_data )      { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap; }
static READ16_HANDLER( mrh16_nop_data )    { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap; }
static READ32_HANDLER( mrh32_nop_data )    { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap; }
static READ64_HANDLER( mrh64_nop_data )    { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap; }

static READ8_HANDLER( mrh8_nop_io )        { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap; }
static READ16_HANDLER( mrh16_nop_io )      { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap; }
static READ32_HANDLER( mrh32_nop_io )      { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap; }
static READ64_HANDLER( mrh64_nop_io )      { return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap; }

static WRITE8_HANDLER( mwh8_nop )          {  }
static WRITE16_HANDLER( mwh16_nop )        {  }
static WRITE32_HANDLER( mwh32_nop )        {  }
static WRITE64_HANDLER( mwh64_nop )        {  }


/*-------------------------------------------------
    get_static_handler - returns points to static
    memory handlers
-------------------------------------------------*/

static genf *get_static_handler(int databits, int readorwrite, int spacenum, int which)
{
	static const struct
	{
		UINT8		databits;
		UINT8		handlernum;
		UINT8		spacenum;
		genf *		read;
		genf *		write;
	} static_handler_list[] =
	{
		{  8, STATIC_UNMAP,  ADDRESS_SPACE_PROGRAM, (genf *)mrh8_unmap_program, (genf *)mwh8_unmap_program },
		{  8, STATIC_UNMAP,  ADDRESS_SPACE_DATA,    (genf *)mrh8_unmap_data,    (genf *)mwh8_unmap_data },
		{  8, STATIC_UNMAP,  ADDRESS_SPACE_IO,      (genf *)mrh8_unmap_io,      (genf *)mwh8_unmap_io },
		{  8, STATIC_NOP,    ADDRESS_SPACE_PROGRAM, (genf *)mrh8_nop_program,   (genf *)mwh8_nop },
		{  8, STATIC_NOP,    ADDRESS_SPACE_DATA,    (genf *)mrh8_nop_data,      (genf *)mwh8_nop },
		{  8, STATIC_NOP,    ADDRESS_SPACE_IO,      (genf *)mrh8_nop_io,        (genf *)mwh8_nop },

		{ 16, STATIC_UNMAP,  ADDRESS_SPACE_PROGRAM, (genf *)mrh16_unmap_program,(genf *)mwh16_unmap_program },
		{ 16, STATIC_UNMAP,  ADDRESS_SPACE_DATA,    (genf *)mrh16_unmap_data,   (genf *)mwh16_unmap_data },
		{ 16, STATIC_UNMAP,  ADDRESS_SPACE_IO,      (genf *)mrh16_unmap_io,     (genf *)mwh16_unmap_io },
		{ 16, STATIC_NOP,    ADDRESS_SPACE_PROGRAM, (genf *)mrh16_nop_program,  (genf *)mwh16_nop },
		{ 16, STATIC_NOP,    ADDRESS_SPACE_DATA,    (genf *)mrh16_nop_data,     (genf *)mwh16_nop },
		{ 16, STATIC_NOP,    ADDRESS_SPACE_IO,      (genf *)mrh16_nop_io,       (genf *)mwh16_nop },

		{ 32, STATIC_UNMAP,  ADDRESS_SPACE_PROGRAM, (genf *)mrh32_unmap_program,(genf *)mwh32_unmap_program },
		{ 32, STATIC_UNMAP,  ADDRESS_SPACE_DATA,    (genf *)mrh32_unmap_data,   (genf *)mwh32_unmap_data },
		{ 32, STATIC_UNMAP,  ADDRESS_SPACE_IO,      (genf *)mrh32_unmap_io,     (genf *)mwh32_unmap_io },
		{ 32, STATIC_NOP,    ADDRESS_SPACE_PROGRAM, (genf *)mrh32_nop_program,  (genf *)mwh32_nop },
		{ 32, STATIC_NOP,    ADDRESS_SPACE_DATA,    (genf *)mrh32_nop_data,     (genf *)mwh32_nop },
		{ 32, STATIC_NOP,    ADDRESS_SPACE_IO,      (genf *)mrh32_nop_io,       (genf *)mwh32_nop },

		{ 64, STATIC_UNMAP,  ADDRESS_SPACE_PROGRAM, (genf *)mrh64_unmap_program,(genf *)mwh64_unmap_program },
		{ 64, STATIC_UNMAP,  ADDRESS_SPACE_DATA,    (genf *)mrh64_unmap_data,   (genf *)mwh64_unmap_data },
		{ 64, STATIC_UNMAP,  ADDRESS_SPACE_IO,      (genf *)mrh64_unmap_io,     (genf *)mwh64_unmap_io },
		{ 64, STATIC_NOP,    ADDRESS_SPACE_PROGRAM, (genf *)mrh64_nop_program,  (genf *)mwh64_nop },
		{ 64, STATIC_NOP,    ADDRESS_SPACE_DATA,    (genf *)mrh64_nop_data,     (genf *)mwh64_nop },
		{ 64, STATIC_NOP,    ADDRESS_SPACE_IO,      (genf *)mrh64_nop_io,       (genf *)mwh64_nop },
	};
	int tablenum;

	for (tablenum = 0; tablenum < sizeof(static_handler_list) / sizeof(static_handler_list[0]); tablenum++)
		if (static_handler_list[tablenum].databits == databits && static_handler_list[tablenum].handlernum == which)
			if (static_handler_list[tablenum].spacenum == 0xff || static_handler_list[tablenum].spacenum == spacenum)
				return readorwrite ? static_handler_list[tablenum].write : static_handler_list[tablenum].read;

	return NULL;
}


/*-------------------------------------------------
    debugging
-------------------------------------------------*/

static const char *handler_to_string(const table_data *table, UINT8 entry)
{
	static const char *const strings[] =
	{
		"invalid",		"bank 1",		"bank 2",		"bank 3",
		"bank 4",		"bank 5",		"bank 6",		"bank 7",
		"bank 8",		"bank 9",		"bank 10",		"bank 11",
		"bank 12",		"bank 13",		"bank 14",		"bank 15",
		"bank 16",		"bank 17",		"bank 18",		"bank 19",
		"bank 20",		"bank 21",		"bank 22",		"bank 23",
		"bank 24",		"bank 25",		"bank 26",		"bank 27",
		"bank 28",		"bank 29",		"bank 30",		"bank 31",
		"bank 32",		"ram[33]",		"ram[34]",		"ram[35]",
		"ram[36]",		"ram[37]",		"ram[38]",		"ram[39]",
		"ram[40]",		"ram[41]",		"ram[42]",		"ram[43]",
		"ram[44]",		"ram[45]",		"ram[46]",		"ram[47]",
		"ram[48]",		"ram[49]",		"ram[50]",		"ram[51]",
		"ram[52]",		"ram[53]",		"ram[54]",		"ram[55]",
		"ram[56]",		"ram[57]",		"ram[58]",		"ram[59]",
		"ram[60]",		"ram[61]",		"ram[62]",		"ram[63]",
		"ram[64]",		"ram[65]",		"ram[66]",		"ram[67]",
		"ram[68]",		"rom",			"nop",			"unmapped"
	};

	/* constant strings for lower entries */
	if (entry < STATIC_COUNT)
		return strings[entry];
	else
		return table->handlers[entry].name ? table->handlers[entry].name : "???";
}

static void dump_map(FILE *file, const addrspace_data *space, const table_data *table)
{
	int l1count = 1 << LEVEL1_BITS;
	int l2count = 1 << LEVEL2_BITS;
	UINT8 lastentry = STATIC_UNMAP;
	int entrymatches = 0;
	int i, j;

	/* dump generic information */
	fprintf(file, "  Address bits = %d\n", space->abits);
	fprintf(file, "     Data bits = %d\n", space->dbits);
	fprintf(file, "       L1 bits = %d\n", LEVEL1_BITS);
	fprintf(file, "       L2 bits = %d\n", LEVEL2_BITS);
	fprintf(file, "  Address mask = %X\n", space->mask);
	fprintf(file, "\n");

	/* loop over level 1 entries */
	for (i = 0; i < l1count; i++)
	{
		UINT8 entry = table->table[i];

		/* if this entry matches the previous one, just count it */
		if (entry < SUBTABLE_BASE && entry == lastentry)
		{
			entrymatches++;
			continue;
		}

		/* otherwise, print accumulated info */
		if (lastentry < SUBTABLE_BASE && lastentry != STATIC_UNMAP)
			fprintf(file, "%08X-%08X    = %02X: %s [offset=%08X]\n",
							(i - entrymatches) << LEVEL2_BITS,
							(i << LEVEL2_BITS) - 1,
							lastentry,
							handler_to_string(table, lastentry),
							table->handlers[lastentry].offset);

		/* start counting with this entry */
		lastentry = entry;
		entrymatches = 1;

		/* if we're a subtable, we need to drill down */
		if (entry >= SUBTABLE_BASE)
		{
			UINT8 lastentry2 = STATIC_UNMAP;
			int entry2matches = 0;

			/* loop over level 2 entries */
			entry -= SUBTABLE_BASE;
			for (j = 0; j < l2count; j++)
			{
				UINT8 entry2 = table->table[(1 << LEVEL1_BITS) + (entry << LEVEL2_BITS) + j];

				/* if this entry matches the previous one, just count it */
				if (entry2 < SUBTABLE_BASE && entry2 == lastentry2)
				{
					entry2matches++;
					continue;
				}

				/* otherwise, print accumulated info */
				if (lastentry2 < SUBTABLE_BASE && lastentry2 != STATIC_UNMAP)
					fprintf(file, "%08X-%08X    = %02X: %s [offset=%08X]\n",
									((i << LEVEL2_BITS) | (j - entry2matches)),
									((i << LEVEL2_BITS) | (j - 1)),
									lastentry2,
									handler_to_string(table, lastentry2),
									table->handlers[lastentry2].offset);

				/* start counting with this entry */
				lastentry2 = entry2;
				entry2matches = 1;
			}

			/* flush the last entry */
			if (lastentry2 < SUBTABLE_BASE && lastentry2 != STATIC_UNMAP)
				fprintf(file, "%08X-%08X    = %02X: %s [offset=%08X]\n",
								((i << LEVEL2_BITS) | (j - entry2matches)),
								((i << LEVEL2_BITS) | (j - 1)),
								lastentry2,
								handler_to_string(table, lastentry2),
								table->handlers[lastentry2].offset);
		}
	}

	/* flush the last entry */
	if (lastentry < SUBTABLE_BASE && lastentry != STATIC_UNMAP)
		fprintf(file, "%08X-%08X    = %02X: %s [offset=%08X]\n",
						(i - entrymatches) << LEVEL2_BITS,
						(i << LEVEL2_BITS) - 1,
						lastentry,
						handler_to_string(table, lastentry),
						table->handlers[lastentry].offset);
}

void memory_dump(FILE *file)
{
	int cpunum, spacenum;

	/* skip if we can't open the file */
	if (!file)
		return;

	/* loop over CPUs */
	for (cpunum = 0; cpunum < MAX_CPU && Machine->drv->cpu[cpunum].type != CPU_DUMMY; cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].space[spacenum].abits)
			{
				fprintf(file, "\n\n"
				              "=========================================\n"
				              "CPU %d address space %d read handler dump\n"
				              "=========================================\n", cpunum, spacenum);
				dump_map(file, &cpudata[cpunum].space[spacenum], &cpudata[cpunum].space[spacenum].read);

				fprintf(file, "\n\n"
				              "==========================================\n"
				              "CPU %d address space %d write handler dump\n"
				              "==========================================\n", cpunum, spacenum);
				dump_map(file, &cpudata[cpunum].space[spacenum], &cpudata[cpunum].space[spacenum].write);
			}
}


/*-------------------------------------------------
    memory_get_handler_string - return a string
    describing the handler at a particular offset
-------------------------------------------------*/

const char *memory_get_handler_string(int read0_or_write1, int cpunum, int spacenum, offs_t offset)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	const table_data *table = read0_or_write1 ? &space->write : &space->read;
	UINT8 entry;

	/* perform the lookup */
	offset &= space->mask;
	entry = table->table[LEVEL1_INDEX(offset)];
	if (entry >= SUBTABLE_BASE)
		entry = table->table[LEVEL2_INDEX(entry, offset)];

	/* 8-bit case: RAM/ROM */
	return handler_to_string(table, entry);
}
