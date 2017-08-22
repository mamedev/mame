// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    addrmap.h

    Macros and helper functions for handling address map definitions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __ADDRMAP_H__
#define __ADDRMAP_H__



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// address map handler types
enum map_handler_type
{
	AMH_NONE = 0,
	AMH_RAM,
	AMH_ROM,
	AMH_NOP,
	AMH_UNMAP,
	AMH_DEVICE_DELEGATE,
	AMH_PORT,
	AMH_BANK,
	AMH_DEVICE_SUBMAP
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// address map handler data
class map_handler_data
{
public:
	map_handler_data()
		: m_type(AMH_NONE),
			m_bits(0),
			m_mask(0),
			m_name(nullptr),
			m_tag(nullptr) { }

	map_handler_type    m_type;             // type of the handler
	u8                  m_bits;             // width of the handler in bits, or 0 for default
	u64                 m_mask;             // mask for which lanes apply
	const char *        m_name;             // name of the handler
	const char *        m_tag;              // tag for I/O ports and banks
};



// ======================> address_map_entry

// address_map_entry is a linked list element describing one address range in a map
class address_map_entry
{
	friend class address_map;

public:
	// construction/destruction
	address_map_entry(device_t &device, address_map &map, offs_t start, offs_t end);

	// getters
	address_map_entry *next() const { return m_next; }

	// simple inline setters
	address_map_entry &mirror(offs_t _mirror) { m_addrmirror = _mirror; return *this; }
	address_map_entry &select(offs_t _select) { m_addrselect = _select; return *this; }
	address_map_entry &region(const char *tag, offs_t offset) { m_region = tag; m_rgnoffs = offset; return *this; }
	address_map_entry &share(const char *tag) { m_share = tag; return *this; }

	address_map_entry &rom() { m_read.m_type = AMH_ROM; return *this; }
	address_map_entry &ram() { m_read.m_type = AMH_RAM; m_write.m_type = AMH_RAM; return *this; }
	address_map_entry &readonly() { m_read.m_type = AMH_RAM; return *this; }
	address_map_entry &writeonly() { m_write.m_type = AMH_RAM; return *this; }
	address_map_entry &unmap() { m_read.m_type = AMH_UNMAP; m_write.m_type = AMH_UNMAP; return *this; }
	address_map_entry &readunmap() { m_read.m_type = AMH_UNMAP; return *this; }
	address_map_entry &writeunmap() { m_write.m_type = AMH_UNMAP; return *this; }
	address_map_entry &nop() { m_read.m_type = AMH_NOP; m_write.m_type = AMH_NOP; return *this; }
	address_map_entry &readnop() { m_read.m_type = AMH_NOP; return *this; }
	address_map_entry &writenop() { m_write.m_type = AMH_NOP; return *this; }

	// mask setting
	address_map_entry &mask(offs_t _mask);

	// I/O port configuration
	address_map_entry &read_port(const char *tag) { m_read.m_type = AMH_PORT; m_read.m_tag = tag; return *this; }
	address_map_entry &write_port(const char *tag) { m_write.m_type = AMH_PORT; m_write.m_tag = tag; return *this; }
	address_map_entry &readwrite_port(const char *tag) { read_port(tag); write_port(tag); return *this; }

	// memory bank configuration
	address_map_entry &read_bank(const char *tag) { m_read.m_type = AMH_BANK; m_read.m_tag = tag; return *this; }
	address_map_entry &write_bank(const char *tag) { m_write.m_type = AMH_BANK; m_write.m_tag = tag; return *this; }
	address_map_entry &readwrite_bank(const char *tag) { read_bank(tag); write_bank(tag); return *this; }

	address_map_entry &rombank(const char *tag) { return read_bank(tag); }
	address_map_entry &rambank(const char *tag) { return readwrite_bank(tag); }

	// set offset handler (only one version, since there is no data width to consider)
	address_map_entry &set_handler(setoffset_delegate func);

