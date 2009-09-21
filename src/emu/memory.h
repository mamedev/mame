/***************************************************************************

    memory.h

    Functions which handle device memory accesses.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "mamecore.h"
#include "tokenize.h"
#include "astring.h"



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


/* static data access handler constants */
enum
{
	STATIC_INVALID = 0,									/* invalid - should never be used */
	STATIC_BANK1 = 1,									/* first memory bank */
	/* entries 1-64 are for fixed banks 1-64 specified by the driver */
	/* entries 65-122 are for dynamically allocated internal banks */
	STATIC_BANKMAX = 122,								/* last memory bank */
	STATIC_RAM,											/* RAM - reads/writes map to dynamic banks */
	STATIC_ROM,											/* ROM - reads = RAM; writes = UNMAP */
	STATIC_NOP,											/* NOP - reads = unmapped value; writes = no-op */
	STATIC_UNMAP,										/* unmapped - same as NOP except we log errors */
	STATIC_WATCHPOINT,									/* watchpoint - used internally */
	STATIC_COUNT										/* total number of static handlers */
};


/* address map tokens */
enum
{
	ADDRMAP_TOKEN_INVALID,

	ADDRMAP_TOKEN_START,
	ADDRMAP_TOKEN_END,
	ADDRMAP_TOKEN_INCLUDE,

	ADDRMAP_TOKEN_GLOBAL_MASK,
	ADDRMAP_TOKEN_UNMAP_VALUE,

	ADDRMAP_TOKEN_RANGE,
	ADDRMAP_TOKEN_MASK,
	ADDRMAP_TOKEN_MIRROR,
	ADDRMAP_TOKEN_READ,
	ADDRMAP_TOKEN_WRITE,
	ADDRMAP_TOKEN_DEVICE_READ,
	ADDRMAP_TOKEN_DEVICE_WRITE,
	ADDRMAP_TOKEN_READ_PORT,
	ADDRMAP_TOKEN_REGION,
	ADDRMAP_TOKEN_SHARE,
	ADDRMAP_TOKEN_BASEPTR,
	ADDRMAP_TOKEN_BASE_MEMBER,
	ADDRMAP_TOKEN_SIZEPTR,
	ADDRMAP_TOKEN_SIZE_MEMBER
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

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
	UINT8		 			entry;				/* live entry */
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
typedef UINT8	(*read8_device_func)  (ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset);
typedef void	(*write8_device_func) (ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data);
typedef UINT16	(*read16_device_func) (ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask);
typedef void	(*write16_device_func)(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask);
typedef UINT32	(*read32_device_func) (ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask);
typedef void	(*write32_device_func)(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask);
typedef UINT64	(*read64_device_func) (ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask);
typedef void	(*write64_device_func)(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask);


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


/* read_handler is a union of all the different read handler types */
typedef union _read_handler read_handler;
union _read_handler
{
	genf *					generic;			/* generic function pointer */
	read8_space_func		shandler8;			/* 8-bit space read handler */
	read16_space_func		shandler16;			/* 16-bit space read handler */
	read32_space_func		shandler32;			/* 32-bit space read handler */
	read64_space_func		shandler64;			/* 64-bit space read handler */
	read8_device_func		dhandler8;			/* 8-bit device read handler */
	read16_device_func		dhandler16;			/* 16-bit device read handler */
	read32_device_func		dhandler32;			/* 32-bit device read handler */
	read64_device_func		dhandler64;			/* 64-bit device read handler */
};


/* write_handler is a union of all the different write handler types */
typedef union _write_handler write_handler;
union _write_handler
{
	genf *					generic;			/* generic function pointer */
	write8_space_func		shandler8;			/* 8-bit space write handler */
	write16_space_func		shandler16;			/* 16-bit space write handler */
	write32_space_func		shandler32;			/* 32-bit space write handler */
	write64_space_func		shandler64;			/* 64-bit space write handler */
	write8_device_func		dhandler8;			/* 8-bit device write handler */
	write16_device_func		dhandler16;			/* 16-bit device write handler */
	write32_device_func		dhandler32;			/* 32-bit device write handler */
	write64_device_func		dhandler64;			/* 64-bit device write handler */
};


/* memory_handler is a union of all read and write handler types */
typedef union _memory_handler memory_handler;
union _memory_handler
{
	genf *					generic;			/* generic function pointer */
	read_handler			read;				/* read handler union */
	write_handler			write;				/* write handler union */
};


/* address_map_entry is a linked list element describing one address range in a map */
typedef struct _address_map_entry address_map_entry;
struct _address_map_entry
{
	address_map_entry *		next;				/* pointer to the next entry */
	astring *				read_devtag_string;	/* string used to hold derived names */
	astring *				write_devtag_string;/* string used to hold derived names */
	astring *				region_string;		/* string used to hold derived names */

