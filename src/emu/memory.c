/***************************************************************************

    memory.c

    Functions which handle device memory access.

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
#define MAX_EXPLICIT_BANKS		96						/* maximum number of explicitly-defined banks */

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

#define SUBTABLE_PTR(tabledata, entry) (&(tabledata)->table[(1 << LEVEL1_BITS) + (((entry) - SUBTABLE_BASE) << LEVEL2_BITS)])



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* a memory block is a chunk of RAM associated with a range of memory in a device's address space */
typedef struct _memory_block memory_block;
struct _memory_block
{
	memory_block *			next;					/* next memory block in the list */
	const address_space *	space;					/* which address space are we associated with? */
	UINT8					isallocated;			/* did we allocate this ourselves? */
	offs_t 					bytestart, byteend;		/* byte-normalized start/end for verifying a match */
	UINT8 *					data;					/* pointer to the data for this block */
};

/* a bank reference is an entry in a list of address spaces that reference a given bank */
typedef struct _bank_reference bank_reference;
struct _bank_reference
{
	bank_reference *		next;					/* link to the next reference */
	const address_space *	space;					/* address space that references us */
};

/* a bank is a global pointer to memory that can be shared across devices and changed dynamically */
typedef struct _bank_data bank_info;
struct _bank_data
{
	UINT8 					used;					/* is this bank used? */
	UINT8 					dynamic;				/* is this bank allocated dynamically? */
	bank_reference *		reflist;				/* linked list of address spaces referencing this bank */
	UINT8 					read;					/* is this bank used for reads? */
	UINT8 					write;					/* is this bank used for writes? */
	offs_t 					bytestart;				/* byte-adjusted start offset */
	offs_t 					byteend;				/* byte-adjusted end offset */
	UINT16					curentry;				/* current entry */
	void *					entry[MAX_BANK_ENTRIES];/* array of entries for this bank */
	void *					entryd[MAX_BANK_ENTRIES];/* array of decrypted entries for this bank */
};

/* In memory.h: typedef struct _direct_range direct_range; */
struct _direct_range
{
    direct_range *			next;					/* pointer to the next range in the list */
    offs_t 					bytestart;				/* starting byte offset of the range */
    offs_t					byteend;				/* ending byte offset of the range */
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
	UINT8 **				bankbaseptr;			/* pointer to the bank base */
};

/* In memory.h: typedef struct _subtable_data subtable_data; */
struct _subtable_data
{
	UINT8					checksum_valid;			/* is the checksum valid */
	UINT32					checksum;				/* checksum over all the bytes */
	UINT32					usecount;				/* number of times this has been used */
};

struct _memory_private
{
	const address_space *	spacelist;						/* list of address spaces */

	UINT8 *					bank_ptr[STATIC_COUNT];			/* array of bank pointers */
	UINT8 *					bankd_ptr[STATIC_COUNT];		/* array of decrypted bank pointers */
	void *					shared_ptr[MAX_SHARED_POINTERS];/* array of shared pointers */

	memory_block *			memory_block_list;				/* head of the list of memory blocks */

	bank_info 				bankdata[STATIC_COUNT];			/* data gathered for each bank */

	UINT8 *					wptable;						/* watchpoint-fill table */
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

#define ACCESSOR_GROUP(width) \
{ \
	memory_read_byte_##width, \
	memory_read_word_##width, \
	memory_read_word_masked_##width, \
	memory_read_dword_##width, \
	memory_read_dword_masked_##width, \
	memory_read_qword_##width, \
	memory_read_qword_masked_##width, \
	memory_write_byte_##width, \
	memory_write_word_##width, \
	memory_write_word_masked_##width, \
	memory_write_dword_##width, \
	memory_write_dword_masked_##width, \
	memory_write_qword_##width, \
	memory_write_qword_masked_##width \
}

static const data_accessors memory_accessors[4][2] =
{
	{ ACCESSOR_GROUP(8le),  ACCESSOR_GROUP(8be)  },
	{ ACCESSOR_GROUP(16le), ACCESSOR_GROUP(16be) },
	{ ACCESSOR_GROUP(32le), ACCESSOR_GROUP(32be) },
	{ ACCESSOR_GROUP(64le), ACCESSOR_GROUP(64be) }
};

const char *const address_space_names[ADDRESS_SPACES] = { "program", "data", "I/O" };



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* internal initialization */
static void memory_init_spaces(running_machine *machine);
static void memory_init_preflight(running_machine *machine);
static void memory_init_populate(running_machine *machine);
static void memory_init_allocate(running_machine *machine);
static void memory_init_locate(running_machine *machine);
static void memory_exit(running_machine *machine);

/* address map helpers */
static void map_detokenize(address_map *map, const game_driver *driver, const char *devtag, const addrmap_token *tokens);

/* memory mapping helpers */
static void space_map_range_private(address_space *space, read_or_write readorwrite, int handlerbits, int handlerunitmask, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, genf *handler, void *object, const char *handler_name);
static void space_map_range(address_space *space, read_or_write readorwrite, int handlerbits, int handlerunitmask, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, genf *handler, void *object, const char *handler_name);
static void *space_find_backing_memory(const address_space *space, offs_t byteaddress);
static int space_needs_backing_store(const address_space *space, const address_map_entry *entry);

/* banking helpers */
static void bank_assign_static(int banknum, const address_space *space, read_or_write readorwrite, offs_t bytestart, offs_t byteend);
static genf *bank_assign_dynamic(const address_space *space, read_or_write readorwrite, offs_t bytestart, offs_t byteend);
static STATE_POSTLOAD( bank_reattach );

/* table management */
static UINT8 table_assign_handler(const address_space *space, handler_data **table, void *object, genf *handler, const char *handler_name, offs_t bytestart, offs_t byteend, offs_t bytemask);
static void table_compute_subhandler(handler_data **table, UINT8 entry, read_or_write readorwrite, int spacebits, int spaceendian, int handlerbits, int handlerunitmask);
static void table_populate_range(address_table *tabledata, offs_t bytestart, offs_t byteend, UINT8 handler);
static void table_populate_range_mirrored(address_space *space, address_table *tabledata, offs_t bytestart, offs_t byteend, offs_t bytemirror, UINT8 handler);
static UINT8 table_derive_range(const address_table *table, offs_t byteaddress, offs_t *bytestart, offs_t *byteend);

/* subtable management */
static UINT8 subtable_alloc(address_table *tabledata);
static void subtable_realloc(address_table *tabledata, UINT8 subentry);
static int subtable_merge(address_table *tabledata);
static void subtable_release(address_table *tabledata, UINT8 subentry);
static UINT8 *subtable_open(address_table *tabledata, offs_t l1index);
static void subtable_close(address_table *tabledata, offs_t l1index);

/* direct memory ranges */
static direct_range *direct_range_find(address_space *space, offs_t byteaddress, UINT8 *entry);
static void direct_range_remove_intersecting(address_space *space, offs_t bytestart, offs_t byteend);

/* memory block allocation */
static void *block_allocate(const address_space *space, offs_t bytestart, offs_t byteend, void *memory);
static address_map_entry *block_assign_intersecting(address_space *space, offs_t bytestart, offs_t byteend, UINT8 *base);

/* internal handlers */
static memory_handler get_stub_handler(read_or_write readorwrite, int spacedbits, int handlerdbits);
static genf *get_static_handler(int handlerbits, int readorwrite, int which);

/* debugging */
static const char *handler_to_string(const address_table *table, UINT8 entry);
static void dump_map(FILE *file, const address_space *space, const address_table *table);
static void mem_dump(running_machine *machine);

/* input port handlers */
static UINT8 input_port_read8(const input_port_config *port, offs_t offset);
static UINT16 input_port_read16(const input_port_config *port, offs_t offset, UINT16 mem_mask);
static UINT32 input_port_read32(const input_port_config *port, offs_t offset, UINT32 mem_mask);
static UINT64 input_port_read64(const input_port_config *port, offs_t offset, UINT64 mem_mask);

/* output port handlers */
static void input_port_write8(const input_port_config *port, offs_t offset, UINT8 data);
static void input_port_write16(const input_port_config *port, offs_t offset, UINT16 data, UINT16 mem_mask);
static void input_port_write32(const input_port_config *port, offs_t offset, UINT32 data, UINT32 mem_mask);
static void input_port_write64(const input_port_config *port, offs_t offset, UINT64 data, UINT64 mem_mask);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    force_opbase_update - ensure that we update
    the opcode base
-------------------------------------------------*/

INLINE void force_opbase_update(const address_space *space)
{
	address_space *spacerw = (address_space *)space;
	spacerw->direct.byteend = 0;
	spacerw->direct.bytestart = 1;
}


/*-------------------------------------------------
    adjust_addresses - adjust addresses for a
    given address space in a standard fashion
-------------------------------------------------*/

INLINE void adjust_addresses(address_space *space, offs_t *start, offs_t *end, offs_t *mask, offs_t *mirror)
{
	/* adjust start/end/mask values */
	if (*mask == 0)
		*mask = space->addrmask & ~*mirror;
	else
		*mask &= space->addrmask;
	*start &= ~*mirror & space->addrmask;
	*end &= ~*mirror & space->addrmask;

	/* adjust to byte values */
	*start = memory_address_to_byte(space, *start);
	*end = memory_address_to_byte_end(space, *end);
	*mask = memory_address_to_byte_end(space, *mask);
	*mirror = memory_address_to_byte(space, *mirror);
}


/*-------------------------------------------------
    bank_references_space - return true if the
    given bank is referenced by a particular
    address space
-------------------------------------------------*/

INLINE int bank_references_space(const bank_info *bank, const address_space *space)
{
	bank_reference *ref;

	for (ref = bank->reflist; ref != NULL; ref = ref->next)
		if (ref->space == space)
			return TRUE;
	return FALSE;
}


/*-------------------------------------------------
    add_bank_reference - add a new address space
    reference to a bank
-------------------------------------------------*/

INLINE void add_bank_reference(bank_info *bank, const address_space *space)
{
	bank_reference **refptr;

	/* make sure we don't already have a reference to the bank */
	for (refptr = &bank->reflist; *refptr != NULL; refptr = &(*refptr)->next)
		if ((*refptr)->space == space)
			return;

	/* allocate a new entry and fill it */
	(*refptr) = alloc_or_die(bank_reference);
	(*refptr)->next = NULL;
	(*refptr)->space = space;
}


/*-------------------------------------------------
    read_byte_generic - read a byte from an
    arbitrary address space
-------------------------------------------------*/

INLINE UINT8 read_byte_generic(const address_space *space, offs_t byteaddress)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT32 entry;
	UINT8 result;

	profiler_mark_start(PROFILER_MEMREAD);

	byteaddress &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->read.handlers[entry];

	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		result = (*handler->bankbaseptr)[byteoffset];
	else
		result = (*handler->handler.read.shandler8)((const address_space *)handler->object, byteoffset);

	profiler_mark_end();
	return result;
}


/*-------------------------------------------------
    write_byte_generic - write a byte to an
    arbitrary address space
-------------------------------------------------*/

INLINE void write_byte_generic(const address_space *space, offs_t byteaddress, UINT8 data)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT32 entry;

	profiler_mark_start(PROFILER_MEMWRITE);

	byteaddress &= space->bytemask;
	entry = space->writelookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->writelookup[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->write.handlers[entry];

	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		(*handler->bankbaseptr)[byteoffset] = data;
	else
		(*handler->handler.write.shandler8)((const address_space *)handler->object, byteoffset, data);

	profiler_mark_end();
}


/*-------------------------------------------------
    read_word_generic - read a word from an
    arbitrary address space
-------------------------------------------------*/

INLINE UINT16 read_word_generic(const address_space *space, offs_t byteaddress, UINT16 mem_mask)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT32 entry;
	UINT16 result;

	profiler_mark_start(PROFILER_MEMREAD);

	byteaddress &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->read.handlers[entry];

	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		result = *(UINT16 *)&(*handler->bankbaseptr)[byteoffset & ~1];
	else
		result = (*handler->handler.read.shandler16)((const address_space *)handler->object, byteoffset >> 1, mem_mask);

	profiler_mark_end();
	return result;
}


/*-------------------------------------------------
    write_word_generic - write a word to an
    arbitrary address space
-------------------------------------------------*/

INLINE void write_word_generic(const address_space *space, offs_t byteaddress, UINT16 data, UINT16 mem_mask)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT32 entry;

	profiler_mark_start(PROFILER_MEMWRITE);

	byteaddress &= space->bytemask;
	entry = space->writelookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->writelookup[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->write.handlers[entry];

	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
	{
		UINT16 *dest = (UINT16 *)&(*handler->bankbaseptr)[byteoffset & ~1];
		*dest = (*dest & ~mem_mask) | (data & mem_mask);
	}
	else
		(*handler->handler.write.shandler16)((const address_space *)handler->object, byteoffset >> 1, data, mem_mask);

	profiler_mark_end();
}


/*-------------------------------------------------
    read_dword_generic - read a dword from an
    arbitrary address space
-------------------------------------------------*/

INLINE UINT32 read_dword_generic(const address_space *space, offs_t byteaddress, UINT32 mem_mask)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT32 entry;
	UINT32 result;

	profiler_mark_start(PROFILER_MEMREAD);

	byteaddress &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->read.handlers[entry];

	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		result = *(UINT32 *)&(*handler->bankbaseptr)[byteoffset & ~3];
	else
		result = (*handler->handler.read.shandler32)((const address_space *)handler->object, byteoffset >> 2, mem_mask);

	profiler_mark_end();
	return result;
}


/*-------------------------------------------------
    write_dword_generic - write a dword to an
    arbitrary address space
-------------------------------------------------*/

INLINE void write_dword_generic(const address_space *space, offs_t byteaddress, UINT32 data, UINT32 mem_mask)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT32 entry;

	profiler_mark_start(PROFILER_MEMWRITE);

	byteaddress &= space->bytemask;
	entry = space->writelookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->writelookup[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->write.handlers[entry];

	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
	{
		UINT32 *dest = (UINT32 *)&(*handler->bankbaseptr)[byteoffset & ~3];
		*dest = (*dest & ~mem_mask) | (data & mem_mask);
	}
	else
		(*handler->handler.write.shandler32)((const address_space *)handler->object, byteoffset >> 2, data, mem_mask);

	profiler_mark_end();
}


/*-------------------------------------------------
    read_qword_generic - read a qword from an
    arbitrary address space
-------------------------------------------------*/

INLINE UINT64 read_qword_generic(const address_space *space, offs_t byteaddress, UINT64 mem_mask)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT32 entry;
	UINT64 result;

	profiler_mark_start(PROFILER_MEMREAD);

	byteaddress &= space->bytemask;
	entry = space->readlookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->readlookup[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->read.handlers[entry];

	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
		result = *(UINT64 *)&(*handler->bankbaseptr)[byteoffset & ~7];
	else
		result = (*handler->handler.read.shandler64)((const address_space *)handler->object, byteoffset >> 3, mem_mask);

	profiler_mark_end();
	return result;
}


/*-------------------------------------------------
    write_qword_generic - write a qword to an
    arbitrary address space
-------------------------------------------------*/

