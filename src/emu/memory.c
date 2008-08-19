/***************************************************************************

    memory.c

    Functions which handle the CPU memory access.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

****************************************************************************

    Basic theory of memory handling:

    An address with up to 32 bits is passed to a memory handler. First,
    an address mask is applied to the address, removing unused bits.

    Next, the address is broken into two halves, an upper half and a
    lower half. The number of bits in each half can be controlled via
    the macros in LEVEL1_BITS and LEVEL2_BITS, but they default to the
    upper 18 bits and the lower 14 bits.

    The upper half is then used as an index into a lookup table of bytes.
    If the value pulled from the table is between SUBTABLE_BASE and 255,
    then the lower half of the address is needed to resolve the final
    handler. In this case, the value from the table is combined with the
    lower address bits to form an index into a subtable.

    The final result of the lookup is a value from 0 to SUBTABLE_BASE - 1.
    These values correspond to memory handlers. The lower numbered
    handlers (from 0 through STATIC_COUNT - 1) are fixed handlers and refer
    to either memory banks or other special cases. The remaining handlers
    (from STATIC_COUNT through SUBTABLE_BASE - 1) are dynamically
    allocated to driver-specified handlers.

    Thus, table entries fall into these categories:

        0 .. STATIC_COUNT - 1 = fixed handlers
        STATIC_COUNT .. SUBTABLE_BASE - 1 = driver-specific handlers
        SUBTABLE_BASE .. 255 = need to look up lower bits in subtable

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

    AM_MASK(mask)
        Specifies a mask for the addresses in the current bucket. This mask
        is applied after a positive hit in the bucket specified by AM_RANGE
        or AM_SPACE, and is computed before accessing the RAM or calling
        through to the read/write handler. If you use AM_MIRROR, below, the
        mask is ANDed implicitly with the logical NOT of the mirror. The
        mask specified by this macro is ANDed against any implicit masks.

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

    AM_REGION(class, tag, offs)
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
#include "debug/debugcpu.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define MEM_DUMP		(0)
#define VERBOSE			(0)
#define ALLOW_ONLY_AUTO_MALLOC_BANKS	0

#define VPRINTF(x)	do { if (VERBOSE) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* banking constants */
#define MAX_BANKS				(STATIC_BANKMAX - STATIC_BANK1) /* maximum number of banks */
#define MAX_BANK_ENTRIES		4096					/* maximum number of possible bank values */
#define MAX_EXPLICIT_BANKS		32						/* maximum number of explicitly-defined banks */

/* address map lookup table definitions */
#define LEVEL1_BITS				18						/* number of address bits in the level 1 table */
#define LEVEL2_BITS				(32 - LEVEL1_BITS)		/* number of address bits in the level 2 table */
#define SUBTABLE_COUNT			64						/* number of slots reserved for subtables */
#define SUBTABLE_BASE			(256 - SUBTABLE_COUNT)	/* first index of a subtable */
#define ENTRY_COUNT				(SUBTABLE_BASE)			/* number of legitimate (non-subtable) entries */
#define SUBTABLE_ALLOC			8						/* number of subtables to allocate at a time */

/* other address map constants */
#define MAX_SHARED_POINTERS		256						/* maximum number of shared pointers in memory maps */
#define MEMORY_BLOCK_CHUNK		65536					/* minimum chunk size of allocated memory blocks */

/* read or write constants */
enum _read_or_write
{
	ROW_READ,
	ROW_WRITE
};
typedef enum _read_or_write read_or_write;


/***************************************************************************
    MACROS
***************************************************************************/

/* table lookup helpers */
#define LEVEL1_INDEX(a)			((a) >> LEVEL2_BITS)
#define LEVEL2_INDEX(e,a)		((1 << LEVEL1_BITS) + (((e) - SUBTABLE_BASE) << LEVEL2_BITS) + ((a) & ((1 << LEVEL2_BITS) - 1)))

/* helper macros */
#define HANDLER_IS_RAM(h)		((FPTR)(h) == STATIC_RAM)
#define HANDLER_IS_ROM(h)		((FPTR)(h) == STATIC_ROM)
#define HANDLER_IS_NOP(h)		((FPTR)(h) == STATIC_NOP)
#define HANDLER_IS_BANK(h)		((FPTR)(h) >= STATIC_BANK1 && (FPTR)(h) <= STATIC_BANKMAX)
#define HANDLER_IS_STATIC(h)	((FPTR)(h) < STATIC_COUNT)

#define HANDLER_TO_BANK(h)		((UINT32)(FPTR)(h))
#define BANK_TO_HANDLER(b)		((genf *)(FPTR)(b))

#undef ADDR2BYTE
#undef BYTE2ADDR
#define ADDR2BYTE(s,a)			(((s)->ashift < 0) ? ((a) << -(s)->ashift) : ((a) >> (s)->ashift))
#define ADDR2BYTE_END(s,a)		(((s)->ashift < 0) ? (((a) << -(s)->ashift) | ((1 << -(s)->ashift) - 1)) : ((a) >> (s)->ashift))
#define BYTE2ADDR(s,a)			(((s)->ashift < 0) ? ((a) >> -(s)->ashift) : ((a) << (s)->ashift))

#define SUBTABLE_PTR(tabledata, entry) (&(tabledata)->table[(1 << LEVEL1_BITS) + (((entry) - SUBTABLE_BASE) << LEVEL2_BITS)])



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* a memory block is a chunk of RAM associated with a range of memory in a CPU's address space */
typedef struct _memory_block memory_block;
struct _memory_block
{
	memory_block *			next;					/* next memory block in the list */
	UINT8					cpunum;					/* which CPU are we associated with? */
	UINT8					spacenum;				/* which address space are we associated with? */
	UINT8					isallocated;			/* did we allocate this ourselves? */
	offs_t 					bytestart, byteend;		/* byte-normalized start/end for verifying a match */
	UINT8 *					data;					/* pointer to the data for this block */
};

typedef struct _bank_data bank_info;
struct _bank_data
{
	UINT8 					used;					/* is this bank used? */
	UINT8 					dynamic;				/* is this bank allocated dynamically? */
	UINT8 					cpunum;					/* the CPU it is used for */
	UINT8 					spacenum;				/* the address space it is used for */
	UINT8 					read;					/* is this bank used for reads? */
	UINT8 					write;					/* is this bank used for writes? */
	offs_t 					bytestart;				/* byte-adjusted start offset */
	offs_t 					byteend;				/* byte-adjusted end offset */
	UINT16					curentry;				/* current entry */
	void *					entry[MAX_BANK_ENTRIES];/* array of entries for this bank */
	void *					entryd[MAX_BANK_ENTRIES];/* array of decrypted entries for this bank */
};

/* In memory.h: typedef struct _handler_data handler_data */
struct _handler_data
{
	memory_handler			handler;				/* function pointer for handler */
	void *					object;					/* object associated with the handler */
	const char *			name;					/* name of the handler */
	memory_handler			subhandler;				/* function pointer for subhandler */
	void *					subobject;				/* object associated with the subhandler */
	UINT8					subunits;				/* number of subunits to access */
	UINT8					subshift[8];			/* shift amounts for up to 8 subunits */
	offs_t					bytestart;				/* byte-adjusted start address for handler */
	offs_t					byteend;				/* byte-adjusted end address for handler */
	offs_t					bytemask;				/* byte-adjusted mask against the final address */
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
	UINT8					endianness;				/* endianness of this space */
	INT8					ashift;					/* address shift */
	UINT8					abits;					/* address bits */
	UINT8 					dbits;					/* data bits */
	offs_t					addrmask;				/* global address mask */
	offs_t					bytemask;				/* byte-converted global address mask */
	UINT64					unmap;					/* unmapped value */
	table_data				read;					/* memory read lookup table */
	table_data				write;					/* memory write lookup table */
	const data_accessors *	accessors;				/* pointer to the memory accessors */
	address_map *			map;					/* original memory map */
};

typedef struct _cpu_data cpu_data;
struct _cpu_data
{
	const char *			tag;					/* CPU's tag */
	UINT8 *					region;					/* pointer to memory region */
	size_t					regionsize;				/* size of region, in bytes */

	opbase_handler_func 	opbase_handler;			/* opcode base handler */
	opbase_data				opbase;					/* dynamic opcode data */

	UINT8					spacemask;				/* mask of which address spaces are used */
	addrspace_data		 	space[ADDRESS_SPACES];	/* info about each address space */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

opbase_data					opbase;							/* opcode data */

address_space				active_address_space[ADDRESS_SPACES];/* address space data */

static UINT8 *				bank_ptr[STATIC_COUNT];			/* array of bank pointers */
static UINT8 *				bankd_ptr[STATIC_COUNT];		/* array of decrypted bank pointers */
static void *				shared_ptr[MAX_SHARED_POINTERS];/* array of shared pointers */

static memory_block *		memory_block_list;				/* head of the list of memory blocks */

static int					cur_context;					/* current CPU context */

static opbase_handler_func	opbase_handler;					/* opcode base override */

static UINT8				debugger_access;				/* treat accesses as coming from the debugger */
static UINT8				log_unmap[ADDRESS_SPACES];		/* log unmapped memory accesses */

static cpu_data				cpudata[MAX_CPU];				/* data gathered for each CPU */
static bank_info 			bankdata[STATIC_COUNT];			/* data gathered for each bank */

static UINT8 *				wptable;						/* watchpoint-fill table */

#define ACCESSOR_GROUP(type, width) \
{ \
	memory_set_opbase, \
	type##_read_byte_##width, \
	type##_read_word_##width, \
	type##_read_word_masked_##width, \
	type##_read_dword_##width, \
	type##_read_dword_masked_##width, \
	type##_read_qword_##width, \
	type##_read_qword_masked_##width, \
	type##_write_byte_##width, \
	type##_write_word_##width, \
	type##_write_word_masked_##width, \
	type##_write_dword_##width, \
	type##_write_dword_masked_##width, \
	type##_write_qword_##width, \
	type##_write_qword_masked_##width \
}

static const data_accessors memory_accessors[ADDRESS_SPACES][4][2] =
{
	/* program accessors */
	{
		{ ACCESSOR_GROUP(program, 8le),  ACCESSOR_GROUP(program, 8be)  },
		{ ACCESSOR_GROUP(program, 16le), ACCESSOR_GROUP(program, 16be) },
		{ ACCESSOR_GROUP(program, 32le), ACCESSOR_GROUP(program, 32be) },
		{ ACCESSOR_GROUP(program, 64le), ACCESSOR_GROUP(program, 64be) }
	},

	/* data accessors */
	{
		{ ACCESSOR_GROUP(data, 8le),  ACCESSOR_GROUP(data, 8be)  },
		{ ACCESSOR_GROUP(data, 16le), ACCESSOR_GROUP(data, 16be) },
		{ ACCESSOR_GROUP(data, 32le), ACCESSOR_GROUP(data, 32be) },
		{ ACCESSOR_GROUP(data, 64le), ACCESSOR_GROUP(data, 64be) }
	},

	/* I/O accessors */
	{
		{ ACCESSOR_GROUP(io, 8le),  ACCESSOR_GROUP(io, 8be)  },
		{ ACCESSOR_GROUP(io, 16le), ACCESSOR_GROUP(io, 16be) },
		{ ACCESSOR_GROUP(io, 32le), ACCESSOR_GROUP(io, 32be) },
		{ ACCESSOR_GROUP(io, 64le), ACCESSOR_GROUP(io, 64be) }
	},
};

const char *const address_space_names[ADDRESS_SPACES] = { "program", "data", "I/O" };



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void address_map_detokenize(address_map *map, const game_driver *driver, const addrmap_token *tokens);

static void memory_init_cpudata(running_machine *machine);
static void memory_init_preflight(running_machine *machine);
static void memory_init_populate(running_machine *machine);
static void space_map_range_private(addrspace_data *space, read_or_write readorwrite, int handlerbits, int handlerunitmask, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, genf *handler, void *object, const char *handler_name);
static void space_map_range(addrspace_data *space, read_or_write readorwrite, int handlerbits, int handlerunitmask, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, genf *handler, void *object, const char *handler_name);
static void bank_assign_static(int banknum, int cpunum, int spacenum, read_or_write readorwrite, offs_t bytestart, offs_t byteend);
static genf *bank_assign_dynamic(int cpunum, int spacenum, read_or_write readorwrite, offs_t bytestart, offs_t byteend);
static UINT8 table_assign_handler(handler_data *table, void *object, genf *handler, const char *handler_name, offs_t bytestart, offs_t byteend, offs_t bytemask);
static void table_compute_subhandler(handler_data *table, UINT8 entry, read_or_write readorwrite, int spacebits, int spaceendian, int handlerbits, int handlerunitmask);
static void table_populate_range(table_data *tabledata, offs_t bytestart, offs_t byteend, UINT8 handler);
static void table_populate_range_mirrored(table_data *tabledata, offs_t bytestart, offs_t byteend, offs_t bytemirror, UINT8 handler);
static UINT8 subtable_alloc(table_data *tabledata);
static void subtable_realloc(table_data *tabledata, UINT8 subentry);
static int subtable_merge(table_data *tabledata);
static void subtable_release(table_data *tabledata, UINT8 subentry);
static UINT8 *subtable_open(table_data *tabledata, offs_t l1index);
static void subtable_close(table_data *tabledata, offs_t l1index);
static void memory_init_allocate(running_machine *machine);
static void *allocate_memory_block(running_machine *machine, int cpunum, int spacenum, offs_t bytestart, offs_t byteend, void *memory);
static void register_for_save(int cpunum, int spacenum, offs_t bytestart, void *base, size_t numbytes);
static address_map_entry *assign_intersecting_blocks(addrspace_data *space, offs_t bytestart, offs_t byteend, UINT8 *base);
static void memory_init_locate(running_machine *machine);
static void *memory_find_base(int cpunum, int spacenum, offs_t byteaddress);
static memory_handler get_stub_handler(read_or_write readorwrite, int spacedbits, int handlerdbits);
static genf *get_static_handler(int handlerbits, int readorwrite, int spacenum, int which);
static void memory_exit(running_machine *machine);
static void mem_dump(void);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    force_opbase_update - ensure that we update
    the opcode base
-------------------------------------------------*/

INLINE void force_opbase_update(void)
{
	opbase.entry = 0xff;
	memory_set_opbase(activecpu_get_physical_pc_byte());
}


/*-------------------------------------------------
    adjust_addresses - adjust addresses for a
    given address space in a standard fashion
-------------------------------------------------*/

INLINE void adjust_addresses(addrspace_data *space, offs_t *start, offs_t *end, offs_t *mask, offs_t *mirror)
{
	/* adjust start/end/mask values */
	if (*mask == 0)
		*mask = space->addrmask & ~*mirror;
	else
		*mask &= space->addrmask;
	*start &= ~*mirror & space->addrmask;
	*end &= ~*mirror & space->addrmask;

	/* adjust to byte values */
	*start = ADDR2BYTE(space, *start);
	*end = ADDR2BYTE_END(space, *end);
	*mask = ADDR2BYTE(space, *mask);
	*mirror = ADDR2BYTE(space, *mirror);
}


/*-------------------------------------------------
    read_byte_generic - read a byte from an
    arbitrary address space
-------------------------------------------------*/

INLINE UINT8 read_byte_generic(UINT8 spacenum, offs_t address)
{
	const address_space *space = &active_address_space[spacenum];
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;
	UINT8 result;

	profiler_mark(PROFILER_MEMREAD);

	address &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(address)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry, address)];
	handler = &space->readhandlers[entry];

	offset = (address - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		result = bank_ptr[entry][offset];
	else
		result = (*handler->handler.read.mhandler8)(handler->object, offset);

	profiler_mark(PROFILER_END);
	return result;
}


/*-------------------------------------------------
    write_byte_generic - write a byte to an
    arbitrary address space
-------------------------------------------------*/

INLINE void write_byte_generic(UINT8 spacenum, offs_t address, UINT8 data)
{
	const address_space *space = &active_address_space[spacenum];
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;

	profiler_mark(PROFILER_MEMWRITE);

	address &= space->bytemask;
	entry = space->writelookup[LEVEL1_INDEX(address)];
	if (entry >= SUBTABLE_BASE)
		entry = space->writelookup[LEVEL2_INDEX(entry, address)];
	handler = &space->writehandlers[entry];

	offset = (address - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		bank_ptr[entry][offset] = data;
	else
		(*handler->handler.write.mhandler8)(handler->object, offset, data);

	profiler_mark(PROFILER_END);
}


/*-------------------------------------------------
    read_word_generic - read a word from an
    arbitrary address space
-------------------------------------------------*/

INLINE UINT16 read_word_generic(UINT8 spacenum, offs_t address, UINT16 mem_mask)
{
	const address_space *space = &active_address_space[spacenum];
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;
	UINT16 result;

	profiler_mark(PROFILER_MEMREAD);

	address &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(address)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry, address)];
	handler = &space->readhandlers[entry];

	offset = (address - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		result = *(UINT16 *)&bank_ptr[entry][offset & ~1];
	else
		result = (*handler->handler.read.mhandler16)(handler->object, offset >> 1, mem_mask);

	profiler_mark(PROFILER_END);
	return result;
}


/*-------------------------------------------------
    write_word_generic - write a word to an
    arbitrary address space
-------------------------------------------------*/

INLINE void write_word_generic(UINT8 spacenum, offs_t address, UINT16 data, UINT16 mem_mask)
{
	const address_space *space = &active_address_space[spacenum];
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;

	profiler_mark(PROFILER_MEMWRITE);

	address &= space->bytemask;
	entry = space->writelookup[LEVEL1_INDEX(address)];
	if (entry >= SUBTABLE_BASE)
		entry = space->writelookup[LEVEL2_INDEX(entry, address)];
	handler = &space->writehandlers[entry];

	offset = (address - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
	{
		UINT16 *dest = (UINT16 *)&bank_ptr[entry][offset & ~1];
		*dest = (*dest & ~mem_mask) | (data & mem_mask);
	}
	else
		(*handler->handler.write.mhandler16)(handler->object, offset >> 1, data, mem_mask);

	profiler_mark(PROFILER_END);
}


/*-------------------------------------------------
    read_dword_generic - read a dword from an
    arbitrary address space
-------------------------------------------------*/

INLINE UINT32 read_dword_generic(UINT8 spacenum, offs_t address, UINT32 mem_mask)
{
	const address_space *space = &active_address_space[spacenum];
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;
	UINT32 result;

	profiler_mark(PROFILER_MEMREAD);

	address &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(address)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry, address)];
	handler = &space->readhandlers[entry];

	offset = (address - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		result = *(UINT32 *)&bank_ptr[entry][offset & ~3];
	else
		result = (*handler->handler.read.mhandler32)(handler->object, offset >> 2, mem_mask);

	profiler_mark(PROFILER_END);
	return result;
}


/*-------------------------------------------------
    write_dword_generic - write a dword to an
    arbitrary address space
-------------------------------------------------*/

INLINE void write_dword_generic(UINT8 spacenum, offs_t address, UINT32 data, UINT32 mem_mask)
{
	const address_space *space = &active_address_space[spacenum];
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;

	profiler_mark(PROFILER_MEMWRITE);

	address &= space->bytemask;
	entry = space->writelookup[LEVEL1_INDEX(address)];
	if (entry >= SUBTABLE_BASE)
		entry = space->writelookup[LEVEL2_INDEX(entry, address)];
	handler = &space->writehandlers[entry];

	offset = (address - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
	{
		UINT32 *dest = (UINT32 *)&bank_ptr[entry][offset & ~3];
		*dest = (*dest & ~mem_mask) | (data & mem_mask);
	}
	else
		(*handler->handler.write.mhandler32)(handler->object, offset >> 2, data, mem_mask);

	profiler_mark(PROFILER_END);
}


/*-------------------------------------------------
    read_qword_generic - read a qword from an
    arbitrary address space
-------------------------------------------------*/

INLINE UINT64 read_qword_generic(UINT8 spacenum, offs_t address, UINT64 mem_mask)
{
	const address_space *space = &active_address_space[spacenum];
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;
	UINT64 result;

	profiler_mark(PROFILER_MEMREAD);

	address &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(address)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry, address)];
	handler = &space->readhandlers[entry];

	offset = (address - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		result = *(UINT64 *)&bank_ptr[entry][offset & ~7];
	else
		result = (*handler->handler.read.mhandler64)(handler->object, offset >> 3, mem_mask);

	profiler_mark(PROFILER_END);
	return result;
}


/*-------------------------------------------------
    write_qword_generic - write a qword to an
    arbitrary address space
-------------------------------------------------*/

