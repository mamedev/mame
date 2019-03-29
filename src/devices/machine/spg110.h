// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_SPG110_H
#define MAME_MACHINE_SPG110_H

#pragma once

//#include "spg2xx.h"
#include "cpu/unsp/unsp.h"
#include "emupal.h"
#include "spg2xx_io.h"
#include "spg110_video.h"

class spg110_device : public device_t

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

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }

	template <size_t Line> auto adc_in() { return m_adc_in[Line].bind(); }

	auto chip_select() { return m_chip_sel.bind(); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return m_spg_video->screen_update(screen,bitmap,cliprect); }
	DECLARE_WRITE_LINE_MEMBER(vblank) { m_spg_video->vblank(state); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void device_add_mconfig(machine_config &config) override;

private:

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;

	required_device<spg2xx_io_device> m_spg_io;
	required_device<spg110_video_device> m_spg_video;

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

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_read16 m_adc_in[2];

	devcb_write8 m_chip_sel;

	DECLARE_READ16_MEMBER(porta_r) { return m_porta_in(); }
	DECLARE_READ16_MEMBER(portb_r) { return m_portb_in(); }
	DECLARE_READ16_MEMBER(portc_r) { return m_portc_in(); }
	DECLARE_WRITE16_MEMBER(porta_w) { m_porta_out(offset, data, mem_mask); }
	DECLARE_WRITE16_MEMBER(portb_w) { m_portb_out(offset, data, mem_mask); }
	DECLARE_WRITE16_MEMBER(portc_w) { m_portc_out(offset, data, mem_mask); }
	template <size_t Line> DECLARE_READ16_MEMBER(adc_r) { return m_adc_in[Line](); }
	DECLARE_WRITE8_MEMBER(cs_w) { m_chip_sel(offset, data, mem_mask); }
	DECLARE_READ16_MEMBER(get_pal_r) { return 0; /*m_pal_flag;*/ }
	void configure_spg_io(spg2xx_io_device* io);

	DECLARE_WRITE_LINE_MEMBER(videoirq_w);
};

DECLARE_DEVICE_TYPE(SPG110, spg110_device)

#endif // MAME_MACHINE_SPG110_H
