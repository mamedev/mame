// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC3/PS34

******************************************************************************/

#ifndef MAME_MACHINE_PSION_ASIC3_H
#define MAME_MACHINE_PSION_ASIC3_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_asic3_device

class psion_asic3_device : public device_t
{
public:
	// callbacks
	auto adin_cb() { return m_adin_cb.bind(); }

	virtual void data_w(uint16_t data) { }
	virtual uint8_t data_r() { return 0x00; }

protected:
	psion_asic3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	static constexpr uint16_t NULL_FRAME    = 0x000;
	static constexpr uint16_t CONTROL_FRAME = 0x100;
	static constexpr uint16_t DATA_FRAME    = 0x200;

	devcb_read16 m_adin_cb;

	uint8_t m_sibo_control;
	uint8_t m_a3_control1;
	uint8_t m_a3_control2;
	uint8_t m_a3_control3;
};


// ======================> psion_psu_asic5_device

class psion_psu_asic5_device : public psion_asic3_device
{
public:
	// construction/destruction
	psion_psu_asic5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void data_w(uint16_t data) override;
	virtual uint8_t data_r() override;
};


// ======================> psion_psu_asic3_device

class psion_psu_asic3_device : public psion_asic3_device
{
public:
	// construction/destruction
	psion_psu_asic3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual void data_w(uint16_t data) override;
	virtual uint8_t data_r() override;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_PSU_ASIC3, psion_psu_asic3_device)
DECLARE_DEVICE_TYPE(PSION_PSU_ASIC5, psion_psu_asic5_device)

#endif // MAME_MACHINE_PSION_ASIC3_H