INLINE void write_qword_generic(UINT8 spacenum, offs_t address, UINT64 data, UINT64 mem_mask)
{
	const address_space *space = &active_address_space[spacenum];
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;

	profiler_mark(PROFILER_MEMWRITE);

	address &= space->bytemask;
	entry = space->writelookup[LEVEL1_INDEX(address)];
	if (entry >= SUBTABLE_BASE)
		entry = space->writelookup[LEVEL2_INDEX(entry, address)];
	handler = &space->writehandlers[entry];

	offset = (address - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
	{
		UINT64 *dest = (UINT64 *)&bank_ptr[entry][offset & ~7];
		*dest = (*dest & ~mem_mask) | (data & mem_mask);
	}
	else
		(*handler->handler.write.mhandler64)(handler->object, offset >> 3, data, mem_mask);

	profiler_mark(PROFILER_END);
}



/***************************************************************************
    CORE SYSTEM OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    memory_init - initialize the memory system
-------------------------------------------------*/

void memory_init(running_machine *machine)
{
	int spacenum;

	add_exit_callback(machine, memory_exit);

	/* no current context to start */
	cur_context = -1;
	for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		log_unmap[spacenum] = TRUE;

	/* reset the shared pointers and bank pointers */
	memset(shared_ptr, 0, sizeof(shared_ptr));
	memset(bank_ptr, 0, sizeof(bank_ptr));
	memset(bankd_ptr, 0, sizeof(bankd_ptr));

	/* build up the cpudata array with info about all CPUs and address spaces */
	memory_init_cpudata(machine);

	/* preflight the memory handlers and check banks */
	memory_init_preflight(machine);

	/* then fill in the tables */
	memory_init_populate(machine);

	/* allocate any necessary memory */
	memory_init_allocate(machine);

	/* find all the allocated pointers */
	memory_init_locate(machine);

	/* dump the final memory configuration */
	mem_dump();
}


/*-------------------------------------------------
    memory_exit - free memory
-------------------------------------------------*/

static void memory_exit(running_machine *machine)
{
	int cpunum, spacenum;

	/* free the memory blocks */
	while (memory_block_list != NULL)
	{
		memory_block *block = memory_block_list;
		memory_block_list = block->next;
		free(block);
	}

	/* free all the tables */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(cpudata); cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			addrspace_data *space = &cpudata[cpunum].space[spacenum];
			if (space->map != NULL)
				address_map_free(space->map);
			if (space->read.table != NULL)
				free(space->read.table);
			if (space->write.table != NULL)
				free(space->write.table);
		}

	/* free the global watchpoint table */
	if (wptable != NULL)
		free(wptable);
}


/*-------------------------------------------------
    memory_set_context - set the memory context
-------------------------------------------------*/

void memory_set_context(int activecpu)
{
	addrspace_data *space;

	/* remember dynamic RAM/ROM */
	if (activecpu == -1)
		activecpu = cur_context;
	else if (cur_context != -1)
		cpudata[cur_context].opbase = opbase;
	cur_context = activecpu;

	opbase = cpudata[activecpu].opbase;
	opbase_handler = cpudata[activecpu].opbase_handler;

	/* program address space */
	space = &cpudata[activecpu].space[ADDRESS_SPACE_PROGRAM];
	active_address_space[ADDRESS_SPACE_PROGRAM].bytemask = space->bytemask;
	active_address_space[ADDRESS_SPACE_PROGRAM].readlookup = ((Machine->debug_flags & DEBUG_FLAG_WPR_PROGRAM) != 0) ? wptable : space->read.table;
	active_address_space[ADDRESS_SPACE_PROGRAM].writelookup = ((Machine->debug_flags & DEBUG_FLAG_WPW_PROGRAM) != 0) ? wptable : space->write.table;
	active_address_space[ADDRESS_SPACE_PROGRAM].readhandlers = space->read.handlers;
	active_address_space[ADDRESS_SPACE_PROGRAM].writehandlers = space->write.handlers;
	active_address_space[ADDRESS_SPACE_PROGRAM].accessors = space->accessors;

	/* data address space */
	if (cpudata[activecpu].spacemask & (1 << ADDRESS_SPACE_DATA))
	{
		space = &cpudata[activecpu].space[ADDRESS_SPACE_DATA];
		active_address_space[ADDRESS_SPACE_DATA].bytemask = space->bytemask;
		active_address_space[ADDRESS_SPACE_DATA].readlookup = ((Machine->debug_flags & DEBUG_FLAG_WPR_DATA) != 0) ? wptable : space->read.table;
		active_address_space[ADDRESS_SPACE_DATA].writelookup = ((Machine->debug_flags & DEBUG_FLAG_WPW_DATA) != 0) ? wptable : space->write.table;
		active_address_space[ADDRESS_SPACE_DATA].readhandlers = space->read.handlers;
		active_address_space[ADDRESS_SPACE_DATA].writehandlers = space->write.handlers;
		active_address_space[ADDRESS_SPACE_DATA].accessors = space->accessors;
	}

	/* I/O address space */
	if (cpudata[activecpu].spacemask & (1 << ADDRESS_SPACE_IO))
	{
		space = &cpudata[activecpu].space[ADDRESS_SPACE_IO];
		active_address_space[ADDRESS_SPACE_IO].bytemask = space->bytemask;
		active_address_space[ADDRESS_SPACE_IO].readlookup = ((Machine->debug_flags & DEBUG_FLAG_WPR_IO) != 0) ? wptable : space->read.table;
		active_address_space[ADDRESS_SPACE_IO].writelookup = ((Machine->debug_flags & DEBUG_FLAG_WPW_IO) != 0) ? wptable : space->write.table;
		active_address_space[ADDRESS_SPACE_IO].readhandlers = space->read.handlers;
		active_address_space[ADDRESS_SPACE_IO].writehandlers = space->write.handlers;
		active_address_space[ADDRESS_SPACE_IO].accessors = space->accessors;
	}
}


/*-------------------------------------------------
    memory_get_accessors - get a pointer to the
    set of memory accessor functions based on
    the address space, databus width, and
    endianness
-------------------------------------------------*/

const data_accessors *memory_get_accessors(int spacenum, int databits, int endianness)
{
	int accessorindex = (databits == 8) ? 0 : (databits == 16) ? 1 : (databits == 32) ? 2 : 3;
	return &memory_accessors[spacenum][accessorindex][(endianness == CPU_IS_LE) ? 0 : 1];
}



/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

/*-------------------------------------------------
    address_map_alloc - build and allocate an
    address map for a CPU's address space
-------------------------------------------------*/

address_map *address_map_alloc(const machine_config *config, const game_driver *driver, int cpunum, int spacenum)
{
	int cputype = config->cpu[cpunum].type;
	const addrmap_token *internal_map = (const addrmap_token *)cputype_get_info_ptr(cputype, CPUINFO_PTR_INTERNAL_MEMORY_MAP + spacenum);
	address_map *map;

	map = malloc_or_die(sizeof(*map));
	memset(map, 0, sizeof(*map));

	/* start by constructing the internal CPU map */
	if (internal_map != NULL)
		address_map_detokenize(map, driver, internal_map);

	/* construct the standard map */
	if (config->cpu[cpunum].address_map[spacenum][0] != NULL)
		address_map_detokenize(map, driver, config->cpu[cpunum].address_map[spacenum][0]);
	if (config->cpu[cpunum].address_map[spacenum][1] != NULL)
		address_map_detokenize(map, driver, config->cpu[cpunum].address_map[spacenum][1]);

	return map;
}


/*-------------------------------------------------
    address_map_free - release allocated memory
    for an address map
-------------------------------------------------*/

void address_map_free(address_map *map)
{
	/* free all entries */
	while (map->entrylist != NULL)
	{
		address_map_entry *entry = map->entrylist;
		map->entrylist = entry->next;
		free(entry);
	}

	/* free the map */
	free(map);
}


/*-------------------------------------------------
    memory_get_address_map - return a pointer to
    the constructed address map for a CPU's
    address space
-------------------------------------------------*/

const address_map *memory_get_address_map(int cpunum, int spacenum)
{
	return cpudata[cpunum].space[spacenum].map;
}


/*-------------------------------------------------
    address_map_detokenize - detokenize an array
    of address map tokens
-------------------------------------------------*/

#define check_map(field) do { \
	if (map->field != 0 && map->field != tmap.field) \
		fatalerror("%s: %s included a mismatched address map (%s %d) for an existing map with %s %d!\n", driver->source_file, driver->name, #field, tmap.field, #field, map->field); \
	} while (0)


#define check_entry_handler(handler) do { \
	if (entry->handler.generic != NULL && entry->handler.generic != SMH_RAM) \
		fatalerror("%s: %s AM_RANGE(0x%x, 0x%x) %s handler already set!\n", driver->source_file, driver->name, entry->addrstart, entry->addrend, #handler); \
	} while (0)

#define check_entry_field(field) do { \
	if (entry->field != 0) \
		fatalerror("%s: %s AM_RANGE(0x%x, 0x%x) setting %s already set!\n", driver->source_file, driver->name, entry->addrstart, entry->addrend, #field); \
	} while (0)

static void address_map_detokenize(address_map *map, const game_driver *driver, const addrmap_token *tokens)
{
	address_map_entry **entryptr;
	address_map_entry *entry;
	address_map tmap = {0};
	UINT32 entrytype;

	/* check the first token */
	TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, tmap.spacenum, 8, tmap.databits, 8);
	if (entrytype != ADDRMAP_TOKEN_START)
		fatalerror("%s: %s Address map missing ADDRMAP_TOKEN_START!\n", driver->source_file, driver->name);
	if (tmap.spacenum >= ADDRESS_SPACES)
		fatalerror("%s: %s Invalid address space %d for memory map!\n", driver->source_file, driver->name, tmap.spacenum);
	if (tmap.databits != 8 && tmap.databits != 16 && tmap.databits != 32 && tmap.databits != 64)
		fatalerror("%s: %s Invalid data bits %d for memory map!\n", driver->source_file, driver->name, tmap.databits);
	check_map(spacenum);
	check_map(databits);

	/* fill in the map values */
	map->spacenum = tmap.spacenum;
	map->databits = tmap.databits;

	/* find the end of the list */
	for (entryptr = &map->entrylist; *entryptr != NULL; entryptr = &(*entryptr)->next) ;
	entry = NULL;

	/* loop over tokens until we hit the end */
	while (entrytype != ADDRMAP_TOKEN_END)
	{
		/* unpack the token from the first entry */
		TOKEN_GET_UINT32_UNPACK1(tokens, entrytype, 8);
		switch (entrytype)
		{
			/* end */
			case ADDRMAP_TOKEN_END:
				break;

			/* including */
			case ADDRMAP_TOKEN_INCLUDE:
				address_map_detokenize(map, driver, TOKEN_GET_PTR(tokens, tokenptr));
				for (entryptr = &map->entrylist; *entryptr != NULL; entryptr = &(*entryptr)->next) ;
				entry = NULL;
				break;

			/* global flags */
			case ADDRMAP_TOKEN_GLOBAL_MASK:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, tmap.globalmask, 32);
				check_map(globalmask);
				map->globalmask = tmap.globalmask;
				break;

			case ADDRMAP_TOKEN_UNMAP_VALUE:
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, tmap.unmapval, 1);
				check_map(unmapval);
				map->unmapval = tmap.unmapval;
				break;

			/* start a new range */
			case ADDRMAP_TOKEN_RANGE:
				entry = *entryptr = malloc_or_die(sizeof(**entryptr));
				entryptr = &entry->next;
				memset(entry, 0, sizeof(*entry));
				TOKEN_GET_UINT64_UNPACK2(tokens, entry->addrstart, 32, entry->addrend, 32);
				break;

			case ADDRMAP_TOKEN_MASK:
				check_entry_field(addrmask);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, entry->addrmask, 32);
				break;

			case ADDRMAP_TOKEN_MIRROR:
				check_entry_field(addrmirror);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, entry->addrmirror, 32);
				break;

			case ADDRMAP_TOKEN_READ:
				check_entry_handler(read);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, entry->read_bits, 8, entry->read_mask, 8);
				entry->read = TOKEN_GET_PTR(tokens, read);
				entry->read_name = TOKEN_GET_STRING(tokens);
				break;

			case ADDRMAP_TOKEN_WRITE:
				check_entry_handler(write);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, entry->write_bits, 8, entry->write_mask, 8);
				entry->write = TOKEN_GET_PTR(tokens, write);
				entry->write_name = TOKEN_GET_STRING(tokens);
				break;

			case ADDRMAP_TOKEN_DEVICE_READ:
				check_entry_handler(read);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, entry->read_bits, 8, entry->read_mask, 8);
				entry->read = TOKEN_GET_PTR(tokens, read);
				entry->read_name = TOKEN_GET_STRING(tokens);
				entry->read_devtype = TOKEN_GET_PTR(tokens, devtype);
				entry->read_devtag = TOKEN_GET_STRING(tokens);
				break;

			case ADDRMAP_TOKEN_DEVICE_WRITE:
				check_entry_handler(write);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, entry->write_bits, 8, entry->write_mask, 8);
				entry->write = TOKEN_GET_PTR(tokens, write);
				entry->write_name = TOKEN_GET_STRING(tokens);
				entry->write_devtype = TOKEN_GET_PTR(tokens, devtype);
				entry->write_devtag = TOKEN_GET_STRING(tokens);
				break;

			case ADDRMAP_TOKEN_READ_PORT:
				check_entry_field(read_porttag);
				entry->read_porttag = TOKEN_GET_STRING(tokens);
				break;

			case ADDRMAP_TOKEN_REGION:
				check_entry_field(region);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, entry->rgnoffs, 32);
				entry->region = TOKEN_GET_STRING(tokens);
				break;

			case ADDRMAP_TOKEN_SHARE:
				check_entry_field(share);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, entry->share, 24);
				break;

			case ADDRMAP_TOKEN_BASEPTR:
				check_entry_field(baseptr);
				entry->baseptr = (void **)TOKEN_GET_PTR(tokens, voidptr);
				break;

			case ADDRMAP_TOKEN_BASE_MEMBER:
				check_entry_field(baseptroffs_plus1);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, entry->baseptroffs_plus1, 24);
				entry->baseptroffs_plus1++;
				break;

			case ADDRMAP_TOKEN_SIZEPTR:
				check_entry_field(sizeptr);
				entry->sizeptr = TOKEN_GET_PTR(tokens, sizeptr);
				break;

			case ADDRMAP_TOKEN_SIZE_MEMBER:
				check_entry_field(sizeptroffs_plus1);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK2(tokens, entrytype, 8, entry->sizeptroffs_plus1, 24);
				entry->sizeptroffs_plus1++;
				break;

			default:
				fatalerror("Invalid token %d in address map\n", entrytype);
				break;
		}
	}
}



/***************************************************************************
    OPCODE BASE CONTROL
***************************************************************************/

/*-------------------------------------------------
    memory_set_decrypted_region - registers an
    address range as having a decrypted data
    pointer
-------------------------------------------------*/

void memory_set_decrypted_region(int cpunum, offs_t addrstart, offs_t addrend, void *base)
{
	addrspace_data *space = &cpudata[cpunum].space[ADDRESS_SPACE_PROGRAM];
	offs_t bytestart = ADDR2BYTE(space, addrstart);
	offs_t byteend = ADDR2BYTE_END(space, addrend);
	int banknum, found = FALSE;

	/* loop over banks looking for a match */
	for (banknum = 0; banknum < STATIC_COUNT; banknum++)
	{
		bank_info *bank = &bankdata[banknum];

		/* consider this bank if it is used for reading and matches the CPU/address space */
		if (bank->used && bank->read && bank->cpunum == cpunum && bank->spacenum == ADDRESS_SPACE_PROGRAM)
		{
			/* verify that the region fully covers the decrypted range */
			if (bank->bytestart >= bytestart && bank->byteend <= byteend)
			{
				/* set the decrypted pointer for the corresponding memory bank */
				bankd_ptr[banknum] = (UINT8 *)base + bank->bytestart - bytestart;
				found = TRUE;

				/* if we are executing from here, force an opcode base update */
				if (cpu_getactivecpu() >= 0 && cpunum == cur_context && opbase.entry == banknum)
					force_opbase_update();
			}

			/* fatal error if the decrypted region straddles the bank */
			else if (bank->bytestart < byteend && bank->byteend > bytestart)
				fatalerror("memory_set_decrypted_region found straddled region %08X-%08X for CPU %d", bytestart, byteend, cpunum);
		}
	}

	/* fatal error as well if we didn't find any relevant memory banks */
	if (!found)
		fatalerror("memory_set_decrypted_region unable to find matching region %08X-%08X for CPU %d", bytestart, byteend, cpunum);
}


/*-------------------------------------------------
    memory_set_opbase_handler - register a
    handler for opcode base changes on a given
    CPU
-------------------------------------------------*/

opbase_handler_func memory_set_opbase_handler(int cpunum, opbase_handler_func function)
{
	opbase_handler_func old = cpudata[cpunum].opbase_handler;
	cpudata[cpunum].opbase_handler = function;
	if (cpunum == cpu_getactivecpu())
		opbase_handler = function;
	return old;
}


/*-------------------------------------------------
    memory_set_opbase - called by CPU cores to
    update the opcode base for the given address
-------------------------------------------------*/

