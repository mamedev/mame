// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    UP9600 RS-232 adapter emulation

**********************************************************************/

#ifndef MAME_BUS_C64_UP9600_H
#define MAME_BUS_C64_UP9600_H

#pragma once


#include "user.h"
#include "bus/rs232/rs232.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_up9600_device

class c64_up9600_device : public device_t, public device_pet_user_port_interface
{
public:
	// construction/destruction
	c64_up9600_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_pet_user_port_interface overrides
	virtual void input_5(int state) override { m_rs232->write_txd(state); output_m(state); }
	virtual void input_d(int state) override { m_rs232->write_rts(!state); }
	virtual void input_e(int state) override { m_rs232->write_dtr(!state); }
	virtual void input_l(int state) override { output_6(state); }
	virtual void input_m(int state) override { m_rs232->write_txd(state); output_5(state); }

private:
	required_device<rs232_port_device> m_rs232;

	void rxd(int state) { output_c(state); output_7(state); output_b(state); }
	void cts(int state) { output_k(!state); }
	void dcd(int state) { output_h(!state); }
	void dsr(int state) { output_6(!state); output_l(!state); }
};


// device type definition
DECLARE_DEVICE_TYPE(UP9600, c64_up9600_device)

#endif // MAME_BUS_C64_UP9600_H
