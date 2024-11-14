// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Sharp Scoop peripheral chip emulation skeleton

***************************************************************************/

#ifndef MAME_MACHINE_SCOOP
#define MAME_MACHINE_SCOOP

#pragma once

class scoop_device : public device_t
{
public:
	scoop_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <unsigned Line> auto gpio_out() { return m_gpio_out[Line].bind(); }
	template <unsigned Line> void gpio_in(int state) { gpio_in((uint16_t)Line, state); }

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void gpio_in(const uint16_t line, const int state);
	void update_gpio_direction(const uint16_t old_dir);
	void update_gpio_outputs(const uint16_t old_latch, const uint16_t changed);

	uint16_t m_gpwr;
	uint16_t m_gpio_in_latch;
	uint16_t m_gpcr;

	devcb_write_line::array<13> m_gpio_out;
};

DECLARE_DEVICE_TYPE(SCOOP, scoop_device)

#endif // MAME_MACHINE_SCOOP
