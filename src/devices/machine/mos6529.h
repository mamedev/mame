// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6529 Single Port Interface Adapter emulation

**********************************************************************
                            _____   _____
                   R/W   1 |*    \_/     | 20  Vdd
                    P0   2 |             | 19  _CS
                    P1   3 |             | 18  D0
                    P2   4 |             | 17  D1
                    P3   5 |   MOS6529   | 16  D2
                    P4   6 |             | 15  D3
                    P5   7 |             | 14  D4
                    P6   8 |             | 13  D5
                    P7   9 |             | 12  D6
                   Vss  10 |_____________| 11  D7

**********************************************************************/

#pragma once

#ifndef __MOS6529__
#define __MOS6529__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MOS6529_P0_HANDLER(_devcb) \
	devcb = &mos6529_device::set_p0_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6529_P1_HANDLER(_devcb) \
	devcb = &mos6529_device::set_p1_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6529_P2_HANDLER(_devcb) \
	devcb = &mos6529_device::set_p2_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6529_P3_HANDLER(_devcb) \
	devcb = &mos6529_device::set_p3_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6529_P4_HANDLER(_devcb) \
	devcb = &mos6529_device::set_p4_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6529_P5_HANDLER(_devcb) \
	devcb = &mos6529_device::set_p5_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6529_P6_HANDLER(_devcb) \
	devcb = &mos6529_device::set_p6_handler(*device, DEVCB_##_devcb);

#define MCFG_MOS6529_P7_HANDLER(_devcb) \
	devcb = &mos6529_device::set_p7_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6529_device

class mos6529_device :  public device_t
{
public:
	// construction/destruction
	mos6529_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_p0_handler(device_t &device, _Object object) { return downcast<mos6529_device &>(device).m_p0_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p1_handler(device_t &device, _Object object) { return downcast<mos6529_device &>(device).m_p1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p2_handler(device_t &device, _Object object) { return downcast<mos6529_device &>(device).m_p2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p3_handler(device_t &device, _Object object) { return downcast<mos6529_device &>(device).m_p3_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p4_handler(device_t &device, _Object object) { return downcast<mos6529_device &>(device).m_p4_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p5_handler(device_t &device, _Object object) { return downcast<mos6529_device &>(device).m_p5_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p6_handler(device_t &device, _Object object) { return downcast<mos6529_device &>(device).m_p6_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_p7_handler(device_t &device, _Object object) { return downcast<mos6529_device &>(device).m_p7_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( write_p0 ) { if (state) m_input |= 1; else m_input &= ~1; }
	DECLARE_WRITE_LINE_MEMBER( write_p1 ) { if (state) m_input |= 2; else m_input &= ~2; }
	DECLARE_WRITE_LINE_MEMBER( write_p2 ) { if (state) m_input |= 4; else m_input &= ~4; }
	DECLARE_WRITE_LINE_MEMBER( write_p3 ) { if (state) m_input |= 8; else m_input &= ~8; }
	DECLARE_WRITE_LINE_MEMBER( write_p4 ) { if (state) m_input |= 16; else m_input &= ~16; }
	DECLARE_WRITE_LINE_MEMBER( write_p5 ) { if (state) m_input |= 32; else m_input &= ~32; }
	DECLARE_WRITE_LINE_MEMBER( write_p6 ) { if (state) m_input |= 64; else m_input &= ~64; }
	DECLARE_WRITE_LINE_MEMBER( write_p7 ) { if (state) m_input |= 128; else m_input &= ~128; }

protected:
	// device-level overrides
	virtual void device_start() override;

	UINT8 m_input;

	devcb_write_line m_p0_handler;
	devcb_write_line m_p1_handler;
	devcb_write_line m_p2_handler;
	devcb_write_line m_p3_handler;
	devcb_write_line m_p4_handler;
	devcb_write_line m_p5_handler;
	devcb_write_line m_p6_handler;
	devcb_write_line m_p7_handler;
};


// device type definition
extern const device_type MOS6529;

#endif