INLINE void write_qword_generic(const address_space *space, offs_t byteaddress, UINT64 data, UINT64 mem_mask)
{
	const handler_data *handler;
	offs_t offset;
	UINT32 entry;

	profiler_mark_start(PROFILER_MEMWRITE);

	byteaddress &= space->bytemask;
	entry = space->writelookup[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->writelookup[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->write.handlers[entry];

	offset = (byteaddress - handler->bytestart) & handler->bytemask;
	if (entry < STATIC_RAM)
	{
		UINT64 *dest = (UINT64 *)&(*handler->bankbaseptr)[offset & ~7];
		*dest = (*dest & ~mem_mask) | (data & mem_mask);
	}
	else
		(*handler->handler.write.shandler64)((const address_space *)handler->object, offset >> 3, data, mem_mask);

	profiler_mark_end();
}



/***************************************************************************
    CORE SYSTEM OPERATIONS
***************************************************************************/

/*-------------------------------------------------
    memory_init - initialize the memory system
-------------------------------------------------*/

void memory_init(running_machine *machine)
{
	memory_private *memdata;

	add_exit_callback(machine, memory_exit);

	/* allocate our private data */
	memdata = machine->memory_data = auto_alloc_clear(machine, memory_private);

	/* build up the list of address spaces */
	memory_init_spaces(machine);

	/* preflight the memory handlers and check banks */
	memory_init_preflight(machine);

	/* then fill in the tables */
	memory_init_populate(machine);

	/* allocate any necessary memory */
	memory_init_allocate(machine);

	/* find all the allocated pointers */
	memory_init_locate(machine);

	/* dump the final memory configuration */
	mem_dump(machine);
}


/*-------------------------------------------------
    memory_find_address_space - find an address
    space in our internal list; for faster access
    use device->space[] after device is started
-------------------------------------------------*/

const address_space *memory_find_address_space(const device_config *device, int spacenum)
{
	memory_private *memdata = device->machine->memory_data;
	const address_space *space;

	for (space = memdata->spacelist; space != NULL; space = space->next)
		if (space->cpu == device && space->spacenum == spacenum)
			return space;
	return NULL;
}



/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

/*-------------------------------------------------
    address_map_alloc - build and allocate an
    address map for a device's address space
-------------------------------------------------*/

address_map *address_map_alloc(const device_config *device, const game_driver *driver, int spacenum)
{
	const addrmap_token *internal_map;
	const addrmap_token *default_map;
	address_map *map;

	map = alloc_clear_or_die(address_map);

	/* append the internal device map (first so it takes priority) */
	internal_map = (const addrmap_token *)device_get_info_ptr(device, DEVINFO_PTR_INTERNAL_MEMORY_MAP + spacenum);
	if (internal_map != NULL)
		map_detokenize(map, driver, device->tag, internal_map);

	/* construct the standard map */
	if (device->address_map[spacenum] != NULL)
		map_detokenize(map, driver, device->tag, device->address_map[spacenum]);

	/* append the default device map (last so it can be overridden) */
	default_map = (const addrmap_token *)device_get_info_ptr(device, DEVINFO_PTR_DEFAULT_MEMORY_MAP + spacenum);
	if (default_map != NULL)
		map_detokenize(map, driver, device->tag, default_map);

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
		if (entry->read_devtag_string != NULL)
			astring_free(entry->read_devtag_string);
		if (entry->write_devtag_string != NULL)
			astring_free(entry->write_devtag_string);
		if (entry->region_string != NULL)
			astring_free(entry->region_string);
		free(entry);
	}

	/* free the map */
	free(map);
}



/***************************************************************************
    DIRECT ACCESS CONTROL
***************************************************************************/

/*-------------------------------------------------
    memory_set_decrypted_region - registers an
    address range as having a decrypted data
    pointer
-------------------------------------------------*/

void memory_set_decrypted_region(const address_space *space, offs_t addrstart, offs_t addrend, void *base)
{
	offs_t bytestart = memory_address_to_byte(space, addrstart);
	offs_t byteend = memory_address_to_byte_end(space, addrend);
	int banknum, found = FALSE;

	/* loop over banks looking for a match */
	for (banknum = 0; banknum < STATIC_COUNT; banknum++)
	{
		bank_info *bank = &space->machine->memory_data->bankdata[banknum];

		/* consider this bank if it is used for reading and matches the address space */
		if (bank->used && bank->read && bank_references_space(bank, space))
		{
			/* verify that the region fully covers the decrypted range */
			if (bank->bytestart >= bytestart && bank->byteend <= byteend)
			{
				/* set the decrypted pointer for the corresponding memory bank */
				space->machine->memory_data->bankd_ptr[banknum] = (UINT8 *)base + bank->bytestart - bytestart;
				found = TRUE;

				/* if we are executing from here, force an opcode base update */
				if (space->direct.entry == banknum)
					force_opbase_update(space);
			}

			/* fatal error if the decrypted region straddles the bank */
			else if (bank->bytestart < byteend && bank->byteend > bytestart)
				fatalerror("memory_set_decrypted_region found straddled region %08X-%08X for device '%s'", bytestart, byteend, space->cpu->tag);
		}
	}

	/* fatal error as well if we didn't find any relevant memory banks */
	if (!found)
		fatalerror("memory_set_decrypted_region unable to find matching region %08X-%08X for device '%s'", bytestart, byteend, space->cpu->tag);
}


/*-------------------------------------------------
    memory_set_direct_update_handler - register a
    handler for opcode base changes on a given
    device
-------------------------------------------------*/

direct_update_func memory_set_direct_update_handler(const address_space *space, direct_update_func function)
{
	address_space *spacerw = (address_space *)space;
	direct_update_func old = spacerw->directupdate;
	spacerw->directupdate = function;
	return old;
}


/*-------------------------------------------------
    memory_set_direct_region - called by device
    cores to update the opcode base for the given
    address
-------------------------------------------------*/

int memory_set_direct_region(const address_space *space, offs_t *byteaddress)
{
	memory_private *memdata = space->machine->memory_data;
	address_space *spacerw = (address_space *)space;
	UINT8 *base = NULL, *based = NULL;
	const handler_data *handlers;
	direct_range *range;
	offs_t maskedbits;
	offs_t overrideaddress = *byteaddress;
	UINT8 entry;

	/* allow overrides */
	if (spacerw->directupdate != NULL)
	{
		overrideaddress = (*spacerw->directupdate)(spacerw, overrideaddress, &spacerw->direct);
		if (overrideaddress == ~0)
			return TRUE;

		*byteaddress = overrideaddress;
	}

	/* remove the masked bits (we'll put them back later) */
	maskedbits = overrideaddress & ~spacerw->bytemask;

	/* find or allocate a matching range */
	range = direct_range_find(spacerw, overrideaddress, &entry);

	/* keep track of current entry */
	spacerw->direct.entry = entry;

	/* if we don't map to a bank, return FALSE */
	if (entry < STATIC_BANK1 || entry >= STATIC_RAM)
	{
		/* ensure future updates to land here as well until we get back into a bank */
		spacerw->direct.byteend = 0;
		spacerw->direct.bytestart = 1;
		if (!spacerw->debugger_access)
			logerror("Device '%s': warning - attempt to direct-map address %s in %s space\n", space->cpu->tag, core_i64_hex_format(overrideaddress, space->addrchars), space->name);
		return FALSE;
	}

	/* if no decrypted opcodes, point to the same base */
	base = memdata->bank_ptr[entry];
	based = memdata->bankd_ptr[entry];
	if (based == NULL)
		based = base;

	/* compute the adjusted base */
	handlers = spacerw->read.handlers[entry];
	spacerw->direct.bytemask = handlers->bytemask;
	spacerw->direct.raw = base - (handlers->bytestart & spacerw->direct.bytemask);
	spacerw->direct.decrypted = based - (handlers->bytestart & spacerw->direct.bytemask);
	spacerw->direct.bytestart = maskedbits | range->bytestart;
	spacerw->direct.byteend = maskedbits | range->byteend;
	return TRUE;
}


/*-------------------------------------------------
    memory_get_read_ptr - return a pointer the
    memory byte provided in the given address
    space, or NULL if it is not mapped to a bank
-------------------------------------------------*/

void *memory_get_read_ptr(const address_space *space, offs_t byteaddress)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT8 entry;

	/* perform the lookup */
	byteaddress &= space->bytemask;
	entry = space->read.table[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->read.table[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->read.handlers[entry];

	/* 8-bit case: RAM/ROM */
	if (entry >= STATIC_RAM)
		return NULL;
	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	return &(*handler->bankbaseptr)[byteoffset];
}


/*-------------------------------------------------
    memory_get_write_ptr - return a pointer the
    memory byte provided in the given address
    space, or NULL if it is not mapped to a
    writeable bank
-------------------------------------------------*/

void *memory_get_write_ptr(const address_space *space, offs_t byteaddress)
{
	const handler_data *handler;
	offs_t byteoffset;
	UINT8 entry;

	/* perform the lookup */
	byteaddress &= space->bytemask;
	entry = space->write.table[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = space->write.table[LEVEL2_INDEX(entry, byteaddress)];
	handler = space->write.handlers[entry];

	/* 8-bit case: RAM/ROM */
	if (entry >= STATIC_RAM)
		return NULL;
	byteoffset = (byteaddress - handler->bytestart) & handler->bytemask;
	return &(*handler->bankbaseptr)[byteoffset];
}



/***************************************************************************
    MEMORY BANKING
***************************************************************************/

/*-------------------------------------------------
    memory_configure_bank - configure the
    addresses for a bank
-------------------------------------------------*/

void memory_configure_bank(running_machine *machine, int banknum, int startentry, int numentries, void *base, offs_t stride)
{
	memory_private *memdata = machine->memory_data;
	bank_info *bank = &memdata->bankdata[banknum];
	int entrynum;

	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bank->used)
		fatalerror("memory_configure_bank called with invalid bank %d", banknum);
	if (bank->dynamic)
		fatalerror("memory_configure_bank called with dynamic bank %d", banknum);
	if (startentry < 0 || startentry + numentries > MAX_BANK_ENTRIES)
		fatalerror("memory_configure_bank called with out-of-range entries %d-%d", startentry, startentry + numentries - 1);
	if (!base)
		fatalerror("memory_configure_bank called NULL base");

	/* fill in the requested bank entries */
	for (entrynum = startentry; entrynum < startentry + numentries; entrynum++)
		bank->entry[entrynum] = (UINT8 *)base + (entrynum - startentry) * stride;

	/* if we have no bankptr yet, set it to the first entry */
	if (memdata->bank_ptr[banknum] == NULL)
		memdata->bank_ptr[banknum] = (UINT8 *)bank->entry[0];
}


/*-------------------------------------------------
    memory_configure_bank_decrypted - configure
    the decrypted addresses for a bank
-------------------------------------------------*/

void memory_configure_bank_decrypted(running_machine *machine, int banknum, int startentry, int numentries, void *base, offs_t stride)
{
	memory_private *memdata = machine->memory_data;
	bank_info *bank = &memdata->bankdata[banknum];
	int entrynum;

	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bank->used)
		fatalerror("memory_configure_bank called with invalid bank %d", banknum);
	if (bank->dynamic)
		fatalerror("memory_configure_bank called with dynamic bank %d", banknum);
	if (startentry < 0 || startentry + numentries > MAX_BANK_ENTRIES)
		fatalerror("memory_configure_bank called with out-of-range entries %d-%d", startentry, startentry + numentries - 1);
	if (!base)
		fatalerror("memory_configure_bank_decrypted called NULL base");

	/* fill in the requested bank entries */
	for (entrynum = startentry; entrynum < startentry + numentries; entrynum++)
		bank->entryd[entrynum] = (UINT8 *)base + (entrynum - startentry) * stride;

	/* if we have no bankptr yet, set it to the first entry */
	if (memdata->bankd_ptr[banknum] == NULL)
		memdata->bankd_ptr[banknum] = (UINT8 *)bank->entryd[0];
}


/*-------------------------------------------------
    memory_set_bank - select one pre-configured
    entry to be the new bank base
-------------------------------------------------*/

void memory_set_bank(running_machine *machine, int banknum, int entrynum)
{
	memory_private *memdata = machine->memory_data;
	bank_info *bank = &memdata->bankdata[banknum];
	bank_reference *ref;

	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bank->used)
		fatalerror("memory_set_bank called with invalid bank %d", banknum);
	if (bank->dynamic)
		fatalerror("memory_set_bank called with dynamic bank %d", banknum);
	if (entrynum < 0 || entrynum > MAX_BANK_ENTRIES)
		fatalerror("memory_set_bank called with out-of-range entry %d", entrynum);
	if (!bank->entry[entrynum])
		fatalerror("memory_set_bank called for bank %d with invalid bank entry %d", banknum, entrynum);

	/* set the base */
	bank->curentry = entrynum;
	memdata->bank_ptr[banknum] = (UINT8 *)bank->entry[entrynum];
	memdata->bankd_ptr[banknum] = (UINT8 *)bank->entryd[entrynum];

	/* invalidate all the direct references to any referenced address spaces */
	for (ref = bank->reflist; ref != NULL; ref = ref->next)
		force_opbase_update(ref->space);
}


/*-------------------------------------------------
    memory_get_bank - return the currently
    selected bank
-------------------------------------------------*/

int memory_get_bank(running_machine *machine, int banknum)
{
	memory_private *memdata = machine->memory_data;
	bank_info *bank = &memdata->bankdata[banknum];

	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bank->used)
		fatalerror("memory_get_bank called with invalid bank %d", banknum);
	if (bank->dynamic)
		fatalerror("memory_get_bank called with dynamic bank %d", banknum);
	return bank->curentry;
}


/*-------------------------------------------------
    memory_set_bankptr - set the base of a bank
-------------------------------------------------*/

void memory_set_bankptr(running_machine *machine, int banknum, void *base)
{
	memory_private *memdata = machine->memory_data;
	bank_info *bank = &memdata->bankdata[banknum];
	bank_reference *ref;

	/* validation checks */
	if (banknum < STATIC_BANK1 || banknum > MAX_EXPLICIT_BANKS || !bank->used)
		fatalerror("memory_set_bankptr called with invalid bank %d", banknum);
	if (bank->dynamic)
		fatalerror("memory_set_bankptr called with dynamic bank %d", banknum);
	if (base == NULL)
		fatalerror("memory_set_bankptr called NULL base");
	if (ALLOW_ONLY_AUTO_MALLOC_BANKS)
		validate_auto_malloc_memory(base, bank->byteend - bank->bytestart + 1);

	/* set the base */
	memdata->bank_ptr[banknum] = (UINT8 *)base;

	/* invalidate all the direct references to any referenced address spaces */
	for (ref = bank->reflist; ref != NULL; ref = ref->next)
		force_opbase_update(ref->space);
}


/*-------------------------------------------------
    memory_find_unused_bank - return the index of
    an unused bank
-------------------------------------------------*/

int memory_find_unused_bank(running_machine *machine)
{
	memory_private *memdata = machine->memory_data;
	int banknum;

	for (banknum = STATIC_BANK1; banknum <= MAX_EXPLICIT_BANKS; banknum++)
		if (!memdata->bankdata[banknum].used)
			return banknum;

	return -1;
}



/***************************************************************************
    DYNAMIC ADDRESS SPACE MAPPING
***************************************************************************/

/*-------------------------------------------------
    _memory_install_handler - install a new memory
    handler into the given address space,
    returning a pointer to the memory backing it,
    if present
-------------------------------------------------*/

