/***************************************************************************

    memory.c

    Functions which handle device memory access.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

    AM_ROM
        Specifies that this bucket contains ROM data by attaching an
        internal read handler. If this address space describes the first
        address space for a device, and if there is a region whose name
        matches the device's name, and if the bucket start/end range is
        within the bounds of that region, then this bucket will automatically
        map to the memory contained in that region.

    AM_RAM
    AM_READONLY
    AM_WRITEONLY
        Specifies that this bucket contains RAM data by attaching internal
        read and/or write handlers. Memory is automatically allocated to back
        this area. AM_RAM maps both reads and writes, while AM_READONLY only
        maps reads and AM_WRITEONLY only maps writes.

    AM_NOP
    AM_READNOP
    AM_WRITENOP
        Specifies that reads and/or writes in this bucket are unmapped, but
        that accesses to them should not be logged. AM_NOP unmaps both reads
        and writes, while AM_READNOP only unmaps reads, and AM_WRITENOP only
        unmaps writes.

    AM_UNMAP
        Specifies that both reads and writes in thus bucket are unmapeed,
        and that accesses to them should be logged. There is rarely a need
        for this, as the entire address space is initialized to behave this
        way by default.

    AM_READ_BANK(tag)
    AM_WRITE_BANK(tag)
    AM_READWRITE_BANK(tag)
        Specifies that reads and/or writes in this bucket map to a memory
        bank with the provided 'tag'. The actual memory this bank points to
        can be later controlled via the same tag.

    AM_READ(read)
    AM_WRITE(write)
    AM_READWRITE(read, write)
        Specifies read and/or write handler callbacks for this bucket. All
        reads and writes in this bucket will trigger a call to the provided
        functions.

    AM_DEVREAD(tag, read)
    AM_DEVWRITE(tag, read)
    AM_DEVREADWRITE(tag, read)
        Specifies a device-specific read and/or write handler for this
        bucket, automatically bound to the device specified by the provided
        'tag'.

    AM_READ_PORT(tag)
    AM_WRITE_PORT(tag)
    AM_READWRITE_PORT(tag)
        Specifies that read and/or write accesses in this bucket will map
        to the I/O port with the provided 'tag'. An internal read/write
        handler is set up to handle this mapping.

    AM_REGION(class, tag, offs)
        Only useful if used in conjunction with AM_ROM, AM_RAM, or
        AM_READ/WRITE_BANK. By default, memory is allocated to back each
        bucket. By specifying AM_REGION, you can tell the memory system to
        point the base of the memory backing this bucket to a given memory
        'region' at the specified 'offs' instead of allocating it.

    AM_SHARE(tag)
        Similar to AM_REGION, this specifies that the memory backing the
        current bucket is shared with other buckets. The first bucket to
        specify the share 'tag' will use its memory as backing for all
        future buckets that specify AM_SHARE with the same 'tag'.

    AM_BASE(base)
        Specifies a pointer to a pointer to the base of the memory backing
        the current bucket.

    AM_SIZE(size)
        Specifies a pointer to a size_t variable which will be filled in
        with the size, in bytes, of the current bucket.

    AM_BASE_MEMBER(struct, basefield)
    AM_SIZE_MEMBER(struct, sizefield)
        Specifies a field within a given struct as where to store the base
        or size of the current bucket. The struct is assumed to be hanging
        off of the machine->driver_data pointer.

    AM_BASE_GENERIC(basefield)
    AM_SIZE_GENERIC(sizefield)
        Specifies a field within the global generic_pointers struct as
        where to store the base or size of the current bucket. The global
        generic_pointer struct lives in machine->generic.

***************************************************************************/

#include "emu.h"
#include "profiler.h"
#include "debug/debugcpu.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define MEM_DUMP		(0)
#define VERBOSE			(0)
#define TEST_HANDLER	(0)

#define VPRINTF(x)	do { if (VERBOSE) printf x; } while (0)



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// banking constants
const int BANK_ENTRY_UNSPECIFIED = -1;

// shares are initially mapped to this invalid pointer
static void *UNMAPPED_SHARE_PTR = ((void *)-1);

// other address map constants
const int MEMORY_BLOCK_CHUNK = 65536;					// minimum chunk size of allocated memory blocks

// static data access handler constants
enum
{
	STATIC_INVALID = 0,									// invalid - should never be used
	STATIC_BANK1 = 1,									// first memory bank
	STATIC_BANKMAX = 122,								// last memory bank
	STATIC_RAM,											// RAM - reads/writes map to dynamic banks
	STATIC_ROM,											// ROM - reads = RAM; writes = UNMAP
	STATIC_NOP,											// NOP - reads = unmapped value; writes = no-op
	STATIC_UNMAP,										// unmapped - same as NOP except we log errors
	STATIC_WATCHPOINT,									// watchpoint - used internally
	STATIC_COUNT										// total number of static handlers
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> memory_block

// a memory block is a chunk of RAM associated with a range of memory in a device's address space
class memory_block
{
	DISABLE_COPYING(memory_block);

	friend class simple_list<memory_block>;
	friend resource_pool_object<memory_block>::~resource_pool_object();

public:
	// construction/destruction
	memory_block(address_space &space, offs_t bytestart, offs_t byteend, void *memory = NULL);
	~memory_block();

	// getters
	memory_block *next() const { return m_next; }
	offs_t bytestart() const { return m_bytestart; }
	offs_t byteend() const { return m_byteend; }
	UINT8 *data() const { return m_data; }

	// is the given range contained by this memory block?
	bool contains(address_space &space, offs_t bytestart, offs_t byteend) const
	{
		return (&space == &m_space && m_bytestart <= bytestart && m_byteend >= byteend);
	}

private:
	// internal state
	memory_block *			m_next;					// next memory block in the list
	running_machine &		m_machine;				// need the machine to free our memory
	address_space &			m_space;				// which address space are we associated with?
	bool					m_isallocated;			// did we allocate this ourselves?
	offs_t					m_bytestart, m_byteend;	// byte-normalized start/end for verifying a match
	UINT8 *					m_data;					// pointer to the data for this block
};


// ======================> memory_bank

// a memory bank is a global pointer to memory that can be shared across devices and changed dynamically
class memory_bank
{
	friend class simple_list<memory_bank>;
	friend resource_pool_object<memory_bank>::~resource_pool_object();

	// a bank reference is an entry in a list of address spaces that reference a given bank
	class bank_reference
	{
		friend class simple_list<bank_reference>;
		friend resource_pool_object<bank_reference>::~resource_pool_object();

	public:
		// construction/destruction
		bank_reference(address_space &space, read_or_write readorwrite)
			: m_next(NULL),
			  m_space(space),
			  m_readorwrite(readorwrite) { }

		// getters
		bank_reference *next() const { return m_next; }
		address_space &space() const { return m_space; }

		// does this reference match the space+read/write combination?
		bool matches(address_space &space, read_or_write readorwrite) const
		{
			return (&space == &m_space && (readorwrite == ROW_READWRITE || readorwrite == m_readorwrite));
		}

	private:
		// internal state
		bank_reference *		m_next;				// link to the next reference
		address_space &			m_space;			// address space that references us
		read_or_write			m_readorwrite;		// used for read or write?
	};

	// a bank_entry contains a raw and decrypted pointer
	struct bank_entry
	{
		UINT8 *			m_raw;
		UINT8 *			m_decrypted;
	};

public:
	// construction/destruction
	memory_bank(address_space &space, int index, offs_t bytestart, offs_t byteend, const char *tag = NULL);
	~memory_bank();

	// getters
	memory_bank *next() const { return m_next; }
	int index() const { return m_index; }
	int entry() const { return m_curentry; }
	bool anonymous() const { return m_anonymous; }
	offs_t bytestart() const { return m_bytestart; }
	void *base() const { return *m_baseptr; }
	void *base_decrypted() const { return *m_basedptr; }
	const char *tag() const { return m_tag; }
	const char *name() const { return m_name; }

	// compare a range against our range
	bool matches_exactly(offs_t bytestart, offs_t byteend) const { return (m_bytestart == bytestart && m_byteend == byteend); }
	bool fully_covers(offs_t bytestart, offs_t byteend) const { return (m_bytestart <= bytestart && m_byteend >= byteend); }
	bool is_covered_by(offs_t bytestart, offs_t byteend) const { return (m_bytestart >= bytestart && m_byteend <= byteend); }
	bool straddles(offs_t bytestart, offs_t byteend) const { return (m_bytestart < byteend && m_byteend > bytestart); }

	// track and verify address space references to this bank
	bool references_space(address_space &space, read_or_write readorwrite) const;
	void add_reference(address_space &space, read_or_write readorwrite);

	// set the base explicitly
	void set_base(void *base);
	void set_base_decrypted(void *base);

	// configure and set entries
	void configure(int entrynum, void *base);
	void configure_decrypted(int entrynum, void *base);
	void set_entry(int entrynum);

private:
	// internal helpers
	void invalidate_references();
	void expand_entries(int entrynum);

	// internal state
	memory_bank *			m_next;					// next bank in sequence
	running_machine &		m_machine;				// need the machine to free our memory
	UINT8 **				m_baseptr;				// pointer to our base pointer in the global array
	UINT8 **				m_basedptr;				// same for the decrypted base pointer
	UINT8					m_index;				// array index for this handler
	bool					m_anonymous;			// are we anonymous or explicit?
	offs_t					m_bytestart;			// byte-adjusted start offset
	offs_t					m_byteend;				// byte-adjusted end offset
	int						m_curentry;				// current entry
	bank_entry *			m_entry;				// array of entries (dynamically allocated)
	int						m_entry_count;			// number of allocated entries
	astring					m_name;					// friendly name for this bank
	astring					m_tag;					// tag for this bank
	simple_list<bank_reference> m_reflist;			// linked list of address spaces referencing this bank
};


// ======================> handler_entry

// a handler entry contains information about a memory handler
class handler_entry : public bindable_object
{
	DISABLE_COPYING(handler_entry);

protected:
	// construction/destruction
	handler_entry(UINT8 width, endianness_t endianness, UINT8 **rambaseptr);
	virtual ~handler_entry();

public:
	// getters
	bool populated() const { return m_populated; }
	offs_t bytestart() const { return m_bytestart; }
	offs_t byteend() const { return m_byteend; }
	offs_t bytemask() const { return m_bytemask; }
	virtual const char *name() const = 0;

	// return offset within the range referenced by this handler
	offs_t byteoffset(offs_t byteaddress) const { return (byteaddress - m_bytestart) & m_bytemask; }

	// return a pointer to the backing RAM at the given offset
	UINT8 *ramptr(offs_t offset = 0) const { return *m_rambaseptr + offset; }

	// see if we are an exact match to the given parameters
	bool matches_exactly(offs_t bytestart, offs_t byteend, offs_t bytemask) const
	{
		return (m_populated && m_bytestart == bytestart && m_byteend == byteend && m_bytemask == bytemask);
	}

	// get the start/end address with the given mirror
	void mirrored_start_end(offs_t byteaddress, offs_t &start, offs_t &end) const
	{
		offs_t mirrorbits = (byteaddress - m_bytestart) & ~m_bytemask;
		start = m_bytestart | mirrorbits;
		end = m_byteend | mirrorbits;
	}

	// configure the handler addresses, and mark as populated
	void configure(offs_t bytestart, offs_t byteend, offs_t bytemask)
	{
		m_populated = true;
		m_bytestart = bytestart;
		m_byteend = byteend;
		m_bytemask = bytemask;
	}

	// apply a global mask
	void apply_mask(offs_t bytemask) { m_bytemask &= bytemask; }

protected:
	// internal helpers
	void configure_subunits(UINT64 handlermask, int handlerbits);

	// internal state
	bool					m_populated;			// populated?
	UINT8					m_datawidth;
	endianness_t			m_endianness;
	offs_t					m_bytestart;			// byte-adjusted start address for handler
	offs_t					m_byteend;				// byte-adjusted end address for handler
	offs_t					m_bytemask;				// byte-adjusted mask against the final address
	UINT8 **				m_rambaseptr;			// pointer to the bank base
	UINT8					m_subunits;				// for width stubs, the number of subunits
	UINT8					m_subshift[8];			// for width stubs, the shift of each subunit
};


// ======================> handler_entry_read

// a read-access-specific extension of handler_entry
class handler_entry_read : public handler_entry
{
public:
	// construction/destruction
	handler_entry_read(UINT8 width, endianness_t endianness, UINT8 **rambaseptr)
		: handler_entry(width, endianness, rambaseptr)
	{
		m_legacy_object.space = NULL;
		m_legacy_handler.space8 = NULL;
	}

	// getters
	virtual const char *name() const;

	// configure delegate callbacks
	void set_delegate(read8_delegate delegate, UINT64 mask = 0);
	void set_delegate(read16_delegate delegate, UINT64 mask = 0);
	void set_delegate(read32_delegate delegate, UINT64 mask = 0);
	void set_delegate(read64_delegate delegate, UINT64 mask = 0);

