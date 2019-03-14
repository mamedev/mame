// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*****************************************************************************

    SunPlus SPG2xx-series SoC peripheral emulation

    TODO:
        - Serial UART
        - I2C
        - SPI

    Known SunPlus SPG2xx/u'nSP-based systems:

         D - SPG240 - Radica Skateboarder (Sunplus QL8041C die)
        ND - SPG243 - Some form of Leapfrog "edutainment" system
        ND - SPG243 - Star Wars: Clone Wars
        ND - SPG243 - Toy Story
        ND - SPG243 - Animal Art Studio
        ND - SPG243 - Finding Nemo
         D - SPG243 - The Batman
         D - SPG243 - Wall-E
         D - SPG243 - KenSingTon / Siatronics / Jungle Soft Vii
 Partial D - SPG200 - VTech V.Smile
        ND - unknown - Zone 40
         D - SPG243 - Zone 60
         D - SPG243 - Wireless 60
        ND - unknown - Wireless Air 60
        ND - Likely many more

**********************************************************************/

#ifndef MAME_MACHINE_SPG2XX_H
#define MAME_MACHINE_SPG2XX_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "spg2xx_audio.h"
#include "screen.h"

class spg2xx_device : public device_t, public device_mixer_interface
{
public:
	spg2xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	void set_pal(bool pal) { m_pal_flag = pal ? 1 : 0; }
	void set_rowscroll_offset(int offset) { m_rowscrolloffset = offset; }

	void map(address_map &map);

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }

	template <size_t Line> auto adc_in() { return m_adc_in[Line].bind(); }

	auto eeprom_w() { return m_eeprom_w.bind(); }
	auto eeprom_r() { return m_eeprom_r.bind(); }

	auto uart_tx() { return m_uart_tx.bind(); }

	auto chip_select() { return m_chip_sel.bind(); }

	void uart_rx(uint8_t data);

	void extint_w(int channel, bool state);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);

	required_device<spg2xx_audio_device> m_spg_audio;

protected:
	spg2xx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, const uint32_t sprite_limit)
		: spg2xx_device(mconfig, type, tag, owner, clock)
	{
		m_sprite_limit = sprite_limit;
	}

	virtual void device_add_mconfig(machine_config &config) override;

	enum
	{
		PAGE_ENABLE_MASK        = 0x0008,
		PAGE_WALLPAPER_MASK     = 0x0004,

		SPRITE_ENABLE_MASK      = 0x0001,
		SPRITE_COORD_TL_MASK    = 0x0002,

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

	virtual DECLARE_READ16_MEMBER(io_r);
	virtual DECLARE_WRITE16_MEMBER(io_w);

	DECLARE_WRITE_LINE_MEMBER(audioirq_w);
	DECLARE_READ16_MEMBER(space_r);

	void check_extint_irq(int channel);
	void check_irqs(const uint16_t changed);
	inline void check_video_irq();

	void spg2xx_map(address_map &map);

	static const device_timer_id TIMER_TMB1 = 0;
	static const device_timer_id TIMER_TMB2 = 1;
	static const device_timer_id TIMER_SCREENPOS = 2;
	static const device_timer_id TIMER_BEAT = 3;
	static const device_timer_id TIMER_UART_TX = 4;
	static const device_timer_id TIMER_UART_RX = 5;
	static const device_timer_id TIMER_4KHZ = 6;
	static const device_timer_id TIMER_SRC_AB = 7;
	static const device_timer_id TIMER_SRC_C = 8;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void update_porta_special_modes();
	void update_portb_special_modes();
	void do_gpio(uint32_t offset, bool write);
	uint16_t do_special_gpio(uint32_t index, uint16_t mask);

	void update_timer_b_rate();
	void update_timer_ab_src();
	void update_timer_c_src();
	void increment_timer_a();

	void uart_transmit_tick();
	void uart_receive_tick();

	void system_timer_tick();

	void do_i2c();
	void do_cpu_dma(uint32_t len);

	void do_sprite_dma(uint32_t len);

	enum blend_enable_t : bool
	{
		BlendOff = false,
		BlendOn = true
	};

	enum rowscroll_enable_t : bool
	{
		RowScrollOff = false,
		RowScrollOn = true
	};

	enum flipx_t : bool
	{
		FlipXOff = false,
		FlipXOn = true
	};

	void apply_saturation(const rectangle &cliprect);
	void apply_fade(const rectangle &cliprect);

	template<blend_enable_t Blend, rowscroll_enable_t RowScroll, flipx_t FlipX>
	void blit(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile);
	void blit_page(const rectangle &cliprect, uint32_t scanline, int depth, uint32_t bitmap_addr, uint16_t *regs);
	void blit_sprite(const rectangle &cliprect, uint32_t scanline, int depth, uint32_t base_addr);
	void blit_sprites(const rectangle &cliprect, uint32_t scanline, int depth);

	uint8_t mix_channel(uint8_t a, uint8_t b);

	uint32_t m_screenbuf[320 * 240];
	uint8_t m_rgb5_to_rgb8[32];
	uint32_t m_rgb555_to_rgb888[0x8000];

	bool m_hide_page0;
	bool m_hide_page1;
	bool m_hide_sprites;
	bool m_debug_sprites;
	bool m_debug_blit;
	bool m_debug_palette;
	uint8_t m_sprite_index_to_debug;


	uint16_t m_io_regs[0x200];
	uint8_t m_uart_rx_fifo[8];
	uint8_t m_uart_rx_fifo_start;
	uint8_t m_uart_rx_fifo_end;
	uint8_t m_uart_rx_fifo_count;
	bool m_uart_rx_available;
	bool m_uart_rx_irq;
	bool m_uart_tx_irq;

	bool m_extint[2];

	uint16_t m_video_regs[0x100];
	uint32_t m_sprite_limit;
	int m_rowscrolloffset; // auto racing in 'zone60' minigames needs this to be 15, the JAKKS games (Star Wars Revenge of the sith - Gunship Battle, Wheel of Fortune, Namco Ms. Pac-Man 5-in-1 Pole Position) need it to be 0, where does it come from?
	uint16_t m_pal_flag;

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_read16 m_adc_in[2];

	devcb_write8 m_eeprom_w;
	devcb_read8 m_eeprom_r;

	devcb_write8 m_uart_tx;

	devcb_write8 m_chip_sel;

	uint16_t m_timer_a_preload;
	uint16_t m_timer_b_preload;
	uint16_t m_timer_b_divisor;
	uint16_t m_timer_b_tick_rate;

	emu_timer *m_tmb1;
	emu_timer *m_tmb2;
	emu_timer *m_timer_src_ab;
	emu_timer *m_timer_src_c;
	emu_timer *m_screenpos_timer;

	emu_timer *m_4khz_timer;
	uint32_t m_2khz_divider;
	uint32_t m_1khz_divider;
	uint32_t m_4hz_divider;

	uint32_t m_uart_baud_rate;
	emu_timer *m_uart_tx_timer;
	emu_timer *m_uart_rx_timer;

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_spriteram;
};

class spg24x_device : public spg2xx_device
{
public:
	template <typename T, typename U>
	spg24x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg24x_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	spg24x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class spg28x_device : public spg2xx_device
{
public:
	template <typename T, typename U>
	spg28x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg28x_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	spg28x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_WRITE16_MEMBER(io_w) override;
};

DECLARE_DEVICE_TYPE(SPG24X, spg24x_device)
DECLARE_DEVICE_TYPE(SPG28X, spg28x_device)

#endif // MAME_MACHINE_SPG2XX_H
