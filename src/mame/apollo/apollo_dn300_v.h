// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont, Chris Toshok, Vladimir Vukicevic
/*
 * apollo_dn300_v.h - the Apollo DN300/DN320 monochrome display controller
 */

#ifndef MAME_APOLLO_APOLLO_DN300_V_H
#define MAME_APOLLO_APOLLO_DN300_V_H

#pragma once

#include "screen.h"


class apollo_dn300_graphics : public device_t
{
public:
	apollo_dn300_graphics(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t reg_r(offs_t offset);
	void reg_w(offs_t offset, uint16_t data);

	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	auto irq_callback() { return m_irq_cb.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// the visible display, in the 1024x1024 bit image memory
	static constexpr int WIDTH = 1024;
	static constexpr int HEIGHT = 800;
	static constexpr int BUFFER_WIDTH = 1024;
	static constexpr int BUFFER_HEIGHT = 1024;
	static constexpr int IMAGE_MEMORY_SIZE = BUFFER_WIDTH * BUFFER_HEIGHT / 16;

	void blt();

	void screen_update1(bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vblank_state_changed(screen_device &screen, bool vblank_state);

	required_device<screen_device> m_screen;
	devcb_write_line m_irq_cb;

	uint16_t m_cr = 0U;
	uint16_t m_sr = 0U;
	bool m_blt_irq = false;
	bool m_eof_irq = false;
	void update_irq() { m_irq_cb((m_blt_irq || m_eof_irq) ? ASSERT_LINE : CLEAR_LINE); }

	uint16_t m_deb = 0U;
	uint16_t m_wssy = 0U;
	uint16_t m_wssx = 0U;
	uint16_t m_dcy = 0U;
	uint16_t m_dcx = 0U;
	uint16_t m_wsdy = 0U;
	uint16_t m_wsdx = 0U;

	uint8_t m_update_flag = 0U;
	uint8_t m_update_pending = 0U;

	std::unique_ptr<uint16_t[]> m_image_memory{};
};

DECLARE_DEVICE_TYPE(APOLLO_DN300_GRAPHICS, apollo_dn300_graphics)

#endif // MAME_APOLLO_APOLLO_DN300_V_H
