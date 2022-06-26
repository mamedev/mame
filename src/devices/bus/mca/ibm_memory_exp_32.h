// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
/***************************************************************************

    IBM 2-8MB Memory Expansion Adapter
    FRU 90X9556

    32-bit memory expansion card. 4 72-pin SIMM slots, 8MB max capacity.

***************************************************************************/

#ifndef MAME_BUS_MCA32_IBM_MEMORY_EXP_H
#define MAME_BUS_MCA32_IBM_MEMORY_EXP_H

#pragma once

#include "mca.h"
#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mca32_ibm_memory_exp_device

class mca32_ibm_memory_exp_device :
		public device_t,
		public device_mca32_card_interface
{
public:
	// construction/destruction
	mca32_ibm_memory_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void unmap() override;
    virtual void remap() override;

    virtual void update_pos_data_1(uint8_t data) override;
	virtual void update_pos_data_2(uint8_t data) override;
	virtual void update_pos_data_3(uint8_t data) override;
	virtual void update_pos_data_4(uint8_t data) override;

protected:
	mca32_ibm_memory_exp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

	virtual bool map_has_changed() override;

	required_device<ram_device> m_expansion_ram;
private:
	uint8_t m_subaddress_hi, m_subaddress_lo;

	bool 	m_is_mapped;
};

// device type definition
DECLARE_DEVICE_TYPE(MCA32_IBM_MEMORY_EXP, mca32_ibm_memory_exp_device)

#endif