	// type setters
	address_map_entry &set_read_type(map_handler_type _type) { m_read.m_type = _type; return *this; }
	address_map_entry &set_write_type(map_handler_type _type) { m_write.m_type = _type; return *this; }

	// submap referencing
	address_map_entry &set_submap(const char *tag, address_map_delegate func, int bits, u64 mask);

	// public state
	address_map_entry *     m_next;                 // pointer to the next entry
	address_map &           m_map;                  // reference to our owning map
	device_t &              m_devbase;              // reference to "base" device for tag lookups

	// basic information
	offs_t                  m_addrstart;            // start address
	offs_t                  m_addrend;              // end address
	offs_t                  m_addrmirror;           // mirror bits
	offs_t                  m_addrmask;             // mask bits
	offs_t                  m_addrselect;           // select bits
	map_handler_data        m_read;                 // data for read handler
	map_handler_data        m_write;                // data for write handler
	map_handler_data        m_setoffsethd;          // data for setoffset handler
	const char *            m_share;                // tag of a shared memory block
	const char *            m_region;               // tag of region containing the memory backing this entry
	offs_t                  m_rgnoffs;              // offset within the region

	// handlers
	read8_delegate          m_rproto8;              // 8-bit read proto-delegate
	read16_delegate         m_rproto16;             // 16-bit read proto-delegate
	read32_delegate         m_rproto32;             // 32-bit read proto-delegate
	read64_delegate         m_rproto64;             // 64-bit read proto-delegate
	write8_delegate         m_wproto8;              // 8-bit write proto-delegate
	write16_delegate        m_wproto16;             // 16-bit write proto-delegate
	write32_delegate        m_wproto32;             // 32-bit write proto-delegate
	write64_delegate        m_wproto64;             // 64-bit write proto-delegate

	setoffset_delegate      m_soproto;              // set offset proto-delegate
	address_map_delegate    m_submap_delegate;
	int                     m_submap_bits;

	// information used during processing
	void *                  m_memory;               // pointer to memory backing this entry
	offs_t                  m_bytestart;            // byte-adjusted start address
	offs_t                  m_byteend;              // byte-adjusted end address
	offs_t                  m_bytemirror;           // byte-adjusted mirror bits
	offs_t                  m_bytemask;             // byte-adjusted mask bits

	// handler setters for 8-bit functions
	address_map_entry &set_handler(read8_delegate func, u64 mask = 0);
	address_map_entry &set_handler(write8_delegate func, u64 mask = 0);
	address_map_entry &set_handler(read8_delegate rfunc, write8_delegate wfunc, u64 mask = 0);

	// handler setters for 16-bit functions
	address_map_entry &set_handler(read16_delegate func, u64 mask = 0);
	address_map_entry &set_handler(write16_delegate func, u64 mask = 0);
	address_map_entry &set_handler(read16_delegate rfunc, write16_delegate wfunc, u64 mask = 0);

	// handler setters for 32-bit functions
	address_map_entry &set_handler(read32_delegate func, u64 mask = 0);
	address_map_entry &set_handler(write32_delegate func, u64 mask = 0);
	address_map_entry &set_handler(read32_delegate rfunc, write32_delegate wfunc, u64 mask = 0);

	// handler setters for 64-bit functions
	address_map_entry &set_handler(read64_delegate func, u64 mask = 0);
	address_map_entry &set_handler(write64_delegate func, u64 mask = 0);
	address_map_entry &set_handler(read64_delegate rfunc, write64_delegate wfunc, u64 mask = 0);

private:
	// helper functions
	bool unitmask_is_appropriate(u8 width, u64 unitmask, const char *string) const;
};

// ======================> address_map

// address_map holds global map parameters plus the head of the list of entries
class address_map
{
public:
	// construction/destruction
	address_map(device_t &device, int spacenum);
	address_map(device_t &device, address_map_entry *entry);
	address_map(const address_space &space, offs_t start, offs_t end, int bits, u64 unitmask, device_t &device, address_map_delegate submap_delegate);
	~address_map();

