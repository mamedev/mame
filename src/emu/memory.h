// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Olivier Galibert
/***************************************************************************

    memory.h

    Functions which handle device memory accesses.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MEMORY_H__
#define __MEMORY_H__



//**************************************************************************
//  CONSTANTS
//**************************************************************************

enum { TOTAL_MEMORY_BANKS = 512 };

// address spaces
enum address_spacenum
{
	AS_0,                           // first address space
	AS_1,                           // second address space
	AS_2,                           // third address space
	AS_3,                           // fourth address space
	ADDRESS_SPACES,                 // maximum number of address spaces

	// alternate address space names for common use
	AS_PROGRAM = AS_0,              // program address space
	AS_DATA = AS_1,                 // data address space
	AS_IO = AS_2,                   // I/O address space
	AS_DECRYPTED_OPCODES = AS_3     // decrypted opcodes, when separate from data accesses
};
DECLARE_ENUM_OPERATORS(address_spacenum)

// read or write constants
enum read_or_write
{
	ROW_READ = 1,
	ROW_WRITE = 2,
	ROW_READWRITE = 3
};



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// referenced types from other classes
class device_memory_interface;
class device_t;
struct game_driver;

// forward declarations of classes defined here
class address_map;
class address_map_entry;
class memory_manager;
class memory_bank;
class memory_block;
class memory_share;
class direct_read_data;
class address_space;
class address_table;
class address_table_read;
class address_table_write;
class address_table_setoffset;


// offsets and addresses are 32-bit (for now...)
typedef UINT32  offs_t;

// address map constructors are functions that build up an address_map
typedef void (*address_map_constructor)(address_map &map, device_t &devconfig);

// submap retriever delegate
typedef delegate<void (address_map &, device_t &)> address_map_delegate;

// struct with function pointers for accessors; use is generally discouraged unless necessary
struct data_accessors
{
	UINT8       (*read_byte)(address_space &space, offs_t byteaddress);
	UINT16      (*read_word)(address_space &space, offs_t byteaddress);
	UINT16      (*read_word_masked)(address_space &space, offs_t byteaddress, UINT16 mask);
	UINT32      (*read_dword)(address_space &space, offs_t byteaddress);
	UINT32      (*read_dword_masked)(address_space &space, offs_t byteaddress, UINT32 mask);
	UINT64      (*read_qword)(address_space &space, offs_t byteaddress);
	UINT64      (*read_qword_masked)(address_space &space, offs_t byteaddress, UINT64 mask);

	void        (*write_byte)(address_space &space, offs_t byteaddress, UINT8 data);
	void        (*write_word)(address_space &space, offs_t byteaddress, UINT16 data);
	void        (*write_word_masked)(address_space &space, offs_t byteaddress, UINT16 data, UINT16 mask);
	void        (*write_dword)(address_space &space, offs_t byteaddress, UINT32 data);
	void        (*write_dword_masked)(address_space &space, offs_t byteaddress, UINT32 data, UINT32 mask);
	void        (*write_qword)(address_space &space, offs_t byteaddress, UINT64 data);
	void        (*write_qword_masked)(address_space &space, offs_t byteaddress, UINT64 data, UINT64 mask);
};


// ======================> direct_update_delegate

// direct region update handler
typedef delegate<offs_t (direct_read_data &, offs_t)> direct_update_delegate;


// ======================> read_delegate

// declare delegates for each width
typedef device_delegate<UINT8 (address_space &, offs_t, UINT8)> read8_delegate;
typedef device_delegate<UINT16 (address_space &, offs_t, UINT16)> read16_delegate;
typedef device_delegate<UINT32 (address_space &, offs_t, UINT32)> read32_delegate;
typedef device_delegate<UINT64 (address_space &, offs_t, UINT64)> read64_delegate;


// ======================> write_delegate

// declare delegates for each width
typedef device_delegate<void (address_space &, offs_t, UINT8, UINT8)> write8_delegate;
typedef device_delegate<void (address_space &, offs_t, UINT16, UINT16)> write16_delegate;
typedef device_delegate<void (address_space &, offs_t, UINT32, UINT32)> write32_delegate;
typedef device_delegate<void (address_space &, offs_t, UINT64, UINT64)> write64_delegate;

// ======================> setoffset_delegate

typedef device_delegate<void (address_space &, offs_t)> setoffset_delegate;


// ======================> direct_read_data

// direct_read_data contains state data for direct read access
class direct_read_data
{
	friend class address_table;

public:
	// direct_range is an internal class that is part of a list of start/end ranges
	class direct_range
	{
	public:
		// construction
		direct_range()
			: m_next(NULL),
				m_bytestart(0),
				m_byteend(~0) { }

		// getters
		direct_range *next() const { return m_next; }

		// internal state
		direct_range *          m_next;                 // pointer to the next range in the list
		offs_t                  m_bytestart;            // starting byte offset of the range
		offs_t                  m_byteend;              // ending byte offset of the range
	};

	// construction/destruction
	direct_read_data(address_space &space);
	~direct_read_data();

	// getters
	address_space &space() const { return m_space; }
	UINT8 *ptr() const { return m_ptr; }

	// see if an address is within bounds, or attempt to update it if not
	bool address_is_valid(offs_t byteaddress) { return EXPECTED(byteaddress >= m_bytestart && byteaddress <= m_byteend) || set_direct_region(byteaddress); }

	// force a recomputation on the next read
	void force_update() { m_byteend = 0; m_bytestart = 1; }
	void force_update(UINT16 if_match) { if (m_entry == if_match) force_update(); }

	// custom update callbacks and configuration
	direct_update_delegate set_direct_update(direct_update_delegate function);
	void explicit_configure(offs_t bytestart, offs_t byteend, offs_t bytemask, void *raw);

	// accessor methods
	void *read_ptr(offs_t byteaddress, offs_t directxor = 0);
	UINT8 read_byte(offs_t byteaddress, offs_t directxor = 0);
	UINT16 read_word(offs_t byteaddress, offs_t directxor = 0);
	UINT32 read_dword(offs_t byteaddress, offs_t directxor = 0);
	UINT64 read_qword(offs_t byteaddress, offs_t directxor = 0);

private:
	// internal helpers
	bool set_direct_region(offs_t &byteaddress);
	direct_range *find_range(offs_t byteaddress, UINT16 &entry);
	void remove_intersecting_ranges(offs_t bytestart, offs_t byteend);

	// internal state
	address_space &             m_space;
	UINT8 *                     m_ptr;                  // direct access data pointer
	offs_t                      m_bytemask;             // byte address mask
	offs_t                      m_bytestart;            // minimum valid byte address
	offs_t                      m_byteend;              // maximum valid byte address
	UINT16                      m_entry;                // live entry
	simple_list<direct_range>   m_rangelist[TOTAL_MEMORY_BANKS];  // list of ranges for each entry
	simple_list<direct_range>   m_freerangelist;        // list of recycled range entries
	direct_update_delegate      m_directupdate;         // fast direct-access update callback
};


// ======================> address_space_config

// describes an address space and provides basic functions to map addresses to bytes
class address_space_config
{
public:
	// construction/destruction
	address_space_config();
	address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift = 0, address_map_constructor internal = NULL, address_map_constructor defmap = NULL);
	address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, UINT8 logwidth, UINT8 pageshift, address_map_constructor internal = NULL, address_map_constructor defmap = NULL);
	address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, address_map_delegate internal, address_map_delegate defmap = address_map_delegate());
	address_space_config(const char *name, endianness_t endian, UINT8 datawidth, UINT8 addrwidth, INT8 addrshift, UINT8 logwidth, UINT8 pageshift, address_map_delegate internal, address_map_delegate defmap = address_map_delegate());

	// getters
	const char *name() const { return m_name; }
	endianness_t endianness() const { return m_endianness; }
	int data_width() const { return m_databus_width; }
	int addr_width() const { return m_addrbus_width; }

	// address-to-byte conversion helpers
	inline offs_t addr2byte(offs_t address) const { return (m_addrbus_shift < 0) ? (address << -m_addrbus_shift) : (address >> m_addrbus_shift); }
	inline offs_t addr2byte_end(offs_t address) const { return (m_addrbus_shift < 0) ? ((address << -m_addrbus_shift) | ((1 << -m_addrbus_shift) - 1)) : (address >> m_addrbus_shift); }
	inline offs_t byte2addr(offs_t address) const { return (m_addrbus_shift > 0) ? (address << m_addrbus_shift) : (address >> -m_addrbus_shift); }
	inline offs_t byte2addr_end(offs_t address) const { return (m_addrbus_shift > 0) ? ((address << m_addrbus_shift) | ((1 << m_addrbus_shift) - 1)) : (address >> -m_addrbus_shift); }

	// state
	const char *        m_name;
	endianness_t        m_endianness;
	UINT8               m_databus_width;
	UINT8               m_addrbus_width;
	INT8                m_addrbus_shift;
	UINT8               m_logaddr_width;
	UINT8               m_page_shift;
	address_map_constructor m_internal_map;
	address_map_constructor m_default_map;
	address_map_delegate m_internal_map_delegate;
	address_map_delegate m_default_map_delegate;
};


// ======================> address_space

// address_space holds live information about an address space
class address_space
{
	friend class address_table;
	friend class address_table_read;
	friend class address_table_write;
	friend class address_table_setoffset;
	friend class direct_read_data;
	friend class simple_list<address_space>;
	friend resource_pool_object<address_space>::~resource_pool_object();

protected:
	// construction/destruction
	address_space(memory_manager &manager, device_memory_interface &memory, address_spacenum spacenum, bool large);
	virtual ~address_space();

public:
	// public allocator
	static address_space &allocate(memory_manager &manager, const address_space_config &config, device_memory_interface &memory, address_spacenum spacenum);

	// getters
	address_space *next() const { return m_next; }
	memory_manager &manager() const { return m_manager; }
	device_t &device() const { return m_device; }
	running_machine &machine() const { return m_machine; }
	const char *name() const { return m_name; }
	address_spacenum spacenum() const { return m_spacenum; }
	address_map *map() const { return m_map.get(); }

	direct_read_data &direct() const { return *m_direct; }

	int data_width() const { return m_config.data_width(); }
	int addr_width() const { return m_config.addr_width(); }
	endianness_t endianness() const { return m_config.endianness(); }
	UINT64 unmap() const { return m_unmap; }

	offs_t addrmask() const { return m_addrmask; }
	offs_t bytemask() const { return m_bytemask; }
	UINT8 addrchars() const { return m_addrchars; }
	offs_t logaddrmask() const { return m_logaddrmask; }
	offs_t logbytemask() const { return m_logbytemask; }
	UINT8 logaddrchars() const { return m_logaddrchars; }

	// debug helpers
	const char *get_handler_string(read_or_write readorwrite, offs_t byteaddress);
	bool debugger_access() const { return m_debugger_access; }
	void set_debugger_access(bool debugger) { m_debugger_access = debugger; }
	bool log_unmap() const { return m_log_unmap; }
	void set_log_unmap(bool log) { m_log_unmap = log; }
	void dump_map(FILE *file, read_or_write readorwrite);

	// watchpoint enablers
	virtual void enable_read_watchpoints(bool enable = true) = 0;
	virtual void enable_write_watchpoints(bool enable = true) = 0;

	// general accessors
	virtual void accessors(data_accessors &accessors) const = 0;
	virtual void *get_read_ptr(offs_t byteaddress) = 0;
	virtual void *get_write_ptr(offs_t byteaddress) = 0;

	// read accessors
	virtual UINT8 read_byte(offs_t byteaddress) = 0;
	virtual UINT16 read_word(offs_t byteaddress) = 0;
	virtual UINT16 read_word(offs_t byteaddress, UINT16 mask) = 0;
	virtual UINT16 read_word_unaligned(offs_t byteaddress) = 0;
	virtual UINT16 read_word_unaligned(offs_t byteaddress, UINT16 mask) = 0;
	virtual UINT32 read_dword(offs_t byteaddress) = 0;
	virtual UINT32 read_dword(offs_t byteaddress, UINT32 mask) = 0;
	virtual UINT32 read_dword_unaligned(offs_t byteaddress) = 0;
	virtual UINT32 read_dword_unaligned(offs_t byteaddress, UINT32 mask) = 0;
	virtual UINT64 read_qword(offs_t byteaddress) = 0;
	virtual UINT64 read_qword(offs_t byteaddress, UINT64 mask) = 0;
	virtual UINT64 read_qword_unaligned(offs_t byteaddress) = 0;
	virtual UINT64 read_qword_unaligned(offs_t byteaddress, UINT64 mask) = 0;

	// write accessors
	virtual void write_byte(offs_t byteaddress, UINT8 data) = 0;
	virtual void write_word(offs_t byteaddress, UINT16 data) = 0;
	virtual void write_word(offs_t byteaddress, UINT16 data, UINT16 mask) = 0;
	virtual void write_word_unaligned(offs_t byteaddress, UINT16 data) = 0;
	virtual void write_word_unaligned(offs_t byteaddress, UINT16 data, UINT16 mask) = 0;
	virtual void write_dword(offs_t byteaddress, UINT32 data) = 0;
	virtual void write_dword(offs_t byteaddress, UINT32 data, UINT32 mask) = 0;
	virtual void write_dword_unaligned(offs_t byteaddress, UINT32 data) = 0;
	virtual void write_dword_unaligned(offs_t byteaddress, UINT32 data, UINT32 mask) = 0;
	virtual void write_qword(offs_t byteaddress, UINT64 data) = 0;
	virtual void write_qword(offs_t byteaddress, UINT64 data, UINT64 mask) = 0;
	virtual void write_qword_unaligned(offs_t byteaddress, UINT64 data) = 0;
	virtual void write_qword_unaligned(offs_t byteaddress, UINT64 data, UINT64 mask) = 0;

	// Set address. This will invoke setoffset handlers for the respective entries.
	virtual void set_address(offs_t byteaddress) = 0;

	// address-to-byte conversion helpers
	offs_t address_to_byte(offs_t address) const { return m_config.addr2byte(address); }
	offs_t address_to_byte_end(offs_t address) const { return m_config.addr2byte_end(address); }
	offs_t byte_to_address(offs_t address) const { return m_config.byte2addr(address); }
	offs_t byte_to_address_end(offs_t address) const { return m_config.byte2addr_end(address); }

	// direct access
	direct_update_delegate set_direct_update_handler(direct_update_delegate function) { return m_direct->set_direct_update(function); }

	// umap ranges (short form)
	void unmap_read(offs_t addrstart, offs_t addrend) { unmap_read(addrstart, addrend, 0, 0); }
	void unmap_write(offs_t addrstart, offs_t addrend) { unmap_write(addrstart, addrend, 0, 0); }
	void unmap_readwrite(offs_t addrstart, offs_t addrend) { unmap_readwrite(addrstart, addrend, 0, 0); }
	void nop_read(offs_t addrstart, offs_t addrend) { nop_read(addrstart, addrend, 0, 0); }
	void nop_write(offs_t addrstart, offs_t addrend) { nop_write(addrstart, addrend, 0, 0); }
	void nop_readwrite(offs_t addrstart, offs_t addrend) { nop_readwrite(addrstart, addrend, 0, 0); }

	// umap ranges (with mirror/mask)
	void unmap_read(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror) { unmap_generic(addrstart, addrend, addrmask, addrmirror, ROW_READ, false); }
	void unmap_write(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror) { unmap_generic(addrstart, addrend, addrmask, addrmirror, ROW_WRITE, false); }
	void unmap_readwrite(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror) { unmap_generic(addrstart, addrend, addrmask, addrmirror, ROW_READWRITE, false); }
	void nop_read(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror) { unmap_generic(addrstart, addrend, addrmask, addrmirror, ROW_READ, true); }
	void nop_write(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror) { unmap_generic(addrstart, addrend, addrmask, addrmirror, ROW_WRITE, true); }
	void nop_readwrite(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror) { unmap_generic(addrstart, addrend, addrmask, addrmirror, ROW_READWRITE, true); }

	// install ports, banks, RAM (short form)
	void install_read_port(offs_t addrstart, offs_t addrend, const char *rtag) { install_read_port(addrstart, addrend, 0, 0, rtag); }
	void install_write_port(offs_t addrstart, offs_t addrend, const char *wtag) { install_write_port(addrstart, addrend, 0, 0, wtag); }
	void install_readwrite_port(offs_t addrstart, offs_t addrend, const char *rtag, const char *wtag) { install_readwrite_port(addrstart, addrend, 0, 0, rtag, wtag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_read_bank(addrstart, addrend, 0, 0, tag); }
	void install_write_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_write_bank(addrstart, addrend, 0, 0, tag); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, const char *tag) { install_readwrite_bank(addrstart, addrend, 0, 0, tag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_read_bank(addrstart, addrend, 0, 0, bank); }
	void install_write_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_write_bank(addrstart, addrend, 0, 0, bank); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, memory_bank *bank) { install_readwrite_bank(addrstart, addrend, 0, 0, bank); }
	void *install_rom(offs_t addrstart, offs_t addrend, void *baseptr = NULL) { return install_rom(addrstart, addrend, 0, 0, baseptr); }
	void *install_writeonly(offs_t addrstart, offs_t addrend, void *baseptr = NULL) { return install_writeonly(addrstart, addrend, 0, 0, baseptr); }
	void *install_ram(offs_t addrstart, offs_t addrend, void *baseptr = NULL) { return install_ram(addrstart, addrend, 0, 0, baseptr); }

	// install ports, banks, RAM (with mirror/mask)
	void install_read_port(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag) { install_readwrite_port(addrstart, addrend, addrmask, addrmirror, rtag, NULL); }
	void install_write_port(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *wtag) { install_readwrite_port(addrstart, addrend, addrmask, addrmirror, NULL, wtag); }
	void install_readwrite_port(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag);
	void install_read_bank(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *tag) { install_bank_generic(addrstart, addrend, addrmask, addrmirror, tag, NULL); }
	void install_write_bank(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *tag) { install_bank_generic(addrstart, addrend, addrmask, addrmirror, NULL, tag); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *tag)  { install_bank_generic(addrstart, addrend, addrmask, addrmirror, tag, tag); }
	void install_read_bank(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, memory_bank *bank) { install_bank_generic(addrstart, addrend, addrmask, addrmirror, bank, NULL); }
	void install_write_bank(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, memory_bank *bank) { install_bank_generic(addrstart, addrend, addrmask, addrmirror, NULL, bank); }
	void install_readwrite_bank(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, memory_bank *bank)  { install_bank_generic(addrstart, addrend, addrmask, addrmirror, bank, bank); }
	void *install_rom(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, void *baseptr = NULL) { return install_ram_generic(addrstart, addrend, addrmask, addrmirror, ROW_READ, baseptr); }
	void *install_writeonly(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, void *baseptr = NULL) { return install_ram_generic(addrstart, addrend, addrmask, addrmirror, ROW_WRITE, baseptr); }
	void *install_ram(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, void *baseptr = NULL) { return install_ram_generic(addrstart, addrend, addrmask, addrmirror, ROW_READWRITE, baseptr); }

	// install device memory maps
	template <typename T> void install_device(offs_t addrstart, offs_t addrend, T &device, void (T::*map)(address_map &map, device_t &device), int bits = 0, UINT64 unitmask = 0) {
		address_map_delegate delegate(map, "dynamic_device_install", &device);
		install_device_delegate(addrstart, addrend, device, delegate, bits, unitmask);
	}

	void install_device_delegate(offs_t addrstart, offs_t addrend, device_t &device, address_map_delegate &map, int bits = 0, UINT64 unitmask = 0);

	// install setoffset handler
	void install_setoffset_handler(offs_t addrstart, offs_t addrend, setoffset_delegate sohandler, UINT64 unitmask = 0) { return install_setoffset_handler(addrstart, addrend, 0, 0, sohandler, unitmask); }
	void install_setoffset_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, setoffset_delegate sohandler, UINT64 unitmask = 0);

	// install new-style delegate handlers (short form)
	UINT8 *install_read_handler(offs_t addrstart, offs_t addrend, read8_delegate rhandler, UINT64 unitmask = 0) { return install_read_handler(addrstart, addrend, 0, 0, rhandler, unitmask); }
	UINT8 *install_write_handler(offs_t addrstart, offs_t addrend, write8_delegate whandler, UINT64 unitmask = 0) { return install_write_handler(addrstart, addrend, 0, 0, whandler, unitmask); }
	UINT8 *install_readwrite_handler(offs_t addrstart, offs_t addrend, read8_delegate rhandler, write8_delegate whandler, UINT64 unitmask = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, rhandler, whandler, unitmask); }
	UINT16 *install_read_handler(offs_t addrstart, offs_t addrend, read16_delegate rhandler, UINT64 unitmask = 0) { return install_read_handler(addrstart, addrend, 0, 0, rhandler, unitmask); }
	UINT16 *install_write_handler(offs_t addrstart, offs_t addrend, write16_delegate whandler, UINT64 unitmask = 0) { return install_write_handler(addrstart, addrend, 0, 0, whandler, unitmask); }
	UINT16 *install_readwrite_handler(offs_t addrstart, offs_t addrend, read16_delegate rhandler, write16_delegate whandler, UINT64 unitmask = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, rhandler, whandler, unitmask); }
	UINT32 *install_read_handler(offs_t addrstart, offs_t addrend, read32_delegate rhandler, UINT64 unitmask = 0) { return install_read_handler(addrstart, addrend, 0, 0, rhandler, unitmask); }
	UINT32 *install_write_handler(offs_t addrstart, offs_t addrend, write32_delegate whandler, UINT64 unitmask = 0) { return install_write_handler(addrstart, addrend, 0, 0, whandler, unitmask); }
	UINT32 *install_readwrite_handler(offs_t addrstart, offs_t addrend, read32_delegate rhandler, write32_delegate whandler, UINT64 unitmask = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, rhandler, whandler, unitmask); }
	UINT64 *install_read_handler(offs_t addrstart, offs_t addrend, read64_delegate rhandler, UINT64 unitmask = 0) { return install_read_handler(addrstart, addrend, 0, 0, rhandler, unitmask); }
	UINT64 *install_write_handler(offs_t addrstart, offs_t addrend, write64_delegate whandler, UINT64 unitmask = 0) { return install_write_handler(addrstart, addrend, 0, 0, whandler, unitmask); }
	UINT64 *install_readwrite_handler(offs_t addrstart, offs_t addrend, read64_delegate rhandler, write64_delegate whandler, UINT64 unitmask = 0) { return install_readwrite_handler(addrstart, addrend, 0, 0, rhandler, whandler, unitmask); }

	// install new-style delegate handlers (with mirror/mask)
	UINT8 *install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_delegate rhandler, UINT64 unitmask = 0);
	UINT8 *install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_delegate whandler, UINT64 unitmask = 0);
	UINT8 *install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_delegate rhandler, write8_delegate whandler, UINT64 unitmask = 0);
	UINT16 *install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_delegate rhandler, UINT64 unitmask = 0);
	UINT16 *install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_delegate whandler, UINT64 unitmask = 0);
	UINT16 *install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_delegate rhandler, write16_delegate whandler, UINT64 unitmask = 0);
	UINT32 *install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_delegate rhandler, UINT64 unitmask = 0);
	UINT32 *install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_delegate whandler, UINT64 unitmask = 0);
	UINT32 *install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_delegate rhandler, write32_delegate whandler, UINT64 unitmask = 0);
	UINT64 *install_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_delegate rhandler, UINT64 unitmask = 0);
	UINT64 *install_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_delegate whandler, UINT64 unitmask = 0);
	UINT64 *install_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_delegate rhandler, write64_delegate whandler, UINT64 unitmask = 0);

	// setup
	void prepare_map();
	void populate_from_map(address_map *map = NULL);
	void allocate_memory();
	void locate_memory();

private:
	// internal helpers
	virtual address_table_read &read() = 0;
	virtual address_table_write &write() = 0;
	virtual address_table_setoffset &setoffset() = 0;

	void populate_map_entry(const address_map_entry &entry, read_or_write readorwrite);
	void populate_map_entry_setoffset(const address_map_entry &entry);
	void unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite, bool quiet);
	void *install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite, void *baseptr);
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag);
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, memory_bank *rbank, memory_bank *wbank);
	void adjust_addresses(offs_t &start, offs_t &end, offs_t &mask, offs_t &mirror);
	void *find_backing_memory(offs_t addrstart, offs_t addrend);
	bool needs_backing_store(const address_map_entry *entry);
	memory_bank &bank_find_or_allocate(const char *tag, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite);
	address_map_entry *block_assign_intersecting(offs_t bytestart, offs_t byteend, UINT8 *base);

protected:
	// private state
	address_space *         m_next;             // next address space in the global list
	const address_space_config &m_config;       // configuration of this space
	device_t &              m_device;           // reference to the owning device
	std::unique_ptr<address_map> m_map;            // original memory map
	offs_t                  m_addrmask;         // physical address mask
	offs_t                  m_bytemask;         // byte-converted physical address mask
	offs_t                  m_logaddrmask;      // logical address mask
	offs_t                  m_logbytemask;      // byte-converted logical address mask
	UINT64                  m_unmap;            // unmapped value
	address_spacenum        m_spacenum;         // address space index
	bool                    m_debugger_access;  // treat accesses as coming from the debugger
	bool                    m_log_unmap;        // log unmapped accesses in this space?
	std::unique_ptr<direct_read_data> m_direct;    // fast direct-access read info
	const char *            m_name;             // friendly name of the address space
	UINT8                   m_addrchars;        // number of characters to use for physical addresses
	UINT8                   m_logaddrchars;     // number of characters to use for logical addresses

private:
	memory_manager &        m_manager;          // reference to the owning manager
	running_machine &       m_machine;          // reference to the owning machine
};


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
	running_machine &machine() const { return m_machine; }
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
	memory_block *          m_next;                 // next memory block in the list
	running_machine &       m_machine;              // need the machine to free our memory
	address_space &         m_space;                // which address space are we associated with?
	offs_t                  m_bytestart, m_byteend; // byte-normalized start/end for verifying a match
	UINT8 *                 m_data;                 // pointer to the data for this block
	dynamic_buffer          m_allocated;            // pointer to the actually allocated block
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
		bank_reference *        m_next;             // link to the next reference
		address_space &         m_space;            // address space that references us
		read_or_write           m_readorwrite;      // used for read or write?
	};

	// a bank_entry contains a pointer
	struct bank_entry
	{
		UINT8 *         m_ptr;
	};

public:
	// construction/destruction
	memory_bank(address_space &space, int index, offs_t bytestart, offs_t byteend, const char *tag = NULL);
	~memory_bank();

	// getters
	memory_bank *next() const { return m_next; }
	running_machine &machine() const { return m_machine; }
	int index() const { return m_index; }
	int entry() const { return m_curentry; }
	bool anonymous() const { return m_anonymous; }
	offs_t bytestart() const { return m_bytestart; }
	void *base() const { return *m_baseptr; }
	const char *tag() const { return m_tag.c_str(); }
	const char *name() const { return m_name.c_str(); }

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

	// configure and set entries
	void configure_entry(int entrynum, void *base);
	void configure_entries(int startentry, int numentries, void *base, offs_t stride);
	void set_entry(int entrynum);

private:
	// internal helpers
	void invalidate_references();
	void expand_entries(int entrynum);

	// internal state
	memory_bank *           m_next;                 // next bank in sequence
	running_machine &       m_machine;              // need the machine to free our memory
	UINT8 **                m_baseptr;              // pointer to our base pointer in the global array
	UINT16                  m_index;                // array index for this handler
	bool                    m_anonymous;            // are we anonymous or explicit?
	offs_t                  m_bytestart;            // byte-adjusted start offset
	offs_t                  m_byteend;              // byte-adjusted end offset
	int                     m_curentry;             // current entry
	std::vector<bank_entry> m_entry;                // array of entries (dynamically allocated)
	std::string             m_name;                 // friendly name for this bank
	std::string             m_tag;                  // tag for this bank
	simple_list<bank_reference> m_reflist;          // linked list of address spaces referencing this bank
};


// ======================> memory_share

// a memory share contains information about shared memory region
class memory_share
{
	friend class simple_list<memory_share>;

public:
	// construction/destruction
	memory_share(UINT8 width, size_t bytes, endianness_t endianness, void *ptr = NULL)
		: m_next(NULL),
			m_ptr(ptr),
			m_bytes(bytes),
			m_endianness(endianness),
			m_bitwidth(width),
			m_bytewidth(width <= 8 ? 1 : width <= 16 ? 2 : width <= 32 ? 4 : 8)
	{ }

	// getters
	memory_share *next() const { return m_next; }
	// NOTE: this being NULL in a C++ member function can lead to undefined behavior.
	// However, it is relied on throughout MAME, so will remain for now.
	void *ptr() const { if (this == NULL) return NULL; return m_ptr; }
	size_t bytes() const { return m_bytes; }
	endianness_t endianness() const { return m_endianness; }
	UINT8 bitwidth() const { return m_bitwidth; }
	UINT8 bytewidth() const { return m_bytewidth; }

	// setters
	void set_ptr(void *ptr) { m_ptr = ptr; }

private:
	// internal state
	memory_share *          m_next;                 // next share in the list
	void *                  m_ptr;                  // pointer to the memory backing the region
	size_t                  m_bytes;                // size of the shared region in bytes
	endianness_t            m_endianness;           // endianness of the memory
	UINT8                   m_bitwidth;             // width of the shared region in bits
	UINT8                   m_bytewidth;            // width in bytes, rounded up to a power of 2

};


// ======================> memory_region

// memory region object
class memory_region
{
	DISABLE_COPYING(memory_region);

	friend class memory_manager;
	friend class simple_list<memory_region>;
	friend resource_pool_object<memory_region>::~resource_pool_object();

	// construction/destruction
	memory_region(running_machine &machine, const char *name, UINT32 length, UINT8 width, endianness_t endian);

public:
	// getters
	running_machine &machine() const { return m_machine; }
	memory_region *next() const { return m_next; }
	UINT8 *base() { return (this != NULL) ? &m_buffer[0] : NULL; }
	UINT8 *end() { return (this != NULL) ? base() + m_buffer.size() : NULL; }
	UINT32 bytes() const { return (this != NULL) ? m_buffer.size() : 0; }
	const char *name() const { return m_name.c_str(); }

	// flag expansion
	endianness_t endianness() const { return m_endianness; }
	UINT8 bitwidth() const { return m_bitwidth; }
	UINT8 bytewidth() const { return m_bytewidth; }

	// data access
	UINT8 &u8(offs_t offset = 0) { return m_buffer[offset]; }
	UINT16 &u16(offs_t offset = 0) { return reinterpret_cast<UINT16 *>(base())[offset]; }
	UINT32 &u32(offs_t offset = 0) { return reinterpret_cast<UINT32 *>(base())[offset]; }
	UINT64 &u64(offs_t offset = 0) { return reinterpret_cast<UINT64 *>(base())[offset]; }

private:
	// internal data
	running_machine &       m_machine;
	memory_region *         m_next;
	std::string             m_name;
	dynamic_buffer          m_buffer;
	endianness_t            m_endianness;
	UINT8                   m_bitwidth;
	UINT8                   m_bytewidth;
};



// ======================> memory_manager

// holds internal state for the memory system
class memory_manager
{
	friend class address_space;
	friend class address_table;
	friend class device_t;
	friend class memory_block;

public:
	// construction/destruction
	memory_manager(running_machine &machine);
	void initialize();

	// getters
	running_machine &machine() const { return m_machine; }
	address_space *first_space() const { return m_spacelist.first(); }
	memory_region *first_region() const { return m_regionlist.first(); }

	// dump the internal memory tables to the given file
	void dump(FILE *file);

	// pointers to a bank pointer (internal usage only)
	UINT8 **bank_pointer_addr(UINT8 index) { return &m_bank_ptr[index]; }

	// regions
	memory_region *region_alloc(const char *name, UINT32 length, UINT8 width, endianness_t endian);
	void region_free(const char *name);

private:
	// internal helpers
	memory_bank *first_bank() const { return m_banklist.first(); }
	memory_bank *bank(const char *tag) const { return m_banklist.find(tag); }
	memory_region *region(const char *tag) { return m_regionlist.find(tag); }
	memory_share *shared(const char *tag) { return m_sharelist.find(tag); }
	void bank_reattach();

	// internal state
	running_machine &           m_machine;              // reference to the machine
	bool                        m_initialized;          // have we completed initialization?

	UINT8 *                     m_bank_ptr[TOTAL_MEMORY_BANKS];  // array of bank pointers

	simple_list<address_space>  m_spacelist;            // list of address spaces
	simple_list<memory_block>   m_blocklist;            // head of the list of memory blocks

	tagged_list<memory_bank>    m_banklist;             // data gathered for each bank
	UINT16                      m_banknext;             // next bank to allocate

	tagged_list<memory_share>   m_sharelist;            // map for share lookups

	tagged_list<memory_region>  m_regionlist;           // list of memory regions
};



//**************************************************************************
//  MACROS
//**************************************************************************

// opcode base adjustment handler function macro
#define DIRECT_UPDATE_MEMBER(name)      offs_t name(ATTR_UNUSED direct_read_data &direct, ATTR_UNUSED offs_t address)
#define DECLARE_DIRECT_UPDATE_MEMBER(name)  offs_t name(ATTR_UNUSED direct_read_data &direct, ATTR_UNUSED offs_t address)



// space read/write handler function macros
#define READ8_MEMBER(name)              UINT8  name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 mem_mask)
#define WRITE8_MEMBER(name)             void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask)
#define READ16_MEMBER(name)             UINT16 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
#define READ32_MEMBER(name)             UINT32 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask)
#define WRITE32_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask)
#define READ64_MEMBER(name)             UINT64 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask)
#define WRITE64_MEMBER(name)            void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask)

#define DECLARE_READ8_MEMBER(name)      UINT8  name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 mem_mask = 0xff)
#define DECLARE_WRITE8_MEMBER(name)     void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask = 0xff)
#define DECLARE_READ16_MEMBER(name)     UINT16 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask = 0xffff)
#define DECLARE_WRITE16_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask = 0xffff)
#define DECLARE_READ32_MEMBER(name)     UINT32 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask = 0xffffffff)
#define DECLARE_WRITE32_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask = 0xffffffff)
#define DECLARE_READ64_MEMBER(name)     UINT64 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask = U64(0xffffffffffffffff))
#define DECLARE_WRITE64_MEMBER(name)    void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask = U64(0xffffffffffffffff))

#define SETOFFSET_MEMBER(name)          void  name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset)
#define DECLARE_SETOFFSET_MEMBER(name)      void  name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset)

// device delegate macros
#define READ8_DELEGATE(_class, _member)                     read8_delegate(FUNC(_class::_member), this)
#define WRITE8_DELEGATE(_class, _member)                    write8_delegate(FUNC(_class::_member), this)
#define READ16_DELEGATE(_class, _member)                    read16_delegate(FUNC(_class::_member), this)
#define WRITE16_DELEGATE(_class, _member)                   write16_delegate(FUNC(_class::_member), this)
#define READ32_DELEGATE(_class, _member)                    read32_delegate(FUNC(_class::_member), this)
#define WRITE32_DELEGATE(_class, _member)                   write32_delegate(FUNC(_class::_member), this)
#define READ64_DELEGATE(_class, _member)                    read64_delegate(FUNC(_class::_member), this)
#define WRITE64_DELEGATE(_class, _member)                   write64_delegate(FUNC(_class::_member), this)

#define READ8_DEVICE_DELEGATE(_device, _class, _member)     read8_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE8_DEVICE_DELEGATE(_device, _class, _member)    write8_delegate(FUNC(_class::_member), (_class *)_device)
#define READ16_DEVICE_DELEGATE(_device, _class, _member)    read16_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE16_DEVICE_DELEGATE(_device, _class, _member)   write16_delegate(FUNC(_class::_member), (_class *)_device)
#define READ32_DEVICE_DELEGATE(_device, _class, _member)    read32_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE32_DEVICE_DELEGATE(_device, _class, _member)   write32_delegate(FUNC(_class::_member), (_class *)_device)
#define READ64_DEVICE_DELEGATE(_device, _class, _member)    read64_delegate(FUNC(_class::_member), (_class *)_device)
#define WRITE64_DEVICE_DELEGATE(_device, _class, _member)   write64_delegate(FUNC(_class::_member), (_class *)_device)


// helper macro for merging data with the memory mask
#define COMBINE_DATA(varptr)            (*(varptr) = (*(varptr) & ~mem_mask) | (data & mem_mask))

#define ACCESSING_BITS_0_7              ((mem_mask & 0x000000ff) != 0)
#define ACCESSING_BITS_8_15             ((mem_mask & 0x0000ff00) != 0)
#define ACCESSING_BITS_16_23            ((mem_mask & 0x00ff0000) != 0)
#define ACCESSING_BITS_24_31            ((mem_mask & 0xff000000) != 0)
#define ACCESSING_BITS_32_39            ((mem_mask & U64(0x000000ff00000000)) != 0)
#define ACCESSING_BITS_40_47            ((mem_mask & U64(0x0000ff0000000000)) != 0)
#define ACCESSING_BITS_48_55            ((mem_mask & U64(0x00ff000000000000)) != 0)
#define ACCESSING_BITS_56_63            ((mem_mask & U64(0xff00000000000000)) != 0)

#define ACCESSING_BITS_0_15             ((mem_mask & 0x0000ffff) != 0)
#define ACCESSING_BITS_16_31            ((mem_mask & 0xffff0000) != 0)
#define ACCESSING_BITS_32_47            ((mem_mask & U64(0x0000ffff00000000)) != 0)
#define ACCESSING_BITS_48_63            ((mem_mask & U64(0xffff000000000000)) != 0)

#define ACCESSING_BITS_0_31             ((mem_mask & 0xffffffff) != 0)
#define ACCESSING_BITS_32_63            ((mem_mask & U64(0xffffffff00000000)) != 0)


// macros for accessing bytes and words within larger chunks

// read/write a byte to a 16-bit space
#define BYTE_XOR_BE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0))
#define BYTE_XOR_LE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,1))

// read/write a byte to a 32-bit space
#define BYTE4_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(3,0))
#define BYTE4_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,3))

// read/write a word to a 32-bit space
#define WORD_XOR_BE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0))
#define WORD_XOR_LE(a)                  ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,2))

// read/write a byte to a 64-bit space
#define BYTE8_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(7,0))
#define BYTE8_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,7))

// read/write a word to a 64-bit space
#define WORD2_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(6,0))
#define WORD2_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,6))

// read/write a dword to a 64-bit space
#define DWORD_XOR_BE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(4,0))
#define DWORD_XOR_LE(a)                 ((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,4))



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  read_ptr - return a pointer to valid RAM
//  referenced by the address, or NULL if no RAM
//  backing that address
//-------------------------------------------------

inline void *direct_read_data::read_ptr(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return &m_ptr[(byteaddress ^ directxor) & m_bytemask];
	return NULL;
}


//-------------------------------------------------
//  read_byte - read a byte via the
//  direct_read_data class
//-------------------------------------------------

inline UINT8 direct_read_data::read_byte(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return m_ptr[(byteaddress ^ directxor) & m_bytemask];
	return m_space.read_byte(byteaddress);
}


//-------------------------------------------------
//  read_word - read a word via the
//  direct_read_data class
//-------------------------------------------------

inline UINT16 direct_read_data::read_word(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT16 *>(&m_ptr[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_word(byteaddress);
}


//-------------------------------------------------
//  read_dword - read a dword via the
//  direct_read_data class
//-------------------------------------------------

inline UINT32 direct_read_data::read_dword(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT32 *>(&m_ptr[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_dword(byteaddress);
}


//-------------------------------------------------
//  read_qword - read a qword via the
//  direct_read_data class
//-------------------------------------------------

inline UINT64 direct_read_data::read_qword(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT64 *>(&m_ptr[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_qword(byteaddress);
}

#endif  /* __MEMORY_H__ */
