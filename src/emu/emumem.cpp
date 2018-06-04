// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    emumem.c

    Functions which handle device memory access.

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
        SUBTABLE_BASE .. TOTAL_MEMORY_BANKS - 1 = need to look up lower bits in subtable

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
        through to the read/write handler. If you use AM_MIRROR, below, there
        should not be any bits in common between mask and mirror.  Same for
        select.

    AM_MIRROR(mirror)
        Specifies mirror addresses for the given bucket. The current bucket
        is mapped repeatedly according to the mirror mask, once where each
        mirror bit is 0, and once where it is 1. For example, a 'mirror'
        value of 0x14000 would map the bucket at 0x00000, 0x04000, 0x10000,
        and 0x14000.

    AM_SELECT(select)
        Mirrors the addresses for the given bucket and pass the corresponding
        address bits to the handler.  Very useful for devices with multiple
        slots/channels/etc were each slot is on a series of consecutive
        addresses regrouping all its registers.

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

    AM_SETOFFSET(setoffset)
        Specifies a handler for a 'set address' operation. The intended use case
        for this operation is to emulate a split-phase memory access: The caller
        (usually a CPU) sets the address bus lines using set_address. Some
        component may then react, for instance, by asserting a control line
        like WAIT before delivering the data on the data bus. The data bits are
        then sampled on the read operation or delivered on the write operation
        that must be called subsequently.
        It is not checked whether the address of the set_address operation
        matches the address of the subsequent read/write operation.
        The address map translates the address to a bucket and an offset,
        hence the name of the macro. If no handler is specified for a bucket,
        a set_address operation hitting that bucket returns silently.

    AM_DEVSETOFFSET(tag, setoffset)
        Specifies a handler for a set_address operation, bound to the device
        specified by 'tag'.


***************************************************************************/

#include <list>
#include <map>

#include "emu.h"
#include "emuopts.h"
#include "debug/debugcpu.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define MEM_DUMP        (0)
#define VERBOSE         (0)
#define TEST_HANDLER    (0)

#define VPRINTF(x)  do { if (VERBOSE) printf x; } while (0)

/*-------------------------------------------------
    core_i64_hex_format - i64 format printf helper
-------------------------------------------------*/

static char *core_i64_hex_format(u64 value, u8 mindigits)
{
	static char buffer[16][64];
	// TODO: this can overflow - e.g. when a lot of unmapped writes are logged
	static int index;
	char *bufbase = &buffer[index++ % 16][0];
	char *bufptr = bufbase;
	s8 curdigit;

	for (curdigit = 15; curdigit >= 0; curdigit--)
	{
		int nibble = (value >> (curdigit * 4)) & 0xf;
		if (nibble != 0 || curdigit < mindigits)
		{
			mindigits = curdigit;
			*bufptr++ = "0123456789ABCDEF"[nibble];
		}
	}
	if (bufptr == bufbase)
		*bufptr++ = '0';
	*bufptr = 0;

	return bufbase;
}



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// banking constants
const int BANK_ENTRY_UNSPECIFIED = -1;

// other address map constants
const int MEMORY_BLOCK_CHUNK = 65536;                   // minimum chunk size of allocated memory blocks

// static data access handler constants
enum
{
	STATIC_INVALID = 0,                                 // invalid - should never be used
	STATIC_BANK1 = 1,                                   // first memory bank
	STATIC_BANKMAX = 0xfb,                              // last memory bank
	STATIC_NOP,                                         // NOP - reads = unmapped value; writes = no-op
	STATIC_UNMAP,                                       // unmapped - same as NOP except we log errors
	STATIC_WATCHPOINT,                                  // watchpoint - used internally
	STATIC_COUNT                                        // total number of static handlers
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> handler_entry

// a handler entry contains information about a memory handler
class handler_entry
{
	DISABLE_COPYING(handler_entry);

protected:
	// construction/destruction
	handler_entry(u8 width, endianness_t endianness, u8 **rambaseptr);
	virtual ~handler_entry();

public:
	// getters
	bool populated() const { return m_populated; }
	offs_t addrstart() const { return m_addrstart; }
	offs_t addrend() const { return m_addrend; }
	offs_t addrmask() const { return m_addrmask; }
	offs_t bytemask() const { return m_bytemask; }
	virtual const char *name() const = 0;
	virtual const char *subunit_name(int entry) const = 0;
	void description(char *buffer) const;

	virtual void copy(handler_entry *entry);

	// return offset within the range referenced by this handler
	offs_t offset(offs_t address) const { return (address - m_addrstart) & m_addrmask; }

	// return a pointer to the backing RAM at the given offset
	u8 *ramptr(offs_t offset = 0) const { return *m_rambaseptr + offset; }

	// see if we are an exact match to the given parameters
	bool matches_exactly(offs_t addrstart, offs_t addrend, offs_t addrmask) const
	{
		return (m_populated && m_addrstart == addrstart && m_addrend == addrend && m_addrmask == addrmask);
	}

	// get the start/end address with the given mirror
	void mirrored_start_end(offs_t address, offs_t &start, offs_t &end) const
	{
		offs_t mirrorbits = (address - m_addrstart) & ~m_addrmask;
		start = m_addrstart | mirrorbits;
		end = m_addrend | mirrorbits;
	}

	// configure the handler addresses, and mark as populated
	void configure(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t bytemask)
	{
		if (m_populated && m_subunits)
			reconfigure_subunits(addrstart);
		m_populated = true;
		m_addrstart = addrstart;
		m_addrend = addrend;
		m_addrmask = addrmask;
		m_bytemask = bytemask;
	}

	// Re-expand the addrmask after subunit fun
	void expand_bytemask(offs_t previous_mask)
	{
		m_bytemask |= previous_mask;
	}

	// reconfigure the subunits on a base address change
	void reconfigure_subunits(offs_t addrstart);

	// depopulate an handler
	void deconfigure()
	{
		m_populated = false;
		m_subunits = 0;
	}

	// apply a global mask
	void apply_mask(offs_t addrmask) { m_addrmask &= addrmask; }

	void clear_conflicting_subunits(u64 handlermask);
	bool overriden_by_mask(u64 handlermask) const;

protected:
	// Subunit description information
	struct subunit_info
	{
		offs_t              m_addrmask;             // addrmask for this subunit
		u64                 m_datamask;             // datamask for this subunit
		u64                 m_csmask;               // chip select mask for this subunit
		s32                 m_offset;               // offset to add to the address
		u32                 m_multiplier;           // multiplier to the pre-split address
		u8                  m_size;                 // size (8, 16 or 32)
		u8                  m_shift;                // shift of the subunit
	};

	// internal helpers
	void configure_subunits(u64 handlermask, int handlerbits, int cswidth, int &start_slot, int &end_slot);
	virtual void remove_subunit(int entry) = 0;

	// internal state
	bool                    m_populated;            // populated?
	u8                      m_datawidth;
	endianness_t            m_endianness;
	offs_t                  m_addrstart;            // start address for handler
	offs_t                  m_addrend;              // end address for handler
	offs_t                  m_addrmask;             // mask against the final address
	offs_t                  m_bytemask;             // mask against the final address, byte resolution
	u8 **                   m_rambaseptr;           // pointer to the bank base
	u8                      m_subunits;             // for width stubs, the number of subunits
	subunit_info            m_subunit_infos[8];     // for width stubs, the associated subunit info
	u64                     m_invsubmask;           // inverted mask of the populated subunits
};


// ======================> handler_entry_read

// a read-access-specific extension of handler_entry
class handler_entry_read : public handler_entry
{
public:
	struct access_handler
	{
		// Constructors mean you can't union them
		read8_delegate              r8;
		read16_delegate             r16;
		read32_delegate             r32;
		read64_delegate             r64;
	};

	// construction/destruction
	handler_entry_read(u8 width, endianness_t endianness, u8 **rambaseptr)
		: handler_entry(width, endianness, rambaseptr),
			m_ioport(nullptr)
	{
	}

	virtual void copy(handler_entry *entry) override;

	// getters
	virtual const char *name() const override;
	virtual const char *subunit_name(int entry) const override;

	// configure delegate callbacks
	void set_delegate(read8_delegate delegate, u64 umask = 0, int cswidth = 8);
	void set_delegate(read16_delegate delegate, u64 umask = 0, int cswidth = 16);
	void set_delegate(read32_delegate delegate, u64 umask = 0, int cswidth = 32);
	void set_delegate(read64_delegate delegate, u64 umask = 0, int cswidth = 64);

	// configure I/O port access
	void set_ioport(ioport_port &ioport);

	// read via the underlying delegates
	u8 read8(address_space &space, offs_t offset, u8 mask) const { return m_read.r8(space, offset, mask); }
	u16 read16(address_space &space, offs_t offset, u16 mask) const { return m_read.r16(space, offset, mask); }
	u32 read32(address_space &space, offs_t offset, u32 mask) const { return m_read.r32(space, offset, mask); }
	u64 read64(address_space &space, offs_t offset, u64 mask) const { return m_read.r64(space, offset, mask); }

private:
	// stubs for converting between address sizes
	u16 read_stub_16(address_space &space, offs_t offset, u16 mask);
	u32 read_stub_32(address_space &space, offs_t offset, u32 mask);
	u64 read_stub_64(address_space &space, offs_t offset, u64 mask);

	// stubs for reading I/O ports
	template<typename UintType>
	UintType read_stub_ioport(address_space &space, offs_t offset, UintType mask) { return m_ioport->read(); }

	// internal helper
	virtual void remove_subunit(int entry) override;

	// internal state
	access_handler              m_read;
	access_handler              m_subread[8];
	ioport_port *   m_ioport;
};


// ======================> handler_entry_write

// a write-access-specific extension of handler_entry
class handler_entry_write : public handler_entry
{
public:
	struct access_handler
	{
		// Constructors mean you can't union them
		write8_delegate             w8;
		write16_delegate            w16;
		write32_delegate            w32;
		write64_delegate            w64;
	};

	// construction/destruction
	handler_entry_write(u8 width, endianness_t endianness, u8 **rambaseptr)
		: handler_entry(width, endianness, rambaseptr),
			m_ioport(nullptr)
	{
	}

	virtual void copy(handler_entry *entry) override;

	// getters
	virtual const char *name() const override;
	virtual const char *subunit_name(int entry) const override;

	// configure delegate callbacks
	void set_delegate(write8_delegate delegate, u64 umask = 0, int cswidth = 8);
	void set_delegate(write16_delegate delegate, u64 umask = 0, int cswidth = 16);
	void set_delegate(write32_delegate delegate, u64 umask = 0, int cswidth = 32);
	void set_delegate(write64_delegate delegate, u64 umask = 0, int cswidth = 64);

	// configure I/O port access
	void set_ioport(ioport_port &ioport);

	// write via the underlying delegates
	void write8(address_space &space, offs_t offset, u8 data, u8 mask) const { m_write.w8(space, offset, data, mask); }
	void write16(address_space &space, offs_t offset, u16 data, u16 mask) const { m_write.w16(space, offset, data, mask); }
	void write32(address_space &space, offs_t offset, u32 data, u32 mask) const { m_write.w32(space, offset, data, mask); }
	void write64(address_space &space, offs_t offset, u64 data, u64 mask) const { m_write.w64(space, offset, data, mask); }

private:
	// stubs for converting between address sizes
	void write_stub_16(address_space &space, offs_t offset, u16 data, u16 mask);
	void write_stub_32(address_space &space, offs_t offset, u32 data, u32 mask);
	void write_stub_64(address_space &space, offs_t offset, u64 data, u64 mask);

	// stubs for writing I/O ports
	template<typename UintType>
	void write_stub_ioport(address_space &space, offs_t offset, UintType data, UintType mask) { m_ioport->write(data, mask); }

	// internal helper
	virtual void remove_subunit(int entry) override;

	// internal state
	access_handler              m_write;
	access_handler              m_subwrite[8];
	ioport_port *   m_ioport;
};

// ======================> handler_entry_setoffset
// a setoffset-access-specific extension of handler_entry
class handler_entry_setoffset : public handler_entry
{
public:
	// construction/destruction
	handler_entry_setoffset()
		: handler_entry(0, ENDIANNESS_LITTLE, nullptr)
	{
	}

	const char *name() const override { return m_setoffset.name(); }
	const char *subunit_name(int entry) const override { return "no subunit"; }

	// Call through only if the setoffset handler has been late-bound before
	// (i.e. if it was declared in the address map)
	void setoffset(address_space &space, offs_t offset) const { if (m_setoffset.has_object()) m_setoffset(space, offset); }

	// configure delegate callbacks
	void set_delegate(setoffset_delegate delegate, u64 umask = 0, int cswidth = 0) { m_setoffset = delegate; }

private:
	setoffset_delegate         m_setoffset;
	// We do not have subunits for setoffset
	// Accordingly, we need not implement unused functions.
	void remove_subunit(int entry) override { }
};

// ======================> handler_entry_proxy

// A proxy class that contains an handler_entry_read or _write and forwards the setter calls

template<typename HandlerEntry>
class handler_entry_proxy
{
public:
	handler_entry_proxy(std::list<HandlerEntry *> &&_handlers, u64 _umask, int _cswidth) : handlers(std::move(_handlers)), umask(_umask), cswidth(_cswidth) {}
	handler_entry_proxy(const handler_entry_proxy<HandlerEntry> &hep) : handlers(hep.handlers), umask(hep.umask), cswidth(hep.cswidth) {}

	// forward delegate callbacks configuration
	template<typename Delegate> void set_delegate(Delegate delegate) const {
		for (const auto & elem : handlers)
			(elem)->set_delegate(delegate, umask, cswidth);
	}

	// forward I/O port access configuration
	void set_ioport(ioport_port &ioport) const {
		for (const auto & elem : handlers)
			(elem)->set_ioport(ioport);
	}

private:
	std::list<HandlerEntry *> handlers;
	u64 umask;
	int cswidth;
};


// ======================> address_table

// address_table contains information about read/write accesses within an address space
class address_table
{
	// address map lookup table definitions
	static const int LEVEL1_BITS    = 18;                       // number of address bits in the level 1 table
	static const int LEVEL2_BITS    = 32 - LEVEL1_BITS;         // number of address bits in the level 2 table
	static const int SUBTABLE_COUNT = 64;                       // number of slots reserved for subtables
	static const int SUBTABLE_BASE  = TOTAL_MEMORY_BANKS - SUBTABLE_COUNT;     // first index of a subtable
	static const int ENTRY_COUNT    = SUBTABLE_BASE;            // number of legitimate (non-subtable) entries
	static const int SUBTABLE_ALLOC = 8;                        // number of subtables to allocate at a time

	inline int level2_bits() const { return m_large ? LEVEL2_BITS : 0; }

public:
	// construction/destruction
	address_table(address_space &space, bool large);
	virtual ~address_table();

	// getters
	virtual handler_entry &handler(u32 index) const = 0;
	bool watchpoints_enabled() const { return (m_live_lookup == s_watchpoint_table); }

	// address lookups
	u32 lookup_live(offs_t address) const { return m_large ? lookup_live_large(address) : lookup_live_small(address); }
	u32 lookup_live_small(offs_t address) const { return m_live_lookup[address]; }

	u32 lookup_live_large(offs_t address) const
	{
		u32 entry = m_live_lookup[level1_index_large(address)];
		if (entry >= SUBTABLE_BASE)
			entry = m_live_lookup[level2_index_large(entry, address)];
		return entry;
	}

	u32 lookup_live_nowp(offs_t address) const { return m_large ? lookup_live_large_nowp(address) : lookup_live_small_nowp(address); }
	u32 lookup_live_small_nowp(offs_t address) const { return m_table[address]; }

	u32 lookup_live_large_nowp(offs_t address) const
	{
		u32 entry = m_table[level1_index_large(address)];
		if (entry >= SUBTABLE_BASE)
			entry = m_table[level2_index_large(entry, address)];
		return entry;
	}

	u32 lookup(offs_t address) const
	{
		u32 entry = m_live_lookup[level1_index(address)];
		if (entry >= SUBTABLE_BASE)
			entry = m_live_lookup[level2_index(entry, address)];
		return entry;
	}

	// enable watchpoints by swapping in the watchpoint table
	void enable_watchpoints(bool enable = true) { m_live_lookup = enable ? s_watchpoint_table : &m_table[0]; }

	// table mapping helpers
	void map_range(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u16 staticentry);
	std::list<u32> setup_range(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask);
	u16 derive_range(offs_t address, offs_t &addrstart, offs_t &addrend) const;

	// misc helpers
	void mask_all_handlers(offs_t mask);
	const char *handler_name(u16 entry) const;

protected:
	// determine table indexes based on the address
	u32 level1_index_large(offs_t address) const { return address >> LEVEL2_BITS; }
	u32 level2_index_large(u16 l1entry, offs_t address) const { return (1 << LEVEL1_BITS) + ((l1entry - SUBTABLE_BASE) << LEVEL2_BITS) + (address & ((1 << LEVEL2_BITS) - 1)); }
	u32 level1_index(offs_t address) const { return m_large ? level1_index_large(address) : address; }
	u32 level2_index(u16 l1entry, offs_t address) const { return m_large ? level2_index_large(l1entry, address) : 0; }

	// table population/depopulation
	void populate_range_mirrored(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 handler);
	void populate_range(offs_t addrstart, offs_t addrend, u16 handler);

	// subtable management
	u16 subtable_alloc();
	void subtable_realloc(u16 subentry);
	int subtable_merge();
	void subtable_release(u16 subentry);
	u16 *subtable_open(offs_t l1index);
	void subtable_close(offs_t l1index);
	u16 *subtable_ptr(u16 entry) { return &m_table[level2_index(entry, 0)]; }

	// internal state
	std::vector<u16>   m_table;                    // pointer to base of table
	u16 *                m_live_lookup;              // current lookup
	address_space &         m_space;                    // pointer back to the space
	bool                    m_large;                    // large memory model?

	// subtable_data is an internal class with information about each subtable
	class subtable_data
	{
	public:
		subtable_data()
			: m_checksum_valid(false),
				m_checksum(0),
				m_usecount(0) { }

		bool                m_checksum_valid;           // is the checksum valid
		u32                 m_checksum;                 // checksum over all the bytes
		u32                 m_usecount;                 // number of times this has been used
	};
	std::vector<subtable_data>   m_subtable;            // info about each subtable
	u16                     m_subtable_alloc;           // number of subtables allocated

	// static global read-only watchpoint table
	static u16              s_watchpoint_table[1 << LEVEL1_BITS];

private:
	int handler_refcount[SUBTABLE_BASE-STATIC_COUNT];
	u16 handler_next_free[SUBTABLE_BASE-STATIC_COUNT];
	u16 handler_free;
	u16 get_free_handler();
	void verify_reference_counts();
	bool range_simply_masks(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask);
	std::list<u32> setup_range_solid(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror);
	std::list<u32> setup_range_masked(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask);

	void handler_ref(u16 entry, int count)
	{
		assert(entry < SUBTABLE_BASE);
		if (entry >= STATIC_COUNT)
			handler_refcount[entry - STATIC_COUNT] += count;
	}

	void handler_unref(u16 entry)
	{
		assert(entry < SUBTABLE_BASE);
		if (entry >= STATIC_COUNT)
			if (! --handler_refcount[entry - STATIC_COUNT])
			{
				handler(entry).deconfigure();
				handler_next_free[entry - STATIC_COUNT] = handler_free;
				handler_free = entry;
			}
	}
};


// ======================> address_table_read

// read access-specific version of an address table
class address_table_read : public address_table
{
public:
	// construction/destruction
	address_table_read(address_space &space, bool large);
	virtual ~address_table_read();

	// getters
	virtual handler_entry &handler(u32 index) const override;
	handler_entry_read &handler_read(u32 index) const { assert(index < ARRAY_LENGTH(m_handlers)); return *m_handlers[index]; }

	// range getter
	handler_entry_proxy<handler_entry_read> handler_map_range(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask = 0, int cswidth = 0) {
		std::list<u32> entries = setup_range(addrstart, addrend, addrmask, addrmirror, umask);
		std::list<handler_entry_read *> handlers;
		for (std::list<u32>::const_iterator i = entries.begin(); i != entries.end(); ++i)
			handlers.push_back(&handler_read(*i));
		return handler_entry_proxy<handler_entry_read>(std::move(handlers), umask, cswidth);
	}

private:
	// internal unmapped handler
	template<typename UintType>
	UintType unmap_r(address_space &space, offs_t offset, UintType mask)
	{
		if (m_space.log_unmap() && !m_space.m_manager.machine().side_effects_disabled())
		{
			m_space.device().logerror(
					m_space.is_octal()
						? "%s: unmapped %s memory read from %0*o & %0*o\n"
						: "%s: unmapped %s memory read from %0*X & %0*X\n",
					m_space.m_manager.machine().describe_context(), m_space.name(),
					m_space.addrchars(), m_space.byte_to_address(offset * sizeof(UintType)),
					2 * sizeof(UintType), mask);
		}
		return m_space.unmap();
	}

	// internal no-op handler
	template<typename UintType>
	UintType nop_r(address_space &space, offs_t offset, UintType mask)
	{
		return m_space.unmap();
	}

	// internal watchpoint handler
	template<typename UintType, int AddrShift>
	UintType watchpoint_r(address_space &space, offs_t offset, UintType mask)
	{
		int base_shift = sizeof(UintType) == 1 ? 0 : sizeof(UintType) == 2 ? 1 : sizeof(UintType) == 4 ? 2 : 3;
		offset = offset << (AddrShift + base_shift);
		m_space.device().debug()->memory_read_hook(m_space, offset, mask);

		u16 *oldtable = m_live_lookup;
		m_live_lookup = &m_table[0];
		UintType result;
		if (sizeof(UintType) == 1) result = m_space.read_byte(offset);
		if (sizeof(UintType) == 2) result = m_space.read_word(offset, mask);
		if (sizeof(UintType) == 4) result = m_space.read_dword(offset, mask);
		if (sizeof(UintType) == 8) result = m_space.read_qword(offset, mask);
		m_live_lookup = oldtable;
		return result;
	}

	// internal state
	std::unique_ptr<handler_entry_read> m_handlers[TOTAL_MEMORY_BANKS];        // array of user-installed handlers
};


// ======================> address_table_write

// write access-specific version of an address table
class address_table_write : public address_table
{
public:
	// construction/destruction
	address_table_write(address_space &space, bool large);
	virtual ~address_table_write();

	// getters
	virtual handler_entry &handler(u32 index) const override;
	handler_entry_write &handler_write(u32 index) const { assert(index < ARRAY_LENGTH(m_handlers)); return *m_handlers[index]; }

