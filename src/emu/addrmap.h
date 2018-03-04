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
			m_name(nullptr),
			m_tag(nullptr) { }

	map_handler_type    m_type;             // type of the handler
	u8                  m_bits;             // width of the handler in bits, or 0 for default
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

	// address mask setting
	address_map_entry &mask(offs_t _mask);

	// unit mask setting
	address_map_entry &umask16(u16 _mask);
	address_map_entry &umask32(u32 _mask);
	address_map_entry &umask64(u64 _mask);

	// chip select width setting
	address_map_entry &cswidth(int _cswidth) { m_cswidth = _cswidth; return *this; }

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
	address_map_entry &m(const char *tag, address_map_constructor func);

	// device tag -> delegate converter
	template<typename _devr> address_map_entry &r(const char *tag, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name) {
		return r(read8_delegate(read, read_name, tag, (_devr *)nullptr));
	}

	template<typename _devr> address_map_entry &r(const char *tag, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name) {
		return r(read16_delegate(read, read_name, tag, (_devr *)nullptr));
	}

	template<typename _devr> address_map_entry &r(const char *tag, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name) {
		return r(read32_delegate(read, read_name, tag, (_devr *)nullptr));
	}

	template<typename _devr> address_map_entry &r(const char *tag, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name) {
		return r(read64_delegate(read, read_name, tag, (_devr *)nullptr));
	}

	template<typename _devw> address_map_entry &w(const char *tag, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return w(write8_delegate(write, write_name, tag, (_devw *)nullptr));
	}

	template<typename _devw> address_map_entry &w(const char *tag, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return w(write16_delegate(write, write_name, tag, (_devw *)nullptr));
	}

	template<typename _devw> address_map_entry &w(const char *tag, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return w(write32_delegate(write, write_name, tag, (_devw *)nullptr));
	}

	template<typename _devw> address_map_entry &w(const char *tag, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return w(write64_delegate(write, write_name, tag, (_devw *)nullptr));
	}

	template<typename _devr, typename _devw> address_map_entry &rw(const char *tag, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return rw(read8_delegate(read, read_name, tag, (_devr *)nullptr),
				  write8_delegate(write, write_name, tag, (_devw *)nullptr));
	}

	template<typename _devr, typename _devw> address_map_entry &rw(const char *tag, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return rw(read16_delegate(read, read_name, tag, (_devr *)nullptr),
				  write16_delegate(write, write_name, tag, (_devw *)nullptr));
	}

	template<typename _devr, typename _devw> address_map_entry &rw(const char *tag, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return rw(read32_delegate(read, read_name, tag, (_devr *)nullptr),
				  write32_delegate(write, write_name, tag, (_devw *)nullptr));
	}

	template<typename _devr, typename _devw> address_map_entry &rw(const char *tag, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return rw(read64_delegate(read, read_name, tag, (_devr *)nullptr),
				  write64_delegate(write, write_name, tag, (_devw *)nullptr));
	}


	// device pointer -> delegate converter
	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(_devc *device, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name) {
		return r(read8_delegate(read, read_name, device->tag(), device));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(_devc *device, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name) {
		return r(read16_delegate(read, read_name, device->tag(), device));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(_devc *device, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name) {
		return r(read32_delegate(read, read_name, device->tag(), device));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(_devc *device, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name) {
		return r(read64_delegate(read, read_name, device->tag(), device));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(_devc *device, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return w(write8_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(_devc *device, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return w(write16_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(_devc *device, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return w(write32_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(_devc *device, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return w(write64_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(_devc *device, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return rw(read8_delegate(read, read_name, device->tag(), device),
				  write8_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(_devc *device, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return rw(read16_delegate(read, read_name, device->tag(), device),
				  write16_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(_devc *device, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return rw(read32_delegate(read, read_name, device->tag(), device),
				  write32_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(_devc *device, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return rw(read64_delegate(read, read_name, device->tag(), device),
				  write64_delegate(write, write_name, device->tag(), device));
	}

	// device pointer unwrapping
	template<typename _devc, typename R> address_map_entry &r(required_device<_devc> devptr, R read, const char *read_name) {
		return r(devptr.target(), read, read_name);
	}

	template<typename _devc, typename R> address_map_entry &r(optional_device<_devc> devptr, R read, const char *read_name) {
		return r(devptr.target(), read, read_name);
	}

	template<typename _devc, typename W> address_map_entry &w(required_device<_devc> devptr, W write, const char *write_name) {
		return w(devptr.target(), write, write_name);
	}

	template<typename _devc, typename W> address_map_entry &w(optional_device<_devc> devptr, W write, const char *write_name) {
		return w(devptr.target(), write, write_name);
	}

	template<typename _devc, typename R, typename W> address_map_entry &rw(required_device<_devc> devptr, R read, const char *read_name, W write, const char *write_name) {
		return rw(devptr.target(), read, read_name, write, write_name);
	}

	template<typename _devc, typename R, typename W> address_map_entry &rw(optional_device<_devc> devptr, R read, const char *read_name, W write, const char *write_name) {
		return rw(devptr.target(), read, read_name, write, write_name);
	}


	// lambda -> delegate converter
	template<typename _lr> address_map_entry &lr8(const char *name, _lr &&read) {
		return r(read8_delegate(read, name));
	}

	template<typename _lr> address_map_entry &lr16(const char *name, _lr &&read) {
		return r(read16_delegate(read, name));
	}

	template<typename _lr> address_map_entry &lr32(const char *name, _lr &&read) {
		return r(read32_delegate(read, name));
	}

	template<typename _lr> address_map_entry &lr64(const char *name, _lr &&read) {
		return r(read64_delegate(read, name));
	}

	template<typename _lw> address_map_entry &lw8(const char *name, _lw &&write) {
		return w(write8_delegate(write, name));
	}

	template<typename _lw> address_map_entry &lw16(const char *name, _lw &&write) {
		return w(write16_delegate(write, name));
	}

	template<typename _lw> address_map_entry &lw32(const char *name, _lw &&write) {
		return w(write32_delegate(write, name));
	}

	template<typename _lw> address_map_entry &lw64(const char *name, _lw &&write) {
		return w(write64_delegate(write, name));
	}

	template<typename _lr, typename _lw> address_map_entry &lrw8(const char *name, _lr &&read, _lw &&write) {
		return rw(read8_delegate(read, name), write8_delegate(write, name));
	}

	template<typename _lr, typename _lw> address_map_entry &lrw16(const char *name, _lr &&read, _lw &&write) {
		return rw(read16_delegate(read, name), write16_delegate(write, name));
	}

	template<typename _lr, typename _lw> address_map_entry &lrw32(const char *name, _lr &&read, _lw &&write) {
		return rw(read32_delegate(read, name), write32_delegate(write, name));
	}

	template<typename _lr, typename _lw> address_map_entry &lrw64(const char *name, _lr &&read, _lw &&write) {
		return rw(read64_delegate(read, name), write64_delegate(write, name));
	}

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
	u64                     m_mask;                 // mask for which lanes apply
	int                     m_cswidth;              // chip select width override
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
	address_map_constructor m_submap_delegate;

	// information used during processing
	void *                  m_memory;               // pointer to memory backing this entry

	// handler setters for 8-bit delegates
	address_map_entry &r(read8_delegate func);
	address_map_entry &w(write8_delegate func);
	address_map_entry &rw(read8_delegate rfunc, write8_delegate wfunc);

	// handler setters for 16-bit delegates
	address_map_entry &r(read16_delegate func);
	address_map_entry &w(write16_delegate func);
	address_map_entry &rw(read16_delegate rfunc, write16_delegate wfunc);

	// handler setters for 32-bit delegates
	address_map_entry &r(read32_delegate func);
	address_map_entry &w(write32_delegate func);
	address_map_entry &rw(read32_delegate rfunc, write32_delegate wfunc);

	// handler setters for 64-bit delegates
	address_map_entry &r(read64_delegate func);
	address_map_entry &w(write64_delegate func);
	address_map_entry &rw(read64_delegate rfunc, write64_delegate wfunc);

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
	address_map(const address_space &space, offs_t start, offs_t end, u64 unitmask, int cswidth, device_t &device, address_map_constructor submap_delegate);
	~address_map();

	// setters
	void global_mask(offs_t mask);
	void unmap_value_low() { m_unmapval = 0; }
	void unmap_value_high() { m_unmapval = ~0; }
	void unmap_value(u8 value) { m_unmapval = value; }

	// add a new entry of the given type
	address_map_entry &operator()(offs_t start, offs_t end);

	// public data
	int                             m_spacenum;     // space number of the map
	device_t *                      m_device;       // associated device
	u8                              m_unmapval;     // unmapped memory value
	offs_t                          m_globalmask;   // global mask
	simple_list<address_map_entry>  m_entrylist;    // list of entries

	void import_submaps(running_machine &machine, device_t &owner, int data_width, endianness_t endian);
	void map_validity_check(validity_checker &valid, int spacenum) const;
};


//**************************************************************************
//  ADDRESS MAP MACROS
//**************************************************************************

#define ADDRESS_MAP_START(_name) void _name(address_map &map) {
#define ADDRESS_MAP_END ;}

// global controls
#define ADDRESS_MAP_GLOBAL_MASK(_mask) \
	;map.global_mask(_mask)
#define ADDRESS_MAP_UNMAP_LOW \
	;map.unmap_value_low()
#define ADDRESS_MAP_UNMAP_HIGH \
	;map.unmap_value_high()

// importing data from other address maps
#define AM_IMPORT_FROM(_name) \
	;_name(map)

// address ranges
#define AM_RANGE(_start, _end) \
	;map(_start, _end)
#define AM_MASK(_mask) \
	.mask(_mask)
#define AM_MIRROR(_mirror) \
	.mirror(_mirror)
#define AM_SELECT(_select) \
	.select(_select)

// driver data reads
#define AM_READ(_handler) \
	.r(this, &std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler)
#define AM_READ8(_handler, _unitmask) \
	.r(this, &std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler).umask64(_unitmask)
#define AM_READ16(_handler, _unitmask) \
	.r(this, &std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler).umask64(_unitmask)
#define AM_READ32(_handler, _unitmask) \
	.r(this, &std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler).umask64(_unitmask)

// driver data writes
#define AM_WRITE(_handler) \
	.w(this, &std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler)
#define AM_WRITE8(_handler, _unitmask) \
	.w(this, &std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler).umask64(_unitmask)
#define AM_WRITE16(_handler, _unitmask) \
	.w(this, &std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler).umask64(_unitmask)
#define AM_WRITE32(_handler, _unitmask) \
	.w(this, &std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler).umask64(_unitmask)

// driver data reads/writes
#define AM_READWRITE(_rhandler, _whandler) \
	.rw(this, &std::remove_pointer_t<decltype(this)>::_rhandler, "driver_data::" #_rhandler, &std::remove_pointer_t<decltype(this)>::_whandler, "driver_data::" #_whandler)
#define AM_READWRITE8(_rhandler, _whandler, _unitmask) \
	.rw(this, &std::remove_pointer_t<decltype(this)>::_rhandler, "driver_data::" #_rhandler, &std::remove_pointer_t<decltype(this)>::_whandler, "driver_data::" #_whandler).umask64(_unitmask)
#define AM_READWRITE16(_rhandler, _whandler, _unitmask) \
	.rw(this, &std::remove_pointer_t<decltype(this)>::_rhandler, "driver_data::" #_rhandler, &std::remove_pointer_t<decltype(this)>::_whandler, "driver_data::" #_whandler).umask64(_unitmask)
#define AM_READWRITE32(_rhandler, _whandler, _unitmask) \
	.rw(this, &std::remove_pointer_t<decltype(this)>::_rhandler, "driver_data::" #_rhandler, &std::remove_pointer_t<decltype(this)>::_whandler, "driver_data::" #_whandler).umask64(_unitmask)

// driver set offset. Upcast to base class because there are no data width variants,
// and the compiler complains if we don't do it explicitly
#define AM_SETOFFSET(_handler) \
	.set_handler(setoffset_delegate(&std::remove_pointer_t<decltype(this)>::_handler, "driver_data::" #_handler, this))

// device reads
#define AM_DEVREAD(_tag, _class, _handler) \
	.r(_tag, &_class::_handler, #_class "::" #_handler)
#define AM_DEVREAD8(_tag, _class, _handler, _unitmask) \
	.r(_tag, &_class::_handler, #_class "::" #_handler).umask64(_unitmask)
#define AM_DEVREAD16(_tag, _class, _handler, _unitmask) \
	.r(_tag, &_class::_handler, #_class "::" #_handler).umask64(_unitmask)
#define AM_DEVREAD32(_tag, _class, _handler, _unitmask) \
	.r(_tag, &_class::_handler, #_class "::" #_handler).umask64(_unitmask)

// device writes
#define AM_DEVWRITE(_tag, _class, _handler) \
	.w(_tag, &_class::_handler, #_class "::" #_handler)
#define AM_DEVWRITE8(_tag, _class, _handler, _unitmask) \
	.w(_tag, &_class::_handler, #_class "::" #_handler).umask64(_unitmask)
#define AM_DEVWRITE16(_tag, _class, _handler, _unitmask) \
	.w(_tag, &_class::_handler, #_class "::" #_handler).umask64(_unitmask)
#define AM_DEVWRITE32(_tag, _class, _handler, _unitmask) \
	.w(_tag, &_class::_handler, #_class "::" #_handler).umask64(_unitmask)

// device reads/writes
#define AM_DEVREADWRITE(_tag, _class, _rhandler, _whandler) \
	.rw(_tag, &_class::_rhandler, #_class "::" #_rhandler, &_class::_whandler, #_class "::" #_whandler)
#define AM_DEVREADWRITE8(_tag, _class, _rhandler, _whandler, _unitmask) \
	.rw(_tag, &_class::_rhandler, #_class "::" #_rhandler, &_class::_whandler, #_class "::" #_whandler).umask64(_unitmask)
#define AM_DEVREADWRITE16(_tag, _class, _rhandler, _whandler, _unitmask) \
	.rw(_tag, &_class::_rhandler, #_class "::" #_rhandler, &_class::_whandler, #_class "::" #_whandler).umask64(_unitmask)
#define AM_DEVREADWRITE32(_tag, _class, _rhandler, _whandler, _unitmask) \
	.rw(_tag, &_class::_rhandler, #_class "::" #_rhandler, &_class::_whandler, #_class "::" #_whandler).umask64(_unitmask)

// device set offset
#define AM_DEVSETOFFSET(_tag, _class, _handler) \
	.set_handler(setoffset_delegate(&_class::_handler, #_class "::" #_handler, _tag, (_class *)nullptr))


// device mapping
#define AM_DEVICE(_tag, _class, _handler) \
	.m(_tag, address_map_constructor(&_class::_handler, #_class "::" #_handler, (_class *)nullptr))
#define AM_DEVICE8(_tag, _class, _handler, _unitmask) \
	.m(_tag, address_map_constructor(&_class::_handler, #_class "::" #_handler, (_class *)nullptr)).umask64(_unitmask)
#define AM_DEVICE16(_tag, _class, _handler, _unitmask) \
	.m(_tag, address_map_constructor(&_class::_handler, #_class "::" #_handler, (_class *)nullptr)).umask64(_unitmask)
#define AM_DEVICE32(_tag, _class, _handler, _unitmask) \
	.m(_tag, address_map_constructor(&_class::_handler, #_class "::" #_handler, (_class *)nullptr)).umask64(_unitmask)

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
