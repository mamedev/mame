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
#include "spg2xx_audio.h"

class spg110_device : public unsp_device, public device_mixer_interface

{
public:
	spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	spg110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag)
		: spg110_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
	}

	void set_video_irq_spidman(bool is_spiderman) { m_is_spiderman = is_spiderman; }

	auto porta_out() { return m_porta_out.bind(); }
	auto portb_out() { return m_portb_out.bind(); }
	auto portc_out() { return m_portc_out.bind(); }
	auto porta_in() { return m_porta_in.bind(); }
	auto portb_in() { return m_portb_in.bind(); }
	auto portc_in() { return m_portc_in.bind(); }

	template <size_t Line> auto adc_in() { return m_adc_in[Line].bind(); }

	auto chip_select() { return m_chip_sel.bind(); }

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) { return m_spg_video->screen_update(screen,bitmap,cliprect); }
	void vblank(int state) { m_spg_video->vblank(state); }

protected:
	spg110_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal);

	void internal_map(address_map &map) ATTR_COLD;

	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<screen_device> m_screen;

	required_device<spg2xx_io_device> m_spg_io;
	required_device<spg110_video_device> m_spg_video;
	required_device<spg110_audio_device> m_spg_audio;

	uint16_t space_r(offs_t offset);
	void audioirq_w(int state);

	devcb_write16 m_porta_out;
	devcb_write16 m_portb_out;
	devcb_write16 m_portc_out;
	devcb_read16 m_porta_in;
	devcb_read16 m_portb_in;
	devcb_read16 m_portc_in;

	devcb_read16::array<4> m_adc_in;

	devcb_write8 m_chip_sel;

	uint16_t porta_r() { return m_porta_in(); }
	uint16_t portb_r() { return m_portb_in(); }
	uint16_t portc_r() { return m_portc_in(); }
	void porta_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_porta_out(offset, data, mem_mask); }
	void portb_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_portb_out(offset, data, mem_mask); }
	void portc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0) { m_portc_out(offset, data, mem_mask); }

	void ffreq1_w(int state);
	void ffreq2_w(int state);

	template <size_t Line> uint16_t adc_r() { return m_adc_in[Line](); }
	void cs_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0) { m_chip_sel(offset, data, mem_mask); }
	uint16_t get_pal_r() { return 0; /*m_pal_flag;*/ }
	void configure_spg_io(spg2xx_io_device* io);

	void videoirq_w(int state);
	bool m_is_spiderman;
};

DECLARE_DEVICE_TYPE(SPG110, spg110_device)

#endif // MAME_MACHINE_SPG110_H
