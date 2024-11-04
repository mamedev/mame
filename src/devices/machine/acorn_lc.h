// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Acorn LC ASIC

*********************************************************************/

#ifndef MAME_MACHINE_ACORN_LC_H
#define MAME_MACHINE_ACORN_LC_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> acorn_lc_device

class acorn_lc_device : public device_t
						//public device_memory_interface,
						//public device_palette_interface,
						//public device_video_interface
{
public:
	acorn_lc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:

	enum
	{
		// VDSR = 512 - lines from start of VSYNC to start of display
		LC_VDSR_L = 0,
		LC_VDSR_M,
		LC_VDSR_H,

		// VDLC = lines per panel - 3
		LC_VDLR_L,
		LC_VDLR_M,
		LC_VDLR_H,

		// HDSR = 2047 - pixels of HSYNC + back porch
		LC_HDSR_L,
		LC_HDSR_M,
		LC_HDSR_H,

		// HDLR = horizontal display pixels / 8
		LC_HDLR_L,
		LC_HDLR_M ,
		LC_HDLR_H,

		LC_LICR_L,
		LC_LICR_M,
		LC_LICR_H,

		LC_RESET,
	};

	enum
	{
		LICR_CLOCK_IOEB  = 0 << 5,
		LICR_CLOCK_CRYS2 = 1 << 5,
		LICR_CLOCK_CRYS  = 2 << 5,
		LICR_CLOCK_MASK  = 3 << 5
	};

	u32 m_vdsr;
	u32 m_vdlr;
	u32 m_hdsr;
	u32 m_hdlr;
	u32 m_licr;
};

// device type definition
DECLARE_DEVICE_TYPE(ACORN_LC, acorn_lc_device)

#endif // MAME_MACHINE_ACORN_LC_H
