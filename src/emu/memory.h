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
enum
{
	ADDRESS_SPACE_0,				// first address space
	ADDRESS_SPACE_1,				// second address space
	ADDRESS_SPACE_2,				// third address space
	ADDRESS_SPACE_3,				// fourth address space
	ADDRESS_SPACES					// maximum number of address spaces
};

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
class device_config;
class device_t;
struct game_driver;

// forward declarations of classes defined here
class address_map;
class address_map_entry;
class memory_bank;
class direct_read_data;
class address_space;
class address_table;
class address_table_read;
class address_table_write;


// offsets and addresses are 32-bit (for now...)
typedef UINT32	offs_t;

// address map constructors are functions that build up an address_map
typedef void (*address_map_constructor)(address_map &map, const device_config &devconfig);


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
typedef proto_delegate_2param<offs_t, direct_read_data &, offs_t> direct_update_proto_delegate;
typedef delegate_2param<offs_t, direct_read_data &, offs_t> direct_update_delegate;

#define direct_update_delegate_create(_Class, _Method, _Object) direct_update_delegate(direct_update_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method), _Object)
#define direct_update_delegate_create_static(_Function, _Object) direct_update_delegate(direct_update_proto_delegate::_create_static<running_machine, &_Function>(#_Function), _Object)


// ======================> read_delegate

// declare proto-delegates for each width
typedef proto_delegate_3param<UINT8, address_space &, offs_t, UINT8> read8_proto_delegate;
typedef proto_delegate_3param<UINT16, address_space &, offs_t, UINT16> read16_proto_delegate;
typedef proto_delegate_3param<UINT32, address_space &, offs_t, UINT32> read32_proto_delegate;
typedef proto_delegate_3param<UINT64, address_space &, offs_t, UINT64> read64_proto_delegate;

// declare delegates for each width
typedef delegate_3param<UINT8, address_space &, offs_t, UINT8> read8_delegate;
typedef delegate_3param<UINT16, address_space &, offs_t, UINT16> read16_delegate;
typedef delegate_3param<UINT32, address_space &, offs_t, UINT32> read32_delegate;
typedef delegate_3param<UINT64, address_space &, offs_t, UINT64> read64_delegate;

// macros for creating read proto-delegates
#define read8_proto_delegate_create(_Class, _Method) read8_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method)
#define read16_proto_delegate_create(_Class, _Method) read16_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method)
#define read32_proto_delegate_create(_Class, _Method) read32_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method)
#define read64_proto_delegate_create(_Class, _Method) read64_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method)

// macros for creating read delegates, bound to the provided object
#define read8_delegate_create(_Class, _Method, _Object) read8_delegate(read8_proto_delegate_create(_Class, _Method), _Object)
#define read16_delegate_create(_Class, _Method, _Object) read16_delegate(read16_proto_delegate_create(_Class, _Method), _Object)
#define read32_delegate_create(_Class, _Method, _Object) read32_delegate(read32_proto_delegate_create(_Class, _Method), _Object)
#define read64_delegate_create(_Class, _Method, _Object) read64_delegate(read64_proto_delegate_create(_Class, _Method), _Object)


// ======================> write_delegate

// declare proto-delegates for each width
typedef proto_delegate_4param<void, address_space &, offs_t, UINT8, UINT8> write8_proto_delegate;
typedef proto_delegate_4param<void, address_space &, offs_t, UINT16, UINT16> write16_proto_delegate;
typedef proto_delegate_4param<void, address_space &, offs_t, UINT32, UINT32> write32_proto_delegate;
typedef proto_delegate_4param<void, address_space &, offs_t, UINT64, UINT64> write64_proto_delegate;

// declare delegates for each width
typedef delegate_4param<void, address_space &, offs_t, UINT8, UINT8> write8_delegate;
typedef delegate_4param<void, address_space &, offs_t, UINT16, UINT16> write16_delegate;
typedef delegate_4param<void, address_space &, offs_t, UINT32, UINT32> write32_delegate;
typedef delegate_4param<void, address_space &, offs_t, UINT64, UINT64> write64_delegate;

