// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Village Tronic Picasso II/Picasso II+

    RTG graphics card for Amiga 2000/3000/4000

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_PICASSO2_H
#define MAME_BUS_AMIGA_ZORRO_PICASSO2_H

#pragma once

#include "zorro.h"
#include "machine/autoconfig.h"
#include "video/pc_vga_cirrus.h"


namespace bus::amiga::zorro {

class picasso2p_device : public device_t, public device_memory_interface, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	picasso2p_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_memory_interface
	virtual space_config_vector memory_space_config() const override;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;
	virtual void busrst_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	void mmio_map(address_map &map) ATTR_COLD;
	void vga_map(address_map &map) ATTR_COLD;

	uint8_t vga0_r(offs_t offset);
	void vga0_w(offs_t offset, uint8_t data);
	uint8_t vga1_r(offs_t offset);
	void vga1_w(offs_t offset, uint8_t data);

	required_device<cirrus_gd5428_vga_device> m_vga;
	address_space_config m_vga_space_config;

	bool m_autoconfig_memory_done;
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_PICASSO2P, bus::amiga::zorro, picasso2p_device)

#endif // MAME_BUS_AMIGA_ZORRO_PICASSO2_H