void memory_set_opbase(offs_t byteaddress)
{
	const address_space *space = &active_address_space[ADDRESS_SPACE_PROGRAM];
	UINT8 *base = NULL, *based = NULL;
	const handler_data *handlers;
	UINT8 entry;

	/* allow overrides */
	if (opbase_handler != NULL)
	{
		byteaddress = (*opbase_handler)(Machine, byteaddress, &opbase);
		if (byteaddress == ~0)
			return;
	}

	/* perform the lookup */
	byteaddress &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry,byteaddress)];

	/* keep track of current entry */
	opbase.entry = entry;

	/* if we don't map to a bank, see if there are any banks we can map to */
	if (entry < STATIC_BANK1 || entry >= STATIC_RAM)
	{
		/* loop over banks and find a match */
		for (entry = 1; entry < STATIC_COUNT; entry++)
		{
			bank_info *bank = &bankdata[entry];
			if (bank->used && bank->cpunum == cur_context && bank->spacenum == ADDRESS_SPACE_PROGRAM &&
				bank->bytestart < byteaddress && bank->byteend > byteaddress)
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
	opbase.mask = handlers->bytemask;
	opbase.ram = base - (handlers->bytestart & opbase.mask);
	opbase.rom = based - (handlers->bytestart & opbase.mask);
	opbase.mem_min = handlers->bytestart;
	opbase.mem_max = handlers->byteend;
}



/***************************************************************************
    OPCODE BASE CONTROL
***************************************************************************/

/*-------------------------------------------------
    memory_get_read_ptr - return a pointer to the
    base of RAM associated with the given CPU
    and offset
-------------------------------------------------*/

void *memory_get_read_ptr(int cpunum, int spacenum, offs_t byteaddress)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	offs_t byteoffset;
	UINT8 entry;

	/* perform the lookup */
	byteaddress &= space->bytemask;
	entry = space->read.table[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->read.table[LEVEL2_INDEX(entry, byteaddress)];

	/* 8-bit case: RAM/ROM */
	if (entry >= STATIC_RAM)
		return NULL;
	byteoffset = (byteaddress - space->read.handlers[entry].bytestart) & space->read.handlers[entry].bytemask;
	return &bank_ptr[entry][byteoffset];
}


/*-------------------------------------------------
    memory_get_write_ptr - return a pointer to the
    base of RAM associated with the given CPU
    and offset
-------------------------------------------------*/

void *memory_get_write_ptr(int cpunum, int spacenum, offs_t byteaddress)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	offs_t byteoffset;
	UINT8 entry;

	/* perform the lookup */
	byteaddress &= space->bytemask;
	entry = space->write.table[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->write.table[LEVEL2_INDEX(entry, byteaddress)];

	/* 8-bit case: RAM/ROM */
	if (entry >= STATIC_RAM)
		return NULL;
	byteoffset = (byteaddress - space->write.handlers[entry].bytestart) & space->write.handlers[entry].bytemask;
	return &bank_ptr[entry][byteoffset];
}


/*-------------------------------------------------
    memory_get_op_ptr - return a pointer to the
    base of opcode RAM associated with the given
    CPU and offset
-------------------------------------------------*/

void *memory_get_op_ptr(int cpunum, offs_t byteaddress, int arg)
{
	addrspace_data *space = &cpudata[cpunum].space[ADDRESS_SPACE_PROGRAM];
	offs_t byteoffset;
	void *ptr = NULL;
	UINT8 entry;

	/* if there is a custom mapper, use that */
	if (cpudata[cpunum].opbase_handler != NULL)
	{
		/* need to save opcode info */
		opbase_data saved_opbase = opbase;

		/* query the handler */
		offs_t new_byteaddress = (*cpudata[cpunum].opbase_handler)(Machine, byteaddress, &opbase);

		/* if it returns ~0, we use whatever data the handler set */
		if (new_byteaddress == ~0)
			ptr = arg ? &opbase.ram[byteaddress] : &opbase.rom[byteaddress];

		/* otherwise, we use the new offset in the generic case below */
		else
			byteaddress = new_byteaddress;

		/* restore opcode info */
		opbase = saved_opbase;

		/* if we got our pointer, we're done */
		if (ptr != NULL)
			return ptr;
	}

	/* perform the lookup */
	byteaddress &= space->bytemask;
	entry = space->read.table[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->read.table[LEVEL2_INDEX(entry, byteaddress)];

	/* if a non-RAM area, return NULL */
	if (entry >= STATIC_RAM)
		return NULL;

	/* adjust the offset */
	byteoffset = (byteaddress - space->read.handlers[entry].bytestart) & space->read.handlers[entry].bytemask;
	return (!arg && bankd_ptr[entry]) ? &bankd_ptr[entry][byteoffset] : &bank_ptr[entry][byteoffset];
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
	if (opbase.entry == banknum && cpu_getactivecpu() >= 0)
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
		validate_auto_malloc_memory(base, bankdata[banknum].byteend - bankdata[banknum].bytestart + 1);

	/* set the base */
	bank_ptr[banknum] = base;

	/* if we're executing out of this bank, adjust the opbase pointer */
	if (opbase.entry == banknum && cpu_getactivecpu() >= 0)
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
    memory_install_handlerX - install
    dynamic machine read and write handlers for
    X-bit case
-------------------------------------------------*/

void *_memory_install_handler(running_machine *machine, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, FPTR rhandler, FPTR whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler >= STATIC_COUNT || whandler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_handler()");
	if (rhandler != 0)
		space_map_range(space, ROW_READ, space->dbits, 0, addrstart, addrend, addrmask, addrmirror, (genf *)(FPTR)rhandler, machine, rhandler_name);
	if (whandler != 0)
		space_map_range(space, ROW_WRITE, space->dbits, 0, addrstart, addrend, addrmask, addrmirror, (genf *)(FPTR)whandler, machine, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}

UINT8 *_memory_install_handler8(running_machine *machine, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_machine_func rhandler, write8_machine_func whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler != NULL)
		space_map_range(space, ROW_READ, 8, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, machine, rhandler_name);
	if (whandler != NULL)
		space_map_range(space, ROW_WRITE, 8, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, machine, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}

UINT16 *_memory_install_handler16(running_machine *machine, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_machine_func rhandler, write16_machine_func whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler != NULL)
		space_map_range(space, ROW_READ, 16, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, machine, rhandler_name);
	if (whandler != NULL)
		space_map_range(space, ROW_WRITE, 16, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, machine, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}

UINT32 *_memory_install_handler32(running_machine *machine, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_machine_func rhandler, write32_machine_func whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler != NULL)
		space_map_range(space, ROW_READ, 32, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, machine, rhandler_name);
	if (whandler != NULL)
		space_map_range(space, ROW_WRITE, 32, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, machine, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}

UINT64 *_memory_install_handler64(running_machine *machine, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_machine_func rhandler, write64_machine_func whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler != NULL)
		space_map_range(space, ROW_READ, 64, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, machine, rhandler_name);
	if (whandler != NULL)
		space_map_range(space, ROW_WRITE, 64, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, machine, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}


/*-------------------------------------------------
    memory_install_device_handlerX -
    install dynamic device read and write handlers
    for X-bit case
-------------------------------------------------*/

void *_memory_install_device_handler(const device_config *device, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, FPTR rhandler, FPTR whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler >= STATIC_COUNT || whandler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_device_handler()");
	if (rhandler != 0)
		space_map_range(space, ROW_READ, space->dbits, 0, addrstart, addrend, addrmask, addrmirror, (genf *)(FPTR)rhandler, (void *)device, rhandler_name);
	if (whandler != 0)
		space_map_range(space, ROW_WRITE, space->dbits, 0, addrstart, addrend, addrmask, addrmirror, (genf *)(FPTR)whandler, (void *)device, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}

UINT8 *_memory_install_device_handler8(const device_config *device, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, write8_device_func whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler != NULL)
		space_map_range(space, ROW_READ, 8, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, (void *)device, rhandler_name);
	if (whandler != NULL)
		space_map_range(space, ROW_WRITE, 8, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, (void *)device, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}

UINT16 *_memory_install_device_handler16(const device_config *device, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, write16_device_func whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler != NULL)
		space_map_range(space, ROW_READ, 16, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, (void *)device, rhandler_name);
	if (whandler != NULL)
		space_map_range(space, ROW_WRITE, 16, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, (void *)device, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}

UINT32 *_memory_install_device_handler32(const device_config *device, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, write32_device_func whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler != NULL)
		space_map_range(space, ROW_READ, 32, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, (void *)device, rhandler_name);
	if (whandler != NULL)
		space_map_range(space, ROW_WRITE, 32, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, (void *)device, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}

UINT64 *_memory_install_device_handler64(const device_config *device, int cpunum, int spacenum, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, write64_device_func whandler, const char *rhandler_name, const char *whandler_name)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	if (rhandler != NULL)
		space_map_range(space, ROW_READ, 64, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, (void *)device, rhandler_name);
	if (whandler != NULL)
		space_map_range(space, ROW_WRITE, 64, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, (void *)device, whandler_name);
	mem_dump();
	return memory_find_base(cpunum, spacenum, ADDR2BYTE(space, addrstart));
}


/*-------------------------------------------------
    memory_init_cpudata - initialize the cpudata
    structure for each CPU
-------------------------------------------------*/

static void memory_init_cpudata(running_machine *machine)
{
	const machine_config *config = machine->config;
	int cpunum, spacenum;

	/* zap the cpudata structure */
	memset(&cpudata, 0, sizeof(cpudata));

	/* create a global watchpoint-filled table */
	wptable = malloc_or_die(1 << LEVEL1_BITS);
	memset(wptable, STATIC_WATCHPOINT, 1 << LEVEL1_BITS);

	/* loop over CPUs */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(cpudata); cpunum++)
	{
		cpu_type cputype = config->cpu[cpunum].type;
		cpu_data *cpu = &cpudata[cpunum];

		/* get pointers to the CPU's memory region */
		cpu->tag = config->cpu[cpunum].tag;
		cpu->region = memory_region(machine, cpu->tag);
		cpu->regionsize = memory_region_length(machine, cpu->tag);

		/* initialize each address space, and build up a mask of spaces */
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			addrspace_data *space = &cpu->space[spacenum];
			int entrynum;

			/* determine the address and data bits */
			space->cpunum = cpunum;
			space->spacenum = spacenum;
			space->endianness = cputype_endianness(cputype);
			space->ashift = cputype_addrbus_shift(cputype, spacenum);
			space->abits = cputype_addrbus_width(cputype, spacenum);
			space->dbits = cputype_databus_width(cputype, spacenum);
			space->addrmask = 0xffffffffUL >> (32 - space->abits);
			space->bytemask = ADDR2BYTE_END(space, space->addrmask);
			space->accessors = memory_get_accessors(spacenum, space->dbits, space->endianness);
			space->map = NULL;

			/* if there's nothing here, just punt */
			if (space->abits == 0)
				continue;
			cpu->spacemask |= 1 << spacenum;

			/* init the static handlers */
			memset(space->read.handlers, 0, sizeof(space->read.handlers));
			memset(space->write.handlers, 0, sizeof(space->write.handlers));
			for (entrynum = 0; entrynum < ENTRY_COUNT; entrynum++)
			{
				space->read.handlers[entrynum].handler.generic = get_static_handler(space->dbits, 0, spacenum, entrynum);
				space->write.handlers[entrynum].handler.generic = get_static_handler(space->dbits, 1, spacenum, entrynum);
			}

			/* make sure we fix up the mask for the unmap and watchpoint handlers */
			space->read.handlers[STATIC_UNMAP].bytemask = ~0;
			space->write.handlers[STATIC_UNMAP].bytemask = ~0;
			space->read.handlers[STATIC_WATCHPOINT].bytemask = ~0;
			space->write.handlers[STATIC_WATCHPOINT].bytemask = ~0;

			/* allocate memory */
			space->read.table = malloc_or_die(1 << LEVEL1_BITS);
			space->write.table = malloc_or_die(1 << LEVEL1_BITS);

			/* initialize everything to unmapped */
			memset(space->read.table, STATIC_UNMAP, 1 << LEVEL1_BITS);
			memset(space->write.table, STATIC_UNMAP, 1 << LEVEL1_BITS);
		}

		/* set the RAM/ROM base */
		cpu->opbase.ram = cpu->opbase.rom = cpu->region;
		cpu->opbase.mask = cpu->space[ADDRESS_SPACE_PROGRAM].bytemask;
		cpu->opbase.mem_min = 0;
		cpu->opbase.mem_max = cpu->regionsize;
		cpu->opbase.entry = STATIC_UNMAP;
		cpu->opbase_handler = NULL;
	}
}


/*-------------------------------------------------
    memory_init_preflight - verify the memory structs
    and track which banks are referenced
-------------------------------------------------*/

static void memory_init_preflight(running_machine *machine)
{
	int cpunum;

	/* zap the bank data */
	memset(&bankdata, 0, sizeof(bankdata));

	/* loop over CPUs */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(cpudata); cpunum++)
	{
		cpu_data *cpu = &cpudata[cpunum];
		int spacenum;

		/* loop over valid address spaces */
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpu->spacemask & (1 << spacenum))
			{
				addrspace_data *space = &cpu->space[spacenum];
				address_map_entry *entry;
				int entrynum;

				/* allocate the address map */
				space->map = address_map_alloc(machine->config, machine->gamedrv, cpunum, spacenum);

				/* extract global parameters specified by the map */
				space->unmap = (space->map->unmapval == 0) ? 0 : ~0;
				if (space->map->globalmask != 0)
				{
					space->addrmask = space->map->globalmask;
					space->bytemask = ADDR2BYTE_END(space, space->addrmask);
				}

				/* make a pass over the address map, adjusting for the CPU and getting memory pointers */
				for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
				{
					/* computed adjusted addresses first */
					entry->bytestart = entry->addrstart;
					entry->byteend = entry->addrend;
					entry->bytemirror = entry->addrmirror;
					entry->bytemask = entry->addrmask;
					adjust_addresses(space, &entry->bytestart, &entry->byteend, &entry->bytemask, &entry->bytemirror);

					/* if this is a ROM handler without a specified region, attach it to the implicit region */
					if (spacenum == ADDRESS_SPACE_PROGRAM && HANDLER_IS_ROM(entry->read.generic) && entry->region == NULL)
					{
						/* make sure it fits within the memory region before doing so, however */
						if (entry->byteend < cpu->regionsize)
						{
							entry->region = cpu->tag;
							entry->rgnoffs = entry->bytestart;
						}
					}

					/* validate adjusted addresses against implicit regions */
					if (entry->region != NULL && entry->share == 0 && entry->baseptr == NULL)
					{
						UINT8 *base = memory_region(machine, entry->region);
						offs_t length = memory_region_length(machine, entry->region);

						/* validate the region */
						if (base == NULL)
							fatalerror("Error: CPU %d space %d memory map entry %X-%X references non-existant region \"%s\"", cpunum, spacenum, entry->addrstart, entry->addrend, entry->region);
						if (entry->rgnoffs + (entry->byteend - entry->bytestart + 1) > length)
							fatalerror("Error: CPU %d space %d memory map entry %X-%X extends beyond region \"%s\" size (%X)", cpunum, spacenum, entry->addrstart, entry->addrend, entry->region, length);
					}

					/* convert any region-relative entries to their memory pointers */
					if (entry->region != NULL)
						entry->memory = memory_region(machine, entry->region) + entry->rgnoffs;

					/* assign static banks for explicitly specified entries */
					if (HANDLER_IS_BANK(entry->read.generic))
						bank_assign_static(HANDLER_TO_BANK(entry->read.generic), cpunum, spacenum, ROW_READ, entry->bytestart, entry->byteend);
					if (HANDLER_IS_BANK(entry->write.generic))
						bank_assign_static(HANDLER_TO_BANK(entry->write.generic), cpunum, spacenum, ROW_WRITE, entry->bytestart, entry->byteend);
				}

				/* now loop over all the handlers and enforce the address mask */
				/* we don't loop over map entries because the mask applies to static handlers as well */
				for (entrynum = 0; entrynum < ENTRY_COUNT; entrynum++)
				{
					space->read.handlers[entrynum].bytemask &= space->bytemask;
					space->write.handlers[entrynum].bytemask &= space->bytemask;
				}
			}
	}
}


/*-------------------------------------------------
    memory_init_populate - populate the memory
    mapping tables with entries
-------------------------------------------------*/

static void memory_init_populate(running_machine *machine)
{
	int cpunum, spacenum;

	/* loop over CPUs and address spaces */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(cpudata); cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].spacemask & (1 << spacenum))
			{
				addrspace_data *space = &cpudata[cpunum].space[spacenum];

				/* install the handlers, using the original, unadjusted memory map */
				if (space->map != NULL)
				{
					const address_map_entry *last_entry = NULL;

					while (last_entry != space->map->entrylist)
					{
						const address_map_entry *entry;
						read_handler rhandler;
						write_handler whandler;

						/* find the entry before the last one we processed */
						for (entry = space->map->entrylist; entry->next != last_entry; entry = entry->next) ;
						last_entry = entry;
						rhandler = entry->read;
						whandler = entry->write;

						/* if we have a read port tag, look it up */
						if (entry->read_porttag != NULL)
						{
							switch (space->dbits)
							{
								case 8:		rhandler.mhandler8  = input_port_read_handler8(machine->portconfig, entry->read_porttag);	break;
								case 16:	rhandler.mhandler16 = input_port_read_handler16(machine->portconfig, entry->read_porttag);	break;
								case 32:	rhandler.mhandler32 = input_port_read_handler32(machine->portconfig, entry->read_porttag);	break;
								case 64:	rhandler.mhandler64 = input_port_read_handler64(machine->portconfig, entry->read_porttag);	break;
							}
							if (rhandler.generic == NULL)
								fatalerror("Non-existent port referenced: '%s'\n", entry->read_porttag);
						}

						/* install the read handler if present */
						if (rhandler.generic != NULL)
						{
							int bits = (entry->read_bits == 0) ? space->dbits : entry->read_bits;
							void *object = machine;
							if (entry->read_devtype != NULL)
							{
								object = (void *)device_list_find_by_tag(machine->config->devicelist, entry->read_devtype, entry->read_devtag);
								if (object == NULL)
									fatalerror("Unidentified object in memory map: type=%s tag=%s\n", devtype_name(entry->read_devtype), entry->read_devtag);
							}
							space_map_range_private(space, ROW_READ, bits, entry->read_mask, entry->addrstart, entry->addrend, entry->addrmask, entry->addrmirror, rhandler.generic, object, entry->read_name);
						}

						/* install the write handler if present */
						if (whandler.generic != NULL)
						{
							int bits = (entry->write_bits == 0) ? space->dbits : entry->write_bits;
							void *object = machine;
							if (entry->write_devtype != NULL)
							{
								object = (void *)device_list_find_by_tag(machine->config->devicelist, entry->write_devtype, entry->write_devtag);
								if (object == NULL)
									fatalerror("Unidentified object in memory map: type=%s tag=%s\n", devtype_name(entry->write_devtype), entry->write_devtag);
							}
							space_map_range_private(space, ROW_WRITE, bits, entry->write_mask, entry->addrstart, entry->addrend, entry->addrmask, entry->addrmirror, whandler.generic, object, entry->write_name);
						}
					}
				}
			}
}


/*-------------------------------------------------
    space_map_range_private - wrapper for
    space_map_range which is used at
    initialization time and converts RAM/ROM
    banks to dynamically assigned banks
-------------------------------------------------*/

static void space_map_range_private(addrspace_data *space, read_or_write readorwrite, int handlerbits, int handlerunitmask, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, genf *handler, void *object, const char *handler_name)
{
	/* translate ROM to RAM/UNMAP here */
	if (HANDLER_IS_ROM(handler))
		handler = (readorwrite == ROW_WRITE) ? (genf *)STATIC_UNMAP : (genf *)SMH_RAM;

	/* assign banks for RAM/ROM areas */
	if (HANDLER_IS_RAM(handler))
	{
		offs_t bytestart = addrstart;
		offs_t byteend = addrend;
		offs_t bytemask = addrmask;
		offs_t bytemirror = addrmirror;

		/* adjust the incoming addresses (temporarily) */
		adjust_addresses(space, &bytestart, &byteend, &bytemask, &bytemirror);

		/* assign a bank to the adjusted addresses */
		handler = (genf *)bank_assign_dynamic(space->cpunum, space->spacenum, readorwrite, bytestart, byteend);
		if (bank_ptr[HANDLER_TO_BANK(handler)] == NULL)
			bank_ptr[HANDLER_TO_BANK(handler)] = memory_find_base(space->cpunum, space->spacenum, bytestart);
	}

	/* then do a normal installation */
	space_map_range(space, readorwrite, handlerbits, handlerunitmask, addrstart, addrend, addrmask, addrmirror, handler, object, handler_name);
}


/*-------------------------------------------------
    space_map_range - maps a range of addresses
    to the specified handler within an address
    space
-------------------------------------------------*/

static void space_map_range(addrspace_data *space, read_or_write readorwrite, int handlerbits, int handlerunitmask, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, genf *handler, void *object, const char *handler_name)
{
	table_data *tabledata = (readorwrite == ROW_WRITE) ? &space->write : &space->read;
	offs_t bytestart, byteend, bytemask, bytemirror;
	UINT8 entry;

	/* sanity checks */
	assert(space != NULL);
	assert(handlerbits == 8 || handlerbits == 16 || handlerbits == 32 || handlerbits == 64);

	/* adjust the incoming addresses */
	bytestart = addrstart;
	byteend = addrend;
	bytemirror = addrmirror;
	bytemask = addrmask;
	adjust_addresses(space, &bytestart, &byteend, &bytemask, &bytemirror);

	/* validity checks */
	assert_always(!HANDLER_IS_ROM(handler), "space_map_range called with ROM after initialization");
	assert_always(!HANDLER_IS_RAM(handler), "space_map_range called with RAM after initialization");
	assert_always(addrstart <= addrend, "space_map_range called with start greater than end");
	assert_always(handlerbits <= space->dbits, "space_map_range called with handlers larger than the address space");
	assert_always((bytestart & (space->dbits / 8 - 1)) == 0, "space_map_range called with misaligned start address");
	assert_always((byteend & (space->dbits / 8 - 1)) == (space->dbits / 8 - 1), "space_map_range called with misaligned end address");

	/* if we're installing a new bank, make sure we mark it */
	if (HANDLER_IS_BANK(handler))
		bank_assign_static(HANDLER_TO_BANK(handler), space->cpunum, space->spacenum, readorwrite, bytestart, byteend);

	/* get the final handler index */
	entry = table_assign_handler(tabledata->handlers, object, handler, handler_name, bytestart, byteend, bytemask);

	/* fix up the handler if a stub is required */
	if (handlerbits != space->dbits)
		table_compute_subhandler(tabledata->handlers, entry, readorwrite, space->dbits, space->endianness, handlerbits, handlerunitmask);

	/* populate it */
	table_populate_range_mirrored(tabledata, bytestart, byteend, bytemirror, entry);

	/* if this is being installed to a live CPU, update the context */
	if (space->cpunum == cur_context)
		memory_set_context(cur_context);
}


/*-------------------------------------------------
    bank_assign_static - assign and tag a static
    bank
-------------------------------------------------*/

static void bank_assign_static(int banknum, int cpunum, int spacenum, read_or_write readorwrite, offs_t bytestart, offs_t byteend)
{
	bank_info *bank = &bankdata[banknum];

	/* if we're not yet used, fill in the data */
	if (!bank->used)
	{
		/* if we're allowed to, wire up state saving for the entry */
		if (state_save_registration_allowed())
			state_save_register_item("memory", banknum, bank->curentry);

		/* fill in information about the bank */
		bank->used = TRUE;
		bank->dynamic = FALSE;
		bank->cpunum = cpunum;
		bank->spacenum = spacenum;
		bank->bytestart = bytestart;
		bank->byteend = byteend;
		bank->curentry = MAX_BANK_ENTRIES;
	}

	/* update the read/write status of the bank */
	if (readorwrite == ROW_READ)
		bank->read = TRUE;
	else
		bank->write = TRUE;
}


/*-------------------------------------------------
    bank_assign_dynamic - finds a free or exact
    matching bank
-------------------------------------------------*/

static genf *bank_assign_dynamic(int cpunum, int spacenum, read_or_write readorwrite, offs_t bytestart, offs_t byteend)
{
	int banknum;

	/* loop over banks, searching for an exact match or an empty */
	for (banknum = MAX_BANKS; banknum >= 1; banknum--)
	{
		bank_info *bank = &bankdata[banknum];
		if (!bank->used || (bank->dynamic && bank->cpunum == cpunum && bank->spacenum == spacenum && bank->bytestart == bytestart))
		{
			bank->used = TRUE;
			bank->dynamic = TRUE;
			bank->cpunum = cpunum;
			bank->spacenum = spacenum;
			bank->bytestart = bytestart;
			bank->byteend = byteend;
			VPRINTF(("Assigned bank %d to %d,%d,%08X\n", banknum, cpunum, spacenum, bytestart));
			return BANK_TO_HANDLER(banknum);
		}
	}

	/* if we got here, we failed */
	fatalerror("cpu #%d: ran out of banks for RAM/ROM regions!", cpunum);
	return NULL;
}


/*-------------------------------------------------
    table_assign_handler - finds the index of a
    handler, or allocates a new one as necessary
-------------------------------------------------*/

static UINT8 table_assign_handler(handler_data *table, void *object, genf *handler, const char *handler_name, offs_t bytestart, offs_t byteend, offs_t bytemask)
{
	int entry;

	/* all static handlers are hardcoded */
	if (HANDLER_IS_STATIC(handler))
	{
		entry = (FPTR)handler;

		/* if it is a bank, copy in the relevant information */
		if (HANDLER_IS_BANK(handler))
		{
			handler_data *hdata = &table[entry];
			hdata->bytestart = bytestart;
			hdata->byteend = byteend;
			hdata->bytemask = bytemask;
			hdata->name = handler_name;
		}
		return entry;
	}

	/* otherwise, we have to search */
	for (entry = STATIC_COUNT; entry < SUBTABLE_BASE; entry++)
	{
		handler_data *hdata = &table[entry];

		/* if we hit a NULL hdata, then we need to allocate this one as a new one */
		if (hdata->handler.generic == NULL)
		{
			hdata->handler.generic = handler;
			hdata->bytestart = bytestart;
			hdata->byteend = byteend;
			hdata->bytemask = bytemask;
			hdata->name = handler_name;
			hdata->object = object;
			return entry;
		}

		/* if we find a perfect match, return a duplicate entry */
		if (hdata->handler.generic == handler && hdata->bytestart == bytestart && hdata->bytemask == bytemask && hdata->object == object)
			return entry;
	}
	return 0;
}