void *_memory_install_handler(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, FPTR rhandler, FPTR whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler >= STATIC_COUNT || whandler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_handler()");
	if (rhandler != 0)
		space_map_range(spacerw, ROW_READ, spacerw->dbits, 0, addrstart, addrend, addrmask, addrmirror, (genf *)(FPTR)rhandler, spacerw, rhandler_name);
	if (whandler != 0)
		space_map_range(spacerw, ROW_WRITE, spacerw->dbits, 0, addrstart, addrend, addrmask, addrmirror, (genf *)(FPTR)whandler, spacerw, whandler_name);
	mem_dump(space->machine);
	return space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_handler8 - same as above but
    explicitly for 8-bit handlers
-------------------------------------------------*/

UINT8 *_memory_install_handler8(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, write8_space_func whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler != NULL)
		space_map_range(spacerw, ROW_READ, 8, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, spacerw, rhandler_name);
	if (whandler != NULL)
		space_map_range(spacerw, ROW_WRITE, 8, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, spacerw, whandler_name);
	mem_dump(space->machine);
	return (UINT8 *)space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_handler16 - same as above but
    explicitly for 16-bit handlers
-------------------------------------------------*/

UINT16 *_memory_install_handler16(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, write16_space_func whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler != NULL)
		space_map_range(spacerw, ROW_READ, 16, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, spacerw, rhandler_name);
	if (whandler != NULL)
		space_map_range(spacerw, ROW_WRITE, 16, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, spacerw, whandler_name);
	mem_dump(space->machine);
	return (UINT16 *)space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_handler32 - same as above but
    explicitly for 32-bit handlers
-------------------------------------------------*/

UINT32 *_memory_install_handler32(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, write32_space_func whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler != NULL)
		space_map_range(spacerw, ROW_READ, 32, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, spacerw, rhandler_name);
	if (whandler != NULL)
		space_map_range(spacerw, ROW_WRITE, 32, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, spacerw, whandler_name);
	mem_dump(space->machine);
	return (UINT32 *)space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_handler64 - same as above but
    explicitly for 64-bit handlers
-------------------------------------------------*/

UINT64 *_memory_install_handler64(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, write64_space_func whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler != NULL)
		space_map_range(spacerw, ROW_READ, 64, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, spacerw, rhandler_name);
	if (whandler != NULL)
		space_map_range(spacerw, ROW_WRITE, 64, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, spacerw, whandler_name);
	mem_dump(space->machine);
	return (UINT64 *)space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_device_handler - install a new
    device memory handler into the given address
    space, returning a pointer to the memory
    backing it, if present
-------------------------------------------------*/

void *_memory_install_device_handler(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, FPTR rhandler, FPTR whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler >= STATIC_COUNT || whandler >= STATIC_COUNT)
		fatalerror("fatal: can only use static banks with memory_install_device_handler()");
	if (rhandler != 0)
		space_map_range(spacerw, ROW_READ, spacerw->dbits, 0, addrstart, addrend, addrmask, addrmirror, (genf *)(FPTR)rhandler, (void *)device, rhandler_name);
	if (whandler != 0)
		space_map_range(spacerw, ROW_WRITE, spacerw->dbits, 0, addrstart, addrend, addrmask, addrmirror, (genf *)(FPTR)whandler, (void *)device, whandler_name);
	mem_dump(space->machine);
	return space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_device_handler8 - same as above
    but explicitly for 8-bit handlers
-------------------------------------------------*/

UINT8 *_memory_install_device_handler8(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, write8_device_func whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler != NULL)
		space_map_range(spacerw, ROW_READ, 8, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, (void *)device, rhandler_name);
	if (whandler != NULL)
		space_map_range(spacerw, ROW_WRITE, 8, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, (void *)device, whandler_name);
	mem_dump(space->machine);
	return (UINT8 *)space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_device_handler16 - same as
    above but explicitly for 16-bit handlers
-------------------------------------------------*/

UINT16 *_memory_install_device_handler16(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, write16_device_func whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler != NULL)
		space_map_range(spacerw, ROW_READ, 16, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, (void *)device, rhandler_name);
	if (whandler != NULL)
		space_map_range(spacerw, ROW_WRITE, 16, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, (void *)device, whandler_name);
	mem_dump(space->machine);
	return (UINT16 *)space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_device_handler32 - same as
    above but explicitly for 32-bit handlers
-------------------------------------------------*/

UINT32 *_memory_install_device_handler32(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, write32_device_func whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler != NULL)
		space_map_range(spacerw, ROW_READ, 32, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, (void *)device, rhandler_name);
	if (whandler != NULL)
		space_map_range(spacerw, ROW_WRITE, 32, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, (void *)device, whandler_name);
	mem_dump(space->machine);
	return (UINT32 *)space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    _memory_install_device_handler64 - same as
    above but explicitly for 64-bit handlers
-------------------------------------------------*/

UINT64 *_memory_install_device_handler64(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, write64_device_func whandler, const char *rhandler_name, const char *whandler_name)
{
	address_space *spacerw = (address_space *)space;
	if (rhandler != NULL)
		space_map_range(spacerw, ROW_READ, 64, 0, addrstart, addrend, addrmask, addrmirror, (genf *)rhandler, (void *)device, rhandler_name);
	if (whandler != NULL)
		space_map_range(spacerw, ROW_WRITE, 64, 0, addrstart, addrend, addrmask, addrmirror, (genf *)whandler, (void *)device, whandler_name);
	mem_dump(space->machine);
	return (UINT64 *)space_find_backing_memory(spacerw, memory_address_to_byte(spacerw, addrstart));
}


/*-------------------------------------------------
    memory_install_read_port_handler - install a
    new 8-bit input port handler into the given
    address space
-------------------------------------------------*/

void memory_install_read_port_handler(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *tag)
{
	const input_port_config *port = input_port_by_tag(space->machine->portconfig, tag);
	address_space *spacerw = (address_space *)space;
	genf *handler = NULL;

	if (port == NULL)
		fatalerror("Non-existent port referenced: '%s'\n", tag);
	switch (space->dbits)
	{
		case 8:		handler = (genf *)input_port_read8; 	break;
		case 16:	handler = (genf *)input_port_read16; 	break;
		case 32:	handler = (genf *)input_port_read32; 	break;
		case 64:	handler = (genf *)input_port_read64; 	break;
	}
	space_map_range(spacerw, ROW_READ, space->dbits, 0, addrstart, addrend, addrmask, addrmirror, handler, (void *)port, tag);
}



/*-------------------------------------------------
    memory_install_write_port_handler - install a
    new 8-bit input port handler into the given
    address space
-------------------------------------------------*/

void memory_install_write_port_handler(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *tag)
{
	const input_port_config *port = input_port_by_tag(space->machine->portconfig, tag);
	address_space *spacerw = (address_space *)space;
	genf *handler = NULL;

	if (port == NULL)
		fatalerror("Non-existent port referenced: '%s'\n", tag);
	switch (space->dbits)
	{
		case 8:		handler = (genf *)input_port_write8; 	break;
		case 16:	handler = (genf *)input_port_write16; 	break;
		case 32:	handler = (genf *)input_port_write32; 	break;
		case 64:	handler = (genf *)input_port_write64; 	break;
	}
	space_map_range(spacerw, ROW_WRITE, space->dbits, 0, addrstart, addrend, addrmask, addrmirror, handler, (void *)port, tag);
}



/***************************************************************************
    DEBUGGER HELPERS
***************************************************************************/

/*-------------------------------------------------
    memory_get_handler_string - return a string
    describing the handler at a particular offset
-------------------------------------------------*/

const char *memory_get_handler_string(const address_space *space, int read0_or_write1, offs_t byteaddress)
{
	const address_table *table = read0_or_write1 ? &space->write : &space->read;
	UINT8 entry;

	/* perform the lookup */
	byteaddress &= space->bytemask;
	entry = table->table[LEVEL1_INDEX(byteaddress)];
	if (entry >= SUBTABLE_BASE)
		entry = table->table[LEVEL2_INDEX(entry, byteaddress)];

	/* 8-bit case: RAM/ROM */
	return handler_to_string(table, entry);
}


/*-------------------------------------------------
    memory_enable_read_watchpoints - enable/disable
    read watchpoint tracking for a given address
    space
-------------------------------------------------*/

void memory_enable_read_watchpoints(const address_space *space, int enable)
{
	address_space *spacerw = (address_space *)space;
	if (enable)
		spacerw->readlookup = space->machine->memory_data->wptable;
	else
		spacerw->readlookup = spacerw->read.table;
}


/*-------------------------------------------------
    memory_enable_write_watchpoints - enable/disable
    write watchpoint tracking for a given address
    space
-------------------------------------------------*/

void memory_enable_write_watchpoints(const address_space *space, int enable)
{
	address_space *spacerw = (address_space *)space;
	if (enable)
		spacerw->writelookup = space->machine->memory_data->wptable;
	else
		spacerw->writelookup = spacerw->write.table;
}


/*-------------------------------------------------
    memory_set_debugger_access - control whether
    subsequent accesses are treated as coming from
    the debugger
-------------------------------------------------*/

void memory_set_debugger_access(const address_space *space, int debugger)
{
	address_space *spacerw = (address_space *)space;
	spacerw->debugger_access = debugger;
}


/*-------------------------------------------------
    memory_set_log_unmap - sets whether unmapped
    memory accesses should be logged or not
-------------------------------------------------*/

void memory_set_log_unmap(const address_space *space, int log)
{
	address_space *spacerw = (address_space *)space;
	spacerw->log_unmap = log;
}


/*-------------------------------------------------
    memory_get_log_unmap - gets whether unmapped
    memory accesses should be logged or not
-------------------------------------------------*/

int memory_get_log_unmap(const address_space *space)
{
	return space->log_unmap;
}


/*-------------------------------------------------
    memory_dump - dump the internal memory tables
    to the given file
-------------------------------------------------*/

void memory_dump(running_machine *machine, FILE *file)
{
	memory_private *memdata = machine->memory_data;
	const address_space *space;

	/* skip if we can't open the file */
	if (!file)
		return;

	/* loop over valid address spaces */
	for (space = memdata->spacelist; space != NULL; space = space->next)
	{
		fprintf(file, "\n\n"
		              "====================================================\n"
		              "Device '%s' %s address space read handler dump\n"
		              "====================================================\n", space->cpu->tag, space->name);
		dump_map(file, space, &space->read);

		fprintf(file, "\n\n"
		              "====================================================\n"
		              "Device '%s' %s address space write handler dump\n"
		              "====================================================\n", space->cpu->tag, space->name);
		dump_map(file, space, &space->read);
	}
}



/***************************************************************************
    INTERNAL INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    memory_init_spaces - create the address
    spaces
-------------------------------------------------*/

static void memory_init_spaces(running_machine *machine)
{
	memory_private *memdata = machine->memory_data;
	address_space **nextptr = (address_space **)&memdata->spacelist;
	const device_config *device;
	int spacenum;

	/* create a global watchpoint-filled table */
	memdata->wptable = auto_alloc_array(machine, UINT8, 1 << LEVEL1_BITS);
	memset(memdata->wptable, STATIC_WATCHPOINT, 1 << LEVEL1_BITS);

	/* loop over devices */
	for (device = machine->config->devicelist; device != NULL; device = device->next)
		for (spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
			if (device_get_addrbus_width(device, spacenum) > 0)
			{
				address_space *space = alloc_clear_or_die(address_space);
				int logbits = cpu_get_logaddr_width(device, spacenum);
				int ashift = device_get_addrbus_shift(device, spacenum);
				int abits = device_get_addrbus_width(device, spacenum);
				int dbits = device_get_databus_width(device, spacenum);
				int endianness = device_get_endianness(device);
				int accessorindex = (dbits == 8) ? 0 : (dbits == 16) ? 1 : (dbits == 32) ? 2 : 3;
				int entrynum;

				/* if logbits is 0, revert to abits */
				if (logbits == 0)
					logbits = abits;

				/* determine the address and data bits */
				space->machine = machine;
				space->cpu = device;
				space->name = address_space_names[spacenum];
				space->accessors = memory_accessors[accessorindex][(endianness == ENDIANNESS_LITTLE) ? 0 : 1];
				space->addrmask = 0xffffffffUL >> (32 - abits);
				space->bytemask = (ashift < 0) ? ((space->addrmask << -ashift) | ((1 << -ashift) - 1)) : (space->addrmask >> ashift);
				space->logaddrmask = 0xffffffffUL >> (32 - logbits);
				space->logbytemask = (ashift < 0) ? ((space->logaddrmask << -ashift) | ((1 << -ashift) - 1)) : (space->logaddrmask >> ashift);
				space->spacenum = spacenum;
				space->endianness = endianness;
				space->ashift = ashift;
				space->abits = abits;
				space->dbits = dbits;
				space->addrchars = (abits + 3) / 4;
				space->logaddrchars = (logbits + 3) / 4;
				space->log_unmap = TRUE;

				/* allocate subtable information; we malloc this manually because it will be realloc'ed */
				space->read.subtable = auto_alloc_array_clear(machine, subtable_data, SUBTABLE_COUNT);
				space->write.subtable = auto_alloc_array_clear(machine, subtable_data, SUBTABLE_COUNT);

				/* allocate the handler table */
				space->read.handlers[0] = auto_alloc_array_clear(machine, handler_data, ARRAY_LENGTH(space->read.handlers));
				space->write.handlers[0] = auto_alloc_array_clear(machine, handler_data, ARRAY_LENGTH(space->write.handlers));
				for (entrynum = 1; entrynum < ARRAY_LENGTH(space->read.handlers); entrynum++)
				{
					space->read.handlers[entrynum] = space->read.handlers[0] + entrynum;
					space->write.handlers[entrynum] = space->write.handlers[0] + entrynum;
				}

				/* init the static handlers */
				for (entrynum = 0; entrynum < ENTRY_COUNT; entrynum++)
				{
					space->read.handlers[entrynum]->handler.generic = get_static_handler(space->dbits, 0, entrynum);
					space->read.handlers[entrynum]->object = space;
					space->write.handlers[entrynum]->handler.generic = get_static_handler(space->dbits, 1, entrynum);
					space->write.handlers[entrynum]->object = space;
				}

				/* make sure we fix up the mask for the unmap and watchpoint handlers */
				space->read.handlers[STATIC_UNMAP]->bytemask = ~0;
				space->write.handlers[STATIC_UNMAP]->bytemask = ~0;
				space->read.handlers[STATIC_WATCHPOINT]->bytemask = ~0;
				space->write.handlers[STATIC_WATCHPOINT]->bytemask = ~0;

				/* allocate memory; these aren't auto-malloc'ed as we need to expand them */
				space->read.table = alloc_array_or_die(UINT8, 1 << LEVEL1_BITS);
				space->write.table = alloc_array_or_die(UINT8, 1 << LEVEL1_BITS);

				/* initialize everything to unmapped */
				memset(space->read.table, STATIC_UNMAP, 1 << LEVEL1_BITS);
				memset(space->write.table, STATIC_UNMAP, 1 << LEVEL1_BITS);

				/* initialize the lookups */
				space->readlookup = space->read.table;
				space->writelookup = space->write.table;

				/* set the direct access information base */
				space->direct.raw = space->direct.decrypted = NULL;
				space->direct.bytemask = space->bytemask;
				space->direct.bytestart = 1;
				space->direct.byteend = 0;
				space->direct.entry = STATIC_UNMAP;
				space->directupdate = NULL;

				/* link us in */
				*nextptr = space;
				nextptr = (address_space **)&space->next;
			}
}


/*-------------------------------------------------
    memory_init_preflight - verify the memory structs
    and track which banks are referenced
-------------------------------------------------*/

static void memory_init_preflight(running_machine *machine)
{
	memory_private *memdata = machine->memory_data;
	address_space *space;

	/* zap the bank data */
	memset(&memdata->bankdata, 0, sizeof(memdata->bankdata));

	/* loop over valid address spaces */
	for (space = (address_space *)memdata->spacelist; space != NULL; space = (address_space *)space->next)
	{
		int regionsize = (space->spacenum == ADDRESS_SPACE_0) ? memory_region_length(space->machine, space->cpu->tag) : 0;
		address_map_entry *entry;
		int entrynum;

		/* allocate the address map */
		space->map = address_map_alloc(space->cpu, machine->gamedrv, space->spacenum);

		/* extract global parameters specified by the map */
		space->unmap = (space->map->unmapval == 0) ? 0 : ~0;
		if (space->map->globalmask != 0)
		{
			space->addrmask = space->map->globalmask;
			space->bytemask = memory_address_to_byte_end(space, space->addrmask);
		}

		/* make a pass over the address map, adjusting for the device and getting memory pointers */
		for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
		{
			/* computed adjusted addresses first */
			entry->bytestart = entry->addrstart;
			entry->byteend = entry->addrend;
			entry->bytemirror = entry->addrmirror;
			entry->bytemask = entry->addrmask;
			adjust_addresses(space, &entry->bytestart, &entry->byteend, &entry->bytemask, &entry->bytemirror);

			/* if this is a ROM handler without a specified region, attach it to the implicit region */
			if (space->spacenum == ADDRESS_SPACE_0 && HANDLER_IS_ROM(entry->read.generic) && entry->region == NULL)
			{
				/* make sure it fits within the memory region before doing so, however */
				if (entry->byteend < regionsize)
				{
					entry->region = space->cpu->tag;
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
					fatalerror("Error: device '%s' %s space memory map entry %X-%X references non-existant region \"%s\"", space->cpu->tag, space->name, entry->addrstart, entry->addrend, entry->region);
				if (entry->rgnoffs + (entry->byteend - entry->bytestart + 1) > length)
					fatalerror("Error: device '%s' %s space memory map entry %X-%X extends beyond region \"%s\" size (%X)", space->cpu->tag, space->name, entry->addrstart, entry->addrend, entry->region, length);
			}

			/* convert any region-relative entries to their memory pointers */
			if (entry->region != NULL)
				entry->memory = memory_region(machine, entry->region) + entry->rgnoffs;

			/* assign static banks for explicitly specified entries */
			if (HANDLER_IS_BANK(entry->read.generic))
				bank_assign_static(HANDLER_TO_BANK(entry->read.generic), space, ROW_READ, entry->bytestart, entry->byteend);
			if (HANDLER_IS_BANK(entry->write.generic))
				bank_assign_static(HANDLER_TO_BANK(entry->write.generic), space, ROW_WRITE, entry->bytestart, entry->byteend);
		}

		/* now loop over all the handlers and enforce the address mask */
		/* we don't loop over map entries because the mask applies to static handlers as well */
		for (entrynum = 0; entrynum < ENTRY_COUNT; entrynum++)
		{
			space->read.handlers[entrynum]->bytemask &= space->bytemask;
			space->write.handlers[entrynum]->bytemask &= space->bytemask;
		}
	}
}


/*-------------------------------------------------
    memory_init_populate - populate the memory
    mapping tables with entries
-------------------------------------------------*/

static void memory_init_populate(running_machine *machine)
{
	memory_private *memdata = machine->memory_data;
	address_space *space;

	/* loop over valid address spaces */
	for (space = (address_space *)memdata->spacelist; space != NULL; space = (address_space *)space->next)
		if (space->map != NULL)
		{
			const address_map_entry *last_entry = NULL;

			/* install the handlers, using the original, unadjusted memory map */
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
					const input_port_config *port = input_port_by_tag(machine->portconfig, entry->read_porttag);
					int bits = (entry->read_bits == 0) ? space->dbits : entry->read_bits;
					genf *handler = NULL;

					if (port == NULL)
						fatalerror("Non-existent port referenced: '%s'\n", entry->read_porttag);
					switch (bits)
					{
						case 8:		handler = (genf *)input_port_read8; 	break;
						case 16:	handler = (genf *)input_port_read16; 	break;
						case 32:	handler = (genf *)input_port_read32; 	break;
						case 64:	handler = (genf *)input_port_read64; 	break;
					}
					space_map_range_private(space, ROW_READ, bits, entry->read_mask, entry->addrstart, entry->addrend, entry->addrmask, entry->addrmirror, handler, (void *)port, entry->read_porttag);
				}

				/* if we have a write port tag, look it up */
				if (entry->write_porttag != NULL)
				{
					const input_port_config *port = input_port_by_tag(machine->portconfig, entry->write_porttag);
					int bits = (entry->write_bits == 0) ? space->dbits : entry->write_bits;
					genf *handler = NULL;

					if (port == NULL)
						fatalerror("Non-existent port referenced: '%s'\n", entry->write_porttag);
					switch (bits)
					{
						case 8:		handler = (genf *)input_port_write8; 	break;
						case 16:	handler = (genf *)input_port_write16; 	break;
						case 32:	handler = (genf *)input_port_write32; 	break;
						case 64:	handler = (genf *)input_port_write64; 	break;
					}
					space_map_range_private(space, ROW_WRITE, bits, entry->write_mask, entry->addrstart, entry->addrend, entry->addrmask, entry->addrmirror, handler, (void *)port, entry->write_porttag);
				}

				/* install the read handler if present */
				if (rhandler.generic != NULL)
				{
					int bits = (entry->read_bits == 0) ? space->dbits : entry->read_bits;
					void *object = space;
					if (entry->read_devtag != NULL)
					{
						object = (void *)device_list_find_by_tag(machine->config->devicelist, entry->read_devtag);
						if (object == NULL)
							fatalerror("Unidentified object in memory map: tag=%s\n", entry->read_devtag);
					}
					space_map_range_private(space, ROW_READ, bits, entry->read_mask, entry->addrstart, entry->addrend, entry->addrmask, entry->addrmirror, rhandler.generic, object, entry->read_name);
				}

				/* install the write handler if present */
				if (whandler.generic != NULL)
				{
					int bits = (entry->write_bits == 0) ? space->dbits : entry->write_bits;
					void *object = space;
					if (entry->write_devtag != NULL)
					{
						object = (void *)device_list_find_by_tag(machine->config->devicelist, entry->write_devtag);
						if (object == NULL)
							fatalerror("Unidentified object in memory map: tag=%s\n", entry->write_devtag);
					}
					space_map_range_private(space, ROW_WRITE, bits, entry->write_mask, entry->addrstart, entry->addrend, entry->addrmask, entry->addrmirror, whandler.generic, object, entry->write_name);
				}
			}
		}
}


/*-------------------------------------------------
    memory_init_allocate - allocate memory for
    device address spaces
-------------------------------------------------*/

static void memory_init_allocate(running_machine *machine)
{
	memory_private *memdata = machine->memory_data;
	address_space *space;

	/* loop over valid address spaces */
	for (space = (address_space *)memdata->spacelist; space != NULL; space = (address_space *)space->next)
	{
		address_map_entry *unassigned = NULL;
		address_map_entry *entry;
		memory_block *prev_memblock_head = memdata->memory_block_list;
		memory_block *memblock;

		/* make a first pass over the memory map and track blocks with hardcoded pointers */
		/* we do this to make sure they are found by space_find_backing_memory first */
		for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
			if (entry->memory != NULL)
				block_allocate(space, entry->bytestart, entry->byteend, entry->memory);

		/* loop over all blocks just allocated and assign pointers from them */
		for (memblock = memdata->memory_block_list; memblock != prev_memblock_head; memblock = memblock->next)
			unassigned = block_assign_intersecting(space, memblock->bytestart, memblock->byteend, memblock->data);

		/* if we don't have an unassigned pointer yet, try to find one */
		if (unassigned == NULL)
			unassigned = block_assign_intersecting(space, ~0, 0, NULL);

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
					if (entry->memory == NULL && entry != unassigned && space_needs_backing_store(space, entry))
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
			block = block_allocate(space, curbytestart, curbyteend, NULL);

			/* assign memory that intersected the new block */
			unassigned = block_assign_intersecting(space, curbytestart, curbyteend, (UINT8 *)block);
		}
	}
}


