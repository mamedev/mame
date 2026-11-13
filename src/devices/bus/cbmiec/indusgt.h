// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Indus GT disk drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_INDUSGT_H
#define MAME_BUS_CBMIEC_INDUSGT_H

#pragma once

#include "c1541.h"

class indus_gt_device : public c1541_device_base
{
public:
	indus_gt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(protect_changed);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	void indusgt_mem(address_map &map) ATTR_COLD;
	u8 sensor_r();
	void floppy_wpt(floppy_image_device *floppy, int state);
	template <unsigned Digit> void digit_w(u8 data) { m_digits[Digit] = u8(~data); }

	required_ioport m_panel;
	output_finder<2> m_digits;
	output_finder<> m_power_led;
	output_finder<> m_protect_led;

	bool m_protect;
};

DECLARE_DEVICE_TYPE(INDUS_GT, indus_gt_device)

#endif // MAME_BUS_CBMIEC_INDUSGT_H
