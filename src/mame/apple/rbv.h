// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_RBV_H
#define MAME_APPLE_RBV_H

#pragma once

#include "emupal.h"
#include "screen.h"

// ======================> rbv_device

class rbv_device :  public device_t
{
public:
	// construction/destruction
	rbv_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: rbv_device(mconfig, tag, owner, (uint32_t)0)
	{
	}

	rbv_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map) ATTR_COLD;

	void set_ram_info(u32 *ram, u32 size);

	auto via6015_callback() { return write_6015.bind(); }
	auto irq_callback() { return write_irq.bind(); }

	template <u8 mask> void slot_irq_w(int state);
	void asc_irq_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	devcb_write_line write_6015, write_irq;

	required_ioport m_montype;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	emu_timer *m_6015_timer;
	u8 m_pseudovia_regs[256], m_pseudovia_ier, m_pseudovia_ifr;
	u8 m_pal_address, m_pal_idx;
	u32 *m_ram_ptr;
	u32 m_ram_size;

	uint8_t pseudovia_r(offs_t offset);
	void pseudovia_w(offs_t offset, uint8_t data);
	void pseudovia_recalc_irqs();

	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	u8 dac_r(offs_t offset);
	void dac_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

// device type definition
DECLARE_DEVICE_TYPE(RBV, rbv_device)

#endif // MAME_APPLE_RBV_H
