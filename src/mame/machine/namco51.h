// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_MACHINE_NAMCO51_H
#define MAME_MACHINE_NAMCO51_H

#pragma once

#include "cpu/mb88xx/mb88xx.h"


class namco_51xx_device : public device_t
{
public:
	namco_51xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_screen_tag(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }
	template <unsigned N> auto input_callback() { return m_in[N].bind(); }
	template <unsigned N> auto output_callback() { return m_out[N].bind(); }

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	required_device<screen_device> m_screen;
	devcb_read8 m_in[4];
	devcb_write8 m_out[2];

	int32_t m_lastcoins;
	int32_t m_lastbuttons;
	int32_t m_credits;
	int32_t m_coins[2];
	int32_t m_coins_per_cred[2];
	int32_t m_creds_per_coin[2];
	int32_t m_in_count;
	int32_t m_mode;
	int32_t m_coincred_mode;
	int32_t m_remap_joy;
};

DECLARE_DEVICE_TYPE(NAMCO_51XX, namco_51xx_device)


#endif // NAMCO_51XX
