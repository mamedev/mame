// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM 2-8MB Memory Expansion Adapter
    FRU 90X9556

    32-bit memory expansion card. 4 72-pin SIMM slots, 8MB max capacity.

***************************************************************************/

#ifndef MAME_BUS_MCA16_IBM_MEMORY_EXP_H
#define MAME_BUS_MCA16_IBM_MEMORY_EXP_H

#pragma once

#include "mca.h"
#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca16_ibm_memory_exp_device

class mca16_ibm_memory_exp_device :
		public device_t,
		public device_mca16_card_interface
{
public:
	// construction/destruction
	mca16_ibm_memory_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void unmap() override;
    virtual void remap() override;

    virtual void update_pos_data_1(uint8_t data) override;
	virtual void update_pos_data_2(uint8_t data) override;
	virtual void update_pos_data_3(uint8_t data) override;
	virtual void update_pos_data_4(uint8_t data) override;

	virtual void pos_w(offs_t offset, uint8_t data) override;

protected:
	mca16_ibm_memory_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

    virtual const tiny_rom_entry *device_rom_region() const override;

	virtual bool map_has_changed() override;

	uint16_t 	io_31a0_r(offs_t offset);
	void 		io_31a0_w(offs_t offset, uint16_t data);

	required_device<ram_device> m_expansion_ram;
private:
	bool		m_ram_enabled;

	bool		m_rom_enabled;
	offs_t		m_rom_base;
	bool 		m_is_mapped;

	uint8_t 	m_consecutive_reads;
	uint8_t		m_command;
	
	offs_t 		m_address_register;

	bool		m_in_mode_e;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA16_IBM_MEMORY_EXP, mca16_ibm_memory_exp_device)

#endif