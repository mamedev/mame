// license:BSD-3-Clause
// copyright-holders:AJR
/******************************************************************************

    Video System C7-01 GGA

*******************************************************************************
                                 _____   _____
                          ?   1 |*    \_/     | 64  ?
                         H0   2 |             | 63  ?
                         H1   3 |             | 62  V.I
                         H2   4 |             | 61  CS
                          ?   5 |             | 60  D0
                          ?   6 |             | 59  D1
                          ?   7 |             | 58  D2
                          ?   8 |             | 57  D3
                          ?   9 |             | 56  D4
                          ?  10 |             | 55  D5
                         H3  11 |             | 54  D6
                         H4  12 |             | 53  D7
                         H5  13 |             | 52  RES
                          ?  14 |             | 51  CLOCK
                          ?  15 |  V-SYSTEM   | 50  INV
                         H6  16 |  C7-01      | 49  ?
                         H7  17 |  GGA        | 48  A0?
                          ?  18 |             | 47  ?
                          ?  19 |             | 46  ?
                          ?  20 |             | 45  ?
                         H8  21 |             | 44  ?
                         V0  22 |             | 43  V7
                        /H3  23 |             | 42  /BLANK
                        /H2  24 |             | 41  VEL? (INTCL)
                        /H1  25 |             | 40  /SYNC
                          ?  26 |             | 39  ?
                          ?  27 |             | 38  ?
                         V1  28 |             | 37  ?
                         V2  29 |             | 36  V7
                         V3  30 |             | 35  V6
                        /H0  31 |             | 34  V4
                          ?  32 |_____________| 33  ?

******************************************************************************/

#ifndef MAME_VSYSTEM_VSYSTEM_GGA_H
#define MAME_VSYSTEM_VSYSTEM_GGA_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vsystem_gga_device

class vsystem_gga_device : public device_t, public device_video_interface
{
public:
	// construction/destruction
	vsystem_gga_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// static configuration
	auto write_cb() { return m_write_cb.bind(); }

	// memory handlers
	void write(offs_t offset, u8 data);

	// temporary accessor
	u8 reg(u8 offset) const { return m_regs[offset]; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// internal state
	u8 m_address_latch;
	u8 m_regs[16];
	devcb_write8 m_write_cb;
};

// device type definition
DECLARE_DEVICE_TYPE(VSYSTEM_GGA, vsystem_gga_device)

#endif // MAME_VSYSTEM_VSYSTEM_GGA_H