	offs_t					addrstart;			/* start address */
	offs_t					addrend;			/* end address */
	offs_t					addrmirror;			/* mirror bits */
	offs_t					addrmask;			/* mask bits */
	read_handler 			read;				/* read handler callback */
	UINT8					read_bits;			/* bits for the read handler callback (0=default, 1=8, 2=16, 3=32) */
	UINT8					read_mask;			/* mask bits indicating which subunits to process */
	const char *			read_name;			/* read handler callback name */
	const char *			read_devtag;		/* read tag for the relevant device */
	const char *			read_porttag;		/* tag for input port reading */
	write_handler 			write;				/* write handler callback */
	UINT8					write_bits;			/* bits for the write handler callback (0=default, 1=8, 2=16, 3=32) */
	UINT8					write_mask;			/* mask bits indicating which subunits to process */
	const char *			write_name;			/* write handler callback name */
	const char *			write_devtag;		/* read tag for the relevant device */
	UINT32					share;				/* index of a shared memory block */
	void **					baseptr;			/* receives pointer to memory (optional) */
	size_t *				sizeptr;			/* receives size of area in bytes (optional) */
	UINT32					baseptroffs_plus1;	/* offset of base pointer within driver_data, plus 1 */
	UINT32					sizeptroffs_plus1;	/* offset of size pointer within driver_data, plus 1 */
	const char *			region;				/* tag of region containing the memory backing this entry */
	offs_t					rgnoffs;			/* offset within the region */