	// range getter
	handler_entry_proxy<handler_entry_write> handler_map_range(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask = 0, int cswidth = 0) {
		std::list<u32> entries = setup_range(addrstart, addrend, addrmask, addrmirror, umask);
		std::list<handler_entry_write *> handlers;
		for (std::list<u32>::const_iterator i = entries.begin(); i != entries.end(); ++i)
			handlers.push_back(&handler_write(*i));
		return handler_entry_proxy<handler_entry_write>(std::move(handlers), umask, cswidth);
	}

private:
	// internal handlers
	template<typename UintType>
	void unmap_w(address_space &space, offs_t offset, UintType data, UintType mask)
	{
		if (m_space.log_unmap() && !m_space.m_manager.machine().side_effects_disabled())
		{
			m_space.device().logerror(
					m_space.is_octal()
						? "%s: unmapped %s memory write to %0*o = %0*o & %0*o\n"
						: "%s: unmapped %s memory write to %0*X = %0*X & %0*X\n",
					m_space.m_manager.machine().describe_context(), m_space.name(),
					m_space.addrchars(), m_space.byte_to_address(offset * sizeof(UintType)),
					2 * sizeof(UintType), data,
					2 * sizeof(UintType), mask);
		}
	}

	template<typename UintType>
	void nop_w(address_space &space, offs_t offset, UintType data, UintType mask)
	{
	}

	template<typename UintType, int AddrShift>
	void watchpoint_w(address_space &space, offs_t offset, UintType data, UintType mask)
	{
		int base_shift = sizeof(UintType) == 1 ? 0 : sizeof(UintType) == 2 ? 1 : sizeof(UintType) == 4 ? 2 : 3;
		offset = offset << (AddrShift + base_shift);
		m_space.device().debug()->memory_write_hook(m_space, offset, data, mask);

		u16 *oldtable = m_live_lookup;
		m_live_lookup = &m_table[0];
		if (sizeof(UintType) == 1) m_space.write_byte(offset, data);
		if (sizeof(UintType) == 2) m_space.write_word(offset, data, mask);
		if (sizeof(UintType) == 4) m_space.write_dword(offset, data, mask);
		if (sizeof(UintType) == 8) m_space.write_qword(offset, data, mask);
		m_live_lookup = oldtable;
	}

	// internal state
	std::unique_ptr<handler_entry_write> m_handlers[TOTAL_MEMORY_BANKS];        // array of user-installed handlers
};

// ======================> address_table_setoffset
// setoffset access-specific version of an address table
class address_table_setoffset : public address_table
{
public:
	// construction/destruction
	address_table_setoffset(address_space &space, bool large)
		: address_table(space, large)
	{
		// allocate handlers for each entry, prepopulating the bankptrs for banks
		for (auto & elem : m_handlers)
			elem = std::make_unique<handler_entry_setoffset>();

		// Watchpoints and unmap states do not make sense for setoffset
		m_handlers[STATIC_NOP]->set_delegate(setoffset_delegate(FUNC(address_table_setoffset::nop_so), this));
		m_handlers[STATIC_NOP]->configure(0, space.addrmask(), ~0, ~0);
	}

	~address_table_setoffset()
	{
	}

	handler_entry &handler(u32 index) const override {    assert(index < ARRAY_LENGTH(m_handlers));   return *m_handlers[index]; }
	handler_entry_setoffset &handler_setoffset(u32 index) const { assert(index < ARRAY_LENGTH(m_handlers)); return *m_handlers[index]; }

	// range getter
	handler_entry_proxy<handler_entry_setoffset> handler_map_range(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask = 0, int cswidth = 0) {
		std::list<u32> entries = setup_range(addrstart, addrend, addrmask, addrmirror, umask);
		std::list<handler_entry_setoffset *> handlers;
		for (std::list<u32>::const_iterator i = entries.begin(); i != entries.end(); ++i)
			handlers.push_back(&handler_setoffset(*i));
		return handler_entry_proxy<handler_entry_setoffset>(std::move(handlers), umask, cswidth);
	}

private:
	// internal handlers
	// Setoffset does not allow for watchpoints, since we assume that a
	// corresponding read/write operation will follow, and the watchpoint will
	// apply for that operation
	// For the same reason it does not make sense to put a warning into the log
	// for unmapped locations, as this will be done by the read/write operation
	void nop_so(address_space &space, offs_t offset)
	{
	}

	// internal state
	std::unique_ptr<handler_entry_setoffset> m_handlers[TOTAL_MEMORY_BANKS];        // array of user-installed handlers
};


// ======================> address_space_specific

// this is a derived class of address_space with specific width, endianness, and table size
template<typename NativeType, endianness_t Endian, int AddrShift, bool Large>
class address_space_specific : public address_space
{
	typedef address_space_specific<NativeType, Endian, AddrShift, Large> this_type;

	// constants describing the native size
	static constexpr u32 NATIVE_BYTES = sizeof(NativeType);
	static constexpr u32 NATIVE_STEP = AddrShift >= 0 ? NATIVE_BYTES << iabs(AddrShift) : NATIVE_BYTES >> iabs(AddrShift);
	static constexpr u32 NATIVE_MASK = NATIVE_STEP - 1;
	static constexpr u32 NATIVE_BITS = 8 * NATIVE_BYTES;
	static constexpr u32 Width = NATIVE_BYTES == 1 ? 0 : NATIVE_BYTES == 2 ? 1 : NATIVE_BYTES == 4 ? 2 : 3;

	// helpers to simplify core code
	u32 read_lookup(offs_t address) const { return Large ? m_read.lookup_live_large(address) : m_read.lookup_live_small(address); }
	u32 write_lookup(offs_t address) const { return Large ? m_write.lookup_live_large(address) : m_write.lookup_live_small(address); }
	u32 setoffset_lookup(offs_t address) const { return Large ? m_setoffset.lookup_live_large(address) : m_setoffset.lookup_live_small(address); }

