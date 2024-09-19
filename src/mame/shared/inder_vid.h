// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Inder / Dinamic Video */


/* */


#ifndef MAME_SHARED_INDER_VID_H
#define MAME_SHARED_INDER_VID_H

#pragma once


#include "cpu/tms34010/tms34010.h"
#include "video/ramdac.h"

#include "emupal.h"

DECLARE_DEVICE_TYPE(INDER_VIDEO, inder_vid_device)


class inder_vid_device : public device_t
/*  public device_video_interface */
{
public:
	// construction/destruction
	inder_vid_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// probably set by a register somewhere either on TMS side or 68k side
	void set_bpp(int bpp)
	{
		m_bpp_mode = bpp;
	}

	void megaphx_tms_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_shared_ptr<uint16_t> m_vram;
	required_device<palette_device> m_palette;
	required_device<tms34010_device> m_tms;

	int m_shiftfull = 0; // this might be a driver specific hack for a TMS bug.

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline);

	int m_bpp_mode;
};

#endif // MAME_SHARED_INDER_VID_H
