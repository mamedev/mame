// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_SPG110_H
#define MAME_MACHINE_SPG110_H

#pragma once

//#include "spg2xx.h"
#include "cpu/unsp/unsp.h"
#include "emupal.h"
#include "spg2xx_io.h"


class spg110_device : public device_t, public device_memory_interface

{
public:
	spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T, typename U>
	spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&screen_tag)
		: spg110_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	void map(address_map &map);
	void map_video(address_map &map);

	double hue2rgb(double p, double q, double t);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank);

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }

	template <size_t Line> auto adc_in() { return m_adc_in[Line].bind(); }

	auto chip_select() { return m_chip_sel.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

	virtual space_config_vector memory_space_config() const override;

	address_space_config        m_space_config;

private:
	enum
	{
		PAGE_ENABLE_MASK        = 0x0008,
		PAGE_WALLPAPER_MASK     = 0x0004,

		PAGE_DEPTH_FLAG_MASK    = 0x3000,
		PAGE_DEPTH_FLAG_SHIFT   = 12,
		PAGE_TILE_HEIGHT_MASK   = 0x00c0,
		PAGE_TILE_HEIGHT_SHIFT  = 6,
		PAGE_TILE_WIDTH_MASK    = 0x0030,
		PAGE_TILE_WIDTH_SHIFT   = 4,

		TILE_X_FLIP             = 0x0004,
		TILE_Y_FLIP             = 0x0008
	};

	enum flipx_t : bool
	{
		FlipXOff = false,
		FlipXOn = true
	};

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_palram;
	required_device<spg2xx_io_device> m_spg_io;

	//TIMER_CALLBACK_MEMBER(test_timer);
	//emu_timer *m_test_timer;


	DECLARE_WRITE16_MEMBER(spg110_201c_w);
	DECLARE_WRITE16_MEMBER(spg110_2020_w);

	DECLARE_WRITE16_MEMBER(spg110_2028_w);
	DECLARE_WRITE16_MEMBER(spg110_2029_w);

	DECLARE_READ16_MEMBER(spg110_2028_r);
	DECLARE_READ16_MEMBER(spg110_2029_r);

	DECLARE_WRITE16_MEMBER(spg110_2031_w);
	DECLARE_WRITE16_MEMBER(spg110_2032_w);
	DECLARE_WRITE16_MEMBER(spg110_2033_w);
	DECLARE_WRITE16_MEMBER(spg110_2034_w);
	DECLARE_WRITE16_MEMBER(spg110_2035_w);
	DECLARE_WRITE16_MEMBER(spg110_2036_w);
	DECLARE_WRITE16_MEMBER(spg110_2037_w);
	DECLARE_WRITE16_MEMBER(spg110_2039_w);

	DECLARE_WRITE16_MEMBER(spg110_203c_w);
	DECLARE_WRITE16_MEMBER(spg110_203d_w);

	DECLARE_WRITE16_MEMBER(spg110_2042_w);

	DECLARE_WRITE16_MEMBER(spg110_2045_w);

	DECLARE_WRITE16_MEMBER(spg110_2050_w);
	DECLARE_WRITE16_MEMBER(spg110_2051_w);
	DECLARE_WRITE16_MEMBER(spg110_2052_w);
	DECLARE_WRITE16_MEMBER(spg110_2053_w);
	DECLARE_WRITE16_MEMBER(spg110_2054_w);
	DECLARE_WRITE16_MEMBER(spg110_2055_w);
	DECLARE_WRITE16_MEMBER(spg110_2056_w);
	DECLARE_WRITE16_MEMBER(spg110_2057_w);
	DECLARE_WRITE16_MEMBER(spg110_2058_w);
	DECLARE_WRITE16_MEMBER(spg110_2059_w);
	DECLARE_WRITE16_MEMBER(spg110_205a_w);
	DECLARE_WRITE16_MEMBER(spg110_205b_w);
	DECLARE_WRITE16_MEMBER(spg110_205c_w);
	DECLARE_WRITE16_MEMBER(spg110_205d_w);
	DECLARE_WRITE16_MEMBER(spg110_205e_w);
	DECLARE_WRITE16_MEMBER(spg110_205f_w);


	DECLARE_READ16_MEMBER(spg110_2037_r);
	DECLARE_READ16_MEMBER(spg110_2042_r);

	DECLARE_WRITE16_MEMBER(dma_dst_w);
	DECLARE_WRITE16_MEMBER(dma_unk_2061_w);
	DECLARE_WRITE16_MEMBER(dma_len_trigger_w);
	DECLARE_WRITE16_MEMBER(spg110_2063_w);
	DECLARE_WRITE16_MEMBER(dma_dst_step_w);
	DECLARE_WRITE16_MEMBER(dma_src_w);
	DECLARE_WRITE16_MEMBER(dma_unk_2067_w);
	DECLARE_WRITE16_MEMBER(dma_src_step_w);

	DECLARE_READ16_MEMBER(dma_len_status_r);
	DECLARE_READ16_MEMBER(spg110_2063_r);

	DECLARE_WRITE16_MEMBER(spg110_3100_w);

	DECLARE_WRITE16_MEMBER(spg110_3101_w);
	DECLARE_WRITE16_MEMBER(spg110_3102_w);
	DECLARE_WRITE16_MEMBER(spg110_3104_w);
	DECLARE_WRITE16_MEMBER(spg110_3105_w);
	DECLARE_WRITE16_MEMBER(spg110_3106_w);
	DECLARE_WRITE16_MEMBER(spg110_3107_w);
	DECLARE_WRITE16_MEMBER(spg110_3108_w);
	DECLARE_WRITE16_MEMBER(spg110_3109_w);

	DECLARE_WRITE16_MEMBER(spg110_310b_w);
	DECLARE_WRITE16_MEMBER(spg110_310c_w);
	DECLARE_WRITE16_MEMBER(spg110_310d_w);

	DECLARE_READ16_MEMBER(spg110_310f_r);

	DECLARE_READ16_MEMBER(tmap0_regs_r);
	DECLARE_READ16_MEMBER(tmap1_regs_r);
	DECLARE_WRITE16_MEMBER(tmap0_regs_w);
	DECLARE_WRITE16_MEMBER(tmap1_regs_w);

	uint16_t tmap0_regs[0x6];
	uint16_t tmap1_regs[0x6];

	uint16_t m_dma_src_step;
	uint16_t m_dma_dst_step;
	uint16_t m_dma_unk_2061;
	uint16_t m_dma_unk_2067;

	uint16_t m_dma_dst;
	uint16_t m_dma_src;

	uint16_t m_bg_scrollx;
	uint16_t m_bg_scrolly;
	uint16_t m_2036_scroll;

	void tilemap_write_regs(int which, uint16_t* regs, int regno, uint16_t data);

	template<flipx_t FlipX>
	void blit(const rectangle &cliprect, uint32_t line, uint32_t xoff, uint32_t yoff, uint32_t attr, uint32_t ctrl, uint32_t bitmap_addr, uint16_t tile);
	void blit_page(const rectangle &cliprect, uint32_t scanline, int depth, uint32_t bitmap_addr, uint16_t *regs);
	uint32_t m_screenbuf[320 * 240];

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_read16 m_adc_in[2];

	devcb_write8 m_chip_sel;

	DECLARE_READ16_MEMBER(porta_r) { return m_porta_in(); };
	DECLARE_READ16_MEMBER(portb_r) { return m_portb_in(); };
	DECLARE_READ16_MEMBER(portc_r) { return m_portc_in(); };
	DECLARE_WRITE16_MEMBER(porta_w) { m_porta_out(offset, data, mem_mask); };
	DECLARE_WRITE16_MEMBER(portb_w) { m_portb_out(offset, data, mem_mask); };
	DECLARE_WRITE16_MEMBER(portc_w) { m_portc_out(offset, data, mem_mask); };
	template <size_t Line> DECLARE_READ16_MEMBER(adc_r) { return m_adc_in[Line](); };
	DECLARE_WRITE8_MEMBER(cs_w) { m_chip_sel(offset, data, mem_mask); };
	DECLARE_READ16_MEMBER(get_pal_r) { return 0; /*m_pal_flag;*/ };
	void configure_spg_io(spg2xx_io_device* io);

};

DECLARE_DEVICE_TYPE(SPG110, spg110_device)

#endif // MAME_MACHINE_SPG110_H
