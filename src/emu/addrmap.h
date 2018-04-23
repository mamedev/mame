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
	address_map_entry &unmaprw() { m_read.m_type = AMH_UNMAP; m_write.m_type = AMH_UNMAP; return *this; }
	address_map_entry &unmapr() { m_read.m_type = AMH_UNMAP; return *this; }
	address_map_entry &unmapw() { m_write.m_type = AMH_UNMAP; return *this; }
	address_map_entry &noprw() { m_read.m_type = AMH_NOP; m_write.m_type = AMH_NOP; return *this; }
	address_map_entry &nopr() { m_read.m_type = AMH_NOP; return *this; }
	address_map_entry &nopw() { m_write.m_type = AMH_NOP; return *this; }

	// address mask setting
	address_map_entry &mask(offs_t _mask);

	// unit mask setting
	address_map_entry &umask16(u16 _mask);
	address_map_entry &umask32(u32 _mask);
	address_map_entry &umask64(u64 _mask);

	// chip select width setting
	address_map_entry &cswidth(int _cswidth) { m_cswidth = _cswidth; return *this; }

	// I/O port configuration
	address_map_entry &portr(const char *tag) { m_read.m_type = AMH_PORT; m_read.m_tag = tag; return *this; }
	address_map_entry &portw(const char *tag) { m_write.m_type = AMH_PORT; m_write.m_tag = tag; return *this; }
	address_map_entry &portrw(const char *tag) { portr(tag); portw(tag); return *this; }

	// memory bank configuration
	address_map_entry &bankr(const char *tag) { m_read.m_type = AMH_BANK; m_read.m_tag = tag; return *this; }
	address_map_entry &bankw(const char *tag) { m_write.m_type = AMH_BANK; m_write.m_tag = tag; return *this; }
	address_map_entry &bankrw(const char *tag) { bankr(tag); bankw(tag); return *this; }

	// set offset handler (only one version, since there is no data width to consider)
	address_map_entry &setoffset(setoffset_delegate func);

	// type setters
	address_map_entry &set_read_type(map_handler_type _type) { m_read.m_type = _type; return *this; }
	address_map_entry &set_write_type(map_handler_type _type) { m_write.m_type = _type; return *this; }

	// submap referencing
	address_map_entry &m(const char *tag, address_map_constructor func);
	address_map_entry &m(device_t *device, address_map_constructor func);


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

	template<typename _dev> address_map_entry &m(const char *tag, void (_dev::*map)(address_map &), const char *map_name) {
		return m(tag, address_map_constructor(map, map_name, (_dev *)nullptr));
	}

	template<typename _dev> address_map_entry &setoffset(const char *tag, void (_dev::*so)(address_space &, offs_t), const char *so_name) {
		return setoffset(setoffset_delegate(so, so_name, tag, (_dev *)nullptr));
	}


	// device pointer/finder -> delegate converter
	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(_devc *device, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name) {
		return r(read8_delegate(read, read_name, device->tag(), device));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(required_device<_devc> device, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name) {
		return r(read8_delegate(read, read_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(optional_device<_devc> device, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name) {
		return r(read8_delegate(read, read_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(_devc *device, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name) {
		return r(read16_delegate(read, read_name, device->tag(), device));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(required_device<_devc> device, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name) {
		return r(read16_delegate(read, read_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(optional_device<_devc> device, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name) {
		return r(read16_delegate(read, read_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(_devc *device, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name) {
		return r(read32_delegate(read, read_name, device->tag(), device));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(required_device<_devc> device, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name) {
		return r(read32_delegate(read, read_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(optional_device<_devc> device, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name) {
		return r(read32_delegate(read, read_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(_devc *device, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name) {
		return r(read64_delegate(read, read_name, device->tag(), device));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(required_device<_devc> device, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name) {
		return r(read64_delegate(read, read_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr> std::enable_if_t<std::is_base_of<_devr, _devc>::value, address_map_entry &> r(optional_device<_devc> device, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name) {
		return r(read64_delegate(read, read_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(_devc *device, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return w(write8_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(required_device<_devc> device, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return w(write8_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(optional_device<_devc> device, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return w(write8_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(_devc *device, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return w(write16_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(required_device<_devc> device, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return w(write16_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(optional_device<_devc> device, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return w(write16_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(_devc *device, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return w(write32_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(required_device<_devc> device, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return w(write32_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(optional_device<_devc> device, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return w(write32_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(_devc *device, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return w(write64_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(required_device<_devc> device, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return w(write64_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devw> std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &> w(optional_device<_devc> device, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return w(write64_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(_devc *device, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return rw(read8_delegate(read, read_name, device->tag(), device),
				  write8_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(required_device<_devc> device, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return rw(read8_delegate(read, read_name, device->tag(), device.target()),
				  write8_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(optional_device<_devc> device, u8 (_devr::*read)(address_space &, offs_t, u8), const char *read_name, void (_devw::*write)(address_space &, offs_t, u8, u8), const char *write_name) {
		return rw(read8_delegate(read, read_name, device->tag(), device.target()),
				  write8_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(_devc *device, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return rw(read16_delegate(read, read_name, device->tag(), device),
				  write16_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(required_device<_devc> device, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return rw(read16_delegate(read, read_name, device->tag(), device.target()),
				  write16_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(optional_device<_devc> device, u16 (_devr::*read)(address_space &, offs_t, u16), const char *read_name, void (_devw::*write)(address_space &, offs_t, u16, u16), const char *write_name) {
		return rw(read16_delegate(read, read_name, device->tag(), device.target()),
				  write16_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(_devc *device, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return rw(read32_delegate(read, read_name, device->tag(), device),
				  write32_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(required_device<_devc> device, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return rw(read32_delegate(read, read_name, device->tag(), device.target()),
				  write32_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(optional_device<_devc> device, u32 (_devr::*read)(address_space &, offs_t, u32), const char *read_name, void (_devw::*write)(address_space &, offs_t, u32, u32), const char *write_name) {
		return rw(read32_delegate(read, read_name, device->tag(), device.target()),
				  write32_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(_devc *device, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return rw(read64_delegate(read, read_name, device->tag(), device),
				  write64_delegate(write, write_name, device->tag(), device));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(required_device<_devc> device, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return rw(read64_delegate(read, read_name, device->tag(), device.target()),
				  write64_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devr, typename _devw> std::enable_if_t<std::is_base_of<_devr, _devc>::value, std::enable_if_t<std::is_base_of<_devw, _devc>::value, address_map_entry &>> rw(optional_device<_devc> device, u64 (_devr::*read)(address_space &, offs_t, u64), const char *read_name, void (_devw::*write)(address_space &, offs_t, u64, u64), const char *write_name) {
		return rw(read64_delegate(read, read_name, device->tag(), device.target()),
				  write64_delegate(write, write_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devm> std::enable_if_t<std::is_base_of<_devm, _devc>::value, address_map_entry &> m(_devc *device, void (_devm::*map)(address_map &), const char *map_name) {
		return m(device, address_map_constructor(map, map_name, device));
	}

	template<typename _devc, typename _devm> std::enable_if_t<std::is_base_of<_devm, _devc>::value, address_map_entry &> m(required_device<_devc> device, void (_devm::*map)(address_map &), const char *map_name) {
		return m(device.target(), address_map_constructor(map, map_name, device.target()));
	}

	template<typename _devc, typename _devm> std::enable_if_t<std::is_base_of<_devm, _devc>::value, address_map_entry &> m(optional_device<_devc> device, void (_devm::*map)(address_map &), const char *map_name) {
		return m(device.target(), address_map_constructor(map, map_name, device.target()));
	}

	template<typename _devc, typename _devs> std::enable_if_t<std::is_base_of<_devs, _devc>::value, address_map_entry &> &setoffset(_devc *device, void (_devs::*so)(address_space &, offs_t), const char *so_name) {
		return setoffset(setoffset_delegate(so, so_name, device->tag(), device));
	}

	template<typename _devc, typename _devs> std::enable_if_t<std::is_base_of<_devs, _devc>::value, address_map_entry &> &setoffset(required_device<_devc> device, void (_devs::*so)(address_space &, offs_t), const char *so_name) {
		return setoffset(setoffset_delegate(so, so_name, device->tag(), device.target()));
	}

	template<typename _devc, typename _devs> std::enable_if_t<std::is_base_of<_devs, _devc>::value, address_map_entry &> &setoffset(optional_device<_devc> device, void (_devs::*so)(address_space &, offs_t), const char *so_name) {
		return setoffset(setoffset_delegate(so, so_name, device->tag(), device.target()));
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
	device_t               *m_submap_device;
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

#endif  /* __ADDRMAP_H__ */