/*-------------------------------------------------
    memory_init_locate - find all the requested
    pointers into the final allocated memory
-------------------------------------------------*/

static void memory_init_locate(running_machine *machine)
{
	memory_private *memdata = machine->memory_data;
	address_space *space;
	int banknum;

	/* loop over valid address spaces */
	for (space = (address_space *)memdata->spacelist; space != NULL; space = (address_space *)space->next)
	{
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
	{
		bank_info *bank = &memdata->bankdata[banknum];
		if (bank->used)
		{
			address_map_entry *entry;
			bank_reference *ref;
			int foundit = FALSE;

			/* set the initial bank pointer */
			for (ref = bank->reflist; !foundit && ref != NULL; ref = ref->next)
				for (entry = ref->space->map->entrylist; entry != NULL; entry = entry->next)
					if (entry->bytestart == bank->bytestart)
					{
						memdata->bank_ptr[banknum] = (UINT8 *)entry->memory;
						foundit = TRUE;
		 				VPRINTF(("assigned bank %d pointer to memory from range %08X-%08X [%p]\n", banknum, entry->addrstart, entry->addrend, entry->memory));
						break;
					}

			/* if the entry was set ahead of time, override the automatically found pointer */
			if (!bank->dynamic && bank->curentry != MAX_BANK_ENTRIES)
				memdata->bank_ptr[banknum] = (UINT8 *)bank->entry[bank->curentry];
		}
	}

	/* request a callback to fix up the banks when done */
	state_save_register_postload(machine, bank_reattach, NULL);
}


/*-------------------------------------------------
    memory_exit - free memory
-------------------------------------------------*/

static void memory_exit(running_machine *machine)
{
	memory_private *memdata = machine->memory_data;
	address_space *space, *nextspace;
	int banknum;

	/* free the memory blocks */
	while (memdata->memory_block_list != NULL)
	{
		memory_block *block = memdata->memory_block_list;
		memdata->memory_block_list = block->next;
		free(block);
	}

	/* free all the bank references */
	for (banknum = 0; banknum < STATIC_COUNT; banknum++)
	{
		bank_info *bank = &memdata->bankdata[banknum];
		while (bank->reflist != NULL)
		{
			bank_reference *ref = bank->reflist;
			bank->reflist = ref->next;
			free(ref);
		}
	}

	/* free all the address spaces and tables */
	for (space = (address_space *)memdata->spacelist; space != NULL; space = nextspace)
	{
		int entry;

		nextspace = (address_space *)space->next;

		/* free all direct ranges */
		for (entry = 0; entry < ARRAY_LENGTH(space->direct.rangelist); entry++)
			while (space->direct.rangelist[entry] != NULL)
			{
				direct_range *range = space->direct.rangelist[entry];
				space->direct.rangelist[entry] = range->next;
				free(range);
			}

		/* free the free list of direct ranges */
		while (space->direct.freerangelist != NULL)
		{
			direct_range *range = space->direct.freerangelist;
			space->direct.freerangelist = range->next;
			free(range);
		}

		/* free the address map and tables */
		if (space->map != NULL)
			address_map_free(space->map);
		if (space->read.table != NULL)
			free(space->read.table);
		if (space->write.table != NULL)
			free(space->write.table);

		free(space);
	}
}



/***************************************************************************
    ADDRESS MAP HELPERS
***************************************************************************/

/*-------------------------------------------------
    map_detokenize - detokenize an array of
    address map tokens
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

static void map_detokenize(address_map *map, const game_driver *driver, const char *devtag, const addrmap_token *tokens)
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
				map_detokenize(map, driver, devtag, TOKEN_GET_PTR(tokens, tokenptr));
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
				entry = *entryptr = alloc_clear_or_die(address_map_entry);
				entryptr = &entry->next;
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
				if (entry->read_devtag_string == NULL)
					entry->read_devtag_string = astring_alloc();
				entry->read_devtag = device_inherit_tag(entry->read_devtag_string, devtag, TOKEN_GET_STRING(tokens));
				break;

			case ADDRMAP_TOKEN_DEVICE_WRITE:
				check_entry_handler(write);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT32_UNPACK3(tokens, entrytype, 8, entry->write_bits, 8, entry->write_mask, 8);
				entry->write = TOKEN_GET_PTR(tokens, write);
				entry->write_name = TOKEN_GET_STRING(tokens);
				if (entry->write_devtag_string == NULL)
					entry->write_devtag_string = astring_alloc();
				entry->write_devtag = device_inherit_tag(entry->write_devtag_string, devtag, TOKEN_GET_STRING(tokens));
				break;

			case ADDRMAP_TOKEN_READ_PORT:
				check_entry_field(read_porttag);
				entry->read_porttag = TOKEN_GET_STRING(tokens);
				break;

			case ADDRMAP_TOKEN_WRITE_PORT:
				check_entry_field(write_porttag);
				entry->write_porttag = TOKEN_GET_STRING(tokens);
				break;

			case ADDRMAP_TOKEN_REGION:
				check_entry_field(region);
				TOKEN_UNGET_UINT32(tokens);
				TOKEN_GET_UINT64_UNPACK2(tokens, entrytype, 8, entry->rgnoffs, 32);
				if (entry->region_string == NULL)
					entry->region_string = astring_alloc();
				entry->region = device_inherit_tag(entry->region_string, devtag, TOKEN_GET_STRING(tokens));
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
    MEMORY MAPPING HELPERS
***************************************************************************/

/*-------------------------------------------------
    space_map_range_private - wrapper for
    space_map_range which is used at
    initialization time and converts RAM/ROM
    banks to dynamically assigned banks
-------------------------------------------------*/

static void space_map_range_private(address_space *space, read_or_write readorwrite, int handlerbits, int handlerunitmask, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, genf *handler, void *object, const char *handler_name)
{
	/* translate ROM to RAM/UNMAP here */
	if (HANDLER_IS_ROM(handler))
		handler = (readorwrite == ROW_WRITE) ? (genf *)STATIC_UNMAP : (genf *)SMH_RAM;

	/* assign banks for RAM/ROM areas */
	if (HANDLER_IS_RAM(handler))
	{
		memory_private *memdata = space->machine->memory_data;
		offs_t bytestart = addrstart;
		offs_t byteend = addrend;
		offs_t bytemask = addrmask;
		offs_t bytemirror = addrmirror;

		/* adjust the incoming addresses (temporarily) */
		adjust_addresses(space, &bytestart, &byteend, &bytemask, &bytemirror);

		/* assign a bank to the adjusted addresses */
		handler = (genf *)bank_assign_dynamic(space, readorwrite, bytestart, byteend);
		if (memdata->bank_ptr[HANDLER_TO_BANK(handler)] == NULL)
			memdata->bank_ptr[HANDLER_TO_BANK(handler)] = (UINT8 *)space_find_backing_memory(space, bytestart);
	}

	/* then do a normal installation */
	space_map_range(space, readorwrite, handlerbits, handlerunitmask, addrstart, addrend, addrmask, addrmirror, handler, object, handler_name);
}


/*-------------------------------------------------
    space_map_range - maps a range of addresses
    to the specified handler within an address
    space
-------------------------------------------------*/

static void space_map_range(address_space *space, read_or_write readorwrite, int handlerbits, int handlerunitmask, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, genf *handler, void *object, const char *handler_name)
{
	address_table *tabledata = (readorwrite == ROW_WRITE) ? &space->write : &space->read;
	int reset_write = (space->writelookup == space->write.table);
	int reset_read = (space->readlookup == space->read.table);
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
		bank_assign_static(HANDLER_TO_BANK(handler), space, readorwrite, bytestart, byteend);

	/* get the final handler index */
	entry = table_assign_handler(space, tabledata->handlers, object, handler, handler_name, bytestart, byteend, bytemask);

	/* fix up the handler if a stub is required */
	if (handlerbits != space->dbits)
		table_compute_subhandler(tabledata->handlers, entry, readorwrite, space->dbits, space->endianness, handlerbits, handlerunitmask);

	/* populate it */
	table_populate_range_mirrored(space, tabledata, bytestart, byteend, bytemirror, entry);

	/* reset read/write pointers if necessary (could have moved due to realloc) */
	if (reset_write)
		space->writelookup = space->write.table;
	if (reset_read)
		space->readlookup = space->read.table;

	/* recompute any direct access on this space if it is a read modification */
	if (readorwrite == ROW_READ && entry == space->direct.entry)
	{
		space->direct.entry = STATIC_UNMAP;
		space->direct.bytestart = 1;
		space->direct.byteend = 0;
	}
}


/*-------------------------------------------------
    space_find_backing_memory - return a pointer to
    the base of RAM associated with the given
    device and offset
-------------------------------------------------*/

static void *space_find_backing_memory(const address_space *space, offs_t byteaddress)
{
	memory_private *memdata = space->machine->memory_data;
	address_map_entry *entry;
	memory_block *block;

	VPRINTF(("space_find_backing_memory('%s',%s,%08X) -> ", space->cpu->tag, space->name, byteaddress));

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
	for (block = memdata->memory_block_list; block != NULL; block = block->next)
		if (block->space == space && block->bytestart <= byteaddress && block->byteend > byteaddress)
		{
			VPRINTF(("found in allocated memory block %08X-%08X [%p]\n", block->bytestart, block->byteend, block->data + (byteaddress - block->bytestart)));
			return block->data + byteaddress - block->bytestart;
		}

	VPRINTF(("did not find\n"));
	return NULL;
}


/*-------------------------------------------------
    space_needs_backing_store - return whether a
    given memory map entry implies the need of
    allocating and registering memory
-------------------------------------------------*/

static int space_needs_backing_store(const address_space *space, const address_map_entry *entry)
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
			(handler != STATIC_ROM || space->spacenum != ADDRESS_SPACE_0 || entry->addrstart >= memory_region_length(space->machine, space->cpu->tag)) &&
			handler != STATIC_NOP &&
			handler != STATIC_UNMAP)
			return TRUE;
	}

	return FALSE;
}



