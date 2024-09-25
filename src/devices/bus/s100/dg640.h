// license:BSD-3-Clause
// copyright-holders:Robbbert
/**********************************************************************

    DG640 Video Card

**********************************************************************/

#ifndef MAME_BUS_S100_DG640_H
#define MAME_BUS_S100_DG640_H

#pragma once

#include "bus/s100/s100.h"

class dg640_device : public device_t, public device_s100_card_interface
{
public:
	// construction/destruction
	dg640_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_s100_card_interface overrides
	virtual u8 s100_smemr_r(offs_t offset) override;
	virtual void s100_mwrt_w(offs_t offset, u8 data) override;

private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// object finders
	required_region_ptr<u8> m_p_chargen;
	required_ioport m_dsw;

	// internal state
	std::unique_ptr<u8[]> m_p_videoram;
	std::unique_ptr<u8[]> m_p_attribram;
	u8 m_framecnt;
};


DECLARE_DEVICE_TYPE(S100_DG640, dg640_device)

#endif // MAME_BUS_S100_DG640_H