/*-------------------------------------------------
    table_compute_subhandler - compute data for
    a subhandler
-------------------------------------------------*/

static void table_compute_subhandler(handler_data *table, UINT8 entry, read_or_write readorwrite, int spacebits, int spaceendian, int handlerbits, int handlerunitmask)
{
	int maxunits = spacebits / handlerbits;
	handler_data *hdata = &table[entry];
	int unitnum;

	assert_always(!HANDLER_IS_STATIC(entry), "table_compute_subhandler called with static handlers and mismatched data bus widths");

	/* copy raw data to the subhandler data */
	hdata->subobject = hdata->object;
	hdata->subhandler = hdata->handler;

	/* fill in a stub as the real handler */
	hdata->object = hdata;
	hdata->handler = get_stub_handler(readorwrite, spacebits, handlerbits);

	/* compute the number of subunits */
	hdata->subunits = 0;
	for (unitnum = 0; unitnum < maxunits; unitnum++)
		if (handlerunitmask & (1 << unitnum))
			hdata->subunits++;
	assert_always(hdata->subunits > 0, "table_compute_subhandler called with no bytes specified in mask");

	/* then fill in the shifts based on the endianness */
	if (spaceendian == CPU_IS_LE)
	{
		UINT8 *unitshift = &hdata->subshift[0];
		for (unitnum = 0; unitnum < maxunits; unitnum++)
			if (handlerunitmask & (1 << unitnum))
				*unitshift++ = unitnum * handlerbits;
	}
	else
	{
		UINT8 *unitshift = &hdata->subshift[hdata->subunits];
		for (unitnum = 0; unitnum < maxunits; unitnum++)
			if (handlerunitmask & (1 << unitnum))
				*--unitshift = unitnum * handlerbits;
	}
}


/*-------------------------------------------------
    table_populate_range - assign a memory handler
    to a range of addresses
-------------------------------------------------*/

static void table_populate_range(table_data *tabledata, offs_t bytestart, offs_t byteend, UINT8 handler)
{
	offs_t l2mask = (1 << LEVEL2_BITS) - 1;
	offs_t l1start = bytestart >> LEVEL2_BITS;
	offs_t l2start = bytestart & l2mask;
	offs_t l1stop = byteend >> LEVEL2_BITS;
	offs_t l2stop = byteend & l2mask;
	offs_t l1index;

	/* sanity check */
	if (bytestart > byteend)
		return;

	/* handle the starting edge if it's not on a block boundary */
	if (l2start != 0)
	{
		UINT8 *subtable = subtable_open(tabledata, l1start);

		/* if the start and stop end within the same block, handle that */
		if (l1start == l1stop)
		{
			memset(&subtable[l2start], handler, l2stop - l2start + 1);
			subtable_close(tabledata, l1start);
			return;
		}

		/* otherwise, fill until the end */
		memset(&subtable[l2start], handler, (1 << LEVEL2_BITS) - l2start);
		subtable_close(tabledata, l1start);
		if (l1start != (offs_t)~0) l1start++;
	}

	/* handle the trailing edge if it's not on a block boundary */
	if (l2stop != l2mask)
	{
		UINT8 *subtable = subtable_open(tabledata, l1stop);

		/* fill from the beginning */
		memset(&subtable[0], handler, l2stop + 1);
		subtable_close(tabledata, l1stop);

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
			subtable_release(tabledata, tabledata->table[l1index]);
		tabledata->table[l1index] = handler;
	}
}


/*-------------------------------------------------
    table_populate_range_mirrored - assign a
    memory handler to a range of addresses
    including mirrors
-------------------------------------------------*/

static void table_populate_range_mirrored(table_data *tabledata, offs_t bytestart, offs_t byteend, offs_t bytemirror, UINT8 handler)
{
	offs_t lmirrorbit[LEVEL2_BITS], lmirrorbits, hmirrorbit[32 - LEVEL2_BITS], hmirrorbits, lmirrorcount, hmirrorcount;
	UINT8 prev_entry = STATIC_INVALID;
	int cur_index, prev_index = 0;
	int i;

	/* determine the mirror bits */
	hmirrorbits = lmirrorbits = 0;
	for (i = 0; i < LEVEL2_BITS; i++)
		if (bytemirror & (1 << i))
			lmirrorbit[lmirrorbits++] = 1 << i;
	for (i = LEVEL2_BITS; i < 32; i++)
		if (bytemirror & (1 << i))
			hmirrorbit[hmirrorbits++] = 1 << i;

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
		cur_index = LEVEL1_INDEX(bytestart + hmirrorbase);
		if (cur_index == LEVEL1_INDEX(byteend + hmirrorbase))
		{
			if (hmirrorcount != 0 && prev_entry == tabledata->table[cur_index])
			{
				VPRINTF(("Quick mapping subtable at %08X to match subtable at %08X\n", cur_index << LEVEL2_BITS, prev_index << LEVEL2_BITS));

				/* release the subtable if the old value was a subtable */
				if (tabledata->table[cur_index] >= SUBTABLE_BASE)
					subtable_release(tabledata, tabledata->table[cur_index]);

				/* reallocate the subtable if the new value is a subtable */
				if (tabledata->table[prev_index] >= SUBTABLE_BASE)
					subtable_realloc(tabledata, tabledata->table[prev_index]);

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
			table_populate_range(tabledata, bytestart + lmirrorbase, byteend + lmirrorbase, handler);
		}
	}
}


/*-------------------------------------------------
    subtable_alloc - allocate a fresh subtable
    and set its usecount to 1
-------------------------------------------------*/

static UINT8 subtable_alloc(table_data *tabledata)
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
		if (!subtable_merge(tabledata))
			fatalerror("Ran out of subtables!");
	}

}


/*-------------------------------------------------
    subtable_realloc - increment the usecount on
    a subtable
-------------------------------------------------*/

static void subtable_realloc(table_data *tabledata, UINT8 subentry)
{
	UINT8 subindex = subentry - SUBTABLE_BASE;

	/* sanity check */
	if (tabledata->subtable[subindex].usecount <= 0)
		fatalerror("Called subtable_realloc on a table with a usecount of 0");

	/* increment the usecount */
	tabledata->subtable[subindex].usecount++;
}


/*-------------------------------------------------
    subtable_merge - merge any duplicate
    subtables
-------------------------------------------------*/

static int subtable_merge(table_data *tabledata)
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
							subtable_release(tabledata, sumindex + SUBTABLE_BASE);
							subtable_realloc(tabledata, subindex + SUBTABLE_BASE);
							tabledata->table[l1index] = subindex + SUBTABLE_BASE;
							merged++;
						}
				}
		}

	return merged;
}


/*-------------------------------------------------
    subtable_release - decrement the usecount on
    a subtable and free it if we're done
-------------------------------------------------*/

static void subtable_release(table_data *tabledata, UINT8 subentry)
{
	UINT8 subindex = subentry - SUBTABLE_BASE;

	/* sanity check */
	if (tabledata->subtable[subindex].usecount <= 0)
		fatalerror("Called subtable_release on a table with a usecount of 0");

	/* decrement the usecount and clear the checksum if we're at 0 */
	tabledata->subtable[subindex].usecount--;
	if (tabledata->subtable[subindex].usecount == 0)
		tabledata->subtable[subindex].checksum = 0;
}


/*-------------------------------------------------
    subtable_open - gain access to a subtable for
    modification
-------------------------------------------------*/

static UINT8 *subtable_open(table_data *tabledata, offs_t l1index)
{
	UINT8 subentry = tabledata->table[l1index];

	/* if we don't have a subtable yet, allocate a new one */
	if (subentry < SUBTABLE_BASE)
	{
		UINT8 newentry = subtable_alloc(tabledata);
		memset(SUBTABLE_PTR(tabledata, newentry), subentry, 1 << LEVEL2_BITS);
		tabledata->table[l1index] = newentry;
		tabledata->subtable[newentry - SUBTABLE_BASE].checksum = (subentry + (subentry << 8) + (subentry << 16) + (subentry << 24)) * ((1 << LEVEL2_BITS)/4);
		subentry = newentry;
	}

	/* if we're sharing this subtable, we also need to allocate a fresh copy */
	else if (tabledata->subtable[subentry - SUBTABLE_BASE].usecount > 1)
	{
		UINT8 newentry = subtable_alloc(tabledata);

		/* allocate may cause some additional merging -- look up the subentry again */
		/* when we're done; it should still require a split */
		subentry = tabledata->table[l1index];
		assert(subentry >= SUBTABLE_BASE);
		assert(tabledata->subtable[subentry - SUBTABLE_BASE].usecount > 1);

		memcpy(SUBTABLE_PTR(tabledata, newentry), SUBTABLE_PTR(tabledata, subentry), 1 << LEVEL2_BITS);
		subtable_release(tabledata, subentry);
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
    subtable_close - stop access to a subtable
-------------------------------------------------*/

static void subtable_close(table_data *tabledata, offs_t l1index)
{
	/* defer any merging until we run out of tables */
}


/*-------------------------------------------------
    Return whether a given memory map entry implies
    the need of allocating and registering memory
-------------------------------------------------*/

static int amentry_needs_backing_store(int cpunum, int spacenum, const address_map_entry *entry)
{
	FPTR handler;

	if (entry->baseptr != NULL || entry->baseptroffs_plus1 != 0)
		return TRUE;

	handler = (FPTR)entry->write.generic;
	if (handler < STATIC_COUNT)
	{
		if (handler != STATIC_INVALID &&
			handler != STATIC_ROM &&
			handler != STATIC_NOP &&
			handler != STATIC_UNMAP)
			return TRUE;
	}

	handler = (FPTR)entry->read.generic;
	if (handler < STATIC_COUNT)
	{
		if (handler != STATIC_INVALID &&
			(handler < STATIC_BANK1 || handler > STATIC_BANK1 + MAX_BANKS - 1) &&
			(handler != STATIC_ROM || spacenum != ADDRESS_SPACE_PROGRAM || entry->addrstart >= cpudata[cpunum].regionsize) &&
			handler != STATIC_NOP &&
			handler != STATIC_UNMAP)
			return TRUE;
	}

	return FALSE;
}


/*-------------------------------------------------
    memory_init_allocate - allocate memory for
    CPU address spaces
-------------------------------------------------*/

static void memory_init_allocate(running_machine *machine)
{
	int cpunum, spacenum;

	/* loop over all CPUs and memory spaces */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(cpudata); cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].spacemask & (1 << spacenum))
			{
				addrspace_data *space = &cpudata[cpunum].space[spacenum];
				address_map_entry *unassigned = NULL;
				address_map_entry *entry;
				memory_block *prev_memblock_head = memory_block_list;
				memory_block *memblock;

				/* make a first pass over the memory map and track blocks with hardcoded pointers */
				/* we do this to make sure they are found by memory_find_base first */
				for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
					if (entry->memory != NULL)
						allocate_memory_block(machine, cpunum, spacenum, entry->bytestart, entry->byteend, entry->memory);

				/* loop over all blocks just allocated and assign pointers from them */
				for (memblock = memory_block_list; memblock != prev_memblock_head; memblock = memblock->next)
					unassigned = assign_intersecting_blocks(space, memblock->bytestart, memblock->byteend, memblock->data);

				/* if we don't have an unassigned pointer yet, try to find one */
				if (unassigned == NULL)
					unassigned = assign_intersecting_blocks(space, ~0, 0, NULL);

				/* loop until we've assigned all memory in this space */
				while (unassigned != NULL)
				{
					offs_t curbytestart, curbyteend;
					int changed;
					void *block;

					/* work in MEMORY_BLOCK_CHUNK-sized chunks */
					offs_t curblockstart = unassigned->bytestart / MEMORY_BLOCK_CHUNK;
					offs_t curblockend = unassigned->byteend / MEMORY_BLOCK_CHUNK;

					/* loop while we keep finding unassigned blocks in neighboring MEMORY_BLOCK_CHUNK chunks */
					do
					{
						changed = FALSE;

						/* scan for unmapped blocks in the adjusted map */
						for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
							if (entry->memory == NULL && entry != unassigned && amentry_needs_backing_store(cpunum, spacenum, entry))
							{
								offs_t blockstart, blockend;

								/* get block start/end blocks for this block */
								blockstart = entry->bytestart / MEMORY_BLOCK_CHUNK;
								blockend = entry->byteend / MEMORY_BLOCK_CHUNK;

								/* if we intersect or are adjacent, adjust the start/end */
								if (blockstart <= curblockend + 1 && blockend >= curblockstart - 1)
								{
									if (blockstart < curblockstart)
										curblockstart = blockstart, changed = TRUE;
									if (blockend > curblockend)
										curblockend = blockend, changed = TRUE;
								}
							}
					} while (changed);

					/* we now have a block to allocate; do it */
					curbytestart = curblockstart * MEMORY_BLOCK_CHUNK;
					curbyteend = curblockend * MEMORY_BLOCK_CHUNK + (MEMORY_BLOCK_CHUNK - 1);
					block = allocate_memory_block(machine, cpunum, spacenum, curbytestart, curbyteend, NULL);

					/* assign memory that intersected the new block */
					unassigned = assign_intersecting_blocks(space, curbytestart, curbyteend, block);
				}
			}
}


/*-------------------------------------------------
    allocate_memory_block - allocate a single
    memory block of data
-------------------------------------------------*/

static void *allocate_memory_block(running_machine *machine, int cpunum, int spacenum, offs_t bytestart, offs_t byteend, void *memory)
{
	int allocatemem = (memory == NULL);
	memory_block *block;
	size_t bytestoalloc;
	const char *region;

	VPRINTF(("allocate_memory_block(%d,%d,%08X,%08X,%p)\n", cpunum, spacenum, bytestart, byteend, memory));

	/* determine how much memory to allocate for this */
	bytestoalloc = sizeof(*block);
	if (allocatemem)
		bytestoalloc += byteend - bytestart + 1;

	/* allocate and clear the memory */
	block = malloc_or_die(bytestoalloc);
	memset(block, 0, bytestoalloc);
	if (allocatemem)
		memory = block + 1;

	/* register for saving, but only if we're not part of a memory region */
	for (region = memory_region_next(machine, NULL); region != NULL; region = memory_region_next(machine, region))
	{
		UINT8 *region_base = memory_region(Machine, region);
		UINT32 region_length = memory_region_length(Machine, region);
		if (region_base != NULL && region_length != 0 && (UINT8 *)memory >= region_base && ((UINT8 *)memory + (byteend - bytestart + 1)) < region_base + region_length)
		{
			VPRINTF(("skipping save of this memory block as it is covered by a memory region\n"));
			break;
		}
	}
	if (region == NULL)
		register_for_save(cpunum, spacenum, bytestart, memory, byteend - bytestart + 1);

	/* fill in the tracking block */
	block->cpunum = cpunum;
	block->spacenum = spacenum;
	block->isallocated = allocatemem;
	block->bytestart = bytestart;
	block->byteend = byteend;
	block->data = memory;

	/* attach us to the head of the list */
	block->next = memory_block_list;
	memory_block_list = block;

	return memory;
}


/*-------------------------------------------------
    register_for_save - register a block of
    memory for save states
-------------------------------------------------*/

static void register_for_save(int cpunum, int spacenum, offs_t bytestart, void *base, size_t numbytes)
{
	int bytes_per_element = cpudata[cpunum].space[spacenum].dbits/8;
	char name[256];

	sprintf(name, "%d.%08x-%08x", spacenum, bytestart, (int)(bytestart + numbytes - 1));
	state_save_register_memory("memory", cpunum, name, base, bytes_per_element, (UINT32)numbytes / bytes_per_element);
}


/*-------------------------------------------------
    assign_intersecting_blocks - find all
    intersecting blocks and assign their pointers
-------------------------------------------------*/

static address_map_entry *assign_intersecting_blocks(addrspace_data *space, offs_t bytestart, offs_t byteend, UINT8 *base)
{
	address_map_entry *entry, *unassigned = NULL;

	/* loop over the adjusted map and assign memory to any blocks we can */
	for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
	{
		/* if we haven't assigned this block yet, do it against the last block */
		if (entry->memory == NULL)
		{
			/* inherit shared pointers first */
			if (entry->share != 0 && shared_ptr[entry->share] != NULL)
			{
				entry->memory = shared_ptr[entry->share];
 				VPRINTF(("memory range %08X-%08X -> shared_ptr[%d] [%p]\n", entry->addrstart, entry->addrend, entry->share, entry->memory));
 			}

			/* otherwise, look for a match in this block */
			else if (entry->bytestart >= bytestart && entry->byteend <= byteend)
			{
				entry->memory = base + (entry->bytestart - bytestart);
				VPRINTF(("memory range %08X-%08X -> found in block from %08X-%08X [%p]\n", entry->addrstart, entry->addrend, bytestart, byteend, entry->memory));
			}
		}

		/* if we're the first match on a shared pointer, assign it now */
		if (entry->memory != NULL && entry->share && shared_ptr[entry->share] == NULL)
			shared_ptr[entry->share] = entry->memory;

		/* keep track of the first unassigned entry */
		if (entry->memory == NULL && unassigned == NULL && amentry_needs_backing_store(space->cpunum, space->spacenum, entry))
			unassigned = entry;
	}

	return unassigned;
}


/*-------------------------------------------------
    reattach_banks - reconnect banks after a load
-------------------------------------------------*/

static STATE_POSTLOAD( reattach_banks )
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
    memory_init_locate - find all the requested pointers
    into the final allocated memory
-------------------------------------------------*/

static void memory_init_locate(running_machine *machine)
{
	int cpunum, spacenum, banknum;

	/* loop over CPUs and address spaces */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(cpudata); cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].spacemask & (1 << spacenum))
			{
				addrspace_data *space = &cpudata[cpunum].space[spacenum];
				const address_map_entry *entry;

				/* fill in base/size entries, and handle shared memory */
				for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
				{
					/* assign base/size values */
					if (entry->baseptr != NULL)
						*entry->baseptr = entry->memory;
					if (entry->baseptroffs_plus1 != 0)
						*(void **)((UINT8 *)machine->driver_data + entry->baseptroffs_plus1 - 1) = entry->memory;
					if (entry->sizeptr != NULL)
						*entry->sizeptr = entry->byteend - entry->bytestart + 1;
					if (entry->sizeptroffs_plus1 != 0)
						*(size_t *)((UINT8 *)machine->driver_data + entry->sizeptroffs_plus1 - 1) = entry->byteend - entry->bytestart + 1;
				}
			}

	/* once this is done, find the starting bases for the banks */
	for (banknum = 1; banknum <= MAX_BANKS; banknum++)
		if (bankdata[banknum].used)
		{
			address_map_entry *entry;

			/* set the initial bank pointer */
			for (entry = cpudata[bankdata[banknum].cpunum].space[bankdata[banknum].spacenum].map->entrylist; entry != NULL; entry = entry->next)
				if (entry->bytestart == bankdata[banknum].bytestart)
				{
					bank_ptr[banknum] = entry->memory;
	 				VPRINTF(("assigned bank %d pointer to memory from range %08X-%08X [%p]\n", banknum, entry->addrstart, entry->addrend, entry->memory));
					break;
				}

			/* if the entry was set ahead of time, override the automatically found pointer */
			if (!bankdata[banknum].dynamic && bankdata[banknum].curentry != MAX_BANK_ENTRIES)
				bank_ptr[banknum] = bankdata[banknum].entry[bankdata[banknum].curentry];
		}

	/* request a callback to fix up the banks when done */
	state_save_register_postload(machine, reattach_banks, NULL);
}


/*-------------------------------------------------
    memory_find_base - return a pointer to the
    base of RAM associated with the given CPU
    and offset
-------------------------------------------------*/

static void *memory_find_base(int cpunum, int spacenum, offs_t byteaddress)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	address_map_entry *entry;
	memory_block *block;

	VPRINTF(("memory_find_base(%d,%d,%08X) -> ", cpunum, spacenum, byteaddress));

	/* look in the address map first */
	for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
	{
		offs_t maskoffs = byteaddress & entry->bytemask;
		if (maskoffs >= entry->bytestart && maskoffs <= entry->byteend)
		{
			VPRINTF(("found in entry %08X-%08X [%p]\n", entry->addrstart, entry->addrend, (UINT8 *)entry->memory + (maskoffs - entry->bytestart)));
			return (UINT8 *)entry->memory + (maskoffs - entry->bytestart);
		}
	}

	/* if not found there, look in the allocated blocks */
	for (block = memory_block_list; block != NULL; block = block->next)
		if (block->cpunum == cpunum && block->spacenum == spacenum && block->bytestart <= byteaddress && block->byteend > byteaddress)
		{
			VPRINTF(("found in allocated memory block %08X-%08X [%p]\n", block->bytestart, block->byteend, block->data + (byteaddress - block->bytestart)));
			return block->data + byteaddress - block->bytestart;
		}

	VPRINTF(("did not find\n"));
	return NULL;
}



/*-------------------------------------------------
    safe opcode reading
-------------------------------------------------*/

UINT8 cpu_readop_safe(offs_t byteaddress)
{
	activecpu_set_opbase(byteaddress);
	return cpu_readop_unsafe(byteaddress);
}

