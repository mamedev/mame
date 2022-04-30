// license:BSD-3-Clause
// copyright-holders:Philip Bennett, James Wallace, David Haywood
#ifndef MAME_INCLUDES_JPMSYS5_H
#define MAME_INCLUDES_JPMSYS5_H

#pragma once

#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "sound/upd7759.h"
#include "sound/ymopl.h"
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
		m_dsw(*this, "DSW"),
		m_dsw2(*this, "DSW2"),
		m_rotary(*this, "ROTARY"),
		m_strobe0(*this, "STROBE0"),
		m_strobe1(*this, "STROBE1"),
		m_strobe2(*this, "STROBE2"),
		m_strobe3(*this, "STROBE3"),
		m_strobe4(*this, "STROBE4"),
		m_unknown_port(*this, "UNKNOWN_PORT"),
		m_direct_port(*this, "DIRECT"),
		m_meters(*this, "meters"),
		m_lamps(*this, "lamp%u", 0U),
		m_sys5leds(*this, "digit%u", 0U),
		m_reellamp_out(*this, "reellamp%u", 0U),
		m_reel(*this, "reel%u", 0U)
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

	uint8_t u29_porta_r();
	void u29_portb_w(uint8_t data);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void jpmsys5_common(machine_config &config);
	void ymsound(machine_config &config);
	void saasound(machine_config &config);
	void reels(machine_config &config);

	void jpm_upd7759_w(offs_t offset, uint16_t data);
	uint16_t jpm_upd7759_r();

	required_device<cpu_device> m_maincpu;
	required_device_array<acia6850_device, 3> m_acia6850;
	optional_device<s16lf01_device> m_vfd;
	required_device<upd7759_device> m_upd7759;

	optional_ioport m_dsw;
	optional_ioport m_dsw2;
	optional_ioport m_rotary;
	optional_ioport m_strobe0;
	optional_ioport m_strobe1;
	optional_ioport m_strobe2;
	optional_ioport m_strobe3;
	optional_ioport m_strobe4;
	optional_ioport m_unknown_port;

	void jpm_sys5_common_map(address_map &map);

protected:
	void m68000_ym_map(address_map &map);

private:
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(reel_optic_cb) { if (state) m_optic_pattern |= (1 << ((7-N)^3)); else m_optic_pattern &= ~(1 << ((7-N)^3)); }

	uint16_t unknown_port_r(offs_t offset, uint16_t mem_mask = ~0);

	uint16_t coins_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t reel_optos_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t unk_48000_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t unk_48002_r(offs_t offset, uint16_t mem_mask = ~0);
	uint16_t unk_48006_r(offs_t offset, uint16_t mem_mask = ~0);

	void reel_0123_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void reel_4567_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void unk_48000_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void unk_48006_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t unk_r(offs_t offset, uint16_t mem_mask = ~0);

	uint16_t reellamps_0123_r(offs_t offset, uint16_t mem_mask = ~0);
	void reellamps_0123_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t reellamps_4567_r(offs_t offset, uint16_t mem_mask = ~0);
	void reellamps_4567_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t m_reellamps_0123 = 0;
	uint16_t m_reellamps_5678 = 0;

	void mux_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mux_r(offs_t offset, uint16_t mem_mask = ~0);

	void sys5_draw_lamps();


	void m68000_awp_map(address_map &map);
	void m68000_awp_map_saa(address_map &map);

	required_ioport m_direct_port;
	optional_device<meters_device> m_meters; //jpmsys5v doesn't use this
	output_finder<16 * 16> m_lamps;
	output_finder<16> m_sys5leds;
	output_finder<32> m_reellamp_out;
	optional_device_array<stepper_device, 8> m_reel;

	int m_lamp_strobe = 0;
	int m_mpxclk = 0;
	uint16_t m_muxram[0x100]{};
	uint16_t m_optic_pattern = 0;
	int m_chop = 0;
	uint8_t m_a0_data_out = 0;
	uint8_t m_a1_data_out = 0;
	uint8_t m_a2_data_out = 0;
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

	void tmsvideo(machine_config &config);

	DECLARE_WRITE_LINE_MEMBER(generate_tms34061_interrupt);
	void sys5_tms34061_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t sys5_tms34061_r(offs_t offset, uint16_t mem_mask = ~0);
	void ramdac_w(offs_t offset, uint16_t data);
	void rombank_w(uint16_t data);
	uint32_t screen_update_jpmsys5v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(touch_cb);

	void m68000_map(address_map &map);

	required_device<tms34061_device> m_tms34061;
	required_device<palette_device> m_palette;
	required_memory_bank m_rombank;
	required_ioport_array<2> m_touch_axes;

	uint8_t m_palette_val[16][3]{};
	int m_pal_addr = 0;
	int m_pal_idx = 0;
	int m_touch_state = 0;
	emu_timer *m_touch_timer = nullptr;
	int m_touch_data_count = 0;
	int m_touch_data[3]{};
	int m_touch_shift_cnt = 0;
};

#endif // MAME_INCLUDES_JPMSYS5_H
