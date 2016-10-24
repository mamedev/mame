// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/**********************************************************************

    geoCable Centronics Cable emulation

**********************************************************************/

#pragma once

#ifndef __C64_GEOCABLE__
#define __C64_GEOCABLE__


#include "emu.h"
#include "user.h"
#include "bus/centronics/ctronics.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_geocable_device

class c64_geocable_device : public device_t,
	public device_pet_user_port_interface
{
public:
	// construction/destruction
	c64_geocable_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_pet_user_port_interface overrides
	virtual void input_c(int state) override { m_centronics->write_data0(state); }
	virtual void input_d(int state) override { m_centronics->write_data1(state); }
	virtual void input_e(int state) override { m_centronics->write_data2(state); }
	virtual void input_f(int state) override { m_centronics->write_data3(state); }
	virtual void input_h(int state) override { m_centronics->write_data4(state); }
	virtual void input_j(int state) override { m_centronics->write_data5(state); }
	virtual void input_k(int state) override { m_centronics->write_data6(state); }
	virtual void input_l(int state) override { m_centronics->write_data7(state); }
	virtual void input_m(int state) override { m_centronics->write_strobe(state); }

private:
	required_device<centronics_device> m_centronics;
};


// device type definition
extern const device_type C64_GEOCABLE;


#endif
