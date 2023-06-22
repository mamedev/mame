// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC3

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
	psion_asic3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto adin_cb() { return m_adin_cb.bind(); }

	void data_w(uint16_t data);
	uint8_t data_r();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	static constexpr uint16_t NULL_FRAME    = 0x000;
	static constexpr uint16_t CONTROL_FRAME = 0x100;
	static constexpr uint16_t DATA_FRAME    = 0x200;

	devcb_read16 m_adin_cb;

	uint8_t m_sibo_control;
	uint8_t m_a3_control1;
	uint8_t m_a3_control2;
	uint8_t m_a3_control3;
	uint8_t m_a3_status;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_ASIC3, psion_asic3_device)

#endif // MAME_MACHINE_PSION_ASIC3_H