/***************************************************************************
    BANKING HELPERS
***************************************************************************/

/*-------------------------------------------------
    bank_assign_static - assign and tag a static
    bank
-------------------------------------------------*/

static void bank_assign_static(int banknum, const address_space *space, read_or_write readorwrite, offs_t bytestart, offs_t byteend)
{
	bank_info *bank = &space->machine->memory_data->bankdata[banknum];

	/* if we're not yet used, fill in the data */
	if (!bank->used)
	{
		/* if we're allowed to, wire up state saving for the entry */
		if (state_save_registration_allowed(space->machine))
			state_save_register_item(space->machine, "memory", NULL, banknum, bank->curentry);

		/* fill in information about the bank */
		bank->used = TRUE;
		bank->dynamic = FALSE;
		add_bank_reference(bank, space);
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

static genf *bank_assign_dynamic(const address_space *space, read_or_write readorwrite, offs_t bytestart, offs_t byteend)
{
	int banknum;

	/* loop over banks, searching for an exact match or an empty */
	for (banknum = MAX_BANKS; banknum >= 1; banknum--)
	{
		bank_info *bank = &space->machine->memory_data->bankdata[banknum];
		if (!bank->used || (bank->dynamic && bank_references_space(bank, space) && bank->bytestart == bytestart))
		{
			bank->used = TRUE;
			bank->dynamic = TRUE;
			add_bank_reference(bank, space);
			bank->bytestart = bytestart;
			bank->byteend = byteend;
			VPRINTF(("Assigned bank %d to '%s',%s,%08X\n", banknum, space->cpu->tag, space->name, bytestart));
			return BANK_TO_HANDLER(banknum);
		}
	}

	/* if we got here, we failed */
	fatalerror("Device '%s': ran out of banks for RAM/ROM regions!", space->cpu->tag);
	return NULL;
}


/*-------------------------------------------------
    bank_reattach - reconnect banks after a load
-------------------------------------------------*/

static STATE_POSTLOAD( bank_reattach )
{
	memory_private *memdata = machine->memory_data;
	int banknum;

	/* once this is done, find the starting bases for the banks */
	for (banknum = 1; banknum <= MAX_BANKS; banknum++)
	{
		bank_info *bank = &memdata->bankdata[banknum];
		if (bank->used && !bank->dynamic)
		{
			/* if this entry has a changed entry, set the appropriate pointer */
			if (bank->curentry != MAX_BANK_ENTRIES)
				memdata->bank_ptr[banknum] = (UINT8 *)bank->entry[bank->curentry];
		}
	}
}



/***************************************************************************
    TABLE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    table_assign_handler - finds the index of a
    handler, or allocates a new one as necessary
-------------------------------------------------*/

static UINT8 table_assign_handler(const address_space *space, handler_data **table, void *object, genf *handler, const char *handler_name, offs_t bytestart, offs_t byteend, offs_t bytemask)
{
	int entry;

	/* all static handlers are hardcoded */
	if (HANDLER_IS_STATIC(handler))
	{
		entry = (FPTR)handler;

		/* if it is a bank, copy in the relevant information */
		if (HANDLER_IS_BANK(handler))
		{
			handler_data *hdata = table[entry];
			hdata->bytestart = bytestart;
			hdata->byteend = byteend;
			hdata->bytemask = bytemask;
			hdata->bankbaseptr = &space->machine->memory_data->bank_ptr[entry];
			hdata->name = handler_name;
		}
		return entry;
	}

	/* otherwise, we have to search */
	for (entry = STATIC_COUNT; entry < SUBTABLE_BASE; entry++)
	{
		handler_data *hdata = table[entry];

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

static void table_compute_subhandler(handler_data **table, UINT8 entry, read_or_write readorwrite, int spacebits, int spaceendian, int handlerbits, int handlerunitmask)
{
	int maxunits = spacebits / handlerbits;
	handler_data *hdata = table[entry];
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
	if (spaceendian == ENDIANNESS_LITTLE)
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

static void table_populate_range(address_table *tabledata, offs_t bytestart, offs_t byteend, UINT8 handler)
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

static void table_populate_range_mirrored(address_space *space, address_table *tabledata, offs_t bytestart, offs_t byteend, offs_t bytemirror, UINT8 handler)
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

		/* invalidate any intersecting cached ranges */
		for (lmirrorcount = 0; lmirrorcount < (1 << lmirrorbits); lmirrorcount++)
		{
			/* compute the base of this mirror */
			offs_t lmirrorbase = hmirrorbase;
			for (i = 0; i < lmirrorbits; i++)
				if (lmirrorcount & (1 << i))
					lmirrorbase |= lmirrorbit[i];
			direct_range_remove_intersecting(space, bytestart + lmirrorbase, byteend + lmirrorbase);
		}

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
    table_derive_range - look up the entry for
    a memory range, and then compute the extent
    of that range based on the lookup tables
-------------------------------------------------*/

static UINT8 table_derive_range(const address_table *table, offs_t byteaddress, offs_t *bytestart, offs_t *byteend)
{
	UINT32 curentry, entry, curl1entry, l1entry;
	const handler_data *handler;
	offs_t minscan, maxscan;

	/* look up the initial address to get the entry we care about */
	entry = l1entry = table->table[LEVEL1_INDEX(byteaddress)];
	if (l1entry >= SUBTABLE_BASE)
		entry = table->table[LEVEL2_INDEX(l1entry, byteaddress)];
	handler = table->handlers[entry];

	/* use the bytemask of the entry to set minimum and maximum bounds */
	minscan = handler->bytestart | ((byteaddress - handler->bytestart) & ~handler->bytemask);
	maxscan = handler->byteend | ((byteaddress - handler->bytestart) & ~handler->bytemask);

	/* first scan backwards to find the start address */
	curl1entry = l1entry;
	curentry = entry;
	*bytestart = byteaddress;
	while (1)
	{
		/* if we need to scan the subtable, do it */
		if (curentry != curl1entry)
		{
			UINT32 minindex = LEVEL2_INDEX(curl1entry, 0);
			UINT32 index;

			/* scan backwards from the current address, until the previous entry doesn't match */
			for (index = LEVEL2_INDEX(curl1entry, *bytestart); index > minindex; index--, *bytestart -= 1)
				if (table->table[index - 1] != entry)
					break;

			/* if we didn't hit the beginning, then we're finished scanning */
			if (index != minindex)
				break;
		}

		/* move to the beginning of this L1 entry; stop at the minimum address */
		*bytestart &= ~((1 << LEVEL2_BITS) - 1);
		if (*bytestart <= minscan)
			break;

		/* look up the entry of the byte at the end of the previous L1 entry; if it doesn't match, stop */
		curentry = curl1entry = table->table[LEVEL1_INDEX(*bytestart - 1)];
		if (curl1entry >= SUBTABLE_BASE)
			curentry = table->table[LEVEL2_INDEX(curl1entry, *bytestart - 1)];
		if (curentry != entry)
			break;

		/* move into the previous entry and resume searching */
		*bytestart -= 1;
	}

	/* then scan forwards to find the end address */
	curl1entry = l1entry;
	curentry = entry;
	*byteend = byteaddress;
	while (1)
	{
		/* if we need to scan the subtable, do it */
		if (curentry != curl1entry)
		{
			UINT32 maxindex = LEVEL2_INDEX(curl1entry, ~0);
			UINT32 index;

			/* scan forwards from the current address, until the next entry doesn't match */
			for (index = LEVEL2_INDEX(curl1entry, *byteend); index < maxindex; index++, *byteend += 1)
				if (table->table[index + 1] != entry)
					break;

			/* if we didn't hit the end, then we're finished scanning */
			if (index != maxindex)
				break;
		}

		/* move to the end of this L1 entry; stop at the maximum address */
		*byteend |= (1 << LEVEL2_BITS) - 1;
		if (*byteend >= maxscan)
			break;

		/* look up the entry of the byte at the start of the next L1 entry; if it doesn't match, stop */
		curentry = curl1entry = table->table[LEVEL1_INDEX(*byteend + 1)];
		if (curl1entry >= SUBTABLE_BASE)
			curentry = table->table[LEVEL2_INDEX(curl1entry, *byteend + 1)];
		if (curentry != entry)
			break;

		/* move into the next entry and resume searching */
		*byteend += 1;
	}

	return entry;
}



/***************************************************************************
    SUBTABLE MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    subtable_alloc - allocate a fresh subtable
    and set its usecount to 1
-------------------------------------------------*/

static UINT8 subtable_alloc(address_table *tabledata)
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
					tabledata->table = (UINT8 *)realloc(tabledata->table, (1 << LEVEL1_BITS) + (tabledata->subtable_alloc << LEVEL2_BITS));
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

static void subtable_realloc(address_table *tabledata, UINT8 subentry)
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

static int subtable_merge(address_table *tabledata)
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

static void subtable_release(address_table *tabledata, UINT8 subentry)
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

static UINT8 *subtable_open(address_table *tabledata, offs_t l1index)
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

static void subtable_close(address_table *tabledata, offs_t l1index)
{
	/* defer any merging until we run out of tables */
}



/***************************************************************************
    DIRECT MEMORY RANGES
***************************************************************************/

/*-------------------------------------------------
    direct_range_find - find a byte address in
    a range
-------------------------------------------------*/

static direct_range *direct_range_find(address_space *space, offs_t byteaddress, UINT8 *entry)
{
	direct_range **rangelistptr;
	direct_range **rangeptr;
	direct_range *range;

	/* determine which entry */
	byteaddress &= space->bytemask;
	*entry = space->read.table[LEVEL1_INDEX(byteaddress)];
	if (*entry >= SUBTABLE_BASE)
		*entry = space->read.table[LEVEL2_INDEX(*entry, byteaddress)];
	rangelistptr = &space->direct.rangelist[*entry];

	/* scan our table */
	for (rangeptr = rangelistptr; *rangeptr != NULL; rangeptr = &(*rangeptr)->next)
		if (byteaddress >= (*rangeptr)->bytestart && byteaddress <= (*rangeptr)->byteend)
		{
			/* found a match; move us to the head of the list if we're not already there */
			range = *rangeptr;
			if (range != *rangelistptr)
			{
				*rangeptr = range->next;
				range->next = *rangelistptr;
				*rangelistptr = range;
			}
			return range;
		}

	/* didn't find out; allocate a new one */
	range = space->direct.freerangelist;
	if (range != NULL)
		space->direct.freerangelist = range->next;
	else
		range = alloc_or_die(direct_range);

	/* fill in the range */
	table_derive_range(&space->read, byteaddress, &range->bytestart, &range->byteend);
	range->next = *rangelistptr;
	*rangelistptr = range;

	return range;
}


/*-------------------------------------------------
    direct_range_remove_intersecting - remove
    all cached ranges that intersect the given
    address range
-------------------------------------------------*/

static void direct_range_remove_intersecting(address_space *space, offs_t bytestart, offs_t byteend)
{
    int entry;

    /* loop over all entries */
    for (entry = 0; entry < ARRAY_LENGTH(space->read.handlers); entry++)
    {
        direct_range **rangeptr, **nextrangeptr;

        /* loop over all ranges in this entry's list */
        for (nextrangeptr = rangeptr = &space->direct.rangelist[entry]; *rangeptr != NULL; rangeptr = nextrangeptr)
        {
        	/* if we intersect, remove and add to the free range list */
            if (bytestart <= (*rangeptr)->byteend && byteend >= (*rangeptr)->bytestart)
            {
                direct_range *range = *rangeptr;
                *rangeptr = range->next;
                range->next = space->direct.freerangelist;
                space->direct.freerangelist = range;
            }

            /* otherwise advance to the next in the list */
            else
                nextrangeptr = &(*rangeptr)->next;
        }
    }
}



/***************************************************************************
    MEMORY BLOCK ALLOCATION
***************************************************************************/

/*-------------------------------------------------
    block_allocate - allocate a single
    memory block of data
-------------------------------------------------*/

static void *block_allocate(const address_space *space, offs_t bytestart, offs_t byteend, void *memory)
{
	memory_private *memdata = space->machine->memory_data;
	int allocatemem = (memory == NULL);
	memory_block *block;
	size_t bytestoalloc;
	const char *region;

	VPRINTF(("block_allocate('%s',%s,%08X,%08X,%p)\n", space->cpu->tag, space->name, bytestart, byteend, memory));

	/* determine how much memory to allocate for this */
	bytestoalloc = sizeof(*block);
	if (allocatemem)
		bytestoalloc += byteend - bytestart + 1;

	/* allocate and clear the memory */
	block = (memory_block *)alloc_array_clear_or_die(UINT8, bytestoalloc);
	if (allocatemem)
		memory = block + 1;

	/* register for saving, but only if we're not part of a memory region */
	for (region = memory_region_next(space->machine, NULL); region != NULL; region = memory_region_next(space->machine, region))
	{
		UINT8 *region_base = memory_region(space->machine, region);
		UINT32 region_length = memory_region_length(space->machine, region);
		if (region_base != NULL && region_length != 0 && (UINT8 *)memory >= region_base && ((UINT8 *)memory + (byteend - bytestart + 1)) < region_base + region_length)
		{
			VPRINTF(("skipping save of this memory block as it is covered by a memory region\n"));
			break;
		}
	}

	/* if we didn't find a match, register */
	if (region == NULL)
	{
		int bytes_per_element = space->dbits/8;
		char name[256];

		sprintf(name, "%08x-%08x", bytestart, byteend);
		state_save_register_memory(space->machine, "memory", space->cpu->tag, space->spacenum, name, memory, bytes_per_element, (UINT32)(byteend - bytestart + 1) / bytes_per_element);
	}

	/* fill in the tracking block */
	block->space = space;
	block->isallocated = allocatemem;
	block->bytestart = bytestart;
	block->byteend = byteend;
	block->data = (UINT8 *)memory;

	/* attach us to the head of the list */
	block->next = memdata->memory_block_list;
	memdata->memory_block_list = block;

	return memory;
}


/*-------------------------------------------------
    block_assign_intersecting - find all
    intersecting blocks and assign their pointers
-------------------------------------------------*/

static address_map_entry *block_assign_intersecting(address_space *space, offs_t bytestart, offs_t byteend, UINT8 *base)
{
	memory_private *memdata = space->machine->memory_data;
	address_map_entry *entry, *unassigned = NULL;

	/* loop over the adjusted map and assign memory to any blocks we can */
	for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
	{
		/* if we haven't assigned this block yet, do it against the last block */
		if (entry->memory == NULL)
		{
			/* inherit shared pointers first */
			if (entry->share != 0 && memdata->shared_ptr[entry->share] != NULL)
			{
				entry->memory = memdata->shared_ptr[entry->share];
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
		if (entry->memory != NULL && entry->share && memdata->shared_ptr[entry->share] == NULL)
			memdata->shared_ptr[entry->share] = entry->memory;

		/* keep track of the first unassigned entry */
		if (entry->memory == NULL && unassigned == NULL && space_needs_backing_store(space, entry))
			unassigned = entry;
	}

	return unassigned;
}



/***************************************************************************
    INTERNAL HANDLERS
***************************************************************************/

/*-------------------------------------------------
    unmapped memory handlers
-------------------------------------------------*/

static READ8_HANDLER( unmap_read8 )
{
	if (space->log_unmap && !space->debugger_access) logerror("%s: unmapped %s memory byte read from %s\n", cpuexec_describe_context(space->machine), space->name, core_i64_hex_format(memory_byte_to_address(space, offset), space->addrchars));
	return space->unmap;
}
static READ16_HANDLER( unmap_read16 )
{
	if (space->log_unmap && !space->debugger_access) logerror("%s: unmapped %s memory word read from %s & %04X\n", cpuexec_describe_context(space->machine), space->name, core_i64_hex_format(memory_byte_to_address(space, offset*2), space->addrchars), mem_mask);
	return space->unmap;
}
static READ32_HANDLER( unmap_read32 )
{
	if (space->log_unmap && !space->debugger_access) logerror("%s: unmapped %s memory dword read from %s & %08X\n", cpuexec_describe_context(space->machine), space->name, core_i64_hex_format(memory_byte_to_address(space, offset*4), space->addrchars), mem_mask);
	return space->unmap;
}
static READ64_HANDLER( unmap_read64 )
{
	if (space->log_unmap && !space->debugger_access) logerror("%s: unmapped %s memory qword read from %s & %s\n", cpuexec_describe_context(space->machine), space->name, core_i64_hex_format(memory_byte_to_address(space, offset*8), space->addrchars), core_i64_hex_format(mem_mask, 16));
	return space->unmap;
}

static WRITE8_HANDLER( unmap_write8 )
{
	if (space->log_unmap && !space->debugger_access) logerror("%s: unmapped %s memory byte write to %s = %02X\n", cpuexec_describe_context(space->machine), space->name, core_i64_hex_format(memory_byte_to_address(space, offset), space->addrchars), data);
}
static WRITE16_HANDLER( unmap_write16 )
{
	if (space->log_unmap && !space->debugger_access) logerror("%s: unmapped %s memory word write to %s = %04X & %04X\n", cpuexec_describe_context(space->machine), space->name, core_i64_hex_format(memory_byte_to_address(space, offset*2), space->addrchars), data, mem_mask);
}
static WRITE32_HANDLER( unmap_write32 )
{
	if (space->log_unmap && !space->debugger_access) logerror("%s: unmapped %s memory dword write to %s = %08X & %08X\n", cpuexec_describe_context(space->machine), space->name, core_i64_hex_format(memory_byte_to_address(space, offset*4), space->addrchars), data, mem_mask);
}
static WRITE64_HANDLER( unmap_write64 )
{
	if (space->log_unmap && !space->debugger_access) logerror("%s: unmapped %s memory qword write to %s = %s & %s\n", cpuexec_describe_context(space->machine), space->name, core_i64_hex_format(memory_byte_to_address(space, offset*8), space->addrchars), core_i64_hex_format(data, 16), core_i64_hex_format(mem_mask, 16));
}


/*-------------------------------------------------
    no-op memory handlers
-------------------------------------------------*/

static READ8_HANDLER( nop_read8 )      { return space->unmap; }
static READ16_HANDLER( nop_read16 )    { return space->unmap; }
static READ32_HANDLER( nop_read32 )    { return space->unmap; }
static READ64_HANDLER( nop_read64 )    { return space->unmap; }

static WRITE8_HANDLER( nop_write8 )    {  }
static WRITE16_HANDLER( nop_write16 )  {  }
static WRITE32_HANDLER( nop_write32 )  {  }
static WRITE64_HANDLER( nop_write64 )  {  }


/*-------------------------------------------------
    watchpoint memory handlers
-------------------------------------------------*/

static READ8_HANDLER( watchpoint_read8 )
{
	address_space *spacerw = (address_space *)space;
	UINT8 *oldtable = spacerw->readlookup;
	UINT8 result;

	debug_cpu_memory_read_hook(spacerw, offset, 0xff);
	spacerw->readlookup = space->read.table;
	result = read_byte_generic(spacerw, offset);
	spacerw->readlookup = oldtable;
	return result;
}

static READ16_HANDLER( watchpoint_read16 )
{
	address_space *spacerw = (address_space *)space;
	UINT8 *oldtable = spacerw->readlookup;
	UINT16 result;

	debug_cpu_memory_read_hook(spacerw, offset << 1, mem_mask);
	spacerw->readlookup = spacerw->read.table;
	result = read_word_generic(spacerw, offset << 1, mem_mask);
	spacerw->readlookup = oldtable;
	return result;
}

static READ32_HANDLER( watchpoint_read32 )
{
	address_space *spacerw = (address_space *)space;
	UINT8 *oldtable = spacerw->readlookup;
	UINT32 result;

	debug_cpu_memory_read_hook(spacerw, offset << 2, mem_mask);
	spacerw->readlookup = spacerw->read.table;
	result = read_dword_generic(spacerw, offset << 2, mem_mask);
	spacerw->readlookup = oldtable;
	return result;
}

static READ64_HANDLER( watchpoint_read64 )
{
	address_space *spacerw = (address_space *)space;
	UINT8 *oldtable = spacerw->readlookup;
	UINT64 result;

	debug_cpu_memory_read_hook(spacerw, offset << 3, mem_mask);
	spacerw->readlookup = spacerw->read.table;
	result = read_qword_generic(spacerw, offset << 3, mem_mask);
	spacerw->readlookup = oldtable;
	return result;
}

static WRITE8_HANDLER( watchpoint_write8 )
{
	address_space *spacerw = (address_space *)space;
	UINT8 *oldtable = spacerw->writelookup;

	debug_cpu_memory_write_hook(spacerw, offset, data, 0xff);
	spacerw->writelookup = spacerw->write.table;
	write_byte_generic(spacerw, offset, data);
	spacerw->writelookup = oldtable;
}

static WRITE16_HANDLER( watchpoint_write16 )
{
	address_space *spacerw = (address_space *)space;
	UINT8 *oldtable = spacerw->writelookup;

	debug_cpu_memory_write_hook(spacerw, offset << 1, data, mem_mask);
	spacerw->writelookup = spacerw->write.table;
	write_word_generic(spacerw, offset << 1, data, mem_mask);
	spacerw->writelookup = oldtable;
}

static WRITE32_HANDLER( watchpoint_write32 )
{
	address_space *spacerw = (address_space *)space;
	UINT8 *oldtable = spacerw->writelookup;

	debug_cpu_memory_write_hook(spacerw, offset << 2, data, mem_mask);
	spacerw->writelookup = spacerw->write.table;
	write_dword_generic(spacerw, offset << 2, data, mem_mask);
	spacerw->writelookup = oldtable;
}

static WRITE64_HANDLER( watchpoint_write64 )
{
	address_space *spacerw = (address_space *)space;
	UINT8 *oldtable = spacerw->writelookup;

	debug_cpu_memory_write_hook(spacerw, offset << 3, data, mem_mask);
	spacerw->writelookup = spacerw->write.table;
	write_qword_generic(spacerw, offset << 3, data, mem_mask);
	spacerw->writelookup = oldtable;
}


/*-------------------------------------------------
    get_static_handler - returns points to static
    memory handlers
-------------------------------------------------*/

static genf *get_static_handler(int handlerbits, int readorwrite, int which)
{
	static const struct
	{
		UINT8		handlerbits;
		UINT8		handlernum;
		genf *		read;
		genf *		write;
	} static_handler_list[] =
	{
		{  8, STATIC_UNMAP,      (genf *)unmap_read8,       (genf *)unmap_write8 },
		{  8, STATIC_NOP,        (genf *)nop_read8,         (genf *)nop_write8 },
		{  8, STATIC_WATCHPOINT, (genf *)watchpoint_read8,  (genf *)watchpoint_write8 },

		{ 16, STATIC_UNMAP,      (genf *)unmap_read16,      (genf *)unmap_write16 },
		{ 16, STATIC_NOP,        (genf *)nop_read16,        (genf *)nop_write16 },
		{ 16, STATIC_WATCHPOINT, (genf *)watchpoint_read16, (genf *)watchpoint_write16 },

		{ 32, STATIC_UNMAP,      (genf *)unmap_read32,      (genf *)unmap_write32 },
		{ 32, STATIC_NOP,        (genf *)nop_read32,        (genf *)nop_write32 },
		{ 32, STATIC_WATCHPOINT, (genf *)watchpoint_read32, (genf *)watchpoint_write32 },

		{ 64, STATIC_UNMAP,      (genf *)unmap_read64,      (genf *)unmap_write64 },
		{ 64, STATIC_NOP,        (genf *)nop_read64,        (genf *)nop_write64 },
		{ 64, STATIC_WATCHPOINT, (genf *)watchpoint_read64, (genf *)watchpoint_write64 },
	};
	int tablenum;

	for (tablenum = 0; tablenum < sizeof(static_handler_list) / sizeof(static_handler_list[0]); tablenum++)
		if (static_handler_list[tablenum].handlerbits == handlerbits && static_handler_list[tablenum].handlernum == which)
			return readorwrite ? static_handler_list[tablenum].write : static_handler_list[tablenum].read;

	return NULL;
}



/***************************************************************************
    DEBUGGING
***************************************************************************/

/*-------------------------------------------------
    handler_to_string - return friendly string
    description of a handler
-------------------------------------------------*/

static const char *handler_to_string(const address_table *table, UINT8 entry)
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
		"bank 32",		"bank 33",		"bank 34",		"bank 35",
		"bank 36",		"bank 37",		"bank 38",		"bank 39",
		"bank 40",		"bank 41",		"bank 42",		"bank 43",
		"bank 44",		"bank 45",		"bank 46",		"bank 47",
		"bank 48",		"bank 49",		"bank 50",		"bank 51",
		"bank 52",		"bank 53",		"bank 54",		"bank 55",
		"bank 56",		"bank 57",		"bank 58",		"bank 59",
		"bank 60",		"bank 61",		"bank 62",		"bank 63",
		"bank 64",		"bank 65",		"bank 66",		"bank 67",
		"bank 68",		"bank 69",		"bank 70",		"bank 71",
		"bank 72",		"bank 73",		"bank 74",		"bank 75",
		"bank 76",		"bank 77",		"bank 78",		"bank 79",
		"bank 80",		"bank 81",		"bank 82",		"bank 83",
		"bank 84",		"bank 85",		"bank 86",		"bank 87",
		"bank 88",		"bank 89",		"bank 90",		"bank 91",
		"bank 92",		"bank 93",		"bank 94",		"bank 95",
		"bank 96",		"ram[97]",		"ram[98]",		"ram[99]",
		"ram[100]",		"ram[101]",		"ram[102]",		"ram[103]",
		"ram[104]",		"ram[105]",		"ram[106]",		"ram[107]",
		"ram[108]",		"ram[109]",		"ram[110]",		"ram[111]",
		"ram[112]",		"ram[113]",		"ram[114]",		"ram[115]",
		"ram[116]",		"ram[117]",		"ram[118]",		"ram[119]",
		"ram[120]",		"ram[121]",		"ram[122]",		"ram",
		"rom",			"nop",			"unmapped",     "watchpoint"
	};

	/* constant strings for lower entries */
	if (entry < STATIC_COUNT)
		return strings[entry];
	else
		return (table->handlers[entry]->name != NULL) ? table->handlers[entry]->name : "???";
}


/*-------------------------------------------------
    dump_map - dump the contents of a single
    address space
-------------------------------------------------*/

static void dump_map(FILE *file, const address_space *space, const address_table *table)
{
	offs_t byteaddress, bytestart, byteend;

	/* dump generic information */
	fprintf(file, "  Address bits = %d\n", space->abits);
	fprintf(file, "     Data bits = %d\n", space->dbits);
	fprintf(file, "       L1 bits = %d\n", LEVEL1_BITS);
	fprintf(file, "       L2 bits = %d\n", LEVEL2_BITS);
	fprintf(file, "  Address mask = %X\n", space->bytemask);
	fprintf(file, "\n");

	/* iterate over addresses */
	for (byteaddress = 0; byteaddress <= space->bytemask; byteaddress = byteend + 1)
	{
		UINT8 entry = table_derive_range(table, byteaddress, &bytestart, &byteend);
		fprintf(file, "%08X-%08X    = %02X: %s [offset=%08X]\n",
						bytestart, byteend, entry, handler_to_string(table, entry), table->handlers[entry]->bytestart);
	}
}


/*-------------------------------------------------
    mem_dump - internal memory dump
-------------------------------------------------*/

static void mem_dump(running_machine *machine)
{
	FILE *file;

	if (MEM_DUMP)
	{
		file = fopen("memdump.log", "w");
		if (file)
		{
			memory_dump(machine, file);
			fclose(file);
		}
	}
}



/***************************************************************************
    INPUT PORT READ HANDLERS
***************************************************************************/

/*-------------------------------------------------
    input port handlers
-------------------------------------------------*/

static UINT8 input_port_read8(const input_port_config *port, offs_t offset)
{
	return input_port_read_direct(port);
}

static UINT16 input_port_read16(const input_port_config *port, offs_t offset, UINT16 mem_mask)
{
	return input_port_read_direct(port);
}

static UINT32 input_port_read32(const input_port_config *port, offs_t offset, UINT32 mem_mask)
{
	return input_port_read_direct(port);
}

static UINT64 input_port_read64(const input_port_config *port, offs_t offset, UINT64 mem_mask)
{
	return input_port_read_direct(port);
}



/*-------------------------------------------------
    output port handlers
-------------------------------------------------*/

static void input_port_write8(const input_port_config *port, offs_t offset, UINT8 data)
{
	input_port_write_direct(port, data, 0xff);
}

static void input_port_write16(const input_port_config *port, offs_t offset, UINT16 data, UINT16 mem_mask)
{
	input_port_write_direct(port, data, mem_mask);
}

static void input_port_write32(const input_port_config *port, offs_t offset, UINT32 data, UINT32 mem_mask)
{
	input_port_write_direct(port, data, mem_mask);
}

static void input_port_write64(const input_port_config *port, offs_t offset, UINT64 data, UINT64 mem_mask)
{
	input_port_write_direct(port, data, mem_mask);
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
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT16 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			result |= (*handler->subhandler.read.shandler8)((const address_space *)handler->subobject, offset) << shift;
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
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT32 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			result |= (*handler->subhandler.read.shandler8)((const address_space *)handler->subobject, offset) << shift;
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
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT64 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			result |= (UINT64)(*handler->subhandler.read.shandler8)((const address_space *)handler->subobject, offset) << shift;
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
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT32 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT16)(mem_mask >> shift) != 0)
			result |= (*handler->subhandler.read.shandler16)((const address_space *)handler->subobject, offset, mem_mask >> shift) << shift;
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
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT64 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT16)(mem_mask >> shift) != 0)
			result |= (UINT64)(*handler->subhandler.read.shandler16)((const address_space *)handler->subobject, offset, mem_mask >> shift) << shift;
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
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;
	UINT64 result = 0;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT32)(mem_mask >> shift) != 0)
			result |= (UINT64)(*handler->subhandler.read.shandler32)((const address_space *)handler->subobject, offset, mem_mask >> shift) << shift;
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
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.shandler8)((const address_space *)handler->subobject, offset, data >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write8_from_32 - convert a 32-bit
    write to one or more byte accesses
-------------------------------------------------*/

static WRITE32_HANDLER( stub_write8_from_32 )
{
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.shandler8)((const address_space *)handler->subobject, offset, data >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write8_from_64 - convert a 64-bit
    write to one or more byte accesses
-------------------------------------------------*/

static WRITE64_HANDLER( stub_write8_from_64 )
{
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT8)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.shandler8)((const address_space *)handler->subobject, offset, data >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write16_from_32 - convert a 32-bit
    write to one or more word accesses
-------------------------------------------------*/

static WRITE32_HANDLER( stub_write16_from_32 )
{
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT16)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.shandler16)((const address_space *)handler->subobject, offset, data >> shift, mem_mask >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write16_from_64 - convert a 64-bit
    write to one or more word accesses
-------------------------------------------------*/

static WRITE64_HANDLER( stub_write16_from_64 )
{
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT16)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.shandler16)((const address_space *)handler->subobject, offset, data >> shift, mem_mask >> shift);
		offset++;
	}
}