	void *					memory;				/* pointer to memory backing this entry */
	offs_t					bytestart;			/* byte-adjusted start address */
	offs_t					byteend;			/* byte-adjusted end address */
	offs_t					bytemirror;			/* byte-adjusted mirror bits */
	offs_t					bytemask;			/* byte-adjusted mask bits */
};


/* address_map holds global map parameters plus the head of the list of entries */
typedef struct _address_map address_map;
struct _address_map
{
	UINT8					spacenum;			/* space number of the map */
	UINT8					databits;			/* data bits represented by the map */
	UINT8					unmapval;			/* unmapped memory value */
	offs_t					globalmask;			/* global mask */
	address_map_entry *		entrylist;			/* list of entries */
};


/* address_table contains information about read/write accesses within an address space */
typedef struct _address_table address_table;
struct _address_table
{
	UINT8 *					table;				/* pointer to base of table */
	UINT8 					subtable_alloc;		/* number of subtables allocated */
	subtable_data *			subtable; 			/* info about each subtable */
	handler_data *			handlers[256];		/* array of user-installed handlers */
};


/* address_space holds live information about an address space */
/* Declared above: typedef struct _address_space address_space; */
struct _address_space
{
	address_space *			next;				/* next address space in the global list */
	running_machine *		machine;			/* reference to the owning machine */
	const device_config *	cpu;				/* reference to the owning device */
	address_map *			map;				/* original memory map */
	const char *			name;				/* friendly name of the address space */
	UINT8 *					readlookup;			/* live lookup table for reads */
	UINT8 *					writelookup;		/* live lookup table for writes */
	data_accessors		 	accessors;			/* data access handlers */
	direct_read_data		direct;				/* fast direct-access read info */
	direct_update_func 		directupdate;		/* fast direct-access update callback */
	UINT64					unmap;				/* unmapped value */
	offs_t					addrmask;			/* physical address mask */
	offs_t					bytemask;			/* byte-converted physical address mask */
	offs_t					logaddrmask;		/* logical address mask */
	offs_t					logbytemask;		/* byte-converted logical address mask */
	UINT8					spacenum;			/* address space index */
	UINT8					endianness;			/* endianness of this space */
	INT8					ashift;				/* address shift */
	UINT8					abits;				/* address bits */
	UINT8 					dbits;				/* data bits */
	UINT8					addrchars;			/* number of characters to use for physical addresses */
	UINT8					logaddrchars;		/* number of characters to use for logical addresses */
	UINT8					debugger_access;	/* treat accesses as coming from the debugger */
	UINT8					log_unmap;			/* log unmapped accesses in this space? */
	address_table			read;				/* memory read lookup table */
	address_table			write;				/* memory write lookup table */
};


/* addrmap_token is a union of all types for a generic address map */
typedef union _addrmap_token addrmap_token;
union _addrmap_token
{
	TOKEN_COMMON_FIELDS
	const addrmap_token *	tokenptr;
	read_handler			read;				/* generic read handlers */
	write_handler			write;				/* generic write handlers */
	UINT8 **				memptr;				/* memory pointer */
	size_t *				sizeptr;			/* size pointer */
};


/* addrmap8_token is a union of all types for an 8-bit address map */
typedef union _addrmap8_token addrmap8_token;
union _addrmap8_token
{
	TOKEN_COMMON_FIELDS
	const addrmap8_token *	tokenptr;
	read8_space_func		sread;				/* pointer to native space read handler */
	write8_space_func		swrite;				/* pointer to native space write handler */
	read8_device_func		dread;				/* pointer to native device read handler */
	write8_device_func		dwrite;				/* pointer to native device write handler */
	read_handler			read;				/* generic read handlers */
	write_handler			write;				/* generic write handlers */
	UINT8 **				memptr;				/* memory pointer */
	size_t *				sizeptr;			/* size pointer */
};


/* addrmap16_token is a union of all types for a 16-bit address map */
typedef union _addrmap16_token addrmap16_token;
union _addrmap16_token
{
	TOKEN_COMMON_FIELDS
	const addrmap16_token *	tokenptr;
	read16_space_func		sread;				/* pointer to native read handler */
	write16_space_func 		swrite;				/* pointer to native write handler */
	read16_device_func		dread;				/* pointer to native device read handler */
	write16_device_func		dwrite;				/* pointer to native device write handler */
	read8_space_func		sread8;				/* pointer to 8-bit space read handler */
	write8_space_func		swrite8;			/* pointer to 8-bit space write handler */
	read8_device_func		dread8;				/* pointer to 8-bit device read handler */
	write8_device_func		dwrite8;			/* pointer to 8-bit device write handler */
	read_handler			read;				/* generic read handlers */
	write_handler			write;				/* generic write handlers */
	UINT16 **				memptr;				/* memory pointer */
	size_t *				sizeptr;			/* size pointer */
};


/* addrmap32_token is a union of all types for a 32-bit address map */
typedef union _addrmap32_token addrmap32_token;
union _addrmap32_token
{
	TOKEN_COMMON_FIELDS
	const addrmap32_token *	tokenptr;
	read32_space_func		sread;				/* pointer to native read handler */
	write32_space_func 		swrite;				/* pointer to native write handler */
	read32_device_func		dread;				/* pointer to native device read handler */
	write32_device_func		dwrite;				/* pointer to native device write handler */
	read8_space_func		sread8;				/* pointer to 8-bit space read handler */
	write8_space_func		swrite8;			/* pointer to 8-bit space write handler */
	read8_device_func		dread8;				/* pointer to 8-bit device read handler */
	write8_device_func		dwrite8;			/* pointer to 8-bit device write handler */
	read16_space_func		sread16;			/* pointer to 16-bit space read handler */
	write16_space_func		swrite16;			/* pointer to 16-bit space write handler */
	read16_device_func		dread16;			/* pointer to 16-bit device read handler */
	write16_device_func		dwrite16;			/* pointer to 16-bit device write handler */
	read_handler			read;				/* generic read handlers */
	write_handler			write;				/* generic write handlers */
	UINT32 **				memptr;				/* memory pointer */
	size_t *				sizeptr;			/* size pointer */
};


/* addrmap64_token is a union of all types for a 64-bit address map */
typedef union _addrmap64_token addrmap64_token;
union _addrmap64_token
{
	TOKEN_COMMON_FIELDS
	const addrmap64_token *	tokenptr;
	read64_space_func		sread;				/* pointer to native read handler */
	write64_space_func 		swrite;				/* pointer to native write handler */
	read64_device_func		dread;				/* pointer to native device read handler */
	write64_device_func		dwrite;				/* pointer to native device write handler */
	read8_space_func		sread8;				/* pointer to 8-bit space read handler */
	write8_space_func		swrite8;			/* pointer to 8-bit space write handler */
	read8_device_func		dread8;				/* pointer to 8-bit device read handler */
	write8_device_func		dwrite8;			/* pointer to 8-bit device write handler */
	read16_space_func		sread16;			/* pointer to 16-bit space read handler */
	write16_space_func		swrite16;			/* pointer to 16-bit space write handler */
	read16_device_func		dread16;			/* pointer to 16-bit device read handler */
	write16_device_func		dwrite16;			/* pointer to 16-bit device write handler */
	read32_space_func		sread32;			/* pointer to 32-bit space read handler */
	write32_space_func		swrite32;			/* pointer to 32-bit space write handler */
	read32_device_func		dread32;			/* pointer to 32-bit device read handler */
	write32_device_func		dwrite32;			/* pointer to 32-bit device write handler */
	read_handler			read;				/* generic read handlers */
	write_handler			write;				/* generic write handlers */
	UINT64 **				memptr;				/* memory pointer */
	size_t *				sizeptr;			/* size pointer */
};



/***************************************************************************
    MACROS
***************************************************************************/

/* opcode base adjustment handler function macro */
#define DIRECT_UPDATE_HANDLER(name)		offs_t name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t address, direct_read_data *direct)


/* space read/write handler function macros */
#define READ8_HANDLER(name) 			UINT8  name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset)
#define WRITE8_HANDLER(name) 			void   name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data)
#define READ16_HANDLER(name)			UINT16 name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_HANDLER(name)			void   name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
#define READ32_HANDLER(name)			UINT32 name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask)
#define WRITE32_HANDLER(name)			void   name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask)
#define READ64_HANDLER(name)			UINT64 name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask)
#define WRITE64_HANDLER(name)			void   name(ATTR_UNUSED const address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask)


/* device read/write handler function macros */
#define READ8_DEVICE_HANDLER(name) 		UINT8  name(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset)
#define WRITE8_DEVICE_HANDLER(name) 	void   name(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data)
#define READ16_DEVICE_HANDLER(name)		UINT16 name(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
#define READ32_DEVICE_HANDLER(name)		UINT32 name(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask)
#define WRITE32_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask)
#define READ64_DEVICE_HANDLER(name)		UINT64 name(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask)
#define WRITE64_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED const device_config *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask)


/* static memory handler (SMH) macros that can be used in place of read/write handlers */
#define SMH_RAM							((void *)STATIC_RAM)
#define SMH_ROM							((void *)STATIC_ROM)
#define SMH_NOP							((void *)STATIC_NOP)
#define SMH_UNMAP						((void *)STATIC_UNMAP)
#define SMH_BANK(n)						((void *)(FPTR)(STATIC_BANK1 + (n) - 1))


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
#define memory_install_read_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler(space, start, end, mask, mirror, rhandler, (FPTR)NULL, #rhandler, NULL)
#define memory_install_read8_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler8(space, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)
#define memory_install_read16_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler16(space, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)
#define memory_install_read32_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler32(space, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)
#define memory_install_read64_handler(space, start, end, mask, mirror, rhandler) \
	_memory_install_handler64(space, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)

#define memory_install_read_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler(space, device, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)
#define memory_install_read8_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler8(space, device, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)
#define memory_install_read16_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler16(space, device, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)
#define memory_install_read32_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler32(space, device, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)
#define memory_install_read64_device_handler(space, device, start, end, mask, mirror, rhandler) \
	_memory_install_device_handler64(space, device, start, end, mask, mirror, rhandler, NULL, #rhandler, NULL)


/* wrappers for dynamic write handler installation */
#define memory_install_write_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler(space, start, end, mask, mirror, (FPTR)NULL, whandler, NULL, #whandler)
#define memory_install_write8_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler8(space, start, end, mask, mirror, NULL, whandler, NULL, #whandler)
#define memory_install_write16_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler16(space, start, end, mask, mirror, NULL, whandler, NULL, #whandler)
#define memory_install_write32_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler32(space, start, end, mask, mirror, NULL, whandler, NULL, #whandler)
#define memory_install_write64_handler(space, start, end, mask, mirror, whandler) \
	_memory_install_handler64(space, start, end, mask, mirror, NULL, whandler, NULL, #whandler)

#define memory_install_write_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler(space, device, start, end, mask, mirror, NULL, whandler, NULL, #whandler)
#define memory_install_write8_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler8(space, device, start, end, mask, mirror, NULL, whandler, NULL, #whandler)
#define memory_install_write16_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler16(space, device, start, end, mask, mirror, NULL, whandler, NULL, #whandler)
#define memory_install_write32_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler32(space, device, start, end, mask, mirror, NULL, whandler, NULL, #whandler)
#define memory_install_write64_device_handler(space, device, start, end, mask, mirror, whandler) \
	_memory_install_device_handler64(space, device, start, end, mask, mirror, NULL, whandler, NULL, #whandler)


/* wrappers for dynamic read/write handler installation */
#define memory_install_readwrite_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler(space, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)
#define memory_install_readwrite8_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler8(space, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)
#define memory_install_readwrite16_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler16(space, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)
#define memory_install_readwrite32_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler32(space, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)
#define memory_install_readwrite64_handler(space, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_handler64(space, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)

#define memory_install_readwrite_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler(space, device, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)
#define memory_install_readwrite8_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler8(space, device, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)
#define memory_install_readwrite16_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler16(space, device, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)
#define memory_install_readwrite32_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler32(space, device, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)
#define memory_install_readwrite64_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	_memory_install_device_handler64(space, device, start, end, mask, mirror, rhandler, whandler, #rhandler, #whandler)


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
#define WORD2_XOR_BE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(6,0))
#define WORD2_XOR_LE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,6))

/* read/write a dword to a 64-bit space */
#define DWORD_XOR_BE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(4,0))
#define DWORD_XOR_LE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,4))



/***************************************************************************
    ADDRESS MAP MACROS
***************************************************************************/

/* so that "0" can be used for unneeded address maps */
#define address_map_0 NULL


/* maps a full 64-bit mask down to an 8-bit byte mask */
#define UNITMASK8(x) \
	((((UINT64)(x) >> (63-7)) & 0x80) | \
	 (((UINT64)(x) >> (55-6)) & 0x40) | \
	 (((UINT64)(x) >> (47-5)) & 0x20) | \
	 (((UINT64)(x) >> (39-4)) & 0x10) | \
	 (((UINT64)(x) >> (31-3)) & 0x08) | \
	 (((UINT64)(x) >> (23-2)) & 0x04) | \
	 (((UINT64)(x) >> (15-1)) & 0x02) | \
	 (((UINT64)(x) >> ( 7-0)) & 0x01))

/* maps a full 64-bit mask down to a 4-bit word mask */
#define UNITMASK16(x) \
	((((UINT64)(x) >> (63-3)) & 0x08) | \
	 (((UINT64)(x) >> (47-2)) & 0x04) | \
	 (((UINT64)(x) >> (31-1)) & 0x02) | \
	 (((UINT64)(x) >> (15-0)) & 0x01))

/* maps a full 64-bit mask down to a 2-bit dword mask */
#define UNITMASK32(x) \
	((((UINT64)(x) >> (63-1)) & 0x02) | \
	 (((UINT64)(x) >> (31-0)) & 0x01))



/* start/end tags for the address map */
#define ADDRESS_MAP_NAME(_name) address_map_##_name
#define ADDRESS_MAP_START(_name, _space, _bits) \
	const addrmap##_bits##_token ADDRESS_MAP_NAME(_name)[] = { \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_START, 8, _space, 8, _bits, 8),

#define ADDRESS_MAP_END \
	TOKEN_UINT32_PACK1(ADDRMAP_TOKEN_END, 8) };

/* use this to declare external references to an address map */
#define ADDRESS_MAP_EXTERN(_name, _bits) \
	extern const addrmap##_bits##_token ADDRESS_MAP_NAME(_name)[]


/* global controls */
#define ADDRESS_MAP_GLOBAL_MASK(_mask) \
	TOKEN_UINT64_PACK2(ADDRMAP_TOKEN_GLOBAL_MASK, 8, _mask, 32),

#define ADDRESS_MAP_UNMAP_LOW \
	TOKEN_UINT32_PACK2(ADDRMAP_TOKEN_UNMAP_VALUE, 8, 0, 1),

#define ADDRESS_MAP_UNMAP_HIGH \
	TOKEN_UINT32_PACK2(ADDRMAP_TOKEN_UNMAP_VALUE, 8, 1, 1),


/* importing data from other address maps */
#define AM_IMPORT_FROM(_name) \
	TOKEN_UINT32_PACK1(ADDRMAP_TOKEN_INCLUDE, 8), \
	TOKEN_PTR(tokenptr, ADDRESS_MAP_NAME(_name)),


/* address ranges */
#define AM_RANGE(_start, _end) \
	TOKEN_UINT32_PACK1(ADDRMAP_TOKEN_RANGE, 8), \
	TOKEN_UINT64_PACK2(_start, 32, _end, 32),

#define AM_MASK(_mask) \
	TOKEN_UINT64_PACK2(ADDRMAP_TOKEN_MASK, 8, _mask, 32),

#define AM_MIRROR(_mirror) \
	TOKEN_UINT64_PACK2(ADDRMAP_TOKEN_MIRROR, 8, _mirror, 32),

#define AM_READ(_handler) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_READ, 8, 0, 8, 0, 8), \
	TOKEN_PTR(sread, _handler), \
	TOKEN_STRING(#_handler),

#define AM_READ8(_handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_READ, 8, 8, 8, UNITMASK8(_unitmask), 8), \
	TOKEN_PTR(sread8, _handler), \
	TOKEN_STRING(#_handler),

#define AM_READ16(_handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_READ, 8, 16, 8, UNITMASK16(_unitmask), 8), \
	TOKEN_PTR(sread16, _handler), \
	TOKEN_STRING(#_handler),

#define AM_READ32(_handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_READ, 8, 32, 8, UNITMASK32(_unitmask), 8), \
	TOKEN_PTR(sread32, _handler), \
	TOKEN_STRING(#_handler),

#define AM_WRITE(_handler) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_WRITE, 8, 0, 8, 0, 8), \
	TOKEN_PTR(swrite, _handler), \
	TOKEN_STRING(#_handler),

#define AM_WRITE8(_handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_WRITE, 8, 8, 8, UNITMASK8(_unitmask), 8), \
	TOKEN_PTR(swrite8, _handler), \
	TOKEN_STRING(#_handler),

#define AM_WRITE16(_handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_WRITE, 8, 16, 8, UNITMASK16(_unitmask), 8), \
	TOKEN_PTR(swrite16, _handler), \
	TOKEN_STRING(#_handler),

#define AM_WRITE32(_handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_WRITE, 8, 32, 8, UNITMASK32(_unitmask), 8), \
	TOKEN_PTR(swrite32, _handler), \
	TOKEN_STRING(#_handler),

#define AM_DEVREAD(_tag, _handler) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_DEVICE_READ, 8, 0, 8, 0, 8), \
	TOKEN_PTR(dread, _handler), \
	TOKEN_STRING(#_handler), \
	TOKEN_STRING(_tag),

#define AM_DEVREAD8(_tag, _handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_DEVICE_READ, 8, 8, 8, UNITMASK8(_unitmask), 8), \
	TOKEN_PTR(dread8, _handler), \
	TOKEN_STRING(#_handler), \
	TOKEN_STRING(_tag),

#define AM_DEVREAD16(_tag, _handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_DEVICE_READ, 8, 16, 8, UNITMASK16(_unitmask), 8), \
	TOKEN_PTR(dread16, _handler), \
	TOKEN_STRING(#_handler), \
	TOKEN_STRING(_tag),

#define AM_DEVREAD32(_tag, _handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_DEVICE_READ, 8, 32, 8, UNITMASK32(_unitmask), 8), \
	TOKEN_PTR(dread32, _handler), \
	TOKEN_STRING(#_handler), \
	TOKEN_STRING(_tag),

#define AM_DEVWRITE(_tag, _handler) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_DEVICE_WRITE, 8, 0, 8, 0, 8), \
	TOKEN_PTR(dwrite, _handler), \
	TOKEN_STRING(#_handler), \
	TOKEN_STRING(_tag),

#define AM_DEVWRITE8(_tag, _handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_DEVICE_WRITE, 8, 8, 8, UNITMASK8(_unitmask), 8), \
	TOKEN_PTR(dwrite8, _handler), \
	TOKEN_STRING(#_handler), \
	TOKEN_STRING(_tag),

#define AM_DEVWRITE16(_tag, _handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_DEVICE_WRITE, 8, 16, 8, UNITMASK16(_unitmask), 8), \
	TOKEN_PTR(dwrite16, _handler), \
	TOKEN_STRING(#_handler), \
	TOKEN_STRING(_tag),

#define AM_DEVWRITE32(_tag, _handler, _unitmask) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_DEVICE_WRITE, 8, 32, 8, UNITMASK32(_unitmask), 8), \
	TOKEN_PTR(dwrite32, _handler), \
	TOKEN_STRING(#_handler), \
	TOKEN_STRING(_tag),

#define AM_READ_PORT(_tag) \
	TOKEN_UINT32_PACK3(ADDRMAP_TOKEN_READ_PORT, 8, 0, 8, 0, 8), \
	TOKEN_STRING(_tag),

#define AM_REGION(_tag, _offs) \
	TOKEN_UINT64_PACK2(ADDRMAP_TOKEN_REGION, 8, _offs, 32), \
	TOKEN_STRING(_tag),

#define AM_SHARE(_index) \
	TOKEN_UINT32_PACK2(ADDRMAP_TOKEN_SHARE, 8, _index, 24),

#define AM_BASE(_base) \
	TOKEN_UINT32_PACK1(ADDRMAP_TOKEN_BASEPTR, 8), \
	TOKEN_PTR(memptr, _base),

#define AM_BASE_MEMBER(_struct, _member) \
	TOKEN_UINT32_PACK2(ADDRMAP_TOKEN_BASE_MEMBER, 8, offsetof(_struct, _member), 24),

#define AM_SIZE(_size) \
	TOKEN_UINT32_PACK1(ADDRMAP_TOKEN_SIZEPTR, 8), \
	TOKEN_PTR(sizeptr, _size),

#define AM_SIZE_MEMBER(_struct, _member) \
	TOKEN_UINT32_PACK2(ADDRMAP_TOKEN_SIZE_MEMBER, 8, offsetof(_struct, _member), 24),


/* common shortcuts */
#define AM_READWRITE(_read,_write)			AM_READ(_read) AM_WRITE(_write)
#define AM_READWRITE8(_read,_write,_mask)	AM_READ8(_read,_mask) AM_WRITE8(_write,_mask)
#define AM_READWRITE16(_read,_write,_mask)	AM_READ16(_read,_mask) AM_WRITE16(_write,_mask)
#define AM_READWRITE32(_read,_write,_mask)	AM_READ32(_read,_mask) AM_WRITE32(_write,_mask)

#define AM_DEVREADWRITE(_tag,_read,_write) AM_DEVREAD(_tag,_read) AM_DEVWRITE(_tag,_write)
#define AM_DEVREADWRITE8(_tag,_read,_write,_mask) AM_DEVREAD8(_tag,_read,_mask) AM_DEVWRITE8(_tag,_write,_mask)
#define AM_DEVREADWRITE16(_tag,_read,_write,_mask) AM_DEVREAD16(_tag,_read,_mask) AM_DEVWRITE16(_tag,_write,_mask)
#define AM_DEVREADWRITE32(_tag,_read,_write,_mask) AM_DEVREAD32(_tag,_read,_mask) AM_DEVWRITE32(_tag,_write,_mask)

#define AM_ROM								AM_READ(SMH_ROM)
#define AM_ROMBANK(_bank)					AM_READ(SMH_BANK(_bank))

#define AM_RAM								AM_READWRITE(SMH_RAM, SMH_RAM)
#define AM_RAMBANK(_bank)					AM_READWRITE(SMH_BANK(_bank), SMH_BANK(_bank))
#define AM_RAM_WRITE(_write)				AM_READWRITE(SMH_RAM, _write)
#define AM_WRITEONLY						AM_WRITE(SMH_RAM)

#define AM_UNMAP							AM_READWRITE(SMH_UNMAP, SMH_UNMAP)
#define AM_NOP								AM_READWRITE(SMH_NOP, SMH_NOP)
#define AM_READNOP							AM_READ(SMH_NOP)
#define AM_WRITENOP							AM_WRITE(SMH_NOP)



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

/* find an address space in our internal list; for faster access use device->space[] after device is started */
const address_space *memory_find_address_space(const device_config *device, int spacenum);



/* ----- address maps ----- */

/* build and allocate an address map for a device's address space */
address_map *address_map_alloc(const device_config *device, const game_driver *driver, int spacenum);

/* release allocated memory for an address map */
void address_map_free(address_map *map);



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
void memory_configure_bank(running_machine *machine, int banknum, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(1, 5);

/* configure the decrypted addresses for a bank */
void memory_configure_bank_decrypted(running_machine *machine, int banknum, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(1, 5);

/* select one pre-configured entry to be the new bank base */
void memory_set_bank(running_machine *machine, int banknum, int entrynum) ATTR_NONNULL(1);

/* return the currently selected bank */
int memory_get_bank(running_machine *machine, int banknum) ATTR_NONNULL(1);

/* set the absolute address of a bank base */
void memory_set_bankptr(running_machine *machine, int banknum, void *base) ATTR_NONNULL(1, 3);

/* return the index of an unused bank */
int memory_find_unused_bank(running_machine *machine) ATTR_NONNULL(1);



/* ----- dynamic address space mapping ----- */

/* install a new memory handler into the given address space, returning a pointer to the memory backing it, if present */
void *_memory_install_handler(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, FPTR rhandler, FPTR whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1);

/* same as above but explicitly for 8-bit handlers */
UINT8 *_memory_install_handler8(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, write8_space_func whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1);

/* same as above but explicitly for 16-bit handlers */
UINT16 *_memory_install_handler16(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, write16_space_func whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1);

/* same as above but explicitly for 32-bit handlers */
UINT32 *_memory_install_handler32(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, write32_space_func whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1);

/* same as above but explicitly for 64-bit handlers */
UINT64 *_memory_install_handler64(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, write64_space_func whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1);

/* install a new device memory handler into the given address space, returning a pointer to the memory backing it, if present */
void *_memory_install_device_handler(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, FPTR rhandler, FPTR whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1, 2);

/* same as above but explicitly for 8-bit handlers */
UINT8 *_memory_install_device_handler8(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, write8_device_func whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1, 2);

/* same as above but explicitly for 16-bit handlers */
UINT16 *_memory_install_device_handler16(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, write16_device_func whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1, 2);

/* same as above but explicitly for 32-bit handlers */
UINT32 *_memory_install_device_handler32(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, write32_device_func whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1, 2);

/* same as above but explicitly for 64-bit handlers */
UINT64 *_memory_install_device_handler64(const address_space *space, const device_config *device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, write64_device_func whandler, const char *rhandler_name, const char *whandler_name) ATTR_NONNULL(1, 2);

/* install a new input port handler into the given address space */
void memory_install_read_port_handler(const address_space *space, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *tag) ATTR_NONNULL(1, 6);



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
