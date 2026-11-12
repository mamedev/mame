// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_IGS_IGS036_H
#define MAME_IGS_IGS036_H

#pragma once

#include "cpu/arm7/arm7.h"
#include "machine/atmel_arm_aic.h"

class igs036_cpu_device : public arm946es_cpu_device
{
public:
	// construction/destruction
	igs036_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_aic_irq(int level, int state) { m_arm_aic->set_irq(level, state); }

	u8 const *get_encryption_table() const { return m_encryption_table; }

	auto set_gpio_r() { return m_read_gpio.bind(); }
	auto set_gpio_w() { return m_write_gpio.bind(); }
	auto set_do_decrypt() { return m_do_decrypt.bind(); }

	bool get_has_decrypted() const { return m_has_decrypted; }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void onboard_peripherals(address_map &map) ATTR_COLD;

	u32 encryption_control_r();
	void encryption_control_w(offs_t offset, u32 data, u32 mem_mask);
	u8 encryption_r(offs_t offset);
	void encryption_w(offs_t offset, u8 data);
	u32 gpio_r();
	void gpio_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	u32 gpio_out_r();

	u32 timer_r(offs_t offset);
	void timer_w(offs_t offset, u32 data, u32 mem_mask = ~0U);
	void timer_schedule(u32 channel);
	u32 timer_clock(u32 channel) const;
	TIMER_CALLBACK_MEMBER(timer_expired);

	required_device<arm_aic_device> m_arm_aic;
	devcb_read32 m_read_gpio;
	devcb_write32 m_write_gpio;
	devcb_write_line m_do_decrypt;

	emu_timer *m_tc_timer[3];
	u32 m_tc_regs[3][16];
	bool m_tc_enabled[3];
	u8 m_encryption_table[0x100];
	bool m_has_decrypted;
	u32 m_pio_out_data;
};

DECLARE_DEVICE_TYPE(IGS036,       igs036_cpu_device)

#endif // MAME_IGS_IGS036_H
