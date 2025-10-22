// license: BSD-3-Clause
// copyright-holders: Peter Ferrie, Angelo Salese

#ifndef MAME_BUS_CENTRONICS_HASP_SAVQUEST_H
#define MAME_BUS_CENTRONICS_HASP_SAVQUEST_H

#pragma once

#include "ctronics.h"

class hasp_savquest_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	hasp_savquest_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_data0(int state) override { if (state) m_data_in |= 0x01; else m_data_in &= ~0x01; update_state(); }
	virtual void input_data1(int state) override { if (state) m_data_in |= 0x02; else m_data_in &= ~0x02; update_state(); }
	virtual void input_data2(int state) override { if (state) m_data_in |= 0x04; else m_data_in &= ~0x04; update_state(); }
	virtual void input_data3(int state) override { if (state) m_data_in |= 0x08; else m_data_in &= ~0x08; update_state(); }
	virtual void input_data4(int state) override { if (state) m_data_in |= 0x10; else m_data_in &= ~0x10; update_state(); }
	virtual void input_data5(int state) override { if (state) m_data_in |= 0x20; else m_data_in &= ~0x20; update_state(); }
	virtual void input_data6(int state) override { if (state) m_data_in |= 0x40; else m_data_in &= ~0x40; update_state(); }
	virtual void input_data7(int state) override { if (state) m_data_in |= 0x80; else m_data_in &= ~0x80; update_state(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void input_select_in(int state) override;

private:
	u8 m_data_in;
	u8 m_prev_data;
	int m_select_in_state;

//	int m_haspind = 0;
	int m_haspstate = 0;
	enum hasp_states
	{
		HASPSTATE_NONE,
		HASPSTATE_PASSBEG,
		HASPSTATE_PASSEND,
		HASPSTATE_READ
	};
//	int m_hasp_passind = 0;
//	uint8_t m_hasp_tmppass[0x29]{};
//	uint8_t m_port379 = 0;
//	int m_hasp_passmode = 0;
//	int m_hasp_prodind = 0;

	void update_state();
};

// device type definition
DECLARE_DEVICE_TYPE(HASP_SAVQUEST, hasp_savquest_device)


#endif // MAME_BUS_CENTRONICS_HASP_SAVQUEST_H