	static constexpr offs_t offset_to_byte(offs_t offset) { return AddrShift < 0 ? offset << iabs(AddrShift) : offset >> iabs(AddrShift); }

public:
	// construction/destruction
	address_space_specific(memory_manager &manager, device_memory_interface &memory, int spacenum)
		: address_space(manager, memory, spacenum, Large),
			m_read(*this, Large),
			m_write(*this, Large),
			m_setoffset(*this, Large)
	{
#if (TEST_HANDLER)
		// test code to verify the read/write handlers are touching the correct bits
		// and returning the correct results

		// install some dummy RAM for the first 16 bytes with well-known values
		u8 buffer[16];
		for (int index = 0; index < 16; index++)
			buffer[index ^ ((Endian == ENDIANNESS_NATIVE) ? 0 : (data_width()/8 - 1))] = index * 0x11;
		install_ram_generic(0x00, 0x0f, 0x0f, 0, read_or_write::READWRITE, buffer);
		printf("\n\naddress_space(%d, %s, %s)\n", NATIVE_BITS, (Endian == ENDIANNESS_LITTLE) ? "little" : "big", Large ? "large" : "small");

		// walk through the first 8 addresses
		for (int address = 0; address < 8; address++)
		{
			// determine expected values
			u64 expected64 = (u64((address + ((Endian == ENDIANNESS_LITTLE) ? 7 : 0)) * 0x11) << 56) |
								(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 6 : 1)) * 0x11) << 48) |
								(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 5 : 2)) * 0x11) << 40) |
								(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 4 : 3)) * 0x11) << 32) |
								(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 3 : 4)) * 0x11) << 24) |
								(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 2 : 5)) * 0x11) << 16) |
								(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 1 : 6)) * 0x11) <<  8) |
								(u64((address + ((Endian == ENDIANNESS_LITTLE) ? 0 : 7)) * 0x11) <<  0);
			u32 expected32 = (Endian == ENDIANNESS_LITTLE) ? expected64 : (expected64 >> 32);
			u16 expected16 = (Endian == ENDIANNESS_LITTLE) ? expected32 : (expected32 >> 16);
			u8 expected8 = (Endian == ENDIANNESS_LITTLE) ? expected16 : (expected16 >> 8);

			u64 result64;
			u32 result32;
			u16 result16;
			u8 result8;

			// validate byte accesses
			printf("\nAddress %d\n", address);
			printf("   read_byte = "); printf("%02X\n", result8 = read_byte(address)); assert(result8 == expected8);

			// validate word accesses (if aligned)
			if (WORD_ALIGNED(address)) { printf("   read_word = "); printf("%04X\n", result16 = read_word(address)); assert(result16 == expected16); }
			if (WORD_ALIGNED(address)) { printf("   read_word (0xff00) = "); printf("%04X\n", result16 = read_word(address, 0xff00)); assert((result16 & 0xff00) == (expected16 & 0xff00)); }
			if (WORD_ALIGNED(address)) { printf("             (0x00ff) = "); printf("%04X\n", result16 = read_word(address, 0x00ff)); assert((result16 & 0x00ff) == (expected16 & 0x00ff)); }

			// validate unaligned word accesses
			printf("   read_word_unaligned = "); printf("%04X\n", result16 = read_word_unaligned(address)); assert(result16 == expected16);
			printf("   read_word_unaligned (0xff00) = "); printf("%04X\n", result16 = read_word_unaligned(address, 0xff00)); assert((result16 & 0xff00) == (expected16 & 0xff00));
			printf("                       (0x00ff) = "); printf("%04X\n", result16 = read_word_unaligned(address, 0x00ff)); assert((result16 & 0x00ff) == (expected16 & 0x00ff));

			// validate dword acceses (if aligned)
			if (DWORD_ALIGNED(address)) { printf("   read_dword = "); printf("%08X\n", result32 = read_dword(address)); assert(result32 == expected32); }
			if (DWORD_ALIGNED(address)) { printf("   read_dword (0xff000000) = "); printf("%08X\n", result32 = read_dword(address, 0xff000000)); assert((result32 & 0xff000000) == (expected32 & 0xff000000)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x00ff0000) = "); printf("%08X\n", result32 = read_dword(address, 0x00ff0000)); assert((result32 & 0x00ff0000) == (expected32 & 0x00ff0000)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x0000ff00) = "); printf("%08X\n", result32 = read_dword(address, 0x0000ff00)); assert((result32 & 0x0000ff00) == (expected32 & 0x0000ff00)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x000000ff) = "); printf("%08X\n", result32 = read_dword(address, 0x000000ff)); assert((result32 & 0x000000ff) == (expected32 & 0x000000ff)); }
			if (DWORD_ALIGNED(address)) { printf("              (0xffff0000) = "); printf("%08X\n", result32 = read_dword(address, 0xffff0000)); assert((result32 & 0xffff0000) == (expected32 & 0xffff0000)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x0000ffff) = "); printf("%08X\n", result32 = read_dword(address, 0x0000ffff)); assert((result32 & 0x0000ffff) == (expected32 & 0x0000ffff)); }
			if (DWORD_ALIGNED(address)) { printf("              (0xffffff00) = "); printf("%08X\n", result32 = read_dword(address, 0xffffff00)); assert((result32 & 0xffffff00) == (expected32 & 0xffffff00)); }
			if (DWORD_ALIGNED(address)) { printf("              (0x00ffffff) = "); printf("%08X\n", result32 = read_dword(address, 0x00ffffff)); assert((result32 & 0x00ffffff) == (expected32 & 0x00ffffff)); }

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
			if (QWORD_ALIGNED(address)) { printf("   read_qword = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address), 16)); assert(result64 == expected64); }
			if (QWORD_ALIGNED(address)) { printf("   read_qword (0xff00000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xff00000000000000U), 16)); assert((result64 & 0xff00000000000000U) == (expected64 & 0xff00000000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ff000000000000U), 16)); assert((result64 & 0x00ff000000000000U) == (expected64 & 0x00ff000000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ff0000000000U), 16)); assert((result64 & 0x0000ff0000000000U) == (expected64 & 0x0000ff0000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000ff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000ff00000000U), 16)); assert((result64 & 0x000000ff00000000U) == (expected64 & 0x000000ff00000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00000000ff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00000000ff000000U), 16)); assert((result64 & 0x00000000ff000000U) == (expected64 & 0x00000000ff000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000000000ff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000000000ff0000U), 16)); assert((result64 & 0x0000000000ff0000U) == (expected64 & 0x0000000000ff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000000000ff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000000000ff00U), 16)); assert((result64 & 0x000000000000ff00U) == (expected64 & 0x000000000000ff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00000000000000ff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00000000000000ffU), 16)); assert((result64 & 0x00000000000000ffU) == (expected64 & 0x00000000000000ffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffff000000000000U), 16)); assert((result64 & 0xffff000000000000U) == (expected64 & 0xffff000000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffff00000000U), 16)); assert((result64 & 0x0000ffff00000000U) == (expected64 & 0x0000ffff00000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00000000ffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00000000ffff0000U), 16)); assert((result64 & 0x00000000ffff0000U) == (expected64 & 0x00000000ffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000000000ffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000000000ffffU), 16)); assert((result64 & 0x000000000000ffffU) == (expected64 & 0x000000000000ffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffff0000000000U), 16)); assert((result64 & 0xffffff0000000000U) == (expected64 & 0xffffff0000000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffffff000000U), 16)); assert((result64 & 0x0000ffffff000000U) == (expected64 & 0x0000ffffff000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000ffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000ffffff0000U), 16)); assert((result64 & 0x000000ffffff0000U) == (expected64 & 0x000000ffffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000000000ffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000000000ffffffU), 16)); assert((result64 & 0x0000000000ffffffU) == (expected64 & 0x0000000000ffffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffffff00000000U), 16)); assert((result64 & 0xffffffff00000000U) == (expected64 & 0xffffffff00000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ffffffff000000U), 16)); assert((result64 & 0x00ffffffff000000U) == (expected64 & 0x00ffffffff000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffffffff0000U), 16)); assert((result64 & 0x0000ffffffff0000U) == (expected64 & 0x0000ffffffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000ffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000ffffffff00U), 16)); assert((result64 & 0x000000ffffffff00U) == (expected64 & 0x000000ffffffff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00000000ffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00000000ffffffffU), 16)); assert((result64 & 0x00000000ffffffffU) == (expected64 & 0x00000000ffffffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffffffff000000U), 16)); assert((result64 & 0xffffffffff000000U) == (expected64 & 0xffffffffff000000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ffffffffff0000U), 16)); assert((result64 & 0x00ffffffffff0000U) == (expected64 & 0x00ffffffffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffffffffff00U), 16)); assert((result64 & 0x0000ffffffffff00U) == (expected64 & 0x0000ffffffffff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x000000ffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x000000ffffffffffU), 16)); assert((result64 & 0x000000ffffffffffU) == (expected64 & 0x000000ffffffffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffffffffff0000U), 16)); assert((result64 & 0xffffffffffff0000U) == (expected64 & 0xffffffffffff0000U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ffffffffffff00U), 16)); assert((result64 & 0x00ffffffffffff00U) == (expected64 & 0x00ffffffffffff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x0000ffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x0000ffffffffffffU), 16)); assert((result64 & 0x0000ffffffffffffU) == (expected64 & 0x0000ffffffffffffU)); }
			if (QWORD_ALIGNED(address)) { printf("              (0xffffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0xffffffffffffff00U), 16)); assert((result64 & 0xffffffffffffff00U) == (expected64 & 0xffffffffffffff00U)); }
			if (QWORD_ALIGNED(address)) { printf("              (0x00ffffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword(address, 0x00ffffffffffffffU), 16)); assert((result64 & 0x00ffffffffffffffU) == (expected64 & 0x00ffffffffffffffU)); }

			// validate unaligned qword accesses
			printf("   read_qword_unaligned = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address), 16)); assert(result64 == expected64);
			printf("   read_qword_unaligned (0xff00000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xff00000000000000U), 16)); assert((result64 & 0xff00000000000000U) == (expected64 & 0xff00000000000000U));
			printf("                        (0x00ff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ff000000000000U), 16)); assert((result64 & 0x00ff000000000000U) == (expected64 & 0x00ff000000000000U));
			printf("                        (0x0000ff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ff0000000000U), 16)); assert((result64 & 0x0000ff0000000000U) == (expected64 & 0x0000ff0000000000U));
			printf("                        (0x000000ff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000ff00000000U), 16)); assert((result64 & 0x000000ff00000000U) == (expected64 & 0x000000ff00000000U));
			printf("                        (0x00000000ff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00000000ff000000U), 16)); assert((result64 & 0x00000000ff000000U) == (expected64 & 0x00000000ff000000U));
			printf("                        (0x0000000000ff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000000000ff0000U), 16)); assert((result64 & 0x0000000000ff0000U) == (expected64 & 0x0000000000ff0000U));
			printf("                        (0x000000000000ff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000000000ff00U), 16)); assert((result64 & 0x000000000000ff00U) == (expected64 & 0x000000000000ff00U));
			printf("                        (0x00000000000000ff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00000000000000ffU), 16)); assert((result64 & 0x00000000000000ffU) == (expected64 & 0x00000000000000ffU));
			printf("                        (0xffff000000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffff000000000000U), 16)); assert((result64 & 0xffff000000000000U) == (expected64 & 0xffff000000000000U));
			printf("                        (0x0000ffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffff00000000U), 16)); assert((result64 & 0x0000ffff00000000U) == (expected64 & 0x0000ffff00000000U));
			printf("                        (0x00000000ffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00000000ffff0000U), 16)); assert((result64 & 0x00000000ffff0000U) == (expected64 & 0x00000000ffff0000U));
			printf("                        (0x000000000000ffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000000000ffffU), 16)); assert((result64 & 0x000000000000ffffU) == (expected64 & 0x000000000000ffffU));
			printf("                        (0xffffff0000000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffff0000000000U), 16)); assert((result64 & 0xffffff0000000000U) == (expected64 & 0xffffff0000000000U));
			printf("                        (0x0000ffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffffff000000U), 16)); assert((result64 & 0x0000ffffff000000U) == (expected64 & 0x0000ffffff000000U));
			printf("                        (0x000000ffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000ffffff0000U), 16)); assert((result64 & 0x000000ffffff0000U) == (expected64 & 0x000000ffffff0000U));
			printf("                        (0x0000000000ffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000000000ffffffU), 16)); assert((result64 & 0x0000000000ffffffU) == (expected64 & 0x0000000000ffffffU));
			printf("                        (0xffffffff00000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffffff00000000U), 16)); assert((result64 & 0xffffffff00000000U) == (expected64 & 0xffffffff00000000U));
			printf("                        (0x00ffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ffffffff000000U), 16)); assert((result64 & 0x00ffffffff000000U) == (expected64 & 0x00ffffffff000000U));
			printf("                        (0x0000ffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffffffff0000U), 16)); assert((result64 & 0x0000ffffffff0000U) == (expected64 & 0x0000ffffffff0000U));
			printf("                        (0x000000ffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000ffffffff00U), 16)); assert((result64 & 0x000000ffffffff00U) == (expected64 & 0x000000ffffffff00U));
			printf("                        (0x00000000ffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00000000ffffffffU), 16)); assert((result64 & 0x00000000ffffffffU) == (expected64 & 0x00000000ffffffffU));
			printf("                        (0xffffffffff000000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffffffff000000U), 16)); assert((result64 & 0xffffffffff000000U) == (expected64 & 0xffffffffff000000U));
			printf("                        (0x00ffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ffffffffff0000U), 16)); assert((result64 & 0x00ffffffffff0000U) == (expected64 & 0x00ffffffffff0000U));
			printf("                        (0x0000ffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffffffffff00U), 16)); assert((result64 & 0x0000ffffffffff00U) == (expected64 & 0x0000ffffffffff00U));
			printf("                        (0x000000ffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x000000ffffffffffU), 16)); assert((result64 & 0x000000ffffffffffU) == (expected64 & 0x000000ffffffffffU));
			printf("                        (0xffffffffffff0000) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffffffffff0000U), 16)); assert((result64 & 0xffffffffffff0000U) == (expected64 & 0xffffffffffff0000U));
			printf("                        (0x00ffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ffffffffffff00U), 16)); assert((result64 & 0x00ffffffffffff00U) == (expected64 & 0x00ffffffffffff00U));
			printf("                        (0x0000ffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x0000ffffffffffffU), 16)); assert((result64 & 0x0000ffffffffffffU) == (expected64 & 0x0000ffffffffffffU));
			printf("                        (0xffffffffffffff00) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0xffffffffffffff00U), 16)); assert((result64 & 0xffffffffffffff00U) == (expected64 & 0xffffffffffffff00U));
			printf("                        (0x00ffffffffffffff) = "); printf("%s\n", core_i64_hex_format(result64 = read_qword_unaligned(address, 0x00ffffffffffffffU), 16)); assert((result64 & 0x00ffffffffffffffU) == (expected64 & 0x00ffffffffffffffU));
		}
#endif
	}

	// accessors
	virtual address_table_read &read() override { return m_read; }
	virtual address_table_write &write() override { return m_write; }
	virtual address_table_setoffset &setoffset() override { return m_setoffset; }

	// watchpoint control
	virtual void enable_read_watchpoints(bool enable = true) override { m_read.enable_watchpoints(enable); }
	virtual void enable_write_watchpoints(bool enable = true) override { m_write.enable_watchpoints(enable); }

	// generate accessor table
	virtual void accessors(data_accessors &accessors) const override
	{
		accessors.read_byte = reinterpret_cast<u8 (*)(address_space &, offs_t)>(&read_byte_static);
		accessors.read_word = reinterpret_cast<u16 (*)(address_space &, offs_t)>(&read_word_static);
		accessors.read_word_masked = reinterpret_cast<u16 (*)(address_space &, offs_t, u16)>(&read_word_masked_static);
		accessors.read_dword = reinterpret_cast<u32 (*)(address_space &, offs_t)>(&read_dword_static);
		accessors.read_dword_masked = reinterpret_cast<u32 (*)(address_space &, offs_t, u32)>(&read_dword_masked_static);
		accessors.read_qword = reinterpret_cast<u64 (*)(address_space &, offs_t)>(&read_qword_static);
		accessors.read_qword_masked = reinterpret_cast<u64 (*)(address_space &, offs_t, u64)>(&read_qword_masked_static);
		accessors.write_byte = reinterpret_cast<void (*)(address_space &, offs_t, u8)>(&write_byte_static);
		accessors.write_word = reinterpret_cast<void (*)(address_space &, offs_t, u16)>(&write_word_static);
		accessors.write_word_masked = reinterpret_cast<void (*)(address_space &, offs_t, u16, u16)>(&write_word_masked_static);
		accessors.write_dword = reinterpret_cast<void (*)(address_space &, offs_t, u32)>(&write_dword_static);
		accessors.write_dword_masked = reinterpret_cast<void (*)(address_space &, offs_t, u32, u32)>(&write_dword_masked_static);
		accessors.write_qword = reinterpret_cast<void (*)(address_space &, offs_t, u64)>(&write_qword_static);
		accessors.write_qword_masked = reinterpret_cast<void (*)(address_space &, offs_t, u64, u64)>(&write_qword_masked_static);
	}

	// return a pointer to the read bank, or nullptr if none
	virtual void *get_read_ptr(offs_t address) override
	{
		// perform the lookup
		address &= m_addrmask;
		u32 entry = read_lookup(address);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// 8-bit case: RAM/ROM
		if (entry > STATIC_BANKMAX)
			return nullptr;
		return handler.ramptr(handler.offset(address));
	}

	// return a pointer to the write bank, or nullptr if none
	virtual void *get_write_ptr(offs_t address) override
	{
		// perform the lookup
		address &= m_addrmask;
		u32 entry = write_lookup(address);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// 8-bit case: RAM/ROM
		if (entry > STATIC_BANKMAX)
			return nullptr;
		return handler.ramptr(handler.offset(address));
	}

	// native read
	NativeType read_native(offs_t offset, NativeType mask)
	{
		g_profiler.start(PROFILER_MEMREAD);

		if (TEST_HANDLER) printf("[r%X,%s]", offset, core_i64_hex_format(mask, sizeof(NativeType) * 2));

		// look up the handler
		offs_t address = offset & m_addrmask;
		u32 entry = read_lookup(address);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// either read directly from RAM, or call the delegate
		offset = offset_to_byte(handler.offset(address));
		NativeType result;
		if (entry <= STATIC_BANKMAX) result = *reinterpret_cast<NativeType *>(handler.ramptr(offset));
		else if (sizeof(NativeType) == 1) result = handler.read8(*this, offset, mask);
		else if (sizeof(NativeType) == 2) result = handler.read16(*this, offset >> 1, mask);
		else if (sizeof(NativeType) == 4) result = handler.read32(*this, offset >> 2, mask);
		else if (sizeof(NativeType) == 8) result = handler.read64(*this, offset >> 3, mask);

		g_profiler.stop();
		return result;
	}

	// mask-less native read
	NativeType read_native(offs_t offset)
	{
		g_profiler.start(PROFILER_MEMREAD);

		if (TEST_HANDLER) printf("[r%X]", offset);

		// look up the handler
		offs_t address = offset & m_addrmask;
		u32 entry = read_lookup(address);
		const handler_entry_read &handler = m_read.handler_read(entry);

		// either read directly from RAM, or call the delegate
		offset = offset_to_byte(handler.offset(address));
		NativeType result;
		if (entry <= STATIC_BANKMAX) result = *reinterpret_cast<NativeType *>(handler.ramptr(offset));
		else if (sizeof(NativeType) == 1) result = handler.read8(*this, offset, 0xff);
		else if (sizeof(NativeType) == 2) result = handler.read16(*this, offset >> 1, 0xffff);
		else if (sizeof(NativeType) == 4) result = handler.read32(*this, offset >> 2, 0xffffffff);
		else if (sizeof(NativeType) == 8) result = handler.read64(*this, offset >> 3, 0xffffffffffffffffU);

		g_profiler.stop();
		return result;
	}

	// native write
	void write_native(offs_t offset, NativeType data, NativeType mask)
	{
		g_profiler.start(PROFILER_MEMWRITE);

		// look up the handler
		offs_t address = offset & m_addrmask;
		u32 entry = write_lookup(address);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// either write directly to RAM, or call the delegate
		offset = offset_to_byte(handler.offset(address));
		if (entry <= STATIC_BANKMAX)
		{
			NativeType *dest = reinterpret_cast<NativeType *>(handler.ramptr(offset));
			*dest = (*dest & ~mask) | (data & mask);
		}
		else if (sizeof(NativeType) == 1) handler.write8(*this, offset, data, mask);
		else if (sizeof(NativeType) == 2) handler.write16(*this, offset >> 1, data, mask);
		else if (sizeof(NativeType) == 4) handler.write32(*this, offset >> 2, data, mask);
		else if (sizeof(NativeType) == 8) handler.write64(*this, offset >> 3, data, mask);

		g_profiler.stop();
	}

	// mask-less native write
	void write_native(offs_t offset, NativeType data)
	{
		g_profiler.start(PROFILER_MEMWRITE);

		// look up the handler
		offs_t address = offset & m_addrmask;
		u32 entry = write_lookup(address);
		const handler_entry_write &handler = m_write.handler_write(entry);

		// either write directly to RAM, or call the delegate
		offset = offset_to_byte(handler.offset(address));
		if (entry <= STATIC_BANKMAX) *reinterpret_cast<NativeType *>(handler.ramptr(offset)) = data;
		else if (sizeof(NativeType) == 1) handler.write8(*this, offset, data, 0xff);
		else if (sizeof(NativeType) == 2) handler.write16(*this, offset >> 1, data, 0xffff);
		else if (sizeof(NativeType) == 4) handler.write32(*this, offset >> 2, data, 0xffffffff);
		else if (sizeof(NativeType) == 8) handler.write64(*this, offset >> 3, data, 0xffffffffffffffffU);

		g_profiler.stop();
	}


	// Allows to announce a pending read or write operation on this address.
	// The user of the address_space calls a set_address operation which leads
	// to some particular set_offset operation for an entry in the address map.
	void set_address(offs_t address) override
	{
		address &= m_addrmask;
		u32 entry = setoffset_lookup(address);
		const handler_entry_setoffset &handler = m_setoffset.handler_setoffset(entry);

		offs_t offset = handler.offset(address);
		handler.setoffset(*this, offset / sizeof(NativeType));
	}

	// virtual access to these functions
	u8 read_byte(offs_t address) override { return Width == 0 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 0, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xff); }
	u16 read_word(offs_t address) override { return Width == 1 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffff); }
	u16 read_word(offs_t address, u16 mask) override { return memory_read_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u16 read_word_unaligned(offs_t address) override { return memory_read_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffff); }
	u16 read_word_unaligned(offs_t address, u16 mask) override { return memory_read_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u32 read_dword(offs_t address) override { return Width == 2 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffff); }
	u32 read_dword(offs_t address, u32 mask) override { return memory_read_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u32 read_dword_unaligned(offs_t address) override { return memory_read_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffff); }
	u32 read_dword_unaligned(offs_t address, u32 mask) override { return memory_read_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u64 read_qword(offs_t address) override { return Width == 3 ? read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	u64 read_qword(offs_t address, u64 mask) override { return memory_read_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }
	u64 read_qword_unaligned(offs_t address) override { return memory_read_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	u64 read_qword_unaligned(offs_t address, u64 mask) override { return memory_read_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType mask) -> NativeType { return read_native(offset, mask); }, address, mask); }

	void write_byte(offs_t address, u8 data) override { if (Width == 0) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 0, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xff); }
	void write_word(offs_t address, u16 data) override { if (Width == 1) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffff); }
	void write_word(offs_t address, u16 data, u16 mask) override { memory_write_generic<Width, AddrShift, Endian, 1, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_word_unaligned(offs_t address, u16 data) override { memory_write_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffff); }
	void write_word_unaligned(offs_t address, u16 data, u16 mask) override { memory_write_generic<Width, AddrShift, Endian, 1, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_dword(offs_t address, u32 data) override { if (Width == 2) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffff); }
	void write_dword(offs_t address, u32 data, u32 mask) override { memory_write_generic<Width, AddrShift, Endian, 2, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_dword_unaligned(offs_t address, u32 data) override { memory_write_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffff); }
	void write_dword_unaligned(offs_t address, u32 data, u32 mask) override { memory_write_generic<Width, AddrShift, Endian, 2, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_qword(offs_t address, u64 data) override { if (Width == 3) write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffffffffffffU); }
	void write_qword(offs_t address, u64 data, u64 mask) override { memory_write_generic<Width, AddrShift, Endian, 3, true>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }
	void write_qword_unaligned(offs_t address, u64 data) override { memory_write_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, 0xffffffffffffffffU); }
	void write_qword_unaligned(offs_t address, u64 data, u64 mask) override { memory_write_generic<Width, AddrShift, Endian, 3, false>([this](offs_t offset, NativeType data, NativeType mask) { write_native(offset, data, mask); }, address, data, mask); }

	// static access to these functions
	static u8 read_byte_static(this_type &space, offs_t address) { return Width == 0 ? space.read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 0, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, 0xff); }
	static u16 read_word_static(this_type &space, offs_t address) { return Width == 1 ? space.read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 1, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, 0xffff); }
	static u16 read_word_masked_static(this_type &space, offs_t address, u16 mask) { return memory_read_generic<Width, AddrShift, Endian, 1, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, mask); }
	static u32 read_dword_static(this_type &space, offs_t address) { return Width == 2 ? space.read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 2, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, 0xffffffff); }
	static u32 read_dword_masked_static(this_type &space, offs_t address, u32 mask) { return memory_read_generic<Width, AddrShift, Endian, 2, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, mask); }
	static u64 read_qword_static(this_type &space, offs_t address) { return Width == 3 ? space.read_native(address & ~NATIVE_MASK) : memory_read_generic<Width, AddrShift, Endian, 3, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, 0xffffffffffffffffU); }
	static u64 read_qword_masked_static(this_type &space, offs_t address, u64 mask) { return memory_read_generic<Width, AddrShift, Endian, 3, true>([&space](offs_t offset, NativeType mask) -> NativeType { return space.read_native(offset, mask); }, address, mask); }
	static void write_byte_static(this_type &space, offs_t address, u8 data) { if (Width == 0) space.write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 0, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, 0xff); }
	static void write_word_static(this_type &space, offs_t address, u16 data) { if (Width == 1) space.write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 1, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, 0xffff); }
	static void write_word_masked_static(this_type &space, offs_t address, u16 data, u16 mask) { memory_write_generic<Width, AddrShift, Endian, 1, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, mask); }
	static void write_dword_static(this_type &space, offs_t address, u32 data) { if (Width == 2) space.write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 2, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, 0xffffffff); }
	static void write_dword_masked_static(this_type &space, offs_t address, u32 data, u32 mask) { memory_write_generic<Width, AddrShift, Endian, 2, true>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, mask); }
	static void write_qword_static(this_type &space, offs_t address, u64 data) { if (Width == 3) space.write_native(address & ~NATIVE_MASK, data); else memory_write_generic<Width, AddrShift, Endian, 3, false>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, 0xffffffffffffffffU); }
	static void write_qword_masked_static(this_type &space, offs_t address, u64 data, u64 mask) { memory_write_generic<Width, AddrShift, Endian, 3, false>([&space](offs_t offset, NativeType data, NativeType mask) { space.write_native(offset, data, mask); }, address, data, mask); }

	address_table_read      m_read;             // memory read lookup table
	address_table_write     m_write;            // memory write lookup table
	address_table_setoffset m_setoffset;        // memory setoffset lookup table
};

typedef address_space_specific<u8,  ENDIANNESS_LITTLE,  0, false> address_space_8_8le_small;
typedef address_space_specific<u8,  ENDIANNESS_BIG,     0, false> address_space_8_8be_small;
typedef address_space_specific<u16, ENDIANNESS_LITTLE,  3, false> address_space_16_1le_small;
typedef address_space_specific<u16, ENDIANNESS_BIG,     3, false> address_space_16_1be_small;
typedef address_space_specific<u16, ENDIANNESS_LITTLE,  0, false> address_space_16_8le_small;
typedef address_space_specific<u16, ENDIANNESS_BIG,     0, false> address_space_16_8be_small;
typedef address_space_specific<u16, ENDIANNESS_LITTLE, -1, false> address_space_16_16le_small;
typedef address_space_specific<u16, ENDIANNESS_BIG,    -1, false> address_space_16_16be_small;
typedef address_space_specific<u32, ENDIANNESS_LITTLE,  0, false> address_space_32_8le_small;
typedef address_space_specific<u32, ENDIANNESS_BIG,     0, false> address_space_32_8be_small;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, -1, false> address_space_32_16le_small;
typedef address_space_specific<u32, ENDIANNESS_BIG,    -1, false> address_space_32_16be_small;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, -2, false> address_space_32_32le_small;
typedef address_space_specific<u32, ENDIANNESS_BIG,    -2, false> address_space_32_32be_small;
typedef address_space_specific<u64, ENDIANNESS_LITTLE,  0, false> address_space_64_8le_small;
typedef address_space_specific<u64, ENDIANNESS_BIG,     0, false> address_space_64_8be_small;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -1, false> address_space_64_16le_small;
typedef address_space_specific<u64, ENDIANNESS_BIG,    -1, false> address_space_64_16be_small;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -2, false> address_space_64_32le_small;
typedef address_space_specific<u64, ENDIANNESS_BIG,    -2, false> address_space_64_32be_small;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -3, false> address_space_64_64le_small;
typedef address_space_specific<u64, ENDIANNESS_BIG,    -3, false> address_space_64_64be_small;

typedef address_space_specific<u8,  ENDIANNESS_LITTLE,  0, true>  address_space_8_8le_large;
typedef address_space_specific<u8,  ENDIANNESS_BIG,     0, true>  address_space_8_8be_large;
typedef address_space_specific<u16, ENDIANNESS_LITTLE,  3, true>  address_space_16_1le_large;
typedef address_space_specific<u16, ENDIANNESS_BIG,     3, true>  address_space_16_1be_large;
typedef address_space_specific<u16, ENDIANNESS_LITTLE,  0, true>  address_space_16_8le_large;
typedef address_space_specific<u16, ENDIANNESS_BIG,     0, true>  address_space_16_8be_large;
typedef address_space_specific<u16, ENDIANNESS_LITTLE, -1, true>  address_space_16_16le_large;
typedef address_space_specific<u16, ENDIANNESS_BIG,    -1, true>  address_space_16_16be_large;
typedef address_space_specific<u32, ENDIANNESS_LITTLE,  0, true>  address_space_32_8le_large;
typedef address_space_specific<u32, ENDIANNESS_BIG,     0, true>  address_space_32_8be_large;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, -1, true>  address_space_32_16le_large;
typedef address_space_specific<u32, ENDIANNESS_BIG,    -1, true>  address_space_32_16be_large;
typedef address_space_specific<u32, ENDIANNESS_LITTLE, -2, true>  address_space_32_32le_large;
typedef address_space_specific<u32, ENDIANNESS_BIG,    -2, true>  address_space_32_32be_large;
typedef address_space_specific<u64, ENDIANNESS_LITTLE,  0, true>  address_space_64_8le_large;
typedef address_space_specific<u64, ENDIANNESS_BIG,     0, true>  address_space_64_8be_large;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -1, true>  address_space_64_16le_large;
typedef address_space_specific<u64, ENDIANNESS_BIG,    -1, true>  address_space_64_16be_large;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -2, true>  address_space_64_32le_large;
typedef address_space_specific<u64, ENDIANNESS_BIG,    -2, true>  address_space_64_32be_large;
typedef address_space_specific<u64, ENDIANNESS_LITTLE, -3, true>  address_space_64_64le_large;
typedef address_space_specific<u64, ENDIANNESS_BIG,    -3, true>  address_space_64_64be_large;




//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// global watchpoint table
u16 address_table::s_watchpoint_table[1 << LEVEL1_BITS];



//**************************************************************************
//  FUNCTION PROTOTYPES
//**************************************************************************

// debugging
static void generate_memdump(running_machine &machine);



//**************************************************************************
//  MEMORY MANAGER
//**************************************************************************

//-------------------------------------------------
//  memory_manager - constructor
//-------------------------------------------------

memory_manager::memory_manager(running_machine &machine)
	: m_machine(machine),
		m_initialized(false),
		m_banknext(STATIC_BANK1)
{
	memset(m_bank_ptr, 0, sizeof(m_bank_ptr));
}

//-------------------------------------------------
//  allocate - allocate memory spaces
//-------------------------------------------------

void memory_manager::allocate(device_memory_interface &memory)
{
	for (int spacenum = 0; spacenum < memory.max_space_count(); ++spacenum)
	{
		// if there is a configuration for this space, we need an address space
		address_space_config const *const spaceconfig = memory.space_config(spacenum);
		if (spaceconfig)
		{
			// allocate one of the appropriate type
			bool const large(spaceconfig->addr2byte_end(0xffffffffUL >> (32 - spaceconfig->m_addr_width)) >= (1 << 18));

			switch (spaceconfig->data_width())
			{
			case 8:
				if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
				{
					if (large)
						memory.allocate<address_space_8_8le_large>(*this, spacenum);
					else
						memory.allocate<address_space_8_8le_small>(*this, spacenum);
				}
				else
				{
					if (large)
						memory.allocate<address_space_8_8be_large>(*this, spacenum);
					else
						memory.allocate<address_space_8_8be_small>(*this, spacenum);
				}
				break;

			case 16:
				switch (spaceconfig->addr_shift())
				{
				case  3:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_16_1le_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_1le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_16_1be_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_1be_small>(*this, spacenum);
					}
					break;

				case  0:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_16_8le_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_8le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_16_8be_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_8be_small>(*this, spacenum);
					}
					break;

				case -1:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_16_16le_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_16le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_16_16be_large>(*this, spacenum);
						else
							memory.allocate<address_space_16_16be_small>(*this, spacenum);
					}
					break;
				}
				break;

			case 32:
				switch (spaceconfig->addr_shift())
				{
				case  0:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_32_8le_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_8le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_32_8be_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_8be_small>(*this, spacenum);
					}
					break;

				case -1:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_32_16le_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_16le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_32_16be_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_16be_small>(*this, spacenum);
					}
					break;

				case -2:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_32_32le_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_32le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_32_32be_large>(*this, spacenum);
						else
							memory.allocate<address_space_32_32be_small>(*this, spacenum);
					}
					break;
				}
				break;

			case 64:
				switch (spaceconfig->addr_shift())
				{
				case  0:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_64_8le_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_8le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_64_8be_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_8be_small>(*this, spacenum);
					}
					break;

				case -1:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_64_16le_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_16le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_64_16be_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_16be_small>(*this, spacenum);
					}
					break;

				case -2:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_64_32le_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_32le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_64_32be_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_32be_small>(*this, spacenum);
					}
					break;

				case -3:
					if (spaceconfig->endianness() == ENDIANNESS_LITTLE)
					{
						if (large)
							memory.allocate<address_space_64_64le_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_64le_small>(*this, spacenum);
					}
					else
					{
						if (large)
							memory.allocate<address_space_64_64be_large>(*this, spacenum);
						else
							memory.allocate<address_space_64_64be_small>(*this, spacenum);
					}
					break;
				}
				break;

			default:
				throw emu_fatalerror("Invalid width %d specified for address_space::allocate", spaceconfig->data_width());
			}
		}
	}
}

//-------------------------------------------------
//  initialize - initialize the memory system
//-------------------------------------------------

void memory_manager::initialize()
{
	// loop over devices and spaces within each device
	memory_interface_iterator iter(machine().root_device());
	std::vector<device_memory_interface *> memories;
	for (device_memory_interface &memory : iter)
	{
		memories.push_back(&memory);
		allocate(memory);
	}

	allocate(m_machine.m_dummy_space);

	// construct and preprocess the address_map for each space
	for (auto const memory : memories)
		memory->prepare_maps();

	// create the handlers from the resulting address maps
	for (auto const memory : memories)
		memory->populate_from_maps();

	// allocate memory needed to back each address space
	for (auto const memory : memories)
		memory->allocate_memory();

	// find all the allocated pointers
	for (auto const memory : memories)
		memory->locate_memory();

	// disable logging of unmapped access when no one receives it
	if (!machine().options().log() && !machine().options().oslog() && !(machine().debug_flags & DEBUG_FLAG_ENABLED))
		for (auto const memory : memories)
			memory->set_log_unmap(false);

	// register a callback to reset banks when reloading state
	machine().save().register_postload(save_prepost_delegate(FUNC(memory_manager::bank_reattach), this));

	// dump the final memory configuration
	generate_memdump(machine());

	// we are now initialized
	m_initialized = true;
}


//-------------------------------------------------
//  region_alloc - allocates memory for a region
//-------------------------------------------------

memory_region *memory_manager::region_alloc(const char *name, u32 length, u8 width, endianness_t endian)
{
	osd_printf_verbose("Region '%s' created\n", name);
	// make sure we don't have a region of the same name; also find the end of the list
	if (m_regionlist.find(name) != m_regionlist.end())
		fatalerror("region_alloc called with duplicate region name \"%s\"\n", name);

	// allocate the region
	m_regionlist.emplace(name, std::make_unique<memory_region>(machine(), name, length, width, endian));
	return m_regionlist.find(name)->second.get();
}


//-------------------------------------------------
//  region_free - releases memory for a region
//-------------------------------------------------

void memory_manager::region_free(const char *name)
{
	m_regionlist.erase(name);
}


//-------------------------------------------------
//  region_containing - helper to determine if
//  a block of memory is part of a region
//-------------------------------------------------

memory_region *memory_manager::region_containing(const void *memory, offs_t bytes) const
{
	const u8 *data = reinterpret_cast<const u8 *>(memory);

	// look through the region list and return the first match
	for (auto &region : m_regionlist)
		if (data >= region.second->base() && (data + bytes) <= region.second->end())
			return region.second.get();

	// didn't find one
	return nullptr;
}


//-------------------------------------------------
//  generate_memdump - internal memory dump
//-------------------------------------------------

static void generate_memdump(running_machine &machine)
{
	if (MEM_DUMP)
	{
		FILE *file = fopen("memdump.log", "w");
		if (file)
		{
			memory_interface_iterator iter(machine.root_device());
			for (device_memory_interface &memory : iter)
				memory.dump(file);
			fclose(file);
		}
	}
}


//-------------------------------------------------
//  bank_reattach - reconnect banks after a load
//-------------------------------------------------

void memory_manager::bank_reattach()
{
	// for each non-anonymous bank, explicitly reset its entry
	for (auto &bank : m_banklist)
		if (!bank.second->anonymous() && bank.second->entry() != BANK_ENTRY_UNSPECIFIED)
			bank.second->set_entry(bank.second->entry());
}



//**************************************************************************
//  ADDRESS SPACE
//**************************************************************************

//-------------------------------------------------
//  address_space - constructor
//-------------------------------------------------

address_space::address_space(memory_manager &manager, device_memory_interface &memory, int spacenum, bool large)
	: m_config(*memory.space_config(spacenum)),
		m_device(memory.device()),
		m_addrmask(0xffffffffUL >> (32 - m_config.m_addr_width)),
		m_logaddrmask(0xffffffffUL >> (32 - m_config.m_logaddr_width)),
		m_unmap(0),
		m_spacenum(spacenum),
		m_log_unmap(true),
		m_name(memory.space_config(spacenum)->name()),
		m_addrchars((m_config.m_addr_width + 3) / 4),
		m_logaddrchars((m_config.m_logaddr_width + 3) / 4),
		m_manager(manager)
{
	switch(m_config.data_width()) {
	case  8:
		if(m_config.endianness() == ENDIANNESS_LITTLE)
			cache_init<0,  0, ENDIANNESS_LITTLE>();
		else
			cache_init<0,  0, ENDIANNESS_BIG>();
		break;
	case 16:
		switch(m_config.addr_shift()) {
		case  3:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<1,  3, ENDIANNESS_LITTLE>();
			else
				cache_init<1,  3, ENDIANNESS_BIG>();
			break;
		case  0:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<1,  0, ENDIANNESS_LITTLE>();
			else
				cache_init<1,  0, ENDIANNESS_BIG>();
			break;
		case -1:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<1, -1, ENDIANNESS_LITTLE>();
			else
				cache_init<1, -1, ENDIANNESS_BIG>();
			break;
		}
		break;
	case 32:
		switch(m_config.addr_shift()) {
		case  0:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<2,  0, ENDIANNESS_LITTLE>();
			else
				cache_init<2,  0, ENDIANNESS_BIG>();
			break;
		case -1:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<2, -1, ENDIANNESS_LITTLE>();
			else
				cache_init<2, -1, ENDIANNESS_BIG>();
			break;
		case -2:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<2, -2, ENDIANNESS_LITTLE>();
			else
				cache_init<2, -2, ENDIANNESS_BIG>();
			break;
		}
		break;
	case 64:
		switch(m_config.addr_shift()) {
		case  0:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<3,  0, ENDIANNESS_LITTLE>();
			else
				cache_init<3,  0, ENDIANNESS_BIG>();
			break;
		case -1:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<3, -1, ENDIANNESS_LITTLE>();
			else
				cache_init<3, -1, ENDIANNESS_BIG>();
			break;
		case -2:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<3, -2, ENDIANNESS_LITTLE>();
			else
				cache_init<3, -2, ENDIANNESS_BIG>();
			break;
		case -3:
			if(m_config.endianness() == ENDIANNESS_LITTLE)
				cache_init<3, -3, ENDIANNESS_LITTLE>();
			else
				cache_init<3, -3, ENDIANNESS_BIG>();
			break;
		}
		break;
	}

	m_cache = m_cache_alloc(nullptr);
}


//-------------------------------------------------
//  ~address_space - destructor
//-------------------------------------------------

address_space::~address_space()
{
	m_cache_delete(m_cache);
}


template<int Width, int AddrShift, int Endian> void address_space::cache_init()
{
	using dt = memory_access_cache<Width, AddrShift, Endian>;
	m_cache_alloc = [this](void *cache) -> void * { (void)cache; return static_cast<void *>(new dt(*this)); };
	m_cache_delete = [](void *cache) { delete static_cast<dt *>(cache); };
	m_cache_invalidate_read_caches = [](void *cache) { static_cast<dt *>(cache)->force_update(); };
	m_cache_invalidate_read_cache_entry = [](void *cache, u16 entry) { static_cast<dt *>(cache)->force_update(entry); };
	m_cache_invalidate_read_cache_range = [](void *cache, offs_t start, offs_t end) { static_cast<dt *>(cache)->remove_intersecting_ranges(start, end); };
}


//-------------------------------------------------
//  adjust_addresses - adjust addresses for a
//  given address space in a standard fashion
//-------------------------------------------------

inline void address_space::adjust_addresses(offs_t &start, offs_t &end, offs_t &mask, offs_t &mirror)
{
	// adjust start/end/mask values
	mask &= m_addrmask;
	start &= ~mirror & m_addrmask;
	end &= ~mirror & m_addrmask;
}

void address_space::check_optimize_all(const char *function, int width, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, u64 unitmask, int cswidth, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror, u64 &nunitmask, int &ncswidth)
{
	if (addrstart > addrend)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, start address is after the end address.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect);
	if (addrstart & ~m_addrmask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, start address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, m_addrmask, addrstart & m_addrmask);
	if (addrend & ~m_addrmask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, end address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, m_addrmask, addrend & m_addrmask);

	// Check the relative data widths
	if (width > m_config.data_width())
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, cannot install a %d-bits wide handler in a %d-bits wide address space.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, width, m_config.data_width());

	// Check the validity of the addresses given their intrinsic width
	// We assume that busses with non-zero address shift have a data width matching the shift (reality says yes)
	offs_t default_lowbits_mask = (m_config.data_width() >> (3 - m_config.m_addr_shift)) - 1;
	offs_t lowbits_mask = width && !m_config.m_addr_shift ? (width >> 3) - 1 : default_lowbits_mask;

	if (addrstart & lowbits_mask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, start address has low bits set, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrstart & ~lowbits_mask);
	if ((~addrend) & lowbits_mask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, end address has low bits unset, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrend | lowbits_mask);


	offs_t set_bits = addrstart | addrend;
	offs_t changing_bits = addrstart ^ addrend;
	// Round up to the nearest power-of-two-minus-one
	changing_bits |= changing_bits >> 1;
	changing_bits |= changing_bits >> 2;
	changing_bits |= changing_bits >> 4;
	changing_bits |= changing_bits >> 8;
	changing_bits |= changing_bits >> 16;

	if (addrmask & ~m_addrmask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mask is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, m_addrmask, addrmask & m_addrmask);
	if (addrselect & ~m_addrmask)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, select is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, m_addrmask, addrselect & m_addrmask);
	if (addrmask & ~changing_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mask is trying to unmask an unchanging address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrmask & changing_bits);
	if (addrmirror & changing_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mirror touches a changing address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrmirror & ~changing_bits);
	if (addrselect & changing_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, select touches a changing address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrselect & ~changing_bits);
	if (addrmirror & set_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mirror touches a set address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrmirror & ~set_bits);
	if (addrselect & set_bits)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, select touches a set address bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrselect & ~set_bits);
	if (addrmirror & addrselect)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, mirror touches a select bit, did you mean %x ?\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, addrmirror & ~addrselect);

	// Check the cswidth, if provided
	if (cswidth > m_config.data_width())
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, the cswidth of %d is too large for a %d-bit space.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, cswidth, m_config.data_width());
	if (width && (cswidth % width) != 0)
		fatalerror("%s: In range %x-%x mask %x mirror %x select %x, the cswidth of %d is not a multiple of handler size %d.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, cswidth, width);
	ncswidth = cswidth ? cswidth : width;

	// Check if the unitmask is structurally correct for the width
	// Not sure what we can actually handle regularity-wise, so don't check that yet
	if (width) {
		// Check if the 1-blocks are of appropriate size
		u64 block_mask = 0xffffffffffffffffU >> (64 - width);
		u64 cs_mask = 0xffffffffffffffffU >> (64 - ncswidth);
		for(int pos = 0; pos < 64; pos += ncswidth) {
			u64 cmask = (unitmask >> pos) & cs_mask;
			while (cmask != 0 && (cmask & block_mask) == 0)
				cmask >>= width;
			if (cmask != 0 && cmask != block_mask)
				fatalerror("%s: In range %x-%x mask %x mirror %x select %x, the unitmask of %s has incorrect granularity for %d-bit chip selection.\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, core_i64_hex_format(unitmask, 16), cswidth);
		}
	}

	// Check if we have to adjust the unitmask and addresses
	nunitmask = 0xffffffffffffffffU >> (64 - m_config.data_width());
	if (unitmask)
		nunitmask &= unitmask;
	if ((addrstart & default_lowbits_mask) || ((~addrend) & default_lowbits_mask)) {
		if ((addrstart ^ addrend) & ~default_lowbits_mask)
			fatalerror("%s: In range %x-%x mask %x mirror %x select %x, start or end is unaligned while the range spans more than one slot (granularity = %d).\n", function, addrstart, addrend, addrmask, addrmirror, addrselect, default_lowbits_mask + 1);
		offs_t lowbyte = m_config.addr2byte(addrstart & default_lowbits_mask);
		offs_t highbyte = m_config.addr2byte((addrend & default_lowbits_mask) + 1);
		if (m_config.endianness() == ENDIANNESS_LITTLE) {
			u64 hmask = 0xffffffffffffffffU >> (64 - 8*highbyte);
			nunitmask = (nunitmask << (8*lowbyte)) & hmask;
		} else {
			u64 hmask = 0xffffffffffffffffU >> ((64 - m_config.data_width()) + 8*lowbyte);
			nunitmask = (nunitmask << (m_config.data_width() - 8*highbyte)) & hmask;
		}

		addrstart &= ~default_lowbits_mask;
		addrend |= default_lowbits_mask;
		if(changing_bits < default_lowbits_mask)
			changing_bits = default_lowbits_mask;
	}

	nstart = addrstart;
	nend = addrend;
	nmask = (addrmask ? addrmask : changing_bits) | addrselect;
	nmirror = (addrmirror & m_addrmask) | addrselect;
	if(nmirror && !(nstart & changing_bits) && !((~nend) & changing_bits)) {
		// If the range covers the a complete power-of-two zone, it is
		// possible to remove 1 bits from the mirror, pushing the end
		// address.  The mask will clamp, and installing the range
		// will be faster.
		while(nmirror & (changing_bits+1)) {
			offs_t bit = nmirror & (changing_bits+1);
			nmirror &= ~bit;
			nend |= bit;
			changing_bits |= bit;
		}
	}
}

void address_space::check_optimize_mirror(const char *function, offs_t addrstart, offs_t addrend, offs_t addrmirror, offs_t &nstart, offs_t &nend, offs_t &nmask, offs_t &nmirror)
{
	if (addrstart > addrend)
		fatalerror("%s: In range %x-%x mirror %x, start address is after the end address.\n", function, addrstart, addrend, addrmirror);
	if (addrstart & ~m_addrmask)
		fatalerror("%s: In range %x-%x mirror %x, start address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmirror, m_addrmask, addrstart & m_addrmask);
	if (addrend & ~m_addrmask)
		fatalerror("%s: In range %x-%x mirror %x, end address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmirror, m_addrmask, addrend & m_addrmask);

	offs_t lowbits_mask = (m_config.data_width() >> (3 - m_config.m_addr_shift)) - 1;
	if (addrstart & lowbits_mask)
		fatalerror("%s: In range %x-%x mirror %x, start address has low bits set, did you mean %x ?\n", function, addrstart, addrend, addrmirror, addrstart & ~lowbits_mask);
	if ((~addrend) & lowbits_mask)
		fatalerror("%s: In range %x-%x mirror %x, end address has low bits unset, did you mean %x ?\n", function, addrstart, addrend, addrmirror, addrend | lowbits_mask);

	offs_t set_bits = addrstart | addrend;
	offs_t changing_bits = addrstart ^ addrend;
	// Round up to the nearest power-of-two-minus-one
	changing_bits |= changing_bits >> 1;
	changing_bits |= changing_bits >> 2;
	changing_bits |= changing_bits >> 4;
	changing_bits |= changing_bits >> 8;
	changing_bits |= changing_bits >> 16;

	if (addrmirror & ~m_addrmask)
		fatalerror("%s: In range %x-%x mirror %x, mirror is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, addrmirror, m_addrmask, addrmirror & m_addrmask);
	if (addrmirror & changing_bits)
		fatalerror("%s: In range %x-%x mirror %x, mirror touches a changing address bit, did you mean %x ?\n", function, addrstart, addrend, addrmirror, addrmirror & ~changing_bits);
	if (addrmirror & set_bits)
		fatalerror("%s: In range %x-%x mirror %x, mirror touches a set address bit, did you mean %x ?\n", function, addrstart, addrend, addrmirror, addrmirror & ~set_bits);

	nstart = addrstart;
	nend = addrend;
	nmask = changing_bits;
	nmirror = addrmirror;

	if(nmirror && !(nstart & changing_bits) && !((~nend) & changing_bits)) {
		// If the range covers the a complete power-of-two zone, it is
		// possible to remove 1 bits from the mirror, pushing the end
		// address.  The mask will clamp, and installing the range
		// will be faster.
		while(nmirror & (changing_bits+1)) {
			offs_t bit = nmirror & (changing_bits+1);
			nmirror &= ~bit;
			nend |= bit;
			changing_bits |= bit;
		}
	}
}

void address_space::check_address(const char *function, offs_t addrstart, offs_t addrend)
{
	if (addrstart > addrend)
		fatalerror("%s: In range %x-%x, start address is after the end address.\n", function, addrstart, addrend);
	if (addrstart & ~m_addrmask)
		fatalerror("%s: In range %x-%x, start address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, m_addrmask, addrstart & m_addrmask);
	if (addrend & ~m_addrmask)
		fatalerror("%s: In range %x-%x, end address is outside of the global address mask %x, did you mean %x ?\n", function, addrstart, addrend, m_addrmask, addrend & m_addrmask);

	offs_t lowbits_mask = (m_config.data_width() >> (3 - m_config.m_addr_shift)) - 1;
	if (addrstart & lowbits_mask)
		fatalerror("%s: In range %x-%x, start address has low bits set, did you mean %x ?\n", function, addrstart, addrend, addrstart & ~lowbits_mask);
	if ((~addrend) & lowbits_mask)
		fatalerror("%s: In range %x-%x, end address has low bits unset, did you mean %x ?\n", function, addrstart, addrend, addrend | lowbits_mask);
}


//-------------------------------------------------
//  prepare_map - allocate the address map and
//  walk through it to find implicit memory regions
//  and identify shared regions
//-------------------------------------------------

void address_space::prepare_map()
{
	memory_region *devregion = (m_spacenum == 0) ? m_device.memregion(DEVICE_SELF) : nullptr;
	u32 devregionsize = (devregion != nullptr) ? devregion->bytes() : 0;

	// allocate the address map
	m_map = std::make_unique<address_map>(m_device, m_spacenum);

	// merge in the submaps
	m_map->import_submaps(m_manager.machine(), m_device.owner() ? *m_device.owner() : m_device, data_width(), endianness());

	// extract global parameters specified by the map
	m_unmap = (m_map->m_unmapval == 0) ? 0 : ~0;
	if (m_map->m_globalmask != 0)
		m_addrmask = m_map->m_globalmask;

	// make a pass over the address map, adjusting for the device and getting memory pointers
	for (address_map_entry &entry : m_map->m_entrylist)
	{
		// computed adjusted addresses first
		adjust_addresses(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror);

		// if we have a share entry, add it to our map
		if (entry.m_share != nullptr)
		{
			// if we can't find it, add it to our map
			std::string fulltag = entry.m_devbase.subtag(entry.m_share);
			if (m_manager.m_sharelist.find(fulltag.c_str()) == m_manager.m_sharelist.end())
			{
				VPRINTF(("Creating share '%s' of length 0x%X\n", fulltag.c_str(), entry.m_addrend + 1 - entry.m_addrstart));
				m_manager.m_sharelist.emplace(fulltag.c_str(), std::make_unique<memory_share>(m_config.data_width(), address_to_byte(entry.m_addrend + 1 - entry.m_addrstart), endianness()));
			}
		}

		// if this is a ROM handler without a specified region, attach it to the implicit region
		if (m_spacenum == 0 && entry.m_read.m_type == AMH_ROM && entry.m_region == nullptr)
		{
			// make sure it fits within the memory region before doing so, however
			if (entry.m_addrend < devregionsize)
			{
				entry.m_region = m_device.tag();
				entry.m_rgnoffs = address_to_byte(entry.m_addrstart);
			}
		}

		// validate adjusted addresses against implicit regions
		if (entry.m_region != nullptr && entry.m_share == nullptr)
		{
			// determine full tag
			std::string fulltag = entry.m_devbase.subtag(entry.m_region);

			// find the region
			memory_region *region = m_manager.machine().root_device().memregion(fulltag.c_str());
			if (region == nullptr)
				fatalerror("device '%s' %s space memory map entry %X-%X references non-existant region \"%s\"\n", m_device.tag(), m_name, entry.m_addrstart, entry.m_addrend, entry.m_region);

			// validate the region
			if (entry.m_rgnoffs + m_config.addr2byte(entry.m_addrend - entry.m_addrstart + 1) > region->bytes())
				fatalerror("device '%s' %s space memory map entry %X-%X extends beyond region \"%s\" size (%X)\n", m_device.tag(), m_name, entry.m_addrstart, entry.m_addrend, entry.m_region, region->bytes());
		}

		// convert any region-relative entries to their memory pointers
		if (entry.m_region != nullptr)
		{
			// determine full tag
			std::string fulltag = entry.m_devbase.subtag(entry.m_region);

			// set the memory address
			entry.m_memory = m_manager.machine().root_device().memregion(fulltag.c_str())->base() + entry.m_rgnoffs;
		}
	}

	// now loop over all the handlers and enforce the address mask
	read().mask_all_handlers(m_addrmask);
	write().mask_all_handlers(m_addrmask);
}


//-------------------------------------------------
//  populate_from_map - walk the map in reverse
//  order and install the appropriate handler for
//  each case
//-------------------------------------------------

void address_space::populate_from_map(address_map *map)
{
	// no map specified, use the space-specific one
	if (map == nullptr)
		map = m_map.get();

	// no map, nothing to do
	if (map == nullptr)
		return;

	// install the handlers, using the original, unadjusted memory map
	for (const address_map_entry &entry : map->m_entrylist)
	{
		// map both read and write halves
		populate_map_entry(entry, read_or_write::READ);
		populate_map_entry(entry, read_or_write::WRITE);
		populate_map_entry_setoffset(entry);
	}
}


//-------------------------------------------------
//  populate_map_entry - map a single read or
//  write entry based on information from an
//  address map entry
//-------------------------------------------------

void address_space::populate_map_entry(const address_map_entry &entry, read_or_write readorwrite)
{
	const map_handler_data &data = (readorwrite == read_or_write::READ) ? entry.m_read : entry.m_write;
	// based on the handler type, alter the bits, name, funcptr, and object
	switch (data.m_type)
	{
		case AMH_NONE:
			return;

		case AMH_ROM:
			// writes to ROM are no-ops
			if (readorwrite == read_or_write::WRITE)
				return;
			// fall through to the RAM case otherwise

		case AMH_RAM:
			install_ram_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, readorwrite, nullptr);
			break;

		case AMH_NOP:
			unmap_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, readorwrite, true);
			break;

		case AMH_UNMAP:
			unmap_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror, readorwrite, false);
			break;

		case AMH_DEVICE_DELEGATE:
			if (readorwrite == read_or_write::READ)
				switch (data.m_bits)
				{
					case 8:     install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, read8_delegate(entry.m_rproto8, entry.m_devbase), entry.m_mask, entry.m_cswidth); break;
					case 16:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, read16_delegate(entry.m_rproto16, entry.m_devbase), entry.m_mask, entry.m_cswidth); break;
					case 32:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, read32_delegate(entry.m_rproto32, entry.m_devbase), entry.m_mask, entry.m_cswidth); break;
					case 64:    install_read_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, read64_delegate(entry.m_rproto64, entry.m_devbase), entry.m_mask, entry.m_cswidth); break;
				}
			else
				switch (data.m_bits)
				{
					case 8:     install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, write8_delegate(entry.m_wproto8, entry.m_devbase), entry.m_mask, entry.m_cswidth); break;
					case 16:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, write16_delegate(entry.m_wproto16, entry.m_devbase), entry.m_mask, entry.m_cswidth); break;
					case 32:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, write32_delegate(entry.m_wproto32, entry.m_devbase), entry.m_mask, entry.m_cswidth); break;
					case 64:    install_write_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask, entry.m_addrmirror, entry.m_addrselect, write64_delegate(entry.m_wproto64, entry.m_devbase), entry.m_mask, entry.m_cswidth); break;
				}
			break;

		case AMH_PORT:
			install_readwrite_port(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror,
							(readorwrite == read_or_write::READ) ? data.m_tag : nullptr,
							(readorwrite == read_or_write::WRITE) ? data.m_tag : nullptr);
			break;

		case AMH_BANK:
			install_bank_generic(entry.m_addrstart, entry.m_addrend, entry.m_addrmirror,
							(readorwrite == read_or_write::READ) ? data.m_tag : nullptr,
							(readorwrite == read_or_write::WRITE) ? data.m_tag : nullptr);
			break;

		case AMH_DEVICE_SUBMAP:
			throw emu_fatalerror("Internal mapping error: leftover mapping of '%s'.\n", data.m_tag);
	}
}

