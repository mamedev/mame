// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller Dot Matrix Display

// A 128x32 plasma display with 16 pages and refreshed at 240Hz (for PWM luminosity control)

#ifndef MAME_PINBALL_WPC_DMD_H
#define MAME_PINBALL_WPC_DMD_H

#include "machine/timer.h"

class wpc_dmd_device : public device_t
{
public:
	wpc_dmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_dmd_device();

	void registers(address_map &map) ATTR_COLD;

	void bank0_w(uint8_t data);
	void bank2_w(uint8_t data);
	void bank4_w(uint8_t data);
	void bank6_w(uint8_t data);
	void bank8_w(uint8_t data);
	void banka_w(uint8_t data);
	void visible_page_w(uint8_t data);
	void firq_scanline_w(uint8_t data);

	auto scanline_callback() { return scanline_cb.bind(); }

protected:
	devcb_write_line scanline_cb;
	required_memory_bank dmd0, dmd2, dmd4, dmd6, dmd8, dmda;

	uint8_t cur_scanline, visible_page, firq_scanline;
	std::vector<uint8_t> ram, screen_buffer, bitcounts;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

DECLARE_DEVICE_TYPE(WPC_DMD, wpc_dmd_device)

#endif // MAME_PINBALL_WPC_DMD_H
