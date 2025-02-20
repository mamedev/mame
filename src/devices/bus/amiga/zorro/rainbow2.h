// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Omega Datentechnik Rainbow II
    Ingenieurbuero Helfrich Rainbow II
    BSC FrameMaster

    24-bit framebuffer for Amiga 2000/3000/4000

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_RAINBOW2_H
#define MAME_BUS_AMIGA_ZORRO_RAINBOW2_H

#pragma once

#include "zorro.h"
#include "machine/autoconfig.h"
#include "screen.h"


namespace bus::amiga::zorro {

class rainbow2_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	rainbow2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	rainbow2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type, uint16_t manufacturer);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;
	virtual void busrst_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	void control_w(offs_t offset, uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
	std::unique_ptr<uint16_t[]> m_vram;
	required_ioport m_jumper;
	uint16_t m_manufacturer;
	uint8_t m_control;
};

class framemaster_device : public rainbow2_device
{
public:
	framemaster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_RAINBOW2, bus::amiga::zorro, rainbow2_device)
DECLARE_DEVICE_TYPE_NS(AMIGA_FRAMEMASTER, bus::amiga::zorro, framemaster_device)

#endif // MAME_BUS_AMIGA_ZORRO_RAINBOW2_H