// macros for creating write proto-delegates
#define write8_proto_delegate_create(_Class, _Method) write8_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method)
#define write16_proto_delegate_create(_Class, _Method) write16_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method)
#define write32_proto_delegate_create(_Class, _Method) write32_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method)
#define write64_proto_delegate_create(_Class, _Method) write64_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method)

// macros for creating write delegates, bound to the provided object
#define write8_delegate_create(_Class, _Method, _Object) write8_delegate(write8_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method), _Object)
#define write16_delegate_create(_Class, _Method, _Object) write16_delegate(write16_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method), _Object)
#define write32_delegate_create(_Class, _Method, _Object) write32_delegate(write32_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method), _Object)
#define write64_delegate_create(_Class, _Method, _Object) write64_delegate(write64_proto_delegate::_create_member<_Class, &_Class::_Method>(#_Class "::" #_Method), _Object)


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
	void *read_raw_ptr(offs_t byteaddress);
	UINT8 read_raw_byte(offs_t byteaddress);
	UINT16 read_raw_word(offs_t byteaddress);
	UINT32 read_raw_dword(offs_t byteaddress);
	UINT64 read_raw_qword(offs_t byteaddress);

	// accessor methods for reading decrypted data
	void *read_decrypted_ptr(offs_t byteaddress);
	UINT8 read_decrypted_byte(offs_t byteaddress);
	UINT16 read_decrypted_word(offs_t byteaddress);
	UINT32 read_decrypted_dword(offs_t byteaddress);
	UINT64 read_decrypted_qword(offs_t byteaddress);

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
class address_space : public bindable_object
{
	friend class address_table;
	friend class address_table_read;
	friend class address_table_write;
	friend class direct_read_data;
	friend class simple_list<address_space>;
	friend resource_pool_object<address_space>::~resource_pool_object();

protected:
	// construction/destruction
	address_space(device_memory_interface &memory, int spacenum, bool large);
	~address_space();

public:
	// public allocator
	static address_space &allocate(running_machine &machine, const address_space_config &config, device_memory_interface &memory, int spacenum);

	// getters
	address_space *next() const { return m_next; }
	device_t &device() const { return m_device; }
	const char *name() const { return m_name; }
	int spacenum() const { return m_spacenum; }
	address_map *map() const { return m_map; }

	direct_read_data &direct() const { return const_cast<direct_read_data &>(m_direct); }

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

	// static handler installation
	void unmap(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite, bool quiet);
	void install_port(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag);
	void install_bank(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, const char *rtag, const char *wtag);
	void *install_ram(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite, void *baseptr = NULL);