UINT16 cpu_readop16_safe(offs_t byteaddress)
{
	activecpu_set_opbase(byteaddress);
	return cpu_readop16_unsafe(byteaddress);
}

UINT32 cpu_readop32_safe(offs_t byteaddress)
{
	activecpu_set_opbase(byteaddress);
	return cpu_readop32_unsafe(byteaddress);
}

UINT64 cpu_readop64_safe(offs_t byteaddress)
{
	activecpu_set_opbase(byteaddress);
	return cpu_readop64_unsafe(byteaddress);
}

UINT8 cpu_readop_arg_safe(offs_t byteaddress)
{
	activecpu_set_opbase(byteaddress);
	return cpu_readop_arg_unsafe(byteaddress);
}

UINT16 cpu_readop_arg16_safe(offs_t byteaddress)
{
	activecpu_set_opbase(byteaddress);
	return cpu_readop_arg16_unsafe(byteaddress);
}

UINT32 cpu_readop_arg32_safe(offs_t byteaddress)
{
	activecpu_set_opbase(byteaddress);
	return cpu_readop_arg32_unsafe(byteaddress);
}

UINT64 cpu_readop_arg64_safe(offs_t byteaddress)
{
	activecpu_set_opbase(byteaddress);
	return cpu_readop_arg64_unsafe(byteaddress);
}


/*-------------------------------------------------
    unmapped memory handlers
-------------------------------------------------*/

static READ8_HANDLER( mrh8_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory byte read from %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap;
}
static READ16_HANDLER( mrh16_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory word read from %08X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*2), mem_mask);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap;
}
static READ32_HANDLER( mrh32_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory dword read from %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*4), mem_mask);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap;
}
static READ64_HANDLER( mrh64_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory qword read from %08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*8), (int)(mem_mask >> 32), (int)(mem_mask & 0xffffffff));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM].unmap;
}

static WRITE8_HANDLER( mwh8_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory byte write to %08X = %02X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset), data);
}
static WRITE16_HANDLER( mwh16_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory word write to %08X = %04X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*2), data, mem_mask);
}
static WRITE32_HANDLER( mwh32_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory dword write to %08X = %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*4), data, mem_mask);
}
static WRITE64_HANDLER( mwh64_unmap_program )
{
	if (log_unmap[ADDRESS_SPACE_PROGRAM] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped program memory qword write to %08X = %08X%08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_PROGRAM], offset*8), (int)(data >> 32), (int)(data & 0xffffffff), (int)(mem_mask >> 32), (int)(mem_mask & 0xffffffff));
}

static READ8_HANDLER( mrh8_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory byte read from %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap;
}
static READ16_HANDLER( mrh16_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory word read from %08X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*2), mem_mask);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap;
}
static READ32_HANDLER( mrh32_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory dword read from %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*4), mem_mask);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap;
}
static READ64_HANDLER( mrh64_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory qword read from %08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*8), (int)(mem_mask >> 32), (int)(mem_mask & 0xffffffff));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA].unmap;
}

static WRITE8_HANDLER( mwh8_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory byte write to %08X = %02X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset), data);
}
static WRITE16_HANDLER( mwh16_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory word write to %08X = %04X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*2), data, mem_mask);
}
static WRITE32_HANDLER( mwh32_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory dword write to %08X = %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*4), data, mem_mask);
}
static WRITE64_HANDLER( mwh64_unmap_data )
{
	if (log_unmap[ADDRESS_SPACE_DATA] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped data memory qword write to %08X = %08X%08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_DATA], offset*8), (int)(data >> 32), (int)(data & 0xffffffff), (int)(mem_mask >> 32), (int)(mem_mask & 0xffffffff));
}

static READ8_HANDLER( mrh8_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O byte read from %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap;
}
static READ16_HANDLER( mrh16_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O word read from %08X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*2), mem_mask);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap;
}
static READ32_HANDLER( mrh32_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O dword read from %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*4), mem_mask);
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap;
}
static READ64_HANDLER( mrh64_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O qword read from %08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*8), (int)(mem_mask >> 32), (int)(mem_mask & 0xffffffff));
	return cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO].unmap;
}

static WRITE8_HANDLER( mwh8_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O byte write to %08X = %02X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset), data);
}
static WRITE16_HANDLER( mwh16_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O word write to %08X = %04X & %04X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*2), data, mem_mask);
}
static WRITE32_HANDLER( mwh32_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O dword write to %08X = %08X & %08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*4), data, mem_mask);
}
static WRITE64_HANDLER( mwh64_unmap_io )
{
	if (log_unmap[ADDRESS_SPACE_IO] && !debugger_access) logerror("cpu #%d (PC=%08X): unmapped I/O qword write to %08X = %08X%08X & %08X%08X\n", cpu_getactivecpu(), activecpu_get_pc(), BYTE2ADDR(&cpudata[cpu_getactivecpu()].space[ADDRESS_SPACE_IO], offset*8), (int)(data >> 32), (int)(data & 0xffffffff), (int)(mem_mask >> 32), (int)(mem_mask & 0xffffffff));
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
    watchpoint memory handlers
-------------------------------------------------*/

INLINE UINT8 watchpoint_read8(int spacenum, offs_t address)
{
	UINT8 result;
	debug_cpu_memory_read_hook(Machine, cur_context, spacenum, address, 0xff);
	active_address_space[spacenum].readlookup = cpudata[cur_context].space[spacenum].read.table;
	result = read_byte_generic(spacenum, address);
	active_address_space[spacenum].readlookup = wptable;
	return result;
}

INLINE UINT16 watchpoint_read16(int spacenum, offs_t address, UINT16 mem_mask)
{
	UINT16 result;
	debug_cpu_memory_read_hook(Machine, cur_context, spacenum, address << 1, mem_mask);
	active_address_space[spacenum].readlookup = cpudata[cur_context].space[spacenum].read.table;
	result = read_word_generic(spacenum, address << 1, mem_mask);
	active_address_space[spacenum].readlookup = wptable;
	return result;
}

INLINE UINT32 watchpoint_read32(int spacenum, offs_t address, UINT32 mem_mask)
{
	UINT32 result;
	debug_cpu_memory_read_hook(Machine, cur_context, spacenum, address << 2, mem_mask);
	active_address_space[spacenum].readlookup = cpudata[cur_context].space[spacenum].read.table;
	result = read_dword_generic(spacenum, address << 2, mem_mask);
	active_address_space[spacenum].readlookup = wptable;
	return result;
}

INLINE UINT64 watchpoint_read64(int spacenum, offs_t address, UINT64 mem_mask)
{
	UINT64 result;
	debug_cpu_memory_read_hook(Machine, cur_context, spacenum, address << 3, mem_mask);
	active_address_space[spacenum].readlookup = cpudata[cur_context].space[spacenum].read.table;
	result = read_qword_generic(spacenum, address << 3, mem_mask);
	active_address_space[spacenum].readlookup = wptable;
	return result;
}

INLINE void watchpoint_write8(int spacenum, offs_t address, UINT8 data)
{
	debug_cpu_memory_write_hook(Machine, cur_context, spacenum, address, data, 0xff);
	active_address_space[spacenum].writelookup = cpudata[cur_context].space[spacenum].write.table;
	write_byte_generic(spacenum, address, data);
	active_address_space[spacenum].writelookup = wptable;
}

INLINE void watchpoint_write16(int spacenum, offs_t address, UINT16 data, UINT16 mem_mask)
{
	debug_cpu_memory_write_hook(Machine, cur_context, spacenum, address << 1, data, mem_mask);
	active_address_space[spacenum].writelookup = cpudata[cur_context].space[spacenum].write.table;
	write_word_generic(spacenum, address << 1, data, mem_mask);
	active_address_space[spacenum].writelookup = wptable;
}

INLINE void watchpoint_write32(int spacenum, offs_t address, UINT32 data, UINT32 mem_mask)
{
	debug_cpu_memory_write_hook(Machine, cur_context, spacenum, address << 2, data, mem_mask);
	active_address_space[spacenum].writelookup = cpudata[cur_context].space[spacenum].write.table;
	write_dword_generic(spacenum, address << 2, data, mem_mask);
	active_address_space[spacenum].writelookup = wptable;
}

INLINE void watchpoint_write64(int spacenum, offs_t address, UINT64 data, UINT64 mem_mask)
{
	debug_cpu_memory_write_hook(Machine, cur_context, spacenum, address << 3, data, mem_mask);
	active_address_space[spacenum].writelookup = cpudata[cur_context].space[spacenum].write.table;
	write_qword_generic(spacenum, address << 3, data, mem_mask);
	active_address_space[spacenum].writelookup = wptable;
}

static READ8_HANDLER( mrh8_watchpoint_program )    { return watchpoint_read8 (ADDRESS_SPACE_PROGRAM, offset);           }
static READ16_HANDLER( mrh16_watchpoint_program )  { return watchpoint_read16(ADDRESS_SPACE_PROGRAM, offset, mem_mask); }
static READ32_HANDLER( mrh32_watchpoint_program )  { return watchpoint_read32(ADDRESS_SPACE_PROGRAM, offset, mem_mask); }
static READ64_HANDLER( mrh64_watchpoint_program )  { return watchpoint_read64(ADDRESS_SPACE_PROGRAM, offset, mem_mask); }

static WRITE8_HANDLER( mwh8_watchpoint_program )   { watchpoint_write8 (ADDRESS_SPACE_PROGRAM, offset, data);           }
static WRITE16_HANDLER( mwh16_watchpoint_program ) { watchpoint_write16(ADDRESS_SPACE_PROGRAM, offset, data, mem_mask); }
static WRITE32_HANDLER( mwh32_watchpoint_program ) { watchpoint_write32(ADDRESS_SPACE_PROGRAM, offset, data, mem_mask); }
static WRITE64_HANDLER( mwh64_watchpoint_program ) { watchpoint_write64(ADDRESS_SPACE_PROGRAM, offset, data, mem_mask); }

static READ8_HANDLER( mrh8_watchpoint_data )       { return watchpoint_read8 (ADDRESS_SPACE_DATA, offset);           }
static READ16_HANDLER( mrh16_watchpoint_data )     { return watchpoint_read16(ADDRESS_SPACE_DATA, offset, mem_mask); }
static READ32_HANDLER( mrh32_watchpoint_data )     { return watchpoint_read32(ADDRESS_SPACE_DATA, offset, mem_mask); }
static READ64_HANDLER( mrh64_watchpoint_data )     { return watchpoint_read64(ADDRESS_SPACE_DATA, offset, mem_mask); }

static WRITE8_HANDLER( mwh8_watchpoint_data )      { watchpoint_write8 (ADDRESS_SPACE_DATA, offset, data);           }
static WRITE16_HANDLER( mwh16_watchpoint_data )    { watchpoint_write16(ADDRESS_SPACE_DATA, offset, data, mem_mask); }
static WRITE32_HANDLER( mwh32_watchpoint_data )    { watchpoint_write32(ADDRESS_SPACE_DATA, offset, data, mem_mask); }
static WRITE64_HANDLER( mwh64_watchpoint_data )    { watchpoint_write64(ADDRESS_SPACE_DATA, offset, data, mem_mask); }

static READ8_HANDLER( mrh8_watchpoint_io )         { return watchpoint_read8 (ADDRESS_SPACE_IO, offset);           }
static READ16_HANDLER( mrh16_watchpoint_io )       { return watchpoint_read16(ADDRESS_SPACE_IO, offset, mem_mask); }
static READ32_HANDLER( mrh32_watchpoint_io )       { return watchpoint_read32(ADDRESS_SPACE_IO, offset, mem_mask); }
static READ64_HANDLER( mrh64_watchpoint_io )       { return watchpoint_read64(ADDRESS_SPACE_IO, offset, mem_mask); }

static WRITE8_HANDLER( mwh8_watchpoint_io )        { watchpoint_write8 (ADDRESS_SPACE_IO, offset, data);           }
static WRITE16_HANDLER( mwh16_watchpoint_io )      { watchpoint_write16(ADDRESS_SPACE_IO, offset, data, mem_mask); }
static WRITE32_HANDLER( mwh32_watchpoint_io )      { watchpoint_write32(ADDRESS_SPACE_IO, offset, data, mem_mask); }
static WRITE64_HANDLER( mwh64_watchpoint_io )      { watchpoint_write64(ADDRESS_SPACE_IO, offset, data, mem_mask); }


/*-------------------------------------------------
    get_static_handler - returns points to static
    memory handlers
-------------------------------------------------*/

static genf *get_static_handler(int handlerbits, int readorwrite, int spacenum, int which)
{
	static const struct
	{
		UINT8		handlerbits;
		UINT8		handlernum;
		UINT8		spacenum;
		genf *		read;
		genf *		write;
	} static_handler_list[] =
	{
		{  8, STATIC_UNMAP,      ADDRESS_SPACE_PROGRAM, (genf *)mrh8_unmap_program,       (genf *)mwh8_unmap_program },
		{  8, STATIC_UNMAP,      ADDRESS_SPACE_DATA,    (genf *)mrh8_unmap_data,          (genf *)mwh8_unmap_data },
		{  8, STATIC_UNMAP,      ADDRESS_SPACE_IO,      (genf *)mrh8_unmap_io,            (genf *)mwh8_unmap_io },
		{  8, STATIC_NOP,        ADDRESS_SPACE_PROGRAM, (genf *)mrh8_nop_program,         (genf *)mwh8_nop },
		{  8, STATIC_NOP,        ADDRESS_SPACE_DATA,    (genf *)mrh8_nop_data,            (genf *)mwh8_nop },
		{  8, STATIC_NOP,        ADDRESS_SPACE_IO,      (genf *)mrh8_nop_io,              (genf *)mwh8_nop },
		{  8, STATIC_WATCHPOINT, ADDRESS_SPACE_PROGRAM, (genf *)mrh8_watchpoint_program,  (genf *)mwh8_watchpoint_program },
		{  8, STATIC_WATCHPOINT, ADDRESS_SPACE_DATA,    (genf *)mrh8_watchpoint_data,     (genf *)mwh8_watchpoint_data },
		{  8, STATIC_WATCHPOINT, ADDRESS_SPACE_IO,      (genf *)mrh8_watchpoint_io,       (genf *)mwh8_watchpoint_io },

		{ 16, STATIC_UNMAP,      ADDRESS_SPACE_PROGRAM, (genf *)mrh16_unmap_program,      (genf *)mwh16_unmap_program },
		{ 16, STATIC_UNMAP,      ADDRESS_SPACE_DATA,    (genf *)mrh16_unmap_data,         (genf *)mwh16_unmap_data },
		{ 16, STATIC_UNMAP,      ADDRESS_SPACE_IO,      (genf *)mrh16_unmap_io,           (genf *)mwh16_unmap_io },
		{ 16, STATIC_NOP,        ADDRESS_SPACE_PROGRAM, (genf *)mrh16_nop_program,        (genf *)mwh16_nop },
		{ 16, STATIC_NOP,        ADDRESS_SPACE_DATA,    (genf *)mrh16_nop_data,           (genf *)mwh16_nop },
		{ 16, STATIC_NOP,        ADDRESS_SPACE_IO,      (genf *)mrh16_nop_io,             (genf *)mwh16_nop },
		{ 16, STATIC_WATCHPOINT, ADDRESS_SPACE_PROGRAM, (genf *)mrh16_watchpoint_program, (genf *)mwh16_watchpoint_program },
		{ 16, STATIC_WATCHPOINT, ADDRESS_SPACE_DATA,    (genf *)mrh16_watchpoint_data,    (genf *)mwh16_watchpoint_data },
		{ 16, STATIC_WATCHPOINT, ADDRESS_SPACE_IO,      (genf *)mrh16_watchpoint_io,      (genf *)mwh16_watchpoint_io },

		{ 32, STATIC_UNMAP,      ADDRESS_SPACE_PROGRAM, (genf *)mrh32_unmap_program,      (genf *)mwh32_unmap_program },
		{ 32, STATIC_UNMAP,      ADDRESS_SPACE_DATA,    (genf *)mrh32_unmap_data,         (genf *)mwh32_unmap_data },
		{ 32, STATIC_UNMAP,      ADDRESS_SPACE_IO,      (genf *)mrh32_unmap_io,           (genf *)mwh32_unmap_io },
		{ 32, STATIC_NOP,        ADDRESS_SPACE_PROGRAM, (genf *)mrh32_nop_program,        (genf *)mwh32_nop },
		{ 32, STATIC_NOP,        ADDRESS_SPACE_DATA,    (genf *)mrh32_nop_data,           (genf *)mwh32_nop },
		{ 32, STATIC_NOP,        ADDRESS_SPACE_IO,      (genf *)mrh32_nop_io,             (genf *)mwh32_nop },
		{ 32, STATIC_WATCHPOINT, ADDRESS_SPACE_PROGRAM, (genf *)mrh32_watchpoint_program, (genf *)mwh32_watchpoint_program },
		{ 32, STATIC_WATCHPOINT, ADDRESS_SPACE_DATA,    (genf *)mrh32_watchpoint_data,    (genf *)mwh32_watchpoint_data },
		{ 32, STATIC_WATCHPOINT, ADDRESS_SPACE_IO,      (genf *)mrh32_watchpoint_io,      (genf *)mwh32_watchpoint_io },

		{ 64, STATIC_UNMAP,      ADDRESS_SPACE_PROGRAM, (genf *)mrh64_unmap_program,      (genf *)mwh64_unmap_program },
		{ 64, STATIC_UNMAP,      ADDRESS_SPACE_DATA,    (genf *)mrh64_unmap_data,         (genf *)mwh64_unmap_data },
		{ 64, STATIC_UNMAP,      ADDRESS_SPACE_IO,      (genf *)mrh64_unmap_io,           (genf *)mwh64_unmap_io },
		{ 64, STATIC_NOP,        ADDRESS_SPACE_PROGRAM, (genf *)mrh64_nop_program,        (genf *)mwh64_nop },
		{ 64, STATIC_NOP,        ADDRESS_SPACE_DATA,    (genf *)mrh64_nop_data,           (genf *)mwh64_nop },
		{ 64, STATIC_NOP,        ADDRESS_SPACE_IO,      (genf *)mrh64_nop_io,             (genf *)mwh64_nop },
		{ 64, STATIC_WATCHPOINT, ADDRESS_SPACE_PROGRAM, (genf *)mrh64_watchpoint_program, (genf *)mwh64_watchpoint_program },
		{ 64, STATIC_WATCHPOINT, ADDRESS_SPACE_DATA,    (genf *)mrh64_watchpoint_data,    (genf *)mwh64_watchpoint_data },
		{ 64, STATIC_WATCHPOINT, ADDRESS_SPACE_IO,      (genf *)mrh64_watchpoint_io,      (genf *)mwh64_watchpoint_io }
	};
	int tablenum;

	for (tablenum = 0; tablenum < sizeof(static_handler_list) / sizeof(static_handler_list[0]); tablenum++)
		if (static_handler_list[tablenum].handlerbits == handlerbits && static_handler_list[tablenum].handlernum == which)
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
	fprintf(file, "  Address mask = %X\n", space->bytemask);
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
							table->handlers[lastentry].bytestart);

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
									table->handlers[lastentry2].bytestart);

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
								table->handlers[lastentry2].bytestart);
		}
	}

	/* flush the last entry */
	if (lastentry < SUBTABLE_BASE && lastentry != STATIC_UNMAP)
		fprintf(file, "%08X-%08X    = %02X: %s [offset=%08X]\n",
						(i - entrymatches) << LEVEL2_BITS,
						(i << LEVEL2_BITS) - 1,
						lastentry,
						handler_to_string(table, lastentry),
						table->handlers[lastentry].bytestart);
}

void memory_dump(FILE *file)
{
	int cpunum, spacenum;

	/* skip if we can't open the file */
	if (!file)
		return;

	/* loop over CPUs */
	for (cpunum = 0; cpunum < ARRAY_LENGTH(cpudata); cpunum++)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (cpudata[cpunum].spacemask & (1 << spacenum))
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

const char *memory_get_handler_string(int read0_or_write1, int cpunum, int spacenum, offs_t byteaddress)
{
	addrspace_data *space = &cpudata[cpunum].space[spacenum];
	const table_data *table = read0_or_write1 ? &space->write : &space->read;
	UINT8 entry;

	/* perform the lookup */
	byteaddress &= space->bytemask;
	entry = table->table[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = table->table[LEVEL2_INDEX(entry, byteaddress)];

	/* 8-bit case: RAM/ROM */
	return handler_to_string(table, entry);
}


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



/***************************************************************************
    STUB HANDLERS THAT MAP TO BYTE READS
***************************************************************************/

/*-------------------------------------------------
    stub_read8_from_16 - return a 16-bit
    value combined from one or more byte accesses
-------------------------------------------------*/

static READ16_HANDLER( stub_read8_from_16 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT16 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			result |= (*handler->subhandler.read.mhandler8)(handler->subobject, offset) << shift;
		offset++;
	}
	return result;
}


/*-------------------------------------------------
    stub_read8_from_32 - return a 32-bit
    value combined from one or more byte accesses
-------------------------------------------------*/

static READ32_HANDLER( stub_read8_from_32 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT32 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			result |= (*handler->subhandler.read.mhandler8)(handler->subobject, offset) << shift;
		offset++;
	}
	return result;
}


