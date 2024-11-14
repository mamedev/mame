// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Classical Games/Protovision 4 Player Interface emulation

**********************************************************************/

#ifndef MAME_BUS_VIC20_4CGA_H
#define MAME_BUS_VIC20_4CGA_H

#pragma once


#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_4cga_device

class c64_4cga_device : public device_t,
	public device_pet_user_port_interface
{
public:
	// construction/destruction
	c64_4cga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void write_joy3_0(int state) { if (state) m_joy3 |= 1; else m_joy3 &= ~1; update_output(); }
	void write_joy3_1(int state) { if (state) m_joy3 |= 2; else m_joy3 &= ~2; update_output(); }
	void write_joy3_2(int state) { if (state) m_joy3 |= 4; else m_joy3 &= ~4; update_output(); }
	void write_joy3_3(int state) { if (state) m_joy3 |= 8; else m_joy3 &= ~8; update_output(); }

	void write_joy4_0(int state) { if (state) m_joy4 |= 1; else m_joy4 &= ~1; update_output(); }
	void write_joy4_1(int state) { if (state) m_joy4 |= 2; else m_joy4 &= ~2; update_output(); }
	void write_joy4_2(int state) { if (state) m_joy4 |= 4; else m_joy4 &= ~4; update_output(); }
	void write_joy4_3(int state) { if (state) m_joy4 |= 8; else m_joy4 &= ~8; update_output(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_pet_user_port_interface overrides
	virtual void input_l(int state) override;

private:
	void update_output();

	int m_port;
	uint8_t m_joy3;
	uint8_t m_joy4;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_4CGA, c64_4cga_device)


#endif // MAME_BUS_VIC20_4CGA_H
