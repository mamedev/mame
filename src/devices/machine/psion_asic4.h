// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC4

******************************************************************************/

#ifndef MAME_MACHINE_PSION_ASIC4_H
#define MAME_MACHINE_PSION_ASIC4_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_asic4_device

class psion_asic4_device : public device_t, public device_memory_interface
{
public:
	psion_asic4_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void set_ext_info_byte(uint16_t info) { m_info_byte = info; }

	void data_w(uint16_t data);
	uint8_t data_r();

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

private:
	address_space_config m_space_config;

	address_space *m_space;

	static constexpr uint16_t NULL_FRAME    = 0x000;
	static constexpr uint16_t CONTROL_FRAME = 0x100;
	static constexpr uint16_t DATA_FRAME    = 0x200;

	uint32_t m_addr_latch;
	uint8_t m_addr_writes;

	uint16_t m_info_byte;
	uint8_t m_sibo_control;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_ASIC4, psion_asic4_device)

#endif // MAME_MACHINE_PSION_ASIC4_H