	// configuration
	void configure(int _spacenum, u8 _databits);

	// setters
	void global_mask(offs_t mask);
	void unmap_value_low() { m_unmapval = 0; }
	void unmap_value_high() { m_unmapval = ~0; }
	void unmap_value(u8 value) { m_unmapval = value; }

	// add a new entry of the given type
	address_map_entry &range(offs_t start, offs_t end);

	// public data
	int                m_spacenum;     // space number of the map
	device_t *                      m_device;       // associated device
	u8                              m_databits;     // data bits represented by the map
	u8                              m_unmapval;     // unmapped memory value
	offs_t                          m_globalmask;   // global mask
	simple_list<address_map_entry>  m_entrylist;    // list of entries

	void uplift_submaps(running_machine &machine, device_t &owner, endianness_t endian);
	void map_validity_check(validity_checker &valid, int spacenum) const;
};


//**************************************************************************
//  ADDRESS MAP MACROS
//**************************************************************************

// so that "0" can be used for unneeded address maps
#define construct_address_map_0 nullptr

// start/end tags for the address map
#define ADDRESS_MAP_NAME(_name) construct_address_map_##_name

#define ADDRESS_MAP_START(_name, _space, _bits, _class) \
void ADDRESS_MAP_NAME(_name)(address_map &map) \
{ \
	typedef read##_bits##_delegate read_delegate ATTR_UNUSED; \
	typedef write##_bits##_delegate write_delegate ATTR_UNUSED; \
	typedef u##_bits native_type ATTR_UNUSED; \
	map.configure(_space, _bits); \
	typedef _class drivdata_class ATTR_UNUSED
#define DEVICE_ADDRESS_MAP_START(_name, _bits, _class) \
void _class :: _name(::address_map &map) \
{ \
	typedef read##_bits##_delegate read_delegate ATTR_UNUSED; \
	typedef write##_bits##_delegate write_delegate ATTR_UNUSED; \
	typedef u##_bits native_type ATTR_UNUSED; \
	map.configure(AS_PROGRAM, _bits);  \
	typedef _class drivdata_class ATTR_UNUSED
#define ADDRESS_MAP_END \
;}

// use this to declare external references to an address map
#define ADDRESS_MAP_EXTERN(_name, _bits) \
	extern void ADDRESS_MAP_NAME(_name)(address_map &map)

// use this to declare an address map as a member of a modern device class
// need to qualify with :: to avoid a collision with descendants of device_memory_interface
#define DECLARE_ADDRESS_MAP(_name, _bits) \
	void _name(::address_map &map)


// global controls
#define ADDRESS_MAP_GLOBAL_MASK(_mask) \
	;map.global_mask(_mask)
#define ADDRESS_MAP_UNMAP_LOW \
	;map.unmap_value_low()
#define ADDRESS_MAP_UNMAP_HIGH \
	;map.unmap_value_high()

// importing data from other address maps
#define AM_IMPORT_FROM(_name) \
	;ADDRESS_MAP_NAME(_name)(map)
// importing data from inherited address maps
#define AM_INHERIT_FROM(_name) \
	;_name(map)

// address ranges
#define AM_RANGE(_start, _end) \
	;map.range(_start, _end)
#define AM_MASK(_mask) \
	.mask(_mask)
#define AM_MIRROR(_mirror) \
	.mirror(_mirror)
#define AM_SELECT(_select) \
	.select(_select)

// driver data reads
#define AM_READ(_handler) \
	.set_handler(read_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr))
#define AM_READ8(_handler, _unitmask) \
	.set_handler(read8_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)
#define AM_READ16(_handler, _unitmask) \
	.set_handler(read16_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)
#define AM_READ32(_handler, _unitmask) \
	.set_handler(read32_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)

// driver data writes
#define AM_WRITE(_handler) \
	.set_handler(write_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr))
#define AM_WRITE8(_handler, _unitmask) \
	.set_handler(write8_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)
#define AM_WRITE16(_handler, _unitmask) \
	.set_handler(write16_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)