	// configure legacy address space functions
	void set_legacy_func(address_space &space, read8_space_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(address_space &space, read16_space_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(address_space &space, read32_space_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(address_space &space, read64_space_func func, const char *name, UINT64 mask = 0);

	// configure legacy device functions
	void set_legacy_func(device_t &device, read8_device_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(device_t &device, read16_device_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(device_t &device, read32_device_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(device_t &device, read64_device_func func, const char *name, UINT64 mask = 0);

	// configure I/O port access
	void set_ioport(const input_port_config &ioport);

	// read via the underlying delegates
	UINT8 read8(address_space &space, offs_t offset, UINT8 mask) const { return m_read8(space, offset, mask); }
	UINT16 read16(address_space &space, offs_t offset, UINT16 mask) const { return m_read16(space, offset, mask); }
	UINT32 read32(address_space &space, offs_t offset, UINT32 mask) const { return m_read32(space, offset, mask); }
	UINT64 read64(address_space &space, offs_t offset, UINT64 mask) const { return m_read64(space, offset, mask); }

private:
	// stubs for converting between address sizes
	UINT16 read_stub_16_from_8(address_space &space, offs_t offset, UINT16 mask);
	UINT32 read_stub_32_from_8(address_space &space, offs_t offset, UINT32 mask);
	UINT64 read_stub_64_from_8(address_space &space, offs_t offset, UINT64 mask);
	UINT32 read_stub_32_from_16(address_space &space, offs_t offset, UINT32 mask);
	UINT64 read_stub_64_from_16(address_space &space, offs_t offset, UINT64 mask);
	UINT64 read_stub_64_from_32(address_space &space, offs_t offset, UINT64 mask);

	// stubs for calling legacy read handlers
	UINT8 read_stub_legacy(address_space &space, offs_t offset, UINT8 mask);
	UINT16 read_stub_legacy(address_space &space, offs_t offset, UINT16 mask);
	UINT32 read_stub_legacy(address_space &space, offs_t offset, UINT32 mask);
	UINT64 read_stub_legacy(address_space &space, offs_t offset, UINT64 mask);

	// stubs for reading I/O ports
	template<typename _UintType>
	_UintType read_stub_ioport(address_space &space, offs_t offset, _UintType mask) { return input_port_read_direct(m_ioport); }

	// internal state
	read8_delegate				m_read8;
	read16_delegate				m_read16;
	read32_delegate				m_read32;
	read64_delegate				m_read64;
	const input_port_config *	m_ioport;

	// unions to hold legacy objects and callbacks
	union
	{
		address_space *	space;
		device_t *				device;
	} m_legacy_object;

	union
	{
		read8_space_func		space8;
		read16_space_func		space16;
		read32_space_func		space32;
		read64_space_func		space64;
		read8_device_func		device8;
		read16_device_func		device16;
		read32_device_func		device32;
		read64_device_func		device64;
	} m_legacy_handler;
};


// ======================> handler_entry_write

// a write-access-specific extension of handler_entry
class handler_entry_write : public handler_entry
{
public:
	// construction/destruction
	handler_entry_write(UINT8 width, endianness_t endianness, UINT8 **rambaseptr)
		: handler_entry(width, endianness, rambaseptr)
	{
		m_legacy_object.space = NULL;
		m_legacy_handler.space8 = NULL;
	}

	// getters
	virtual const char *name() const;

	// configure delegate callbacks
	void set_delegate(write8_delegate delegate, UINT64 mask = 0);
	void set_delegate(write16_delegate delegate, UINT64 mask = 0);
	void set_delegate(write32_delegate delegate, UINT64 mask = 0);
	void set_delegate(write64_delegate delegate, UINT64 mask = 0);

	// configure legacy address space functions
	void set_legacy_func(address_space &space, write8_space_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(address_space &space, write16_space_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(address_space &space, write32_space_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(address_space &space, write64_space_func func, const char *name, UINT64 mask = 0);

	// configure legacy device functions
	void set_legacy_func(device_t &device, write8_device_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(device_t &device, write16_device_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(device_t &device, write32_device_func func, const char *name, UINT64 mask = 0);
	void set_legacy_func(device_t &device, write64_device_func func, const char *name, UINT64 mask = 0);

	// configure I/O port access
	void set_ioport(const input_port_config &ioport);

	// write via the underlying delegates
	void write8(address_space &space, offs_t offset, UINT8 data, UINT8 mask) const { m_write8(space, offset, data, mask); }
	void write16(address_space &space, offs_t offset, UINT16 data, UINT16 mask) const { m_write16(space, offset, data, mask); }
	void write32(address_space &space, offs_t offset, UINT32 data, UINT32 mask) const { m_write32(space, offset, data, mask); }
	void write64(address_space &space, offs_t offset, UINT64 data, UINT64 mask) const { m_write64(space, offset, data, mask); }

private:
	// stubs for converting between address sizes
	void write_stub_16_from_8(address_space &space, offs_t offset, UINT16 data, UINT16 mask);
	void write_stub_32_from_8(address_space &space, offs_t offset, UINT32 data, UINT32 mask);
	void write_stub_64_from_8(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write_stub_32_from_16(address_space &space, offs_t offset, UINT32 data, UINT32 mask);
	void write_stub_64_from_16(address_space &space, offs_t offset, UINT64 data, UINT64 mask);
	void write_stub_64_from_32(address_space &space, offs_t offset, UINT64 data, UINT64 mask);

	// stubs for calling legacy write handlers
	void write_stub_legacy(address_space &space, offs_t offset, UINT8 data, UINT8 mask);
	void write_stub_legacy(address_space &space, offs_t offset, UINT16 data, UINT16 mask);
	void write_stub_legacy(address_space &space, offs_t offset, UINT32 data, UINT32 mask);
	void write_stub_legacy(address_space &space, offs_t offset, UINT64 data, UINT64 mask);

	// stubs for writing I/O ports
	template<typename _UintType>
	void write_stub_ioport(address_space &space, offs_t offset, _UintType data, _UintType mask) { input_port_write_direct(m_ioport, data, mask); }

	// internal state
	write8_delegate				m_write8;
	write16_delegate			m_write16;
	write32_delegate			m_write32;
	write64_delegate			m_write64;
	const input_port_config *	m_ioport;

	// unions to hold legacy objects and callbacks
	union
	{
		address_space *	space;
		device_t *				device;
	} m_legacy_object;

	union
	{
		write8_space_func		space8;
		write16_space_func		space16;
		write32_space_func		space32;
		write64_space_func		space64;
		write8_device_func		device8;
		write16_device_func		device16;
		write32_device_func		device32;
		write64_device_func		device64;
	} m_legacy_handler;
};


// ======================> address_table

// address_table contains information about read/write accesses within an address space
class address_table : public bindable_object
{
	// address map lookup table definitions
	static const int LEVEL1_BITS	= 18;						// number of address bits in the level 1 table
	static const int LEVEL2_BITS	= 32 - LEVEL1_BITS;			// number of address bits in the level 2 table
	static const int SUBTABLE_COUNT	= 64;						// number of slots reserved for subtables
	static const int SUBTABLE_BASE	= 256 - SUBTABLE_COUNT;		// first index of a subtable
	static const int ENTRY_COUNT	= SUBTABLE_BASE;			// number of legitimate (non-subtable) entries
	static const int SUBTABLE_ALLOC	= 8;						// number of subtables to allocate at a time

	inline int level2_bits() const { return m_large ? LEVEL2_BITS : 0; }

public:
	// construction/destruction
	address_table(address_space &space, bool large);
	~address_table();

	// getters
	virtual handler_entry &handler(UINT32 index) const = 0;
	bool watchpoints_enabled() const { return (m_live_lookup == s_watchpoint_table); }

	// address lookups
	UINT32 lookup_live(offs_t byteaddress) const { return m_large ? lookup_live_large(byteaddress) : lookup_live_small(byteaddress); }
	UINT32 lookup_live_small(offs_t byteaddress) const { return m_live_lookup[byteaddress]; }

	UINT32 lookup_live_large(offs_t byteaddress) const
	{
		UINT32 entry = m_live_lookup[level1_index_large(byteaddress)];
		if (entry >= SUBTABLE_BASE)
			entry = m_live_lookup[level2_index_large(entry, byteaddress)];
		return entry;
	}

	UINT32 lookup(offs_t byteaddress) const
	{
		UINT32 entry = m_live_lookup[level1_index(byteaddress)];
		if (entry >= SUBTABLE_BASE)
			entry = m_live_lookup[level2_index(entry, byteaddress)];
		return entry;
	}

	// enable watchpoints by swapping in the watchpoint table
	void enable_watchpoints(bool enable = true) { m_live_lookup = enable ? s_watchpoint_table : m_table; }

	// table mapping helpers
	UINT8 map_range(offs_t bytestart, offs_t byteend, offs_t bytemask, offs_t bytemirror, UINT8 staticentry = 0);
	UINT8 derive_range(offs_t byteaddress, offs_t &bytestart, offs_t &byteend) const;

	// misc helpers
	void mask_all_handlers(offs_t mask);
	const char *handler_name(UINT8 entry) const;

protected:
	// determine table indexes based on the address
	UINT32 level1_index_large(offs_t address) const { return address >> LEVEL2_BITS; }
	UINT32 level2_index_large(UINT8 l1entry, offs_t address) const { return (1 << LEVEL1_BITS) + ((l1entry - SUBTABLE_BASE) << LEVEL2_BITS) + (address & ((1 << LEVEL2_BITS) - 1)); }
	UINT32 level1_index(offs_t address) const { return m_large ? level1_index_large(address) : address; }
	UINT32 level2_index(UINT8 l1entry, offs_t address) const { return m_large ? level2_index_large(l1entry, address) : 0; }

	// table population/depopulation
	void populate_range_mirrored(offs_t bytestart, offs_t byteend, offs_t bytemirror, UINT8 handler);
	void populate_range(offs_t bytestart, offs_t byteend, UINT8 handler);
	void depopulate_unused();

	// subtable management
	UINT8 subtable_alloc();
	void subtable_realloc(UINT8 subentry);
	int subtable_merge();
	void subtable_release(UINT8 subentry);
	UINT8 *subtable_open(offs_t l1index);
	void subtable_close(offs_t l1index);
	UINT8 *subtable_ptr(UINT8 entry) { return &m_table[level2_index(entry, 0)]; }

	// internal state
	UINT8 *					m_table;					// pointer to base of table
	UINT8 *					m_live_lookup;				// current lookup
	address_space &			m_space;					// pointer back to the space
	bool					m_large;					// large memory model?

	// subtable_data is an internal class with information about each subtable
	class subtable_data
	{
	public:
		subtable_data()
			: m_checksum_valid(false),
			  m_checksum(0),
			  m_usecount(0) { }

		bool				m_checksum_valid;			// is the checksum valid
		UINT32				m_checksum;					// checksum over all the bytes
		UINT32				m_usecount;					// number of times this has been used
	};
	subtable_data *			m_subtable;					// info about each subtable
	UINT8					m_subtable_alloc;			// number of subtables allocated

	// static global read-only watchpoint table
	static UINT8			s_watchpoint_table[1 << LEVEL1_BITS];
};


// ======================> address_table_read

// read access-specific version of an address table
class address_table_read : public address_table
{
public:
	// construction/destruction
	address_table_read(address_space &space, bool large);
	~address_table_read();

	// getters
	virtual handler_entry &handler(UINT32 index) const;
	handler_entry_read &handler_read(UINT32 index) const { assert(index < ARRAY_LENGTH(m_handlers)); return *m_handlers[index]; }

private:
	// internal unmapped handler
	template<typename _UintType>
	_UintType unmap_r(address_space &space, offs_t offset, _UintType mask)
	{
		if (m_space.log_unmap() && !m_space.debugger_access())
			logerror("%s: unmapped %s memory read from %s & %s\n",
						cpuexec_describe_context(&m_space.m_machine), m_space.name(),
						core_i64_hex_format(m_space.byte_to_address(offset * sizeof(_UintType)), m_space.addrchars()),
						core_i64_hex_format(mask, 2 * sizeof(_UintType)));
		return m_space.unmap();
	}

	// internal no-op handler
	template<typename _UintType>
	_UintType nop_r(address_space &space, offs_t offset, _UintType mask)
	{
		return m_space.unmap();
	}

	// internal watchpoint handler
	template<typename _UintType>
	_UintType watchpoint_r(address_space &space, offs_t offset, _UintType mask)
	{
		m_space.device().debug()->memory_read_hook(m_space, offset * sizeof(_UintType), mask);

		UINT8 *oldtable = m_live_lookup;
		m_live_lookup = m_table;
		_UintType result;
		if (sizeof(_UintType) == 1) result = m_space.read_byte(offset);
		if (sizeof(_UintType) == 2) result = m_space.read_word(offset << 1, mask);
		if (sizeof(_UintType) == 4) result = m_space.read_dword(offset << 2, mask);
		if (sizeof(_UintType) == 8) result = m_space.read_qword(offset << 3, mask);
		m_live_lookup = oldtable;
		return result;
	}

	// internal state
	handler_entry_read *		m_handlers[256];		// array of user-installed handlers
};


// ======================> address_table_write

// write access-specific version of an address table
class address_table_write : public address_table
{
public:
	// construction/destruction
	address_table_write(address_space &space, bool large);
	~address_table_write();

	// getters
	virtual handler_entry &handler(UINT32 index) const;
	handler_entry_write &handler_write(UINT32 index) const { assert(index < ARRAY_LENGTH(m_handlers)); return *m_handlers[index]; }

private:
	// internal handlers
	template<typename _UintType>
	void unmap_w(address_space &space, offs_t offset, _UintType data, _UintType mask)
	{
		if (m_space.log_unmap() && !m_space.debugger_access())
			logerror("%s: unmapped %s memory write to %s = %s & %s\n",
					cpuexec_describe_context(&m_space.m_machine), m_space.name(),
					core_i64_hex_format(m_space.byte_to_address(offset * sizeof(_UintType)), m_space.addrchars()),
					core_i64_hex_format(data, 2 * sizeof(_UintType)),
					core_i64_hex_format(mask, 2 * sizeof(_UintType)));
	}

	template<typename _UintType>
	void nop_w(address_space &space, offs_t offset, _UintType data, _UintType mask)
	{
	}

	template<typename _UintType>
	void watchpoint_w(address_space &space, offs_t offset, _UintType data, _UintType mask)
	{
		m_space.device().debug()->memory_write_hook(m_space, offset * sizeof(_UintType), data, mask);

		UINT8 *oldtable = m_live_lookup;
		m_live_lookup = m_table;
		if (sizeof(_UintType) == 1) m_space.write_byte(offset, data);
		if (sizeof(_UintType) == 2) m_space.write_word(offset << 1, data, mask);
		if (sizeof(_UintType) == 4) m_space.write_dword(offset << 2, data, mask);
		if (sizeof(_UintType) == 8) m_space.write_qword(offset << 3, data, mask);
		m_live_lookup = oldtable;
	}

	// internal state
	handler_entry_write *		m_handlers[256];		// array of user-installed handlers
};


// ======================> address_space_specific

// this is a derived class of address_space with specific width, endianness, and table size
template<typename _NativeType, endianness_t _Endian, bool _Large>
class address_space_specific : public address_space
{
	typedef address_space_specific<_NativeType, _Endian, _Large> this_type;

	// constants describing the native size
	static const UINT32 NATIVE_BYTES = sizeof(_NativeType);
	static const UINT32 NATIVE_MASK = NATIVE_BYTES - 1;
	static const UINT32 NATIVE_BITS = 8 * NATIVE_BYTES;

	// helpers to simplify core code
	UINT32 read_lookup(offs_t byteaddress) const { return _Large ? m_read.lookup_live_large(byteaddress) : m_read.lookup_live_small(byteaddress); }
	UINT32 write_lookup(offs_t byteaddress) const { return _Large ? m_write.lookup_live_large(byteaddress) : m_write.lookup_live_small(byteaddress); }

public:
	// construction/destruction
	address_space_specific(device_memory_interface &memory, int spacenum)
		: address_space(memory, spacenum, _Large),
		  m_read(*this, _Large),
		  m_write(*this, _Large)
	{
#if (TEST_HANDLER)
		// test code to verify the read/write handlers are touching the correct bits
		// and returning the correct results

		// install some dummy RAM for the first 16 bytes with well-known values
		UINT8 buffer[16];
		for (int index = 0; index < 16; index++)
			buffer[index ^ ((_Endian == ENDIANNESS_NATIVE) ? 0 : (data_width()/8 - 1))] = index * 0x11;
		install_ram(0x00, 0x0f, 0x0f, 0, ROW_READWRITE, buffer);
		printf("\n\naddress_space(%d, %s, %s)\n", NATIVE_BITS, (_Endian == ENDIANNESS_LITTLE) ? "little" : "big", _Large ? "large" : "small");

		// walk through the first 8 addresses
		for (int address = 0; address < 8; address++)
		{
			// determine expected values
			UINT64 expected64 = ((UINT64)((address + ((_Endian == ENDIANNESS_LITTLE) ? 7 : 0)) * 0x11) << 56) |
							    ((UINT64)((address + ((_Endian == ENDIANNESS_LITTLE) ? 6 : 1)) * 0x11) << 48) |
							    ((UINT64)((address + ((_Endian == ENDIANNESS_LITTLE) ? 5 : 2)) * 0x11) << 40) |
							    ((UINT64)((address + ((_Endian == ENDIANNESS_LITTLE) ? 4 : 3)) * 0x11) << 32) |
							    ((UINT64)((address + ((_Endian == ENDIANNESS_LITTLE) ? 3 : 4)) * 0x11) << 24) |
							    ((UINT64)((address + ((_Endian == ENDIANNESS_LITTLE) ? 2 : 5)) * 0x11) << 16) |
							    ((UINT64)((address + ((_Endian == ENDIANNESS_LITTLE) ? 1 : 6)) * 0x11) <<  8) |
							    ((UINT64)((address + ((_Endian == ENDIANNESS_LITTLE) ? 0 : 7)) * 0x11) <<  0);
			UINT32 expected32 = (_Endian == ENDIANNESS_LITTLE) ? expected64 : (expected64 >> 32);
			UINT16 expected16 = (_Endian == ENDIANNESS_LITTLE) ? expected32 : (expected32 >> 16);
			UINT8 expected8 = (_Endian == ENDIANNESS_LITTLE) ? expected16 : (expected16 >> 8);

			UINT64 result64;
			UINT32 result32;
			UINT16 result16;
			UINT8 result8;

			// validate byte accesses
			printf("\nAddress %d\n", address);
			printf("   read_byte = "); printf("%02X\n", result8 = read_byte(address)); assert(result8 == expected8);

			// validate word accesses (if aligned)
			if (address % 2 == 0) { printf("   read_word = "); printf("%04X\n", result16 = read_word(address)); assert(result16 == expected16); }
			if (address % 2 == 0) { printf("   read_word (0xff00) = "); printf("%04X\n", result16 = read_word(address, 0xff00)); assert((result16 & 0xff00) == (expected16 & 0xff00)); }
			if (address % 2 == 0) { printf("             (0x00ff) = "); printf("%04X\n", result16 = read_word(address, 0x00ff)); assert((result16 & 0x00ff) == (expected16 & 0x00ff)); }

			// validate unaligned word accesses
			printf("   read_word_unaligned = "); printf("%04X\n", result16 = read_word_unaligned(address)); assert(result16 == expected16);
			printf("   read_word_unaligned (0xff00) = "); printf("%04X\n", result16 = read_word_unaligned(address, 0xff00)); assert((result16 & 0xff00) == (expected16 & 0xff00));
			printf("                       (0x00ff) = "); printf("%04X\n", result16 = read_word_unaligned(address, 0x00ff)); assert((result16 & 0x00ff) == (expected16 & 0x00ff));

			// validate dword acceses (if aligned)
			if (address % 4 == 0) { printf("   read_dword = "); printf("%08X\n", result32 = read_dword(address)); assert(result32 == expected32); }
			if (address % 4 == 0) { printf("   read_dword (0xff000000) = "); printf("%08X\n", result32 = read_dword(address, 0xff000000)); assert((result32 & 0xff000000) == (expected32 & 0xff000000)); }
			if (address % 4 == 0) { printf("              (0x00ff0000) = "); printf("%08X\n", result32 = read_dword(address, 0x00ff0000)); assert((result32 & 0x00ff0000) == (expected32 & 0x00ff0000)); }
			if (address % 4 == 0) { printf("              (0x0000ff00) = "); printf("%08X\n", result32 = read_dword(address, 0x0000ff00)); assert((result32 & 0x0000ff00) == (expected32 & 0x0000ff00)); }
			if (address % 4 == 0) { printf("              (0x000000ff) = "); printf("%08X\n", result32 = read_dword(address, 0x000000ff)); assert((result32 & 0x000000ff) == (expected32 & 0x000000ff)); }
			if (address % 4 == 0) { printf("              (0xffff0000) = "); printf("%08X\n", result32 = read_dword(address, 0xffff0000)); assert((result32 & 0xffff0000) == (expected32 & 0xffff0000)); }
			if (address % 4 == 0) { printf("              (0x0000ffff) = "); printf("%08X\n", result32 = read_dword(address, 0x0000ffff)); assert((result32 & 0x0000ffff) == (expected32 & 0x0000ffff)); }
			if (address % 4 == 0) { printf("              (0xffffff00) = "); printf("%08X\n", result32 = read_dword(address, 0xffffff00)); assert((result32 & 0xffffff00) == (expected32 & 0xffffff00)); }
			if (address % 4 == 0) { printf("              (0x00ffffff) = "); printf("%08X\n", result32 = read_dword(address, 0x00ffffff)); assert((result32 & 0x00ffffff) == (expected32 & 0x00ffffff)); }

			// validate unaligned dword accesses
			printf("   read_dword_unaligned = "); printf("%08X\n", result32 = read_dword_unaligned(address)); assert(result32 == expected32);
			printf("   read_dword_unaligned (0xff000000) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0xff000000)); assert((result32 & 0xff000000) == (expected32 & 0xff000000));
			printf("                        (0x00ff0000) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x00ff0000)); assert((result32 & 0x00ff0000) == (expected32 & 0x00ff0000));
			printf("                        (0x0000ff00) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x0000ff00)); assert((result32 & 0x0000ff00) == (expected32 & 0x0000ff00));
			printf("                        (0x000000ff) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x000000ff)); assert((result32 & 0x000000ff) == (expected32 & 0x000000ff));
			printf("                        (0xffff0000) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0xffff0000)); assert((result32 & 0xffff0000) == (expected32 & 0xffff0000));
			printf("                        (0x0000ffff) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x0000ffff)); assert((result32 & 0x0000ffff) == (expected32 & 0x0000ffff));
			printf("                        (0xffffff00) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0xffffff00)); assert((result32 & 0xffffff00) == (expected32 & 0xffffff00));
			printf("                        (0x00ffffff) = "); printf("%08X\n", result32 = read_dword_unaligned(address, 0x00ffffff)); assert((result32 & 0x00ffffff) == (expected32 & 0x00ffffff));

			// validate qword acceses (if aligned)
			if (address % 8 == 0) { printf("   read_qword = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address), 16)); assert(result64 == expected64); }
			if (address % 8 == 0) { printf("   read_qword (0xff00000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0xff00000000000000)), 16)); assert((result64 & U64(0xff00000000000000)) == (expected64 & U64(0xff00000000000000))); }
			if (address % 8 == 0) { printf("              (0x00ff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00ff000000000000)), 16)); assert((result64 & U64(0x00ff000000000000)) == (expected64 & U64(0x00ff000000000000))); }
			if (address % 8 == 0) { printf("              (0x0000ff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x0000ff0000000000)), 16)); assert((result64 & U64(0x0000ff0000000000)) == (expected64 & U64(0x0000ff0000000000))); }
			if (address % 8 == 0) { printf("              (0x000000ff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x000000ff00000000)), 16)); assert((result64 & U64(0x000000ff00000000)) == (expected64 & U64(0x000000ff00000000))); }
			if (address % 8 == 0) { printf("              (0x00000000ff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00000000ff000000)), 16)); assert((result64 & U64(0x00000000ff000000)) == (expected64 & U64(0x00000000ff000000))); }
			if (address % 8 == 0) { printf("              (0x0000000000ff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x0000000000ff0000)), 16)); assert((result64 & U64(0x0000000000ff0000)) == (expected64 & U64(0x0000000000ff0000))); }
			if (address % 8 == 0) { printf("              (0x000000000000ff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x000000000000ff00)), 16)); assert((result64 & U64(0x000000000000ff00)) == (expected64 & U64(0x000000000000ff00))); }
			if (address % 8 == 0) { printf("              (0x00000000000000ff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00000000000000ff)), 16)); assert((result64 & U64(0x00000000000000ff)) == (expected64 & U64(0x00000000000000ff))); }
			if (address % 8 == 0) { printf("              (0xffff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0xffff000000000000)), 16)); assert((result64 & U64(0xffff000000000000)) == (expected64 & U64(0xffff000000000000))); }
			if (address % 8 == 0) { printf("              (0x0000ffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x0000ffff00000000)), 16)); assert((result64 & U64(0x0000ffff00000000)) == (expected64 & U64(0x0000ffff00000000))); }
			if (address % 8 == 0) { printf("              (0x00000000ffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00000000ffff0000)), 16)); assert((result64 & U64(0x00000000ffff0000)) == (expected64 & U64(0x00000000ffff0000))); }
			if (address % 8 == 0) { printf("              (0x000000000000ffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x000000000000ffff)), 16)); assert((result64 & U64(0x000000000000ffff)) == (expected64 & U64(0x000000000000ffff))); }
			if (address % 8 == 0) { printf("              (0xffffff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0xffffff0000000000)), 16)); assert((result64 & U64(0xffffff0000000000)) == (expected64 & U64(0xffffff0000000000))); }
			if (address % 8 == 0) { printf("              (0x0000ffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x0000ffffff000000)), 16)); assert((result64 & U64(0x0000ffffff000000)) == (expected64 & U64(0x0000ffffff000000))); }
			if (address % 8 == 0) { printf("              (0x000000ffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x000000ffffff0000)), 16)); assert((result64 & U64(0x000000ffffff0000)) == (expected64 & U64(0x000000ffffff0000))); }
			if (address % 8 == 0) { printf("              (0x0000000000ffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x0000000000ffffff)), 16)); assert((result64 & U64(0x0000000000ffffff)) == (expected64 & U64(0x0000000000ffffff))); }
			if (address % 8 == 0) { printf("              (0xffffffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0xffffffff00000000)), 16)); assert((result64 & U64(0xffffffff00000000)) == (expected64 & U64(0xffffffff00000000))); }
			if (address % 8 == 0) { printf("              (0x00ffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00ffffffff000000)), 16)); assert((result64 & U64(0x00ffffffff000000)) == (expected64 & U64(0x00ffffffff000000))); }
			if (address % 8 == 0) { printf("              (0x0000ffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x0000ffffffff0000)), 16)); assert((result64 & U64(0x0000ffffffff0000)) == (expected64 & U64(0x0000ffffffff0000))); }
			if (address % 8 == 0) { printf("              (0x000000ffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x000000ffffffff00)), 16)); assert((result64 & U64(0x000000ffffffff00)) == (expected64 & U64(0x000000ffffffff00))); }
			if (address % 8 == 0) { printf("              (0x00000000ffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00000000ffffffff)), 16)); assert((result64 & U64(0x00000000ffffffff)) == (expected64 & U64(0x00000000ffffffff))); }
			if (address % 8 == 0) { printf("              (0xffffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0xffffffffff000000)), 16)); assert((result64 & U64(0xffffffffff000000)) == (expected64 & U64(0xffffffffff000000))); }
			if (address % 8 == 0) { printf("              (0x00ffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00ffffffffff0000)), 16)); assert((result64 & U64(0x00ffffffffff0000)) == (expected64 & U64(0x00ffffffffff0000))); }
			if (address % 8 == 0) { printf("              (0x0000ffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x0000ffffffffff00)), 16)); assert((result64 & U64(0x0000ffffffffff00)) == (expected64 & U64(0x0000ffffffffff00))); }
			if (address % 8 == 0) { printf("              (0x000000ffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x000000ffffffffff)), 16)); assert((result64 & U64(0x000000ffffffffff)) == (expected64 & U64(0x000000ffffffffff))); }
			if (address % 8 == 0) { printf("              (0xffffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0xffffffffffff0000)), 16)); assert((result64 & U64(0xffffffffffff0000)) == (expected64 & U64(0xffffffffffff0000))); }
			if (address % 8 == 0) { printf("              (0x00ffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00ffffffffffff00)), 16)); assert((result64 & U64(0x00ffffffffffff00)) == (expected64 & U64(0x00ffffffffffff00))); }
			if (address % 8 == 0) { printf("              (0x0000ffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x0000ffffffffffff)), 16)); assert((result64 & U64(0x0000ffffffffffff)) == (expected64 & U64(0x0000ffffffffffff))); }
			if (address % 8 == 0) { printf("              (0xffffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0xffffffffffffff00)), 16)); assert((result64 & U64(0xffffffffffffff00)) == (expected64 & U64(0xffffffffffffff00))); }
			if (address % 8 == 0) { printf("              (0x00ffffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, U64(0x00ffffffffffffff)), 16)); assert((result64 & U64(0x00ffffffffffffff)) == (expected64 & U64(0x00ffffffffffffff))); }

			// validate unaligned qword accesses
			printf("   read_qword_unaligned = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address), 16)); assert(result64 == expected64);
			printf("   read_qword_unaligned (0xff00000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0xff00000000000000)), 16)); assert((result64 & U64(0xff00000000000000)) == (expected64 & U64(0xff00000000000000)));
			printf("                        (0x00ff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00ff000000000000)), 16)); assert((result64 & U64(0x00ff000000000000)) == (expected64 & U64(0x00ff000000000000)));
			printf("                        (0x0000ff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x0000ff0000000000)), 16)); assert((result64 & U64(0x0000ff0000000000)) == (expected64 & U64(0x0000ff0000000000)));
			printf("                        (0x000000ff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x000000ff00000000)), 16)); assert((result64 & U64(0x000000ff00000000)) == (expected64 & U64(0x000000ff00000000)));
			printf("                        (0x00000000ff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00000000ff000000)), 16)); assert((result64 & U64(0x00000000ff000000)) == (expected64 & U64(0x00000000ff000000)));
			printf("                        (0x0000000000ff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x0000000000ff0000)), 16)); assert((result64 & U64(0x0000000000ff0000)) == (expected64 & U64(0x0000000000ff0000)));
			printf("                        (0x000000000000ff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x000000000000ff00)), 16)); assert((result64 & U64(0x000000000000ff00)) == (expected64 & U64(0x000000000000ff00)));
			printf("                        (0x00000000000000ff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00000000000000ff)), 16)); assert((result64 & U64(0x00000000000000ff)) == (expected64 & U64(0x00000000000000ff)));
			printf("                        (0xffff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0xffff000000000000)), 16)); assert((result64 & U64(0xffff000000000000)) == (expected64 & U64(0xffff000000000000)));
			printf("                        (0x0000ffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x0000ffff00000000)), 16)); assert((result64 & U64(0x0000ffff00000000)) == (expected64 & U64(0x0000ffff00000000)));
			printf("                        (0x00000000ffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00000000ffff0000)), 16)); assert((result64 & U64(0x00000000ffff0000)) == (expected64 & U64(0x00000000ffff0000)));
			printf("                        (0x000000000000ffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x000000000000ffff)), 16)); assert((result64 & U64(0x000000000000ffff)) == (expected64 & U64(0x000000000000ffff)));
			printf("                        (0xffffff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0xffffff0000000000)), 16)); assert((result64 & U64(0xffffff0000000000)) == (expected64 & U64(0xffffff0000000000)));
			printf("                        (0x0000ffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x0000ffffff000000)), 16)); assert((result64 & U64(0x0000ffffff000000)) == (expected64 & U64(0x0000ffffff000000)));
			printf("                        (0x000000ffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x000000ffffff0000)), 16)); assert((result64 & U64(0x000000ffffff0000)) == (expected64 & U64(0x000000ffffff0000)));
			printf("                        (0x0000000000ffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x0000000000ffffff)), 16)); assert((result64 & U64(0x0000000000ffffff)) == (expected64 & U64(0x0000000000ffffff)));
			printf("                        (0xffffffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0xffffffff00000000)), 16)); assert((result64 & U64(0xffffffff00000000)) == (expected64 & U64(0xffffffff00000000)));
			printf("                        (0x00ffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00ffffffff000000)), 16)); assert((result64 & U64(0x00ffffffff000000)) == (expected64 & U64(0x00ffffffff000000)));
			printf("                        (0x0000ffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x0000ffffffff0000)), 16)); assert((result64 & U64(0x0000ffffffff0000)) == (expected64 & U64(0x0000ffffffff0000)));
			printf("                        (0x000000ffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x000000ffffffff00)), 16)); assert((result64 & U64(0x000000ffffffff00)) == (expected64 & U64(0x000000ffffffff00)));
			printf("                        (0x00000000ffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00000000ffffffff)), 16)); assert((result64 & U64(0x00000000ffffffff)) == (expected64 & U64(0x00000000ffffffff)));
			printf("                        (0xffffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0xffffffffff000000)), 16)); assert((result64 & U64(0xffffffffff000000)) == (expected64 & U64(0xffffffffff000000)));
			printf("                        (0x00ffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00ffffffffff0000)), 16)); assert((result64 & U64(0x00ffffffffff0000)) == (expected64 & U64(0x00ffffffffff0000)));
			printf("                        (0x0000ffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x0000ffffffffff00)), 16)); assert((result64 & U64(0x0000ffffffffff00)) == (expected64 & U64(0x0000ffffffffff00)));
			printf("                        (0x000000ffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x000000ffffffffff)), 16)); assert((result64 & U64(0x000000ffffffffff)) == (expected64 & U64(0x000000ffffffffff)));
			printf("                        (0xffffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0xffffffffffff0000)), 16)); assert((result64 & U64(0xffffffffffff0000)) == (expected64 & U64(0xffffffffffff0000)));
			printf("                        (0x00ffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00ffffffffffff00)), 16)); assert((result64 & U64(0x00ffffffffffff00)) == (expected64 & U64(0x00ffffffffffff00)));
			printf("                        (0x0000ffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x0000ffffffffffff)), 16)); assert((result64 & U64(0x0000ffffffffffff)) == (expected64 & U64(0x0000ffffffffffff)));
			printf("                        (0xffffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0xffffffffffffff00)), 16)); assert((result64 & U64(0xffffffffffffff00)) == (expected64 & U64(0xffffffffffffff00)));
			printf("                        (0x00ffffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, U64(0x00ffffffffffffff)), 16)); assert((result64 & U64(0x00ffffffffffffff)) == (expected64 & U64(0x00ffffffffffffff)));
		}
#endif
	}

	// accessors
	virtual address_table_read &read() { return m_read; }
	virtual address_table_write &write() { return m_write; }

	// watchpoint control
	virtual void enable_read_watchpoints(bool enable = true) { m_read.enable_watchpoints(enable); }
	virtual void enable_write_watchpoints(bool enable = true) { m_write.enable_watchpoints(enable); }

	// generate accessor table
	virtual void accessors(data_accessors &accessors) const
	{
		accessors.read_byte = reinterpret_cast<UINT8 (*)(address_space *, offs_t)>(&read_byte_static);
		accessors.read_word = reinterpret_cast<UINT16 (*)(address_space *, offs_t)>(&read_word_static);
		accessors.read_word_masked = reinterpret_cast<UINT16 (*)(address_space *, offs_t, UINT16)>(&read_word_masked_static);
		accessors.read_dword = reinterpret_cast<UINT32 (*)(address_space *, offs_t)>(&read_dword_static);
		accessors.read_dword_masked = reinterpret_cast<UINT32 (*)(address_space *, offs_t, UINT32)>(&read_dword_masked_static);
		accessors.read_qword = reinterpret_cast<UINT64 (*)(address_space *, offs_t)>(&read_qword_static);
		accessors.read_qword_masked = reinterpret_cast<UINT64 (*)(address_space *, offs_t, UINT64)>(&read_qword_masked_static);
		accessors.write_byte = reinterpret_cast<void (*)(address_space *, offs_t, UINT8)>(&write_byte_static);
		accessors.write_word = reinterpret_cast<void (*)(address_space *, offs_t, UINT16)>(&write_word_static);
		accessors.write_word_masked = reinterpret_cast<void (*)(address_space *, offs_t, UINT16, UINT16)>(&write_word_masked_static);
		accessors.write_dword = reinterpret_cast<void (*)(address_space *, offs_t, UINT32)>(&write_dword_static);
		accessors.write_dword_masked = reinterpret_cast<void (*)(address_space *, offs_t, UINT32, UINT32)>(&write_dword_masked_static);
		accessors.write_qword = reinterpret_cast<void (*)(address_space *, offs_t, UINT64)>(&write_qword_static);
		accessors.write_qword_masked = reinterpret_cast<void (*)(address_space *, offs_t, UINT64, UINT64)>(&write_qword_masked_static);
	}

	// return a pointer to the read bank, or NULL if none
	virtual void *get_read_ptr(offs_t byteaddress)
	{
		// perform the lookup
		byteaddress &= m_bytemask;
		UINT32 entry = read_lookup(byteaddress);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// 8-bit case: RAM/ROM
		if (entry >= STATIC_RAM)
			return NULL;
		return handler.ramptr(handler.byteoffset(byteaddress));
	}

	// return a pointer to the write bank, or NULL if none
	virtual void *get_write_ptr(offs_t byteaddress)
	{
		// perform the lookup
		byteaddress &= m_bytemask;
		UINT32 entry = read_lookup(byteaddress);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// 8-bit case: RAM/ROM
		if (entry >= STATIC_RAM)
			return NULL;
		return handler.ramptr(handler.byteoffset(byteaddress));
	}

	// native read
	_NativeType read_native(offs_t offset, _NativeType mask)
	{
		g_profiler.start(PROFILER_MEMREAD);

		if (TEST_HANDLER) printf("[r%X,%s]", offset, core_i64_hex_format(mask, sizeof(_NativeType) * 2));

		// look up the handler
		offs_t byteaddress = offset & m_bytemask;
		UINT32 entry = read_lookup(byteaddress);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// either read directly from RAM, or call the delegate
		offset = handler.byteoffset(byteaddress);
		_NativeType result;
		if (entry < STATIC_RAM) result = *reinterpret_cast<_NativeType *>(handler.ramptr(offset));
		else if (sizeof(_NativeType) == 1) result = handler.read8(*this, offset, mask);
		else if (sizeof(_NativeType) == 2) result = handler.read16(*this, offset >> 1, mask);
		else if (sizeof(_NativeType) == 4) result = handler.read32(*this, offset >> 2, mask);
		else if (sizeof(_NativeType) == 8) result = handler.read64(*this, offset >> 3, mask);

		g_profiler.stop();
		return result;
	}

	// mask-less native read
	_NativeType read_native(offs_t offset)
	{
		g_profiler.start(PROFILER_MEMREAD);

		if (TEST_HANDLER) printf("[r%X]", offset);

		// look up the handler
		offs_t byteaddress = offset & m_bytemask;
		UINT32 entry = read_lookup(byteaddress);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// either read directly from RAM, or call the delegate
		offset = handler.byteoffset(byteaddress);
		_NativeType result;
		if (entry < STATIC_RAM) result = *reinterpret_cast<_NativeType *>(handler.ramptr(offset));
		else if (sizeof(_NativeType) == 1) result = handler.read8(*this, offset, 0xff);
		else if (sizeof(_NativeType) == 2) result = handler.read16(*this, offset >> 1, 0xffff);
		else if (sizeof(_NativeType) == 4) result = handler.read32(*this, offset >> 2, 0xffffffff);
		else if (sizeof(_NativeType) == 8) result = handler.read64(*this, offset >> 3, U64(0xffffffffffffffff));

		g_profiler.stop();
		return result;
	}

	// native write
	void write_native(offs_t offset, _NativeType data, _NativeType mask)
	{
		g_profiler.start(PROFILER_MEMWRITE);

		// look up the handler
		offs_t byteaddress = offset & m_bytemask;
		UINT32 entry = write_lookup(byteaddress);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// either write directly to RAM, or call the delegate
		offset = handler.byteoffset(byteaddress);
		if (entry < STATIC_RAM)
		{
			_NativeType *dest = reinterpret_cast<_NativeType *>(handler.ramptr(offset));
			*dest = (*dest & ~mask) | (data & mask);
		}
		else if (sizeof(_NativeType) == 1) handler.write8(*this, offset, data, mask);
		else if (sizeof(_NativeType) == 2) handler.write16(*this, offset >> 1, data, mask);
		else if (sizeof(_NativeType) == 4) handler.write32(*this, offset >> 2, data, mask);
		else if (sizeof(_NativeType) == 8) handler.write64(*this, offset >> 3, data, mask);

		g_profiler.stop();
	}

	// mask-less native write
	void write_native(offs_t offset, _NativeType data)
	{
		g_profiler.start(PROFILER_MEMWRITE);

		// look up the handler
		offs_t byteaddress = offset & m_bytemask;
		UINT32 entry = write_lookup(byteaddress);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// either write directly to RAM, or call the delegate
		offset = handler.byteoffset(byteaddress);
		if (entry < STATIC_RAM) *reinterpret_cast<_NativeType *>(handler.ramptr(offset)) = data;
		else if (sizeof(_NativeType) == 1) handler.write8(*this, offset, data, 0xff);
		else if (sizeof(_NativeType) == 2) handler.write16(*this, offset >> 1, data, 0xffff);
		else if (sizeof(_NativeType) == 4) handler.write32(*this, offset >> 2, data, 0xffffffff);
		else if (sizeof(_NativeType) == 8) handler.write64(*this, offset >> 3, data, U64(0xffffffffffffffff));

		g_profiler.stop();
	}

	// generic direct read
	template<typename _TargetType, bool _Aligned>
	_TargetType read_direct(offs_t address, _TargetType mask)
	{
		const UINT32 TARGET_BYTES = sizeof(_TargetType);
		const UINT32 TARGET_BITS = 8 * TARGET_BYTES;

		// equal to native size and aligned; simple pass-through to the native reader
		if (NATIVE_BYTES == TARGET_BYTES && (_Aligned || (address & NATIVE_MASK) == 0))
			return read_native(address & ~NATIVE_MASK, mask);

		// if native size is larger, see if we can do a single masked read (guaranteed if we're aligned)
		if (NATIVE_BYTES > TARGET_BYTES)
		{
			UINT32 offsbits = 8 * (address & (NATIVE_BYTES - (_Aligned ? TARGET_BYTES : 1)));
			if (_Aligned || (offsbits + TARGET_BITS <= NATIVE_BITS))
			{
				if (_Endian != ENDIANNESS_LITTLE) offsbits = NATIVE_BITS - TARGET_BITS - offsbits;
				return read_native(address & ~NATIVE_MASK, (_NativeType)mask << offsbits) >> offsbits;
			}
		}

		// determine our alignment against the native boundaries, and mask the address
		UINT32 offsbits = 8 * (address & (NATIVE_BYTES - 1));
		address &= ~NATIVE_MASK;

		// if we're here, and native size is larger or equal to the target, we need exactly 2 reads
		if (NATIVE_BYTES >= TARGET_BYTES)
		{
			// little-endian case
			if (_Endian == ENDIANNESS_LITTLE)
			{
				// read lower bits from lower address
				_TargetType result = 0;
				_NativeType curmask = (_NativeType)mask << offsbits;
				if (curmask != 0) result = read_native(address, curmask) >> offsbits;

				// read upper bits from upper address
				offsbits = NATIVE_BITS - offsbits;
				curmask = mask >> offsbits;
				if (curmask != 0) result |= read_native(address + NATIVE_BYTES, curmask) << offsbits;
				return result;
			}

			// big-endian case
			else
			{
				// left-justify the mask to the target type
				const UINT32 LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT = ((NATIVE_BITS >= TARGET_BITS) ? (NATIVE_BITS - TARGET_BITS) : 0);
				_NativeType result = 0;
				_NativeType ljmask = (_NativeType)mask << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
				_NativeType curmask = ljmask >> offsbits;

				// read upper bits from lower address
				if (curmask != 0) result = read_native(address, curmask) << offsbits;
				offsbits = NATIVE_BITS - offsbits;

				// read lower bits from upper address
				curmask = ljmask << offsbits;
				if (curmask != 0) result |= read_native(address + NATIVE_BYTES, curmask) >> offsbits;

				// return the un-justified result
				return result >> LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
			}
		}

		// if we're here, then we have 2 or more reads needed to get our final result
		else
		{
			// compute the maximum number of loops; we do it this way so that there are
			// a fixed number of loops for the compiler to unroll if it desires
			const UINT32 MAX_SPLITS_MINUS_ONE = TARGET_BYTES / NATIVE_BYTES - 1;
			_TargetType result = 0;

			// little-endian case
			if (_Endian == ENDIANNESS_LITTLE)
			{
				// read lowest bits from first address
				_NativeType curmask = mask << offsbits;
				if (curmask != 0) result = read_native(address, curmask) >> offsbits;

				// read middle bits from subsequent addresses
				offsbits = NATIVE_BITS - offsbits;
				for (UINT32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
				{
					address += NATIVE_BYTES;
					curmask = mask >> offsbits;
					if (curmask != 0) result |= (_TargetType)read_native(address, curmask) << offsbits;
					offsbits += NATIVE_BITS;
				}

				// if we're not aligned and we still have bits left, read uppermost bits from last address
				if (!_Aligned && offsbits < TARGET_BITS)
				{
					curmask = mask >> offsbits;
					if (curmask != 0) result |= (_TargetType)read_native(address + NATIVE_BYTES, curmask) << offsbits;
				}
			}

			// big-endian case
			else
			{
				// read highest bits from first address
				offsbits = TARGET_BITS - (NATIVE_BITS - offsbits);
				_NativeType curmask = mask >> offsbits;
				if (curmask != 0) result = (_TargetType)read_native(address, curmask) << offsbits;

				// read middle bits from subsequent addresses
				for (UINT32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
				{
					offsbits -= NATIVE_BITS;
					address += NATIVE_BYTES;
					curmask = mask >> offsbits;
					if (curmask != 0) result |= (_TargetType)read_native(address, curmask) << offsbits;
				}

				// if we're not aligned and we still have bits left, read lowermost bits from the last address
				if (!_Aligned && offsbits != 0)
				{
					offsbits = NATIVE_BITS - offsbits;
					curmask = mask << offsbits;
					if (curmask != 0) result |= read_native(address + NATIVE_BYTES, curmask) >> offsbits;
				}
			}
			return result;
		}
	}

	// generic direct write
	template<typename _TargetType, bool _Aligned>
	void write_direct(offs_t address, _TargetType data, _TargetType mask)
	{
		const UINT32 TARGET_BYTES = sizeof(_TargetType);
		const UINT32 TARGET_BITS = 8 * TARGET_BYTES;

		// equal to native size and aligned; simple pass-through to the native writer
		if (NATIVE_BYTES == TARGET_BYTES && (_Aligned || (address & NATIVE_MASK) == 0))
			return write_native(address & ~NATIVE_MASK, data, mask);

		// if native size is larger, see if we can do a single masked write (guaranteed if we're aligned)
		if (NATIVE_BYTES > TARGET_BYTES)
		{
			UINT32 offsbits = 8 * (address & (NATIVE_BYTES - (_Aligned ? TARGET_BYTES : 1)));
			if (_Aligned || (offsbits + TARGET_BITS <= NATIVE_BITS))
			{
				if (_Endian != ENDIANNESS_LITTLE) offsbits = NATIVE_BITS - TARGET_BITS - offsbits;
				return write_native(address & ~NATIVE_MASK, (_NativeType)data << offsbits, (_NativeType)mask << offsbits);
			}
		}

		// determine our alignment against the native boundaries, and mask the address
		UINT32 offsbits = 8 * (address & (NATIVE_BYTES - 1));
		address &= ~NATIVE_MASK;

		// if we're here, and native size is larger or equal to the target, we need exactly 2 writes
		if (NATIVE_BYTES >= TARGET_BYTES)
		{
			// little-endian case
			if (_Endian == ENDIANNESS_LITTLE)
			{
				// write lower bits to lower address
				_NativeType curmask = (_NativeType)mask << offsbits;
				if (curmask != 0) write_native(address, (_NativeType)data << offsbits, curmask);

				// write upper bits to upper address
				offsbits = NATIVE_BITS - offsbits;
				curmask = mask >> offsbits;
				if (curmask != 0) write_native(address + NATIVE_BYTES, data >> offsbits, curmask);
			}

			// big-endian case
			else
			{
				// left-justify the mask and data to the target type
				const UINT32 LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT = ((NATIVE_BITS >= TARGET_BITS) ? (NATIVE_BITS - TARGET_BITS) : 0);
				_NativeType ljdata = (_NativeType)data << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;
				_NativeType ljmask = (_NativeType)mask << LEFT_JUSTIFY_TARGET_TO_NATIVE_SHIFT;

				// write upper bits to lower address
				_NativeType curmask = ljmask >> offsbits;
				if (curmask != 0) write_native(address, ljdata >> offsbits, curmask);

				// write lower bits to upper address
				offsbits = NATIVE_BITS - offsbits;
				curmask = ljmask << offsbits;
				if (curmask != 0) write_native(address + NATIVE_BYTES, ljdata << offsbits, curmask);
			}
		}

		// if we're here, then we have 2 or more writes needed to get our final result
		else
		{
			// compute the maximum number of loops; we do it this way so that there are
			// a fixed number of loops for the compiler to unroll if it desires
			const UINT32 MAX_SPLITS_MINUS_ONE = TARGET_BYTES / NATIVE_BYTES - 1;

			// little-endian case
			if (_Endian == ENDIANNESS_LITTLE)
			{
				// write lowest bits to first address
				_NativeType curmask = mask << offsbits;
				if (curmask != 0) write_native(address, data << offsbits, curmask);

				// write middle bits to subsequent addresses
				offsbits = NATIVE_BITS - offsbits;
				for (UINT32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
				{
					address += NATIVE_BYTES;
					curmask = mask >> offsbits;
					if (curmask != 0) write_native(address, data >> offsbits, curmask);
					offsbits += NATIVE_BITS;
				}

				// if we're not aligned and we still have bits left, write uppermost bits to last address
				if (!_Aligned && offsbits < TARGET_BITS)
				{
					curmask = mask >> offsbits;
					if (curmask != 0) write_native(address + NATIVE_BYTES, data >> offsbits, curmask);
				}
			}

			// big-endian case
			else
			{
				// write highest bits to first address
				offsbits = TARGET_BITS - (NATIVE_BITS - offsbits);
				_NativeType curmask = mask >> offsbits;
				if (curmask != 0) write_native(address, data >> offsbits, curmask);

				// write middle bits to subsequent addresses
				for (UINT32 index = 0; index < MAX_SPLITS_MINUS_ONE; index++)
				{
					offsbits -= NATIVE_BITS;
					address += NATIVE_BYTES;
					curmask = mask >> offsbits;
					if (curmask != 0) write_native(address, data >> offsbits, curmask);
				}

				// if we're not aligned and we still have bits left, write lowermost bits to the last address
				if (!_Aligned && offsbits != 0)
				{
					offsbits = NATIVE_BITS - offsbits;
					curmask = mask << offsbits;
					if (curmask != 0) write_native(address + NATIVE_BYTES, data << offsbits, curmask);
				}
			}
		}
	}

	// virtual access to these functions
	UINT8 read_byte(offs_t address) { return (NATIVE_BITS == 8) ? read_native(address & ~NATIVE_MASK) : read_direct<UINT8, true>(address, 0xff); }
	UINT16 read_word(offs_t address) { return (NATIVE_BITS == 16) ? read_native(address & ~NATIVE_MASK) : read_direct<UINT16, true>(address, 0xffff); }
	UINT16 read_word(offs_t address, UINT16 mask) { return read_direct<UINT16, true>(address, mask); }
	UINT16 read_word_unaligned(offs_t address) { return read_direct<UINT16, false>(address, 0xffff); }
	UINT16 read_word_unaligned(offs_t address, UINT16 mask) { return read_direct<UINT16, false>(address, mask); }
	UINT32 read_dword(offs_t address) { return (NATIVE_BITS == 32) ? read_native(address & ~NATIVE_MASK) : read_direct<UINT32, true>(address, 0xffffffff); }
	UINT32 read_dword(offs_t address, UINT32 mask) { return read_direct<UINT32, true>(address, mask); }
	UINT32 read_dword_unaligned(offs_t address) { return read_direct<UINT32, false>(address, 0xffffffff); }
	UINT32 read_dword_unaligned(offs_t address, UINT32 mask) { return read_direct<UINT32, false>(address, mask); }
	UINT64 read_qword(offs_t address) { return (NATIVE_BITS == 64) ? read_native(address & ~NATIVE_MASK) : read_direct<UINT64, true>(address, U64(0xffffffffffffffff)); }
	UINT64 read_qword(offs_t address, UINT64 mask) { return read_direct<UINT64, true>(address, mask); }
	UINT64 read_qword_unaligned(offs_t address) { return read_direct<UINT64, false>(address, U64(0xffffffffffffffff)); }
	UINT64 read_qword_unaligned(offs_t address, UINT64 mask) { return read_direct<UINT64, false>(address, mask); }

	void write_byte(offs_t address, UINT8 data) { if (NATIVE_BITS == 8) write_native(address & ~NATIVE_MASK, data); else write_direct<UINT8, true>(address, data, 0xff); }
	void write_word(offs_t address, UINT16 data) { if (NATIVE_BITS == 16) write_native(address & ~NATIVE_MASK, data); else write_direct<UINT16, true>(address, data, 0xffff); }
	void write_word(offs_t address, UINT16 data, UINT16 mask) { write_direct<UINT16, true>(address, data, mask); }
	void write_word_unaligned(offs_t address, UINT16 data) { write_direct<UINT16, false>(address, data, 0xffff); }
	void write_word_unaligned(offs_t address, UINT16 data, UINT16 mask) { write_direct<UINT16, false>(address, data, mask); }
	void write_dword(offs_t address, UINT32 data) { if (NATIVE_BITS == 32) write_native(address & ~NATIVE_MASK, data); else write_direct<UINT32, true>(address, data, 0xffffffff); }
	void write_dword(offs_t address, UINT32 data, UINT32 mask) { write_direct<UINT32, true>(address, data, mask); }
	void write_dword_unaligned(offs_t address, UINT32 data) { write_direct<UINT32, false>(address, data, 0xffffffff); }
	void write_dword_unaligned(offs_t address, UINT32 data, UINT32 mask) { write_direct<UINT32, false>(address, data, mask); }
	void write_qword(offs_t address, UINT64 data) { if (NATIVE_BITS == 64) write_native(address & ~NATIVE_MASK, data); else write_direct<UINT64, true>(address, data, U64(0xffffffffffffffff)); }
	void write_qword(offs_t address, UINT64 data, UINT64 mask) { write_direct<UINT64, true>(address, data, mask); }
	void write_qword_unaligned(offs_t address, UINT64 data) { write_direct<UINT64, false>(address, data, U64(0xffffffffffffffff)); }
	void write_qword_unaligned(offs_t address, UINT64 data, UINT64 mask) { write_direct<UINT64, false>(address, data, mask); }

	// static access to these functions
	static UINT8 read_byte_static(this_type *space, offs_t address) { return (NATIVE_BITS == 8) ? space->read_native(address & ~NATIVE_MASK) : space->read_direct<UINT8, true>(address, 0xff); }
	static UINT16 read_word_static(this_type *space, offs_t address) { return (NATIVE_BITS == 16) ? space->read_native(address & ~NATIVE_MASK) : space->read_direct<UINT16, true>(address, 0xffff); }
	static UINT16 read_word_masked_static(this_type *space, offs_t address, UINT16 mask) { return space->read_direct<UINT16, true>(address, mask); }
	static UINT32 read_dword_static(this_type *space, offs_t address) { return (NATIVE_BITS == 32) ? space->read_native(address & ~NATIVE_MASK) : space->read_direct<UINT32, true>(address, 0xffffffff); }
	static UINT32 read_dword_masked_static(this_type *space, offs_t address, UINT32 mask) { return space->read_direct<UINT32, true>(address, mask); }
	static UINT64 read_qword_static(this_type *space, offs_t address) { return (NATIVE_BITS == 64) ? space->read_native(address & ~NATIVE_MASK) : space->read_direct<UINT64, true>(address, U64(0xffffffffffffffff)); }
	static UINT64 read_qword_masked_static(this_type *space, offs_t address, UINT64 mask) { return space->read_direct<UINT64, true>(address, mask); }
	static void write_byte_static(this_type *space, offs_t address, UINT8 data) { if (NATIVE_BITS == 8) space->write_native(address & ~NATIVE_MASK, data); else space->write_direct<UINT8, true>(address, data, 0xff); }
	static void write_word_static(this_type *space, offs_t address, UINT16 data) { if (NATIVE_BITS == 16) space->write_native(address & ~NATIVE_MASK, data); else space->write_direct<UINT16, true>(address, data, 0xffff); }
	static void write_word_masked_static(this_type *space, offs_t address, UINT16 data, UINT16 mask) { space->write_direct<UINT16, true>(address, data, mask); }
	static void write_dword_static(this_type *space, offs_t address, UINT32 data) { if (NATIVE_BITS == 32) space->write_native(address & ~NATIVE_MASK, data); else space->write_direct<UINT32, true>(address, data, 0xffffffff); }
	static void write_dword_masked_static(this_type *space, offs_t address, UINT32 data, UINT32 mask) { space->write_direct<UINT32, true>(address, data, mask); }
	static void write_qword_static(this_type *space, offs_t address, UINT64 data) { if (NATIVE_BITS == 64) space->write_native(address & ~NATIVE_MASK, data); else space->write_direct<UINT64, true>(address, data, U64(0xffffffffffffffff)); }
	static void write_qword_masked_static(this_type *space, offs_t address, UINT64 data, UINT64 mask) { space->write_direct<UINT64, true>(address, data, mask); }

	address_table_read		m_read;				// memory read lookup table
	address_table_write		m_write;			// memory write lookup table
};

typedef address_space_specific<UINT8,  ENDIANNESS_LITTLE, false> address_space_8le_small;
typedef address_space_specific<UINT8,  ENDIANNESS_BIG,    false> address_space_8be_small;
typedef address_space_specific<UINT16, ENDIANNESS_LITTLE, false> address_space_16le_small;
typedef address_space_specific<UINT16, ENDIANNESS_BIG,    false> address_space_16be_small;
typedef address_space_specific<UINT32, ENDIANNESS_LITTLE, false> address_space_32le_small;
typedef address_space_specific<UINT32, ENDIANNESS_BIG,    false> address_space_32be_small;
typedef address_space_specific<UINT64, ENDIANNESS_LITTLE, false> address_space_64le_small;
typedef address_space_specific<UINT64, ENDIANNESS_BIG,    false> address_space_64be_small;

typedef address_space_specific<UINT8,  ENDIANNESS_LITTLE, true> address_space_8le_large;
typedef address_space_specific<UINT8,  ENDIANNESS_BIG,    true> address_space_8be_large;
typedef address_space_specific<UINT16, ENDIANNESS_LITTLE, true> address_space_16le_large;
typedef address_space_specific<UINT16, ENDIANNESS_BIG,    true> address_space_16be_large;
typedef address_space_specific<UINT32, ENDIANNESS_LITTLE, true> address_space_32le_large;
typedef address_space_specific<UINT32, ENDIANNESS_BIG,    true> address_space_32be_large;
typedef address_space_specific<UINT64, ENDIANNESS_LITTLE, true> address_space_64le_large;
typedef address_space_specific<UINT64, ENDIANNESS_BIG,    true> address_space_64be_large;


// ======================> _memory_private

// holds internal state for the memory system
struct _memory_private
{
	bool					initialized;					// have we completed initialization?

	UINT8 *					bank_ptr[STATIC_COUNT];			// array of bank pointers
	UINT8 *					bankd_ptr[STATIC_COUNT];		// array of decrypted bank pointers

	simple_list<address_space> spacelist;					// list of address spaces
	simple_list<memory_block> blocklist;					// head of the list of memory blocks

	simple_list<memory_bank> banklist;						// data gathered for each bank
	tagmap_t<memory_bank *>	bankmap;						// map for fast bank lookups
	UINT8					banknext;						// next bank to allocate

	tagmap_t<void *>		sharemap;						// map for share lookups
};



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// global watchpoint table
UINT8 address_table::s_watchpoint_table[1 << LEVEL1_BITS];



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// banking helpers
static STATE_POSTLOAD( bank_reattach );

// debugging
static void generate_memdump(running_machine *machine);



//**************************************************************************
//  CORE SYSTEM OPERATIONS
//**************************************************************************

//-------------------------------------------------
//  memory_init - initialize the memory system
//-------------------------------------------------

void memory_init(running_machine *machine)
{
	// allocate our private data
	memory_private *memdata = machine->memory_data = auto_alloc_clear(machine, memory_private);
	memdata->banknext = STATIC_BANK1;

	// loop over devices and spaces within each device
	device_memory_interface *memory = NULL;
	for (bool gotone = machine->m_devicelist.first(memory); gotone; gotone = memory->next(memory))
		for (int spacenum = 0; spacenum < ADDRESS_SPACES; spacenum++)
		{
			// if there is a configuration for this space, we need an address space
			const address_space_config *spaceconfig = memory->space_config(spacenum);
			if (spaceconfig != NULL)
				memdata->spacelist.append(address_space::allocate(*machine, *spaceconfig, *memory, spacenum));
		}

	// construct and preprocess the address_map for each space
	for (address_space *space = memdata->spacelist.first(); space != NULL; space = space->next())
		space->prepare_map();

	// create the handlers from the resulting address maps
	for (address_space *space = memdata->spacelist.first(); space != NULL; space = space->next())
		space->populate_from_map();

	// allocate memory needed to back each address space
	for (address_space *space = memdata->spacelist.first(); space != NULL; space = space->next())
		space->allocate_memory();

	// find all the allocated pointers
	for (address_space *space = memdata->spacelist.first(); space != NULL; space = space->next())
		space->locate_memory();

	// register a callback to reset banks when reloading state
	state_save_register_postload(machine, bank_reattach, NULL);

	// dump the final memory configuration
	generate_memdump(machine);

	// borrow the first address space to be used as a dummy space
	machine->m_nonspecific_space = memdata->spacelist.first();

	// we are now initialized
	memdata->initialized = true;
}



//**************************************************************************
//  MEMORY BANKING
//**************************************************************************

//-------------------------------------------------
//  memory_configure_bank - configure the
//  addresses for a bank
//-------------------------------------------------

void memory_configure_bank(running_machine *machine, const char *tag, int startentry, int numentries, void *base, offs_t stride)
{
	// validation checks
	memory_bank *bank = machine->memory_data->bankmap.find_hash_only(tag);
	if (bank == NULL)
		fatalerror("memory_configure_bank called for unknown bank '%s'", tag);
	if (base == NULL)
		fatalerror("memory_configure_bank called NULL base");

	// fill in the requested bank entries (backwards to improve allocation)
	for (int entrynum = startentry + numentries - 1; entrynum >= startentry; entrynum--)
		bank->configure(entrynum, reinterpret_cast<UINT8 *>(base) + (entrynum - startentry) * stride);
}


//-------------------------------------------------
//  memory_configure_bank_decrypted - configure
//  the decrypted addresses for a bank
//-------------------------------------------------

void memory_configure_bank_decrypted(running_machine *machine, const char *tag, int startentry, int numentries, void *base, offs_t stride)
{
	// validation checks
	memory_bank *bank = machine->memory_data->bankmap.find_hash_only(tag);
	if (bank == NULL)
		fatalerror("memory_configure_bank_decrypted called for unknown bank '%s'", tag);
	if (base == NULL)
		fatalerror("memory_configure_bank_decrypted called NULL base");

	// fill in the requested bank entries (backwards to improve allocation)
	for (int entrynum = startentry + numentries - 1; entrynum >= startentry; entrynum--)
		bank->configure_decrypted(entrynum, reinterpret_cast<UINT8 *>(base) + (entrynum - startentry) * stride);
}


//-------------------------------------------------
//  memory_set_bank - select one pre-configured
//  entry to be the new bank base
//-------------------------------------------------

void memory_set_bank(running_machine *machine, const char *tag, int entrynum)
{
	// validation checks
	memory_bank *bank = machine->memory_data->bankmap.find_hash_only(tag);
	if (bank == NULL)
		fatalerror("memory_set_bank called for unknown bank '%s'", tag);

	// set the base
	bank->set_entry(entrynum);
}


//-------------------------------------------------
//  memory_get_bank - return the currently
//  selected bank
//-------------------------------------------------

int memory_get_bank(running_machine *machine, const char *tag)
{
	// validation checks
	memory_bank *bank = machine->memory_data->bankmap.find_hash_only(tag);
	if (bank == NULL)
		fatalerror("memory_get_bank called for unknown bank '%s'", tag);

	// return the current entry
	return bank->entry();
}


//-------------------------------------------------
//  memory_set_bankptr - set the base of a bank
//-------------------------------------------------

void memory_set_bankptr(running_machine *machine, const char *tag, void *base)
{
	// validation checks
	memory_bank *bank = machine->memory_data->bankmap.find_hash_only(tag);
	if (bank == NULL)
		throw emu_fatalerror("memory_set_bankptr called for unknown bank '%s'", tag);

	// set the base
	bank->set_base(base);
}


//-------------------------------------------------
//  memory_dump - dump the internal memory tables
//  to the given file
//-------------------------------------------------

void memory_dump(running_machine *machine, FILE *file)
{
	// skip if we can't open the file
	if (file == NULL)
		return;

	// loop over address spaces
	for (address_space *space = machine->memory_data->spacelist.first(); space != NULL; space = space->next())
	{
		fprintf(file, "\n\n"
		              "====================================================\n"
		              "Device '%s' %s address space read handler dump\n"
		              "====================================================\n", space->device().tag(), space->name());
		space->dump_map(file, ROW_READ);

		fprintf(file, "\n\n"
		              "====================================================\n"
		              "Device '%s' %s address space write handler dump\n"
		              "====================================================\n", space->device().tag(), space->name());
		space->dump_map(file, ROW_WRITE);
	}
}


//-------------------------------------------------
//  generate_memdump - internal memory dump
//-------------------------------------------------

static void generate_memdump(running_machine *machine)
{
	if (MEM_DUMP)
	{
		FILE *file = fopen("memdump.log", "w");
		if (file)
		{
			memory_dump(machine, file);
			fclose(file);
		}
	}
}


//-------------------------------------------------
//  bank_reattach - reconnect banks after a load
//-------------------------------------------------

static STATE_POSTLOAD( bank_reattach )
{
	// for each non-anonymous bank, explicitly reset its entry
	for (memory_bank *bank = machine->memory_data->banklist.first(); bank != NULL; bank = bank->next())
		if (!bank->anonymous() && bank->entry() != BANK_ENTRY_UNSPECIFIED)
			bank->set_entry(bank->entry());
}



//**************************************************************************
//  ADDRESS SPACE
//**************************************************************************

//-------------------------------------------------
//  address_space - constructor
//-------------------------------------------------

address_space::address_space(device_memory_interface &memory, int spacenum, bool large)
	: machine(memory.device().machine),
	  cpu(&memory.device()),
	  m_machine(*memory.device().machine),
	  m_next(NULL),
	  m_config(*memory.space_config(spacenum)),
	  m_device(memory.device()),
	  m_map(NULL),
	  m_addrmask(0xffffffffUL >> (32 - m_config.m_addrbus_width)),
	  m_bytemask(address_to_byte_end(m_addrmask)),
	  m_logaddrmask(0xffffffffUL >> (32 - m_config.m_logaddr_width)),
	  m_logbytemask(address_to_byte_end(m_logaddrmask)),
	  m_unmap(0),
	  m_spacenum(spacenum),
	  m_debugger_access(false),
	  m_log_unmap(true),
	  m_direct(*auto_alloc(memory.device().machine, direct_read_data(*this))),
	  m_name(memory.space_config(spacenum)->name()),
	  m_addrchars((m_config.m_databus_width + 3) / 4),
	  m_logaddrchars((m_config.m_logaddr_width + 3) / 4)
{
	// notify the device
	memory.set_address_space(spacenum, *this);
}


//-------------------------------------------------
//  ~address_space - destructor
//-------------------------------------------------

address_space::~address_space()
{
	global_free(&m_direct);
	global_free(m_map);
}


//-------------------------------------------------
//  allocate - static smart allocator of subtypes
//-------------------------------------------------

address_space &address_space::allocate(running_machine &machine, const address_space_config &config, device_memory_interface &memory, int spacenum)
{
	// allocate one of the appropriate type
	bool large = (config.addr2byte_end(0xffffffffUL >> (32 - config.m_addrbus_width)) >= (1 << 18));

	switch (config.data_width())
	{
		case 8:
			if (config.endianness() == ENDIANNESS_LITTLE)
			{
				if (large)
					return *auto_alloc(&machine, address_space_8le_large(memory, spacenum));
				else
					return *auto_alloc(&machine, address_space_8le_small(memory, spacenum));
			}
			else
			{
				if (large)
					return *auto_alloc(&machine, address_space_8be_large(memory, spacenum));
				else
					return *auto_alloc(&machine, address_space_8be_small(memory, spacenum));
			}

		case 16:
			if (config.endianness() == ENDIANNESS_LITTLE)
			{
				if (large)
					return *auto_alloc(&machine, address_space_16le_large(memory, spacenum));
				else
					return *auto_alloc(&machine, address_space_16le_small(memory, spacenum));
			}
			else
			{
				if (large)
					return *auto_alloc(&machine, address_space_16be_large(memory, spacenum));
				else
					return *auto_alloc(&machine, address_space_16be_small(memory, spacenum));
			}

		case 32:
			if (config.endianness() == ENDIANNESS_LITTLE)
			{
				if (large)
					return *auto_alloc(&machine, address_space_32le_large(memory, spacenum));
				else
					return *auto_alloc(&machine, address_space_32le_small(memory, spacenum));
			}
			else
			{
				if (large)
					return *auto_alloc(&machine, address_space_32be_large(memory, spacenum));
				else
					return *auto_alloc(&machine, address_space_32be_small(memory, spacenum));
			}

		case 64:
			if (config.endianness() == ENDIANNESS_LITTLE)
			{
				if (large)
					return *auto_alloc(&machine, address_space_64le_large(memory, spacenum));
				else
					return *auto_alloc(&machine, address_space_64le_small(memory, spacenum));
			}
			else
			{
				if (large)
					return *auto_alloc(&machine, address_space_64be_large(memory, spacenum));
				else
					return *auto_alloc(&machine, address_space_64be_small(memory, spacenum));
			}
	}
	throw emu_fatalerror("Invalid width %d specified for address_space::allocate", config.data_width());
}


//-------------------------------------------------
//  adjust_addresses - adjust addresses for a
//  given address space in a standard fashion
//-------------------------------------------------

inline void address_space::adjust_addresses(offs_t &start, offs_t &end, offs_t &mask, offs_t &mirror)
{
	// adjust start/end/mask values
	if (mask == 0)
		mask = m_addrmask & ~mirror;
	else
		mask &= m_addrmask;
	start &= ~mirror & m_addrmask;
	end &= ~mirror & m_addrmask;

	// adjust to byte values
	start = address_to_byte(start);
	end = address_to_byte_end(end);
	mask = address_to_byte_end(mask);
	mirror = address_to_byte(mirror);
}


//-------------------------------------------------
//  prepare_map - allocate the address map and
//  walk through it to find implcit memory regions
//  and identify shared regions
//-------------------------------------------------

void address_space::prepare_map()
{
	const region_info *devregion = (m_spacenum == ADDRESS_SPACE_0) ? m_machine.region(m_device.tag()) : NULL;
	UINT32 devregionsize = (devregion != NULL) ? devregion->bytes() : 0;

	// allocate the address map
	m_map = global_alloc(address_map(m_device.baseconfig(), m_spacenum));

	// extract global parameters specified by the map
	m_unmap = (m_map->m_unmapval == 0) ? 0 : ~0;
	if (m_map->m_globalmask != 0)
	{
		m_addrmask = m_map->m_globalmask;
		m_bytemask = address_to_byte_end(m_addrmask);
	}

	// make a pass over the address map, adjusting for the device and getting memory pointers
	for (address_map_entry *entry = m_map->m_entrylist.first(); entry != NULL; entry = entry->next())
	{
		// if we have a share entry, add it to our map
		if (entry->m_share != NULL)
			m_machine.memory_data->sharemap.add(entry->m_share, UNMAPPED_SHARE_PTR, false);

		// computed adjusted addresses first
		entry->m_bytestart = entry->m_addrstart;
		entry->m_byteend = entry->m_addrend;
		entry->m_bytemirror = entry->m_addrmirror;
		entry->m_bytemask = entry->m_addrmask;
		adjust_addresses(entry->m_bytestart, entry->m_byteend, entry->m_bytemask, entry->m_bytemirror);

		// if this is a ROM handler without a specified region, attach it to the implicit region
		if (m_spacenum == ADDRESS_SPACE_0 && entry->m_read.m_type == AMH_ROM && entry->m_region == NULL)
		{
			// make sure it fits within the memory region before doing so, however
			if (entry->m_byteend < devregionsize)
			{
				entry->m_region = m_device.tag();
				entry->m_rgnoffs = entry->m_bytestart;
			}
		}

		// validate adjusted addresses against implicit regions
		if (entry->m_region != NULL && entry->m_share == NULL && entry->m_baseptr == NULL)
		{
			const region_info *region = m_machine.region(entry->m_region);
			if (region == NULL)
				fatalerror("Error: device '%s' %s space memory map entry %X-%X references non-existant region \"%s\"", m_device.tag(), m_name, entry->m_addrstart, entry->m_addrend, entry->m_region);

			// validate the region
			if (entry->m_rgnoffs + (entry->m_byteend - entry->m_bytestart + 1) > region->bytes())
				fatalerror("Error: device '%s' %s space memory map entry %X-%X extends beyond region \"%s\" size (%X)", m_device.tag(), m_name, entry->m_addrstart, entry->m_addrend, entry->m_region, region->bytes());
		}

		// convert any region-relative entries to their memory pointers
		if (entry->m_region != NULL)
			entry->m_memory = m_machine.region(entry->m_region)->base() + entry->m_rgnoffs;
	}

	// now loop over all the handlers and enforce the address mask
	read().mask_all_handlers(m_bytemask);
	write().mask_all_handlers(m_bytemask);
}


//-------------------------------------------------
//  populate_from_map - walk the map in reverse
//  order and install the appropriate handler for
//  each case
//-------------------------------------------------

void address_space::populate_from_map()
{
	// no map, nothing to do
	if (m_map == NULL)
		return;

	// install the handlers, using the original, unadjusted memory map
	const address_map_entry *last_entry = NULL;
	while (last_entry != m_map->m_entrylist.first())
	{
		// find the entry before the last one we processed
		const address_map_entry *entry;
		for (entry = m_map->m_entrylist.first(); entry->next() != last_entry; entry = entry->next()) ;
		last_entry = entry;

		// map both read and write halves
		populate_map_entry(*entry, ROW_READ);
		populate_map_entry(*entry, ROW_WRITE);
	}
}


//-------------------------------------------------
//  populate_map_entry - map a single read or
//  write entry based on information from an
//  address map entry
//-------------------------------------------------

void address_space::populate_map_entry(const address_map_entry &entry, read_or_write readorwrite)
{
	const map_handler_data &data = (readorwrite == ROW_READ) ? entry.m_read : entry.m_write;
	bindable_object *object;
	device_t *device;

	// based on the handler type, alter the bits, name, funcptr, and object
	switch (data.m_type)
	{
		case AMH_NONE:
			return;

		case AMH_ROM:
			// writes to ROM are no-ops
			if (readorwrite == ROW_WRITE)
				return;
			// fall through to the RAM case otherwise

		case AMH_RAM:
			install_ram(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, readorwrite);
			break;

		case AMH_NOP:
			unmap(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, readorwrite, true);
			break;

		case AMH_UNMAP:
			unmap(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, readorwrite, false);
			break;

		case AMH_DRIVER_DELEGATE:
		case AMH_DEVICE_DELEGATE:
			if (data.m_type == AMH_DRIVER_DELEGATE)
			{
				object = m_machine.driver_data<driver_data_t>();
				if (object == NULL)
					throw emu_fatalerror("Attempted to map a driver delegate in space %s of device '%s' when there is no driver data\n", m_name, m_device.tag());
			}
			else
			{
				object = m_machine.device(data.m_tag);
				if (object == NULL)
					throw emu_fatalerror("Attempted to map a non-existent device '%s' in space %s of device '%s'\n", data.m_tag, m_name, m_device.tag());
			}

			if (readorwrite == ROW_READ)
				switch (data.m_bits)
				{
					case 8:		install_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, read8_delegate(entry.m_rproto8, *object), data.m_mask);		break;
					case 16:	install_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, read16_delegate(entry.m_rproto16, *object), data.m_mask);		break;
					case 32:	install_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, read32_delegate(entry.m_rproto32, *object), data.m_mask);		break;
					case 64:	install_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, read64_delegate(entry.m_rproto64, *object), data.m_mask);		break;
				}
			else
				switch (data.m_bits)
				{
					case 8:		install_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, write8_delegate(entry.m_wproto8, *object), data.m_mask);		break;
					case 16:	install_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, write16_delegate(entry.m_wproto16, *object), data.m_mask);	break;
					case 32:	install_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, write32_delegate(entry.m_wproto32, *object), data.m_mask);	break;
					case 64:	install_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, write64_delegate(entry.m_wproto64, *object), data.m_mask);	break;
				}
			break;

		case AMH_LEGACY_SPACE_HANDLER:
			if (readorwrite == ROW_READ)
				switch (data.m_bits)
				{
					case 8:		install_legacy_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_rspace8, data.m_name, data.m_mask);	break;
					case 16:	install_legacy_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_rspace16, data.m_name, data.m_mask);	break;
					case 32:	install_legacy_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_rspace32, data.m_name, data.m_mask);	break;
					case 64:	install_legacy_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_rspace64, data.m_name, data.m_mask);	break;
				}
			else
				switch (data.m_bits)
				{
					case 8:		install_legacy_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_wspace8, data.m_name, data.m_mask);	break;
					case 16:	install_legacy_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_wspace16, data.m_name, data.m_mask);	break;
					case 32:	install_legacy_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_wspace32, data.m_name, data.m_mask);	break;
					case 64:	install_legacy_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_wspace64, data.m_name, data.m_mask);	break;
				}
			break;

		case AMH_LEGACY_DEVICE_HANDLER:
			device = m_machine.device(data.m_tag);
			if (device == NULL)
				fatalerror("Attempted to map a non-existent device '%s' in space %s of device '%s'\n", data.m_tag, m_name, m_device.tag());

			if (readorwrite == ROW_READ)
				switch (data.m_bits)
				{
					case 8:		install_legacy_handler(*device, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_rdevice8, data.m_name, data.m_mask);	break;
					case 16:	install_legacy_handler(*device, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_rdevice16, data.m_name, data.m_mask);	break;
					case 32:	install_legacy_handler(*device, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_rdevice32, data.m_name, data.m_mask);	break;
					case 64:	install_legacy_handler(*device, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_rdevice64, data.m_name, data.m_mask);	break;
				}
			else
				switch (data.m_bits)
				{
					case 8:		install_legacy_handler(*device, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_wdevice8, data.m_name, data.m_mask);	break;
					case 16:	install_legacy_handler(*device, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_wdevice16, data.m_name, data.m_mask);	break;
					case 32:	install_legacy_handler(*device, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_wdevice32, data.m_name, data.m_mask);	break;
					case 64:	install_legacy_handler(*device, entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_wdevice64, data.m_name, data.m_mask);	break;
				}
			break;

		case AMH_PORT:
			install_port(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror,
							(readorwrite == ROW_READ) ? data.m_tag : NULL,
							(readorwrite == ROW_WRITE) ? data.m_tag : NULL);
			break;

		case AMH_BANK:
			install_bank(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror,
							(readorwrite == ROW_READ) ? data.m_tag : NULL,
							(readorwrite == ROW_WRITE) ? data.m_tag : NULL);
			break;
	}
}


//-------------------------------------------------
//  allocate_memory - determine all neighboring
//  address ranges and allocate memory to back
//  them
//-------------------------------------------------

void address_space::allocate_memory()
{
	simple_list<memory_block> &blocklist = m_machine.memory_data->blocklist;

	// make a first pass over the memory map and track blocks with hardcoded pointers
	// we do this to make sure they are found by space_find_backing_memory first
	memory_block *prev_memblock_tail = blocklist.last();
	for (address_map_entry *entry = m_map->m_entrylist.first(); entry != NULL; entry = entry->next())
		if (entry->m_memory != NULL)
			blocklist.append(*auto_alloc(&m_machine, memory_block(*this, entry->m_bytestart, entry->m_byteend, entry->m_memory)));

	// loop over all blocks just allocated and assign pointers from them
	address_map_entry *unassigned = NULL;
	memory_block *first_new_block = (prev_memblock_tail != NULL) ? prev_memblock_tail->next() : blocklist.first();
	for (memory_block *memblock = first_new_block; memblock != NULL; memblock = memblock->next())
		unassigned = block_assign_intersecting(memblock->bytestart(), memblock->byteend(), memblock->data());

	// if we don't have an unassigned pointer yet, try to find one
	if (unassigned == NULL)
		unassigned = block_assign_intersecting(~0, 0, NULL);

	// loop until we've assigned all memory in this space
	while (unassigned != NULL)
	{
		// work in MEMORY_BLOCK_CHUNK-sized chunks
		offs_t curblockstart = unassigned->m_bytestart / MEMORY_BLOCK_CHUNK;
		offs_t curblockend = unassigned->m_byteend / MEMORY_BLOCK_CHUNK;

		// loop while we keep finding unassigned blocks in neighboring MEMORY_BLOCK_CHUNK chunks
		bool changed;
		do
		{
			changed = false;

			// scan for unmapped blocks in the adjusted map
			for (address_map_entry *entry = m_map->m_entrylist.first(); entry != NULL; entry = entry->next())
				if (entry->m_memory == NULL && entry != unassigned && needs_backing_store(entry))
				{
					// get block start/end blocks for this block
					offs_t blockstart = entry->m_bytestart / MEMORY_BLOCK_CHUNK;
					offs_t blockend = entry->m_byteend / MEMORY_BLOCK_CHUNK;

					// if we intersect or are adjacent, adjust the start/end
					if (blockstart <= curblockend + 1 && blockend >= curblockstart - 1)
					{
						if (blockstart < curblockstart)
							curblockstart = blockstart, changed = true;
						if (blockend > curblockend)
							curblockend = blockend, changed = true;
					}
				}
		} while (changed);

		// we now have a block to allocate; do it
		offs_t curbytestart = curblockstart * MEMORY_BLOCK_CHUNK;
		offs_t curbyteend = curblockend * MEMORY_BLOCK_CHUNK + (MEMORY_BLOCK_CHUNK - 1);
		memory_block &block = blocklist.append(*auto_alloc(&m_machine, memory_block(*this, curbytestart, curbyteend)));

		// assign memory that intersected the new block
		unassigned = block_assign_intersecting(curbytestart, curbyteend, block.data());
	}
}


//-------------------------------------------------
//  locate_memory - find all the requested
//  pointers into the final allocated memory
//-------------------------------------------------

void address_space::locate_memory()
{
	// fill in base/size entries
	for (const address_map_entry *entry = m_map->m_entrylist.first(); entry != NULL; entry = entry->next())
	{
		if (entry->m_baseptr != NULL)
			*entry->m_baseptr = entry->m_memory;
		if (entry->m_baseptroffs_plus1 != 0)
			*(void **)(reinterpret_cast<UINT8 *>(m_machine.driver_data<void>()) + entry->m_baseptroffs_plus1 - 1) = entry->m_memory;
		if (entry->m_genbaseptroffs_plus1 != 0)
			*(void **)((UINT8 *)&m_machine.generic + entry->m_genbaseptroffs_plus1 - 1) = entry->m_memory;
		if (entry->m_sizeptr != NULL)
			*entry->m_sizeptr = entry->m_byteend - entry->m_bytestart + 1;
		if (entry->m_sizeptroffs_plus1 != 0)
			*(size_t *)(reinterpret_cast<UINT8 *>(m_machine.driver_data<void>()) + entry->m_sizeptroffs_plus1 - 1) = entry->m_byteend - entry->m_bytestart + 1;
		if (entry->m_gensizeptroffs_plus1 != 0)
			*(size_t *)((UINT8 *)&m_machine.generic + entry->m_gensizeptroffs_plus1 - 1) = entry->m_byteend - entry->m_bytestart + 1;
	}

	// once this is done, find the starting bases for the banks
	for (memory_bank *bank = m_machine.memory_data->banklist.first(); bank != NULL; bank = bank->next())
		if (bank->base() == NULL && bank->references_space(*this, ROW_READWRITE))
		{
			// set the initial bank pointer
			for (address_map_entry *entry = m_map->m_entrylist.first(); entry != NULL; entry = entry->next())
				if (entry->m_bytestart == bank->bytestart() && entry->m_memory != NULL)
				{
					bank->set_base(entry->m_memory);
					VPRINTF(("assigned bank '%s' pointer to memory from range %08X-%08X [%p]\n", bank->tag(), entry->m_addrstart, entry->m_addrend, entry->m_memory));
					break;
				}

			// if the entry was set ahead of time, override the automatically found pointer
			if (!bank->anonymous() && bank->entry() != BANK_ENTRY_UNSPECIFIED)
				bank->set_entry(bank->entry());
		}
}


//-------------------------------------------------
//  set_decrypted_region - registers an address
//  range as having a decrypted data pointer
//-------------------------------------------------

void address_space::set_decrypted_region(offs_t addrstart, offs_t addrend, void *base)
{
	offs_t bytestart = address_to_byte(addrstart);
	offs_t byteend = address_to_byte_end(addrend);
	bool found = false;

	// loop over banks looking for a match
	for (memory_bank *bank = m_machine.memory_data->banklist.first(); bank != NULL; bank = bank->next())
	{
		// consider this bank if it is used for reading and matches the address space
		if (bank->references_space(*this, ROW_READ))
		{
			// verify that the provided range fully covers this bank
			if (bank->is_covered_by(bytestart, byteend))
			{
				// set the decrypted pointer for the corresponding memory bank
				bank->set_base_decrypted(reinterpret_cast<UINT8 *>(base) + bank->bytestart() - bytestart);
				found = true;
			}

			// fatal error if the decrypted region straddles the bank
			else if (bank->straddles(bytestart, byteend))
				throw emu_fatalerror("memory_set_decrypted_region found straddled region %08X-%08X for device '%s'", bytestart, byteend, m_device.tag());
		}
	}

	// fatal error as well if we didn't find any relevant memory banks
	if (!found)
		throw emu_fatalerror("memory_set_decrypted_region unable to find matching region %08X-%08X for device '%s'", bytestart, byteend, m_device.tag());
}


//-------------------------------------------------
//  block_assign_intersecting - find all
//  intersecting blocks and assign their pointers
//-------------------------------------------------

address_map_entry *address_space::block_assign_intersecting(offs_t bytestart, offs_t byteend, UINT8 *base)
{
	memory_private *memdata = m_machine.memory_data;
	address_map_entry *unassigned = NULL;

	// loop over the adjusted map and assign memory to any blocks we can
	for (address_map_entry *entry = m_map->m_entrylist.first(); entry != NULL; entry = entry->next())
	{
		// if we haven't assigned this block yet, see if we have a mapped shared pointer for it
		if (entry->m_memory == NULL && entry->m_share != NULL)
		{
			void *shareptr = memdata->sharemap.find(entry->m_share);
			if (shareptr != UNMAPPED_SHARE_PTR)
			{
				entry->m_memory = shareptr;
				VPRINTF(("memory range %08X-%08X -> shared_ptr '%s' [%p]\n", entry->m_addrstart, entry->m_addrend, entry->m_share, entry->m_memory));
			}
		}

		// otherwise, look for a match in this block
		if (entry->m_memory == NULL && entry->m_bytestart >= bytestart && entry->m_byteend <= byteend)
		{
			entry->m_memory = base + (entry->m_bytestart - bytestart);
			VPRINTF(("memory range %08X-%08X -> found in block from %08X-%08X [%p]\n", entry->m_addrstart, entry->m_addrend, bytestart, byteend, entry->m_memory));
		}

		// if we're the first match on a shared pointer, assign it now
		if (entry->m_memory != NULL && entry->m_share != NULL)
		{
			void *shareptr = memdata->sharemap.find(entry->m_share);
			if (shareptr == UNMAPPED_SHARE_PTR)
				memdata->sharemap.add(entry->m_share, entry->m_memory, TRUE);
		}

		// keep track of the first unassigned entry
		if (entry->m_memory == NULL && unassigned == NULL && needs_backing_store(entry))
			unassigned = entry;
	}

	return unassigned;
}


//-------------------------------------------------
//  get_handler_string - return a string
//  describing the handler at a particular offset
//-------------------------------------------------

const char *address_space::get_handler_string(read_or_write readorwrite, offs_t byteaddress)
{
	if (readorwrite == ROW_READ)
		return read().handler_name(read().lookup(byteaddress));
	else
		return write().handler_name(write().lookup(byteaddress));
}


//-------------------------------------------------
//  dump_map - dump the contents of a single
//  address space
//-------------------------------------------------

void address_space::dump_map(FILE *file, read_or_write readorwrite)
{
	const address_table &table = (readorwrite == ROW_READ) ? static_cast<address_table &>(read()) : static_cast<address_table &>(write());

	// dump generic information
	fprintf(file, "  Address bits = %d\n", m_config.m_addrbus_width);
	fprintf(file, "     Data bits = %d\n", m_config.m_databus_width);
	fprintf(file, "  Address mask = %X\n", m_bytemask);
	fprintf(file, "\n");

	// iterate over addresses
	offs_t bytestart, byteend;
	for (offs_t byteaddress = 0; byteaddress <= m_bytemask; byteaddress = byteend)
	{
		UINT8 entry = table.derive_range(byteaddress, bytestart, byteend);
		fprintf(file, "%08X-%08X    = %02X: %s [offset=%08X]\n",
						bytestart, byteend, entry, table.handler_name(entry), table.handler(entry).bytestart());
		if (++byteend == 0)
			break;
	}
}


//**************************************************************************
//  DYNAMIC ADDRESS SPACE MAPPING
//**************************************************************************

//-------------------------------------------------
//  unmap - unmap a section of address space
//-------------------------------------------------

void address_space::unmap(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite, bool quiet)
{
	VPRINTF(("address_space::unmap(%s-%s mask=%s mirror=%s, %s, %s)\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 (readorwrite == ROW_READ) ? "read" : (readorwrite == ROW_WRITE) ? "write" : (readorwrite == ROW_READWRITE) ? "read/write" : "??",
			 quiet ? "quiet" : "normal"));

	// read space
	if (readorwrite == ROW_READ || readorwrite == ROW_READWRITE)
		read().map_range(addrstart, addrend, addrmask, addrmirror, quiet ? STATIC_NOP : STATIC_UNMAP);

	// write space
	if (readorwrite == ROW_WRITE || readorwrite == ROW_READWRITE)
		write().map_range(addrstart, addrend, addrmask, addrmirror, quiet ? STATIC_NOP : STATIC_UNMAP);
}


//-------------------------------------------------
//  install_port - install a new I/O port handler
//  into this address space
//-------------------------------------------------

void address_space::install_port(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag)
{
	VPRINTF(("address_space::install_port(%s-%s mask=%s mirror=%s, read=\"%s\" / write=\"%s\")\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 (rtag != NULL) ? rtag : "(none)", (wtag != NULL) ? wtag : "(none)"));

	// read handler
	if (rtag != NULL)
	{
		// find the port
		const input_port_config *port = m_machine.port(rtag);
		if (port == NULL)
			throw emu_fatalerror("Attempted to map non-existent port '%s' for read in space %s of device '%s'\n", rtag, m_name, m_device.tag());

		// map the range and set the ioport
		UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
		read().handler_read(entry).set_ioport(*port);
	}

	if (wtag != NULL)
	{
		// find the port
		const input_port_config *port = m_machine.port(wtag);
		if (port == NULL)
			fatalerror("Attempted to map non-existent port '%s' for write in space %s of device '%s'\n", wtag, m_name, m_device.tag());

		// map the range and set the ioport
		UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
		write().handler_write(entry).set_ioport(*port);
	}

	// update the memory dump
	generate_memdump(&m_machine);
}


//-------------------------------------------------
//  install_bank - install a range as mapping to
//  a particular bank
//-------------------------------------------------

void address_space::install_bank(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag)
{
	VPRINTF(("address_space::install_bank(%s-%s mask=%s mirror=%s, read=\"%s\" / write=\"%s\")\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 (rtag != NULL) ? rtag : "(none)", (wtag != NULL) ? wtag : "(none)"));

	// map the read bank
	if (rtag != NULL)
	{
		memory_bank &bank = bank_find_or_allocate(rtag, addrstart, addrend, addrmask, addrmirror, ROW_READ);
		read().map_range(addrstart, addrend, addrmask, addrmirror, bank.index());
	}

	// map the write bank
	if (wtag != NULL)
	{
		memory_bank &bank = bank_find_or_allocate(wtag, addrstart, addrend, addrmask, addrmirror, ROW_WRITE);
		write().map_range(addrstart, addrend, addrmask, addrmirror, bank.index());
	}

	// update the memory dump
	generate_memdump(&m_machine);
}


//-------------------------------------------------
//  install_ram - install a simple fixed RAM
//  region into the given address space
//-------------------------------------------------

void *address_space::install_ram(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite, void *baseptr)
{
	memory_private *memdata = m_machine.memory_data;

	VPRINTF(("address_space::install_ram(%s-%s mask=%s mirror=%s, %s, %p)\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 (readorwrite == ROW_READ) ? "read" : (readorwrite == ROW_WRITE) ? "write" : (readorwrite == ROW_READWRITE) ? "read/write" : "??",
			 baseptr));

	// map for read
	if (readorwrite == ROW_READ || readorwrite == ROW_READWRITE)
	{
		// find a bank and map it
		memory_bank &bank = bank_find_or_allocate(NULL, addrstart, addrend, addrmask, addrmirror, ROW_READ);
		read().map_range(addrstart, addrend, addrmask, addrmirror, bank.index());

		// if we are provided a pointer, set it
		if (baseptr != NULL)
			bank.set_base(baseptr);

		// if we don't have a bank pointer yet, try to find one
		if (bank.base() == NULL)
		{
			void *backing = find_backing_memory(addrstart, addrend);
			if (backing != NULL)
				bank.set_base(backing);
		}

		// if we still don't have a pointer, and we're past the initialization phase, allocate a new block
		if (bank.base() == NULL && memdata->initialized)
		{
			if (m_machine.phase() >= MACHINE_PHASE_RESET)
				fatalerror("Attempted to call memory_install_ram() after initialization time without a baseptr!");
			memory_block &block = memdata->blocklist.append(*auto_alloc(&m_machine, memory_block(*this, address_to_byte(addrstart), address_to_byte_end(addrend))));
			bank.set_base(block.data());
		}
	}

	// map for write
	if (readorwrite == ROW_WRITE || readorwrite == ROW_READWRITE)
	{
		// find a bank and map it
		memory_bank &bank = bank_find_or_allocate(NULL, addrstart, addrend, addrmask, addrmirror, ROW_WRITE);
		write().map_range(addrstart, addrend, addrmask, addrmirror, bank.index());

		// if we are provided a pointer, set it
		if (baseptr != NULL)
			bank.set_base(baseptr);

		// if we don't have a bank pointer yet, try to find one
		if (bank.base() == NULL)
		{
			void *backing = find_backing_memory(addrstart, addrend);
			if (backing != NULL)
				bank.set_base(backing);
		}

		// if we still don't have a pointer, and we're past the initialization phase, allocate a new block
		if (bank.base() == NULL && memdata->initialized)
		{
			if (m_machine.phase() >= MACHINE_PHASE_RESET)
				fatalerror("Attempted to call memory_install_ram() after initialization time without a baseptr!");
			memory_block &block = memdata->blocklist.append(*auto_alloc(&m_machine, memory_block(*this, address_to_byte(addrstart), address_to_byte_end(addrend))));
			bank.set_base(block.data());
		}
	}

	return (void *)find_backing_memory(addrstart, addrend);
}


//-------------------------------------------------
//  install_handler - install 8-bit read/write
//  delegate handlers for the space
//-------------------------------------------------

UINT8 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_delegate handler, UINT64 unitmask)
{
	VPRINTF(("address_space::install_handler(%s-%s mask=%s mirror=%s, %s, %s)\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 handler.name(), core_i64_hex_format(unitmask, data_width() / 4)));

	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_delegate(handler, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT8 *>(find_backing_memory(addrstart, addrend));
}

UINT8 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_delegate handler, UINT64 unitmask)
{
	VPRINTF(("address_space::install_handler(%s-%s mask=%s mirror=%s, %s, %s)\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 handler.name(), core_i64_hex_format(unitmask, data_width() / 4)));

	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_delegate(handler, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT8 *>(find_backing_memory(addrstart, addrend));
}

UINT8 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_delegate rhandler, write8_delegate whandler, UINT64 unitmask)
{
	install_handler(addrstart, addrend, addrmask, addrmirror, rhandler, unitmask);
	return install_handler(addrstart, addrend, addrmask, addrmirror, whandler, unitmask);
}


//-------------------------------------------------
//  install_legacy_handler - install 8-bit read/
//  write legacy address space handlers for the
//  space
//-------------------------------------------------

UINT8 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, const char *rname, UINT64 unitmask)
{
	VPRINTF(("address_space::install_legacy_handler(%s-%s mask=%s mirror=%s, %s, %s) [read8]\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 rname, core_i64_hex_format(unitmask, data_width() / 4)));

	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_legacy_func(*this, rhandler, rname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT8 *>(find_backing_memory(addrstart, addrend));
}

UINT8 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_space_func whandler, const char *wname, UINT64 unitmask)
{
	VPRINTF(("address_space::install_legacy_handler(%s-%s mask=%s mirror=%s, %s, %s) [write8]\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 wname, core_i64_hex_format(unitmask, data_width() / 4)));

	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_legacy_func(*this, whandler, wname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT8 *>(find_backing_memory(addrstart, addrend));
}

UINT8 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, const char *rname, write8_space_func whandler, const char *wname, UINT64 unitmask)
{
	install_legacy_handler(addrstart, addrend, addrmask, addrmirror, rhandler, rname, unitmask);
	return install_legacy_handler(addrstart, addrend, addrmask, addrmirror, whandler, wname, unitmask);
}


//-------------------------------------------------
//  install_legacy_handler - install 8-bit read/
//  write legacy device handlers for the space
//-------------------------------------------------

UINT8 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, const char *rname, UINT64 unitmask)
{
	VPRINTF(("address_space::install_legacy_handler(%s-%s mask=%s mirror=%s, %s, %s, \"%s\") [read8]\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 rname, core_i64_hex_format(unitmask, data_width() / 4), device.tag()));

	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_legacy_func(device, rhandler, rname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT8 *>(find_backing_memory(addrstart, addrend));
}

UINT8 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_device_func whandler, const char *wname, UINT64 unitmask)
{
	VPRINTF(("address_space::install_legacy_handler(%s-%s mask=%s mirror=%s, %s, %s, \"%s\") [write8]\n",
			 core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
			 core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
			 wname, core_i64_hex_format(unitmask, data_width() / 4), device.tag()));

	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_legacy_func(device, whandler, wname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT8 *>(find_backing_memory(addrstart, addrend));
}

UINT8 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, const char *rname, write8_device_func whandler, const char *wname, UINT64 unitmask)
{
	install_legacy_handler(device, addrstart, addrend, addrmask, addrmirror, rhandler, rname, unitmask);
	return install_legacy_handler(device, addrstart, addrend, addrmask, addrmirror, whandler, wname, unitmask);
}


//-------------------------------------------------
//  install_handler - install 16-bit read/write
//  delegate handlers for the space
//-------------------------------------------------

UINT16 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_delegate handler, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_delegate(handler, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT16 *>(find_backing_memory(addrstart, addrend));
}

UINT16 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_delegate handler, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_delegate(handler, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT16 *>(find_backing_memory(addrstart, addrend));
}

UINT16 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_delegate rhandler, write16_delegate whandler, UINT64 unitmask)
{
	install_handler(addrstart, addrend, addrmask, addrmirror, rhandler, unitmask);
	return install_handler(addrstart, addrend, addrmask, addrmirror, whandler, unitmask);
}


//-------------------------------------------------
//  install_legacy_handler - install 16-bit read/
//  write legacy address space handlers for the
//  space
//-------------------------------------------------

UINT16 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, const char *rname, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_legacy_func(*this, rhandler, rname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT16 *>(find_backing_memory(addrstart, addrend));
}

UINT16 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_space_func whandler, const char *wname, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_legacy_func(*this, whandler, wname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT16 *>(find_backing_memory(addrstart, addrend));
}

UINT16 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, const char *rname, write16_space_func whandler, const char *wname, UINT64 unitmask)
{
	install_legacy_handler(addrstart, addrend, addrmask, addrmirror, rhandler, rname, unitmask);
	return install_legacy_handler(addrstart, addrend, addrmask, addrmirror, whandler, wname, unitmask);
}


//-------------------------------------------------
//  install_legacy_handler - install 16-bit read/
//  write legacy device handlers for the space
//-------------------------------------------------

UINT16 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, const char *rname, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_legacy_func(device, rhandler, rname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT16 *>(find_backing_memory(addrstart, addrend));
}

UINT16 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_device_func whandler, const char *wname, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_legacy_func(device, whandler, wname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT16 *>(find_backing_memory(addrstart, addrend));
}

UINT16 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, const char *rname, write16_device_func whandler, const char *wname, UINT64 unitmask)
{
	install_legacy_handler(device, addrstart, addrend, addrmask, addrmirror, rhandler, rname, unitmask);
	return install_legacy_handler(device, addrstart, addrend, addrmask, addrmirror, whandler, wname, unitmask);
}


//-------------------------------------------------
//  install_handler - install 32-bit read/write
//  delegate handlers for the space
//-------------------------------------------------

UINT32 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_delegate handler, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_delegate(handler, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT32 *>(find_backing_memory(addrstart, addrend));
}

UINT32 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_delegate handler, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_delegate(handler, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT32 *>(find_backing_memory(addrstart, addrend));
}

UINT32 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_delegate rhandler, write32_delegate whandler, UINT64 unitmask)
{
	install_handler(addrstart, addrend, addrmask, addrmirror, rhandler, unitmask);
	return install_handler(addrstart, addrend, addrmask, addrmirror, whandler, unitmask);
}


//-------------------------------------------------
//  install_legacy_handler - install 32-bit read/
//  write legacy address space handlers for the
//  space
//-------------------------------------------------

UINT32 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, const char *rname, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_legacy_func(*this, rhandler, rname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT32 *>(find_backing_memory(addrstart, addrend));
}

UINT32 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_space_func whandler, const char *wname, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_legacy_func(*this, whandler, wname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT32 *>(find_backing_memory(addrstart, addrend));
}

UINT32 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, const char *rname, write32_space_func whandler, const char *wname, UINT64 unitmask)
{
	install_legacy_handler(addrstart, addrend, addrmask, addrmirror, rhandler, rname, unitmask);
	return install_legacy_handler(addrstart, addrend, addrmask, addrmirror, whandler, wname, unitmask);
}


//-------------------------------------------------
//  install_legacy_handler - install 32-bit read/
//  write legacy device handlers for the space
//-------------------------------------------------

UINT32 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, const char *rname, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_legacy_func(device, rhandler, rname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT32 *>(find_backing_memory(addrstart, addrend));
}

UINT32 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_device_func whandler, const char *wname, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_legacy_func(device, whandler, wname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT32 *>(find_backing_memory(addrstart, addrend));
}

UINT32 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, const char *rname, write32_device_func whandler, const char *wname, UINT64 unitmask)
{
	install_legacy_handler(device, addrstart, addrend, addrmask, addrmirror, rhandler, rname, unitmask);
	return install_legacy_handler(device, addrstart, addrend, addrmask, addrmirror, whandler, wname, unitmask);
}


//-------------------------------------------------
//  install_handler64 - install 64-bit read/write
//  delegate handlers for the space
//-------------------------------------------------

UINT64 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_delegate handler, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_delegate(handler, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT64 *>(find_backing_memory(addrstart, addrend));
}

UINT64 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_delegate handler, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_delegate(handler, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT64 *>(find_backing_memory(addrstart, addrend));
}

UINT64 *address_space::install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_delegate rhandler, write64_delegate whandler, UINT64 unitmask)
{
	install_handler(addrstart, addrend, addrmask, addrmirror, rhandler, unitmask);
	return install_handler(addrstart, addrend, addrmask, addrmirror, whandler, unitmask);
}


//-------------------------------------------------
//  install_legacy_handler - install 64-bit read/
//  write legacy address space handlers for the
//  space
//-------------------------------------------------

UINT64 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, const char *rname, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_legacy_func(*this, rhandler, rname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT64 *>(find_backing_memory(addrstart, addrend));
}

UINT64 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_space_func whandler, const char *wname, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_legacy_func(*this, whandler, wname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT64 *>(find_backing_memory(addrstart, addrend));
}

UINT64 *address_space::install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, const char *rname, write64_space_func whandler, const char *wname, UINT64 unitmask)
{
	install_legacy_handler(addrstart, addrend, addrmask, addrmirror, rhandler, rname, unitmask);
	return install_legacy_handler(addrstart, addrend, addrmask, addrmirror, whandler, wname, unitmask);
}


//-------------------------------------------------
//  install_legacy_handler - install 64-bit read/
//  write legacy device handlers for the space
//-------------------------------------------------

UINT64 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, const char *rname, UINT64 unitmask)
{
	UINT32 entry = read().map_range(addrstart, addrend, addrmask, addrmirror);
	read().handler_read(entry).set_legacy_func(device, rhandler, rname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT64 *>(find_backing_memory(addrstart, addrend));
}

UINT64 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_device_func whandler, const char *wname, UINT64 unitmask)
{
	UINT32 entry = write().map_range(addrstart, addrend, addrmask, addrmirror);
	write().handler_write(entry).set_legacy_func(device, whandler, wname, unitmask);
	generate_memdump(machine);
	return reinterpret_cast<UINT64 *>(find_backing_memory(addrstart, addrend));
}

UINT64 *address_space::install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, const char *rname, write64_device_func whandler, const char *wname, UINT64 unitmask)
{
	install_legacy_handler(device, addrstart, addrend, addrmask, addrmirror, rhandler, rname, unitmask);
	return install_legacy_handler(device, addrstart, addrend, addrmask, addrmirror, whandler, wname, unitmask);
}



//**************************************************************************
//  INTERNAL INITIALIZATION
//**************************************************************************


//**************************************************************************
//  MEMORY MAPPING HELPERS
//**************************************************************************

//-------------------------------------------------
//  find_backing_memory - return a pointer to
//  the base of RAM associated with the given
//  device and offset
//-------------------------------------------------

void *address_space::find_backing_memory(offs_t addrstart, offs_t addrend)
{
	offs_t bytestart = address_to_byte(addrstart);
	offs_t byteend = address_to_byte_end(addrend);

	VPRINTF(("address_space::find_backing_memory('%s',%s,%08X-%08X) -> ", m_device.tag(), m_name, bytestart, byteend));

	if (m_map == NULL)
		return NULL;

	// look in the address map first
	for (address_map_entry *entry = m_map->m_entrylist.first(); entry != NULL; entry = entry->next())
	{
		offs_t maskstart = bytestart & entry->m_bytemask;
		offs_t maskend = byteend & entry->m_bytemask;
		if (entry->m_memory != NULL && maskstart >= entry->m_bytestart && maskend <= entry->m_byteend)
		{
			VPRINTF(("found in entry %08X-%08X [%p]\n", entry->m_addrstart, entry->m_addrend, (UINT8 *)entry->m_memory + (maskstart - entry->m_bytestart)));
			return (UINT8 *)entry->m_memory + (maskstart - entry->m_bytestart);
		}
	}

	// if not found there, look in the allocated blocks
	for (memory_block *block = m_machine.memory_data->blocklist.first(); block != NULL; block = block->next())
		if (block->contains(*this, bytestart, byteend))
		{
			VPRINTF(("found in allocated memory block %08X-%08X [%p]\n", block->bytestart(), block->byteend(), block->data() + (bytestart - block->bytestart())));
			return block->data() + bytestart - block->bytestart();
		}

	VPRINTF(("did not find\n"));
	return NULL;
}


//-------------------------------------------------
//  space_needs_backing_store - return whether a
//  given memory map entry implies the need of
//  allocating and registering memory
//-------------------------------------------------

bool address_space::needs_backing_store(const address_map_entry *entry)
{
	// if we are asked to provide a base pointer, then yes, we do need backing
	if (entry->m_baseptr != NULL || entry->m_baseptroffs_plus1 != 0 || entry->m_genbaseptroffs_plus1 != 0)
		return true;

	// if we're writing to any sort of bank or RAM, then yes, we do need backing
	if (entry->m_write.m_type == AMH_BANK || entry->m_write.m_type == AMH_RAM)
		return true;

	// if we're reading from RAM or from ROM outside of address space 0 or its region, then yes, we do need backing
	const region_info *region = m_machine.region(m_device.tag());
	if (entry->m_read.m_type == AMH_RAM ||
		(entry->m_read.m_type == AMH_ROM && (m_spacenum != ADDRESS_SPACE_0 || region == NULL || entry->m_addrstart >= region->bytes())))
		return true;

	// all other cases don't need backing
	return false;
}



//**************************************************************************
//  BANKING HELPERS
//**************************************************************************

//-------------------------------------------------
//  bank_find_or_allocate - allocate a new
//  bank, or find an existing one, and return the
//  read/write handler
//-------------------------------------------------

memory_bank &address_space::bank_find_or_allocate(const char *tag, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite)
{
	memory_private *memdata = m_machine.memory_data;

	// adjust the addresses, handling mirrors and such
	offs_t bytemirror = addrmirror;
	offs_t bytestart = addrstart;
	offs_t bytemask = addrmask;
	offs_t byteend = addrend;
	adjust_addresses(bytestart, byteend, bytemask, bytemirror);

	// if this bank is named, look it up
	memory_bank *bank = NULL;
	if (tag != NULL)
		bank = memdata->bankmap.find_hash_only(tag);

	// else try to find an exact match
	else
		for (bank = memdata->banklist.first(); bank != NULL; bank = bank->next())
			if (bank->anonymous() && bank->references_space(*this, ROW_READWRITE) && bank->matches_exactly(bytestart, byteend))
				break;

	// if we don't have a bank yet, find a free one
	if (bank == NULL)
	{
		// handle failure
		int banknum = memdata->banknext++;
		if (banknum > STATIC_BANKMAX)
		{
			if (tag != NULL)
				throw emu_fatalerror("Unable to allocate new bank '%s'", tag);
			else
				throw emu_fatalerror("Unable to allocate bank for RAM/ROM area %X-%X\n", bytestart, byteend);
		}

		// allocate the bank
		bank = auto_alloc(&m_machine, memory_bank(*this, banknum, bytestart, byteend, tag));
		memdata->banklist.append(*bank);

		// for named banks, add to the map and register for save states
		if (tag != NULL)
			memdata->bankmap.add_unique_hash(tag, bank, false);
	}

	// add a reference for this space
	bank->add_reference(*this, readorwrite);
	return *bank;
}



//**************************************************************************
//  TABLE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  address_table - constructor
//-------------------------------------------------

address_table::address_table(address_space &space, bool large)
	: m_table(auto_alloc_array(&space.m_machine, UINT8, 1 << LEVEL1_BITS)),
	  m_live_lookup(m_table),
	  m_space(space),
	  m_large(large),
	  m_subtable(auto_alloc_array(&space.m_machine, subtable_data, SUBTABLE_COUNT)),
	  m_subtable_alloc(0)
{
	// make our static table all watchpoints
	if (s_watchpoint_table[0] != STATIC_WATCHPOINT)
		memset(s_watchpoint_table, STATIC_WATCHPOINT, sizeof(s_watchpoint_table));

	// initialize everything to unmapped
	memset(m_table, STATIC_UNMAP, 1 << LEVEL1_BITS);
}


//-------------------------------------------------
//  ~address_table - destructor
//-------------------------------------------------

address_table::~address_table()
{
	auto_free(&m_space.m_machine, m_table);
	auto_free(&m_space.m_machine, m_subtable);
}


//-------------------------------------------------
//  map_range - finds an approprite handler entry
//  and requests to populate the address map with
//  it
//-------------------------------------------------

UINT8 address_table::map_range(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, UINT8 staticentry)
{
	// convert addresses to bytes
	offs_t bytestart = addrstart;
	offs_t byteend = addrend;
	offs_t bytemask = addrmask;
	offs_t bytemirror = addrmirror;
	m_space.adjust_addresses(bytestart, byteend, bytemask, bytemirror);

	// validity checks
	assert_always(addrstart <= addrend, "address_table::map_range called with start greater than end");
	assert_always((bytestart & (m_space.data_width() / 8 - 1)) == 0, "address_table::map_range called with misaligned start address");
	assert_always((byteend & (m_space.data_width() / 8 - 1)) == (m_space.data_width() / 8 - 1), "address_table::map_range called with misaligned end address");

	// if we weren't given an explicit entry, find a free one
	UINT8 entry = staticentry;
	if (entry == STATIC_INVALID)
	{
		// two attempts to find an empty
		for (int attempt = 0; attempt < 2; attempt++)
		{
			// scan all possible assigned entries for something unpopulated, or for an exact match
			for (UINT8 scanentry = STATIC_COUNT; scanentry < SUBTABLE_BASE; scanentry++)
			{
				handler_entry &curentry = handler(scanentry);

				// exact match takes precedence
				if (curentry.matches_exactly(bytestart, byteend, bytemask))
				{
					entry = scanentry;
					break;
				}

				// unpopulated is our second choice
				if (entry == STATIC_INVALID && !curentry.populated())
					entry = scanentry;
			}

			// if we didn't find anything, find something to depopulate
			if (entry != STATIC_INVALID)
				break;
			depopulate_unused();
		}

		// if we utterly failed, it's fatal
		if (entry == STATIC_INVALID)
			throw emu_fatalerror("Out of handler entries in address table");
	}

	// configure the entry to our parameters
	handler_entry &curentry = handler(entry);
	curentry.configure(bytestart, byteend, bytemask);

	// populate it
	populate_range_mirrored(bytestart, byteend, bytemirror, entry);

	// recompute any direct access on this space if it is a read modification
	m_space.m_direct.force_update(entry);
	return entry;
}


//-------------------------------------------------
//  populate_range - assign a memory handler to a
//  range of addresses
//-------------------------------------------------

void address_table::populate_range(offs_t bytestart, offs_t byteend, UINT8 handlerindex)
{
	offs_t l2mask = (1 << level2_bits()) - 1;
	offs_t l1start = bytestart >> level2_bits();
	offs_t l2start = bytestart & l2mask;
	offs_t l1stop = byteend >> level2_bits();
	offs_t l2stop = byteend & l2mask;

	// sanity check
	if (bytestart > byteend)
		return;

	// handle the starting edge if it's not on a block boundary
	if (l2start != 0)
	{
		UINT8 *subtable = subtable_open(l1start);

		// if the start and stop end within the same block, handle that
		if (l1start == l1stop)
		{
			memset(&subtable[l2start], handlerindex, l2stop - l2start + 1);
			subtable_close(l1start);
			return;
		}

		// otherwise, fill until the end
		memset(&subtable[l2start], handlerindex, (1 << level2_bits()) - l2start);
		subtable_close(l1start);
		if (l1start != (offs_t)~0)
			l1start++;
	}

	// handle the trailing edge if it's not on a block boundary
	if (l2stop != l2mask)
	{
		UINT8 *subtable = subtable_open(l1stop);

		// fill from the beginning
		memset(&subtable[0], handlerindex, l2stop + 1);
		subtable_close(l1stop);

		// if the start and stop end within the same block, handle that
		if (l1start == l1stop)
			return;
		if (l1stop != 0)
			l1stop--;
	}

	// now fill in the middle tables
	for (offs_t l1index = l1start; l1index <= l1stop; l1index++)
	{
		// if we have a subtable here, release it
		if (m_table[l1index] >= SUBTABLE_BASE)
			subtable_release(m_table[l1index]);
		m_table[l1index] = handlerindex;
	}
}


//-------------------------------------------------
//  populate_range_mirrored - assign a memory
//  handler to a range of addresses including
//  mirrors
//-------------------------------------------------

void address_table::populate_range_mirrored(offs_t bytestart, offs_t byteend, offs_t bytemirror, UINT8 handlerindex)
{
	// determine the mirror bits
	offs_t lmirrorbits = 0;
	offs_t lmirrorbit[32];
	for (int bit = 0; bit < level2_bits(); bit++)
		if (bytemirror & (1 << bit))
			lmirrorbit[lmirrorbits++] = 1 << bit;

	offs_t hmirrorbits = 0;
	offs_t hmirrorbit[32];
	for (int bit = level2_bits(); bit < 32; bit++)
		if (bytemirror & (1 << bit))
			hmirrorbit[hmirrorbits++] = 1 << bit;

	// loop over mirrors in the level 2 table
	UINT8 prev_entry = STATIC_INVALID;
	int prev_index = 0;
	for (offs_t hmirrorcount = 0; hmirrorcount < (1 << hmirrorbits); hmirrorcount++)
	{
		// compute the base of this mirror
		offs_t hmirrorbase = 0;
		for (int bit = 0; bit < hmirrorbits; bit++)
			if (hmirrorcount & (1 << bit))
				hmirrorbase |= hmirrorbit[bit];

		// invalidate any intersecting cached ranges
		for (offs_t lmirrorcount = 0; lmirrorcount < (1 << lmirrorbits); lmirrorcount++)
		{
			// compute the base of this mirror
			offs_t lmirrorbase = hmirrorbase;
			for (int bit = 0; bit < lmirrorbits; bit++)
				if (lmirrorcount & (1 << bit))
					lmirrorbase |= lmirrorbit[bit];
			m_space.m_direct.remove_intersecting_ranges(bytestart + lmirrorbase, byteend + lmirrorbase);
		}

		// if this is not our first time through, and the level 2 entry matches the previous
		// level 2 entry, just do a quick map and get out; note that this only works for entries
		// which don't span multiple level 1 table entries
		int cur_index = level1_index(bytestart + hmirrorbase);
		if (cur_index == level1_index(byteend + hmirrorbase))
		{
			if (hmirrorcount != 0 && prev_entry == m_table[cur_index])
			{
				VPRINTF(("Quick mapping subtable at %08X to match subtable at %08X\n", cur_index << level2_bits(), prev_index << level2_bits()));

				// release the subtable if the old value was a subtable
				if (m_table[cur_index] >= SUBTABLE_BASE)
					subtable_release(m_table[cur_index]);

				// reallocate the subtable if the new value is a subtable
				if (m_table[prev_index] >= SUBTABLE_BASE)
					subtable_realloc(m_table[prev_index]);

				// set the new value and short-circuit the mapping step
				m_table[cur_index] = m_table[prev_index];
				continue;
			}
			prev_index = cur_index;
			prev_entry = m_table[cur_index];
		}

		// loop over mirrors in the level 1 table
		for (offs_t lmirrorcount = 0; lmirrorcount < (1 << lmirrorbits); lmirrorcount++)
		{
			// compute the base of this mirror
			offs_t lmirrorbase = hmirrorbase;
			for (int bit = 0; bit < lmirrorbits; bit++)
				if (lmirrorcount & (1 << bit))
					lmirrorbase |= lmirrorbit[bit];

			// populate the tables
			populate_range(bytestart + lmirrorbase, byteend + lmirrorbase, handlerindex);
		}
	}
}


//-------------------------------------------------
//  depopulate_unused - scan the table and
//	eliminate entries that are no longer used
//-------------------------------------------------

void address_table::depopulate_unused()
{
	assert(false);
}


//-------------------------------------------------
//  derive_range - look up the entry for a memory
//  range, and then compute the extent of that
//  range based on the lookup tables
//-------------------------------------------------

UINT8 address_table::derive_range(offs_t byteaddress, offs_t &bytestart, offs_t &byteend) const
{
	// look up the initial address to get the entry we care about
	UINT8 l1entry;
	UINT8 entry = l1entry = m_table[level1_index(byteaddress)];
	if (l1entry >= SUBTABLE_BASE)
		entry = m_table[level2_index(l1entry, byteaddress)];

	// use the bytemask of the entry to set minimum and maximum bounds
	offs_t minscan, maxscan;
	handler(entry).mirrored_start_end(byteaddress, minscan, maxscan);

	// first scan backwards to find the start address
	UINT8 curl1entry = l1entry;
	UINT8 curentry = entry;
	bytestart = byteaddress;
	while (1)
	{
		// if we need to scan the subtable, do it
		if (curentry != curl1entry)
		{
			UINT32 minindex = level2_index(curl1entry, 0);
			UINT32 index;

			// scan backwards from the current address, until the previous entry doesn't match
			for (index = level2_index(curl1entry, bytestart); index > minindex; index--, bytestart -= 1)
				if (m_table[index - 1] != entry)
					break;

			// if we didn't hit the beginning, then we're finished scanning
			if (index != minindex)
				break;
		}

		// move to the beginning of this L1 entry; stop at the minimum address
		bytestart &= ~((1 << level2_bits()) - 1);
		if (bytestart <= minscan)
			break;

		// look up the entry of the byte at the end of the previous L1 entry; if it doesn't match, stop
		curentry = curl1entry = m_table[level1_index(bytestart - 1)];
		if (curl1entry >= SUBTABLE_BASE)
			curentry = m_table[level2_index(curl1entry, bytestart - 1)];
		if (curentry != entry)
			break;

		// move into the previous entry and resume searching
		bytestart -= 1;
	}

	// then scan forwards to find the end address
	curl1entry = l1entry;
	curentry = entry;
	byteend = byteaddress;
	while (1)
	{
		// if we need to scan the subtable, do it
		if (curentry != curl1entry)
		{
			UINT32 maxindex = level2_index(curl1entry, ~0);
			UINT32 index;

			// scan forwards from the current address, until the next entry doesn't match
			for (index = level2_index(curl1entry, byteend); index < maxindex; index++, byteend += 1)
				if (m_table[index + 1] != entry)
					break;

			// if we didn't hit the end, then we're finished scanning
			if (index != maxindex)
				break;
		}

		// move to the end of this L1 entry; stop at the maximum address
		byteend |= (1 << level2_bits()) - 1;
		if (byteend >= maxscan)
			break;

		// look up the entry of the byte at the start of the next L1 entry; if it doesn't match, stop
		curentry = curl1entry = m_table[level1_index(byteend + 1)];
		if (curl1entry >= SUBTABLE_BASE)
			curentry = m_table[level2_index(curl1entry, byteend + 1)];
		if (curentry != entry)
			break;

		// move into the next entry and resume searching
		byteend += 1;
	}

	return entry;
}


//-------------------------------------------------
//  mask_all_handlers - apply a mask to all
//  address handlers
//-------------------------------------------------

void address_table::mask_all_handlers(offs_t mask)
{
	// we don't loop over map entries because the mask applies to static handlers as well
	for (int entrynum = 0; entrynum < ENTRY_COUNT; entrynum++)
		handler(entrynum).apply_mask(mask);
}



//**************************************************************************
//  SUBTABLE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  subtable_alloc - allocate a fresh subtable
//  and set its usecount to 1
//-------------------------------------------------

UINT8 address_table::subtable_alloc()
{
	// loop
	while (1)
	{
		// find a subtable with a usecount of 0
		for (UINT8 subindex = 0; subindex < SUBTABLE_COUNT; subindex++)
			if (m_subtable[subindex].m_usecount == 0)
			{
				// if this is past our allocation budget, allocate some more
				if (subindex >= m_subtable_alloc)
				{
					UINT32 oldsize = (1 << LEVEL1_BITS) + (m_subtable_alloc << level2_bits());
					m_subtable_alloc += SUBTABLE_ALLOC;
					UINT32 newsize = (1 << LEVEL1_BITS) + (m_subtable_alloc << level2_bits());

					UINT8 *newtable = auto_alloc_array_clear(&m_space.m_machine, UINT8, newsize);
					memcpy(newtable, m_table, oldsize);
					if (m_live_lookup == m_table)
						m_live_lookup = newtable;
					auto_free(&m_space.m_machine, m_table);
					m_table = newtable;
				}

				// bump the usecount and return
				m_subtable[subindex].m_usecount++;
				return subindex + SUBTABLE_BASE;
			}

		// merge any subtables we can
		if (!subtable_merge())
			fatalerror("Ran out of subtables!");
	}
}


//-------------------------------------------------
//  subtable_realloc - increment the usecount on
//  a subtable
//-------------------------------------------------

void address_table::subtable_realloc(UINT8 subentry)
{
	UINT8 subindex = subentry - SUBTABLE_BASE;

	// sanity check
	if (m_subtable[subindex].m_usecount <= 0)
		fatalerror("Called subtable_realloc on a table with a usecount of 0");

	// increment the usecount
	m_subtable[subindex].m_usecount++;
}


//-------------------------------------------------
//  subtable_merge - merge any duplicate
//  subtables
//-------------------------------------------------

int address_table::subtable_merge()
{
	int merged = 0;
	UINT8 subindex;

	VPRINTF(("Merging subtables....\n"));

	// okay, we failed; update all the checksums and merge tables
	for (subindex = 0; subindex < SUBTABLE_COUNT; subindex++)
		if (!m_subtable[subindex].m_checksum_valid && m_subtable[subindex].m_usecount != 0)
		{
			UINT32 *subtable = reinterpret_cast<UINT32 *>(subtable_ptr(subindex + SUBTABLE_BASE));
			UINT32 checksum = 0;

			// update the checksum
			for (int l2index = 0; l2index < (1 << level2_bits())/4; l2index++)
				checksum += subtable[l2index];
			m_subtable[subindex].m_checksum = checksum;
			m_subtable[subindex].m_checksum_valid = true;
		}

	// see if there's a matching checksum
	for (subindex = 0; subindex < SUBTABLE_COUNT; subindex++)
		if (m_subtable[subindex].m_usecount != 0)
		{
			UINT8 *subtable = subtable_ptr(subindex + SUBTABLE_BASE);
			UINT32 checksum = m_subtable[subindex].m_checksum;
			UINT8 sumindex;

			for (sumindex = subindex + 1; sumindex < SUBTABLE_COUNT; sumindex++)
				if (m_subtable[sumindex].m_usecount != 0 &&
					m_subtable[sumindex].m_checksum == checksum &&
					!memcmp(subtable, subtable_ptr(sumindex + SUBTABLE_BASE), 1 << level2_bits()))
				{
					int l1index;

					VPRINTF(("Merging subtable %d and %d....\n", subindex, sumindex));

					// find all the entries in the L1 tables that pointed to the old one, and point them to the merged table
					for (l1index = 0; l1index <= (0xffffffffUL >> level2_bits()); l1index++)
						if (m_table[l1index] == sumindex + SUBTABLE_BASE)
						{
							subtable_release(sumindex + SUBTABLE_BASE);
							subtable_realloc(subindex + SUBTABLE_BASE);
							m_table[l1index] = subindex + SUBTABLE_BASE;
							merged++;
						}
				}
		}

	return merged;
}


//-------------------------------------------------
//  subtable_release - decrement the usecount on
//  a subtable and free it if we're done
//-------------------------------------------------

void address_table::subtable_release(UINT8 subentry)
{
	UINT8 subindex = subentry - SUBTABLE_BASE;

	// sanity check
	if (m_subtable[subindex].m_usecount <= 0)
		fatalerror("Called subtable_release on a table with a usecount of 0");

	// decrement the usecount and clear the checksum if we're at 0
	m_subtable[subindex].m_usecount--;
	if (m_subtable[subindex].m_usecount == 0)
		m_subtable[subindex].m_checksum = 0;
}


//-------------------------------------------------
//  subtable_open - gain access to a subtable for
//  modification
//-------------------------------------------------

UINT8 *address_table::subtable_open(offs_t l1index)
{
	UINT8 subentry = m_table[l1index];

	// if we don't have a subtable yet, allocate a new one
	if (subentry < SUBTABLE_BASE)
	{
		UINT8 newentry = subtable_alloc();
		memset(subtable_ptr(newentry), subentry, 1 << level2_bits());
		m_table[l1index] = newentry;
		m_subtable[newentry - SUBTABLE_BASE].m_checksum = (subentry + (subentry << 8) + (subentry << 16) + (subentry << 24)) * ((1 << level2_bits())/4);
		subentry = newentry;
	}

	// if we're sharing this subtable, we also need to allocate a fresh copy
	else if (m_subtable[subentry - SUBTABLE_BASE].m_usecount > 1)
	{
		UINT8 newentry = subtable_alloc();

		// allocate may cause some additional merging -- look up the subentry again
		// when we're done; it should still require a split
		subentry = m_table[l1index];
		assert(subentry >= SUBTABLE_BASE);
		assert(m_subtable[subentry - SUBTABLE_BASE].m_usecount > 1);

		memcpy(subtable_ptr(newentry), subtable_ptr(subentry), 1 << level2_bits());
		subtable_release(subentry);
		m_table[l1index] = newentry;
		m_subtable[newentry - SUBTABLE_BASE].m_checksum = m_subtable[subentry - SUBTABLE_BASE].m_checksum;
		subentry = newentry;
	}

	// mark the table dirty
	m_subtable[subentry - SUBTABLE_BASE].m_checksum_valid = false;

	// return the pointer to the subtable
	return subtable_ptr(subentry);
}


//-------------------------------------------------
//  subtable_close - stop access to a subtable
//-------------------------------------------------

void address_table::subtable_close(offs_t l1index)
{
	// defer any merging until we run out of tables
}


//-------------------------------------------------
//  handler_name - return friendly string
//  description of a handler
//-------------------------------------------------

const char *address_table::handler_name(UINT8 entry) const
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
		"bank 96",		"bank 97",		"bank 98",		"bank 99",
		"bank 100",		"bank 101",		"bank 102",		"bank 103",
		"bank 104",		"bank 105",		"bank 106",		"bank 107",
		"bank 108",		"bank 109",		"bank 110",		"bank 111",
		"bank 112",		"bank 113",		"bank 114",		"bank 115",
		"bank 116",		"bank 117",		"bank 118",		"bank 119",
		"bank 120",		"bank 121",		"bank 122",		"ram",
		"rom",			"nop",			"unmapped",     "watchpoint"
	};

	// banks have names
	if (entry >= STATIC_BANK1 && entry <= STATIC_BANKMAX)
		for (memory_bank *info = m_space.m_machine.memory_data->banklist.first(); info != NULL; info = info->next())
			if (info->index() == entry)
				return info->name();

	// constant strings for lower entries
	if (entry < ARRAY_LENGTH(strings))
		return strings[entry];
	else if (handler(entry).name() != NULL)
		return handler(entry).name();
	else
		return "???";
}


//-------------------------------------------------
//  address_table_read - constructor
//-------------------------------------------------

address_table_read::address_table_read(address_space &space, bool large)
	: address_table(space, large)
{
	// allocate handlers for each entry, prepopulating the bankptrs for banks
	for (int entrynum = 0; entrynum < ARRAY_LENGTH(m_handlers); entrynum++)
	{
		UINT8 **bankptr = (entrynum >= STATIC_BANK1 && entrynum <= STATIC_BANKMAX) ? &space.m_machine.memory_data->bank_ptr[entrynum] : NULL;
		m_handlers[entrynum] = auto_alloc(&space.m_machine, handler_entry_read(space.data_width(), space.endianness(), bankptr));
	}

	// we have to allocate different object types based on the data bus width
	switch (space.data_width())
	{
		// 8-bit case
		case 8:
			m_handlers[STATIC_UNMAP]->set_delegate(read8_delegate_create(address_table_read, unmap_r<UINT8>, *this));
			m_handlers[STATIC_NOP]->set_delegate(read8_delegate_create(address_table_read, nop_r<UINT8>, *this));
			m_handlers[STATIC_WATCHPOINT]->set_delegate(read8_delegate_create(address_table_read, watchpoint_r<UINT8>, *this));
			break;

		// 16-bit case
		case 16:
			m_handlers[STATIC_UNMAP]->set_delegate(read16_delegate_create(address_table_read, unmap_r<UINT16>, *this));
			m_handlers[STATIC_NOP]->set_delegate(read16_delegate_create(address_table_read, nop_r<UINT16>, *this));
			m_handlers[STATIC_WATCHPOINT]->set_delegate(read16_delegate_create(address_table_read, watchpoint_r<UINT16>, *this));
			break;

		// 32-bit case
		case 32:
			m_handlers[STATIC_UNMAP]->set_delegate(read32_delegate_create(address_table_read, unmap_r<UINT32>, *this));
			m_handlers[STATIC_NOP]->set_delegate(read32_delegate_create(address_table_read, nop_r<UINT32>, *this));
			m_handlers[STATIC_WATCHPOINT]->set_delegate(read32_delegate_create(address_table_read, watchpoint_r<UINT32>, *this));
			break;

		// 64-bit case
		case 64:
			m_handlers[STATIC_UNMAP]->set_delegate(read64_delegate_create(address_table_read, unmap_r<UINT64>, *this));
			m_handlers[STATIC_NOP]->set_delegate(read64_delegate_create(address_table_read, nop_r<UINT64>, *this));
			m_handlers[STATIC_WATCHPOINT]->set_delegate(read64_delegate_create(address_table_read, watchpoint_r<UINT64>, *this));
			break;
	}

	// reset the byte masks on the special handlers to open up the full address space for proper reporting
	m_handlers[STATIC_UNMAP]->configure(0, space.bytemask(), ~0);
	m_handlers[STATIC_NOP]->configure(0, space.bytemask(), ~0);
	m_handlers[STATIC_WATCHPOINT]->configure(0, space.bytemask(), ~0);
}


//-------------------------------------------------
//  address_table_read - destructor
//-------------------------------------------------

address_table_read::~address_table_read()
{
	for (int handnum = 0; handnum < ARRAY_LENGTH(m_handlers); handnum++)
		auto_free(&m_space.m_machine, m_handlers[handnum]);
}


//-------------------------------------------------
//  handler - return the generic handler entry for
//  this index
//-------------------------------------------------

handler_entry &address_table_read::handler(UINT32 index) const
{
	assert(index < ARRAY_LENGTH(m_handlers));
	return *m_handlers[index];
}


//-------------------------------------------------
//  address_table_write - constructor
//-------------------------------------------------

address_table_write::address_table_write(address_space &space, bool large)
	: address_table(space, large)
{
	// allocate handlers for each entry, prepopulating the bankptrs for banks
	for (int entrynum = 0; entrynum < ARRAY_LENGTH(m_handlers); entrynum++)
	{
		UINT8 **bankptr = (entrynum >= STATIC_BANK1 && entrynum <= STATIC_BANKMAX) ? &space.m_machine.memory_data->bank_ptr[entrynum] : NULL;
		m_handlers[entrynum] = auto_alloc(&space.m_machine, handler_entry_write(space.data_width(), space.endianness(), bankptr));
	}

	// we have to allocate different object types based on the data bus width
	switch (space.data_width())
	{
		// 8-bit case
		case 8:
			m_handlers[STATIC_UNMAP]->set_delegate(write8_delegate_create(address_table_write, unmap_w<UINT8>, *this));
			m_handlers[STATIC_NOP]->set_delegate(write8_delegate_create(address_table_write, nop_w<UINT8>, *this));
			m_handlers[STATIC_WATCHPOINT]->set_delegate(write8_delegate_create(address_table_write, watchpoint_w<UINT8>, *this));
			break;

		// 16-bit case
		case 16:
			m_handlers[STATIC_UNMAP]->set_delegate(write16_delegate_create(address_table_write, unmap_w<UINT16>, *this));
			m_handlers[STATIC_NOP]->set_delegate(write16_delegate_create(address_table_write, nop_w<UINT16>, *this));
			m_handlers[STATIC_WATCHPOINT]->set_delegate(write16_delegate_create(address_table_write, watchpoint_w<UINT16>, *this));
			break;

		// 32-bit case
		case 32:
			m_handlers[STATIC_UNMAP]->set_delegate(write32_delegate_create(address_table_write, unmap_w<UINT32>, *this));
			m_handlers[STATIC_NOP]->set_delegate(write32_delegate_create(address_table_write, nop_w<UINT32>, *this));
			m_handlers[STATIC_WATCHPOINT]->set_delegate(write32_delegate_create(address_table_write, watchpoint_w<UINT32>, *this));
			break;

		// 64-bit case
		case 64:
			m_handlers[STATIC_UNMAP]->set_delegate(write64_delegate_create(address_table_write, unmap_w<UINT64>, *this));
			m_handlers[STATIC_NOP]->set_delegate(write64_delegate_create(address_table_write, nop_w<UINT64>, *this));
			m_handlers[STATIC_WATCHPOINT]->set_delegate(write64_delegate_create(address_table_write, watchpoint_w<UINT64>, *this));
			break;
	}

	// reset the byte masks on the special handlers to open up the full address space for proper reporting
	m_handlers[STATIC_UNMAP]->configure(0, space.bytemask(), ~0);
	m_handlers[STATIC_NOP]->configure(0, space.bytemask(), ~0);
	m_handlers[STATIC_WATCHPOINT]->configure(0, space.bytemask(), ~0);
}


//-------------------------------------------------
//  address_table_write - destructor
//-------------------------------------------------

address_table_write::~address_table_write()
{
	for (int handnum = 0; handnum < ARRAY_LENGTH(m_handlers); handnum++)
		auto_free(&m_space.m_machine, m_handlers[handnum]);
}


//-------------------------------------------------
//  handler - return the generic handler entry for
//  this index
//-------------------------------------------------

handler_entry &address_table_write::handler(UINT32 index) const
{
	assert(index < ARRAY_LENGTH(m_handlers));
	return *m_handlers[index];
}



//**************************************************************************
//  DIRECT MEMORY RANGES
//**************************************************************************

//-------------------------------------------------
//  direct_read_data - constructor
//-------------------------------------------------

direct_read_data::direct_read_data(address_space &space)
	: m_space(space),
	  m_raw(NULL),
	  m_decrypted(NULL),
	  m_bytemask(space.bytemask()),
	  m_bytestart(1),
	  m_byteend(0),
	  m_entry(STATIC_UNMAP)
{
}


//-------------------------------------------------
//  ~direct_read_data - destructor
//-------------------------------------------------

direct_read_data::~direct_read_data()
{
}


//-------------------------------------------------
//  set_direct_region - called by device cores to
//  update the opcode base for the given address
//-------------------------------------------------

bool direct_read_data::set_direct_region(offs_t &byteaddress)
{
	// allow overrides
	offs_t overrideaddress = byteaddress;
	if (!m_directupdate.isnull())
	{
		overrideaddress = m_directupdate(*this, overrideaddress);
		if (overrideaddress == ~0)
			return true;

		byteaddress = overrideaddress;
	}

	// remove the masked bits (we'll put them back later)
	offs_t maskedbits = overrideaddress & ~m_bytemask;

	// find or allocate a matching range
	direct_range *range = find_range(overrideaddress, m_entry);

	// if we don't map to a bank, return FALSE
	if (m_entry < STATIC_BANK1 || m_entry >= STATIC_RAM)
	{
		// ensure future updates to land here as well until we get back into a bank
		m_byteend = 0;
		m_bytestart = 1;
		return false;
	}

	// if no decrypted opcodes, point to the same base
	UINT8 *base = m_space.m_machine.memory_data->bank_ptr[m_entry];
	UINT8 *based = m_space.m_machine.memory_data->bankd_ptr[m_entry];
	if (based == NULL)
		based = base;

	// compute the adjusted base
	const handler_entry_read &handler = m_space.read().handler_read(m_entry);
	m_bytemask = handler.bytemask();
	m_raw = base - (handler.bytestart() & m_bytemask);
	m_decrypted = based - (handler.bytestart() & m_bytemask);
	m_bytestart = maskedbits | range->m_bytestart;
	m_byteend = maskedbits | range->m_byteend;
	return true;
}


//-------------------------------------------------
//  find_range - find a byte address in a range
//-------------------------------------------------

direct_read_data::direct_range *direct_read_data::find_range(offs_t byteaddress, UINT8 &entry)
{
	// determine which entry
	byteaddress &= m_space.m_bytemask;
	entry = m_space.read().lookup_live(byteaddress);

	// scan our table
	for (direct_range *range = m_rangelist[entry].first(); range != NULL; range = range->next())
		if (byteaddress >= range->m_bytestart && byteaddress <= range->m_byteend)
			return range;

	// didn't find out; allocate a new one
	direct_range *range = m_freerangelist.first();
	if (range != NULL)
		m_freerangelist.detach(*range);
	else
		range = auto_alloc(&m_space.m_machine, direct_range);

	// fill in the range
	m_space.read().derive_range(byteaddress, range->m_bytestart, range->m_byteend);
	m_rangelist[entry].prepend(*range);

	return range;
}


//-------------------------------------------------
//  remove_intersecting_ranges - remove all cached
//  ranges that intersect the given address range
//-------------------------------------------------

void direct_read_data::remove_intersecting_ranges(offs_t bytestart, offs_t byteend)
{
	// loop over all entries
	for (int entry = 0; entry < ARRAY_LENGTH(m_rangelist); entry++)
	{
		// loop over all ranges in this entry's list
		direct_range *nextrange;
		for (direct_range *range = m_rangelist[entry].first(); range != NULL; range = nextrange)
		{
			nextrange = range->next();

			// if we intersect, remove and add to the free range list
			if (bytestart <= range->m_byteend && byteend >= range->m_bytestart)
			{
				m_rangelist[entry].detach(*range);
				m_freerangelist.prepend(*range);
			}
		}
	}
}


//-------------------------------------------------
//  set_direct_update - set a custom direct range
//  update callback
//-------------------------------------------------

direct_update_delegate direct_read_data::set_direct_update(direct_update_delegate function)
{
	direct_update_delegate old = m_directupdate;
	m_directupdate = function;
	return old;
}


//-------------------------------------------------
//  explicit_configure - explicitly configure
//  the start/end/mask and the pointers from
//  within a custom callback
//-------------------------------------------------

void direct_read_data::explicit_configure(offs_t bytestart, offs_t byteend, offs_t bytemask, void *raw, void *decrypted)
{
	m_bytestart = bytestart;
	m_byteend = byteend;
	m_bytemask = bytemask;
	m_raw = reinterpret_cast<UINT8 *>(raw);
	m_decrypted = reinterpret_cast<UINT8 *>((decrypted == NULL) ? raw : decrypted);
	m_raw -= bytestart & bytemask;
	m_decrypted -= bytestart & bytemask;
}



//**************************************************************************
//  MEMORY BLOCK
//**************************************************************************

//-------------------------------------------------
//  memory_block - constructor
//-------------------------------------------------

memory_block::memory_block(address_space &space, offs_t bytestart, offs_t byteend, void *memory)
	: m_next(NULL),
	  m_machine(space.m_machine),
	  m_space(space),
	  m_isallocated(memory == NULL),
	  m_bytestart(bytestart),
	  m_byteend(byteend),
	  m_data((memory != NULL) ? reinterpret_cast<UINT8 *>(memory) : auto_alloc_array_clear(&space.m_machine, UINT8, byteend + 1 - bytestart))
{
	VPRINTF(("block_allocate('%s',%s,%08X,%08X,%p)\n", space.device().tag(), space.name(), bytestart, byteend, memory));

	// register for saving, but only if we're not part of a memory region
	const region_info *region;
	for (region = space.m_machine.m_regionlist.first(); region != NULL; region = region->next())
		if (m_data >= region->base() && (m_data + (byteend - bytestart + 1)) < region->end())
		{
			VPRINTF(("skipping save of this memory block as it is covered by a memory region\n"));
			break;
		}

	// if we didn't find a match, register
	if (region == NULL)
	{
		int bytes_per_element = space.data_width() / 8;
		astring name;
		name.printf("%08x-%08x", bytestart, byteend);
		state_save_register_memory(&space.m_machine, "memory", space.device().tag(), space.spacenum(), name, m_data, bytes_per_element, (UINT32)(byteend + 1 - bytestart) / bytes_per_element, __FILE__, __LINE__);
	}
}


//-------------------------------------------------
//  memory_block - destructor
//-------------------------------------------------

memory_block::~memory_block()
{
	if (m_isallocated)
		auto_free(&m_machine, m_data);
}



//**************************************************************************
//  MEMORY BANK
//**************************************************************************

//-------------------------------------------------
//  memory_bank - constructor
//-------------------------------------------------

memory_bank::memory_bank(address_space &space, int index, offs_t bytestart, offs_t byteend, const char *tag)
	: m_next(NULL),
	  m_machine(space.m_machine),
	  m_baseptr(&space.m_machine.memory_data->bank_ptr[index]),
	  m_basedptr(&space.m_machine.memory_data->bankd_ptr[index]),
	  m_index(index),
	  m_anonymous(tag == NULL),
	  m_bytestart(bytestart),
	  m_byteend(byteend),
	  m_curentry(BANK_ENTRY_UNSPECIFIED),
	  m_entry(NULL),
	  m_entry_count(0)
{
	// generate an internal tag if we don't have one
	if (tag == NULL)
	{
		m_tag.printf("~%d~", index);
		m_name.printf("Internal bank #%d", index);
	}
	else
	{
		m_tag.cpy(tag);
		m_name.printf("Bank '%s'", tag);
	}

	if (!m_anonymous && state_save_registration_allowed(&space.m_machine))
		state_save_register_item(&space.m_machine, "memory", m_tag, 0, m_curentry);
}


//-------------------------------------------------
//  memory_bank - destructor
//-------------------------------------------------

memory_bank::~memory_bank()
{
	auto_free(&m_machine, m_entry);
}


//-------------------------------------------------
//  references_space - walk the list of references
//  to find a match against the provided space
//  and read/write
//-------------------------------------------------

bool memory_bank::references_space(address_space &space, read_or_write readorwrite) const
{
	for (bank_reference *ref = m_reflist.first(); ref != NULL; ref = ref->next())
		if (ref->matches(space, readorwrite))
			return true;
	return false;
}


//-------------------------------------------------
//  add_reference - add a new reference to the
//  given space
//-------------------------------------------------

void memory_bank::add_reference(address_space &space, read_or_write readorwrite)
{
	// if we already have a reference, skip it
	if (references_space(space, readorwrite))
		return;
	m_reflist.append(*auto_alloc(&space.m_machine, bank_reference(space, readorwrite)));
}


//-------------------------------------------------
//  invalidate_references - force updates on all
//  referencing address spaces
//-------------------------------------------------

void memory_bank::invalidate_references()
{
	// invalidate all the direct references to any referenced address spaces
	for (bank_reference *ref = m_reflist.first(); ref != NULL; ref = ref->next())
		ref->space().direct().force_update();
}


//-------------------------------------------------
//  set_base - set the bank base explicitly
//-------------------------------------------------

void memory_bank::set_base(void *base)
{
	// NULL is not an option
	if (base == NULL)
		throw emu_fatalerror("memory_bank::set_base called NULL base");

	// set the base and invalidate any referencing spaces
	*m_baseptr = reinterpret_cast<UINT8 *>(base);
	invalidate_references();
}


//-------------------------------------------------
//  set_base_decrypted - set the decrypted base
//  explicitly
//-------------------------------------------------

void memory_bank::set_base_decrypted(void *base)
{
	// NULL is not an option
	if (base == NULL)
		throw emu_fatalerror("memory_bank::set_base called NULL base");

	// set the base and invalidate any referencing spaces
	*m_basedptr = reinterpret_cast<UINT8 *>(base);
	invalidate_references();
}


//-------------------------------------------------
//  set_entry - set the base to a pre-configured
//  entry
//-------------------------------------------------

void memory_bank::set_entry(int entrynum)
{
	// validate
	if (m_anonymous)
		throw emu_fatalerror("memory_bank::set_entry called for anonymous bank");
	if (entrynum < 0 || entrynum >= m_entry_count)
		throw emu_fatalerror("memory_bank::set_entry called with out-of-range entry %d", entrynum);
	if (m_entry[entrynum].m_raw == NULL)
		throw emu_fatalerror("memory_bank::set_entry called for bank '%s' with invalid bank entry %d", m_tag.cstr(), entrynum);

	// set both raw and decrypted values
	m_curentry = entrynum;
	*m_baseptr = m_entry[entrynum].m_raw;
	*m_basedptr = m_entry[entrynum].m_decrypted;

	// invalidate referencing spaces
	invalidate_references();
}


//-------------------------------------------------
//  expand_entries - expand the allocated array
//  of entries
//-------------------------------------------------

void memory_bank::expand_entries(int entrynum)
{
	int newcount = entrynum + 1;

	// allocate a new array and copy from the old one; zero out the new entries
	bank_entry *newentry = auto_alloc_array(&m_machine, bank_entry, newcount);
	memcpy(newentry, m_entry, sizeof(m_entry[0]) * m_entry_count);
	memset(&newentry[m_entry_count], 0, (newcount - m_entry_count) * sizeof(m_entry[0]));

	// free the old array and set the updated values
	auto_free(&m_machine, m_entry);
	m_entry = newentry;
	m_entry_count = newcount;
}


//-------------------------------------------------
//  configure - configure an entry
//-------------------------------------------------

void memory_bank::configure(int entrynum, void *base)
{
	// must be positive
	if (entrynum < 0)
		throw emu_fatalerror("memory_bank::configure called with out-of-range entry %d", entrynum);

	// if we haven't allocated this many entries yet, expand our array
	if (entrynum >= m_entry_count)
		expand_entries(entrynum);

	// set the entry
	m_entry[entrynum].m_raw = reinterpret_cast<UINT8 *>(base);

	// if the bank base is not configured, and we're the first entry, set us up
	if (*m_baseptr == NULL && entrynum == 0)
		*m_baseptr = m_entry[entrynum].m_raw;
}


//-------------------------------------------------
//  configure_decrypted - configure a decrypted
//  entry
//-------------------------------------------------

void memory_bank::configure_decrypted(int entrynum, void *base)
{
	// must be positive
	if (entrynum < 0)
		throw emu_fatalerror("memory_bank::configure called with out-of-range entry %d", entrynum);

	// if we haven't allocated this many entries yet, expand our array
	if (entrynum >= m_entry_count)
		expand_entries(entrynum);

	// set the entry
	m_entry[entrynum].m_decrypted = reinterpret_cast<UINT8 *>(base);

	// if the bank base is not configured, and we're the first entry, set us up
	if (*m_basedptr == NULL && entrynum == 0)
		*m_basedptr = m_entry[entrynum].m_decrypted;
}



//**************************************************************************
//  HANDLER ENTRY
//**************************************************************************

//-------------------------------------------------
//  handler_entry - constructor
//-------------------------------------------------

handler_entry::handler_entry(UINT8 width, endianness_t endianness, UINT8 **rambaseptr)
	: m_populated(false),
	  m_datawidth(width),
	  m_endianness(endianness),
	  m_bytestart(0),
	  m_byteend(0),
	  m_bytemask(~0),
	  m_rambaseptr(rambaseptr),
	  m_subunits(0)
{
}


//-------------------------------------------------
//  ~handler_entry - destructor
//-------------------------------------------------

handler_entry::~handler_entry()
{
}


//-------------------------------------------------
//  configure_subunits - configure the subunits
//  and subshift array to represent the provided
//  mask
//-------------------------------------------------

void handler_entry::configure_subunits(UINT64 handlermask, int handlerbits)
{
	UINT64 unitmask = ((UINT64)1 << handlerbits) - 1;
	assert(handlermask != 0);

	// compute the maximum possible subunits
	int maxunits = m_datawidth / handlerbits;
	assert(maxunits > 1);
	assert(maxunits < ARRAY_LENGTH(m_subshift));

	// walk the handlermask to find out how many we have
	m_subunits = 0;
	for (int unitnum = 0; unitnum < maxunits; unitnum++)
	{
		UINT64 scanmask = unitmask << (unitnum * handlerbits);
		assert((handlermask & scanmask) == 0 || (handlermask & scanmask) == scanmask);
		if ((handlermask & scanmask) != 0)
			m_subunits++;
	}

	// then fill in the shifts based on the endianness
	if (m_endianness == ENDIANNESS_LITTLE)
	{
		UINT8 *unitshift = &m_subshift[0];
		for (int unitnum = 0; unitnum < maxunits; unitnum++)
			if ((handlermask & (unitmask << (unitnum * handlerbits))) != 0)
				*unitshift++ = unitnum * handlerbits;
	}
	else
	{
		UINT8 *unitshift = &m_subshift[m_subunits];
		for (int unitnum = 0; unitnum < maxunits; unitnum++)
			if ((handlermask & (unitmask << (unitnum * handlerbits))) != 0)
				*--unitshift = unitnum * handlerbits;
	}
}



//**************************************************************************
//  HANDLER ENTRY READ
//**************************************************************************

//-------------------------------------------------
//  name - return the handler name, from the
//  appropriately-sized delegate
//-------------------------------------------------

const char *handler_entry_read::name() const
{
	switch (m_datawidth)
	{
		case 8:		return m_read8.name();
		case 16:	return m_read16.name();
		case 32:	return m_read32.name();
		case 64:	return m_read64.name();
	}
	return NULL;
}


//-------------------------------------------------
//  set_delegate - set an 8-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_read::set_delegate(read8_delegate delegate, UINT64 mask)
{
	// error if no object
	if (!delegate.has_object())
		throw emu_fatalerror("Attempted to install delegate '%s' without a bound object", delegate.name());

	// make sure this is a valid size
	assert(m_datawidth >= 8);
	m_read8 = delegate;

	// if mismatched bus width, configure a stub
	if (m_datawidth != 8)
	{
		configure_subunits(mask, 8);
		if (m_datawidth == 16)
			set_delegate(read16_delegate(read16_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_16_from_8>(delegate.name()), *this));
		else if (m_datawidth == 32)
			set_delegate(read32_delegate(read32_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_32_from_8>(delegate.name()), *this));
		else if (m_datawidth == 64)
			set_delegate(read64_delegate(read64_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_64_from_8>(delegate.name()), *this));
	}
}


//-------------------------------------------------
//  set_delegate - set a 16-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_read::set_delegate(read16_delegate delegate, UINT64 mask)
{
	// error if no object
	if (!delegate.has_object())
		throw emu_fatalerror("Attempted to install delegate '%s' without a bound object", delegate.name());

	// make sure this is a valid size
	assert(m_datawidth >= 16);
	m_read16 = delegate;

	// if mismatched bus width, configure a stub
	if (m_datawidth != 16)
	{
		configure_subunits(mask, 16);
		if (m_datawidth == 32)
			set_delegate(read32_delegate(read32_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_32_from_16>(delegate.name()), *this));
		else if (m_datawidth == 64)
			set_delegate(read64_delegate(read64_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_64_from_16>(delegate.name()), *this));
	}
}


//-------------------------------------------------
//  set_delegate - set a 32-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_read::set_delegate(read32_delegate delegate, UINT64 mask)
{
	// error if no object
	if (!delegate.has_object())
		throw emu_fatalerror("Attempted to install delegate '%s' without a bound object", delegate.name());

	// make sure this is a valid size
	assert(m_datawidth >= 32);
	m_read32 = delegate;

	// if mismatched bus width, configure a stub
	if (m_datawidth != 32)
	{
		configure_subunits(mask, 16);
		if (m_datawidth == 64)
			set_delegate(read64_delegate(read64_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_64_from_32>(delegate.name()), *this));
	}
}


//-------------------------------------------------
//  set_delegate - set a 64-bit delegate
//-------------------------------------------------

void handler_entry_read::set_delegate(read64_delegate delegate, UINT64 mask)
{
	// error if no object
	if (!delegate.has_object())
		throw emu_fatalerror("Attempted to install delegate '%s' without a bound object", delegate.name());

	// make sure this is a valid size
	assert(m_datawidth >= 64);
	m_read64 = delegate;
}


//-------------------------------------------------
//  set_legacy_func - configure a legacy address
//  space stub of the appropriate size
//-------------------------------------------------

void handler_entry_read::set_legacy_func(address_space &space, read8_space_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.space8 = func;
	m_legacy_object.space = &space;
	set_delegate(read8_delegate(read8_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_legacy>(name), *this), mask);
}

void handler_entry_read::set_legacy_func(address_space &space, read16_space_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.space16 = func;
	m_legacy_object.space = &space;
	set_delegate(read16_delegate(read16_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_legacy>(name), *this), mask);
}

void handler_entry_read::set_legacy_func(address_space &space, read32_space_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.space32 = func;
	m_legacy_object.space = &space;
	set_delegate(read32_delegate(read32_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_legacy>(name), *this), mask);
}

void handler_entry_read::set_legacy_func(address_space &space, read64_space_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.space64 = func;
	m_legacy_object.space = &space;
	set_delegate(read64_delegate(read64_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_legacy>(name), *this), mask);
}


//-------------------------------------------------
//  set_legacy_func - configure a legacy device
//  stub of the appropriate size
//-------------------------------------------------

void handler_entry_read::set_legacy_func(device_t &device, read8_device_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.device8 = func;
	m_legacy_object.device = &device;
	set_delegate(read8_delegate(read8_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_legacy>(name), *this), mask);
}

void handler_entry_read::set_legacy_func(device_t &device, read16_device_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.device16 = func;
	m_legacy_object.device = &device;
	set_delegate(read16_delegate(read16_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_legacy>(name), *this), mask);
}

void handler_entry_read::set_legacy_func(device_t &device, read32_device_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.device32 = func;
	m_legacy_object.device = &device;
	set_delegate(read32_delegate(read32_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_legacy>(name), *this), mask);
}

void handler_entry_read::set_legacy_func(device_t &device, read64_device_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.device64 = func;
	m_legacy_object.device = &device;
	set_delegate(read64_delegate(read64_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_legacy>(name), *this), mask);
}


//-------------------------------------------------
//  set_ioport - configure an I/O port read stub
//  of the appropriate size
//-------------------------------------------------

void handler_entry_read::set_ioport(const input_port_config &ioport)
{
	m_ioport = &ioport;
	if (m_datawidth == 8)
		set_delegate(read8_delegate(read8_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_ioport<UINT8> >(ioport.tag), *this));
	else if (m_datawidth == 16)
		set_delegate(read16_delegate(read16_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_ioport<UINT16> >(ioport.tag), *this));
	else if (m_datawidth == 32)
		set_delegate(read32_delegate(read32_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_ioport<UINT32> >(ioport.tag), *this));
	else if (m_datawidth == 64)
		set_delegate(read64_delegate(read64_proto_delegate::_create_member<handler_entry_read, &handler_entry_read::read_stub_ioport<UINT64> >(ioport.tag), *this));
}


//-------------------------------------------------
//  read_stub_16_from_8 - construct a 16-bit read
//  from 8-bit sources
//-------------------------------------------------

UINT16 handler_entry_read::read_stub_16_from_8(address_space &space, offs_t offset, UINT16 mask)
{
	UINT16 result = 0;
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT8 mask8 = mask >> shift;
		if (mask8 != 0)
			result |= m_read8(space, offset * m_subunits + index, mask8) << shift;
	}
	return result;
}


//-------------------------------------------------
//  read_stub_32_from_8 - construct a 32-bit read
//  from 8-bit sources
//-------------------------------------------------

UINT32 handler_entry_read::read_stub_32_from_8(address_space &space, offs_t offset, UINT32 mask)
{
	UINT32 result = 0;
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT8 mask8 = mask >> shift;
		if (mask8 != 0)
			result |= m_read8(space, offset * m_subunits + index, mask8) << shift;
	}
	return result;
}


//-------------------------------------------------
//  read_stub_64_from_8 - construct a 64-bit read
//  from 8-bit sources
//-------------------------------------------------

UINT64 handler_entry_read::read_stub_64_from_8(address_space &space, offs_t offset, UINT64 mask)
{
	UINT64 result = 0;
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT8 mask8 = mask >> shift;
		if (mask8 != 0)
			result |= (UINT64)m_read8(space, offset * m_subunits + index, mask8) << shift;
	}
	return result;
}


//-------------------------------------------------
//  read_stub_32_from_16 - construct a 32-bit read
//  from 16-bit sources
//-------------------------------------------------

UINT32 handler_entry_read::read_stub_32_from_16(address_space &space, offs_t offset, UINT32 mask)
{
	UINT32 result = 0;
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT16 mask16 = mask >> shift;
		if (mask16 != 0)
			result |= m_read16(space, offset * m_subunits + index, mask16) << shift;
	}
	return result;
}


//-------------------------------------------------
//  read_stub_64_from_16 - construct a 64-bit read
//  from 16-bit sources
//-------------------------------------------------

UINT64 handler_entry_read::read_stub_64_from_16(address_space &space, offs_t offset, UINT64 mask)
{
	UINT64 result = 0;
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT16 mask16 = mask >> shift;
		if (mask16 != 0)
			result |= (UINT64)m_read16(space, offset * m_subunits + index, mask16) << shift;
	}
	return result;
}


//-------------------------------------------------
//  read_stub_64_from_32 - construct a 64-bit read
//  from 32-bit sources
//-------------------------------------------------

UINT64 handler_entry_read::read_stub_64_from_32(address_space &space, offs_t offset, UINT64 mask)
{
	UINT64 result = 0;
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT32 mask32 = mask >> shift;
		if (mask32 != 0)
			result |= (UINT64)m_read32(space, offset * m_subunits + index, mask32) << shift;
	}
	return result;
}


//-------------------------------------------------
//  read_stub_legacy - perform a read using legacy
//  handler callbacks
//-------------------------------------------------

UINT8 handler_entry_read::read_stub_legacy(address_space &space, offs_t offset, UINT8 mask)
{
	return m_legacy_handler.space8(m_legacy_object.space, offset);
}

UINT16 handler_entry_read::read_stub_legacy(address_space &space, offs_t offset, UINT16 mask)
{
	return m_legacy_handler.space16(m_legacy_object.space, offset, mask);
}

UINT32 handler_entry_read::read_stub_legacy(address_space &space, offs_t offset, UINT32 mask)
{
	return m_legacy_handler.space32(m_legacy_object.space, offset, mask);
}

UINT64 handler_entry_read::read_stub_legacy(address_space &space, offs_t offset, UINT64 mask)
{
	return m_legacy_handler.space64(m_legacy_object.space, offset, mask);
}



//**************************************************************************
//  HANDLER ENTRY WRITE
//**************************************************************************

//-------------------------------------------------
//  name - return the handler name, from the
//  appropriately-sized delegate
//-------------------------------------------------

const char *handler_entry_write::name() const
{
	switch (m_datawidth)
	{
		case 8:		return m_write8.name();
		case 16:	return m_write16.name();
		case 32:	return m_write32.name();
		case 64:	return m_write64.name();
	}
	return NULL;
}


//-------------------------------------------------
//  set_delegate - set an 8-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_write::set_delegate(write8_delegate delegate, UINT64 mask)
{
	assert(m_datawidth >= 8);
	m_write8 = delegate;

	// if mismatched bus width, configure a stub
	if (m_datawidth != 8)
	{
		configure_subunits(mask, 8);
		if (m_datawidth == 16)
			set_delegate(write16_delegate(write16_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_16_from_8>(delegate.name()), *this));
		else if (m_datawidth == 32)
			set_delegate(write32_delegate(write32_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_32_from_8>(delegate.name()), *this));
		else if (m_datawidth == 64)
			set_delegate(write64_delegate(write64_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_64_from_8>(delegate.name()), *this));
	}
}


//-------------------------------------------------
//  set_delegate - set a 16-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_write::set_delegate(write16_delegate delegate, UINT64 mask)
{
	assert(m_datawidth >= 16);
	m_write16 = delegate;

	// if mismatched bus width, configure a stub
	if (m_datawidth != 16)
	{
		configure_subunits(mask, 16);
		if (m_datawidth == 32)
			set_delegate(write32_delegate(write32_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_32_from_16>(delegate.name()), *this));
		else if (m_datawidth == 64)
			set_delegate(write64_delegate(write64_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_64_from_16>(delegate.name()), *this));
	}
}


//-------------------------------------------------
//  set_delegate - set a 32-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_write::set_delegate(write32_delegate delegate, UINT64 mask)
{
	assert(m_datawidth >= 32);
	m_write32 = delegate;

	// if mismatched bus width, configure a stub
	if (m_datawidth != 32)
	{
		configure_subunits(mask, 16);
		if (m_datawidth == 64)
			set_delegate(write64_delegate(write64_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_64_from_32>(delegate.name()), *this));
	}
}


//-------------------------------------------------
//  set_delegate - set a 64-bit delegate
//-------------------------------------------------

void handler_entry_write::set_delegate(write64_delegate delegate, UINT64 mask)
{
	assert(m_datawidth >= 64);
	m_write64 = delegate;
}


//-------------------------------------------------
//  set_legacy_func - configure a legacy address
//  space stub of the appropriate size
//-------------------------------------------------

void handler_entry_write::set_legacy_func(address_space &space, write8_space_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.space8 = func;
	m_legacy_object.space = &space;
	set_delegate(write8_delegate(write8_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_legacy>(name), *this), mask);
}

void handler_entry_write::set_legacy_func(address_space &space, write16_space_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.space16 = func;
	m_legacy_object.space = &space;
	set_delegate(write16_delegate(write16_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_legacy>(name), *this), mask);
}

void handler_entry_write::set_legacy_func(address_space &space, write32_space_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.space32 = func;
	m_legacy_object.space = &space;
	set_delegate(write32_delegate(write32_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_legacy>(name), *this), mask);
}

void handler_entry_write::set_legacy_func(address_space &space, write64_space_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.space64 = func;
	m_legacy_object.space = &space;
	set_delegate(write64_delegate(write64_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_legacy>(name), *this), mask);
}


//-------------------------------------------------
//  set_legacy_func - configure a legacy device
//  stub of the appropriate size
//-------------------------------------------------

void handler_entry_write::set_legacy_func(device_t &device, write8_device_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.device8 = func;
	m_legacy_object.device = &device;
	set_delegate(write8_delegate(write8_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_legacy>(name), *this), mask);
}

void handler_entry_write::set_legacy_func(device_t &device, write16_device_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.device16 = func;
	m_legacy_object.device = &device;
	set_delegate(write16_delegate(write16_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_legacy>(name), *this), mask);
}

void handler_entry_write::set_legacy_func(device_t &device, write32_device_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.device32 = func;
	m_legacy_object.device = &device;
	set_delegate(write32_delegate(write32_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_legacy>(name), *this), mask);
}

void handler_entry_write::set_legacy_func(device_t &device, write64_device_func func, const char *name, UINT64 mask)
{
	m_legacy_handler.device64 = func;
	m_legacy_object.device = &device;
	set_delegate(write64_delegate(write64_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_legacy>(name), *this), mask);
}


//-------------------------------------------------
//  set_ioport - configure an I/O port read stub
//  of the appropriate size
//-------------------------------------------------

void handler_entry_write::set_ioport(const input_port_config &ioport)
{
	m_ioport = &ioport;
	if (m_datawidth == 8)
		set_delegate(write8_delegate(write8_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_ioport<UINT8> >(ioport.tag), *this));
	else if (m_datawidth == 16)
		set_delegate(write16_delegate(write16_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_ioport<UINT16> >(ioport.tag), *this));
	else if (m_datawidth == 32)
		set_delegate(write32_delegate(write32_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_ioport<UINT32> >(ioport.tag), *this));
	else if (m_datawidth == 64)
		set_delegate(write64_delegate(write64_proto_delegate::_create_member<handler_entry_write, &handler_entry_write::write_stub_ioport<UINT64> >(ioport.tag), *this));
}


//-------------------------------------------------
//  write_stub_16_from_8 - construct a 16-bit write
//  from 8-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_16_from_8(address_space &space, offs_t offset, UINT16 data, UINT16 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT8 mask8 = mask >> shift;
		if (mask8 != 0)
			m_write8(space, offset * m_subunits + index, data >> shift, mask8);
	}
}


//-------------------------------------------------
//  write_stub_32_from_8 - construct a 32-bit write
//  from 8-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_32_from_8(address_space &space, offs_t offset, UINT32 data, UINT32 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT8 mask8 = mask >> shift;
		if (mask8 != 0)
			m_write8(space, offset * m_subunits + index, data >> shift, mask8);
	}
}


//-------------------------------------------------
//  write_stub_64_from_8 - construct a 64-bit write
//  from 8-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_64_from_8(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT8 mask8 = mask >> shift;
		if (mask8 != 0)
			m_write8(space, offset * m_subunits + index, data >> shift, mask8);
	}
}


//-------------------------------------------------
//  write_stub_32_from_16 - construct a 32-bit
//  write from 16-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_32_from_16(address_space &space, offs_t offset, UINT32 data, UINT32 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT16 mask16 = mask >> shift;
		if (mask16 != 0)
			m_write16(space, offset * m_subunits + index, data >> shift, mask16);
	}
}


//-------------------------------------------------
//  write_stub_64_from_16 - construct a 64-bit
//  write from 16-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_64_from_16(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT16 mask16 = mask >> shift;
		if (mask16 != 0)
			m_write16(space, offset * m_subunits + index, data >> shift, mask16);
	}
}


//-------------------------------------------------
//  write_stub_64_from_32 - construct a 64-bit
//  write from 32-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_64_from_32(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		int shift = m_subshift[index];
		UINT32 mask32 = mask >> shift;
		if (mask32 != 0)
			m_write32(space, offset * m_subunits + index, data >> shift, mask32);
	}
}


//-------------------------------------------------
//  write_stub_legacy - perform a write using
//  legacy handler callbacks
//-------------------------------------------------

void handler_entry_write::write_stub_legacy(address_space &space, offs_t offset, UINT8 data, UINT8 mask)
{
	m_legacy_handler.space8(m_legacy_object.space, offset, data);
}

void handler_entry_write::write_stub_legacy(address_space &space, offs_t offset, UINT16 data, UINT16 mask)
{
	m_legacy_handler.space16(m_legacy_object.space, offset, data, mask);
}

void handler_entry_write::write_stub_legacy(address_space &space, offs_t offset, UINT32 data, UINT32 mask)
{
	m_legacy_handler.space32(m_legacy_object.space, offset, data, mask);
}

void handler_entry_write::write_stub_legacy(address_space &space, offs_t offset, UINT64 data, UINT64 mask)
{
	m_legacy_handler.space64(m_legacy_object.space, offset, data, mask);
}
