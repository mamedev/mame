// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpcsnd.h - Williams WPC pinball sound
 *
 *  Created on: 4/10/2013
 */

#ifndef MAME_PINBALL_WPCSND_H
#define MAME_PINBALL_WPCSND_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "sound/hc55516.h"
#include "sound/ymopm.h"


class wpcsnd_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	wpcsnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	required_device<cpu_device> m_cpu;
	required_device<ym2151_device> m_ym2151;
	required_device<hc55516_device> m_hc55516;
	required_memory_bank m_cpubank;
	required_memory_bank m_fixedbank;
	required_memory_region m_rom;

	void ctrl_w(uint8_t data);
	void data_w(uint8_t data);
	uint8_t ctrl_r();
	uint8_t data_r();

	template <typename T> void set_romregion(T &&tag) { m_rom.set_tag(std::forward<T>(tag)); }

	// callbacks
	auto reply_callback() { return m_reply_cb.bind(); }

	void wpcsnd_map(address_map &map) ATTR_COLD;
protected:
	// overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint8_t m_latch = 0;
	uint8_t m_reply = 0;
	bool m_reply_available;

	// callback
	devcb_write_line m_reply_cb;

	void ym2151_irq_w(int state);

	void bg_cvsd_clock_set_w(uint8_t data);
	void bg_cvsd_digit_clock_clear_w(uint8_t data);
	void rombank_w(uint8_t data);
	uint8_t latch_r();
	void latch_w(uint8_t data);
	void volume_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(WPCSND, wpcsnd_device)

#endif // MAME_PINBALL_WPCSND_H