	// install new-style delegate handlers
	UINT8 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_delegate rhandler, UINT64 unitmask = 0);
	UINT8 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_delegate whandler, UINT64 unitmask = 0);
	UINT8 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_delegate rhandler, write8_delegate whandler, UINT64 unitmask = 0);
	UINT16 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_delegate rhandler, UINT64 unitmask = 0);
	UINT16 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_delegate whandler, UINT64 unitmask = 0);
	UINT16 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_delegate rhandler, write16_delegate whandler, UINT64 unitmask = 0);
	UINT32 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_delegate rhandler, UINT64 unitmask = 0);
	UINT32 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_delegate whandler, UINT64 unitmask = 0);
	UINT32 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_delegate rhandler, write32_delegate whandler, UINT64 unitmask = 0);
	UINT64 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_delegate rhandler, UINT64 unitmask = 0);
	UINT64 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_delegate whandler, UINT64 unitmask = 0);
	UINT64 *install_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_delegate rhandler, write64_delegate whandler, UINT64 unitmask = 0);

	// install legacy address space handlers
	UINT8 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT8 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT8 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_space_func rhandler, const char *rname, write8_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT16 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT16 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT16 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_space_func rhandler, const char *rname, write16_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT32 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT32 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT32 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_space_func rhandler, const char *rname, write32_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT64 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT64 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_space_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT64 *install_legacy_handler(offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_space_func rhandler, const char *rname, write64_space_func whandler, const char *wname, UINT64 unitmask = 0);

	// install legacy device handlers
	UINT8 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT8 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write8_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT8 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read8_device_func rhandler, const char *rname, write8_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT16 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT16 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write16_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT16 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read16_device_func rhandler, const char *rname, write16_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT32 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT32 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write32_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT32 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read32_device_func rhandler, const char *rname, write32_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT64 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, const char *rname, UINT64 unitmask = 0);
	UINT64 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, write64_device_func whandler, const char *wname, UINT64 unitmask = 0);
	UINT64 *install_legacy_handler(device_t &device, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read64_device_func rhandler, const char *rname, write64_device_func whandler, const char *wname, UINT64 unitmask = 0);

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
	void bind_and_install_handler(const address_map_entry &entry, read_or_write readorwrite, device_t *device);
	void adjust_addresses(offs_t &start, offs_t &end, offs_t &mask, offs_t &mirror);
	void *find_backing_memory(offs_t addrstart, offs_t addrend);
	bool needs_backing_store(const address_map_entry *entry);
	memory_bank &bank_find_or_allocate(const char *tag, offs_t addrstart, offs_t addrend, offs_t addrmask, offs_t addrmirror, read_or_write readorwrite);
	address_map_entry *block_assign_intersecting(offs_t bytestart, offs_t byteend, UINT8 *base);

public:
	// public state (eventually will go away)
	running_machine *		machine;			// kept for backwards compatibility
	device_t *				cpu;				// kept for backwards compatibility
	running_machine &		m_machine;			// reference to the owning machine

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
	UINT8					m_spacenum;			// address space index
	bool					m_debugger_access;	// treat accesses as coming from the debugger
	bool					m_log_unmap;		// log unmapped accesses in this space?
	direct_read_data &		m_direct;			// fast direct-access read info
	const char *			m_name;				// friendly name of the address space
	UINT8					m_addrchars;		// number of characters to use for physical addresses
	UINT8					m_logaddrchars;		// number of characters to use for logical addresses
};



//**************************************************************************
//  MACROS
//**************************************************************************

// opcode base adjustment handler function macro
#define DIRECT_UPDATE_MEMBER(name)		offs_t name(ATTR_UNUSED direct_read_data &direct, ATTR_UNUSED offs_t address)
#define DIRECT_UPDATE_HANDLER(name)		offs_t name(ATTR_UNUSED running_machine *machine, ATTR_UNUSED direct_read_data &direct, ATTR_UNUSED offs_t address)


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


// wrappers for dynamic read handler installation
#define memory_install_read8_handler(space, start, end, mask, mirror, rhandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, rhandler, #rhandler)
#define memory_install_read16_handler(space, start, end, mask, mirror, rhandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, rhandler, #rhandler)
#define memory_install_read32_handler(space, start, end, mask, mirror, rhandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, rhandler, #rhandler)
#define memory_install_read64_handler(space, start, end, mask, mirror, rhandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, rhandler, #rhandler)

#define memory_install_read8_device_handler(space, device, start, end, mask, mirror, rhandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, rhandler, #rhandler)
#define memory_install_read16_device_handler(space, device, start, end, mask, mirror, rhandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, rhandler, #rhandler)
#define memory_install_read32_device_handler(space, device, start, end, mask, mirror, rhandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, rhandler, #rhandler)
#define memory_install_read64_device_handler(space, device, start, end, mask, mirror, rhandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, rhandler, #rhandler)

#define memory_install_read_port(space, start, end, mask, mirror, rtag) \
	const_cast<address_space *>(space)->install_port(start, end, mask, mirror, rtag, NULL)
