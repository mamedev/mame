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

#ifndef MAME_MACHINE_MOS6529_H
#define MAME_MACHINE_MOS6529_H

#pragma once




//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos6529_device

class mos6529_device :  public device_t
{
public:
	// construction/destruction
	mos6529_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <std::size_t Bit> auto p_handler() { return m_p_handler[Bit].bind(); }

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

	uint8_t m_input;

	devcb_write_line m_p_handler[8];
};


// device type definition
DECLARE_DEVICE_TYPE(MOS6529, mos6529_device)

#endif // MAME_MACHINE_MOS6529_H