/*-------------------------------------------------
    stub_write32_from_64 - convert a 64-bit
    write to one or more word accesses
-------------------------------------------------*/

static WRITE64_HANDLER( stub_write32_from_64 )
{
	const handler_data *handler = (const handler_data *)space;
	const UINT8 *subshift = handler->subshift;
	int subunits = handler->subunits;

	offset *= subunits;
	while (subunits-- != 0)
	{
		int shift = *subshift++;
		if ((UINT32)(mem_mask >> shift) != 0)
			(*handler->subhandler.write.shandler32)((const address_space *)handler->subobject, offset, data >> shift, mem_mask >> shift);
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
				result.read.shandler16 = stub_read8_from_16;
		}

		/* 32-bit read stubs */
		else if (spacedbits == 32)
		{
			if (handlerdbits == 8)
				result.read.shandler32 = stub_read8_from_32;
			else if (handlerdbits == 16)
				result.read.shandler32 = stub_read16_from_32;
		}

		/* 64-bit read stubs */
		else if (spacedbits == 64)
		{
			if (handlerdbits == 8)
				result.read.shandler64 = stub_read8_from_64;
			else if (handlerdbits == 16)
				result.read.shandler64 = stub_read16_from_64;
			else if (handlerdbits == 32)
				result.read.shandler64 = stub_read32_from_64;
		}
	}

	/* write stubs */
	else if (readorwrite == ROW_WRITE)
	{
		/* 16-bit write stubs */
		if (spacedbits == 16)
		{
			if (handlerdbits == 8)
				result.write.shandler16 = stub_write8_from_16;
		}

		/* 32-bit write stubs */
		else if (spacedbits == 32)
		{
			if (handlerdbits == 8)
				result.write.shandler32 = stub_write8_from_32;
			else if (handlerdbits == 16)
				result.write.shandler32 = stub_write16_from_32;
		}

		/* 64-bit write stubs */
		else if (spacedbits == 64)
		{
			if (handlerdbits == 8)
				result.write.shandler64 = stub_write8_from_64;
			else if (handlerdbits == 16)
				result.write.shandler64 = stub_write16_from_64;
			else if (handlerdbits == 32)
				result.write.shandler64 = stub_write32_from_64;
		}
	}

	assert(result.read.generic != NULL);
	return result;
}



