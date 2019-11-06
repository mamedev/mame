// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

	SNK NEO-ZMC Memory controller
	reference : https://wiki.neogeodev.org/index.php?title=NEO-ZMC

***************************************************************************/

#ifndef MAME_MACHINE_NEO_ZMC_H
#define MAME_MACHINE_NEO_ZMC_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> neo_zmc_device

class neo_zmc_device : public device_t, public device_memory_interface
{
public:
	// construction/destruction
	neo_zmc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// I/O operations
	u8 banked_space_r(offs_t offset);
	u8 set_bank_r(offs_t offset);

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;
private:
	address_space                                *m_data;
	memory_access_cache<0, 0, ENDIANNESS_LITTLE> *m_cache;

	u8 m_bank[4];
};

// device type definition
DECLARE_DEVICE_TYPE(NEO_ZMC, neo_zmc_device)

#endif // MAME_MACHINE_NEO_ZMC_H