#define AM_WRITE32(_handler, _unitmask) \
	.set_handler(write32_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)

// driver data reads/writes
#define AM_READWRITE(_rhandler, _whandler) \
	.set_handler(read_delegate(&drivdata_class::_rhandler, "driver_data::" #_rhandler, DEVICE_SELF, (drivdata_class *)nullptr), write_delegate(&drivdata_class::_whandler, "driver_data::" #_whandler, DEVICE_SELF, (drivdata_class *)nullptr))
#define AM_READWRITE8(_rhandler, _whandler, _unitmask) \
	.set_handler(read8_delegate(&drivdata_class::_rhandler, "driver_data::" #_rhandler, DEVICE_SELF, (drivdata_class *)nullptr), write8_delegate(&drivdata_class::_whandler, "driver_data::" #_whandler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)
#define AM_READWRITE16(_rhandler, _whandler, _unitmask) \
	.set_handler(read16_delegate(&drivdata_class::_rhandler, "driver_data::" #_rhandler, DEVICE_SELF, (drivdata_class *)nullptr), write16_delegate(&drivdata_class::_whandler, "driver_data::" #_whandler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)
#define AM_READWRITE32(_rhandler, _whandler, _unitmask) \
	.set_handler(read32_delegate(&drivdata_class::_rhandler, "driver_data::" #_rhandler, DEVICE_SELF, (drivdata_class *)nullptr), write32_delegate(&drivdata_class::_whandler, "driver_data::" #_whandler, DEVICE_SELF, (drivdata_class *)nullptr), _unitmask)

// driver set offset. Upcast to base class because there are no data width variants,
// and the compiler complains if we don't do it explicitly
#define AM_SETOFFSET(_handler) \
	.set_handler(setoffset_delegate(&drivdata_class::_handler, "driver_data::" #_handler, DEVICE_SELF, (drivdata_class *)nullptr))

// device reads
#define AM_DEVREAD(_tag, _class, _handler) \
	.set_handler(read_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr))
#define AM_DEVREAD8(_tag, _class, _handler, _unitmask) \
	.set_handler(read8_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVREAD16(_tag, _class, _handler, _unitmask) \
	.set_handler(read16_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVREAD32(_tag, _class, _handler, _unitmask) \
	.set_handler(read32_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)

// device writes
#define AM_DEVWRITE(_tag, _class, _handler) \
	.set_handler(write_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr))
#define AM_DEVWRITE8(_tag, _class, _handler, _unitmask) \
	.set_handler(write8_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVWRITE16(_tag, _class, _handler, _unitmask) \
	.set_handler(write16_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVWRITE32(_tag, _class, _handler, _unitmask) \
	.set_handler(write32_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)

// device reads/writes
#define AM_DEVREADWRITE(_tag, _class, _rhandler, _whandler) \
	.set_handler(read_delegate(&_class::_rhandler, #_class "::" #_rhandler, _tag, (_class *)nullptr), write_delegate(&_class::_whandler, #_class "::" #_whandler, _tag, (_class *)nullptr))
#define AM_DEVREADWRITE8(_tag, _class, _rhandler, _whandler, _unitmask) \
	.set_handler(read8_delegate(&_class::_rhandler, #_class "::" #_rhandler, _tag, (_class *)nullptr), write8_delegate(&_class::_whandler, #_class "::" #_whandler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVREADWRITE16(_tag, _class, _rhandler, _whandler, _unitmask) \
	.set_handler(read16_delegate(&_class::_rhandler, #_class "::" #_rhandler, _tag, (_class *)nullptr), write16_delegate(&_class::_whandler, #_class "::" #_whandler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVREADWRITE32(_tag, _class, _rhandler, _whandler, _unitmask) \
	.set_handler(read32_delegate(&_class::_rhandler, #_class "::" #_rhandler, _tag, (_class *)nullptr), write32_delegate(&_class::_whandler, #_class "::" #_whandler, _tag, (_class *)nullptr), _unitmask)

// device reads with address shift
#define AM_DEVREAD_RSHIFT(_tag, _class, _handler, _rshift) \
	.set_handler(read_delegate([](_class &device, address_space &space, offs_t offset, native_type mem_mask)->native_type { return device._handler(space, offset >> _rshift, mem_mask); }, #_class "::" #_handler, _tag, (_class *)nullptr))
#define AM_DEVREAD8_RSHIFT(_tag, _class, _handler, _unitmask, _rshift) \
	.set_handler(read8_delegate([](_class &device, address_space &space, offs_t offset, u8 mem_mask)->u8 { return device._handler(space, offset >> _rshift, mem_mask); }, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVREAD16_RSHIFT(_tag, _class, _handler, _unitmask, _rshift) \
	.set_handler(read16_delegate([](_class &device, address_space &space, offs_t offset, u16 mem_mask)->u16 { return device._handler(space, offset >> _rshift, mem_mask); }, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVREAD32_RSHIFT(_tag, _class, _handler, _unitmask, _rshift) \
	.set_handler(read32_delegate([](_class &device, address_space &space, offs_t offset, u32 mem_mask)->u32 { return device._handler(space, offset >> _rshift, mem_mask); }, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)

// device writes with address shift
#define AM_DEVWRITE_RSHIFT(_tag, _class, _handler, _rshift) \
	.set_handler(write_delegate([](_class &device, address_space &space, offs_t offset, native_type data, native_type mem_mask) { device._handler(space, offset >> _rshift, data, mem_mask); }, #_class "::" #_handler, _tag, (_class *)nullptr))
#define AM_DEVWRITE8_RSHIFT(_tag, _class, _handler, _unitmask, _rshift) \
	.set_handler(write8_delegate([](_class &device, address_space &space, offs_t offset, u8 data, u8 mem_mask) { device._handler(space, offset >> _rshift, data, mem_mask); }, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVWRITE16_RSHIFT(_tag, _class, _handler, _unitmask, _rshift) \
	.set_handler(write16_delegate([](_class &device, address_space &space, offs_t offset, u16 data, u16 mem_mask) { device._handler(space, offset >> _rshift, data, mem_mask); }, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVWRITE32_RSHIFT(_tag, _class, _handler, _unitmask, _rshift) \
	.set_handler(write32_delegate([](_class &device, address_space &space, offs_t offset, u32 data, u32 mem_mask) { device._handler(space, offset >> _rshift, data, mem_mask); }, #_class "::" #_handler, _tag, (_class *)nullptr), _unitmask)

// device reads/writes with address shift
#define AM_DEVREADWRITE_RSHIFT(_tag, _class, _rhandler, _whandler, _rshift) \
	.set_handler(read_delegate([](_class &device, address_space &space, offs_t offset, native_type mem_mask)->native_type { return device._rhandler(space, offset >> _rshift, mem_mask); }, #_class "::" #_rhandler, _tag, (_class *)nullptr), write_delegate([](_class &device, address_space &space, offs_t offset, native_type data, native_type mem_mask) { device._whandler(space, offset >> _rshift, data, mem_mask); }, #_class "::" #_whandler, _tag, (_class *)nullptr))
#define AM_DEVREADWRITE8_RSHIFT(_tag, _class, _rhandler, _whandler, _unitmask, _rshift) \
	.set_handler(read8_delegate([](_class &device, address_space &space, offs_t offset, u8 mem_mask)->u8 { return device._rhandler(space, offset >> _rshift, mem_mask); }, #_class "::" #_rhandler, _tag, (_class *)nullptr), write8_delegate([](_class &device, address_space &space, offs_t offset, u8 data, u8 mem_mask) { device._whandler(space, offset >> _rshift, data, mem_mask); }, #_class "::" #_whandler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVREADWRITE16_RSHIFT(_tag, _class, _rhandler, _whandler, _unitmask, _rshift) \
	.set_handler(read16_delegate([](_class &device, address_space &space, offs_t offset, u16 mem_mask)->u16 { return device._rhandler(space, offset >> _rshift, mem_mask); }, #_class "::" #_rhandler, _tag, (_class *)nullptr), write16_delegate([](_class &device, address_space &space, offs_t offset, u16 data, u16 mem_mask) { device._whandler(space, offset >> _rshift, data, mem_mask); }, #_class "::" #_whandler, _tag, (_class *)nullptr), _unitmask)
#define AM_DEVREADWRITE32_RSHIFT(_tag, _class, _rhandler, _whandler, _unitmask, _rshift) \
	.set_handler(read32_delegate([](_class &device, address_space &space, offs_t offset, u32 mem_mask)->u32 { return device._rhandler(space, offset >> _rshift, mem_mask); }, #_class "::" #_rhandler, _tag, (_class *)nullptr), write32_delegate([](_class &device, address_space &space, offs_t offset, u32 data, u32 mem_mask) { device._whandler(space, offset >> _rshift, data, mem_mask); }, #_class "::" #_whandler, _tag, (_class *)nullptr), _unitmask)

// device set offset
#define AM_DEVSETOFFSET(_tag, _class, _handler) \
	.set_handler(setoffset_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr))


// device mapping
#define AM_DEVICE(_tag, _class, _handler) \
	.set_submap(_tag, address_map_delegate(&_class::_handler, #_class "::" #_handler, (_class *)nullptr), 0, 0)
#define AM_DEVICE8(_tag, _class, _handler, _unitmask) \
	.set_submap(_tag, address_map_delegate(&_class::_handler, #_class "::" #_handler, (_class *)nullptr), 8, _unitmask)
#define AM_DEVICE16(_tag, _class, _handler, _unitmask) \
	.set_submap(_tag, address_map_delegate(&_class::_handler, #_class "::" #_handler, (_class *)nullptr), 16, _unitmask)
#define AM_DEVICE32(_tag, _class, _handler, _unitmask) \
	.set_submap(_tag, address_map_delegate(&_class::_handler, #_class "::" #_handler, (_class *)nullptr), 32, _unitmask)

// special-case accesses
#define AM_ROM \
	.rom()
#define AM_RAM \
	.ram()
#define AM_READONLY \
	.readonly()
#define AM_WRITEONLY \
	.writeonly()
#define AM_UNMAP \
	.unmap()
#define AM_READUNMAP \
	.readunmap()
#define AM_WRITEUNMAP \
	.writeunmap()
#define AM_NOP \
	.nop()
#define AM_READNOP \
	.readnop()
#define AM_WRITENOP \
	.writenop()

// port accesses
#define AM_READ_PORT(_tag) \
	.read_port(_tag)
#define AM_WRITE_PORT(_tag) \
	.write_port(_tag)
#define AM_READWRITE_PORT(_tag) \
	.readwrite_port(_tag)

// bank accesses
#define AM_READ_BANK(_tag) \
	.read_bank(_tag)
#define AM_WRITE_BANK(_tag) \
	.write_bank(_tag)
#define AM_READWRITE_BANK(_tag) \
	.readwrite_bank(_tag)

// attributes for accesses
#define AM_REGION(_tag, _offs) \
	.region(_tag, _offs)
#define AM_SHARE(_tag) \
	.share(_tag)

// common shortcuts
#define AM_ROMBANK(_bank)                   .rombank(_bank)
#define AM_RAMBANK(_bank)                   .rambank(_bank)
#define AM_RAM_READ(_read)                  AM_READ(_read) AM_WRITEONLY
#define AM_RAM_WRITE(_write)                AM_READONLY AM_WRITE(_write)
#define AM_RAM_DEVREAD(_tag, _class, _read) AM_DEVREAD(_tag, _class, _read) AM_WRITEONLY
#define AM_RAM_DEVWRITE(_tag, _class, _write) AM_READONLY AM_DEVWRITE(_tag, _class, _write)


#endif  /* __ADDRMAP_H__ */
