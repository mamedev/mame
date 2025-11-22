// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC7

******************************************************************************/

#ifndef MAME_MACHINE_PSION_ASIC7_H
#define MAME_MACHINE_PSION_ASIC7_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_asic7_device

class psion_asic7_device : public device_t
{
public:
	// construction/destruction
	psion_asic7_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock);

	auto as2rd()  { return m_as2rd_callback.bind(); }
	auto as2wr()  { return m_as2wr_callback.bind(); }
	auto pgsel()  { return m_pgsel_callback.bind(); }
	auto lcdcom() { return m_lcdcom_callback.bind(); }
	auto caps()   { return m_caps_callback.bind(); }
	auto numl()   { return m_numl_callback.bind(); }
	auto scrl()   { return m_scrl_callback.bind(); }
	auto batt()   { return m_batt_callback.bind(); }
	auto stby()   { return m_stby_callback.bind(); }

	uint8_t io_r(offs_t offset, uint8_t mem_mask);
	void io_w(offs_t offset, uint8_t data, uint8_t mem_mask);

	void intr_w(int state) { m_intr_state = state; update_ramen(); };
	void nmi_w(int state) { m_nmi_state = state; update_ramen(); };

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_ramen();

	devcb_read8 m_as2rd_callback;
	devcb_write8 m_as2wr_callback;
	devcb_write_line m_pgsel_callback;
	devcb_write_line m_lcdcom_callback;
	devcb_write_line m_caps_callback;
	devcb_write_line m_numl_callback;
	devcb_write_line m_scrl_callback;
	devcb_write_line m_batt_callback;
	devcb_write_line m_stby_callback;

	uint8_t m_a7_control;
	uint8_t m_a7_key_command;
	uint16_t m_a7_access_key;

	int m_intr_state;
	int m_nmi_state;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_ASIC7, psion_asic7_device)

#endif // MAME_MACHINE_PSION_ASIC7_H
