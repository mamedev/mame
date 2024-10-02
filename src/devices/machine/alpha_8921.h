// license:BSD-3-Clause
// copyright-holders:cam900
/***************************************************************************

    Alpha denshi ALPHA-8921 emulation

***************************************************************************/

#ifndef MAME_MACHINE_ALPHA_8921_H
#define MAME_MACHINE_ALPHA_8921_H

#pragma once


// ======================> alpha_8921_device

class alpha_8921_device : public device_t
{
public:
	// construction/destruction
	alpha_8921_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inputs
	void clk_w(int state);
	void load_w(int state);
	void even_w(int state);
	void h_w(int state);
	void c_w(u32 data);

	// outputs
	u8 gad_r();
	u8 gbd_r();
	int dota_r();
	int dotb_r();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// inputs
	bool m_clk = false; // CLK pin
	bool m_load = false; // LOAD pin
	bool m_even = false; // EVEN pin
	bool m_h = false; // H pin
	u32 m_c = 0; // C data (32 bit)

	// outputs
	u8 m_gad = 0; // GAD data (4 bit)
	u8 m_gbd = 0; // GBD data (4 bit)
	bool m_dota = false; // DOTA pin
	bool m_dotb = false; // DOTB pin

	// internal status
	u32 m_sr = 0;
	u32 m_old_sr = ~0;
	bool m_old_even = true;
	bool m_old_h = true;
	void update_output();
};


// device type definition
DECLARE_DEVICE_TYPE(ALPHA_8921, alpha_8921_device)

#endif // MAME_MACHINE_ALPHA_8921_H