//-------------------------------------------------
//  populate_map_entry_setoffset - special case for setoffset
//-------------------------------------------------

void address_space::populate_map_entry_setoffset(const address_map_entry &entry)
{
	install_setoffset_handler(entry.m_addrstart, entry.m_addrend, entry.m_addrmask,
		entry.m_addrmirror, entry.m_addrselect, setoffset_delegate(entry.m_soproto, entry.m_devbase), entry.m_mask);
}

//-------------------------------------------------
//  allocate_memory - determine all neighboring
//  address ranges and allocate memory to back
//  them
//-------------------------------------------------

void address_space::allocate_memory()
{
	auto &blocklist = m_manager.m_blocklist;

	// make a first pass over the memory map and track blocks with hardcoded pointers
	// we do this to make sure they are found by space_find_backing_memory first
	// do it back-to-front so that overrides work correctly
	int tail = blocklist.size();
	for (address_map_entry &entry : m_map->m_entrylist)
		if (entry.m_memory != nullptr)
			blocklist.insert(blocklist.begin() + tail, std::make_unique<memory_block>(*this, entry.m_addrstart, entry.m_addrend, entry.m_memory));

	// loop over all blocks just allocated and assign pointers from them
	address_map_entry *unassigned = nullptr;

	for (auto memblock = blocklist.begin() + tail; memblock != blocklist.end(); ++memblock)
		unassigned = block_assign_intersecting(memblock->get()->addrstart(), memblock->get()->addrend(), memblock->get()->data());

	// if we don't have an unassigned pointer yet, try to find one
	if (unassigned == nullptr)
		unassigned = block_assign_intersecting(~0, 0, nullptr);

	// loop until we've assigned all memory in this space
	while (unassigned != nullptr)
	{
		// work in MEMORY_BLOCK_CHUNK-sized chunks
		offs_t curblockstart = unassigned->m_addrstart / MEMORY_BLOCK_CHUNK;
		offs_t curblockend = unassigned->m_addrend / MEMORY_BLOCK_CHUNK;

		// loop while we keep finding unassigned blocks in neighboring MEMORY_BLOCK_CHUNK chunks
		bool changed;
		do
		{
			changed = false;

			// scan for unmapped blocks in the adjusted map
			for (address_map_entry &entry : m_map->m_entrylist)
				if (entry.m_memory == nullptr && &entry != unassigned && needs_backing_store(entry))
				{
					// get block start/end blocks for this block
					offs_t blockstart = entry.m_addrstart / MEMORY_BLOCK_CHUNK;
					offs_t blockend = entry.m_addrend / MEMORY_BLOCK_CHUNK;

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
		offs_t curaddrstart = curblockstart * MEMORY_BLOCK_CHUNK;
		offs_t curaddrend = curblockend * MEMORY_BLOCK_CHUNK + (MEMORY_BLOCK_CHUNK - 1);
		auto block = std::make_unique<memory_block>(*this, curaddrstart, curaddrend);

		// assign memory that intersected the new block
		unassigned = block_assign_intersecting(curaddrstart, curaddrend, block.get()->data());
		blocklist.push_back(std::move(block));
	}
}


//-------------------------------------------------
//  locate_memory - find all the requested
//  pointers into the final allocated memory
//-------------------------------------------------

void address_space::locate_memory()
{
	// once this is done, find the starting bases for the banks
	for (auto &bank : m_manager.banks())
		if (bank.second->base() == nullptr && bank.second->references_space(*this, read_or_write::READWRITE))
		{
			// set the initial bank pointer
			for (address_map_entry &entry : m_map->m_entrylist)
				if (entry.m_addrstart == bank.second->addrstart() && entry.m_memory != nullptr)
				{
					bank.second->set_base(entry.m_memory);
					VPRINTF(("assigned bank '%s' pointer to memory from range %08X-%08X [%p]\n", bank.second->tag(), entry.m_addrstart, entry.m_addrend, entry.m_memory));
					break;
				}

			// if the entry was set ahead of time, override the automatically found pointer
			if (!bank.second->anonymous() && bank.second->entry() != BANK_ENTRY_UNSPECIFIED)
				bank.second->set_entry(bank.second->entry());
		}
}




//-------------------------------------------------
//  block_assign_intersecting - find all
//  intersecting blocks and assign their pointers
//-------------------------------------------------

address_map_entry *address_space::block_assign_intersecting(offs_t addrstart, offs_t addrend, u8 *base)
{
	address_map_entry *unassigned = nullptr;

	// loop over the adjusted map and assign memory to any blocks we can
	for (address_map_entry &entry : m_map->m_entrylist)
	{
		// if we haven't assigned this block yet, see if we have a mapped shared pointer for it
		if (entry.m_memory == nullptr && entry.m_share != nullptr)
		{
			std::string fulltag = entry.m_devbase.subtag(entry.m_share);
			auto share = m_manager.shares().find(fulltag.c_str());
			if (share != m_manager.shares().end() && share->second->ptr() != nullptr)
			{
				entry.m_memory = share->second->ptr();
				VPRINTF(("memory range %08X-%08X -> shared_ptr '%s' [%p]\n", entry.m_addrstart, entry.m_addrend, entry.m_share, entry.m_memory));
			}
			else
			{
				VPRINTF(("memory range %08X-%08X -> shared_ptr '%s' but not found\n", entry.m_addrstart, entry.m_addrend, entry.m_share));
			}
		}

		// otherwise, look for a match in this block
		if (entry.m_memory == nullptr && entry.m_addrstart >= addrstart && entry.m_addrend <= addrend)
		{
			entry.m_memory = base + m_config.addr2byte(entry.m_addrstart - addrstart);
			VPRINTF(("memory range %08X-%08X -> found in block from %08X-%08X [%p]\n", entry.m_addrstart, entry.m_addrend, addrstart, addrend, entry.m_memory));
		}

		// if we're the first match on a shared pointer, assign it now
		if (entry.m_memory != nullptr && entry.m_share != nullptr)
		{
			std::string fulltag = entry.m_devbase.subtag(entry.m_share);
			auto share = m_manager.shares().find(fulltag.c_str());
			if (share != m_manager.shares().end() && share->second->ptr() == nullptr)
			{
				share->second->set_ptr(entry.m_memory);
				VPRINTF(("setting shared_ptr '%s' = %p\n", entry.m_share, entry.m_memory));
			}
		}

		// keep track of the first unassigned entry
		if (entry.m_memory == nullptr && unassigned == nullptr && needs_backing_store(entry))
			unassigned = &entry;
	}

	return unassigned;
}


//-------------------------------------------------
//  get_handler_string - return a string
//  describing the handler at a particular offset
//-------------------------------------------------

const char *address_space::get_handler_string(read_or_write readorwrite, offs_t address)
{
	if (readorwrite == read_or_write::READ)
		return read().handler_name(read().lookup(address));
	else
		return write().handler_name(write().lookup(address));
}


//-------------------------------------------------
//  dump_map - dump the contents of a single
//  address space
//-------------------------------------------------

void address_space::dump_map(FILE *file, read_or_write readorwrite)
{
	const address_table &table = (readorwrite == read_or_write::READ) ? static_cast<address_table &>(read()) : static_cast<address_table &>(write());

	// dump generic information
	fprintf(file, "  Address bits = %d\n", m_config.m_addr_width);
	fprintf(file, "     Data bits = %d\n", m_config.m_data_width);
	fprintf(file, "  Address mask = %X\n", m_addrmask);
	fprintf(file, "\n");

	// iterate over addresses
	offs_t addrstart, addrend;
	for (offs_t address = 0; address <= m_addrmask; address = addrend)
	{
		u16 entry = table.derive_range(address, addrstart, addrend);
		fprintf(file, "%08X-%08X    = %02X: %s [offset=%08X]\n",
						addrstart, addrend, entry, table.handler_name(entry), table.handler(entry).addrstart());
		if (++addrend == 0)
			break;
	}
}


//**************************************************************************
//  DYNAMIC ADDRESS SPACE MAPPING
//**************************************************************************

//-------------------------------------------------
//  unmap - unmap a section of address space
//-------------------------------------------------

void address_space::unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, bool quiet)
{
	VPRINTF(("address_space::unmap(%s-%s mirror=%s, %s, %s)\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				(readorwrite == read_or_write::READ) ? "read" : (readorwrite == read_or_write::WRITE) ? "write" : (readorwrite == read_or_write::READWRITE) ? "read/write" : "??",
				quiet ? "quiet" : "normal"));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("unmap_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// read space
	if (readorwrite == read_or_write::READ || readorwrite == read_or_write::READWRITE)
		read().map_range(nstart, nend, nmask, nmirror, quiet ? STATIC_NOP : STATIC_UNMAP);

	// write space
	if (readorwrite == read_or_write::WRITE || readorwrite == read_or_write::READWRITE)
		write().map_range(nstart, nend, nmask, nmirror, quiet ? STATIC_NOP : STATIC_UNMAP);
}


//-------------------------------------------------
//  install_device_delegate - install the memory map
//  of a live device into this address space
//-------------------------------------------------

void address_space::install_device_delegate(offs_t addrstart, offs_t addrend, device_t &device, address_map_constructor &delegate, u64 unitmask, int cswidth)
{
	check_address("install_device_delegate", addrstart, addrend);
	address_map map(*this, addrstart, addrend, unitmask, cswidth, m_device, delegate);
	map.import_submaps(m_manager.machine(), device, data_width(), endianness());
	populate_from_map(&map);
}



//-------------------------------------------------
//  install_readwrite_port - install a new I/O port
//  handler into this address space
//-------------------------------------------------

void address_space::install_readwrite_port(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *rtag, const char *wtag)
{
	VPRINTF(("address_space::install_readwrite_port(%s-%s mirror=%s, read=\"%s\" / write=\"%s\")\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				(rtag != nullptr) ? rtag : "(none)", (wtag != nullptr) ? wtag : "(none)"));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_readwrite_port", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// read handler
	if (rtag != nullptr)
	{
		// find the port
		ioport_port *port = device().owner()->ioport(rtag);
		if (port == nullptr)
			throw emu_fatalerror("Attempted to map non-existent port '%s' for read in space %s of device '%s'\n", rtag, m_name, m_device.tag());

		// map the range and set the ioport
		read().handler_map_range(nstart, nend, nmask, nmirror).set_ioport(*port);
	}

	if (wtag != nullptr)
	{
		// find the port
		ioport_port *port = device().owner()->ioport(wtag);
		if (port == nullptr)
			fatalerror("Attempted to map non-existent port '%s' for write in space %s of device '%s'\n", wtag, m_name, m_device.tag());

		// map the range and set the ioport
		write().handler_map_range(nstart, nend, nmask, nmirror).set_ioport(*port);
	}

	// update the memory dump
	generate_memdump(m_manager.machine());
}


//-------------------------------------------------
//  install_bank_generic - install a range as
//  mapping to a particular bank
//-------------------------------------------------

void address_space::install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, const char *rtag, const char *wtag)
{
	VPRINTF(("address_space::install_readwrite_bank(%s-%s mirror=%s, read=\"%s\" / write=\"%s\")\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				(rtag != nullptr) ? rtag : "(none)", (wtag != nullptr) ? wtag : "(none)"));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_bank_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// map the read bank
	if (rtag != nullptr)
	{
		std::string fulltag = device().siblingtag(rtag);
		memory_bank &bank = bank_find_or_allocate(fulltag.c_str(), addrstart, addrend, addrmirror, read_or_write::READ);
		read().map_range(nstart, nend, nmask, nmirror, bank.index());
	}

	// map the write bank
	if (wtag != nullptr)
	{
		std::string fulltag = device().siblingtag(wtag);
		memory_bank &bank = bank_find_or_allocate(fulltag.c_str(), addrstart, addrend, addrmirror, read_or_write::WRITE);
		write().map_range(nstart, nend, nmask, nmirror, bank.index());
	}

	// update the memory dump
	generate_memdump(m_manager.machine());
}


void address_space::install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, memory_bank *rbank, memory_bank *wbank)
{
	VPRINTF(("address_space::install_readwrite_bank(%s-%s mirror=%s, read=\"%s\" / write=\"%s\")\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				(rbank != nullptr) ? rbank->tag() : "(none)", (wbank != nullptr) ? wbank->tag() : "(none)"));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_bank_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// map the read bank
	if (rbank != nullptr)
	{
		read().map_range(nstart, nend, nmask, nmirror, rbank->index());
	}

	// map the write bank
	if (wbank != nullptr)
	{
		write().map_range(nstart, nend, nmask, nmirror, wbank->index());
	}

	// update the memory dump
	generate_memdump(m_manager.machine());
}


//-------------------------------------------------
//  install_ram_generic - install a simple fixed
//  RAM region into the given address space
//-------------------------------------------------

void address_space::install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite, void *baseptr)
{
	VPRINTF(("address_space::install_ram_generic(%s-%s mirror=%s, %s, %p)\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmirror, m_addrchars),
				(readorwrite == read_or_write::READ) ? "read" : (readorwrite == read_or_write::WRITE) ? "write" : (readorwrite == read_or_write::READWRITE) ? "read/write" : "??",
				baseptr));

	offs_t nstart, nend, nmask, nmirror;
	check_optimize_mirror("install_ram_generic", addrstart, addrend, addrmirror, nstart, nend, nmask, nmirror);

	// map for read
	if (readorwrite == read_or_write::READ || readorwrite == read_or_write::READWRITE)
	{
		// find a bank and map it
		memory_bank &bank = bank_find_or_allocate(nullptr, addrstart, addrend, addrmirror, read_or_write::READ);
		read().map_range(nstart, nend, nmask, nmirror, bank.index());

		// if we are provided a pointer, set it
		if (baseptr != nullptr)
			bank.set_base(baseptr);

		// if we don't have a bank pointer yet, try to find one
		if (bank.base() == nullptr)
		{
			void *backing = find_backing_memory(addrstart, addrend);
			if (backing != nullptr)
				bank.set_base(backing);
		}

		// if we still don't have a pointer, and we're past the initialization phase, allocate a new block
		if (bank.base() == nullptr && m_manager.m_initialized)
		{
			if (m_manager.machine().phase() >= machine_phase::RESET)
				fatalerror("Attempted to call install_ram_generic() after initialization time without a baseptr!\n");
			auto block = std::make_unique<memory_block>(*this, addrstart, addrend);
			bank.set_base(block.get()->data());
			m_manager.m_blocklist.push_back(std::move(block));
		}
	}

	// map for write
	if (readorwrite == read_or_write::WRITE || readorwrite == read_or_write::READWRITE)
	{
		// find a bank and map it
		memory_bank &bank = bank_find_or_allocate(nullptr, addrstart, addrend, addrmirror, read_or_write::WRITE);
		write().map_range(nstart, nend, nmask, nmirror, bank.index());

		// if we are provided a pointer, set it
		if (baseptr != nullptr)
			bank.set_base(baseptr);

		// if we don't have a bank pointer yet, try to find one
		if (bank.base() == nullptr)
		{
			void *backing = find_backing_memory(addrstart, addrend);
			if (backing != nullptr)
				bank.set_base(backing);
		}

		// if we still don't have a pointer, and we're past the initialization phase, allocate a new block
		if (bank.base() == nullptr && m_manager.m_initialized)
		{
			if (m_manager.machine().phase() >= machine_phase::RESET)
				fatalerror("Attempted to call install_ram_generic() after initialization time without a baseptr!\n");
			auto block = std::make_unique<memory_block>(*this, address_to_byte(addrstart), address_to_byte_end(addrend));
			bank.set_base(block.get()->data());
			m_manager.m_blocklist.push_back(std::move(block));
		}
	}
}


//-------------------------------------------------
//  install_handler - install 8-bit read/write
//  delegate handlers for the space
//-------------------------------------------------

void address_space::install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate handler, u64 unitmask, int cswidth)
{
	VPRINTF(("address_space::install_read_handler(%s-%s mask=%s mirror=%s, %s, %s)\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				handler.name(), core_i64_hex_format(unitmask, data_width() / 4)));

	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_read_handler", 8, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

	read().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
	generate_memdump(m_manager.machine());
}

void address_space::install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write8_delegate handler, u64 unitmask, int cswidth)
{
	VPRINTF(("address_space::install_write_handler(%s-%s mask=%s mirror=%s, %s, %s)\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				handler.name(), core_i64_hex_format(unitmask, data_width() / 4)));

	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_write_handler", 8, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);

	write().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
	generate_memdump(m_manager.machine());
}

void address_space::install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read8_delegate rhandler, write8_delegate whandler, u64 unitmask, int cswidth)
{
	install_read_handler(addrstart, addrend, addrmask, addrmirror, addrselect, rhandler, unitmask, cswidth);
	install_write_handler(addrstart, addrend, addrmask, addrmirror, addrselect, whandler, unitmask, cswidth);
}


//-------------------------------------------------
//  install_handler - install 16-bit read/write
//  delegate handlers for the space
//-------------------------------------------------

void address_space::install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate handler, u64 unitmask, int cswidth)
{
	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_read_handler", 16, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);
	read().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
	generate_memdump(m_manager.machine());
}

void address_space::install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write16_delegate handler, u64 unitmask, int cswidth)
{
	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_write_handler", 16, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);
	write().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
	generate_memdump(m_manager.machine());
}

void address_space::install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read16_delegate rhandler, write16_delegate whandler, u64 unitmask, int cswidth)
{
	install_read_handler(addrstart, addrend, addrmask, addrmirror, addrselect, rhandler, unitmask, cswidth);
	install_write_handler(addrstart, addrend, addrmask, addrmirror, addrselect, whandler, unitmask, cswidth);
}


//-------------------------------------------------
//  install_handler - install 32-bit read/write
//  delegate handlers for the space
//-------------------------------------------------

void address_space::install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate handler, u64 unitmask, int cswidth)
{
	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_read_handler", 32, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);
	read().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
	generate_memdump(m_manager.machine());
}

void address_space::install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write32_delegate handler, u64 unitmask, int cswidth)
{
	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_write_handler", 32, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);
	write().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
	generate_memdump(m_manager.machine());
}

void address_space::install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read32_delegate rhandler, write32_delegate whandler, u64 unitmask, int cswidth)
{
	install_read_handler(addrstart, addrend, addrmask, addrmirror, addrselect, rhandler, unitmask, cswidth);
	install_write_handler(addrstart, addrend, addrmask, addrmirror, addrselect, whandler, unitmask, cswidth);
}


