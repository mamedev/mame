// license: BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_MEGADRIVE_CART_EVERDRIVE_H
#define MAME_BUS_MEGADRIVE_CART_EVERDRIVE_H

#pragma once

#include "machine/intelfsh.h"
#include "machine/spi_sdcard.h"

#include "slot.h"

class megadrive_hb_everdrive_device : public device_t,
									  public device_megadrive_cart_interface
{
public:
	megadrive_hb_everdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::MEDIA; }

	virtual void cart_map(address_map &map) override ATTR_COLD;
	virtual void time_io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
private:
	required_device<st_m29w640ft_device> m_flash;
	required_device<spi_sdcard_device> m_sdcard;
	memory_view m_game_mode;

	TIMER_CALLBACK_MEMBER(spi_clock_cb);
	u16 spi_data_r(offs_t offset, u16 mem_mask = ~0);
	void spi_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 state_r(offs_t offset, u16 mem_mask = ~0);
	void cfg_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	emu_timer *m_spi_clock;
	bool m_spi_clock_state;
	bool m_spi_full_speed;
	int m_spi_clock_cycles;
	int m_in_bit;
	u16 m_in_latch;
	u16 m_out_latch;
	bool m_spi_16;

	u32 m_rom_map_port;
	u16 m_vblv;
};

DECLARE_DEVICE_TYPE(MEGADRIVE_HB_EVERDRIVE,    megadrive_hb_everdrive_device)

#endif // MAME_BUS_MEGADRIVE_CART_DEV_H
