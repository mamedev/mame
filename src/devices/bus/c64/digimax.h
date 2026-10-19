// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    DigiMax 8-Bit Stereo Sound Cart emulation

**********************************************************************/

#ifndef MAME_BUS_C64_DIGIMAX_H
#define MAME_BUS_C64_DIGIMAX_H

#pragma once


#include "user.h"
#include "sound/dac.h"
#include "speaker.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_digimax_device

class c64_digimax_device : public device_t, public device_pet_user_port_interface
{
public:
	// construction/destruction
	c64_digimax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_pet_user_port_interface overrides
	virtual void input_8(int state) override { m_dac[m_a]->write(m_data); }
	virtual void input_m(int state) override { m_a = (m_a & 0x02) | (state << 0); }
	virtual void input_9(int state) override { m_a = (m_a & 0x01) | (state << 1); }
	virtual void input_c(int state) override { m_data = (m_data & 0xfe) | (state << 0); }
	virtual void input_d(int state) override { m_data = (m_data & 0xfd) | (state << 1); }
	virtual void input_e(int state) override { m_data = (m_data & 0xfb) | (state << 2); }
	virtual void input_f(int state) override { m_data = (m_data & 0xf7) | (state << 3); }
	virtual void input_h(int state) override { m_data = (m_data & 0xef) | (state << 4); }
	virtual void input_j(int state) override { m_data = (m_data & 0xdf) | (state << 5); }
	virtual void input_k(int state) override { m_data = (m_data & 0xbf) | (state << 6); }
	virtual void input_l(int state) override { m_data = (m_data & 0x7f) | (state << 7); }

private:
	required_device_array<dac_8bit_r2r_device, 4> m_dac;

	u8 m_data;
	offs_t m_a;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_DIGIMAX, c64_digimax_device)

#endif // MAME_BUS_C64_DIGIMAX_H