//-------------------------------------------------
//  install_handler64 - install 64-bit read/write
//  delegate handlers for the space
//-------------------------------------------------

void address_space::install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate handler, u64 unitmask, int cswidth)
{
	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_read_handler", 64, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);
	read().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
	generate_memdump(m_manager.machine());
}

void address_space::install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, write64_delegate handler, u64 unitmask, int cswidth)
{
	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_write_handler", 64, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);
	write().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
	generate_memdump(m_manager.machine());
}

void address_space::install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, read64_delegate rhandler, write64_delegate whandler, u64 unitmask, int cswidth)
{
	install_read_handler(addrstart, addrend, addrmask, addrmirror, addrselect, rhandler, unitmask, cswidth);
	install_write_handler(addrstart, addrend, addrmask, addrmirror, addrselect, whandler, unitmask, cswidth);
}


//-----------------------------------------------------------------------
//  install_setoffset_handler - install set_offset delegate handlers for the space
//-----------------------------------------------------------------------

void address_space::install_setoffset_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, offs_t addrselect, setoffset_delegate handler, u64 unitmask, int cswidth)
{
	VPRINTF(("address_space::install_setoffset_handler(%s-%s mask=%s mirror=%s, %s, %s)\n",
				core_i64_hex_format(addrstart, m_addrchars), core_i64_hex_format(addrend, m_addrchars),
				core_i64_hex_format(addrmask, m_addrchars), core_i64_hex_format(addrmirror, m_addrchars),
				handler.name(), core_i64_hex_format(unitmask, data_width() / 4)));

	offs_t nstart, nend, nmask, nmirror;
	u64 nunitmask;
	int ncswidth;
	check_optimize_all("install_setoffset_handler", 8, addrstart, addrend, addrmask, addrmirror, addrselect, unitmask, cswidth, nstart, nend, nmask, nmirror, nunitmask, ncswidth);
	setoffset().handler_map_range(nstart, nend, nmask, nmirror, nunitmask, ncswidth).set_delegate(handler);
}

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
	VPRINTF(("address_space::find_backing_memory('%s',%s,%08X-%08X) -> ", m_device.tag(), m_name, addrstart, addrend));

	if (m_map == nullptr)
		return nullptr;

	// look in the address map first, last winning for overrides
	void *result = nullptr;
	for (address_map_entry &entry : m_map->m_entrylist)
	{
		if (entry.m_memory != nullptr && addrstart >= entry.m_addrstart && addrend <= entry.m_addrend)
		{
			VPRINTF(("found in entry %08X-%08X [%p]\n", entry.m_addrstart, entry.m_addrend, (u8 *)entry.m_memory + address_to_byte(addrstart - entry.m_addrstart)));
			result = (u8 *)entry.m_memory + address_to_byte(addrstart - entry.m_addrstart);
		}
	}
	if (result)
		return result;

	// if not found there, look in the allocated blocks
	for (auto &block : m_manager.m_blocklist)
		if (block->contains(*this, addrstart, addrend))
		{
			VPRINTF(("found in allocated memory block %08X-%08X [%p]\n", block->addrstart(), block->addrend(), block->data() + address_to_byte(addrstart - block->addrstart())));
			return block->data() + address_to_byte(addrstart - block->addrstart());
		}

	VPRINTF(("did not find\n"));
	return nullptr;
}


//-------------------------------------------------
//  space_needs_backing_store - return whether a
//  given memory map entry implies the need of
//  allocating and registering memory
//-------------------------------------------------