/*-------------------------------------------------
    stub_read8_from_64 - return a 64-bit
    value combined from one or more byte accesses
-------------------------------------------------*/

static READ64_HANDLER( stub_read8_from_64 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT64 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			result |= (UINT64)(*handler->subhandler.read.mhandler8)(handler->subobject, offset) << shift;
		offset++;
	}
	return result;
}


/*-------------------------------------------------
    stub_read16_from_32 - return a 32-bit
    value combined from one or more word accesses
-------------------------------------------------*/

static READ32_HANDLER( stub_read16_from_32 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT32 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT16)(mem_mask >> shift) != 0)
			result |= (*handler->subhandler.read.mhandler16)(handler->subobject, offset, mem_mask >> shift) << shift;
		offset++;
	}
	return result;
}


/*-------------------------------------------------
    stub_read16_from_64 - return a 64-bit
    value combined from one or more word accesses
-------------------------------------------------*/

static READ64_HANDLER( stub_read16_from_64 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT64 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT16)(mem_mask >> shift) != 0)
			result |= (UINT64)(*handler->subhandler.read.mhandler16)(handler->subobject, offset, mem_mask >> shift) << shift;
		offset++;
	}
	return result;
}


/*-------------------------------------------------
    stub_read32_from_64 - return a 64-bit
    value combined from one or more dword accesses
-------------------------------------------------*/

static READ64_HANDLER( stub_read32_from_64 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT64 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT32)(mem_mask >> shift) != 0)
			result |= (UINT64)(*handler->subhandler.read.mhandler32)(handler->subobject, offset, mem_mask >> shift) << shift;
		offset++;
	}
	return result;
}



/***************************************************************************
    STUB HANDLERS THAT MAP TO BYTE WRITES
***************************************************************************/

/*-------------------------------------------------
    stub_write8_from_16 - convert a 16-bit
    write to one or more byte accesses
-------------------------------------------------*/

static WRITE16_HANDLER( stub_write8_from_16 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.mhandler8)(handler->subobject, offset, data >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write8_from_32 - convert a 32-bit
    write to one or more byte accesses
-------------------------------------------------*/

static WRITE32_HANDLER( stub_write8_from_32 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.mhandler8)(handler->subobject, offset, data >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write8_from_64 - convert a 64-bit
    write to one or more byte accesses
-------------------------------------------------*/

static WRITE64_HANDLER( stub_write8_from_64 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.mhandler8)(handler->subobject, offset, data >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write16_from_32 - convert a 32-bit
    write to one or more word accesses
-------------------------------------------------*/

static WRITE32_HANDLER( stub_write16_from_32 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT16)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.mhandler16)(handler->subobject, offset, data >> shift, mem_mask >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write16_from_64 - convert a 64-bit
    write to one or more word accesses
-------------------------------------------------*/

static WRITE64_HANDLER( stub_write16_from_64 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT16)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.mhandler16)(handler->subobject, offset, data >> shift, mem_mask >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write32_from_64 - convert a 64-bit
    write to one or more word accesses
-------------------------------------------------*/

static WRITE64_HANDLER( stub_write32_from_64 )
{
	const handler_data *handler = (const handler_data *)machine;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT32)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.mhandler32)(handler->subobject, offset, data >> shift, mem_mask >> shift);
		offset++;
	}
}



/***************************************************************************
    STUB ACCESSORS
***************************************************************************/

/*-------------------------------------------------
    get_stub_handler - return the appropriate
    stub handler
-------------------------------------------------*/

static memory_handler get_stub_handler(read_or_write readorwrite, int spacedbits, int handlerdbits)
{
	memory_handler result = { 0 };

	/* read stubs */
	if (readorwrite == ROW_READ)
	{
		/* 16-bit read stubs */
		if (spacedbits == 16)
		{
			if (handlerdbits == 8)
				result.read.mhandler16 = stub_read8_from_16;
		}

		/* 32-bit read stubs */
		else if (spacedbits == 32)
		{
			if (handlerdbits == 8)
				result.read.mhandler32 = stub_read8_from_32;
			else if (handlerdbits == 16)
				result.read.mhandler32 = stub_read16_from_32;
		}

		/* 64-bit read stubs */
		else if (spacedbits == 64)
		{
			if (handlerdbits == 8)
				result.read.mhandler64 = stub_read8_from_64;
			else if (handlerdbits == 16)
				result.read.mhandler64 = stub_read16_from_64;
			else if (handlerdbits == 32)
				result.read.mhandler64 = stub_read32_from_64;
		}
	}

	/* write stubs */
	else if (readorwrite == ROW_WRITE)
	{
		/* 16-bit write stubs */
		if (spacedbits == 16)
		{
			if (handlerdbits == 8)
				result.write.mhandler16 = stub_write8_from_16;
		}

		/* 32-bit write stubs */
		else if (spacedbits == 32)
		{
			if (handlerdbits == 8)
				result.write.mhandler32 = stub_write8_from_32;
			else if (handlerdbits == 16)
				result.write.mhandler32 = stub_write16_from_32;
		}

		/* 64-bit write stubs */
		else if (spacedbits == 64)
		{
			if (handlerdbits == 8)
				result.write.mhandler64 = stub_write8_from_64;
			else if (handlerdbits == 16)
				result.write.mhandler64 = stub_write16_from_64;
			else if (handlerdbits == 32)
				result.write.mhandler64 = stub_write32_from_64;
		}
	}

	assert(result.read.generic != NULL);
	return result;
}



/***************************************************************************
    8-BIT READ HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_SPACE_PROGRAM
-------------------------------------------------*/

UINT8 program_read_byte_8le(offs_t address)
{
	return read_byte_generic(ADDRESS_SPACE_PROGRAM, address);
}

UINT8 program_read_byte_8be(offs_t address)
{
	return read_byte_generic(ADDRESS_SPACE_PROGRAM, address);
}

UINT16 program_read_word_8le(offs_t address)
{
	UINT16 result = program_read_byte_8le(address + 0) << 0;
	return result | (program_read_byte_8le(address + 1) << 8);
}

UINT16 program_read_word_masked_8le(offs_t address, UINT16 mask)
{
	UINT16 result = 0;
	if (mask & 0x00ff) result |= program_read_byte_8le(address + 0) << 0;
	if (mask & 0xff00) result |= program_read_byte_8le(address + 1) << 8;
	return result;
}

UINT16 program_read_word_8be(offs_t address)
{
	UINT16 result = program_read_byte_8be(address + 0) << 8;
	return result | (program_read_byte_8be(address + 1) << 0);
}

UINT16 program_read_word_masked_8be(offs_t address, UINT16 mask)
{
	UINT16 result = 0;
	if (mask & 0xff00) result |= program_read_byte_8be(address + 0) << 8;
	if (mask & 0x00ff) result |= program_read_byte_8be(address + 1) << 0;
	return result;
}

UINT32 program_read_dword_8le(offs_t address)
{
	UINT32 result = program_read_word_8le(address + 0) << 0;
	return result | (program_read_word_8le(address + 2) << 16);
}

UINT32 program_read_dword_masked_8le(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0x0000ffff) result |= program_read_word_masked_8le(address + 0, mask >> 0) << 0;
	if (mask & 0xffff0000) result |= program_read_word_masked_8le(address + 2, mask >> 16) << 16;
	return result;
}

UINT32 program_read_dword_8be(offs_t address)
{
	UINT32 result = program_read_word_8be(address + 0) << 16;
	return result | (program_read_word_8be(address + 2) << 0);
}

UINT32 program_read_dword_masked_8be(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0xffff0000) result |= program_read_word_masked_8be(address + 0, mask >> 16) << 16;
	if (mask & 0x0000ffff) result |= program_read_word_masked_8be(address + 2, mask >> 0) << 0;
	return result;
}

UINT64 program_read_qword_8le(offs_t address)
{
	UINT64 result = (UINT64)program_read_dword_8le(address + 0) << 0;
	return result | ((UINT64)program_read_dword_8le(address + 4) << 32);
}

UINT64 program_read_qword_masked_8le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)program_read_dword_masked_8le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)program_read_dword_masked_8le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 program_read_qword_8be(offs_t address)
{
	UINT64 result = (UINT64)program_read_dword_8le(address + 0) << 32;
	return result | ((UINT64)program_read_dword_8le(address + 4) << 0);
}

UINT64 program_read_qword_masked_8be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)program_read_dword_masked_8be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)program_read_dword_masked_8be(address + 4, mask >> 0) << 0;
	return result;
}


/*-------------------------------------------------
    ADDRESS_SPACE_DATA
-------------------------------------------------*/

UINT8 data_read_byte_8le(offs_t address)
{
	return read_byte_generic(ADDRESS_SPACE_DATA, address);
}

UINT8 data_read_byte_8be(offs_t address)
{
	return read_byte_generic(ADDRESS_SPACE_DATA, address);
}

UINT16 data_read_word_8le(offs_t address)
{
	UINT16 result = data_read_byte_8le(address + 0) << 0;
	return result | (data_read_byte_8le(address + 1) << 8);
}

UINT16 data_read_word_masked_8le(offs_t address, UINT16 mask)
{
	UINT16 result = 0;
	if (mask & 0x00ff) result |= data_read_byte_8le(address + 0) << 0;
	if (mask & 0xff00) result |= data_read_byte_8le(address + 1) << 8;
	return result;
}

UINT16 data_read_word_8be(offs_t address)
{
	UINT16 result = data_read_byte_8be(address + 0) << 8;
	return result | (data_read_byte_8be(address + 1) << 0);
}

UINT16 data_read_word_masked_8be(offs_t address, UINT16 mask)
{
	UINT16 result = 0;
	if (mask & 0xff00) result |= data_read_byte_8be(address + 0) << 8;
	if (mask & 0x00ff) result |= data_read_byte_8be(address + 1) << 0;
	return result;
}

UINT32 data_read_dword_8le(offs_t address)
{
	UINT32 result = data_read_word_8le(address + 0) << 0;
	return result | (data_read_word_8le(address + 2) << 16);
}

UINT32 data_read_dword_masked_8le(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0x0000ffff) result |= data_read_word_masked_8le(address + 0, mask >> 0) << 0;
	if (mask & 0xffff0000) result |= data_read_word_masked_8le(address + 2, mask >> 16) << 16;
	return result;
}

UINT32 data_read_dword_8be(offs_t address)
{
	UINT32 result = data_read_word_8be(address + 0) << 16;
	return result | (data_read_word_8be(address + 2) << 0);
}

UINT32 data_read_dword_masked_8be(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0xffff0000) result |= data_read_word_masked_8be(address + 0, mask >> 16) << 16;
	if (mask & 0x0000ffff) result |= data_read_word_masked_8be(address + 2, mask >> 0) << 0;
	return result;
}

UINT64 data_read_qword_8le(offs_t address)
{
	UINT64 result = (UINT64)data_read_dword_8le(address + 0) << 0;
	return result | ((UINT64)data_read_dword_8le(address + 4) << 32);
}

UINT64 data_read_qword_masked_8le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)data_read_dword_masked_8le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)data_read_dword_masked_8le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 data_read_qword_8be(offs_t address)
{
	UINT64 result = (UINT64)data_read_dword_8le(address + 0) << 32;
	return result | ((UINT64)data_read_dword_8le(address + 4) << 0);
}

UINT64 data_read_qword_masked_8be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)data_read_dword_masked_8be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)data_read_dword_masked_8be(address + 4, mask >> 0) << 0;
	return result;
}


/*-------------------------------------------------
    ADDRESS_SPACE_IO
-------------------------------------------------*/

UINT8 io_read_byte_8le(offs_t address)
{
	return read_byte_generic(ADDRESS_SPACE_IO, address);
}

UINT8 io_read_byte_8be(offs_t address)
{
	return read_byte_generic(ADDRESS_SPACE_IO, address);
}

UINT16 io_read_word_8le(offs_t address)
{
	UINT16 result = io_read_byte_8le(address + 0) << 0;
	return result | (io_read_byte_8le(address + 1) << 8);
}

UINT16 io_read_word_masked_8le(offs_t address, UINT16 mask)
{
	UINT16 result = 0;
	if (mask & 0x00ff) result |= io_read_byte_8le(address + 0) << 0;
	if (mask & 0xff00) result |= io_read_byte_8le(address + 1) << 8;
	return result;
}

UINT16 io_read_word_8be(offs_t address)
{
	UINT16 result = io_read_byte_8be(address + 0) << 8;
	return result | (io_read_byte_8be(address + 1) << 0);
}

UINT16 io_read_word_masked_8be(offs_t address, UINT16 mask)
{
	UINT16 result = 0;
	if (mask & 0xff00) result |= io_read_byte_8be(address + 0) << 8;
	if (mask & 0x00ff) result |= io_read_byte_8be(address + 1) << 0;
	return result;
}

UINT32 io_read_dword_8le(offs_t address)
{
	UINT32 result = io_read_word_8le(address + 0) << 0;
	return result | (io_read_word_8le(address + 2) << 16);
}

UINT32 io_read_dword_masked_8le(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0x0000ffff) result |= io_read_word_masked_8le(address + 0, mask >> 0) << 0;
	if (mask & 0xffff0000) result |= io_read_word_masked_8le(address + 2, mask >> 16) << 16;
	return result;
}

UINT32 io_read_dword_8be(offs_t address)
{
	UINT32 result = io_read_word_8be(address + 0) << 16;
	return result | (io_read_word_8be(address + 2) << 0);
}

UINT32 io_read_dword_masked_8be(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0xffff0000) result |= io_read_word_masked_8be(address + 0, mask >> 16) << 16;
	if (mask & 0x0000ffff) result |= io_read_word_masked_8be(address + 2, mask >> 0) << 0;
	return result;
}

UINT64 io_read_qword_8le(offs_t address)
{
	UINT64 result = (UINT64)io_read_dword_8le(address + 0) << 0;
	return result | ((UINT64)io_read_dword_8le(address + 4) << 32);
}

UINT64 io_read_qword_masked_8le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)io_read_dword_masked_8le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)io_read_dword_masked_8le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 io_read_qword_8be(offs_t address)
{
	UINT64 result = (UINT64)io_read_dword_8le(address + 0) << 32;
	return result | ((UINT64)io_read_dword_8le(address + 4) << 0);
}

UINT64 io_read_qword_masked_8be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)io_read_dword_masked_8be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)io_read_dword_masked_8be(address + 4, mask >> 0) << 0;
	return result;
}



/***************************************************************************
    8-BIT WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_SPACE_PROGRAM
-------------------------------------------------*/

void program_write_byte_8le(offs_t address, UINT8 data)
{
	write_byte_generic(ADDRESS_SPACE_PROGRAM, address, data);
}

void program_write_byte_8be(offs_t address, UINT8 data)
{
	write_byte_generic(ADDRESS_SPACE_PROGRAM, address, data);
}

void program_write_word_8le(offs_t address, UINT16 data)
{
	program_write_byte_8le(address + 0, data >> 0);
	program_write_byte_8le(address + 1, data >> 8);
}

void program_write_word_masked_8le(offs_t address, UINT16 data, UINT16 mask)
{
	if (mask & 0x00ff) program_write_byte_8le(address + 0, data >> 0);
	if (mask & 0xff00) program_write_byte_8le(address + 1, data >> 8);
}

void program_write_word_8be(offs_t address, UINT16 data)
{
	program_write_byte_8be(address + 0, data >> 8);
	program_write_byte_8be(address + 1, data >> 0);
}

void program_write_word_masked_8be(offs_t address, UINT16 data, UINT16 mask)
{
	if (mask & 0xff00) program_write_byte_8be(address + 0, data >> 8);
	if (mask & 0x00ff) program_write_byte_8be(address + 1, data >> 0);
}

void program_write_dword_8le(offs_t address, UINT32 data)
{
	program_write_word_8le(address + 0, data >> 0);
	program_write_word_8le(address + 2, data >> 16);
}

void program_write_dword_masked_8le(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0x0000ffff) program_write_word_masked_8le(address + 0, data >> 0, mask >> 0);
	if (mask & 0xffff0000) program_write_word_masked_8le(address + 2, data >> 16, mask >> 16);
}

void program_write_dword_8be(offs_t address, UINT32 data)
{
	program_write_word_8be(address + 0, data >> 16);
	program_write_word_8be(address + 2, data >> 0);
}

void program_write_dword_masked_8be(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0xffff0000) program_write_word_masked_8be(address + 0, data >> 16, mask >> 16);
	if (mask & 0x0000ffff) program_write_word_masked_8be(address + 2, data >> 0, mask >> 0);
}

void program_write_qword_8le(offs_t address, UINT64 data)
{
	program_write_dword_8le(address + 0, data >> 0);
	program_write_dword_8le(address + 4, data >> 32);
}

void program_write_qword_masked_8le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) program_write_dword_masked_8le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) program_write_dword_masked_8le(address + 4, data >> 32, mask >> 32);
}

void program_write_qword_8be(offs_t address, UINT64 data)
{
	program_write_dword_8be(address + 0, data >> 32);
	program_write_dword_8be(address + 4, data >> 0);
}

void program_write_qword_masked_8be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) program_write_dword_masked_8be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) program_write_dword_masked_8be(address + 4, data >> 0, mask >> 0);
}


/*-------------------------------------------------
    ADDRESS_SPACE_DATA
-------------------------------------------------*/

void data_write_byte_8le(offs_t address, UINT8 data)
{
	write_byte_generic(ADDRESS_SPACE_DATA, address, data);
}

void data_write_byte_8be(offs_t address, UINT8 data)
{
	write_byte_generic(ADDRESS_SPACE_DATA, address, data);
}

void data_write_word_8le(offs_t address, UINT16 data)
{
	data_write_byte_8le(address + 0, data >> 0);
	data_write_byte_8le(address + 1, data >> 8);
}

void data_write_word_masked_8le(offs_t address, UINT16 data, UINT16 mask)
{
	if (mask & 0x00ff) data_write_byte_8le(address + 0, data >> 0);
	if (mask & 0xff00) data_write_byte_8le(address + 1, data >> 8);
}

void data_write_word_8be(offs_t address, UINT16 data)
{
	data_write_byte_8be(address + 0, data >> 8);
	data_write_byte_8be(address + 1, data >> 0);
}

void data_write_word_masked_8be(offs_t address, UINT16 data, UINT16 mask)
{
	if (mask & 0xff00) data_write_byte_8be(address + 0, data >> 8);
	if (mask & 0x00ff) data_write_byte_8be(address + 1, data >> 0);
}

void data_write_dword_8le(offs_t address, UINT32 data)
{
	data_write_word_8le(address + 0, data >> 0);
	data_write_word_8le(address + 2, data >> 16);
}

void data_write_dword_masked_8le(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0x0000ffff) data_write_word_masked_8le(address + 0, data >> 0, mask >> 0);
	if (mask & 0xffff0000) data_write_word_masked_8le(address + 2, data >> 16, mask >> 16);
}

void data_write_dword_8be(offs_t address, UINT32 data)
{
	data_write_word_8be(address + 0, data >> 16);
	data_write_word_8be(address + 2, data >> 0);
}

void data_write_dword_masked_8be(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0xffff0000) data_write_word_masked_8be(address + 0, data >> 16, mask >> 16);
	if (mask & 0x0000ffff) data_write_word_masked_8be(address + 2, data >> 0, mask >> 0);
}

void data_write_qword_8le(offs_t address, UINT64 data)
{
	data_write_dword_8le(address + 0, data >> 0);
	data_write_dword_8le(address + 4, data >> 32);
}

void data_write_qword_masked_8le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) data_write_dword_masked_8le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) data_write_dword_masked_8le(address + 4, data >> 32, mask >> 32);
}

void data_write_qword_8be(offs_t address, UINT64 data)
{
	data_write_dword_8be(address + 0, data >> 32);
	data_write_dword_8be(address + 4, data >> 0);
}

void data_write_qword_masked_8be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) data_write_dword_masked_8be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) data_write_dword_masked_8be(address + 4, data >> 0, mask >> 0);
}


/*-------------------------------------------------
    ADDRESS_SPACE_IO
-------------------------------------------------*/

void io_write_byte_8le(offs_t address, UINT8 data)
{
	write_byte_generic(ADDRESS_SPACE_IO, address, data);
}

void io_write_byte_8be(offs_t address, UINT8 data)
{
	write_byte_generic(ADDRESS_SPACE_IO, address, data);
}

void io_write_word_8le(offs_t address, UINT16 data)
{
	io_write_byte_8le(address + 0, data >> 0);
	io_write_byte_8le(address + 1, data >> 8);
}

void io_write_word_masked_8le(offs_t address, UINT16 data, UINT16 mask)
{
	if (mask & 0x00ff) io_write_byte_8le(address + 0, data >> 0);
	if (mask & 0xff00) io_write_byte_8le(address + 1, data >> 8);
}

void io_write_word_8be(offs_t address, UINT16 data)
{
	io_write_byte_8be(address + 0, data >> 8);
	io_write_byte_8be(address + 1, data >> 0);
}

void io_write_word_masked_8be(offs_t address, UINT16 data, UINT16 mask)
{
	if (mask & 0xff00) io_write_byte_8be(address + 0, data >> 8);
	if (mask & 0x00ff) io_write_byte_8be(address + 1, data >> 0);
}

void io_write_dword_8le(offs_t address, UINT32 data)
{
	io_write_word_8le(address + 0, data >> 0);
	io_write_word_8le(address + 2, data >> 16);
}