/***************************************************************************
    8-BIT READ HANDLERS
***************************************************************************/

UINT8 memory_read_byte_8le(const address_space *space, offs_t address)
{
	return read_byte_generic(space, address);
}

UINT8 memory_read_byte_8be(const address_space *space, offs_t address)
{
	return read_byte_generic(space, address);
}

UINT16 memory_read_word_8le(const address_space *space, offs_t address)
{
	UINT16 result = memory_read_byte_8le(space, address + 0) << 0;
	return result | (memory_read_byte_8le(space, address + 1) << 8);
}

UINT16 memory_read_word_masked_8le(const address_space *space, offs_t address, UINT16 mask)
{
	UINT16 result = 0;
	if (mask & 0x00ff) result |= memory_read_byte_8le(space, address + 0) << 0;
	if (mask & 0xff00) result |= memory_read_byte_8le(space, address + 1) << 8;
	return result;
}

UINT16 memory_read_word_8be(const address_space *space, offs_t address)
{
	UINT16 result = memory_read_byte_8be(space, address + 0) << 8;
	return result | (memory_read_byte_8be(space, address + 1) << 0);
}

UINT16 memory_read_word_masked_8be(const address_space *space, offs_t address, UINT16 mask)
{
	UINT16 result = 0;
	if (mask & 0xff00) result |= memory_read_byte_8be(space, address + 0) << 8;
	if (mask & 0x00ff) result |= memory_read_byte_8be(space, address + 1) << 0;
	return result;
}

UINT32 memory_read_dword_8le(const address_space *space, offs_t address)
{
	UINT32 result = memory_read_word_8le(space, address + 0) << 0;
	return result | (memory_read_word_8le(space, address + 2) << 16);
}

UINT32 memory_read_dword_masked_8le(const address_space *space, offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0x0000ffff) result |= memory_read_word_masked_8le(space, address + 0, mask >> 0) << 0;
	if (mask & 0xffff0000) result |= memory_read_word_masked_8le(space, address + 2, mask >> 16) << 16;
	return result;
}

UINT32 memory_read_dword_8be(const address_space *space, offs_t address)
{
	UINT32 result = memory_read_word_8be(space, address + 0) << 16;
	return result | (memory_read_word_8be(space, address + 2) << 0);
}

UINT32 memory_read_dword_masked_8be(const address_space *space, offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0xffff0000) result |= memory_read_word_masked_8be(space, address + 0, mask >> 16) << 16;
	if (mask & 0x0000ffff) result |= memory_read_word_masked_8be(space, address + 2, mask >> 0) << 0;
	return result;
}

UINT64 memory_read_qword_8le(const address_space *space, offs_t address)
{
	UINT64 result = (UINT64)memory_read_dword_8le(space, address + 0) << 0;
	return result | ((UINT64)memory_read_dword_8le(space, address + 4) << 32);
}

UINT64 memory_read_qword_masked_8le(const address_space *space, offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)memory_read_dword_masked_8le(space, address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)memory_read_dword_masked_8le(space, address + 4, mask >> 32) << 32;
	return result;
}

UINT64 memory_read_qword_8be(const address_space *space, offs_t address)
{
	UINT64 result = (UINT64)memory_read_dword_8be(space, address + 0) << 32;
	return result | ((UINT64)memory_read_dword_8be(space, address + 4) << 0);
}

UINT64 memory_read_qword_masked_8be(const address_space *space, offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)memory_read_dword_masked_8be(space, address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)memory_read_dword_masked_8be(space, address + 4, mask >> 0) << 0;
	return result;
}



/***************************************************************************
    8-BIT WRITE HANDLERS
***************************************************************************/

void memory_write_byte_8le(const address_space *space, offs_t address, UINT8 data)
{
	write_byte_generic(space, address, data);
}

void memory_write_byte_8be(const address_space *space, offs_t address, UINT8 data)
{
	write_byte_generic(space, address, data);
}

void memory_write_word_8le(const address_space *space, offs_t address, UINT16 data)
{
	memory_write_byte_8le(space, address + 0, data >> 0);
	memory_write_byte_8le(space, address + 1, data >> 8);
}

void memory_write_word_masked_8le(const address_space *space, offs_t address, UINT16 data, UINT16 mask)
{
	if (mask & 0x00ff) memory_write_byte_8le(space, address + 0, data >> 0);
	if (mask & 0xff00) memory_write_byte_8le(space, address + 1, data >> 8);
}

void memory_write_word_8be(const address_space *space, offs_t address, UINT16 data)
{
	memory_write_byte_8be(space, address + 0, data >> 8);
	memory_write_byte_8be(space, address + 1, data >> 0);
}

void memory_write_word_masked_8be(const address_space *space, offs_t address, UINT16 data, UINT16 mask)
{
	if (mask & 0xff00) memory_write_byte_8be(space, address + 0, data >> 8);
	if (mask & 0x00ff) memory_write_byte_8be(space, address + 1, data >> 0);
}

void memory_write_dword_8le(const address_space *space, offs_t address, UINT32 data)
{
	memory_write_word_8le(space, address + 0, data >> 0);
	memory_write_word_8le(space, address + 2, data >> 16);
}

void memory_write_dword_masked_8le(const address_space *space, offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0x0000ffff) memory_write_word_masked_8le(space, address + 0, data >> 0, mask >> 0);
	if (mask & 0xffff0000) memory_write_word_masked_8le(space, address + 2, data >> 16, mask >> 16);
}

void memory_write_dword_8be(const address_space *space, offs_t address, UINT32 data)
{
	memory_write_word_8be(space, address + 0, data >> 16);
	memory_write_word_8be(space, address + 2, data >> 0);
}

void memory_write_dword_masked_8be(const address_space *space, offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0xffff0000) memory_write_word_masked_8be(space, address + 0, data >> 16, mask >> 16);
	if (mask & 0x0000ffff) memory_write_word_masked_8be(space, address + 2, data >> 0, mask >> 0);
}

void memory_write_qword_8le(const address_space *space, offs_t address, UINT64 data)
{
	memory_write_dword_8le(space, address + 0, data >> 0);
	memory_write_dword_8le(space, address + 4, data >> 32);
}

void memory_write_qword_masked_8le(const address_space *space, offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) memory_write_dword_masked_8le(space, address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) memory_write_dword_masked_8le(space, address + 4, data >> 32, mask >> 32);
}

void memory_write_qword_8be(const address_space *space, offs_t address, UINT64 data)
{
	memory_write_dword_8be(space, address + 0, data >> 32);
	memory_write_dword_8be(space, address + 4, data >> 0);
}

void memory_write_qword_masked_8be(const address_space *space, offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) memory_write_dword_masked_8be(space, address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) memory_write_dword_masked_8be(space, address + 4, data >> 0, mask >> 0);
}



/***************************************************************************
    16-BIT READ HANDLERS
***************************************************************************/

UINT8 memory_read_byte_16le(const address_space *space, offs_t address)
{
	UINT32 shift = (address & 1) * 8;
	return read_word_generic(space, address, 0xff << shift) >> shift;
}

UINT8 memory_read_byte_16be(const address_space *space, offs_t address)
{
	UINT32 shift = (~address & 1) * 8;
	return read_word_generic(space, address, 0xff << shift) >> shift;
}

UINT16 memory_read_word_16le(const address_space *space, offs_t address)
{
	return read_word_generic(space, address, 0xffff);
}

UINT16 memory_read_word_masked_16le(const address_space *space, offs_t address, UINT16 mask)
{
	return read_word_generic(space, address, mask);
}

UINT16 memory_read_word_16be(const address_space *space, offs_t address)
{
	return read_word_generic(space, address, 0xffff);
}

UINT16 memory_read_word_masked_16be(const address_space *space, offs_t address, UINT16 mask)
{
	return read_word_generic(space, address, mask);
}

UINT32 memory_read_dword_16le(const address_space *space, offs_t address)
{
	UINT32 result = memory_read_word_16le(space, address + 0) << 0;
	return result | (memory_read_word_16le(space, address + 2) << 16);
}

UINT32 memory_read_dword_masked_16le(const address_space *space, offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0x0000ffff) result |= memory_read_word_masked_16le(space, address + 0, mask >> 0) << 0;
	if (mask & 0xffff0000) result |= memory_read_word_masked_16le(space, address + 2, mask >> 16) << 16;
	return result;
}

UINT32 memory_read_dword_16be(const address_space *space, offs_t address)
{
	UINT32 result = memory_read_word_16be(space, address + 0) << 16;
	return result | (memory_read_word_16be(space, address + 2) << 0);
}

UINT32 memory_read_dword_masked_16be(const address_space *space, offs_t address, UINT32 mask)
{
	UINT32 result = 0;
	if (mask & 0xffff0000) result |= memory_read_word_masked_16be(space, address + 0, mask >> 16) << 16;
	if (mask & 0x0000ffff) result |= memory_read_word_masked_16be(space, address + 2, mask >> 0) << 0;
	return result;
}