bool address_space::needs_backing_store(const address_map_entry &entry)
{
	// if we are sharing, and we don't have a pointer yet, create one
	if (entry.m_share != nullptr)
	{
		std::string fulltag = entry.m_devbase.subtag(entry.m_share);
		auto share = m_manager.shares().find(fulltag.c_str());
		if (share != m_manager.shares().end() && share->second->ptr() == nullptr)
			return true;
	}

	// if we're writing to any sort of bank or RAM, then yes, we do need backing
	if (entry.m_write.m_type == AMH_BANK || entry.m_write.m_type == AMH_RAM)
		return true;

	// if we're reading from RAM or from ROM outside of address space 0 or its region, then yes, we do need backing
	memory_region *region = m_manager.machine().root_device().memregion(m_device.tag());
	if (entry.m_read.m_type == AMH_RAM ||
		(entry.m_read.m_type == AMH_ROM && (m_spacenum != 0 || region == nullptr || entry.m_addrstart >= region->bytes())))
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

memory_bank &address_space::bank_find_or_allocate(const char *tag, offs_t addrstart, offs_t addrend, offs_t addrmirror, read_or_write readorwrite)
{
	// adjust the addresses, handling mirrors and such
	offs_t addrmask = ~addrmirror;
	adjust_addresses(addrstart, addrend, addrmask, addrmirror);

	// look up the bank by name, or else by byte range
	memory_bank *membank = nullptr;
	if (tag != nullptr) {
		auto bank = m_manager.banks().find(tag);
		if (bank != m_manager.banks().end())
			membank = bank->second.get();
	}
	else {
		membank = bank_find_anonymous(addrstart, addrend);
	}

	// if we don't have a bank yet, find a free one
	if (membank == nullptr)
	{
		// handle failure
		int banknum = m_manager.m_banknext++;
		if (banknum > STATIC_BANKMAX)
		{
			if (tag != nullptr)
				throw emu_fatalerror("Unable to allocate new bank '%s'", tag);
			else
				throw emu_fatalerror("Unable to allocate bank for RAM/ROM area %X-%X\n", addrstart, addrend);
		}

		// if no tag, create a unique one
		auto bank = std::make_unique<memory_bank>(*this, banknum, addrstart, addrend, tag);
		std::string temptag;
		if (tag == nullptr) {
			temptag = string_format("anon_%p", bank.get());
			tag = temptag.c_str();
		}
		m_manager.m_banklist.emplace(tag, std::move(bank));
		membank = m_manager.m_banklist.find(tag)->second.get();
	}

	// add a reference for this space
	membank->add_reference(*this, readorwrite);
	return *membank;
}


//-------------------------------------------------
//  bank_find_anonymous - try to find an anonymous
//  bank matching the given byte range
//-------------------------------------------------
memory_bank *address_space::bank_find_anonymous(offs_t addrstart, offs_t addrend) const
{
	// try to find an exact match
	for (auto &bank : m_manager.banks())
		if (bank.second->anonymous() && bank.second->references_space(*this, read_or_write::READWRITE) && bank.second->matches_exactly(addrstart, addrend))
			return bank.second.get();

	// not found
	return nullptr;
}

//-------------------------------------------------
//  address_space::invalidate_read_caches -- clear
//  the read cache (cache) for a specific entry
//  or all of them
//-------------------------------------------------

void address_space::invalidate_read_caches()
{
	m_cache_invalidate_read_caches(m_cache);
}

void address_space::invalidate_read_caches(u16 entry)
{
	m_cache_invalidate_read_cache_entry(m_cache, entry);
}

void address_space::invalidate_read_caches(offs_t start, offs_t end)
{
	m_cache_invalidate_read_cache_range(m_cache, start, end);
}


//**************************************************************************
//  TABLE MANAGEMENT
//**************************************************************************

//-------------------------------------------------
//  address_table - constructor
//-------------------------------------------------

address_table::address_table(address_space &space, bool large)
	: m_table(1 << LEVEL1_BITS),
		m_space(space),
		m_large(large),
		m_subtable(SUBTABLE_COUNT),
		m_subtable_alloc(0)
{
	m_live_lookup = &m_table[0];

	// make our static table all watchpoints
	if (s_watchpoint_table[0] != STATIC_WATCHPOINT)
		for (unsigned int i=0; i != ARRAY_LENGTH(s_watchpoint_table); i++)
			s_watchpoint_table[i] = STATIC_WATCHPOINT;

	// initialize everything to unmapped
	for (unsigned int i=0; i != 1 << LEVEL1_BITS; i++)
		m_table[i] = STATIC_UNMAP;

	// initialize the handlers freelist
	for (int i=0; i != SUBTABLE_BASE-STATIC_COUNT-1; i++)
		handler_next_free[i] = i+STATIC_COUNT+1;
	handler_next_free[SUBTABLE_BASE-STATIC_COUNT-1] = STATIC_INVALID;
	handler_free = STATIC_COUNT;

	// initialize the handlers refcounts
	memset(handler_refcount, 0, sizeof(handler_refcount));
}


//-------------------------------------------------
//  ~address_table - destructor
//-------------------------------------------------

address_table::~address_table()
{
}


//-------------------------------------------------
//  map_range - map a specific entry in the address
//  map
//-------------------------------------------------

void address_table::map_range(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u16 entry)
{
	// convert addresses to bytes
	m_space.adjust_addresses(addrstart, addrend, addrmask, addrmirror);

	// validity checks
	assert_always(addrstart <= addrend, "address_table::map_range called with start greater than end");
	assert_always((addrstart & (m_space.alignment() - 1)) == 0, "address_table::map_range called with misaligned start address");
	assert_always((addrend & (m_space.alignment() - 1)) == (m_space.alignment() - 1), "address_table::map_range called with misaligned end address");

	// configure the entry to our parameters (but not for static non-banked cases)
	handler_entry &curentry = handler(entry);
	if (entry <= STATIC_BANKMAX || entry >= STATIC_COUNT)
		curentry.configure(addrstart, addrend, addrmask, m_space.address_to_byte_end(addrmask));

	// populate it
	populate_range_mirrored(addrstart, addrend, addrmirror, entry);

	// recompute any cache access on this space if it is a read modification
	m_space.invalidate_read_caches(entry);

	//  verify_reference_counts();
}

u16 address_table::get_free_handler()
{
	if (handler_free == STATIC_INVALID)
		throw emu_fatalerror("Out of handler entries in address table");

	u16 handler = handler_free;
	handler_free = handler_next_free[handler - STATIC_COUNT];
	return handler;
}


//-------------------------------------------------
//  setup_range - finds an appropriate handler entry
//  and requests to populate the address map with
//  it
//-------------------------------------------------

std::list<u32> address_table::setup_range(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask)
{
	// Careful, you can't shift by 64 or more
	u64 testmask = (1ULL << (m_space.data_width()-1) << 1) - 1;

	if ((umask & testmask) == 0 || (umask & testmask) == testmask || range_simply_masks(addrstart, addrend, addrmask, addrmirror, umask))
		return setup_range_solid(addrstart, addrend, addrmask, addrmirror);
	else
		return setup_range_masked(addrstart, addrend, addrmask, addrmirror, umask);
}

//-------------------------------------------------
//  range_simply_masks - determine if a handler
//  can fully override part of one existing range
//  (speeds up mapping in heavily mirrored cases)
//-------------------------------------------------

bool address_table::range_simply_masks(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask)
{
	// convert addresses to bytes
	m_space.adjust_addresses(addrstart, addrend, addrmask, addrmirror);

	// Validity checks
	assert_always(addrstart <= addrend, "address_table::setup_range called with start greater than end");
	assert_always((addrstart & (m_space.alignment() - 1)) == 0, "address_table::setup_range called with misaligned start address");
	assert_always((addrend & (m_space.alignment() - 1)) == (m_space.alignment() - 1), "address_table::setup_range called with misaligned end address");

	offs_t range_start, range_end;
	u16 entry = derive_range(addrstart, range_start, range_end);

	return range_end >= (addrend | addrmirror) && (entry < STATIC_COUNT || handler(entry).overriden_by_mask(umask));
}

//-------------------------------------------------
//  setup_range_solid - finds an appropriate handler
//  entry and requests to populate the address map with
//  it.  Replace what's there.
//-------------------------------------------------

std::list<u32> address_table::setup_range_solid(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror)
{
	std::list<u32> entries;

	// Grab a free entry
	u16 entry = get_free_handler();

	// Add it in the "to be setup" list
	entries.push_back(entry);

	// Configure and map it
	map_range(addrstart, addrend, addrmask, addrmirror, entry);

	return entries;
}

//-------------------------------------------------
//  setup_range_masked - finds an appropriate handler
//  entry and requests to populate the address map with
//  it.  Handle non-overlapping subunits.
//-------------------------------------------------

namespace {
	struct subrange {
		offs_t start, end;
		subrange(offs_t _start, offs_t _end) : start(_start), end(_end) {}
	};
}

std::list<u32> address_table::setup_range_masked(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, u64 umask)
{
	std::list<u32> entries;

	// convert addresses to bytes
	m_space.adjust_addresses(addrstart, addrend, addrmask, addrmirror);

	// Validity checks
	assert_always(addrstart <= addrend, "address_table::setup_range called with start greater than end");
	assert_always((addrstart & (m_space.alignment() - 1)) == 0, "address_table::setup_range called with misaligned start address");
	assert_always((addrend & (m_space.alignment() - 1)) == (m_space.alignment() - 1), "address_table::setup_range called with misaligned end address");

	offs_t bytemask = m_space.address_to_byte_end(addrmask);

	// Scan the memory to see what has to be done
	std::list<subrange> range_override;
	std::map<u16, std::list<subrange> > range_partial;

	offs_t base_mirror = 0;
	do
	{
		offs_t base_address = base_mirror | addrstart;
		offs_t end_address  = base_mirror | addrend;

		do
		{
			offs_t range_start, range_end;
			u16 entry = derive_range(base_address, range_start, range_end);
			u32 stop_address = std::min(range_end, end_address);

			if (entry < STATIC_COUNT || handler(entry).overriden_by_mask(umask))
				range_override.push_back(subrange(base_address, stop_address));
			else
				range_partial[entry].push_back(subrange(base_address, stop_address));

			base_address = stop_address + 1;
		}
		while (base_address != end_address + 1);

		// Efficient method to go to the next range start given a mirroring mask
		base_mirror = (base_mirror + 1 + ~addrmirror) & addrmirror;
	}
	while (base_mirror);

	// Ranges in range_override must be plain replaced by the new handler
	if (!range_override.empty())
	{
		// Grab a free entry
		u16 entry = get_free_handler();

		// configure the entry to our parameters
		handler_entry &curentry = handler(entry);
		curentry.configure(addrstart, addrend, addrmask, bytemask);

		// Populate it wherever needed
		for (std::list<subrange>::const_iterator i = range_override.begin(); i != range_override.end(); ++i)
			populate_range(i->start, i->end, entry);

		// Add it in the "to be setup" list
		entries.push_back(entry);

		// recompute any cache access on this space if it is a read modification
		m_space.invalidate_read_caches(entry);
	}

	// Ranges in range_partial must be duplicated then partially changed
	if (!range_partial.empty())
	{
		for (std::map<u16, std::list<subrange> >::const_iterator i = range_partial.begin(); i != range_partial.end(); ++i)
		{
			// Theorically, if the handler to change matches the
			// characteristics of ours, we can directly change it.  In
			// practice, it's more complex than that because the
			// mirroring is not saved, so we're not sure there aren't
			// mappings on the handler outside of the zones we're
			// supposed to change.  So we won't do the obvious
			// optimization at this point.

			// Get the original handler
			handler_entry *base_entry = &handler(i->first);

			offs_t previous_bytemask = base_entry->bytemask();

			// Grab a new handler and copy it there
			u16 entry = get_free_handler();
			handler_entry &curentry = handler(entry);
			curentry.copy(base_entry);

			// Clear the colliding entries
			curentry.clear_conflicting_subunits(umask);

			// Reconfigure the base addresses
			curentry.configure(addrstart, addrend, addrmask, bytemask);

			// Populate it wherever needed
			for (const auto & elem : i->second)
				populate_range(elem.start, elem.end, entry);

			curentry.expand_bytemask(previous_bytemask);

			// Add it in the "to be setup" list
			entries.push_back(entry);

			// recompute any cache access on this space if it is a read modification
			m_space.invalidate_read_caches(entry);
		}
	}

	//  verify_reference_counts();
	return entries;
}


//-------------------------------------------------
//  verify_reference_counts - check how much of a
//  hash we've made of things
//-------------------------------------------------

void address_table::verify_reference_counts()
{
	int actual_refcounts[SUBTABLE_BASE-STATIC_COUNT];
	memset(actual_refcounts, 0, sizeof(actual_refcounts));

	bool subtable_seen[TOTAL_MEMORY_BANKS - SUBTABLE_BASE];
	memset(subtable_seen, 0, sizeof(subtable_seen));

	for (int level1 = 0; level1 != 1 << LEVEL1_BITS; level1++)
	{
		u16 l1_entry = m_table[level1];
		if (l1_entry >= SUBTABLE_BASE)
		{
			assert(m_large);
			if (subtable_seen[l1_entry - SUBTABLE_BASE])
				continue;

			subtable_seen[l1_entry - SUBTABLE_BASE] = true;
			const u16 *subtable = subtable_ptr(l1_entry);
			for (int level2 = 0; level2 != 1 << LEVEL2_BITS; level2++)
			{
				u16 l2_entry = subtable[level2];
				assert(l2_entry < SUBTABLE_BASE);
				if (l2_entry >= STATIC_COUNT)
					actual_refcounts[l2_entry - STATIC_COUNT]++;
			}
		}
		else if (l1_entry >= STATIC_COUNT)
			actual_refcounts[l1_entry - STATIC_COUNT]++;
	}

	if (memcmp(actual_refcounts, handler_refcount, sizeof(handler_refcount)))
	{
		osd_printf_error("Refcount failure:\n");
		for(int i = STATIC_COUNT; i != SUBTABLE_BASE; i++)
			osd_printf_error("%02x: %4x .. %4x\n", i, handler_refcount[i-STATIC_COUNT], actual_refcounts[i-STATIC_COUNT]);
		throw emu_fatalerror("memory.c: refcounts are fucked.\n");
	}
}


//-------------------------------------------------
//  populate_range - assign a memory handler to a
//  range of addresses
//-------------------------------------------------

void address_table::populate_range(offs_t addrstart, offs_t addrend, u16 handlerindex)
{
	offs_t l2mask = (1 << level2_bits()) - 1;
	offs_t l1start = addrstart >> level2_bits();
	offs_t l2start = addrstart & l2mask;
	offs_t l1stop = addrend >> level2_bits();
	offs_t l2stop = addrend & l2mask;

	// sanity check
	if (addrstart > addrend)
		return;

	// handle the starting edge if it's not on a block boundary
	if (l2start != 0)
	{
		u16 *subtable = subtable_open(l1start);

		// if the start and stop end within the same block, handle that
		if (l1start == l1stop)
		{
			handler_ref(handlerindex, l2stop-l2start+1);
			for (int i = l2start; i <= l2stop; i++)
			{
				handler_unref(subtable[i]);
				subtable[i] = handlerindex;
			}
			subtable_close(l1start);
			return;
		}

		// otherwise, fill until the end
		handler_ref(handlerindex, l2mask - l2start + 1);
		for (int i = l2start; i <= l2mask; i++)
		{
			handler_unref(subtable[i]);
			subtable[i] = handlerindex;
		}
		subtable_close(l1start);
		if (l1start != (offs_t)~0)
			l1start++;
	}

	// handle the trailing edge if it's not on a block boundary
	if (l2stop != l2mask)
	{
		u16 *subtable = subtable_open(l1stop);

		// fill from the beginning
		handler_ref(handlerindex, l2stop+1);
		for (int i = 0; i <= l2stop; i++)
		{
			handler_unref(subtable[i]);
			subtable[i] = handlerindex;
		}
		subtable_close(l1stop);

		// if the start and stop end within the same block, handle that
		if (l1start == l1stop)
			return;
		if (l1stop != 0)
			l1stop--;
	}

	// now fill in the middle tables
	handler_ref(handlerindex, l1stop - l1start + 1);
	for (offs_t l1index = l1start; l1index <= l1stop; l1index++)
	{
		u16 subindex = m_table[l1index];

		// if we have a subtable here, release it
		if (subindex >= SUBTABLE_BASE)
			subtable_release(subindex);
		else
			handler_unref(subindex);
		m_table[l1index] = handlerindex;
	}
}


//-------------------------------------------------
//  populate_range_mirrored - assign a memory
//  handler to a range of addresses including
//  mirrors
//-------------------------------------------------

void address_table::populate_range_mirrored(offs_t addrstart, offs_t addrend, offs_t addrmirror, u16 handlerindex)
{
	// determine the mirror bits
	offs_t lmirrorbits = 0;
	offs_t lmirrorbit[32];
	for (int bit = 0; bit < level2_bits(); bit++)
		if (addrmirror & (1 << bit))
			lmirrorbit[lmirrorbits++] = 1 << bit;

	offs_t hmirrorbits = 0;
	offs_t hmirrorbit[32];
	for (int bit = level2_bits(); bit < 32; bit++)
		if (addrmirror & (1 << bit))
			hmirrorbit[hmirrorbits++] = 1 << bit;

	// loop over mirrors in the level 2 table
	u16 prev_entry = STATIC_INVALID;
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
			m_space.invalidate_read_caches(addrstart + lmirrorbase, addrend + lmirrorbase);
		}

		// if this is not our first time through, and the level 2 entry matches the previous
		// level 2 entry, just do a quick map and get out; note that this only works for entries
		// which don't span multiple level 1 table entries
		int cur_index = level1_index(addrstart + hmirrorbase);
		if (cur_index == level1_index(addrend + hmirrorbase))
		{
			if (hmirrorcount != 0 && prev_entry == m_table[cur_index])
			{
				VPRINTF(("Quick mapping subtable at %08X to match subtable at %08X\n", cur_index << level2_bits(), prev_index << level2_bits()));

				// release the subtable if the old value was a subtable
				if (m_table[cur_index] >= SUBTABLE_BASE)
					subtable_release(m_table[cur_index]);
				else
					handler_unref(m_table[cur_index]);

				// reallocate the subtable if the new value is a subtable
				if (m_table[prev_index] >= SUBTABLE_BASE)
					subtable_realloc(m_table[prev_index]);
				else
					handler_ref(m_table[prev_index], 1);

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
			populate_range(addrstart + lmirrorbase, addrend + lmirrorbase, handlerindex);
		}
	}
}


//-------------------------------------------------
//  derive_range - look up the entry for a memory
//  range, and then compute the extent of that
//  range based on the lookup tables
//-------------------------------------------------

