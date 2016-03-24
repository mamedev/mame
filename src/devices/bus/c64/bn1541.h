// license:BSD-3-Clause
// copyright-holders:Curt Coder, smf
/**********************************************************************

    SpeedDOS / Burst Nibbler 1541/1571 Parallel Cable emulation

**********************************************************************/

#pragma once

#ifndef __C64_BN1541__
#define __C64_BN1541__


#include "emu.h"
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

	virtual void parallel_data_w(UINT8 data) = 0;
	virtual void parallel_strobe_w(int state) = 0;

	device_c64_floppy_parallel_interface *m_other;

protected:
	UINT8 m_parallel_data;
};


// ======================> c64_bn1541_device

class c64_bn1541_device : public device_t,
	public device_pet_user_port_interface,
	public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c64_bn1541_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(UINT8 data) override;
	virtual void parallel_strobe_w(int state) override;

	// device_pet_user_port_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER(input_8) override;
	virtual WRITE_LINE_MEMBER(input_c) override { if (state) m_parallel_output |= 1; else m_parallel_output &= ~1; update_output(); }
	virtual WRITE_LINE_MEMBER(input_d) override { if (state) m_parallel_output |= 2; else m_parallel_output &= ~2; update_output(); }
	virtual WRITE_LINE_MEMBER(input_e) override { if (state) m_parallel_output |= 4; else m_parallel_output &= ~4; update_output(); }
	virtual WRITE_LINE_MEMBER(input_f) override { if (state) m_parallel_output |= 8; else m_parallel_output &= ~8; update_output(); }
	virtual WRITE_LINE_MEMBER(input_h) override { if (state) m_parallel_output |= 16; else m_parallel_output &= ~16; update_output(); }
	virtual WRITE_LINE_MEMBER(input_j) override { if (state) m_parallel_output |= 32; else m_parallel_output &= ~32; update_output(); }
	virtual WRITE_LINE_MEMBER(input_k) override { if (state) m_parallel_output |= 64; else m_parallel_output &= ~64; update_output(); }
	virtual WRITE_LINE_MEMBER(input_l) override { if (state) m_parallel_output |= 128; else m_parallel_output &= ~128; update_output(); }

private:
	void update_output();
	UINT8 m_parallel_output;
};


// device type definition
extern const device_type C64_BN1541;


#endif
