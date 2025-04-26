// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_RBV_H
#define MAME_APPLE_RBV_H

#pragma once

#include "machine/pseudovia.h"

#include "screen.h"


// ======================> rbv_device

class rbv_device :  public device_t, public device_palette_interface
{
public:
	// construction/destruction
	rbv_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void map(address_map &map) ATTR_COLD;

	void set_ram_info(u32 *ram, u32 size);

	auto via6015_callback() { return write_6015.bind(); }
	auto irq_callback() { return write_irq.bind(); }

	template <u8 mask> void slot_irq_w(int state);
	void asc_irq_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_palette_interface implementation
	u32 palette_entries() const noexcept override;

private:
	devcb_write_line write_6015, write_irq;

	required_ioport m_io_montype;
	required_device<screen_device> m_screen;
	required_device<pseudovia_device> m_pseudovia;

	emu_timer *m_6015_timer;

	bool m_configured;
	s32 m_hres, m_vres;
	u8 m_montype;
	bool m_monochrome;

	u8 m_pal_address, m_pal_idx;
	u32 *m_ram_ptr;
	u32 m_ram_size;
	u8 m_video_config;

	u8 via2_video_config_r();
	void via2_video_config_w(u8 data);
	void via2_irq_w(int state);

	TIMER_CALLBACK_MEMBER(mac_6015_tick);

	u8 dac_r(offs_t offset);
	void dac_w(offs_t offset, u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	template <bool Mono> u32 update_screen(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

// device type definition
DECLARE_DEVICE_TYPE(RBV, rbv_device)

#endif // MAME_APPLE_RBV_H
