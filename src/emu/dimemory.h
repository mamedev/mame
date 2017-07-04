// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    dimemory.h

    Device memory interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DIMEMORY_H
#define MAME_EMU_DIMEMORY_H


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// Translation intentions
constexpr int TRANSLATE_TYPE_MASK       = 0x03;     // read write or fetch
constexpr int TRANSLATE_USER_MASK       = 0x04;     // user mode or fully privileged
constexpr int TRANSLATE_DEBUG_MASK      = 0x08;     // debug mode (no side effects)

constexpr int TRANSLATE_READ            = 0;        // translate for read
constexpr int TRANSLATE_WRITE           = 1;        // translate for write
constexpr int TRANSLATE_FETCH           = 2;        // translate for instruction fetch
constexpr int TRANSLATE_READ_USER       = (TRANSLATE_READ | TRANSLATE_USER_MASK);
constexpr int TRANSLATE_WRITE_USER      = (TRANSLATE_WRITE | TRANSLATE_USER_MASK);
constexpr int TRANSLATE_FETCH_USER      = (TRANSLATE_FETCH | TRANSLATE_USER_MASK);
constexpr int TRANSLATE_READ_DEBUG      = (TRANSLATE_READ | TRANSLATE_DEBUG_MASK);
constexpr int TRANSLATE_WRITE_DEBUG     = (TRANSLATE_WRITE | TRANSLATE_DEBUG_MASK);
constexpr int TRANSLATE_FETCH_DEBUG     = (TRANSLATE_FETCH | TRANSLATE_DEBUG_MASK);



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DEVICE_ADDRESS_MAP(_space, _map) \
	device_memory_interface::static_set_addrmap(*device, _space, ADDRESS_MAP_NAME(_map));

#define MCFG_DEVICE_REMOVE_ADDRESS_MAP(_space) \
	device_memory_interface::static_set_addrmap(*device, _space, nullptr);

#define MCFG_DEVICE_PROGRAM_MAP(_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_PROGRAM, _map)

#define MCFG_DEVICE_DATA_MAP(_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_DATA, _map)

#define MCFG_DEVICE_IO_MAP(_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_IO, _map)

#define MCFG_DEVICE_DECRYPTED_OPCODES_MAP(_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_OPCODES, _map)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_memory_interface

class device_memory_interface : public device_interface
{
	friend class device_scheduler;

public:
	// construction/destruction
	device_memory_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_memory_interface();

	// configuration access
	address_map_constructor address_map(int spacenum = 0) const { return spacenum >= 0 && spacenum < int(m_address_map.size()) ? m_address_map[spacenum] : nullptr; }
	const address_space_config *space_config(int spacenum = 0) const { return spacenum >= 0 && spacenum < int(m_address_config.size()) ? m_address_config[spacenum] : nullptr; }
	int max_space_count() const { return m_address_config.size(); }

	// static inline configuration helpers
	static void static_set_addrmap(device_t &device, int spacenum, address_map_constructor map);
	void set_addrmap(int spacenum, address_map_constructor map);

	// basic information getters
	bool has_space(int index = 0) const { return index >= 0 && index < int(m_addrspace.size()) && m_addrspace[index]; }
	bool has_configured_map(int index = 0) const { return index >= 0 && index < int(m_address_map.size()) && m_address_map[index]; }
	address_space &space(int index = 0) const { assert(index >= 0 && index < int(m_addrspace.size()) && m_addrspace[index]); return *m_addrspace[index]; }

	// address space accessors
	void set_address_space(int spacenum, address_space &space);

	// address translation
	bool translate(int spacenum, int intention, offs_t &address) { return memory_translate(spacenum, intention, address); }

	// deliberately ambiguous functions; if you have the memory interface
	// just use it
	device_memory_interface &memory() { return *this; }

	void load_configs();

protected:
	// required overrides
	virtual std::vector<std::pair<int, const address_space_config *>> memory_space_config() const = 0;

	// optional operation overrides
	virtual bool memory_translate(int spacenum, int intention, offs_t &address);

	// interface-level overrides
	virtual void interface_validity_check(validity_checker &valid) const override;

	// configuration
	std::vector<address_map_constructor> m_address_map; // address maps for each address space

private:
	// internal state
	std::vector<const address_space_config *> m_address_config; // configuration for each space
	std::vector<address_space *>              m_addrspace; // reported address spaces
};

// iterator
typedef device_interface_iterator<device_memory_interface> memory_interface_iterator;



#endif  /* MAME_EMU_DIMEMORY_H */
