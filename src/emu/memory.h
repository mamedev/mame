/***************************************************************************

    memory.h

    Functions which handle device memory accesses.

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

// address spaces
enum address_spacenum
{
	AS_0,							// first address space
	AS_1,							// second address space
	AS_2,							// third address space
	AS_3,							// fourth address space
	ADDRESS_SPACES,					// maximum number of address spaces

	// alternate address space names for common use
	AS_PROGRAM = AS_0,				// program address space
	AS_DATA = AS_1,					// data address space
	AS_IO = AS_2					// I/O address space
};
DECLARE_ENUM_OPERATORS(address_spacenum);

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


// offsets and addresses are 32-bit (for now...)
typedef UINT32	offs_t;

// address map constructors are functions that build up an address_map
typedef void (*address_map_constructor)(address_map &map, const device_t &devconfig);


// legacy space read/write handlers
typedef UINT8	(*read8_space_func)  (ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset);
typedef void	(*write8_space_func) (ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data);
typedef UINT16	(*read16_space_func) (ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask);
typedef void	(*write16_space_func)(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask);
typedef UINT32	(*read32_space_func) (ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask);
typedef void	(*write32_space_func)(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask);
typedef UINT64	(*read64_space_func) (ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask);
typedef void	(*write64_space_func)(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask);

// legacy device read/write handlers
typedef UINT8	(*read8_device_func)  (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset);
typedef void	(*write8_device_func) (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data);
typedef UINT16	(*read16_device_func) (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask);
typedef void	(*write16_device_func)(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask);
typedef UINT32	(*read32_device_func) (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask);
typedef void	(*write32_device_func)(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask);
typedef UINT64	(*read64_device_func) (ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask);
typedef void	(*write64_device_func)(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask);


// struct with function pointers for accessors; use is generally discouraged unless necessary
struct data_accessors
{
	UINT8		(*read_byte)(address_space *space, offs_t byteaddress);
	UINT16		(*read_word)(address_space *space, offs_t byteaddress);
	UINT16		(*read_word_masked)(address_space *space, offs_t byteaddress, UINT16 mask);
	UINT32		(*read_dword)(address_space *space, offs_t byteaddress);
	UINT32		(*read_dword_masked)(address_space *space, offs_t byteaddress, UINT32 mask);
	UINT64		(*read_qword)(address_space *space, offs_t byteaddress);
	UINT64		(*read_qword_masked)(address_space *space, offs_t byteaddress, UINT64 mask);

	void		(*write_byte)(address_space *space, offs_t byteaddress, UINT8 data);
	void		(*write_word)(address_space *space, offs_t byteaddress, UINT16 data);
	void		(*write_word_masked)(address_space *space, offs_t byteaddress, UINT16 data, UINT16 mask);
	void		(*write_dword)(address_space *space, offs_t byteaddress, UINT32 data);
	void		(*write_dword_masked)(address_space *space, offs_t byteaddress, UINT32 data, UINT32 mask);
	void		(*write_qword)(address_space *space, offs_t byteaddress, UINT64 data);
	void		(*write_qword_masked)(address_space *space, offs_t byteaddress, UINT64 data, UINT64 mask);
};


// ======================> direct_update_delegate

// direct region update handler
typedef delegate<offs_t (direct_read_data &, offs_t)> direct_update_delegate;


// ======================> read_delegate

// declare delegates for each width
typedef delegate<UINT8 (address_space &, offs_t, UINT8)> read8_delegate;
typedef delegate<UINT16 (address_space &, offs_t, UINT16)> read16_delegate;
typedef delegate<UINT32 (address_space &, offs_t, UINT32)> read32_delegate;
typedef delegate<UINT64 (address_space &, offs_t, UINT64)> read64_delegate;


// ======================> write_delegate

// declare delegates for each width
typedef delegate<void (address_space &, offs_t, UINT8, UINT8)> write8_delegate;
typedef delegate<void (address_space &, offs_t, UINT16, UINT16)> write16_delegate;
typedef delegate<void (address_space &, offs_t, UINT32, UINT32)> write32_delegate;
typedef delegate<void (address_space &, offs_t, UINT64, UINT64)> write64_delegate;


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
		direct_range *			m_next;					// pointer to the next range in the list
		offs_t					m_bytestart;			// starting byte offset of the range
		offs_t					m_byteend;				// ending byte offset of the range
	};

	// construction/destruction
	direct_read_data(address_space &space);
	~direct_read_data();

	// getters
	address_space &space() const { return m_space; }
	UINT8 *raw() const { return m_raw; }
	UINT8 *decrypted() const { return m_decrypted; }

	// see if an address is within bounds, or attempt to update it if not
	bool address_is_valid(offs_t byteaddress) { return EXPECTED(byteaddress >= m_bytestart && byteaddress <= m_byteend) || set_direct_region(byteaddress); }

	// force a recomputation on the next read
	void force_update() { m_byteend = 0; m_bytestart = 1; }
	void force_update(UINT8 if_match) { if (m_entry == if_match) force_update(); }

	// custom update callbacks and configuration
	direct_update_delegate set_direct_update(direct_update_delegate function);
	void explicit_configure(offs_t bytestart, offs_t byteend, offs_t bytemask, void *raw, void *decrypted = NULL);

	// accessor methods for reading raw data
	void *read_raw_ptr(offs_t byteaddress, offs_t directxor = 0);
	UINT8 read_raw_byte(offs_t byteaddress, offs_t directxor = 0);
	UINT16 read_raw_word(offs_t byteaddress, offs_t directxor = 0);
	UINT32 read_raw_dword(offs_t byteaddress, offs_t directxor = 0);
	UINT64 read_raw_qword(offs_t byteaddress, offs_t directxor = 0);

	// accessor methods for reading decrypted data
	void *read_decrypted_ptr(offs_t byteaddress, offs_t directxor = 0);
	UINT8 read_decrypted_byte(offs_t byteaddress, offs_t directxor = 0);
	UINT16 read_decrypted_word(offs_t byteaddress, offs_t directxor = 0);
	UINT32 read_decrypted_dword(offs_t byteaddress, offs_t directxor = 0);
	UINT64 read_decrypted_qword(offs_t byteaddress, offs_t directxor = 0);

private:
	// internal helpers
	bool set_direct_region(offs_t &byteaddress);
	direct_range *find_range(offs_t byteaddress, UINT8 &entry);
	void remove_intersecting_ranges(offs_t bytestart, offs_t byteend);

	// internal state
	address_space &				m_space;
	UINT8 *						m_raw;					// direct access data pointer (raw)
	UINT8 *						m_decrypted;			// direct access data pointer (decrypted)
	offs_t						m_bytemask;				// byte address mask
	offs_t						m_bytestart;			// minimum valid byte address
	offs_t						m_byteend;				// maximum valid byte address
	UINT8						m_entry;				// live entry
	simple_list<direct_range>	m_rangelist[256];		// list of ranges for each entry
	simple_list<direct_range>	m_freerangelist;		// list of recycled range entries
	direct_update_delegate		m_directupdate;			// fast direct-access update callback
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
	const char *		m_name;
	endianness_t		m_endianness;
	UINT8				m_databus_width;
	UINT8				m_addrbus_width;
	INT8				m_addrbus_shift;
	UINT8				m_logaddr_width;
	UINT8				m_page_shift;
	address_map_constructor m_internal_map;
	address_map_constructor m_default_map;
};


// ======================> address_space

// address_space holds live information about an address space
class address_space
{
	friend class address_table;
	friend class address_table_read;
	friend class address_table_write;
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
	address_map *map() const { return m_map; }

	direct_read_data &direct() const { return m_direct; }

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

	// address-to-byte conversion helpers
	offs_t address_to_byte(offs_t address) const { return m_config.addr2byte(address); }
	offs_t address_to_byte_end(offs_t address) const { return m_config.addr2byte_end(address); }
	offs_t byte_to_address(offs_t address) const { return m_config.byte2addr(address); }
	offs_t byte_to_address_end(offs_t address) const { return m_config.byte2addr_end(address); }

	// decryption
	void set_decrypted_region(offs_t addrstart, offs_t addrend, void *base);

	// direct access
	direct_update_delegate set_direct_update_handler(direct_update_delegate function) { return m_direct.set_direct_update(function); }
	bool set_direct_region(offs_t &byteaddress);

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
	void *install_rom(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, void *baseptr = NULL) { return install_ram_generic(addrstart, addrend, addrmask, addrmirror, ROW_READ, baseptr); }
	void *install_writeonly(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, void *baseptr = NULL) { return install_ram_generic(addrstart, addrend, addrmask, addrmirror, ROW_WRITE, baseptr); }
	void *install_ram(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, void *baseptr = NULL) { return install_ram_generic(addrstart, addrend, addrmask, addrmirror, ROW_READWRITE, baseptr); }

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

	// install legacy address space handlers (short form)
	UINT8 *install_legacy_read_handler(offs_t addrstart, offs_t addrend, read8_space_func rhandler, const char *rname, UINT64 unitmask = 0) { return install_legacy_read_handler(addrstart, addrend, 0, 0, rhandler, rname, unitmask); }
	UINT8 *install_legacy_write_handler(offs_t addrstart, offs_t addrend, write8_space_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_write_handler(addrstart, addrend, 0, 0, whandler, wname, unitmask); }
	UINT8 *install_legacy_readwrite_handler(offs_t addrstart, offs_t addrend, read8_space_func rhandler, const char *rname, write8_space_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_readwrite_handler(addrstart, addrend, 0, 0, rhandler, rname, whandler, wname, unitmask); }
	UINT16 *install_legacy_read_handler(offs_t addrstart, offs_t addrend, read16_space_func rhandler, const char *rname, UINT64 unitmask = 0) { return install_legacy_read_handler(addrstart, addrend, 0, 0, rhandler, rname, unitmask); }
	UINT16 *install_legacy_write_handler(offs_t addrstart, offs_t addrend, write16_space_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_write_handler(addrstart, addrend, 0, 0, whandler, wname, unitmask); }
	UINT16 *install_legacy_readwrite_handler(offs_t addrstart, offs_t addrend, read16_space_func rhandler, const char *rname, write16_space_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_readwrite_handler(addrstart, addrend, 0, 0, rhandler, rname, whandler, wname, unitmask); }
	UINT32 *install_legacy_read_handler(offs_t addrstart, offs_t addrend, read32_space_func rhandler, const char *rname, UINT64 unitmask = 0) { return install_legacy_read_handler(addrstart, addrend, 0, 0, rhandler, rname, unitmask); }
	UINT32 *install_legacy_write_handler(offs_t addrstart, offs_t addrend, write32_space_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_write_handler(addrstart, addrend, 0, 0, whandler, wname, unitmask); }
	UINT32 *install_legacy_readwrite_handler(offs_t addrstart, offs_t addrend, read32_space_func rhandler, const char *rname, write32_space_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_readwrite_handler(addrstart, addrend, 0, 0, rhandler, rname, whandler, wname, unitmask); }
	UINT64 *install_legacy_read_handler(offs_t addrstart, offs_t addrend, read64_space_func rhandler, const char *rname, UINT64 unitmask = 0) { return install_legacy_read_handler(addrstart, addrend, 0, 0, rhandler, rname, unitmask); }
	UINT64 *install_legacy_write_handler(offs_t addrstart, offs_t addrend, write64_space_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_write_handler(addrstart, addrend, 0, 0, whandler, wname, unitmask); }
	UINT64 *install_legacy_readwrite_handler(offs_t addrstart, offs_t addrend, read64_space_func rhandler, const char *rname, write64_space_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_readwrite_handler(addrstart, addrend, 0, 0, rhandler, rname, whandler, wname, unitmask); }

	// install legacy address space handlers (with mirror/mask)
	UINT8 *install_legacy_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT8 *install_legacy_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT8 *install_legacy_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, const char *rname, write8_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT16 *install_legacy_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT16 *install_legacy_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT16 *install_legacy_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, const char *rname, write16_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT32 *install_legacy_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT32 *install_legacy_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT32 *install_legacy_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, const char *rname, write32_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT64 *install_legacy_read_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT64 *install_legacy_write_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT64 *install_legacy_readwrite_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, const char *rname, write64_space_func whandler, const char *wname, UINT64 unitmask = 0);

	// install legacy device handlers (short form)
	UINT8 *install_legacy_read_handler(device_t &device, offs_t addrstart, offs_t addrend, read8_device_func rhandler, const char *rname, UINT64 unitmask = 0) { return install_legacy_read_handler(device, addrstart, addrend, 0, 0, rhandler, rname, unitmask); }
	UINT8 *install_legacy_write_handler(device_t &device, offs_t addrstart, offs_t addrend, write8_device_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_write_handler(device, addrstart, addrend, 0, 0, whandler, wname, unitmask); }
	UINT8 *install_legacy_readwrite_handler(device_t &device, offs_t addrstart, offs_t addrend, read8_device_func rhandler, const char *rname, write8_device_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_readwrite_handler(device, addrstart, addrend, 0, 0, rhandler, rname, whandler, wname, unitmask); }
	UINT16 *install_legacy_read_handler(device_t &device, offs_t addrstart, offs_t addrend, read16_device_func rhandler, const char *rname, UINT64 unitmask = 0) { return install_legacy_read_handler(device, addrstart, addrend, 0, 0, rhandler, rname, unitmask); }
	UINT16 *install_legacy_write_handler(device_t &device, offs_t addrstart, offs_t addrend, write16_device_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_write_handler(device, addrstart, addrend, 0, 0, whandler, wname, unitmask); }
	UINT16 *install_legacy_readwrite_handler(device_t &device, offs_t addrstart, offs_t addrend, read16_device_func rhandler, const char *rname, write16_device_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_readwrite_handler(device, addrstart, addrend, 0, 0, rhandler, rname, whandler, wname, unitmask); }
	UINT32 *install_legacy_read_handler(device_t &device, offs_t addrstart, offs_t addrend, read32_device_func rhandler, const char *rname, UINT64 unitmask = 0) { return install_legacy_read_handler(device, addrstart, addrend, 0, 0, rhandler, rname, unitmask); }
	UINT32 *install_legacy_write_handler(device_t &device, offs_t addrstart, offs_t addrend, write32_device_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_write_handler(device, addrstart, addrend, 0, 0, whandler, wname, unitmask); }
	UINT32 *install_legacy_readwrite_handler(device_t &device, offs_t addrstart, offs_t addrend, read32_device_func rhandler, const char *rname, write32_device_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_readwrite_handler(device, addrstart, addrend, 0, 0, rhandler, rname, whandler, wname, unitmask); }
	UINT64 *install_legacy_read_handler(device_t &device, offs_t addrstart, offs_t addrend, read64_device_func rhandler, const char *rname, UINT64 unitmask = 0) { return install_legacy_read_handler(device, addrstart, addrend, 0, 0, rhandler, rname, unitmask); }
	UINT64 *install_legacy_write_handler(device_t &device, offs_t addrstart, offs_t addrend, write64_device_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_write_handler(device, addrstart, addrend, 0, 0, whandler, wname, unitmask); }
	UINT64 *install_legacy_readwrite_handler(device_t &device, offs_t addrstart, offs_t addrend, read64_device_func rhandler, const char *rname, write64_device_func whandler, const char *wname, UINT64 unitmask = 0) { return install_legacy_readwrite_handler(device, addrstart, addrend, 0, 0, rhandler, rname, whandler, wname, unitmask); }

	// install legacy device handlers (with mirror/mask)
	UINT8 *install_legacy_read_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT8 *install_legacy_write_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT8 *install_legacy_readwrite_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, const char *rname, write8_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT16 *install_legacy_read_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT16 *install_legacy_write_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT16 *install_legacy_readwrite_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, const char *rname, write16_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT32 *install_legacy_read_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT32 *install_legacy_write_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT32 *install_legacy_readwrite_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, const char *rname, write32_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT64 *install_legacy_read_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT64 *install_legacy_write_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT64 *install_legacy_readwrite_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, const char *rname, write64_device_func whandler, const char *wname, UINT64 unitmask = 0);

	// setup
	void prepare_map();
	void populate_from_map();
	void allocate_memory();
	void locate_memory();

private:
	// internal helpers
	virtual address_table_read &read() = 0;
	virtual address_table_write &write() = 0;
	void populate_map_entry(const address_map_entry &entry, read_or_write readorwrite);
	void unmap_generic(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite, bool quiet);
	void *install_ram_generic(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite, void *baseptr);
	void install_bank_generic(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag);
	void bind_and_install_handler(const address_map_entry &entry, read_or_write readorwrite, device_t *device);
	void adjust_addresses(offs_t &start, offs_t &end, offs_t &mask, offs_t &mirror);
	void *find_backing_memory(offs_t addrstart, offs_t addrend);
	bool needs_backing_store(const address_map_entry *entry);
	memory_bank &bank_find_or_allocate(const char *tag, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite);
	address_map_entry *block_assign_intersecting(offs_t bytestart, offs_t byteend, UINT8 *base);

protected:
	// private state
	address_space *			m_next;				// next address space in the global list
	const address_space_config &m_config;		// configuration of this space
	device_t &				m_device;			// reference to the owning device
	address_map *			m_map;				// original memory map
	offs_t					m_addrmask;			// physical address mask
	offs_t					m_bytemask;			// byte-converted physical address mask
	offs_t					m_logaddrmask;		// logical address mask
	offs_t					m_logbytemask;		// byte-converted logical address mask
	UINT64					m_unmap;			// unmapped value
	address_spacenum		m_spacenum;			// address space index
	bool					m_debugger_access;	// treat accesses as coming from the debugger
	bool					m_log_unmap;		// log unmapped accesses in this space?
	direct_read_data &		m_direct;			// fast direct-access read info
	const char *			m_name;				// friendly name of the address space
	UINT8					m_addrchars;		// number of characters to use for physical addresses
	UINT8					m_logaddrchars;		// number of characters to use for logical addresses

private:
	memory_manager &		m_manager;			// reference to the owning manager
	running_machine &		m_machine;			// reference to the owning machine
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
	memory_block *			m_next;					// next memory block in the list
	running_machine &		m_machine;				// need the machine to free our memory
	address_space &			m_space;				// which address space are we associated with?
	offs_t					m_bytestart, m_byteend;	// byte-normalized start/end for verifying a match
	UINT8 *					m_data;					// pointer to the data for this block
	UINT8 *					m_allocated;			// pointer to the actually allocated block
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
	running_machine &machine() const { return m_machine; }
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


// ======================> memory_share

// a memory share contains information about shared memory region
class memory_share
{
	friend class simple_list<memory_share>;

public:
	// construction/destruction
	memory_share(UINT8 width, size_t bytes, void *ptr = NULL)
		: m_ptr(ptr),
		  m_bytes(bytes),
		  m_width(width) { }

	// getters
	memory_share *next() const { return m_next; }
	void *ptr() const { return m_ptr; }
	size_t bytes() const { return m_bytes; }
	UINT8 width() const { return m_width; }

	// setters
	void set_ptr(void *ptr) { m_ptr = ptr; }

private:
	// internal state
	memory_share *			m_next;					// next share in the list
	void *					m_ptr;					// pointer to the memory backing the region
	size_t					m_bytes;				// size of the shared region in bytes
	UINT8					m_width;				// width of the shared region
};


// ======================> memory_manager

// holds internal state for the memory system
class memory_manager
{
	friend class address_space;

public:
	// construction/destruction
	memory_manager(running_machine &machine);

	// getters
	running_machine &machine() const { return m_machine; }
	address_space *first_space() const { return m_spacelist.first(); }
	memory_bank *first_bank() const { return m_banklist.first(); }

	// configure the addresses for a bank
	void configure_bank(const char *tag, int startentry, int numentries, void *base, offs_t stride);
	void configure_bank(device_t &device, const char *tag, int startentry, int numentries, void *base, offs_t stride);

	// configure the decrypted addresses for a bank
	void configure_bank_decrypted(const char *tag, int startentry, int numentries, void *base, offs_t stride);
	void configure_bank_decrypted(device_t &device, const char *tag, int startentry, int numentries, void *base, offs_t stride);

	// select one pre-configured entry to be the new bank base
	void set_bank(const char *tag, int entrynum);
	void set_bank(device_t &device, const char *tag, int entrynum);

	// return the currently selected bank
	int bank(const char *tag);
	int bank(device_t &device, const char *tag);

	// set the absolute address of a bank base
	void set_bankptr(const char *tag, void *base) ATTR_NONNULL(3);
	void set_bankptr(device_t &device, const char *tag, void *base) ATTR_NONNULL(3);

	// get a pointer to a shared memory region by tag
	memory_share *shared(const char *tag);
	memory_share *shared(device_t &device, const char *tag);

	// dump the internal memory tables to the given file
	void dump(FILE *file);

	// pointers to a bank pointer (internal usage only)
	UINT8 **bank_pointer_addr(UINT8 index, bool decrypted = false) { return decrypted ? &m_bankd_ptr[index] : &m_bank_ptr[index]; }

private:
	// internal helpers
	void bank_reattach();

	// internal state
	running_machine &			m_machine;				// reference to the machine
	bool						m_initialized;			// have we completed initialization?

	UINT8 *						m_bank_ptr[256];		// array of bank pointers
	UINT8 *						m_bankd_ptr[256];		// array of decrypted bank pointers

	simple_list<address_space>	m_spacelist;			// list of address spaces
	simple_list<memory_block>	m_blocklist;			// head of the list of memory blocks

	simple_list<memory_bank>	m_banklist;				// data gathered for each bank
	tagmap_t<memory_bank *>		m_bankmap;				// map for fast bank lookups
	UINT8						m_banknext;				// next bank to allocate

	tagged_list<memory_share>	m_sharelist;			// map for share lookups
};



//**************************************************************************
//  MACROS
//**************************************************************************

// opcode base adjustment handler function macro
#define DIRECT_UPDATE_MEMBER(name)		offs_t name(ATTR_UNUSED direct_read_data &direct, ATTR_UNUSED offs_t address)
#define DIRECT_UPDATE_HANDLER(name)		offs_t name(ATTR_UNUSED running_machine &machine, ATTR_UNUSED direct_read_data &direct, ATTR_UNUSED offs_t address)


// space read/write handler function macros
#define READ8_HANDLER(name) 			UINT8  name(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset)
#define WRITE8_HANDLER(name)			void   name(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data)
#define READ16_HANDLER(name)			UINT16 name(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_HANDLER(name)			void   name(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
#define READ32_HANDLER(name)			UINT32 name(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask)
#define WRITE32_HANDLER(name)			void   name(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask)
#define READ64_HANDLER(name)			UINT64 name(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask)
#define WRITE64_HANDLER(name)			void   name(ATTR_UNUSED address_space *space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask)


// device read/write handler function macros
#define READ8_DEVICE_HANDLER(name)		UINT8  name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset)
#define WRITE8_DEVICE_HANDLER(name) 	void   name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data)
#define READ16_DEVICE_HANDLER(name)		UINT16 name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
#define READ32_DEVICE_HANDLER(name)		UINT32 name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask)
#define WRITE32_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask)
#define READ64_DEVICE_HANDLER(name)		UINT64 name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask)
#define WRITE64_DEVICE_HANDLER(name)	void   name(ATTR_UNUSED device_t *device, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask)


// space read/write handler function macros
#define READ8_MEMBER(name)				UINT8  name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 mem_mask)
#define WRITE8_MEMBER(name)				void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask)
#define READ16_MEMBER(name)				UINT16 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask)
#define WRITE16_MEMBER(name)			void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask)
#define READ32_MEMBER(name)				UINT32 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask)
#define WRITE32_MEMBER(name)			void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask)
#define READ64_MEMBER(name)				UINT64 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask)
#define WRITE64_MEMBER(name)			void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask)

#define DECLARE_READ8_MEMBER(name)		UINT8  name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 mem_mask = 0xff)
#define DECLARE_WRITE8_MEMBER(name)		void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 data, ATTR_UNUSED UINT8 mem_mask = 0xff)
#define DECLARE_READ16_MEMBER(name)		UINT16 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 mem_mask = 0xffff)
#define DECLARE_WRITE16_MEMBER(name)	void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 data, ATTR_UNUSED UINT16 mem_mask = 0xffff)
#define DECLARE_READ32_MEMBER(name)		UINT32 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 mem_mask = 0xffffffff)
#define DECLARE_WRITE32_MEMBER(name)	void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT32 data, ATTR_UNUSED UINT32 mem_mask = 0xffffffff)
#define DECLARE_READ64_MEMBER(name)		UINT64 name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 mem_mask = U64(0xffffffffffffffff))
#define DECLARE_WRITE64_MEMBER(name)	void   name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT64 data, ATTR_UNUSED UINT64 mem_mask = U64(0xffffffffffffffff))


// helper macro for merging data with the memory mask
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


// macros for accessing bytes and words within larger chunks

// read/write a byte to a 16-bit space
#define BYTE_XOR_BE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(1,0))
#define BYTE_XOR_LE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,1))

// read/write a byte to a 32-bit space
#define BYTE4_XOR_BE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(3,0))
#define BYTE4_XOR_LE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,3))

// read/write a word to a 32-bit space
#define WORD_XOR_BE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(2,0))
#define WORD_XOR_LE(a)  				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,2))

// read/write a byte to a 64-bit space
#define BYTE8_XOR_BE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(7,0))
#define BYTE8_XOR_LE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,7))

// read/write a word to a 64-bit space
#define WORD2_XOR_BE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(6,0))
#define WORD2_XOR_LE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,6))

// read/write a dword to a 64-bit space
#define DWORD_XOR_BE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(4,0))
#define DWORD_XOR_LE(a) 				((a) ^ NATIVE_ENDIAN_VALUE_LE_BE(0,4))



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const char *const address_space_names[ADDRESS_SPACES];



//**************************************************************************
//  FUNCTION PROTOTYPES FOR CORE MEMORY FUNCTIONS
//**************************************************************************

// configure the addresses for a bank
void memory_configure_bank(running_machine &machine, const char *tag, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(5);
void memory_configure_bank(device_t &device, const char *tag, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(5);

// configure the decrypted addresses for a bank
void memory_configure_bank_decrypted(running_machine &machine, const char *tag, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(5);
void memory_configure_bank_decrypted(device_t &device, const char *tag, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(5);

// select one pre-configured entry to be the new bank base
void memory_set_bank(running_machine &machine, const char *tag, int entrynum);
void memory_set_bank(device_t &device, const char *tag, int entrynum);

// set the absolute address of a bank base
void memory_set_bankptr(running_machine &machine, const char *tag, void *base) ATTR_NONNULL(3);
void memory_set_bankptr(device_t &device, const char *tag, void *base) ATTR_NONNULL(3);



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  read_raw_ptr - return a pointer to valid RAM
//  referenced by the address, or NULL if no RAM
//  backing that address
//-------------------------------------------------

inline void *direct_read_data::read_raw_ptr(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return &m_raw[(byteaddress ^ directxor) & m_bytemask];
	return NULL;
}

inline void *direct_read_data::read_decrypted_ptr(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return &m_decrypted[(byteaddress ^ directxor) & m_bytemask];
	return NULL;
}


//-------------------------------------------------
//  read_raw_byte - read a byte via the
//  direct_read_data class
//-------------------------------------------------

inline UINT8 direct_read_data::read_raw_byte(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return m_raw[(byteaddress ^ directxor) & m_bytemask];
	return m_space.read_byte(byteaddress);
}

inline UINT8 direct_read_data::read_decrypted_byte(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return m_decrypted[(byteaddress ^ directxor) & m_bytemask];
	return m_space.read_byte(byteaddress);
}


//-------------------------------------------------
//  read_raw_word - read a word via the
//  direct_read_data class
//-------------------------------------------------

inline UINT16 direct_read_data::read_raw_word(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT16 *>(&m_raw[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_word(byteaddress);
}

inline UINT16 direct_read_data::read_decrypted_word(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT16 *>(&m_decrypted[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_word(byteaddress);
}


//-------------------------------------------------
//  read_raw_dword - read a dword via the
//  direct_read_data class
//-------------------------------------------------

inline UINT32 direct_read_data::read_raw_dword(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT32 *>(&m_raw[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_dword(byteaddress);
}

inline UINT32 direct_read_data::read_decrypted_dword(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT32 *>(&m_decrypted[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_dword(byteaddress);
}


//-------------------------------------------------
//  read_raw_qword - read a qword via the
//  direct_read_data class
//-------------------------------------------------

inline UINT64 direct_read_data::read_raw_qword(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT64 *>(&m_raw[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_qword(byteaddress);
}

inline UINT64 direct_read_data::read_decrypted_qword(offs_t byteaddress, offs_t directxor)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT64 *>(&m_decrypted[(byteaddress ^ directxor) & m_bytemask]);
	return m_space.read_qword(byteaddress);
}

#endif	/* __MEMORY_H__ */
