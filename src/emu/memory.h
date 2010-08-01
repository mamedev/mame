/***************************************************************************

    memory.h

    Functions which handle device memory accesses.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MEMORY_H__
#define __MEMORY_H__



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* address spaces */
enum
{
	ADDRESS_SPACE_0,				/* first address space */
	ADDRESS_SPACE_1,				/* second address space */
	ADDRESS_SPACE_2,				/* third address space */
	ADDRESS_SPACE_3,				/* fourth address space */
	ADDRESS_SPACES					/* maximum number of address spaces */
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* referenced types from other classes */
class device_config;
class device_t;
class address_map;
struct game_driver;


/* handler_data and subtable_data are opaque types used to hold information about a particular handler */
typedef struct _handler_data handler_data;
typedef struct _subtable_data subtable_data;

/* direct_range is an opaque type used to track ranges for direct access */
typedef struct _direct_range direct_range;

/* forward-declare the address_space structure */
typedef struct _address_space address_space;


/* offsets and addresses are 32-bit (for now...) */
typedef UINT32	offs_t;


/* direct_read_data contains state data for direct read access */
typedef struct _direct_read_data direct_read_data;
struct _direct_read_data
{
	UINT8 *					raw;				/* direct access data pointer (raw) */
	UINT8 *					decrypted;			/* direct access data pointer (decrypted) */
	offs_t					bytemask;			/* byte address mask */
	offs_t					bytestart;			/* minimum valid byte address */
	offs_t					byteend;			/* maximum valid byte address */
	UINT8					entry;				/* live entry */
	direct_range *			rangelist[256];		/* list of ranges for each entry */
	direct_range *			freerangelist;		/* list of recycled range entries */
};


/* direct region update handler */
typedef offs_t	(*direct_update_func) (ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t address, ATTR_UNUSED direct_read_data *direct);


/* space read/write handlers */
typedef UINT8	(*read8_space_func)  (ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset);
typedef void	(*write8_space_func) (ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data);
typedef UINT16	(*read16_space_func) (ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask);
typedef void	(*write16_space_func)(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask);
typedef UINT32	(*read32_space_func) (ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask);
typedef void	(*write32_space_func)(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask);
typedef UINT64	(*read64_space_func) (ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask);
typedef void	(*write64_space_func)(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask);


/* device read/write handlers */
typedef UINT8	(*read8_device_func)  (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset);
typedef void	(*write8_device_func) (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data);
typedef UINT16	(*read16_device_func) (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask);
typedef void	(*write16_device_func)(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask);
typedef UINT32	(*read32_device_func) (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask);
typedef void	(*write32_device_func)(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask);
typedef UINT64	(*read64_device_func) (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask);
typedef void	(*write64_device_func)(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask);


/* data_accessors is a struct with accessors of all flavors */
typedef struct _data_accessors data_accessors;
struct _data_accessors
{
	UINT8		(*read_byte)(const address_space *space, offs_t byteaddress);
	UINT16		(*read_word)(const address_space *space, offs_t byteaddress);
	UINT16		(*read_word_masked)(const address_space *space, offs_t byteaddress, UINT16 mask);
	UINT32		(*read_dword)(const address_space *space, offs_t byteaddress);
	UINT32		(*read_dword_masked)(const address_space *space, offs_t byteaddress, UINT32 mask);
	UINT64		(*read_qword)(const address_space *space, offs_t byteaddress);
	UINT64		(*read_qword_masked)(const address_space *space, offs_t byteaddress, UINT64 mask);