#define memory_install_read_bank(space, start, end, mask, mirror, rtag) \
	const_cast<address_space *>(space)->install_bank(start, end, mask, mirror, rtag, NULL)
#define memory_install_rom(space, start, end, mask, mirror, baseptr) \
	const_cast<address_space *>(space)->install_ram(start, end, mask, mirror, ROW_READ, baseptr)
#define memory_unmap_read(space, start, end, mask, mirror) \
	const_cast<address_space *>(space)->unmap(start, end, mask, mirror, ROW_READ, false)
#define memory_nop_read(space, start, end, mask, mirror) \
	const_cast<address_space *>(space)->unmap(start, end, mask, mirror, ROW_READ, true)

// wrappers for dynamic write handler installation
#define memory_install_write8_handler(space, start, end, mask, mirror, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, whandler, #whandler)
#define memory_install_write16_handler(space, start, end, mask, mirror, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, whandler, #whandler)
#define memory_install_write32_handler(space, start, end, mask, mirror, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, whandler, #whandler)
#define memory_install_write64_handler(space, start, end, mask, mirror, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, whandler, #whandler)

#define memory_install_write8_device_handler(space, device, start, end, mask, mirror, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, whandler, #whandler)
#define memory_install_write16_device_handler(space, device, start, end, mask, mirror, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, whandler, #whandler)
#define memory_install_write32_device_handler(space, device, start, end, mask, mirror, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, whandler, #whandler)
#define memory_install_write64_device_handler(space, device, start, end, mask, mirror, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, whandler, #whandler)

#define memory_install_write_port(space, start, end, mask, mirror, wtag) \
	const_cast<address_space *>(space)->install_port(start, end, mask, mirror, NULL, wtag)
#define memory_install_write_bank(space, start, end, mask, mirror, wtag) \
	const_cast<address_space *>(space)->install_bank(start, end, mask, mirror, NULL, wtag)
#define memory_install_writeonly(space, start, end, mask, mirror, baseptr) \
	const_cast<address_space *>(space)->install_ram(start, end, mask, mirror, ROW_WRITE, baseptr)
#define memory_unmap_write(space, start, end, mask, mirror) \
	const_cast<address_space *>(space)->unmap(start, end, mask, mirror, ROW_WRITE, false)
#define memory_nop_write(space, start, end, mask, mirror) \
	const_cast<address_space *>(space)->unmap(start, end, mask, mirror, ROW_WRITE, true)

// wrappers for dynamic read/write handler installation
#define memory_install_readwrite8_handler(space, start, end, mask, mirror, rhandler, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler)
#define memory_install_readwrite16_handler(space, start, end, mask, mirror, rhandler, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler)
#define memory_install_readwrite32_handler(space, start, end, mask, mirror, rhandler, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler)
#define memory_install_readwrite64_handler(space, start, end, mask, mirror, rhandler, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler)

