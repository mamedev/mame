// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation

    TODO:
        - Serial UART
        - I2C
        - SPI

**********************************************************************/

#ifndef DEVICES_MACHINE_SPG2XX_H
#define DEVICES_MACHINE_SPG2XX_H

#pragma once

#include "emu.h"
#include "cpu/unsp/unsp.h"
#include "screen.h"

class spg2xx_device : public device_t
{
public:
	template <typename T, typename U>
	spg2xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg2xx_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	spg2xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }

	auto eeprom_w() { return m_eeprom_w.bind(); }
	auto eeprom_r() { return m_eeprom_r.bind(); }

	auto uart_tx() { return m_uart_tx.bind(); }
	auto uart_rx() { return m_uart_rx.bind(); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);

protected:
	enum
	{
		PAGE_ENABLE_MASK        = 0x0008,
		PAGE_BLANK_MASK			= 0x0004,

		SPRITE_ENABLE_MASK		= 0x0001,
		SPRITE_COORD_TL_MASK	= 0x0002,

		PAGE_DEPTH_FLAG_MASK    = 0x3000,
		PAGE_DEPTH_FLAG_SHIFT   = 12,
		PAGE_TILE_HEIGHT_MASK   = 0x00c0,
		PAGE_TILE_HEIGHT_SHIFT  = 6,
		PAGE_TILE_WIDTH_MASK    = 0x0030,
		PAGE_TILE_WIDTH_SHIFT   = 4,
		TILE_X_FLIP             = 0x0004,
		TILE_Y_FLIP             = 0x0008
	};

	DECLARE_READ16_MEMBER(video_r);
	DECLARE_WRITE16_MEMBER(video_w);
	DECLARE_READ16_MEMBER(audio_r);
	DECLARE_WRITE16_MEMBER(audio_w);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE16_MEMBER(io_w);

	void check_irqs(const uint16_t changed);
	inline void check_video_irq();

	void spg2xx_map(address_map &map);

	static const device_timer_id TIMER_TMB1 = 0;
	static const device_timer_id TIMER_TMB2 = 1;
	static const device_timer_id TIMER_SCREENPOS = 2;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void do_gpio(uint32_t offset);
	void do_i2c();
	void do_video_dma(uint32_t len);
	void do_cpu_dma(uint32_t len);

	void blit(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile);
	void blit_page(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, uint32_t bitmap_addr, uint16_t *regs);
	void blit_sprite(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth, uint32_t base_addr);
	void blit_sprites(bitmap_rgb32 &bitmap, const rectangle &cliprect, int depth);
	inline uint8_t expand_rgb5_to_rgb8(uint8_t val);
	inline uint8_t mix_channel(uint8_t a, uint8_t b);
	void mix_pixel(uint32_t offset, uint16_t rgb);
	void set_pixel(uint32_t offset, uint16_t rgb);

	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3, 4);

	struct
	{
		uint8_t r, g, b;
	}
	m_screenbuf[320 * 240];

	bool m_hide_page0;
	bool m_hide_page1;
	bool m_hide_sprites;
	bool m_debug_sprites;
	bool m_debug_blit;
	uint8_t m_sprite_index_to_debug;

	uint16_t m_video_regs[0x100];
	uint16_t m_io_regs[0x200];

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_write8 m_eeprom_w;
	devcb_read8 m_eeprom_r;

	devcb_write8 m_uart_tx;
	devcb_read8 m_uart_rx;

	emu_timer *m_tmb1;
	emu_timer *m_tmb2;
	emu_timer *m_screenpos_timer;

	required_device<cpu_device> m_cpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_spriteram;
};

DECLARE_DEVICE_TYPE(SPG2XX, spg2xx_device)

#endif // DEVICES_MACHINE_SPG2XX_H
