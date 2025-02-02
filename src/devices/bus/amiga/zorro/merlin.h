// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    XPert/ProDev Merlin

    RTG graphics card for Amiga 2000/3000/4000

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_MERLIN_H
#define MAME_BUS_AMIGA_ZORRO_MERLIN_H

#pragma once

#include "zorro.h"
#include "machine/autoconfig.h"
#include "video/bt48x.h"
#include "video/pc_vga_tseng.h"


namespace bus::amiga::zorro {

class merlin_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	merlin_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;
	virtual void busrst_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	void mmio_map(address_map &map) ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<et4kw32i_vga_device> m_vga;
	required_device<bt482_device> m_ramdac;

	bool m_autoconfig_memory_done;
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_MERLIN, bus::amiga::zorro, merlin_device)

#endif // MAME_BUS_AMIGA_ZORRO_MERLIN_H
