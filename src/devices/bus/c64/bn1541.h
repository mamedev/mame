// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/**********************************************************************

    SpeedDOS / Burst Nibbler 1541/1571 Parallel Cable emulation

**********************************************************************/

#ifndef MAME_BUS_C64_BN1541_H
#define MAME_BUS_C64_BN1541_H

#pragma once


#include "user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_c64_floppy_parallel_interface

class device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	device_c64_floppy_parallel_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_c64_floppy_parallel_interface();

	virtual void parallel_data_w(uint8_t data) = 0;
	virtual void parallel_strobe_w(int state) = 0;

	device_c64_floppy_parallel_interface *m_other;

protected:
	uint8_t m_parallel_data;
};


// ======================> c64_bn1541_device

class c64_bn1541_device : public device_t,
		public device_pet_user_port_interface,
		public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c64_bn1541_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(uint8_t data) override;
	virtual void parallel_strobe_w(int state) override;

	// device_pet_user_port_interface overrides
	virtual void input_8(int state) override;
	virtual void input_c(int state) override { if (state) m_parallel_output |= 1; else m_parallel_output &= ~1; update_output(); }
	virtual void input_d(int state) override { if (state) m_parallel_output |= 2; else m_parallel_output &= ~2; update_output(); }
	virtual void input_e(int state) override { if (state) m_parallel_output |= 4; else m_parallel_output &= ~4; update_output(); }
	virtual void input_f(int state) override { if (state) m_parallel_output |= 8; else m_parallel_output &= ~8; update_output(); }
	virtual void input_h(int state) override { if (state) m_parallel_output |= 16; else m_parallel_output &= ~16; update_output(); }
	virtual void input_j(int state) override { if (state) m_parallel_output |= 32; else m_parallel_output &= ~32; update_output(); }
	virtual void input_k(int state) override { if (state) m_parallel_output |= 64; else m_parallel_output &= ~64; update_output(); }
	virtual void input_l(int state) override { if (state) m_parallel_output |= 128; else m_parallel_output &= ~128; update_output(); }

private:
	void update_output();
	uint8_t m_parallel_output;
};


// device type definition
DECLARE_DEVICE_TYPE(C64_BN1541, c64_bn1541_device)


#endif // MAME_BUS_C64_BN1541_H