void io_write_dword_masked_8le(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0x0000ffff) io_write_word_masked_8le(address + 0, data >> 0, mask >> 0);
	if (mask & 0xffff0000) io_write_word_masked_8le(address + 2, data >> 16, mask >> 16);
}

void io_write_dword_8be(offs_t address, UINT32 data)
{
	io_write_word_8be(address + 0, data >> 16);
	io_write_word_8be(address + 2, data >> 0);
}

void io_write_dword_masked_8be(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0xffff0000) io_write_word_masked_8be(address + 0, data >> 16, mask >> 16);
	if (mask & 0x0000ffff) io_write_word_masked_8be(address + 2, data >> 0, mask >> 0);
}

void io_write_qword_8le(offs_t address, UINT64 data)
{
	io_write_dword_8le(address + 0, data >> 0);
	io_write_dword_8le(address + 4, data >> 32);
}

void io_write_qword_masked_8le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) io_write_dword_masked_8le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) io_write_dword_masked_8le(address + 4, data >> 32, mask >> 32);
}

void io_write_qword_8be(offs_t address, UINT64 data)
{
	io_write_dword_8be(address + 0, data >> 32);
	io_write_dword_8be(address + 4, data >> 0);
}

void io_write_qword_masked_8be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) io_write_dword_masked_8be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) io_write_dword_masked_8be(address + 4, data >> 0, mask >> 0);
}



/***************************************************************************
    16-BIT READ HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_SPACE_PROGRAM
-------------------------------------------------*/

UINT8 program_read_byte_16le(offs_t address)
{
	UINT32 shift = (address & 1) * 8;
	return read_word_generic(ADDRESS_SPACE_PROGRAM, address, 0xff << shift) >> shift;
}

UINT8 program_read_byte_16be(offs_t address)
{
	UINT32 shift = (~address & 1) * 8;
	return read_word_generic(ADDRESS_SPACE_PROGRAM, address, 0xff << shift) >> shift;
}

UINT16 program_read_word_16le(offs_t address)
{
	return read_word_generic(ADDRESS_SPACE_PROGRAM, address, 0xffff);
}

UINT16 program_read_word_masked_16le(offs_t address, UINT16 mask)
{
	return read_word_generic(ADDRESS_SPACE_PROGRAM, address, mask);
}

UINT16 program_read_word_16be(offs_t address)
{
	return read_word_generic(ADDRESS_SPACE_PROGRAM, address, 0xffff);
}

UINT16 program_read_word_masked_16be(offs_t address, UINT16 mask)
{
	return read_word_generic(ADDRESS_SPACE_PROGRAM, address, mask);
}

UINT32 program_read_dword_16le(offs_t address)
{
	UINT32 result = program_read_word_16le(address + 0) << 0;
	return result | (program_read_word_16le(address + 2) << 16);
}

UINT32 program_read_dword_masked_16le(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0x0000ffff) result |= program_read_word_masked_16le(address + 0, mask >> 0) << 0;
	if (mask & 0xffff0000) result |= program_read_word_masked_16le(address + 2, mask >> 16) << 16;
	return result;
}

UINT32 program_read_dword_16be(offs_t address)
{
	UINT32 result = program_read_word_16be(address + 0) << 16;
	return result | (program_read_word_16be(address + 2) << 0);
}

UINT32 program_read_dword_masked_16be(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0xffff0000) result |= program_read_word_masked_16be(address + 0, mask >> 16) << 16;
	if (mask & 0x0000ffff) result |= program_read_word_masked_16be(address + 2, mask >> 0) << 0;
	return result;
}

UINT64 program_read_qword_16le(offs_t address)
{
	UINT64 result = (UINT64)program_read_dword_16le(address + 0) << 0;
	return result | ((UINT64)program_read_dword_16le(address + 4) << 32);
}

UINT64 program_read_qword_masked_16le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)program_read_dword_masked_16le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)program_read_dword_masked_16le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 program_read_qword_16be(offs_t address)
{
	UINT64 result = (UINT64)program_read_dword_16le(address + 0) << 32;
	return result | ((UINT64)program_read_dword_16le(address + 4) << 0);
}

UINT64 program_read_qword_masked_16be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)program_read_dword_masked_16be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)program_read_dword_masked_16be(address + 4, mask >> 0) << 0;
	return result;
}


/*-------------------------------------------------
    ADDRESS_SPACE_DATA
-------------------------------------------------*/

UINT8 data_read_byte_16le(offs_t address)
{
	UINT32 shift = (address & 1) * 8;
	return read_word_generic(ADDRESS_SPACE_DATA, address, 0xff << shift) >> shift;
}

UINT8 data_read_byte_16be(offs_t address)
{
	UINT32 shift = (~address & 1) * 8;
	return read_word_generic(ADDRESS_SPACE_DATA, address, 0xff << shift) >> shift;
}

UINT16 data_read_word_16le(offs_t address)
{
	return read_word_generic(ADDRESS_SPACE_DATA, address, 0xffff);
}

UINT16 data_read_word_masked_16le(offs_t address, UINT16 mask)
{
	return read_word_generic(ADDRESS_SPACE_DATA, address, mask);
}

UINT16 data_read_word_16be(offs_t address)
{
	return read_word_generic(ADDRESS_SPACE_DATA, address, 0xffff);
}

UINT16 data_read_word_masked_16be(offs_t address, UINT16 mask)
{
	return read_word_generic(ADDRESS_SPACE_DATA, address, mask);
}

UINT32 data_read_dword_16le(offs_t address)
{
	UINT32 result = data_read_word_16le(address + 0) << 0;
	return result | (data_read_word_16le(address + 2) << 16);
}

UINT32 data_read_dword_masked_16le(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0x0000ffff) result |= data_read_word_masked_16le(address + 0, mask >> 0) << 0;
	if (mask & 0xffff0000) result |= data_read_word_masked_16le(address + 2, mask >> 16) << 16;
	return result;
}

UINT32 data_read_dword_16be(offs_t address)
{
	UINT32 result = data_read_word_16be(address + 0) << 16;
	return result | (data_read_word_16be(address + 2) << 0);
}

UINT32 data_read_dword_masked_16be(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0xffff0000) result |= data_read_word_masked_16be(address + 0, mask >> 16) << 16;
	if (mask & 0x0000ffff) result |= data_read_word_masked_16be(address + 2, mask >> 0) << 0;
	return result;
}

UINT64 data_read_qword_16le(offs_t address)
{
	UINT64 result = (UINT64)data_read_dword_16le(address + 0) << 0;
	return result | ((UINT64)data_read_dword_16le(address + 4) << 32);
}

UINT64 data_read_qword_masked_16le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)data_read_dword_masked_16le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)data_read_dword_masked_16le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 data_read_qword_16be(offs_t address)
{
	UINT64 result = (UINT64)data_read_dword_16le(address + 0) << 32;
	return result | ((UINT64)data_read_dword_16le(address + 4) << 0);
}

UINT64 data_read_qword_masked_16be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)data_read_dword_masked_16be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)data_read_dword_masked_16be(address + 4, mask >> 0) << 0;
	return result;
}


/*-------------------------------------------------
    ADDRESS_SPACE_IO
-------------------------------------------------*/

UINT8 io_read_byte_16le(offs_t address)
{
	UINT32 shift = (address & 1) * 8;
	return read_word_generic(ADDRESS_SPACE_IO, address, 0xff << shift) >> shift;
}

UINT8 io_read_byte_16be(offs_t address)
{
	UINT32 shift = (~address & 1) * 8;
	return read_word_generic(ADDRESS_SPACE_IO, address, 0xff << shift) >> shift;
}

UINT16 io_read_word_16le(offs_t address)
{
	return read_word_generic(ADDRESS_SPACE_IO, address, 0xffff);
}

UINT16 io_read_word_masked_16le(offs_t address, UINT16 mask)
{
	return read_word_generic(ADDRESS_SPACE_IO, address, mask);
}

UINT16 io_read_word_16be(offs_t address)
{
	return read_word_generic(ADDRESS_SPACE_IO, address, 0xffff);
}

UINT16 io_read_word_masked_16be(offs_t address, UINT16 mask)
{
	return read_word_generic(ADDRESS_SPACE_IO, address, mask);
}

UINT32 io_read_dword_16le(offs_t address)
{
	UINT32 result = io_read_word_16le(address + 0) << 0;
	return result | (io_read_word_16le(address + 2) << 16);
}

UINT32 io_read_dword_masked_16le(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0x0000ffff) result |= io_read_word_masked_16le(address + 0, mask >> 0) << 0;
	if (mask & 0xffff0000) result |= io_read_word_masked_16le(address + 2, mask >> 16) << 16;
	return result;
}

UINT32 io_read_dword_16be(offs_t address)
{
	UINT32 result = io_read_word_16be(address + 0) << 16;
	return result | (io_read_word_16be(address + 2) << 0);
}

UINT32 io_read_dword_masked_16be(offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0xffff0000) result |= io_read_word_masked_16be(address + 0, mask >> 16) << 16;
	if (mask & 0x0000ffff) result |= io_read_word_masked_16be(address + 2, mask >> 0) << 0;
	return result;
}

UINT64 io_read_qword_16le(offs_t address)
{
	UINT64 result = (UINT64)io_read_dword_16le(address + 0) << 0;
	return result | ((UINT64)io_read_dword_16le(address + 4) << 32);
}

UINT64 io_read_qword_masked_16le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)io_read_dword_masked_16le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)io_read_dword_masked_16le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 io_read_qword_16be(offs_t address)
{
	UINT64 result = (UINT64)io_read_dword_16le(address + 0) << 32;
	return result | ((UINT64)io_read_dword_16le(address + 4) << 0);
}

UINT64 io_read_qword_masked_16be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)io_read_dword_masked_16be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)io_read_dword_masked_16be(address + 4, mask >> 0) << 0;
	return result;
}



/***************************************************************************
    16-BIT WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_SPACE_PROGRAM
-------------------------------------------------*/

void program_write_byte_16le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 1) * 8;
	write_word_generic(ADDRESS_SPACE_PROGRAM, address, data << shift, 0xff << shift);
}

void program_write_byte_16be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 1) * 8;
	write_word_generic(ADDRESS_SPACE_PROGRAM, address, data << shift, 0xff << shift);
}

void program_write_word_16le(offs_t address, UINT16 data)
{
	write_word_generic(ADDRESS_SPACE_PROGRAM, address, data, 0xffff);
}

void program_write_word_masked_16le(offs_t address, UINT16 data, UINT16 mask)
{
	write_word_generic(ADDRESS_SPACE_PROGRAM, address, data, mask);
}

void program_write_word_16be(offs_t address, UINT16 data)
{
	write_word_generic(ADDRESS_SPACE_PROGRAM, address, data, 0xffff);
}

void program_write_word_masked_16be(offs_t address, UINT16 data, UINT16 mask)
{
	write_word_generic(ADDRESS_SPACE_PROGRAM, address, data, mask);
}

void program_write_dword_16le(offs_t address, UINT32 data)
{
	program_write_word_16le(address + 0, data >> 0);
	program_write_word_16le(address + 2, data >> 16);
}

void program_write_dword_masked_16le(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0x0000ffff) program_write_word_masked_16le(address + 0, data >> 0, mask >> 0);
	if (mask & 0xffff0000) program_write_word_masked_16le(address + 2, data >> 16, mask >> 16);
}

void program_write_dword_16be(offs_t address, UINT32 data)
{
	program_write_word_16be(address + 0, data >> 16);
	program_write_word_16be(address + 2, data >> 0);
}

void program_write_dword_masked_16be(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0xffff0000) program_write_word_masked_16be(address + 0, data >> 16, mask >> 16);
	if (mask & 0x0000ffff) program_write_word_masked_16be(address + 2, data >> 0, mask >> 0);
}

void program_write_qword_16le(offs_t address, UINT64 data)
{
	program_write_dword_16le(address + 0, data >> 0);
	program_write_dword_16le(address + 4, data >> 32);
}

void program_write_qword_masked_16le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) program_write_dword_masked_16le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) program_write_dword_masked_16le(address + 4, data >> 32, mask >> 32);
}

void program_write_qword_16be(offs_t address, UINT64 data)
{
	program_write_dword_16be(address + 0, data >> 32);
	program_write_dword_16be(address + 4, data >> 0);
}

void program_write_qword_masked_16be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) program_write_dword_masked_16be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) program_write_dword_masked_16be(address + 4, data >> 0, mask >> 0);
}


/*-------------------------------------------------
    ADDRESS_SPACE_DATA
-------------------------------------------------*/

void data_write_byte_16le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 1) * 8;
	write_word_generic(ADDRESS_SPACE_DATA, address, data << shift, 0xff << shift);
}

void data_write_byte_16be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 1) * 8;
	write_word_generic(ADDRESS_SPACE_DATA, address, data << shift, 0xff << shift);
}

void data_write_word_16le(offs_t address, UINT16 data)
{
	write_word_generic(ADDRESS_SPACE_DATA, address, data, 0xffff);
}

void data_write_word_masked_16le(offs_t address, UINT16 data, UINT16 mask)
{
	write_word_generic(ADDRESS_SPACE_DATA, address, data, mask);
}

void data_write_word_16be(offs_t address, UINT16 data)
{
	write_word_generic(ADDRESS_SPACE_DATA, address, data, 0xffff);
}

void data_write_word_masked_16be(offs_t address, UINT16 data, UINT16 mask)
{
	write_word_generic(ADDRESS_SPACE_DATA, address, data, mask);
}

void data_write_dword_16le(offs_t address, UINT32 data)
{
	data_write_word_16le(address + 0, data >> 0);
	data_write_word_16le(address + 2, data >> 16);
}

void data_write_dword_masked_16le(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0x0000ffff) data_write_word_masked_16le(address + 0, data >> 0, mask >> 0);
	if (mask & 0xffff0000) data_write_word_masked_16le(address + 2, data >> 16, mask >> 16);
}

void data_write_dword_16be(offs_t address, UINT32 data)
{
	data_write_word_16be(address + 0, data >> 16);
	data_write_word_16be(address + 2, data >> 0);
}

void data_write_dword_masked_16be(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0xffff0000) data_write_word_masked_16be(address + 0, data >> 16, mask >> 16);
	if (mask & 0x0000ffff) data_write_word_masked_16be(address + 2, data >> 0, mask >> 0);
}

void data_write_qword_16le(offs_t address, UINT64 data)
{
	data_write_dword_16le(address + 0, data >> 0);
	data_write_dword_16le(address + 4, data >> 32);
}

void data_write_qword_masked_16le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) data_write_dword_masked_16le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) data_write_dword_masked_16le(address + 4, data >> 32, mask >> 32);
}

void data_write_qword_16be(offs_t address, UINT64 data)
{
	data_write_dword_16be(address + 0, data >> 32);
	data_write_dword_16be(address + 4, data >> 0);
}

void data_write_qword_masked_16be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) data_write_dword_masked_16be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) data_write_dword_masked_16be(address + 4, data >> 0, mask >> 0);
}


/*-------------------------------------------------
    ADDRESS_SPACE_IO
-------------------------------------------------*/

void io_write_byte_16le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 1) * 8;
	write_word_generic(ADDRESS_SPACE_IO, address, data << shift, 0xff << shift);
}

void io_write_byte_16be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 1) * 8;
	write_word_generic(ADDRESS_SPACE_IO, address, data << shift, 0xff << shift);
}

void io_write_word_16le(offs_t address, UINT16 data)
{
	write_word_generic(ADDRESS_SPACE_IO, address, data, 0xffff);
}

void io_write_word_masked_16le(offs_t address, UINT16 data, UINT16 mask)
{
	write_word_generic(ADDRESS_SPACE_IO, address, data, mask);
}

void io_write_word_16be(offs_t address, UINT16 data)
{
	write_word_generic(ADDRESS_SPACE_IO, address, data, 0xffff);
}

void io_write_word_masked_16be(offs_t address, UINT16 data, UINT16 mask)
{
	write_word_generic(ADDRESS_SPACE_IO, address, data, mask);
}

void io_write_dword_16le(offs_t address, UINT32 data)
{
	io_write_word_16le(address + 0, data >> 0);
	io_write_word_16le(address + 2, data >> 16);
}

void io_write_dword_masked_16le(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0x0000ffff) io_write_word_masked_16le(address + 0, data >> 0, mask >> 0);
	if (mask & 0xffff0000) io_write_word_masked_16le(address + 2, data >> 16, mask >> 16);
}

void io_write_dword_16be(offs_t address, UINT32 data)
{
	io_write_word_16be(address + 0, data >> 16);
	io_write_word_16be(address + 2, data >> 0);
}

void io_write_dword_masked_16be(offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0xffff0000) io_write_word_masked_16be(address + 0, data >> 16, mask >> 16);
	if (mask & 0x0000ffff) io_write_word_masked_16be(address + 2, data >> 0, mask >> 0);
}

void io_write_qword_16le(offs_t address, UINT64 data)
{
	io_write_dword_16le(address + 0, data >> 0);
	io_write_dword_16le(address + 4, data >> 32);
}

void io_write_qword_masked_16le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) io_write_dword_masked_16le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) io_write_dword_masked_16le(address + 4, data >> 32, mask >> 32);
}

void io_write_qword_16be(offs_t address, UINT64 data)
{
	io_write_dword_16be(address + 0, data >> 32);
	io_write_dword_16be(address + 4, data >> 0);
}

void io_write_qword_masked_16be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) io_write_dword_masked_16be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) io_write_dword_masked_16be(address + 4, data >> 0, mask >> 0);
}



/***************************************************************************
    32-BIT READ HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_SPACE_PROGRAM
-------------------------------------------------*/

UINT8 program_read_byte_32le(offs_t address)
{
	UINT32 shift = (address & 3) * 8;
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, 0xff << shift) >> shift;
}

UINT8 program_read_byte_32be(offs_t address)
{
	UINT32 shift = (~address & 3) * 8;
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, 0xff << shift) >> shift;
}

UINT16 program_read_word_32le(offs_t address)
{
	UINT32 shift = (address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, 0xffff << shift) >> shift;
}

UINT16 program_read_word_masked_32le(offs_t address, UINT16 mask)
{
	UINT32 shift = (address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, mask << shift) >> shift;
}

UINT16 program_read_word_32be(offs_t address)
{
	UINT32 shift = (~address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, 0xffff << shift) >> shift;
}

UINT16 program_read_word_masked_32be(offs_t address, UINT16 mask)
{
	UINT32 shift = (~address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, mask << shift) >> shift;
}

UINT32 program_read_dword_32le(offs_t address)
{
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, 0xffffffff);
}

UINT32 program_read_dword_masked_32le(offs_t address, UINT32 mask)
{
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, mask);
}

UINT32 program_read_dword_32be(offs_t address)
{
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, 0xffffffff);
}

UINT32 program_read_dword_masked_32be(offs_t address, UINT32 mask)
{
	return read_dword_generic(ADDRESS_SPACE_PROGRAM, address, mask);
}

UINT64 program_read_qword_32le(offs_t address)
{
	UINT64 result = (UINT64)program_read_dword_32le(address + 0) << 0;
	return result | ((UINT64)program_read_dword_32le(address + 4) << 32);
}

UINT64 program_read_qword_masked_32le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)program_read_dword_masked_32le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)program_read_dword_masked_32le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 program_read_qword_32be(offs_t address)
{
	UINT64 result = (UINT64)program_read_dword_32le(address + 0) << 32;
	return result | ((UINT64)program_read_dword_32le(address + 4) << 0);
}

UINT64 program_read_qword_masked_32be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)program_read_dword_masked_32be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)program_read_dword_masked_32be(address + 4, mask >> 0) << 0;
	return result;
}


/*-------------------------------------------------
    ADDRESS_SPACE_DATA
-------------------------------------------------*/

UINT8 data_read_byte_32le(offs_t address)
{
	UINT32 shift = (address & 3) * 8;
	return read_dword_generic(ADDRESS_SPACE_DATA, address, 0xff << shift) >> shift;
}

UINT8 data_read_byte_32be(offs_t address)
{
	UINT32 shift = (~address & 3) * 8;
	return read_dword_generic(ADDRESS_SPACE_DATA, address, 0xff << shift) >> shift;
}

UINT16 data_read_word_32le(offs_t address)
{
	UINT32 shift = (address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_DATA, address, 0xffff << shift) >> shift;
}

UINT16 data_read_word_masked_32le(offs_t address, UINT16 mask)
{
	UINT32 shift = (address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_DATA, address, mask << shift) >> shift;
}

UINT16 data_read_word_32be(offs_t address)
{
	UINT32 shift = (~address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_DATA, address, 0xffff << shift) >> shift;
}

UINT16 data_read_word_masked_32be(offs_t address, UINT16 mask)
{
	UINT32 shift = (~address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_DATA, address, mask << shift) >> shift;
}

UINT32 data_read_dword_32le(offs_t address)
{
	return read_dword_generic(ADDRESS_SPACE_DATA, address, 0xffffffff);
}

UINT32 data_read_dword_masked_32le(offs_t address, UINT32 mask)
{
	return read_dword_generic(ADDRESS_SPACE_DATA, address, mask);
}

UINT32 data_read_dword_32be(offs_t address)
{
	return read_dword_generic(ADDRESS_SPACE_DATA, address, 0xffffffff);
}

UINT32 data_read_dword_masked_32be(offs_t address, UINT32 mask)
{
	return read_dword_generic(ADDRESS_SPACE_DATA, address, mask);
}