#define memory_install_readwrite8_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler)
#define memory_install_readwrite16_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler)
#define memory_install_readwrite32_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler)
#define memory_install_readwrite64_device_handler(space, device, start, end, mask, mirror, rhandler, whandler) \
	const_cast<address_space *>(space)->install_legacy_handler(*(device), start, end, mask, mirror, rhandler, #rhandler, whandler, #whandler)

#define memory_install_readwrite_port(space, start, end, mask, mirror, rtag, wtag) \
	const_cast<address_space *>(space)->install_port(start, end, mask, mirror, rtag, wtag)
#define memory_install_readwrite_bank(space, start, end, mask, mirror, tag) \
	const_cast<address_space *>(space)->install_bank(start, end, mask, mirror, tag, tag)
#define memory_install_ram(space, start, end, mask, mirror, baseptr) \
	const_cast<address_space *>(space)->install_ram(start, end, mask, mirror, ROW_READWRITE, baseptr)
#define memory_unmap_readwrite(space, start, end, mask, mirror) \
	const_cast<address_space *>(space)->unmap(start, end, mask, mirror, ROW_READWRITE, false)
#define memory_nop_readwrite(space, start, end, mask, mirror) \
	const_cast<address_space *>(space)->unmap(start, end, mask, mirror, ROW_READWRITE, true)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const char *const address_space_names[ADDRESS_SPACES];



//**************************************************************************
//  FUNCTION PROTOTYPES FOR CORE MEMORY FUNCTIONS
//**************************************************************************

// initialize the memory system
void memory_init(running_machine *machine);

// configure the addresses for a bank
void memory_configure_bank(running_machine *machine, const char *tag, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(1, 5);

// configure the decrypted addresses for a bank
void memory_configure_bank_decrypted(running_machine *machine, const char *tag, int startentry, int numentries, void *base, offs_t stride) ATTR_NONNULL(1, 5);

// select one pre-configured entry to be the new bank base
void memory_set_bank(running_machine *machine, const char *tag, int entrynum) ATTR_NONNULL(1);

// return the currently selected bank
int memory_get_bank(running_machine *machine, const char *tag) ATTR_NONNULL(1);

// set the absolute address of a bank base
void memory_set_bankptr(running_machine *machine, const char *tag, void *base) ATTR_NONNULL(1, 3);

// dump the internal memory tables to the given file
void memory_dump(running_machine *machine, FILE *file);



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

//-------------------------------------------------
//  read_raw_ptr - return a pointer to valid RAM
//  referenced by the address, or NULL if no RAM
//  backing that address
//-------------------------------------------------

inline void *direct_read_data::read_raw_ptr(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return &m_raw[byteaddress & m_bytemask];
	return NULL;
}

inline void *direct_read_data::read_decrypted_ptr(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return &m_decrypted[byteaddress & m_bytemask];
	return NULL;
}


//-------------------------------------------------
//  read_raw_byte - read a byte via the
//  direct_read_data class
//-------------------------------------------------

inline UINT8 direct_read_data::read_raw_byte(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return m_raw[byteaddress & m_bytemask];
	return m_space.read_byte(byteaddress);
}

inline UINT8 direct_read_data::read_decrypted_byte(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return m_decrypted[byteaddress & m_bytemask];
	return m_space.read_byte(byteaddress);
}


//-------------------------------------------------
//  read_raw_word - read a word via the
//  direct_read_data class
//-------------------------------------------------

inline UINT16 direct_read_data::read_raw_word(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT16 *>(&m_raw[byteaddress & m_bytemask]);
	return m_space.read_word(byteaddress);
}

inline UINT16 direct_read_data::read_decrypted_word(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT16 *>(&m_decrypted[byteaddress & m_bytemask]);
	return m_space.read_word(byteaddress);
}


//-------------------------------------------------
//  read_raw_dword - read a dword via the
//  direct_read_data class
//-------------------------------------------------

inline UINT32 direct_read_data::read_raw_dword(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT32 *>(&m_raw[byteaddress & m_bytemask]);
	return m_space.read_dword(byteaddress);
}

inline UINT32 direct_read_data::read_decrypted_dword(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT32 *>(&m_decrypted[byteaddress & m_bytemask]);
	return m_space.read_dword(byteaddress);
}


//-------------------------------------------------
//  read_raw_qword - read a qword via the
//  direct_read_data class
//-------------------------------------------------

inline UINT64 direct_read_data::read_raw_qword(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT64 *>(&m_raw[byteaddress & m_bytemask]);
	return m_space.read_qword(byteaddress);
}

inline UINT64 direct_read_data::read_decrypted_qword(offs_t byteaddress)
{
	if (address_is_valid(byteaddress))
		return *reinterpret_cast<UINT64 *>(&m_decrypted[byteaddress & m_bytemask]);
	return m_space.read_qword(byteaddress);
}


#endif	/* __MEMORY_H__ */