u16 address_table::derive_range(offs_t address, offs_t &addrstart, offs_t &addrend) const
{
	// look up the initial address to get the entry we care about
	u16 l1entry;
	u16 entry = l1entry = m_table[level1_index(address)];
	if (l1entry >= SUBTABLE_BASE)
		entry = m_table[level2_index(l1entry, address)];

	// use the addrmask of the entry to set minimum and maximum bounds
	offs_t minscan, maxscan;
	handler(entry).mirrored_start_end(address, minscan, maxscan);

	// first scan backwards to find the start address
	u16 curl1entry = l1entry;
	u16 curentry = entry;
	addrstart = address;
	while (1)
	{
		// if we need to scan the subtable, do it
		if (curentry != curl1entry)
		{
			u32 minindex = level2_index(curl1entry, 0);
			u32 index;

			// scan backwards from the current address, until the previous entry doesn't match
			for (index = level2_index(curl1entry, addrstart); index > minindex; index--, addrstart -= 1)
				if (m_table[index - 1] != entry)
					break;

			// if we didn't hit the beginning, then we're finished scanning
			if (index != minindex)
				break;
		}

		// move to the beginning of this L1 entry; stop at the minimum address
		addrstart &= ~((1 << level2_bits()) - 1);
		if (addrstart <= minscan)
			break;

		// look up the entry of the byte at the end of the previous L1 entry; if it doesn't match, stop
		curentry = curl1entry = m_table[level1_index(addrstart - 1)];
		if (curl1entry >= SUBTABLE_BASE)
			curentry = m_table[level2_index(curl1entry, addrstart - 1)];
		if (curentry != entry)
			break;

		// move into the previous entry and resume searching
		addrstart -= 1;
	}

	// then scan forwards to find the end address
	curl1entry = l1entry;
	curentry = entry;
	addrend = address;
	while (1)
	{
		// if we need to scan the subtable, do it
		if (curentry != curl1entry)
		{
			u32 maxindex = level2_index(curl1entry, ~0);
			u32 index;

			// scan forwards from the current address, until the next entry doesn't match
			for (index = level2_index(curl1entry, addrend); index < maxindex; index++, addrend += 1)
				if (m_table[index + 1] != entry)
					break;

			// if we didn't hit the end, then we're finished scanning
			if (index != maxindex)
				break;
		}

		// move to the end of this L1 entry; stop at the maximum address
		addrend |= (1 << level2_bits()) - 1;
		if (addrend >= maxscan)
			break;

		// look up the entry of the byte at the start of the next L1 entry; if it doesn't match, stop
		curentry = curl1entry = m_table[level1_index(addrend + 1)];
		if (curl1entry >= SUBTABLE_BASE)
			curentry = m_table[level2_index(curl1entry, addrend + 1)];
		if (curentry != entry)
			break;

		// move into the next entry and resume searching
		addrend += 1;
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

u16 address_table::subtable_alloc()
{
	// loop
	while (1)
	{
		// find a subtable with a usecount of 0
		for (u16 subindex = 0; subindex < SUBTABLE_COUNT; subindex++)
			if (m_subtable[subindex].m_usecount == 0)
			{
				// if this is past our allocation budget, allocate some more
				if (subindex >= m_subtable_alloc)
				{
					m_subtable_alloc += SUBTABLE_ALLOC;
					u32 newsize = (1 << LEVEL1_BITS) + (m_subtable_alloc << level2_bits());

					bool was_live = (m_live_lookup == &m_table[0]);
					int oldsize = m_table.size();
					m_table.resize(newsize);
					memset(&m_table[oldsize], 0, (newsize-oldsize)*sizeof(m_table[0]));
					if (was_live)
						m_live_lookup = &m_table[0];
				}
				// bump the usecount and return
				m_subtable[subindex].m_usecount++;
				return subindex + SUBTABLE_BASE;
			}

		// merge any subtables we can
		if (!subtable_merge())
			fatalerror("Ran out of subtables!\n");
	}
}


//-------------------------------------------------
//  subtable_realloc - increment the usecount on
//  a subtable
//-------------------------------------------------

void address_table::subtable_realloc(u16 subentry)
{
	u16 subindex = subentry - SUBTABLE_BASE;

	// sanity check
	if (m_subtable[subindex].m_usecount <= 0)
		fatalerror("Called subtable_realloc on a table with a usecount of 0\n");

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
	u16 subindex;

	VPRINTF(("Merging subtables....\n"));

	// okay, we failed; update all the checksums and merge tables
	for (subindex = 0; subindex < SUBTABLE_COUNT; subindex++)
		if (!m_subtable[subindex].m_checksum_valid && m_subtable[subindex].m_usecount != 0)
		{
			u32 *subtable = reinterpret_cast<u32 *>(subtable_ptr(subindex + SUBTABLE_BASE));
			u32 checksum = 0;

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
			u16 *subtable = subtable_ptr(subindex + SUBTABLE_BASE);
			u32 checksum = m_subtable[subindex].m_checksum;
			u16 sumindex;

			for (sumindex = subindex + 1; sumindex < SUBTABLE_COUNT; sumindex++)
				if (m_subtable[sumindex].m_usecount != 0 &&
					m_subtable[sumindex].m_checksum == checksum &&
					!memcmp(subtable, subtable_ptr(sumindex + SUBTABLE_BASE), 2*(1 << level2_bits())))
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

void address_table::subtable_release(u16 subentry)
{
	u16 subindex = subentry - SUBTABLE_BASE;
	// sanity check
	if (m_subtable[subindex].m_usecount <= 0)
		fatalerror("Called subtable_release on a table with a usecount of 0\n");

	// decrement the usecount and clear the checksum if we're at 0
	// also unref the subhandlers
	m_subtable[subindex].m_usecount--;
	if (m_subtable[subindex].m_usecount == 0)
	{
		m_subtable[subindex].m_checksum = 0;
		u16 *subtable = subtable_ptr(subentry);
		for (int i = 0; i < (1 << LEVEL2_BITS); i++)
			handler_unref(subtable[i]);
	}
}


//-------------------------------------------------
//  subtable_open - gain access to a subtable for
//  modification
//-------------------------------------------------

u16 *address_table::subtable_open(offs_t l1index)
{
	u16 subentry = m_table[l1index];

	// if we don't have a subtable yet, allocate a new one
	if (subentry < SUBTABLE_BASE)
	{
		int size = 1 << level2_bits();
		u16 newentry = subtable_alloc();
		handler_ref(subentry, size-1);
		u16 *subptr = subtable_ptr(newentry);
		for (int i=0; i<size; i++)
			subptr[i] = subentry;
		m_table[l1index] = newentry;
		u32 subkey = subentry + (subentry << 8) + (subentry << 16) + (subentry << 24);
		m_subtable[newentry - SUBTABLE_BASE].m_checksum = subkey * (((1 << level2_bits())/4));
		subentry = newentry;
	}

	// if we're sharing this subtable, we also need to allocate a fresh copy
	else if (m_subtable[subentry - SUBTABLE_BASE].m_usecount > 1)
	{
		u16 newentry = subtable_alloc();

		// allocate may cause some additional merging -- look up the subentry again
		// when we're done; it should still require a split
		subentry = m_table[l1index];
		assert(subentry >= SUBTABLE_BASE);
		assert(m_subtable[subentry - SUBTABLE_BASE].m_usecount > 1);

		int size = 1 << level2_bits();
		u16 *src = subtable_ptr(subentry);
		for(int i=0; i != size; i++)
			handler_ref(src[i], 1);

		memcpy(subtable_ptr(newentry), src, 2*size);
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

const char *address_table::handler_name(u16 entry) const
{
	// banks have names
	if (entry >= STATIC_BANK1 && entry <= STATIC_BANKMAX)
		for (auto &info : m_space.m_manager.banks())
			if (info.second->index() == entry)
				return info.second->name();

	// constant strings for static entries
	if (entry == STATIC_INVALID) return "invalid";
	if (entry == STATIC_NOP) return "nop";
	if (entry == STATIC_UNMAP) return "unmapped";
	if (entry == STATIC_WATCHPOINT) return "watchpoint";

	static char desc[4096];
	handler(entry).description(desc);
	if (desc[0])
		return desc;
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
		u8 **bankptr = (entrynum >= STATIC_BANK1 && entrynum <= STATIC_BANKMAX) ? space.m_manager.bank_pointer_addr(entrynum) : nullptr;
		m_handlers[entrynum] = std::make_unique<handler_entry_read>(space.data_width(), space.endianness(), bankptr);
	}

	// we have to allocate different object types based on the data bus width
	switch (space.data_width())
	{
		// 8-bit case
		case 8:
			m_handlers[STATIC_UNMAP]->set_delegate(read8_delegate(FUNC(address_table_read::unmap_r<u8>), this));
			m_handlers[STATIC_NOP]->set_delegate(read8_delegate(FUNC(address_table_read::nop_r<u8>), this));
			switch (space.addr_shift())
			{
				case  0: m_handlers[STATIC_WATCHPOINT]->set_delegate(read8_delegate (&address_table_read::watchpoint_r<u8,   0>, "watchpoint_r", this)); break;
				default: abort();
			}
			break;

		// 16-bit case
		case 16:
			m_handlers[STATIC_UNMAP]->set_delegate(read16_delegate(FUNC(address_table_read::unmap_r<u16>), this));
			m_handlers[STATIC_NOP]->set_delegate(read16_delegate(FUNC(address_table_read::nop_r<u16>), this));
			switch (space.addr_shift())
			{
				case -1: m_handlers[STATIC_WATCHPOINT]->set_delegate(read16_delegate(&address_table_read::watchpoint_r<u16, -1>, "watchpoint_r", this)); break;
				case  0: m_handlers[STATIC_WATCHPOINT]->set_delegate(read16_delegate(&address_table_read::watchpoint_r<u16,  0>, "watchpoint_r", this)); break;
				case  3: m_handlers[STATIC_WATCHPOINT]->set_delegate(read16_delegate(&address_table_read::watchpoint_r<u16,  3>, "watchpoint_r", this)); break;
				default: abort();
			}
			break;

		// 32-bit case
		case 32:
			m_handlers[STATIC_UNMAP]->set_delegate(read32_delegate(FUNC(address_table_read::unmap_r<u32>), this));
			m_handlers[STATIC_NOP]->set_delegate(read32_delegate(FUNC(address_table_read::nop_r<u32>), this));
			switch (space.addr_shift())
			{
				case -2: m_handlers[STATIC_WATCHPOINT]->set_delegate(read32_delegate(&address_table_read::watchpoint_r<u32, -2>, "watchpoint_r", this)); break;
				case -1: m_handlers[STATIC_WATCHPOINT]->set_delegate(read32_delegate(&address_table_read::watchpoint_r<u32, -1>, "watchpoint_r", this)); break;
				case  0: m_handlers[STATIC_WATCHPOINT]->set_delegate(read32_delegate(&address_table_read::watchpoint_r<u32,  0>, "watchpoint_r", this)); break;
				default: abort();
			}
			break;

		// 64-bit case
		case 64:
			m_handlers[STATIC_UNMAP]->set_delegate(read64_delegate(FUNC(address_table_read::unmap_r<u64>), this));
			m_handlers[STATIC_NOP]->set_delegate(read64_delegate(FUNC(address_table_read::nop_r<u64>), this));
			switch (space.addr_shift())
			{
				case -3: m_handlers[STATIC_WATCHPOINT]->set_delegate(read64_delegate(&address_table_read::watchpoint_r<u64, -3>, "watchpoint_r", this)); break;
				case -2: m_handlers[STATIC_WATCHPOINT]->set_delegate(read64_delegate(&address_table_read::watchpoint_r<u64, -2>, "watchpoint_r", this)); break;
				case -1: m_handlers[STATIC_WATCHPOINT]->set_delegate(read64_delegate(&address_table_read::watchpoint_r<u64, -1>, "watchpoint_r", this)); break;
				case  0: m_handlers[STATIC_WATCHPOINT]->set_delegate(read64_delegate(&address_table_read::watchpoint_r<u64,  0>, "watchpoint_r", this)); break;
				default: abort();
			}
			break;
	}

	// reset the byte masks on the special handlers to open up the full address space for proper reporting
	m_handlers[STATIC_UNMAP]->configure(0, space.addrmask(), ~0, ~0);
	m_handlers[STATIC_NOP]->configure(0, space.addrmask(), ~0, ~0);
	m_handlers[STATIC_WATCHPOINT]->configure(0, space.addrmask(), ~0, ~0);
}


//-------------------------------------------------
//  address_table_read - destructor
//-------------------------------------------------

address_table_read::~address_table_read()
{
}


//-------------------------------------------------
//  handler - return the generic handler entry for
//  this index
//-------------------------------------------------

handler_entry &address_table_read::handler(u32 index) const
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
		u8 **bankptr = (entrynum >= STATIC_BANK1 && entrynum <= STATIC_BANKMAX) ? space.m_manager.bank_pointer_addr(entrynum) : nullptr;
		m_handlers[entrynum] = std::make_unique<handler_entry_write>(space.data_width(), space.endianness(), bankptr);
	}

	// we have to allocate different object types based on the data bus width
	switch (space.data_width())
	{
		// 8-bit case
		case 8:
			m_handlers[STATIC_UNMAP]->set_delegate(write8_delegate(FUNC(address_table_write::unmap_w<u8>), this));
			m_handlers[STATIC_NOP]->set_delegate(write8_delegate(FUNC(address_table_write::nop_w<u8>), this));
			switch (space.addr_shift())
			{
				case  0: m_handlers[STATIC_WATCHPOINT]->set_delegate(write8_delegate (&address_table_write::watchpoint_w<u8,   0>, "watchpoint_w", this)); break;
				default: abort();
			}
			break;

		// 16-bit case
		case 16:
			m_handlers[STATIC_UNMAP]->set_delegate(write16_delegate(FUNC(address_table_write::unmap_w<u16>), this));
			m_handlers[STATIC_NOP]->set_delegate(write16_delegate(FUNC(address_table_write::nop_w<u16>), this));
			switch (space.addr_shift())
			{
				case -1: m_handlers[STATIC_WATCHPOINT]->set_delegate(write16_delegate(&address_table_write::watchpoint_w<u16, -1>, "watchpoint_w", this)); break;
				case  0: m_handlers[STATIC_WATCHPOINT]->set_delegate(write16_delegate(&address_table_write::watchpoint_w<u16,  0>, "watchpoint_w", this)); break;
				case  3: m_handlers[STATIC_WATCHPOINT]->set_delegate(write16_delegate(&address_table_write::watchpoint_w<u16,  3>, "watchpoint_w", this)); break;
				default: abort();
			}
			break;

		// 32-bit case
		case 32:
			m_handlers[STATIC_UNMAP]->set_delegate(write32_delegate(FUNC(address_table_write::unmap_w<u32>), this));
			m_handlers[STATIC_NOP]->set_delegate(write32_delegate(FUNC(address_table_write::nop_w<u32>), this));
			switch (space.addr_shift())
			{
				case -2: m_handlers[STATIC_WATCHPOINT]->set_delegate(write32_delegate(&address_table_write::watchpoint_w<u32, -2>, "watchpoint_w", this)); break;
				case -1: m_handlers[STATIC_WATCHPOINT]->set_delegate(write32_delegate(&address_table_write::watchpoint_w<u32, -1>, "watchpoint_w", this)); break;
				case  0: m_handlers[STATIC_WATCHPOINT]->set_delegate(write32_delegate(&address_table_write::watchpoint_w<u32,  0>, "watchpoint_w", this)); break;
				default: abort();
			}
			break;

		// 64-bit case
		case 64:
			m_handlers[STATIC_UNMAP]->set_delegate(write64_delegate(FUNC(address_table_write::unmap_w<u64>), this));
			m_handlers[STATIC_NOP]->set_delegate(write64_delegate(FUNC(address_table_write::nop_w<u64>), this));
			switch (space.addr_shift())
			{
				case -3: m_handlers[STATIC_WATCHPOINT]->set_delegate(write64_delegate(&address_table_write::watchpoint_w<u64, -3>, "watchpoint_w", this)); break;
				case -2: m_handlers[STATIC_WATCHPOINT]->set_delegate(write64_delegate(&address_table_write::watchpoint_w<u64, -2>, "watchpoint_w", this)); break;
				case -1: m_handlers[STATIC_WATCHPOINT]->set_delegate(write64_delegate(&address_table_write::watchpoint_w<u64, -1>, "watchpoint_w", this)); break;
				case  0: m_handlers[STATIC_WATCHPOINT]->set_delegate(write64_delegate(&address_table_write::watchpoint_w<u64,  0>, "watchpoint_w", this)); break;
				default: abort();
			}
			break;
	}

	// reset the byte masks on the special handlers to open up the full address space for proper reporting
	m_handlers[STATIC_UNMAP]->configure(0, space.addrmask(), ~0, ~0);
	m_handlers[STATIC_NOP]->configure(0, space.addrmask(), ~0, ~0);
	m_handlers[STATIC_WATCHPOINT]->configure(0, space.addrmask(), ~0, ~0);
}


//-------------------------------------------------
//  address_table_write - destructor
//-------------------------------------------------

address_table_write::~address_table_write()
{
}


//-------------------------------------------------
//  handler - return the generic handler entry for
//  this index
//-------------------------------------------------

handler_entry &address_table_write::handler(u32 index) const
{
	assert(index < ARRAY_LENGTH(m_handlers));
	return *m_handlers[index];
}



//**************************************************************************
//  CACHE MEMORY RANGES
//**************************************************************************

//-------------------------------------------------
//  memory_access_cache - constructor
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> memory_access_cache<Width, AddrShift, Endian>::memory_access_cache(address_space &space)
	: m_space(space),
		m_ptr(nullptr),
		m_addrmask(space.addrmask()),
		m_addrstart(1),
		m_addrend(0),
		m_entry(STATIC_UNMAP)
{
}


//-------------------------------------------------
//  ~memory_access_cache - destructor
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> memory_access_cache<Width, AddrShift, Endian>::~memory_access_cache()
{
}


//-------------------------------------------------
//  set_cache_region - called by device cores to
//  update the opcode base for the given address
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> bool memory_access_cache<Width, AddrShift, Endian>::set_cache_region(offs_t address)
{
	// find or allocate a matching range
	cache_range *range = find_range(address, m_entry);

	// if we don't map to a bank, return false
	if (m_entry < STATIC_BANK1 || m_entry > STATIC_BANKMAX)
	{
		// ensure future updates to land here as well until we get back into a bank
		m_addrend = 0;
		m_addrstart = 1;
		return false;
	}

	u8 *base = *m_space.m_manager.bank_pointer_addr(m_entry);

	// compute the adjusted base
	offs_t maskedbits = address & ~m_space.addrmask();
	const handler_entry_read &handler = m_space.read().handler_read(m_entry);
	m_addrmask = handler.addrmask();
	u32 delta = handler.addrstart() & m_addrmask;
	if(AddrShift < 0)
		delta = delta << iabs(AddrShift);
	else if(AddrShift > 0)
		delta = delta >> iabs(AddrShift);

	m_ptr = base - delta;
	m_addrstart = maskedbits | range->m_addrstart;
	m_addrend = maskedbits | range->m_addrend;
	return true;
}


//-------------------------------------------------
//  find_range - find a byte address in a range
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> typename memory_access_cache<Width, AddrShift, Endian>::cache_range *memory_access_cache<Width, AddrShift, Endian>::find_range(offs_t address, u16 &entry)
{
	// determine which entry
	address &= m_space.m_addrmask;
	entry = m_space.read().lookup_live_nowp(address);

	// scan our table
	for (auto &range : m_rangelist[entry])
		if (address >= range.m_addrstart && address <= range.m_addrend)
			return &range;

	// didn't find out; create a new one
	cache_range range;
	m_space.read().derive_range(address, range.m_addrstart, range.m_addrend);
	m_rangelist[entry].push_front(range);

	return &m_rangelist[entry].front();
}


//-------------------------------------------------
//  remove_intersecting_ranges - remove all cached
//  ranges that intersect the given address range
//-------------------------------------------------

template<int Width, int AddrShift, int Endian> void memory_access_cache<Width, AddrShift, Endian>::remove_intersecting_ranges(offs_t addrstart, offs_t addrend)
{
	// loop over all entries
	for (auto & elem : m_rangelist)
	{
		// loop over all ranges in this entry's list
		for (auto range = elem.begin(); range!=elem.end();)
		{
			// if we intersect, remove
			if (addrstart <= range->m_addrend && addrend >= range->m_addrstart)
				range = elem.erase(range);
			else
				range ++;
		}
	}
}

template class memory_access_cache<0,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<0,  0, ENDIANNESS_BIG>;
template class memory_access_cache<1,  3, ENDIANNESS_LITTLE>;
template class memory_access_cache<1,  3, ENDIANNESS_BIG>;
template class memory_access_cache<1,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<1,  0, ENDIANNESS_BIG>;
template class memory_access_cache<1, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<1, -1, ENDIANNESS_BIG>;
template class memory_access_cache<2,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<2,  0, ENDIANNESS_BIG>;
template class memory_access_cache<2, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<2, -1, ENDIANNESS_BIG>;
template class memory_access_cache<2, -2, ENDIANNESS_LITTLE>;
template class memory_access_cache<2, -2, ENDIANNESS_BIG>;
template class memory_access_cache<3,  0, ENDIANNESS_LITTLE>;
template class memory_access_cache<3,  0, ENDIANNESS_BIG>;
template class memory_access_cache<3, -1, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -1, ENDIANNESS_BIG>;
template class memory_access_cache<3, -2, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -2, ENDIANNESS_BIG>;
template class memory_access_cache<3, -3, ENDIANNESS_LITTLE>;
template class memory_access_cache<3, -3, ENDIANNESS_BIG>;



//**************************************************************************
//  MEMORY BLOCK
//**************************************************************************

//-------------------------------------------------
//  memory_block - constructor
//-------------------------------------------------

memory_block::memory_block(address_space &space, offs_t addrstart, offs_t addrend, void *memory)
	: m_machine(space.m_manager.machine()),
		m_space(space),
		m_addrstart(addrstart),
		m_addrend(addrend),
		m_data(reinterpret_cast<u8 *>(memory))
{
	offs_t const length = space.address_to_byte(addrend + 1 - addrstart);
	VPRINTF(("block_allocate('%s',%s,%08X,%08X,%p)\n", space.device().tag(), space.name(), addrstart, addrend, memory));

	// allocate a block if needed
	if (m_data == nullptr)
	{
		if (length < 4096)
		{
			m_allocated.resize(length);
			memset(&m_allocated[0], 0, length);
			m_data = &m_allocated[0];
		}
		else
		{
			m_allocated.resize(length + 0xfff);
			memset(&m_allocated[0], 0, length + 0xfff);
			m_data = reinterpret_cast<u8 *>((reinterpret_cast<uintptr_t>(&m_allocated[0]) + 0xfff) & ~0xfff);
		}
	}

	// register for saving, but only if we're not part of a memory region
	if (space.m_manager.region_containing(m_data, length) != nullptr)
		VPRINTF(("skipping save of this memory block as it is covered by a memory region\n"));
	else
	{
		int bytes_per_element = space.data_width() / 8;
		std::string name = string_format("%08x-%08x", addrstart, addrend);
		machine().save().save_memory(&space.device(), "memory", space.device().tag(), space.spacenum(), name.c_str(), m_data, bytes_per_element, (u32)length / bytes_per_element);
	}
}


//-------------------------------------------------
//  memory_block - destructor
//-------------------------------------------------

memory_block::~memory_block()
{
}



//**************************************************************************
//  MEMORY BANK
//**************************************************************************

//-------------------------------------------------
//  memory_bank - constructor
//-------------------------------------------------

memory_bank::memory_bank(address_space &space, int index, offs_t addrstart, offs_t addrend, const char *tag)
	: m_machine(space.m_manager.machine()),
		m_baseptr(space.m_manager.bank_pointer_addr(index)),
		m_index(index),
		m_anonymous(tag == nullptr),
		m_addrstart(addrstart),
		m_addrend(addrend),
		m_curentry(BANK_ENTRY_UNSPECIFIED)
{
	// generate an internal tag if we don't have one
	if (tag == nullptr)
	{
		m_tag = string_format("~%d~", index);
		m_name = string_format("Internal bank #%d", index);
	}
	else
	{
		m_tag.assign(tag);
		m_name = string_format("Bank '%s'", tag);
	}

	if (!m_anonymous && machine().save().registration_allowed())
		machine().save().save_item(&space.device(), "memory", m_tag.c_str(), 0, NAME(m_curentry));
}


//-------------------------------------------------
//  memory_bank - destructor
//-------------------------------------------------

memory_bank::~memory_bank()
{
}


//-------------------------------------------------
//  references_space - walk the list of references
//  to find a match against the provided space
//  and read/write
//-------------------------------------------------

bool memory_bank::references_space(const address_space &space, read_or_write readorwrite) const
{
	for (auto &ref : m_reflist)
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
	m_reflist.push_back(std::make_unique<bank_reference>(space, readorwrite));
}


//-------------------------------------------------
//  invalidate_references - force updates on all
//  referencing address spaces
//-------------------------------------------------

void memory_bank::invalidate_references()
{
	// invalidate all the cache references to any referenced address spaces
	for (auto &ref : m_reflist)
		ref->space().invalidate_read_caches();
}


//-------------------------------------------------
//  set_base - set the bank base explicitly
//-------------------------------------------------

void memory_bank::set_base(void *base)
{
	// nullptr is not an option
	if (base == nullptr)
		throw emu_fatalerror("memory_bank::set_base called nullptr base");

	// set the base and invalidate any referencing spaces
	*m_baseptr = reinterpret_cast<u8 *>(base);
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
	if (entrynum < 0 || entrynum >= int(m_entry.size()))
		throw emu_fatalerror("memory_bank::set_entry called with out-of-range entry %d", entrynum);
	if (m_entry[entrynum].m_ptr == nullptr)
		throw emu_fatalerror("memory_bank::set_entry called for bank '%s' with invalid bank entry %d", m_tag.c_str(), entrynum);

	m_curentry = entrynum;
	*m_baseptr = m_entry[entrynum].m_ptr;

	// invalidate referencing spaces
	invalidate_references();
}


//-------------------------------------------------
//  expand_entries - expand the allocated array
//  of entries
//-------------------------------------------------

void memory_bank::expand_entries(int entrynum)
{
	// allocate a new array and copy from the old one; zero out the new entries
	int old_size = m_entry.size();
	m_entry.resize(entrynum + 1);
	memset(&m_entry[old_size], 0, (entrynum+1-old_size)*sizeof(m_entry[0]));
}


//-------------------------------------------------
//  configure_entry - configure an entry
//-------------------------------------------------

void memory_bank::configure_entry(int entrynum, void *base)
{
	// must be positive
	if (entrynum < 0)
		throw emu_fatalerror("memory_bank::configure_entry called with out-of-range entry %d", entrynum);

	// if we haven't allocated this many entries yet, expand our array
	if (entrynum >= int(m_entry.size()))
		expand_entries(entrynum);

	// set the entry
	m_entry[entrynum].m_ptr = reinterpret_cast<u8 *>(base);

	// if the bank base is not configured, and we're the first entry, set us up
	if (*m_baseptr == nullptr && entrynum == 0)
		*m_baseptr = m_entry[entrynum].m_ptr;
}


//-------------------------------------------------
//  configure_entries - configure multiple entries
//-------------------------------------------------

void memory_bank::configure_entries(int startentry, int numentries, void *base, offs_t stride)
{
	// fill in the requested bank entries (backwards to improve allocation)
	for (int entrynum = startentry + numentries - 1; entrynum >= startentry; entrynum--)
		configure_entry(entrynum, reinterpret_cast<u8 *>(base) + (entrynum - startentry) * stride);
}


//**************************************************************************
//  MEMORY REGIONS
//**************************************************************************

//-------------------------------------------------
//  memory_region - constructor
//-------------------------------------------------

memory_region::memory_region(running_machine &machine, const char *name, u32 length, u8 width, endianness_t endian)
	: m_machine(machine),
		m_name(name),
		m_buffer(length),
		m_endianness(endian),
		m_bitwidth(width * 8),
		m_bytewidth(width)
{
	assert(width == 1 || width == 2 || width == 4 || width == 8);
}



//**************************************************************************
//  HANDLER ENTRY
//**************************************************************************

//-------------------------------------------------
//  handler_entry - constructor
//-------------------------------------------------

handler_entry::handler_entry(u8 width, endianness_t endianness, u8 **rambaseptr)
	: m_populated(false),
		m_datawidth(width),
		m_endianness(endianness),
		m_addrstart(0),
		m_addrend(0),
		m_addrmask(~0),
		m_bytemask(~0),
		m_rambaseptr(rambaseptr),
		m_subunits(0),
		m_invsubmask(0)
{
}


//-------------------------------------------------
//  ~handler_entry - destructor
//-------------------------------------------------

handler_entry::~handler_entry()
{
}


//-------------------------------------------------
//  copy - copy another handler_entry, but only
//  if it is populated and constitutes of one or
//  more subunit handlers
//-------------------------------------------------

void handler_entry::copy(handler_entry *entry)
{
	assert(entry->m_populated);
	assert(entry->m_subunits);
	assert(!entry->m_rambaseptr);
	assert(!m_populated);

	m_populated = true;
	m_datawidth = entry->m_datawidth;
	m_endianness = entry->m_endianness;
	m_addrstart = entry->m_addrstart;
	m_addrend = entry->m_addrend;
	m_addrmask = entry->m_addrmask;
	m_bytemask = entry->m_bytemask;
	m_rambaseptr = nullptr;
	m_subunits = entry->m_subunits;
	memcpy(m_subunit_infos, entry->m_subunit_infos, m_subunits*sizeof(subunit_info));
	m_invsubmask = entry->m_invsubmask;
}


//-------------------------------------------------
//  reconfigure_subunits - reconfigure the subunits
//  to handle a new base address
//-------------------------------------------------
void handler_entry::reconfigure_subunits(offs_t addrstart)
{
	s32 delta = addrstart - m_addrstart;
	for (int i=0; i != m_subunits; i++)
		m_subunit_infos[i].m_offset += delta / (m_subunit_infos[i].m_size / 8);
}


//-------------------------------------------------
//  configure_subunits - configure the subunits
//  and subshift array to represent the provided
//  mask
//-------------------------------------------------

void handler_entry::configure_subunits(u64 handlermask, int handlerbits, int cswidth, int &start_slot, int &end_slot)
{
	u64 unitmask = (u64(1) << handlerbits) - 1;
	u64 selectmask = (u64(1) << cswidth) - 1;
	assert(handlermask != 0);
	assert(m_datawidth == 64 || (handlermask >> m_datawidth) == 0);

	// compute the maximum possible subunits
	int maxunits = m_datawidth / handlerbits;
	assert(maxunits > 1);
	assert(maxunits <= ARRAY_LENGTH(m_subunit_infos));

	int granularity = cswidth / handlerbits;
	assert(granularity > 0);

	int shift_xor_mask = m_endianness == ENDIANNESS_LITTLE ? 0 : maxunits - 1;

	// walk the handlermask to find out how many we have
	int count = 0;
	for (u64 scanmask = handlermask; scanmask != 0; scanmask >>= handlerbits)
	{
		assert((scanmask & unitmask) == 0 || (scanmask & unitmask) == unitmask);
		if ((scanmask & unitmask) != 0)
			count++;
	}
	if (count == 0 || count > maxunits)
		throw emu_fatalerror("Invalid subunit mask %s for %d-bit handler in %d-bit space", core_i64_hex_format(handlermask, m_datawidth / 4), handlerbits, m_datawidth);

	// make sure that the multiplier is a power of 2
	int multiplier = count;
	while ((multiplier & (multiplier-1)) != 0)
		multiplier++;

	// fill in the shifts
	int cur_offset = 0;
	start_slot = m_subunits;
	for (int unitnum = 0; unitnum < maxunits; unitnum++)
	{
		u32 shift = (unitnum^shift_xor_mask) * handlerbits;
		if (((handlermask >> shift) & unitmask) != 0)
		{
			m_subunit_infos[m_subunits].m_addrmask = m_bytemask / (maxunits / multiplier);
			m_subunit_infos[m_subunits].m_datamask = unitmask << shift;
			m_subunit_infos[m_subunits].m_csmask = selectmask << (((unitnum^shift_xor_mask) & ~(granularity-1)) * handlerbits);
			m_subunit_infos[m_subunits].m_offset = cur_offset++;
			m_subunit_infos[m_subunits].m_size = handlerbits;
			m_subunit_infos[m_subunits].m_shift = shift;
			m_subunit_infos[m_subunits].m_multiplier = multiplier;
			m_subunits++;
		}
	}
	end_slot = m_subunits;

	// compute the inverse mask
	m_invsubmask = 0;
	for (int i = 0; i < m_subunits; i++)
		m_invsubmask |= unitmask << m_subunit_infos[i].m_shift;
	m_invsubmask = ~m_invsubmask;
}


//-------------------------------------------------
//  clear_conflicting_subunits - clear the subunits
//  conflicting with the provided mask
//-------------------------------------------------

void handler_entry::clear_conflicting_subunits(u64 handlermask)
{
	// A mask of 0 is in fact an alternative way of saying ~0
	if (!handlermask)
	{
		m_subunits = 0;
		return;
	}

	// Start by the end to avoid unnecessary memmoves
	for (int i=m_subunits-1; i>=0; i--)
		if ((handlermask & m_subunit_infos[i].m_datamask) != 0)
		{
			if (i != m_subunits-1)
				memmove (m_subunit_infos+i, m_subunit_infos+i+1, (m_subunits-i-1)*sizeof(m_subunit_infos[0]));
			remove_subunit(i);
		}

	// Recompute the inverse mask
	m_invsubmask = 0;
	for (int i = 0; i < m_subunits; i++)
		m_invsubmask |= m_subunit_infos[i].m_datamask;
	m_invsubmask = ~m_invsubmask;
}


//-------------------------------------------------
//  overriden_by_mask - check whether a handler with
//  the provided mask fully overrides everything
//  that's currently present
//-------------------------------------------------

bool handler_entry::overriden_by_mask(u64 handlermask) const
{
	// A mask of 0 is in fact an alternative way of saying ~0
	if (!handlermask)
		return true;

	// If there are no subunits, it's going to override
	if (!m_subunits)
		return true;

	// Check whether a subunit would be left
	for (int i=0; i != m_subunits; i++)
		if ((handlermask & m_subunit_infos[i].m_datamask) == 0)
			return false;

	return true;
}


//-------------------------------------------------
//  description - build a printable description
//  of the handler
//-------------------------------------------------

void handler_entry::description(char *buffer) const
{
	if (m_subunits)
	{
		for (int i=0; i != m_subunits; i++)
		{
			if (i)
				*buffer++ = ' ';
			buffer += sprintf (buffer, "%d:%d:%x:%d:%x:%s:%s:%s",
								m_subunit_infos[i].m_size,
								m_subunit_infos[i].m_shift,
								m_subunit_infos[i].m_offset,
								m_subunit_infos[i].m_multiplier,
								m_subunit_infos[i].m_addrmask,
								core_i64_hex_format(m_subunit_infos[i].m_datamask, m_datawidth),
								core_i64_hex_format(m_subunit_infos[i].m_csmask, m_datawidth),
								subunit_name(i));
		}
	}
	else
		strcpy (buffer, name());
}


//**************************************************************************
//  HANDLER ENTRY READ
//**************************************************************************

//-------------------------------------------------
//  copy - copy another handler_entry, but only
//  if it is populated and constitutes of one or
//  more subunit handlers
//-------------------------------------------------

void handler_entry_read::copy(handler_entry *entry)
{
	handler_entry::copy(entry);
	handler_entry_read *rentry = static_cast<handler_entry_read *>(entry);
	m_read = rentry->m_read;
	for(int i = 0; i < m_subunits; ++i)
	{
		switch(m_subunit_infos[i].m_size)
		{
			case 8:
				m_subread[i].r8 = rentry->m_subread[i].r8;
				break;
			case 16:
				m_subread[i].r16 = rentry->m_subread[i].r16;
				break;
			case 32:
				m_subread[i].r32 = rentry->m_subread[i].r32;
				break;
		}
	}
}

//-------------------------------------------------
//  name - return the handler name, from the
//  appropriately-sized delegate
//-------------------------------------------------

const char *handler_entry_read::name() const
{
	switch (m_datawidth)
	{
		case 8:     return m_read.r8.name();
		case 16:    return m_read.r16.name();
		case 32:    return m_read.r32.name();
		case 64:    return m_read.r64.name();
	}
	return nullptr;
}

//-------------------------------------------------
//  subunit_name - return the handler name, from the
//  appropriately-sized delegate of a subunit
//-------------------------------------------------

const char *handler_entry_read::subunit_name(int entry) const
{
	switch (m_subunit_infos[entry].m_size)
	{
		case 8:     return m_subread[entry].r8.name();
		case 16:    return m_subread[entry].r16.name();
		case 32:    return m_subread[entry].r32.name();
		case 64:    return m_subread[entry].r64.name();
	}
	return nullptr;
}


//-------------------------------------------------
//  remove_subunit - delete a subunit specific
//  information and shift up the following ones
//-------------------------------------------------
void handler_entry_read::remove_subunit(int entry)
{
	int moving = m_subunits - entry - 1;
	if (moving)
	{
		memmove(m_subread+entry,        m_subread+entry+1,        moving*sizeof(m_subread[0]));
	}
	m_subunits--;
}


//-------------------------------------------------
//  set_delegate - set an 8-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_read::set_delegate(read8_delegate delegate, u64 umask, int cswidth)
{
	// error if no object
	if (!delegate.has_object())
		throw emu_fatalerror("Attempted to install delegate '%s' without a bound object", delegate.name());

	// make sure this is a valid size
	assert(m_datawidth >= 8);

	// if mismatched bus width, configure a stub
	if (m_datawidth != 8)
	{
		int start_slot, end_slot;
		configure_subunits(umask, 8, cswidth, start_slot, end_slot);
		for (int i=start_slot; i != end_slot; i++)
		{
			m_subread[i].r8 = delegate;
		}
		if (m_datawidth == 16)
			set_delegate(read16_delegate(&handler_entry_read::read_stub_16, delegate.name(), this));
		else if (m_datawidth == 32)
			set_delegate(read32_delegate(&handler_entry_read::read_stub_32, delegate.name(), this));
		else if (m_datawidth == 64)
			set_delegate(read64_delegate(&handler_entry_read::read_stub_64, delegate.name(), this));
	}
	else
	{
		m_read.r8 = delegate;
	}
}


//-------------------------------------------------
//  set_delegate - set a 16-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_read::set_delegate(read16_delegate delegate, u64 umask, int cswidth)
{
	// error if no object
	if (!delegate.has_object())
		throw emu_fatalerror("Attempted to install delegate '%s' without a bound object", delegate.name());

	// make sure this is a valid size
	assert(m_datawidth >= 16);

	// if mismatched bus width, configure a stub
	if (m_datawidth != 16)
	{
		int start_slot, end_slot;
		configure_subunits(umask, 16, cswidth, start_slot, end_slot);
		for (int i=start_slot; i != end_slot; i++)
		{
			m_subread[i].r16 = delegate;
		}
		if (m_datawidth == 32)
			set_delegate(read32_delegate(&handler_entry_read::read_stub_32, delegate.name(), this));
		else if (m_datawidth == 64)
			set_delegate(read64_delegate(&handler_entry_read::read_stub_64, delegate.name(), this));
	}
	else
	{
		m_read.r16 = delegate;
	}
}


//-------------------------------------------------
//  set_delegate - set a 32-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_read::set_delegate(read32_delegate delegate, u64 umask, int cswidth)
{
	// error if no object
	if (!delegate.has_object())
		throw emu_fatalerror("Attempted to install delegate '%s' without a bound object", delegate.name());

	// make sure this is a valid size
	assert(m_datawidth >= 32);

	// if mismatched bus width, configure a stub
	if (m_datawidth != 32)
	{
		int start_slot, end_slot;
		configure_subunits(umask, 32, cswidth, start_slot, end_slot);
		for (int i=start_slot; i != end_slot; i++)
		{
			m_subread[i].r32 = delegate;
		}
		if (m_datawidth == 64)
			set_delegate(read64_delegate(&handler_entry_read::read_stub_64, delegate.name(), this));
	}
	else
	{
		m_read.r32 = delegate;
	}
}


//-------------------------------------------------
//  set_delegate - set a 64-bit delegate
//-------------------------------------------------

void handler_entry_read::set_delegate(read64_delegate delegate, u64 umask, int cswidth)
{
	// error if no object
	if (!delegate.has_object())
		throw emu_fatalerror("Attempted to install delegate '%s' without a bound object", delegate.name());

	// make sure this is a valid size
	assert(m_datawidth >= 64);
	m_read.r64 = delegate;
}


//-------------------------------------------------
//  set_ioport - configure an I/O port read stub
//  of the appropriate size
//-------------------------------------------------

void handler_entry_read::set_ioport(ioport_port &ioport)
{
	m_ioport = &ioport;
	if (m_datawidth == 8)
		set_delegate(read8_delegate(&handler_entry_read::read_stub_ioport<u8>, ioport.tag(), this));
	else if (m_datawidth == 16)
		set_delegate(read16_delegate(&handler_entry_read::read_stub_ioport<u16>, ioport.tag(), this));
	else if (m_datawidth == 32)
		set_delegate(read32_delegate(&handler_entry_read::read_stub_ioport<u32>, ioport.tag(), this));
	else if (m_datawidth == 64)
		set_delegate(read64_delegate(&handler_entry_read::read_stub_ioport<u64>, ioport.tag(), this));
}


//-------------------------------------------------
//  read_stub_16 - construct a 16-bit read from
//  8-bit sources
//-------------------------------------------------

u16 handler_entry_read::read_stub_16(address_space &space, offs_t offset, u16 mask)
{
	u16 result = space.unmap() & m_invsubmask;
	for (int index = 0; index < m_subunits; index++)
	{
		const subunit_info &si = m_subunit_infos[index];
		if (mask & si.m_csmask)
		{
			offs_t aoffset = offset * si.m_multiplier + si.m_offset;
			u8 val;
			val = m_subread[index].r8(space, aoffset & si.m_addrmask, mask >> si.m_shift);
			result |= val << si.m_shift;
		}
	}
	return result;
}


//-------------------------------------------------
//  read_stub_32 - construct a 32-bit read from
//  8-bit and 16-bit sources
//-------------------------------------------------

u32 handler_entry_read::read_stub_32(address_space &space, offs_t offset, u32 mask)
{
	u32 result = space.unmap() & m_invsubmask;
	for (int index = 0; index < m_subunits; index++)
	{
		const subunit_info &si = m_subunit_infos[index];
		if (mask & si.m_csmask)
		{
			offs_t aoffset = offset * si.m_multiplier + si.m_offset;
			u16 val = 0;
			switch (si.m_size)
			{
			case 8:
				val = m_subread[index].r8(space, aoffset & si.m_addrmask, mask >> si.m_shift);
				break;
			case 16:
				val = m_subread[index].r16(space, aoffset & si.m_addrmask, mask >> si.m_shift);
				break;
			}
			result |= val << si.m_shift;
		}
	}
	return result;
}


//-------------------------------------------------
//  read_stub_64 - construct a 64-bit read from
//  8-bit, 16-bit and 32-bit sources
//-------------------------------------------------

u64 handler_entry_read::read_stub_64(address_space &space, offs_t offset, u64 mask)
{
	u64 result = space.unmap() & m_invsubmask;
	for (int index = 0; index < m_subunits; index++)
	{
		const subunit_info &si = m_subunit_infos[index];
		if (mask & si.m_csmask)
		{
			offs_t aoffset = offset * si.m_multiplier + si.m_offset;
			u32 val = 0;
			switch (si.m_size)
			{
			case 8:
				val = m_subread[index].r8(space, aoffset & si.m_addrmask, mask >> si.m_shift);
				break;
			case 16:
				val = m_subread[index].r16(space, aoffset & si.m_addrmask, mask >> si.m_shift);
				break;
			case 32:
				val = m_subread[index].r32(space, aoffset & si.m_addrmask, mask >> si.m_shift);
				break;
			}
			result |= u64(val) << si.m_shift;
		}
	}
	return result;
}


//**************************************************************************
//  HANDLER ENTRY WRITE
//**************************************************************************

//-------------------------------------------------
//  copy - copy another handler_entry, but only
//  if it is populated and constitutes of one or
//  more subunit handlers
//-------------------------------------------------

void handler_entry_write::copy(handler_entry *entry)
{
	handler_entry::copy(entry);
	handler_entry_write *wentry = static_cast<handler_entry_write *>(entry);
	m_write = wentry->m_write;
	for(int i = 0; i < m_subunits; ++i)
	{
		switch(m_subunit_infos[i].m_size)
		{
			case 8:
				m_subwrite[i].w8 = wentry->m_subwrite[i].w8;
				break;
			case 16:
				m_subwrite[i].w16 = wentry->m_subwrite[i].w16;
				break;
			case 32:
				m_subwrite[i].w32 = wentry->m_subwrite[i].w32;
				break;
		}
	}
}

//-------------------------------------------------
//  name - return the handler name, from the
//  appropriately-sized delegate
//-------------------------------------------------

const char *handler_entry_write::name() const
{
	switch (m_datawidth)
	{
		case 8:     return m_write.w8.name();
		case 16:    return m_write.w16.name();
		case 32:    return m_write.w32.name();
		case 64:    return m_write.w64.name();
	}
	return nullptr;
}


//-------------------------------------------------
//  subunit_name - return the handler name, from the
//  appropriately-sized delegate of a subunit
//-------------------------------------------------

const char *handler_entry_write::subunit_name(int entry) const
{
	switch (m_subunit_infos[entry].m_size)
	{
		case 8:     return m_subwrite[entry].w8.name();
		case 16:    return m_subwrite[entry].w16.name();
		case 32:    return m_subwrite[entry].w32.name();
		case 64:    return m_subwrite[entry].w64.name();
	}
	return nullptr;
}


//-------------------------------------------------
//  remove_subunit - delete a subunit specific
//  information and shift up the following ones
//-------------------------------------------------
void handler_entry_write::remove_subunit(int entry)
{
	int moving = m_subunits - entry - 1;
	if (moving)
	{
		memmove(m_subwrite+entry,       m_subwrite+entry+1,       moving*sizeof(m_subwrite[0]));
	}
	m_subunits--;
}


//-------------------------------------------------
//  set_delegate - set an 8-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_write::set_delegate(write8_delegate delegate, u64 umask, int cswidth)
{
	assert(m_datawidth >= 8);

	// if mismatched bus width, configure a stub
	if (m_datawidth != 8)
	{
		int start_slot, end_slot;
		configure_subunits(umask, 8, cswidth, start_slot, end_slot);
		for (int i=start_slot; i != end_slot; i++)
		{
			m_subwrite[i].w8 = delegate;
		}
		if (m_datawidth == 16)
			set_delegate(write16_delegate(&handler_entry_write::write_stub_16, delegate.name(), this));
		else if (m_datawidth == 32)
			set_delegate(write32_delegate(&handler_entry_write::write_stub_32, delegate.name(), this));
		else if (m_datawidth == 64)
			set_delegate(write64_delegate(&handler_entry_write::write_stub_64, delegate.name(), this));
	}
	else
	{
		m_write.w8 = delegate;
	}
}


//-------------------------------------------------
//  set_delegate - set a 16-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_write::set_delegate(write16_delegate delegate, u64 umask, int cswidth)
{
	assert(m_datawidth >= 16);

	// if mismatched bus width, configure a stub
	if (m_datawidth != 16)
	{
		int start_slot, end_slot;
		configure_subunits(umask, 16, cswidth, start_slot, end_slot);
		for (int i=start_slot; i != end_slot; i++)
		{
			m_subwrite[i].w16 = delegate;
		}
		if (m_datawidth == 32)
			set_delegate(write32_delegate(&handler_entry_write::write_stub_32, delegate.name(), this));
		else if (m_datawidth == 64)
			set_delegate(write64_delegate(&handler_entry_write::write_stub_64, delegate.name(), this));
	}
	else
	{
		m_write.w16 = delegate;
	}
}


//-------------------------------------------------
//  set_delegate - set a 32-bit delegate, and
//  configure a stub if necessary
//-------------------------------------------------

void handler_entry_write::set_delegate(write32_delegate delegate, u64 umask, int cswidth)
{
	assert(m_datawidth >= 32);

	// if mismatched bus width, configure a stub
	if (m_datawidth != 32)
	{
		int start_slot, end_slot;
		configure_subunits(umask, 32, cswidth, start_slot, end_slot);
		for (int i=start_slot; i != end_slot; i++)
		{
			m_subwrite[i].w32 = delegate;
		}
		if (m_datawidth == 64)
			set_delegate(write64_delegate(&handler_entry_write::write_stub_64, delegate.name(), this));
	}
	else
	{
		m_write.w32 = delegate;
	}
}


//-------------------------------------------------
//  set_delegate - set a 64-bit delegate
//-------------------------------------------------

void handler_entry_write::set_delegate(write64_delegate delegate, u64 mask, int cswidth)
{
	assert(m_datawidth >= 64);
	m_write.w64 = delegate;
}


//-------------------------------------------------
//  set_ioport - configure an I/O port read stub
//  of the appropriate size
//-------------------------------------------------

void handler_entry_write::set_ioport(ioport_port &ioport)
{
	m_ioport = &ioport;
	if (m_datawidth == 8)
		set_delegate(write8_delegate(&handler_entry_write::write_stub_ioport<u8>, ioport.tag(), this));
	else if (m_datawidth == 16)
		set_delegate(write16_delegate(&handler_entry_write::write_stub_ioport<u16>, ioport.tag(), this));
	else if (m_datawidth == 32)
		set_delegate(write32_delegate(&handler_entry_write::write_stub_ioport<u32>, ioport.tag(), this));
	else if (m_datawidth == 64)
		set_delegate(write64_delegate(&handler_entry_write::write_stub_ioport<u64>, ioport.tag(), this));
}


//-------------------------------------------------
//  write_stub_16 - construct a 16-bit write from
//  8-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_16(address_space &space, offs_t offset, u16 data, u16 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		const subunit_info &si = m_subunit_infos[index];
		if (mask & si.m_csmask)
		{
			offs_t aoffset = offset * si.m_multiplier + si.m_offset;
			u8 adata = data >> si.m_shift;
			u8 amask = mask >> si.m_shift;
			m_subwrite[index].w8(space, aoffset & si.m_addrmask, adata, amask);
		}
	}
}


//-------------------------------------------------
//  write_stub_32 - construct a 32-bit write from
//  8-bit and 16-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_32(address_space &space, offs_t offset, u32 data, u32 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		const subunit_info &si = m_subunit_infos[index];
		if (mask & si.m_csmask)
		{
			offs_t aoffset = offset * si.m_multiplier + si.m_offset;
			u16 adata = data >> si.m_shift;
			u16 amask = mask >> si.m_shift;
			switch (si.m_size)
			{
			case 8:
				m_subwrite[index].w8(space, aoffset & si.m_addrmask, adata, amask);
				break;
			case 16:
				m_subwrite[index].w16(space, aoffset & si.m_addrmask, adata, amask);
				break;
			}
		}
	}
}


//-------------------------------------------------
//  write_stub_64 - construct a 64-bit write from
//  8-bit, 16-bit and 32-bit sources
//-------------------------------------------------

void handler_entry_write::write_stub_64(address_space &space, offs_t offset, u64 data, u64 mask)
{
	for (int index = 0; index < m_subunits; index++)
	{
		const subunit_info &si = m_subunit_infos[index];
		if (mask & si.m_csmask)
		{
			offs_t aoffset = offset * si.m_multiplier + si.m_offset;
			u32 adata = data >> si.m_shift;
			u32 amask = mask >> si.m_shift;
			switch (si.m_size)
			{
			case 8:
				m_subwrite[index].w8(space, aoffset & si.m_addrmask, adata, amask);
				break;
			case 16:
				m_subwrite[index].w16(space, aoffset & si.m_addrmask, adata, amask);
				break;
			case 32:
				m_subwrite[index].w32(space, aoffset & si.m_addrmask, adata, amask);
				break;
			}
		}
	}
}