UINT64 memory_read_qword_16le(const address_space *space, offs_t address)
{
	UINT64 result = (UINT64)memory_read_dword_16le(space, address + 0) << 0;
	return result | ((UINT64)memory_read_dword_16le(space, address + 4) << 32);
}

UINT64 memory_read_qword_masked_16le(const address_space *space, offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)memory_read_dword_masked_16le(space, address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)memory_read_dword_masked_16le(space, address + 4, mask >> 32) << 32;
	return result;
}

UINT64 memory_read_qword_16be(const address_space *space, offs_t address)
{
	UINT64 result = (UINT64)memory_read_dword_16be(space, address + 0) << 32;
	return result | ((UINT64)memory_read_dword_16be(space, address + 4) << 0);
}

UINT64 memory_read_qword_masked_16be(const address_space *space, offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)memory_read_dword_masked_16be(space, address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)memory_read_dword_masked_16be(space, address + 4, mask >> 0) << 0;
	return result;
}



/***************************************************************************
    16-BIT WRITE HANDLERS
***************************************************************************/

void memory_write_byte_16le(const address_space *space, offs_t address, UINT8 data)
{
	UINT32 shift = (address & 1) * 8;
	write_word_generic(space, address, data << shift, 0xff << shift);
}

void memory_write_byte_16be(const address_space *space, offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 1) * 8;
	write_word_generic(space, address, data << shift, 0xff << shift);
}

void memory_write_word_16le(const address_space *space, offs_t address, UINT16 data)
{
	write_word_generic(space, address, data, 0xffff);
}

void memory_write_word_masked_16le(const address_space *space, offs_t address, UINT16 data, UINT16 mask)
{
	write_word_generic(space, address, data, mask);
}

void memory_write_word_16be(const address_space *space, offs_t address, UINT16 data)
{
	write_word_generic(space, address, data, 0xffff);
}

void memory_write_word_masked_16be(const address_space *space, offs_t address, UINT16 data, UINT16 mask)
{
	write_word_generic(space, address, data, mask);
}

void memory_write_dword_16le(const address_space *space, offs_t address, UINT32 data)
{
	memory_write_word_16le(space, address + 0, data >> 0);
	memory_write_word_16le(space, address + 2, data >> 16);
}

void memory_write_dword_masked_16le(const address_space *space, offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0x0000ffff) memory_write_word_masked_16le(space, address + 0, data >> 0, mask >> 0);
	if (mask & 0xffff0000) memory_write_word_masked_16le(space, address + 2, data >> 16, mask >> 16);
}

void memory_write_dword_16be(const address_space *space, offs_t address, UINT32 data)
{
	memory_write_word_16be(space, address + 0, data >> 16);
	memory_write_word_16be(space, address + 2, data >> 0);
}

void memory_write_dword_masked_16be(const address_space *space, offs_t address, UINT32 data, UINT32 mask)
{
	if (mask & 0xffff0000) memory_write_word_masked_16be(space, address + 0, data >> 16, mask >> 16);
	if (mask & 0x0000ffff) memory_write_word_masked_16be(space, address + 2, data >> 0, mask >> 0);
}

void memory_write_qword_16le(const address_space *space, offs_t address, UINT64 data)
{
	memory_write_dword_16le(space, address + 0, data >> 0);
	memory_write_dword_16le(space, address + 4, data >> 32);
}

void memory_write_qword_masked_16le(const address_space *space, offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) memory_write_dword_masked_16le(space, address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) memory_write_dword_masked_16le(space, address + 4, data >> 32, mask >> 32);
}

void memory_write_qword_16be(const address_space *space, offs_t address, UINT64 data)
{
	memory_write_dword_16be(space, address + 0, data >> 32);
	memory_write_dword_16be(space, address + 4, data >> 0);
}

void memory_write_qword_masked_16be(const address_space *space, offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) memory_write_dword_masked_16be(space, address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) memory_write_dword_masked_16be(space, address + 4, data >> 0, mask >> 0);
}



/***************************************************************************
    32-BIT READ HANDLERS
***************************************************************************/

UINT8 memory_read_byte_32le(const address_space *space, offs_t address)
{
	UINT32 shift = (address & 3) * 8;
	return read_dword_generic(space, address, 0xff << shift) >> shift;
}

UINT8 memory_read_byte_32be(const address_space *space, offs_t address)
{
	UINT32 shift = (~address & 3) * 8;
	return read_dword_generic(space, address, 0xff << shift) >> shift;
}

UINT16 memory_read_word_32le(const address_space *space, offs_t address)
{
	UINT32 shift = (address & 2) * 8;
	return read_dword_generic(space, address, 0xffff << shift) >> shift;
}

UINT16 memory_read_word_masked_32le(const address_space *space, offs_t address, UINT16 mask)
{
	UINT32 shift = (address & 2) * 8;
	return read_dword_generic(space, address, mask << shift) >> shift;
}

UINT16 memory_read_word_32be(const address_space *space, offs_t address)
{
	UINT32 shift = (~address & 2) * 8;
	return read_dword_generic(space, address, 0xffff << shift) >> shift;
}

UINT16 memory_read_word_masked_32be(const address_space *space, offs_t address, UINT16 mask)
{
	UINT32 shift = (~address & 2) * 8;
	return read_dword_generic(space, address, mask << shift) >> shift;
}

UINT32 memory_read_dword_32le(const address_space *space, offs_t address)
{
	return read_dword_generic(space, address, 0xffffffff);
}

UINT32 memory_read_dword_masked_32le(const address_space *space, offs_t address, UINT32 mask)
{
	return read_dword_generic(space, address, mask);
}

UINT32 memory_read_dword_32be(const address_space *space, offs_t address)
{
	return read_dword_generic(space, address, 0xffffffff);
}

UINT32 memory_read_dword_masked_32be(const address_space *space, offs_t address, UINT32 mask)
{
	return read_dword_generic(space, address, mask);
}

UINT64 memory_read_qword_32le(const address_space *space, offs_t address)
{
	UINT64 result = (UINT64)memory_read_dword_32le(space, address + 0) << 0;
	return result | ((UINT64)memory_read_dword_32le(space, address + 4) << 32);
}

UINT64 memory_read_qword_masked_32le(const address_space *space, offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)memory_read_dword_masked_32le(space, address + 0, mask >> 0) << 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)memory_read_dword_masked_32le(space, address + 4, mask >> 32) << 32;
	return result;
}

UINT64 memory_read_qword_32be(const address_space *space, offs_t address)
{
	UINT64 result = (UINT64)memory_read_dword_32be(space, address + 0) << 32;
	return result | ((UINT64)memory_read_dword_32be(space, address + 4) << 0);
}

UINT64 memory_read_qword_masked_32be(const address_space *space, offs_t address, UINT64 mask)
{
	UINT64 result = 0;
	if (mask & U64(0xffffffff00000000)) result |= (UINT64)memory_read_dword_masked_32be(space, address + 0, mask >> 32) << 32;
	if (mask & U64(0x00000000ffffffff)) result |= (UINT64)memory_read_dword_masked_32be(space, address + 4, mask >> 0) << 0;
	return result;
}



/***************************************************************************
    32-BIT WRITE HANDLERS
***************************************************************************/

void memory_write_byte_32le(const address_space *space, offs_t address, UINT8 data)
{
	UINT32 shift = (address & 3) * 8;
	write_dword_generic(space, address, data << shift, 0xff << shift);
}

void memory_write_byte_32be(const address_space *space, offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 3) * 8;
	write_dword_generic(space, address, data << shift, 0xff << shift);
}

void memory_write_word_32le(const address_space *space, offs_t address, UINT16 data)
{
	UINT32 shift = (address & 2) * 8;
	write_dword_generic(space, address, data << shift, 0xffff << shift);
}

void memory_write_word_masked_32le(const address_space *space, offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (address & 2) * 8;
	write_dword_generic(space, address, data << shift, mask << shift);
}

void memory_write_word_32be(const address_space *space, offs_t address, UINT16 data)
{
	UINT32 shift = (~address & 2) * 8;
	write_dword_generic(space, address, data << shift, 0xffff << shift);
}

void memory_write_word_masked_32be(const address_space *space, offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (~address & 2) * 8;
	write_dword_generic(space, address, data << shift, mask << shift);
}

void memory_write_dword_32le(const address_space *space, offs_t address, UINT32 data)
{
	write_dword_generic(space, address, data, 0xffffffff);
}

void memory_write_dword_masked_32le(const address_space *space, offs_t address, UINT32 data, UINT32 mask)
{
	write_dword_generic(space, address, data, mask);
}

void memory_write_dword_32be(const address_space *space, offs_t address, UINT32 data)
{
	write_dword_generic(space, address, data, 0xffffffff);
}

void memory_write_dword_masked_32be(const address_space *space, offs_t address, UINT32 data, UINT32 mask)
{
	write_dword_generic(space, address, data, mask);
}

void memory_write_qword_32le(const address_space *space, offs_t address, UINT64 data)
{
	memory_write_dword_32le(space, address + 0, data >> 0);
	memory_write_dword_32le(space, address + 4, data >> 32);
}

void memory_write_qword_masked_32le(const address_space *space, offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0x00000000ffffffff)) memory_write_dword_masked_32le(space, address + 0, data >> 0, mask >> 0);
	if (mask & U64(0xffffffff00000000)) memory_write_dword_masked_32le(space, address + 4, data >> 32, mask >> 32);
}

void memory_write_qword_32be(const address_space *space, offs_t address, UINT64 data)
{
	memory_write_dword_32be(space, address + 0, data >> 32);
	memory_write_dword_32be(space, address + 4, data >> 0);
}

void memory_write_qword_masked_32be(const address_space *space, offs_t address, UINT64 data, UINT64 mask)
{
	if (mask & U64(0xffffffff00000000)) memory_write_dword_masked_32be(space, address + 0, data >> 32, mask >> 32);
	if (mask & U64(0x00000000ffffffff)) memory_write_dword_masked_32be(space, address + 4, data >> 0, mask >> 0);
}



/***************************************************************************
    64-BIT READ HANDLERS
***************************************************************************/

UINT8 memory_read_byte_64le(const address_space *space, offs_t address)
{
	UINT32 shift = (address & 7) * 8;
	return read_qword_generic(space, address, (UINT64)0xff << shift) >> shift;
}

UINT8 memory_read_byte_64be(const address_space *space, offs_t address)
{
	UINT32 shift = (~address & 7) * 8;
	return read_qword_generic(space, address, (UINT64)0xff << shift) >> shift;
}

UINT16 memory_read_word_64le(const address_space *space, offs_t address)
{
	UINT32 shift = (address & 6) * 8;
	return read_qword_generic(space, address, (UINT64)0xffff << shift) >> shift;
}

UINT16 memory_read_word_masked_64le(const address_space *space, offs_t address, UINT16 mask)
{
	UINT32 shift = (address & 6) * 8;
	return read_qword_generic(space, address, (UINT64)mask << shift) >> shift;
}

UINT16 memory_read_word_64be(const address_space *space, offs_t address)
{
	UINT32 shift = (~address & 6) * 8;
	return read_qword_generic(space, address, (UINT64)0xffff << shift) >> shift;
}

UINT16 memory_read_word_masked_64be(const address_space *space, offs_t address, UINT16 mask)
{
	UINT32 shift = (~address & 6) * 8;
	return read_qword_generic(space, address, (UINT64)mask << shift) >> shift;
}

UINT32 memory_read_dword_64le(const address_space *space, offs_t address)
{
	UINT32 shift = (address & 4) * 8;
	return read_qword_generic(space, address, (UINT64)0xffffffff << shift) >> shift;
}

UINT32 memory_read_dword_masked_64le(const address_space *space, offs_t address, UINT32 mask)
{
	UINT32 shift = (address & 4) * 8;
	return read_qword_generic(space, address, (UINT64)mask << shift) >> shift;
}

UINT32 memory_read_dword_64be(const address_space *space, offs_t address)
{
	UINT32 shift = (~address & 4) * 8;
	return read_qword_generic(space, address, (UINT64)0xffffffff << shift) >> shift;
}

UINT32 memory_read_dword_masked_64be(const address_space *space, offs_t address, UINT32 mask)
{
	UINT32 shift = (~address & 4) * 8;
	return read_qword_generic(space, address, (UINT64)mask << shift) >> shift;
}

UINT64 memory_read_qword_64le(const address_space *space, offs_t address)
{
	return read_qword_generic(space, address, U64(0xffffffffffffffff));
}

UINT64 memory_read_qword_masked_64le(const address_space *space, offs_t address, UINT64 mask)
{
	return read_qword_generic(space, address, mask);
}

UINT64 memory_read_qword_64be(const address_space *space, offs_t address)
{
	return read_qword_generic(space, address, U64(0xffffffffffffffff));
}

UINT64 memory_read_qword_masked_64be(const address_space *space, offs_t address, UINT64 mask)
{
	return read_qword_generic(space, address, mask);
}



/***************************************************************************
    64-BIT WRITE HANDLERS
***************************************************************************/

void memory_write_byte_64le(const address_space *space, offs_t address, UINT8 data)
{
	UINT32 shift = (address & 7) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)0xff << shift);
}

void memory_write_byte_64be(const address_space *space, offs_t address, UINT8 data)
{
	UINT32 shift = (~address & 7) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)0xff << shift);
}

void memory_write_word_64le(const address_space *space, offs_t address, UINT16 data)
{
	UINT32 shift = (address & 6) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)0xffff << shift);
}

void memory_write_word_masked_64le(const address_space *space, offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (address & 6) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void memory_write_word_64be(const address_space *space, offs_t address, UINT16 data)
{
	UINT32 shift = (~address & 6) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)0xffff << shift);
}

void memory_write_word_masked_64be(const address_space *space, offs_t address, UINT16 data, UINT16 mask)
{
	UINT32 shift = (~address & 6) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void memory_write_dword_64le(const address_space *space, offs_t address, UINT32 data)
{
	UINT32 shift = (address & 4) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)0xffffffff << shift);
}

void memory_write_dword_masked_64le(const address_space *space, offs_t address, UINT32 data, UINT32 mask)
{
	UINT32 shift = (address & 4) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void memory_write_dword_64be(const address_space *space, offs_t address, UINT32 data)
{
	UINT32 shift = (~address & 4) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)0xffffffff << shift);
}

void memory_write_dword_masked_64be(const address_space *space, offs_t address, UINT32 data, UINT32 mask)
{
	UINT32 shift = (~address & 4) * 8;
	write_qword_generic(space, address, (UINT64)data << shift, (UINT64)mask << shift);
}

void memory_write_qword_64le(const address_space *space, offs_t address, UINT64 data)
{
	write_qword_generic(space, address, data, U64(0xffffffffffffffff));
}

void memory_write_qword_masked_64le(const address_space *space, offs_t address, UINT64 data, UINT64 mask)
{
	write_qword_generic(space, address, data, mask);
}

void memory_write_qword_64be(const address_space *space, offs_t address, UINT64 data)
{
	write_qword_generic(space, address, data, U64(0xffffffffffffffff));
}

void memory_write_qword_masked_64be(const address_space *space, offs_t address, UINT64 data, UINT64 mask)
{
	write_qword_generic(space, address, data, mask);
}