	void		(*write_byte)(const address_space *space, offs_t byteaddress, UINT8 data);
	void		(*write_word)(const address_space *space, offs_t byteaddress, UINT16 data);
	void		(*write_word_masked)(const address_space *space, offs_t byteaddress, UINT16 data, UINT16 mask);
	void		(*write_dword)(const address_space *space, offs_t byteaddress, UINT32 data);
	void		(*write_dword_masked)(const address_space *space, offs_t byteaddress, UINT32 data, UINT32 mask);
	void		(*write_qword)(const address_space *space, offs_t byteaddress, UINT64 data);
	void		(*write_qword_masked)(const address_space *space, offs_t byteaddress, UINT64 data, UINT64 mask);
};


/* address_table contains information about read/write accesses within an address space */
typedef struct _address_table address_table;
struct _address_table
{
	UINT8 *					table;				/* pointer to base of table */
	UINT8					subtable_alloc;		/* number of subtables allocated */
	subtable_data *			subtable;			/* info about each subtable */
	handler_data *			handlers[256];		/* array of user-installed handlers */
	running_machine *		machine;			/* pointer back to the machine */
};


/* address_space holds live information about an address space */
/* Declared above: typedef struct _address_space address_space; */
struct _address_space
{
	address_space *			next;				/* next address space in the global list */
	running_machine *		machine;			/* reference to the owning machine */
	device_t *				cpu;				/* reference to the owning device */
	const device_config *	devconfig;			/* pointer to the owning device's config */
	address_map *			map;				/* original memory map */
	const char *			name;				/* friendly name of the address space */
	UINT8 *					readlookup;			/* live lookup table for reads */
	UINT8 *					writelookup;		/* live lookup table for writes */
	data_accessors			accessors;			/* data access handlers */
	direct_read_data		direct;				/* fast direct-access read info */
	direct_update_func		directupdate;		/* fast direct-access update callback */
	UINT64					unmap;				/* unmapped value */
	offs_t					addrmask;			/* physical address mask */
	offs_t					bytemask;			/* byte-converted physical address mask */
	offs_t					logaddrmask;		/* logical address mask */
	offs_t					logbytemask;		/* byte-converted logical address mask */
	UINT8					spacenum;			/* address space index */
	endianness_t			endianness;			/* endianness of this space */
	INT8					ashift;				/* address shift */
	UINT8					abits;				/* address bits */
	UINT8					dbits;				/* data bits */
	UINT8					addrchars;			/* number of characters to use for physical addresses */
	UINT8					logaddrchars;		/* number of characters to use for logical addresses */
	UINT8					debugger_access;	/* treat accesses as coming from the debugger */
	UINT8					log_unmap;			/* log unmapped accesses in this space? */
	address_table			read;				/* memory read lookup table */
	address_table			write;				/* memory write lookup table */
};



/***************************************************************************
    MACROS
***************************************************************************/

/* opcode base adjustment handler function macro */
#define DIRECT_UPDATE_HANDLER(name)		offs_t name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t address, direct_read_data *direct)


/* space read/write handler function macros */
#define READ8_HANDLER(name) 			UINT8  name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset)
#define WRITE8_HANDLER(name)			void   name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data)
#define READ16_HANDLER(name)			UINT16 name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_HANDLER(name)			void   name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
#define READ32_HANDLER(name)			UINT32 name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask)
#define WRITE32_HANDLER(name)			void   name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask)
#define READ64_HANDLER(name)			UINT64 name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask)
#define WRITE64_HANDLER(name)			void   name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask)


/* device read/write handler function macros */
#define READ8_DEVICE_HANDLER(name)		UINT8  name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset)
#define WRITE8_DEVICE_HANDLER(name) 	void   name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data)
#define READ16_DEVICE_HANDLER(name)		UINT16 name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
#define READ32_DEVICE_HANDLER(name)		UINT32 name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask)
#define WRITE32_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask)
#define READ64_DEVICE_HANDLER(name)		UINT64 name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask)
#define WRITE64_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask)


/* helper macro for merging data with the memory mask */
#define COMBINE_DATA(varptr)			(*(varptr) = (*(varptr) & ~mem_mask) | (data & mem_mask))

#define ACCESSING_BITS_0_7				((mem_mask & 0x000000ff) != 0)
#define ACCESSING_BITS_8_15				((mem_mask & 0x0000ff00) != 0)
#define ACCESSING_BITS_16_23			((mem_mask & 0x00ff0000) != 0)
#define ACCESSING_BITS_24_31			((mem_mask & 0xff000000) != 0)
#define ACCESSING_BITS_32_39			((mem_mask & U64(0x000000ff00000000)) != 0)
#define ACCESSING_BITS_40_47			((mem_mask & U64(0x0000ff0000000000)) != 0)
#define ACCESSING_BITS_48_55			((mem_mask & U64(0x00ff000000000000)) != 0)
#define ACCESSING_BITS_56_63			((mem_mask & U64(0xff00000000000000)) != 0)

#define ACCESSING_BITS_0_15				((mem_mask & 0x0000ffff) != 0)
#define ACCESSING_BITS_16_31			((mem_mask & 0xffff0000) != 0)
#define ACCESSING_BITS_32_47			((mem_mask & U64(0x0000ffff00000000)) != 0)
#define ACCESSING_BITS_48_63			((mem_mask & U64(0xffff000000000000)) != 0)

#define ACCESSING_BITS_0_31				((mem_mask & 0xffffffff) != 0)
#define ACCESSING_BITS_32_63			((mem_mask & U64(0xffffffff00000000)) != 0)


/* opcode range safety check */
#define memory_address_outside_direct_region(S,A)	((UNEXPECTED((A) < (S)->direct.bytestart) || UNEXPECTED((A) > (S)->direct.byteend)))


/* wrappers for dynamic read handler installation */
#define memory_install_read8_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler8(space, start, end, mask, mirror, rhandler, #rhandler, NULL, NULL, 0)
#define memory_install_read16_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler16(space, start, end, mask, mirror, rhandler, #rhandler, NULL, NULL, 0)
#define memory_install_read32_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler32(space, start, end, mask, mirror, rhandler, #rhandler, NULL, NULL, 0)
#define memory_install_read64_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler64(space, start, end, mask, mirror, rhandler, #rhandler, NULL, NULL, 0)

#define memory_install_read8_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler8(space, device, start, end, mask, mirror, rhandler, #rhandler, NULL, NULL, 0)
#define memory_install_read16_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler16(space, device, start, end, mask, mirror, rhandler, #rhandler, NULL, NULL, 0)
#define memory_install_read32_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler32(space, device, start, end, mask, mirror, rhandler, #rhandler, NULL, NULL, 0)
#define memory_install_read64_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler64(space, device, start, end, mask, mirror, rhandler, #rhandler, NULL, NULL, 0)

#define memory_install_read_port(space, start, end, mask, mirror, rtag) \
	_memory_install_port(space, start, end, mask, mirror, rtag, NULL)
#define memory_install_read_bank(space, start, end, mask, mirror, rtag) \
	_memory_install_bank(space, start, end, mask, mirror, rtag, NULL)
#define memory_install_rom(space, start, end, mask, mirror, baseptr) \
	_memory_install_ram(space, start, end, mask, mirror, TRUE, FALSE, baseptr)
#define memory_unmap_read(space, start, end, mask, mirror) \
	_memory_unmap(space, start, end, mask, mirror, TRUE, FALSE, FALSE)
#define memory_nop_read(space, start, end, mask, mirror) \
	_memory_unmap(space, start, end, mask, mirror, TRUE, FALSE, TRUE)

/* wrappers for dynamic write handler installation */
#define memory_install_write8_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler8(space, start, end, mask, mirror, NULL, NULL, whandler, #whandler, 0)
#define memory_install_write16_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler16(space, start, end, mask, mirror, NULL, NULL, whandler, #whandler, 0)
#define memory_install_write32_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler32(space, start, end, mask, mirror, NULL, NULL, whandler, #whandler, 0)
#define memory_install_write64_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler64(space, start, end, mask, mirror, NULL, NULL, whandler, #whandler, 0)

#define memory_install_write8_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler8(space, device, start, end, mask, mirror, NULL, NULL, whandler, #whandler, 0)
#define memory_install_write16_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler16(space, device, start, end, mask, mirror, NULL, NULL, whandler, #whandler, 0)
#define memory_install_write32_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler32(space, device, start, end, mask, mirror, NULL, NULL, whandler, #whandler, 0)
#define memory_install_write64_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler64(space, device, start, end, mask, mirror, NULL, NULL, whandler, #whandler, 0)

#define memory_install_write_port(space, start, end, mask, mirror, wtag) \
	_memory_install_port(space, start, end, mask, mirror, NULL, wtag)
#define memory_install_write_bank(space, start, end, mask, mirror, wtag) \
	_memory_install_bank(space, start, end, mask, mirror, NULL, wtag)
#define memory_install_writeonly(space, start, end, mask, mirror, baseptr) \
	_memory_install_ram(space, start, end, mask, mirror, FALSE, TRUE, baseptr)
#define memory_unmap_write(space, start, end, mask, mirror) \
	_memory_unmap(space, start, end, mask, mirror, FALSE, TRUE, FALSE)
#define memory_nop_write(space, start, end, mask, mirror) \
	_memory_unmap(space, start, end, mask, mirror, FALSE, TRUE, TRUE)

/* wrappers for dynamic read/write handler installation */
#define memory_install_readwrite8_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler8(space, start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler, 0)
#define memory_install_readwrite16_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler16(space, start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler, 0)
#define memory_install_readwrite32_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler32(space, start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler, 0)
#define memory_install_readwrite64_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler64(space, start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler, 0)

#define memory_install_readwrite8_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler8(space, device, start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler, 0)
#define memory_install_readwrite16_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler16(space, device, start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler, 0)
#define memory_install_readwrite32_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler32(space, device, start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler, 0)
#define memory_install_readwrite64_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler64(space, device, start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler, 0)

#define memory_install_readwrite_port(space, start, end, mask, mirror, rtag, wtag) \
	_memory_install_port(space, start, end, mask, mirror, rtag, wtag)
#define memory_install_readwrite_bank(space, start, end, mask, mirror, tag) \
	_memory_install_bank(space, start, end, mask, mirror, tag, tag)
#define memory_install_ram(space, start, end, mask, mirror, baseptr) \
	_memory_install_ram(space, start, end, mask, mirror, TRUE, TRUE, baseptr)
#define memory_unmap_readwrite(space, start, end, mask, mirror) \
	_memory_unmap(space, start, end, mask, mirror, TRUE, TRUE, FALSE)
#define memory_nop_readwrite(space, start, end, mask, mirror) \
	_memory_unmap(space, start, end, mask, mirror, TRUE, TRUE, TRUE)


/* macros for accessing bytes and words within larger chunks */

/* read/write a byte to a 16-bit space */
#define BYTE_XOR_BE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0))
#define BYTE_XOR_LE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,1))

/* read/write a byte to a 32-bit space */
#define BYTE4_XOR_BE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(3,0))
#define BYTE4_XOR_LE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,3))

/* read/write a word to a 32-bit space */
#define WORD_XOR_BE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0))
#define WORD_XOR_LE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,2))

/* read/write a byte to a 64-bit space */
#define BYTE8_XOR_BE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(7,0))
#define BYTE8_XOR_LE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,7))

/* read/write a word to a 64-bit space */
#define WORD2_XOR_BE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(6,0))
#define WORD2_XOR_LE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,6))

/* read/write a dword to a 64-bit space */
#define DWORD_XOR_BE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(4,0))
#define DWORD_XOR_LE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,4))



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

extern const char *const address_space_names[ADDRESS_SPACES];



/***************************************************************************
    FUNCTION PROTOTYPES FOR CORE MEMORY FUNCTIONS
***************************************************************************/


/* ----- core system operations ----- */

/* initialize the memory system */
void memory_init(running_machine *machine);



/* ----- direct access control ----- */

/* registers an address range as having a decrypted data pointer */
void memory_set_decrypted_region(const address_space *space, offs_t addrstart, offs_t addrend, void *base) ATTR_NONNULL(1, 4);

/* register a handler for opcode base changes on a given device */
direct_update_func memory_set_direct_update_handler(const address_space *space, direct_update_func function) ATTR_NONNULL(1);

/* called by device cores to update the opcode base for the given address */
int memory_set_direct_region(const address_space *space, offs_t *byteaddress) ATTR_NONNULL(1, 2);

/* return a pointer the memory byte provided in the given address space, or NULL if it is not mapped to a bank */
void *memory_get_read_ptr(const address_space *space, offs_t byteaddress) ATTR_NONNULL(1);

/* return a pointer the memory byte provided in the given address space, or NULL if it is not mapped to a writeable bank */
void *memory_get_write_ptr(const address_space *space, offs_t byteaddress) ATTR_NONNULL(1);



/* ----- memory banking ----- */

/* configure the addresses for a bank */
void memory_configure_bank(running_machine *machine, const char *tag, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(1, 5);

/* configure the decrypted addresses for a bank */
void memory_configure_bank_decrypted(running_machine *machine, const char *tag, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(1, 5);

/* select one pre-configured entry to be the new bank base */
void memory_set_bank(running_machine *machine, const char *tag, int entrynum) ATTR_NONNULL(1);

/* return the currently selected bank */
int memory_get_bank(running_machine *machine, const char *tag) ATTR_NONNULL(1);

/* set the absolute address of a bank base */
void memory_set_bankptr(running_machine *machine, const char *tag, void *base) ATTR_NONNULL(1, 3);



/* ----- dynamic address space mapping ----- */

/* install a new 8-bit memory handler into the given address space, returning a pointer to the memory backing it, if present */
UINT8 *_memory_install_handler8(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, const char *rhandler_name, write8_space_func whandler, const char *whandler_name, int handlerunitmask) ATTR_NONNULL(1);

/* same as above but explicitly for 16-bit handlers */
UINT16 *_memory_install_handler16(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, const char *rhandler_name, write16_space_func whandler, const char *whandler_name, int handlerunitmask) ATTR_NONNULL(1);

/* same as above but explicitly for 32-bit handlers */
UINT32 *_memory_install_handler32(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, const char *rhandler_name, write32_space_func whandler, const char *whandler_name, int handlerunitmask) ATTR_NONNULL(1);

/* same as above but explicitly for 64-bit handlers */
UINT64 *_memory_install_handler64(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, const char *rhandler_name, write64_space_func whandler, const char *whandler_name, int handlerunitmask) ATTR_NONNULL(1);

/* install a new 8-bit device memory handler into the given address space, returning a pointer to the memory backing it, if present */
UINT8 *_memory_install_device_handler8(const address_space *space, device_t *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, const char *rhandler_name, write8_device_func whandler, const char *whandler_name, int handlerunitmask) ATTR_NONNULL(1, 2);

/* same as above but explicitly for 16-bit handlers */
UINT16 *_memory_install_device_handler16(const address_space *space, device_t *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, const char *rhandler_name, write16_device_func whandler, const char *whandler_name, int handlerunitmask) ATTR_NONNULL(1, 2);

/* same as above but explicitly for 32-bit handlers */
UINT32 *_memory_install_device_handler32(const address_space *space, device_t *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, const char *rhandler_name, write32_device_func whandler, const char *whandler_name, int handlerunitmask) ATTR_NONNULL(1, 2);

/* same as above but explicitly for 64-bit handlers */
UINT64 *_memory_install_device_handler64(const address_space *space, device_t *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, const char *rhandler_name, write64_device_func whandler, const char *whandler_name, int handlerunitmask) ATTR_NONNULL(1, 2);

/* install a new port into the given address space */
void _memory_install_port(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag) ATTR_NONNULL(1);

/* install a new bank into the given address space */
void _memory_install_bank(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag) ATTR_NONNULL(1);

/* install a simple fixed RAM region into the given address space */
void *_memory_install_ram(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, UINT8 install_read, UINT8 install_write, void *baseptr) ATTR_NONNULL(1);

/* unmap a section of address space */
void _memory_unmap(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, UINT8 unmap_read, UINT8 unmap_write, UINT8 quiet) ATTR_NONNULL(1);



/* ----- debugger helpers ----- */

/* return a string describing the handler at a particular offset */
const char *memory_get_handler_string(const address_space *space, int read0_or_write1, offs_t byteaddress);

/* enable/disable read watchpoint tracking for a given address space */
void memory_enable_read_watchpoints(const address_space *space, int enable);

/* enable/disable write watchpoint tracking for a given address space */
void memory_enable_write_watchpoints(const address_space *space, int enable);

/* control whether subsequent accesses are treated as coming from the debugger */
void memory_set_debugger_access(const address_space *space, int debugger);

/* sets whether unmapped memory accesses should be logged or not */
void memory_set_log_unmap(const address_space *space, int log);

/* gets whether unmapped memory accesses are being logged or not */
int	memory_get_log_unmap(const address_space *space);

/* dump the internal memory tables to the given file */
void memory_dump(running_machine *machine, FILE *file);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    memory_address_to_byte - convert an address in
    the specified address space to a byte offset
-------------------------------------------------*/

INLINE offs_t memory_address_to_byte(const address_space *space, offs_t address)
{
	return (space->ashift < 0) ? (address << -space->ashift) : (address >> space->ashift);
}


/*-------------------------------------------------
    memory_address_to_byte_end - convert an address
    in the specified address space to a byte
    offset specifying the last byte covered by
    the address
-------------------------------------------------*/

INLINE offs_t memory_address_to_byte_end(const address_space *space, offs_t address)
{
	return (space->ashift < 0) ? ((address << -space->ashift) | ((1 << -space->ashift) - 1)) : (address >> space->ashift);
}


/*-------------------------------------------------
    memory_byte_to_address - convert a byte offset
    to an address in the specified address space
-------------------------------------------------*/

INLINE offs_t memory_byte_to_address(const address_space *space, offs_t address)
{
	return (space->ashift < 0) ? (address >> -space->ashift) : (address << space->ashift);
}


/*-------------------------------------------------
    memory_byte_to_address_end - convert a byte
    offset to an address in the specified address
    space specifying the last address covered by
    the byte
-------------------------------------------------*/

INLINE offs_t memory_byte_to_address_end(const address_space *space, offs_t address)
{
	return (space->ashift < 0) ? (address >> -space->ashift) : ((address << space->ashift) | ((1 << space->ashift) - 1));
}


/*-------------------------------------------------
    memory_read_byte/word/dword/qword - read a
    value from the specified address space
-------------------------------------------------*/

INLINE UINT8 memory_read_byte(const address_space *space, offs_t byteaddress)
{
	return (*space->accessors.read_byte)(space, byteaddress);
}

INLINE UINT16 memory_read_word(const address_space *space, offs_t byteaddress)
{
	return (*space->accessors.read_word)(space, byteaddress);
}

INLINE UINT16 memory_read_word_masked(const address_space *space, offs_t byteaddress, UINT16 mask)
{
	return (*space->accessors.read_word_masked)(space, byteaddress, mask);
}

INLINE UINT32 memory_read_dword(const address_space *space, offs_t byteaddress)
{
	return (*space->accessors.read_dword)(space, byteaddress);
}

INLINE UINT32 memory_read_dword_masked(const address_space *space, offs_t byteaddress, UINT32 mask)
{
	return (*space->accessors.read_dword_masked)(space, byteaddress, mask);
}

INLINE UINT64 memory_read_qword(const address_space *space, offs_t byteaddress)
{
	return (*space->accessors.read_qword)(space, byteaddress);
}

INLINE UINT64 memory_read_qword_masked(const address_space *space, offs_t byteaddress, UINT64 mask)
{
	return (*space->accessors.read_qword_masked)(space, byteaddress, mask);
}


/*-------------------------------------------------
    memory_write_byte/word/dword/qword - write a
    value to the specified address space
-------------------------------------------------*/

INLINE void memory_write_byte(const address_space *space, offs_t byteaddress, UINT8 data)
{
	(*space->accessors.write_byte)(space, byteaddress, data);
}

INLINE void memory_write_word(const address_space *space, offs_t byteaddress, UINT16 data)
{
	(*space->accessors.write_word)(space, byteaddress, data);
}

INLINE void memory_write_word_masked(const address_space *space, offs_t byteaddress, UINT16 data, UINT16 mask)
{
	(*space->accessors.write_word_masked)(space, byteaddress, data, mask);
}

INLINE void memory_write_dword(const address_space *space, offs_t byteaddress, UINT32 data)
{
	(*space->accessors.write_dword)(space, byteaddress, data);
}

INLINE void memory_write_dword_masked(const address_space *space, offs_t byteaddress, UINT32 data, UINT32 mask)
{
	(*space->accessors.write_dword_masked)(space, byteaddress, data, mask);
}

INLINE void memory_write_qword(const address_space *space, offs_t byteaddress, UINT64 data)
{
	(*space->accessors.write_qword)(space, byteaddress, data);
}

INLINE void memory_write_qword_masked(const address_space *space, offs_t byteaddress, UINT64 data, UINT64 mask)
{
	(*space->accessors.write_qword_masked)(space, byteaddress, data, mask);
}


/*-------------------------------------------------
    memory_decrypted_read_byte/word/dword/qword -
    read a value from the specified address space
    using the direct addressing mechanism and
    the decrypted base pointer
-------------------------------------------------*/

INLINE void *memory_decrypted_read_ptr(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return &space->direct.decrypted[byteaddress & space->direct.bytemask];
	return NULL;
}

INLINE UINT8 memory_decrypted_read_byte(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return space->direct.decrypted[byteaddress & space->direct.bytemask];
	return memory_read_byte(space, byteaddress);
}

INLINE UINT16 memory_decrypted_read_word(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return *(UINT16 *)&space->direct.decrypted[byteaddress & space->direct.bytemask];
	return memory_read_word(space, byteaddress);
}

INLINE UINT32 memory_decrypted_read_dword(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return *(UINT32 *)&space->direct.decrypted[byteaddress & space->direct.bytemask];
	return memory_read_dword(space, byteaddress);
}

INLINE UINT64 memory_decrypted_read_qword(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return *(UINT64 *)&space->direct.decrypted[byteaddress & space->direct.bytemask];
	return memory_read_qword(space, byteaddress);
}


/*-------------------------------------------------
    memory_raw_read_byte/word/dword/qword -
    read a value from the specified address space
    using the direct addressing mechanism and
    the raw base pointer
-------------------------------------------------*/

INLINE void *memory_raw_read_ptr(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return &space->direct.raw[byteaddress & space->direct.bytemask];
	return NULL;
}

INLINE UINT8 memory_raw_read_byte(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return space->direct.raw[byteaddress & space->direct.bytemask];
	return memory_read_byte(space, byteaddress);
}

INLINE UINT16 memory_raw_read_word(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return *(UINT16 *)&space->direct.raw[byteaddress & space->direct.bytemask];
	return memory_read_word(space, byteaddress);
}

INLINE UINT32 memory_raw_read_dword(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return *(UINT32 *)&space->direct.raw[byteaddress & space->direct.bytemask];
	return memory_read_dword(space, byteaddress);
}

INLINE UINT64 memory_raw_read_qword(const address_space *space, offs_t byteaddress)
{
	if (!memory_address_outside_direct_region(space, byteaddress) || memory_set_direct_region(space, &byteaddress))
		return *(UINT64 *)&space->direct.raw[byteaddress & space->direct.bytemask];
	return memory_read_qword(space, byteaddress);
}



/***************************************************************************
    FUNCTION PROTOTYPES FOR CORE READ/WRITE ROUTINES
***************************************************************************/

/* declare generic address space handlers */
UINT8 memory_read_byte_8le(const address_space *space, offs_t address);
UINT16 memory_read_word_8le(const address_space *space, offs_t address);
UINT16 memory_read_word_masked_8le(const address_space *space, offs_t address, UINT16 mask);
UINT32 memory_read_dword_8le(const address_space *space, offs_t address);
UINT32 memory_read_dword_masked_8le(const address_space *space, offs_t address, UINT32 mask);
UINT64 memory_read_qword_8le(const address_space *space, offs_t address);
UINT64 memory_read_qword_masked_8le(const address_space *space, offs_t address, UINT64 mask);
void memory_write_byte_8le(const address_space *space, offs_t address, UINT8 data);
void memory_write_word_8le(const address_space *space, offs_t address, UINT16 data);
void memory_write_word_masked_8le(const address_space *space, offs_t address, UINT16 data, UINT16 mask);
void memory_write_dword_8le(const address_space *space, offs_t address, UINT32 data);
void memory_write_dword_masked_8le(const address_space *space, offs_t address, UINT32 data, UINT32 mask);
void memory_write_qword_8le(const address_space *space, offs_t address, UINT64 data);
void memory_write_qword_masked_8le(const address_space *space, offs_t address, UINT64 data, UINT64 mask);

UINT8 memory_read_byte_8be(const address_space *space, offs_t address);
UINT16 memory_read_word_8be(const address_space *space, offs_t address);
UINT16 memory_read_word_masked_8be(const address_space *space, offs_t address, UINT16 mask);
UINT32 memory_read_dword_8be(const address_space *space, offs_t address);
UINT32 memory_read_dword_masked_8be(const address_space *space, offs_t address, UINT32 mask);
UINT64 memory_read_qword_8be(const address_space *space, offs_t address);
UINT64 memory_read_qword_masked_8be(const address_space *space, offs_t address, UINT64 mask);
void memory_write_byte_8be(const address_space *space, offs_t address, UINT8 data);
void memory_write_word_8be(const address_space *space, offs_t address, UINT16 data);
void memory_write_word_masked_8be(const address_space *space, offs_t address, UINT16 data, UINT16 mask);
void memory_write_dword_8be(const address_space *space, offs_t address, UINT32 data);
void memory_write_dword_masked_8be(const address_space *space, offs_t address, UINT32 data, UINT32 mask);
void memory_write_qword_8be(const address_space *space, offs_t address, UINT64 data);
void memory_write_qword_masked_8be(const address_space *space, offs_t address, UINT64 data, UINT64 mask);

UINT8 memory_read_byte_16le(const address_space *space, offs_t address);
UINT16 memory_read_word_16le(const address_space *space, offs_t address);
UINT16 memory_read_word_masked_16le(const address_space *space, offs_t address, UINT16 mask);
UINT32 memory_read_dword_16le(const address_space *space, offs_t address);
UINT32 memory_read_dword_masked_16le(const address_space *space, offs_t address, UINT32 mask);
UINT64 memory_read_qword_16le(const address_space *space, offs_t address);
UINT64 memory_read_qword_masked_16le(const address_space *space, offs_t address, UINT64 mask);
void memory_write_byte_16le(const address_space *space, offs_t address, UINT8 data);
void memory_write_word_16le(const address_space *space, offs_t address, UINT16 data);
void memory_write_word_masked_16le(const address_space *space, offs_t address, UINT16 data, UINT16 mask);
void memory_write_dword_16le(const address_space *space, offs_t address, UINT32 data);
void memory_write_dword_masked_16le(const address_space *space, offs_t address, UINT32 data, UINT32 mask);
void memory_write_qword_16le(const address_space *space, offs_t address, UINT64 data);
void memory_write_qword_masked_16le(const address_space *space, offs_t address, UINT64 data, UINT64 mask);

UINT8 memory_read_byte_16be(const address_space *space, offs_t address);
UINT16 memory_read_word_16be(const address_space *space, offs_t address);
UINT16 memory_read_word_masked_16be(const address_space *space, offs_t address, UINT16 mask);
UINT32 memory_read_dword_16be(const address_space *space, offs_t address);
UINT32 memory_read_dword_masked_16be(const address_space *space, offs_t address, UINT32 mask);
UINT64 memory_read_qword_16be(const address_space *space, offs_t address);
UINT64 memory_read_qword_masked_16be(const address_space *space, offs_t address, UINT64 mask);
void memory_write_byte_16be(const address_space *space, offs_t address, UINT8 data);
void memory_write_word_16be(const address_space *space, offs_t address, UINT16 data);
void memory_write_word_masked_16be(const address_space *space, offs_t address, UINT16 data, UINT16 mask);
void memory_write_dword_16be(const address_space *space, offs_t address, UINT32 data);
void memory_write_dword_masked_16be(const address_space *space, offs_t address, UINT32 data, UINT32 mask);
void memory_write_qword_16be(const address_space *space, offs_t address, UINT64 data);
void memory_write_qword_masked_16be(const address_space *space, offs_t address, UINT64 data, UINT64 mask);

UINT8 memory_read_byte_32le(const address_space *space, offs_t address);
UINT16 memory_read_word_32le(const address_space *space, offs_t address);
UINT16 memory_read_word_masked_32le(const address_space *space, offs_t address, UINT16 mask);
UINT32 memory_read_dword_32le(const address_space *space, offs_t address);
UINT32 memory_read_dword_masked_32le(const address_space *space, offs_t address, UINT32 mask);
UINT64 memory_read_qword_32le(const address_space *space, offs_t address);
UINT64 memory_read_qword_masked_32le(const address_space *space, offs_t address, UINT64 mask);
void memory_write_byte_32le(const address_space *space, offs_t address, UINT8 data);
void memory_write_word_32le(const address_space *space, offs_t address, UINT16 data);
void memory_write_word_masked_32le(const address_space *space, offs_t address, UINT16 data, UINT16 mask);
void memory_write_dword_32le(const address_space *space, offs_t address, UINT32 data);
void memory_write_dword_masked_32le(const address_space *space, offs_t address, UINT32 data, UINT32 mask);
void memory_write_qword_32le(const address_space *space, offs_t address, UINT64 data);
void memory_write_qword_masked_32le(const address_space *space, offs_t address, UINT64 data, UINT64 mask);

UINT8 memory_read_byte_32be(const address_space *space, offs_t address);
UINT16 memory_read_word_32be(const address_space *space, offs_t address);
UINT16 memory_read_word_masked_32be(const address_space *space, offs_t address, UINT16 mask);
UINT32 memory_read_dword_32be(const address_space *space, offs_t address);
UINT32 memory_read_dword_masked_32be(const address_space *space, offs_t address, UINT32 mask);
UINT64 memory_read_qword_32be(const address_space *space, offs_t address);
UINT64 memory_read_qword_masked_32be(const address_space *space, offs_t address, UINT64 mask);
void memory_write_byte_32be(const address_space *space, offs_t address, UINT8 data);
void memory_write_word_32be(const address_space *space, offs_t address, UINT16 data);
void memory_write_word_masked_32be(const address_space *space, offs_t address, UINT16 data, UINT16 mask);
void memory_write_dword_32be(const address_space *space, offs_t address, UINT32 data);
void memory_write_dword_masked_32be(const address_space *space, offs_t address, UINT32 data, UINT32 mask);
void memory_write_qword_32be(const address_space *space, offs_t address, UINT64 data);
void memory_write_qword_masked_32be(const address_space *space, offs_t address, UINT64 data, UINT64 mask);

UINT8 memory_read_byte_64le(const address_space *space, offs_t address);
UINT16 memory_read_word_64le(const address_space *space, offs_t address);
UINT16 memory_read_word_masked_64le(const address_space *space, offs_t address, UINT16 mask);
UINT32 memory_read_dword_64le(const address_space *space, offs_t address);
UINT32 memory_read_dword_masked_64le(const address_space *space, offs_t address, UINT32 mask);
UINT64 memory_read_qword_64le(const address_space *space, offs_t address);
UINT64 memory_read_qword_masked_64le(const address_space *space, offs_t address, UINT64 mask);
void memory_write_byte_64le(const address_space *space, offs_t address, UINT8 data);
void memory_write_word_64le(const address_space *space, offs_t address, UINT16 data);
void memory_write_word_masked_64le(const address_space *space, offs_t address, UINT16 data, UINT16 mask);
void memory_write_dword_64le(const address_space *space, offs_t address, UINT32 data);
void memory_write_dword_masked_64le(const address_space *space, offs_t address, UINT32 data, UINT32 mask);
void memory_write_qword_64le(const address_space *space, offs_t address, UINT64 data);
void memory_write_qword_masked_64le(const address_space *space, offs_t address, UINT64 data, UINT64 mask);

UINT8 memory_read_byte_64be(const address_space *space, offs_t address);
UINT16 memory_read_word_64be(const address_space *space, offs_t address);
UINT16 memory_read_word_masked_64be(const address_space *space, offs_t address, UINT16 mask);
UINT32 memory_read_dword_64be(const address_space *space, offs_t address);
UINT32 memory_read_dword_masked_64be(const address_space *space, offs_t address, UINT32 mask);
UINT64 memory_read_qword_64be(const address_space *space, offs_t address);
UINT64 memory_read_qword_masked_64be(const address_space *space, offs_t address, UINT64 mask);
void memory_write_byte_64be(const address_space *space, offs_t address, UINT8 data);
void memory_write_word_64be(const address_space *space, offs_t address, UINT16 data);
void memory_write_word_masked_64be(const address_space *space, offs_t address, UINT16 data, UINT16 mask);
void memory_write_dword_64be(const address_space *space, offs_t address, UINT32 data);
void memory_write_dword_masked_64be(const address_space *space, offs_t address, UINT32 data, UINT32 mask);
void memory_write_qword_64be(const address_space *space, offs_t address, UINT64 data);
void memory_write_qword_masked_64be(const address_space *space, offs_t address, UINT64 data, UINT64 mask);

#endif	/* __MEMORY_H__ */