UINT64 data_read_qword_32le(offs_t address)
{
	UINT64 result = (UINT64)data_read_dword_32le(address + 0) << 0;
	return result | ((UINT64)data_read_dword_32le(address + 4) << 32);
}

UINT64 data_read_qword_masked_32le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)data_read_dword_masked_32le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)data_read_dword_masked_32le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 data_read_qword_32be(offs_t address)
{
	UINT64 result = (UINT64)data_read_dword_32le(address + 0) << 32;
	return result | ((UINT64)data_read_dword_32le(address + 4) << 0);
}

UINT64 data_read_qword_masked_32be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)data_read_dword_masked_32be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)data_read_dword_masked_32be(address + 4, mask >> 0) << 0;
	return result;
}


/*-------------------------------------------------
    ADDRESS_SPACE_IO
-------------------------------------------------*/

UINT8 io_read_byte_32le(offs_t address)
{
	UINT32 shift = (address & 3) * 8;
	return read_dword_generic(ADDRESS_SPACE_IO, address, 0xff << shift) >> shift;
}

UINT8 io_read_byte_32be(offs_t address)
{
	UINT32 shift = (~address & 3) * 8;
	return read_dword_generic(ADDRESS_SPACE_IO, address, 0xff << shift) >> shift;
}

UINT16 io_read_word_32le(offs_t address)
{
	UINT32 shift = (address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_IO, address, 0xffff << shift) >> shift;
}

UINT16 io_read_word_masked_32le(offs_t address, UINT16 mask)
{
	UINT32 shift = (address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_IO, address, mask << shift) >> shift;
}

UINT16 io_read_word_32be(offs_t address)
{
	UINT32 shift = (~address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_IO, address, 0xffff << shift) >> shift;
}

UINT16 io_read_word_masked_32be(offs_t address, UINT16 mask)
{
	UINT32 shift = (~address & 2) * 8;
	return read_dword_generic(ADDRESS_SPACE_IO, address, mask << shift) >> shift;
}

UINT32 io_read_dword_32le(offs_t address)
{
	return read_dword_generic(ADDRESS_SPACE_IO, address, 0xffffffff);
}

UINT32 io_read_dword_masked_32le(offs_t address, UINT32 mask)
{
	return read_dword_generic(ADDRESS_SPACE_IO, address, mask);
}

UINT32 io_read_dword_32be(offs_t address)
{
	return read_dword_generic(ADDRESS_SPACE_IO, address, 0xffffffff);
}

UINT32 io_read_dword_masked_32be(offs_t address, UINT32 mask)
{
	return read_dword_generic(ADDRESS_SPACE_IO, address, mask);
}

UINT64 io_read_qword_32le(offs_t address)
{
	UINT64 result = (UINT64)io_read_dword_32le(address + 0) << 0;
	return result | ((UINT64)io_read_dword_32le(address + 4) << 32);
}

UINT64 io_read_qword_masked_32le(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)io_read_dword_masked_32le(address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)io_read_dword_masked_32le(address + 4, mask >> 32) << 32;
	return result;
}

UINT64 io_read_qword_32be(offs_t address)
{
	UINT64 result = (UINT64)io_read_dword_32le(address + 0) << 32;
	return result | ((UINT64)io_read_dword_32le(address + 4) << 0);
}

UINT64 io_read_qword_masked_32be(offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)io_read_dword_masked_32be(address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)io_read_dword_masked_32be(address + 4, mask >> 0) << 0;
	return result;
}



/***************************************************************************
    32-BIT WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_SPACE_PROGRAM
-------------------------------------------------*/

void program_write_byte_32le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 3) * 8;
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data << shift, 0xff << shift);
}

void program_write_byte_32be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 3) * 8;
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data << shift, 0xff << shift);
}

void program_write_word_32le(offs_t address, UINT16 data)
{
	UINT32 shift = (address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data << shift, 0xffff << shift);
}

void program_write_word_masked_32le(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data << shift, mask << shift);
}

void program_write_word_32be(offs_t address, UINT16 data)
{
	UINT32 shift = (~address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data << shift, 0xffff << shift);
}

void program_write_word_masked_32be(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (~address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data << shift, mask << shift);
}

void program_write_dword_32le(offs_t address, UINT32 data)
{
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data, 0xffffffff);
}

void program_write_dword_masked_32le(offs_t address, UINT32 data, UINT32 mask)
{
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data, mask);
}

void program_write_dword_32be(offs_t address, UINT32 data)
{
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data, 0xffffffff);
}

void program_write_dword_masked_32be(offs_t address, UINT32 data, UINT32 mask)
{
	write_dword_generic(ADDRESS_SPACE_PROGRAM, address, data, mask);
}

void program_write_qword_32le(offs_t address, UINT64 data)
{
	program_write_dword_32le(address + 0, data >> 0);
	program_write_dword_32le(address + 4, data >> 32);
}

void program_write_qword_masked_32le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) program_write_dword_masked_32le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) program_write_dword_masked_32le(address + 4, data >> 32, mask >> 32);
}

void program_write_qword_32be(offs_t address, UINT64 data)
{
	program_write_dword_32be(address + 0, data >> 32);
	program_write_dword_32be(address + 4, data >> 0);
}

void program_write_qword_masked_32be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) program_write_dword_masked_32be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) program_write_dword_masked_32be(address + 4, data >> 0, mask >> 0);
}


/*-------------------------------------------------
    ADDRESS_SPACE_DATA
-------------------------------------------------*/

void data_write_byte_32le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 3) * 8;
	write_dword_generic(ADDRESS_SPACE_DATA, address, data << shift, 0xff << shift);
}

void data_write_byte_32be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 3) * 8;
	write_dword_generic(ADDRESS_SPACE_DATA, address, data << shift, 0xff << shift);
}

void data_write_word_32le(offs_t address, UINT16 data)
{
	UINT32 shift = (address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_DATA, address, data << shift, 0xffff << shift);
}

void data_write_word_masked_32le(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_DATA, address, data << shift, mask << shift);
}

void data_write_word_32be(offs_t address, UINT16 data)
{
	UINT32 shift = (~address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_DATA, address, data << shift, 0xffff << shift);
}

void data_write_word_masked_32be(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (~address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_DATA, address, data << shift, mask << shift);
}

void data_write_dword_32le(offs_t address, UINT32 data)
{
	write_dword_generic(ADDRESS_SPACE_DATA, address, data, 0xffffffff);
}

void data_write_dword_masked_32le(offs_t address, UINT32 data, UINT32 mask)
{
	write_dword_generic(ADDRESS_SPACE_DATA, address, data, mask);
}

void data_write_dword_32be(offs_t address, UINT32 data)
{
	write_dword_generic(ADDRESS_SPACE_DATA, address, data, 0xffffffff);
}

void data_write_dword_masked_32be(offs_t address, UINT32 data, UINT32 mask)
{
	write_dword_generic(ADDRESS_SPACE_DATA, address, data, mask);
}

void data_write_qword_32le(offs_t address, UINT64 data)
{
	data_write_dword_32le(address + 0, data >> 0);
	data_write_dword_32le(address + 4, data >> 32);
}

void data_write_qword_masked_32le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) data_write_dword_masked_32le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) data_write_dword_masked_32le(address + 4, data >> 32, mask >> 32);
}

void data_write_qword_32be(offs_t address, UINT64 data)
{
	data_write_dword_32be(address + 0, data >> 32);
	data_write_dword_32be(address + 4, data >> 0);
}

void data_write_qword_masked_32be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) data_write_dword_masked_32be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) data_write_dword_masked_32be(address + 4, data >> 0, mask >> 0);
}


/*-------------------------------------------------
    ADDRESS_SPACE_IO
-------------------------------------------------*/

void io_write_byte_32le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 3) * 8;
	write_dword_generic(ADDRESS_SPACE_IO, address, data << shift, 0xff << shift);
}

void io_write_byte_32be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 3) * 8;
	write_dword_generic(ADDRESS_SPACE_IO, address, data << shift, 0xff << shift);
}

void io_write_word_32le(offs_t address, UINT16 data)
{
	UINT32 shift = (address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_IO, address, data << shift, 0xffff << shift);
}

void io_write_word_masked_32le(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_IO, address, data << shift, mask << shift);
}

void io_write_word_32be(offs_t address, UINT16 data)
{
	UINT32 shift = (~address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_IO, address, data << shift, 0xffff << shift);
}

void io_write_word_masked_32be(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (~address & 2) * 8;
	write_dword_generic(ADDRESS_SPACE_IO, address, data << shift, mask << shift);
}

void io_write_dword_32le(offs_t address, UINT32 data)
{
	write_dword_generic(ADDRESS_SPACE_IO, address, data, 0xffffffff);
}

void io_write_dword_masked_32le(offs_t address, UINT32 data, UINT32 mask)
{
	write_dword_generic(ADDRESS_SPACE_IO, address, data, mask);
}

void io_write_dword_32be(offs_t address, UINT32 data)
{
	write_dword_generic(ADDRESS_SPACE_IO, address, data, 0xffffffff);
}

void io_write_dword_masked_32be(offs_t address, UINT32 data, UINT32 mask)
{
	write_dword_generic(ADDRESS_SPACE_IO, address, data, mask);
}

void io_write_qword_32le(offs_t address, UINT64 data)
{
	io_write_dword_32le(address + 0, data >> 0);
	io_write_dword_32le(address + 4, data >> 32);
}

void io_write_qword_masked_32le(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) io_write_dword_masked_32le(address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) io_write_dword_masked_32le(address + 4, data >> 32, mask >> 32);
}

void io_write_qword_32be(offs_t address, UINT64 data)
{
	io_write_dword_32be(address + 0, data >> 32);
	io_write_dword_32be(address + 4, data >> 0);
}

void io_write_qword_masked_32be(offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) io_write_dword_masked_32be(address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) io_write_dword_masked_32be(address + 4, data >> 0, mask >> 0);
}



/***************************************************************************
    64-BIT READ HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_SPACE_PROGRAM
-------------------------------------------------*/

UINT8 program_read_byte_64le(offs_t address)
{
	UINT32 shift = (address & 7) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)0xff << shift) >> shift;
}

UINT8 program_read_byte_64be(offs_t address)
{
	UINT32 shift = (~address & 7) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)0xff << shift) >> shift;
}

UINT16 program_read_word_64le(offs_t address)
{
	UINT32 shift = (address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)0xffff << shift) >> shift;
}

UINT16 program_read_word_masked_64le(offs_t address, UINT16 mask)
{
	UINT32 shift = (address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)mask << shift) >> shift;
}

UINT16 program_read_word_64be(offs_t address)
{
	UINT32 shift = (~address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)0xffff << shift) >> shift;
}

UINT16 program_read_word_masked_64be(offs_t address, UINT16 mask)
{
	UINT32 shift = (~address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)mask << shift) >> shift;
}

UINT32 program_read_dword_64le(offs_t address)
{
	UINT32 shift = (address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)0xffffffff << shift) >> shift;
}

UINT32 program_read_dword_masked_64le(offs_t address, UINT32 mask)
{
	UINT32 shift = (address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)mask << shift) >> shift;
}

UINT32 program_read_dword_64be(offs_t address)
{
	UINT32 shift = (~address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)0xffffffff << shift) >> shift;
}

UINT32 program_read_dword_masked_64be(offs_t address, UINT32 mask)
{
	UINT32 shift = (~address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)mask << shift) >> shift;
}

UINT64 program_read_qword_64le(offs_t address)
{
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, U64(0xffffffffffffffff));
}

UINT64 program_read_qword_masked_64le(offs_t address, UINT64 mask)
{
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, mask);
}

UINT64 program_read_qword_64be(offs_t address)
{
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, U64(0xffffffffffffffff));
}

UINT64 program_read_qword_masked_64be(offs_t address, UINT64 mask)
{
	return read_qword_generic(ADDRESS_SPACE_PROGRAM, address, mask);
}


/*-------------------------------------------------
    ADDRESS_SPACE_DATA
-------------------------------------------------*/

UINT8 data_read_byte_64le(offs_t address)
{
	UINT32 shift = (address & 7) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)0xff << shift) >> shift;
}

UINT8 data_read_byte_64be(offs_t address)
{
	UINT32 shift = (~address & 7) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)0xff << shift) >> shift;
}

UINT16 data_read_word_64le(offs_t address)
{
	UINT32 shift = (address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)0xffff << shift) >> shift;
}

UINT16 data_read_word_masked_64le(offs_t address, UINT16 mask)
{
	UINT32 shift = (address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)mask << shift) >> shift;
}

UINT16 data_read_word_64be(offs_t address)
{
	UINT32 shift = (~address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)0xffff << shift) >> shift;
}

UINT16 data_read_word_masked_64be(offs_t address, UINT16 mask)
{
	UINT32 shift = (~address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)mask << shift) >> shift;
}

UINT32 data_read_dword_64le(offs_t address)
{
	UINT32 shift = (address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)0xffffffff << shift) >> shift;
}

UINT32 data_read_dword_masked_64le(offs_t address, UINT32 mask)
{
	UINT32 shift = (address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)mask << shift) >> shift;
}

UINT32 data_read_dword_64be(offs_t address)
{
	UINT32 shift = (~address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)0xffffffff << shift) >> shift;
}

UINT32 data_read_dword_masked_64be(offs_t address, UINT32 mask)
{
	UINT32 shift = (~address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)mask << shift) >> shift;
}

UINT64 data_read_qword_64le(offs_t address)
{
	return read_qword_generic(ADDRESS_SPACE_DATA, address, U64(0xffffffffffffffff));
}

UINT64 data_read_qword_masked_64le(offs_t address, UINT64 mask)
{
	return read_qword_generic(ADDRESS_SPACE_DATA, address, mask);
}

UINT64 data_read_qword_64be(offs_t address)
{
	return read_qword_generic(ADDRESS_SPACE_DATA, address, U64(0xffffffffffffffff));
}

UINT64 data_read_qword_masked_64be(offs_t address, UINT64 mask)
{
	return read_qword_generic(ADDRESS_SPACE_DATA, address, mask);
}


/*-------------------------------------------------
    ADDRESS_SPACE_IO
-------------------------------------------------*/

UINT8 io_read_byte_64le(offs_t address)
{
	UINT32 shift = (address & 7) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)0xff << shift) >> shift;
}

UINT8 io_read_byte_64be(offs_t address)
{
	UINT32 shift = (~address & 7) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)0xff << shift) >> shift;
}

UINT16 io_read_word_64le(offs_t address)
{
	UINT32 shift = (address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)0xffff << shift) >> shift;
}

UINT16 io_read_word_masked_64le(offs_t address, UINT16 mask)
{
	UINT32 shift = (address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)mask << shift) >> shift;
}

UINT16 io_read_word_64be(offs_t address)
{
	UINT32 shift = (~address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)0xffff << shift) >> shift;
}

UINT16 io_read_word_masked_64be(offs_t address, UINT16 mask)
{
	UINT32 shift = (~address & 6) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)mask << shift) >> shift;
}

UINT32 io_read_dword_64le(offs_t address)
{
	UINT32 shift = (address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)0xffffffff << shift) >> shift;
}

UINT32 io_read_dword_masked_64le(offs_t address, UINT32 mask)
{
	UINT32 shift = (address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)mask << shift) >> shift;
}

UINT32 io_read_dword_64be(offs_t address)
{
	UINT32 shift = (~address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)0xffffffff << shift) >> shift;
}

UINT32 io_read_dword_masked_64be(offs_t address, UINT32 mask)
{
	UINT32 shift = (~address & 4) * 8;
	return read_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)mask << shift) >> shift;
}

UINT64 io_read_qword_64le(offs_t address)
{
	return read_qword_generic(ADDRESS_SPACE_IO, address, U64(0xffffffffffffffff));
}

UINT64 io_read_qword_masked_64le(offs_t address, UINT64 mask)
{
	return read_qword_generic(ADDRESS_SPACE_IO, address, mask);
}

UINT64 io_read_qword_64be(offs_t address)
{
	return read_qword_generic(ADDRESS_SPACE_IO, address, U64(0xffffffffffffffff));
}

UINT64 io_read_qword_masked_64be(offs_t address, UINT64 mask)
{
	return read_qword_generic(ADDRESS_SPACE_IO, address, mask);
}



/***************************************************************************
    64-BIT WRITE HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ADDRESS_SPACE_PROGRAM
-------------------------------------------------*/

void program_write_byte_64le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 7) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)0xff << shift);
}

void program_write_byte_64be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 7) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)0xff << shift);
}

void program_write_word_64le(offs_t address, UINT16 data)
{
	UINT32 shift = (address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)0xffff << shift);
}

void program_write_word_masked_64le(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void program_write_word_64be(offs_t address, UINT16 data)
{
	UINT32 shift = (~address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)0xffff << shift);
}

void program_write_word_masked_64be(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (~address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void program_write_dword_64le(offs_t address, UINT32 data)
{
	UINT32 shift = (address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)0xffffffff << shift);
}

void program_write_dword_masked_64le(offs_t address, UINT32 data, UINT32 mask)
{
	UINT32 shift = (address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void program_write_dword_64be(offs_t address, UINT32 data)
{
	UINT32 shift = (~address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)0xffffffff << shift);
}

void program_write_dword_masked_64be(offs_t address, UINT32 data, UINT32 mask)
{
	UINT32 shift = (~address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void program_write_qword_64le(offs_t address, UINT64 data)
{
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, data, U64(0xffffffffffffffff));
}

void program_write_qword_masked_64le(offs_t address, UINT64 data, UINT64 mask)
{
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, data, mask);
}

void program_write_qword_64be(offs_t address, UINT64 data)
{
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, data, U64(0xffffffffffffffff));
}

void program_write_qword_masked_64be(offs_t address, UINT64 data, UINT64 mask)
{
	write_qword_generic(ADDRESS_SPACE_PROGRAM, address, data, mask);
}


/*-------------------------------------------------
    ADDRESS_SPACE_DATA
-------------------------------------------------*/

void data_write_byte_64le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 7) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)0xff << shift);
}

void data_write_byte_64be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 7) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)0xff << shift);
}

void data_write_word_64le(offs_t address, UINT16 data)
{
	UINT32 shift = (address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)0xffff << shift);
}

void data_write_word_masked_64le(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void data_write_word_64be(offs_t address, UINT16 data)
{
	UINT32 shift = (~address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)0xffff << shift);
}

void data_write_word_masked_64be(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (~address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void data_write_dword_64le(offs_t address, UINT32 data)
{
	UINT32 shift = (address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)0xffffffff << shift);
}

void data_write_dword_masked_64le(offs_t address, UINT32 data, UINT32 mask)
{
	UINT32 shift = (address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void data_write_dword_64be(offs_t address, UINT32 data)
{
	UINT32 shift = (~address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)0xffffffff << shift);
}

void data_write_dword_masked_64be(offs_t address, UINT32 data, UINT32 mask)
{
	UINT32 shift = (~address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_DATA, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void data_write_qword_64le(offs_t address, UINT64 data)
{
	write_qword_generic(ADDRESS_SPACE_DATA, address, data, U64(0xffffffffffffffff));
}

void data_write_qword_masked_64le(offs_t address, UINT64 data, UINT64 mask)
{
	write_qword_generic(ADDRESS_SPACE_DATA, address, data, mask);
}

void data_write_qword_64be(offs_t address, UINT64 data)
{
	write_qword_generic(ADDRESS_SPACE_DATA, address, data, U64(0xffffffffffffffff));
}

void data_write_qword_masked_64be(offs_t address, UINT64 data, UINT64 mask)
{
	write_qword_generic(ADDRESS_SPACE_DATA, address, data, mask);
}


/*-------------------------------------------------
    ADDRESS_SPACE_IO
-------------------------------------------------*/

void io_write_byte_64le(offs_t address, UINT8 data)
{
	UINT32 shift = (address & 7) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)0xff << shift);
}

void io_write_byte_64be(offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 7) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)0xff << shift);
}

void io_write_word_64le(offs_t address, UINT16 data)
{
	UINT32 shift = (address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)0xffff << shift);
}

void io_write_word_masked_64le(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void io_write_word_64be(offs_t address, UINT16 data)
{
	UINT32 shift = (~address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)0xffff << shift);
}

void io_write_word_masked_64be(offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (~address & 6) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void io_write_dword_64le(offs_t address, UINT32 data)
{
	UINT32 shift = (address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)0xffffffff << shift);
}

void io_write_dword_masked_64le(offs_t address, UINT32 data, UINT32 mask)
{
	UINT32 shift = (address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void io_write_dword_64be(offs_t address, UINT32 data)
{
	UINT32 shift = (~address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)0xffffffff << shift);
}

void io_write_dword_masked_64be(offs_t address, UINT32 data, UINT32 mask)
{
	UINT32 shift = (~address & 4) * 8;
	write_qword_generic(ADDRESS_SPACE_IO, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void io_write_qword_64le(offs_t address, UINT64 data)
{
	write_qword_generic(ADDRESS_SPACE_IO, address, data, U64(0xffffffffffffffff));
}

void io_write_qword_masked_64le(offs_t address, UINT64 data, UINT64 mask)
{
	write_qword_generic(ADDRESS_SPACE_IO, address, data, mask);
}

void io_write_qword_64be(offs_t address, UINT64 data)
{
	write_qword_generic(ADDRESS_SPACE_IO, address, data, U64(0xffffffffffffffff));
}

void io_write_qword_masked_64be(offs_t address, UINT64 data, UINT64 mask)
{
	write_qword_generic(ADDRESS_SPACE_IO, address, data, mask);
}
