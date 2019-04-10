// license:BSD-3-Clause
// copyright-holders:Philip Bennett, James Wallace, David Haywood
#ifndef MAME_INCLUDES_JPMSYS5_H
#define MAME_INCLUDES_JPMSYS5_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "sound/ym2413.h"
#include "sound/upd7759.h"
#include "video/tms34061.h"
#include "machine/nvram.h"
#include "video/awpvid.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/meters.h"
#include "emupal.h"

class jpmsys5_state : public driver_device
{
public:
	jpmsys5_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_acia6850(*this, "acia6850_%u", 0U),
		m_vfd(*this, "vfd"),
		m_upd7759(*this, "upd7759"),
		m_direct_port(*this, "DIRECT"),
		m_meters(*this, "meters"),
		m_lamps(*this, "lamp%u", 0U),
		m_sys5leds(*this, "sys5led%u", 0U)
	{ }

	void jpmsys5(machine_config &config);
	void jpmsys5_ym(machine_config &config);

	DECLARE_WRITE_LINE_MEMBER(ptm_irq);
	DECLARE_WRITE_LINE_MEMBER(u26_o1_callback);
	DECLARE_WRITE_LINE_MEMBER(pia_irq);
	DECLARE_WRITE_LINE_MEMBER(u29_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(u29_cb2_w);
	DECLARE_WRITE_LINE_MEMBER(a0_tx_w);
	DECLARE_WRITE_LINE_MEMBER(a1_tx_w);
	DECLARE_WRITE_LINE_MEMBER(a2_tx_w);

	DECLARE_READ8_MEMBER(u29_porta_r);
	DECLARE_WRITE8_MEMBER(u29_portb_w);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE16_MEMBER(jpm_upd7759_w);
	DECLARE_READ16_MEMBER(jpm_upd7759_r);

	required_device<cpu_device> m_maincpu;
	required_device_array<acia6850_device, 3> m_acia6850;
	optional_device<s16lf01_device> m_vfd;
	required_device<upd7759_device> m_upd7759;

	void jpm_sys5_common_map(address_map &map);

private:
	DECLARE_READ16_MEMBER(coins_r);
	DECLARE_WRITE16_MEMBER(coins_w);
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_WRITE16_MEMBER(mux_w);
	DECLARE_READ16_MEMBER(mux_r);

	DECLARE_READ16_MEMBER(mux_awp_r);
	DECLARE_READ16_MEMBER(coins_awp_r);
	void sys5_draw_lamps();

	void m68000_awp_map(address_map &map);
	void m68000_awp_map_saa(address_map &map);

	required_ioport m_direct_port;
	optional_device<meters_device> m_meters; //jpmsys5v doesn't use this
	output_finder<16 * 16> m_lamps;
	output_finder<16 * 8> m_sys5leds;

	int m_lamp_strobe;
	int m_mpxclk;
	int m_muxram[255];
	int m_chop;
	uint8_t m_a0_data_out;
	uint8_t m_a1_data_out;
	uint8_t m_a2_data_out;
};


class jpmsys5v_state : public jpmsys5_state
{
public:
	jpmsys5v_state(const machine_config &mconfig, device_type type, const char *tag) :
		jpmsys5_state(mconfig, type, tag),
		m_tms34061(*this, "tms34061"),
		m_palette(*this, "palette"),
		m_rombank(*this, "bank1"),
		m_touch_axes(*this, { "TOUCH_X", "TOUCH_Y" }),
		m_touch_timer(nullptr)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(touchscreen_press);

	void jpmsys5v(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE_LINE_MEMBER(generate_tms34061_interrupt);
	DECLARE_WRITE16_MEMBER(sys5_tms34061_w);
	DECLARE_READ16_MEMBER(sys5_tms34061_r);
	DECLARE_WRITE16_MEMBER(ramdac_w);
	DECLARE_WRITE16_MEMBER(rombank_w);
	uint32_t screen_update_jpmsys5v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(touch_cb);

	void m68000_map(address_map &map);

	required_device<tms34061_device> m_tms34061;
	required_device<palette_device> m_palette;
	required_memory_bank m_rombank;
	required_ioport_array<2> m_touch_axes;

	uint8_t m_palette_val[16][3];
	int m_pal_addr;
	int m_pal_idx;
	int m_touch_state;
	emu_timer *m_touch_timer;
	int m_touch_data_count;
	int m_touch_data[3];
	int m_touch_shift_cnt;
};

#endif // MAME_INCLUDES_JPMSYS5_H
