// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega vector hardware

*************************************************************************/
#ifndef MAME_SEGA_SEGAG80V_H
#define MAME_SEGA_SEGAG80V_H

#pragma once

#include "segag80.h"
#include "segaspeech.h"
#include "segausb.h"

#include "cpu/z80/z80.h"
#include "segag80_m.h"
#include "sound/ay8910.h"
#include "sound/samples.h"
#include "sound/tms5110.h"
#include "video/vector.h"

#include "screen.h"

#define CPU_CLOCK           8000000     /* not used when video boards are connected */
#define VIDEO_CLOCK         15468480

class segag80v_state : public driver_device
{
public:
	segag80v_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mainrom(*this, "maincpu"),
		m_mainram(*this, "mainram"),
		m_vectorram(*this, "vectorram"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_speech(*this, "speech"),
		m_usb(*this, "usbsnd"),
		m_g80_audio(*this, "g80sound"),
		m_aysnd(*this, "aysnd"),
		m_vector(*this, "vector"),
		m_screen(*this, "screen"),
		m_d7d6(*this, "D7D6"),
		m_d5d4(*this, "D5D4"),
		m_d3d2(*this, "D3D2"),
		m_d1d0(*this, "D1D0"),
		m_fc(*this, "FC"),
		m_coins(*this, "COINS"),
		m_spinner(*this, "SPINNER"),
		m_mult_data{0,0},
		m_mult_result(0),
		m_spinner_select(0),
		m_spinner_sign(0),
		m_spinner_count(0),
		m_coin_ff_state(0),
		m_coin_last_state(0),
		m_edgint_ff_state(0),
		m_scrambled_write_pc(0),
		m_decrypt(nullptr),
		m_min_x(0),
		m_min_y(0),
		m_draw_end_time(attotime::zero)
	{ }

	void g80v_base(machine_config &config);
	void tacscan(machine_config &config);
	void elim2(machine_config &config);
	void startrek(machine_config &config);
	void zektor(machine_config &config);
	void spacfury(machine_config &config);
	void spacfurybl(machine_config &config);

	void init_waitstates();
	void init_zektor();
	void init_startrek();
	void init_elim4();
	void init_elim2();
	void init_tacscan();
	void init_spacfury();
	void init_spacfurybl();

	int elim4_joint_coin_r();
	int draw_r();
	void service_switch_w(int state);
	void irq_ack_w(int state);

	template<int Index>
	void coin_w(int state)
	{
		const u8 mask = 1 << Index;

		if (state == 0 && (m_coin_last_state & mask) != 0)
			m_coin_ff_state |= mask;
		else
			m_coin_ff_state &= ~mask;

		if (state)
			m_coin_last_state |= mask;
		else
			m_coin_last_state &= ~mask;

		update_int();
	}

private:
	required_memory_region m_mainrom;
	required_shared_ptr<u8> m_mainram;
	required_shared_ptr<u8> m_vectorram;

	required_device<z80_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<sega_speech_device> m_speech;
	optional_device<usb_sound_device> m_usb;
	optional_device<segag80_audio_device_base> m_g80_audio;
	optional_device<ay8912_device> m_aysnd;
	required_device<vector_device> m_vector;
	required_device<screen_device> m_screen;

	required_ioport m_d7d6;
	required_ioport m_d5d4;
	required_ioport m_d3d2;
	required_ioport m_d1d0;
	required_ioport m_fc;
	optional_ioport m_coins;
	optional_ioport m_spinner;

	u8 m_mult_data[2];
	u16 m_mult_result;
	u8 m_spinner_select;
	u8 m_spinner_sign;
	u8 m_spinner_count;
	u8 m_coin_ff_state;
	u8 m_coin_last_state;
	u8 m_edgint_ff_state;
	offs_t m_scrambled_write_pc;
	segag80_decrypt_func m_decrypt;
	int m_min_x;
	int m_min_y;
	attotime m_draw_end_time;

	u8 opcode_r(offs_t offset);
	u8 mainrom_r(offs_t offset);
	void mainram_w(offs_t offset, u8 data);
	void vectorram_w(offs_t offset, u8 data);
	u8 mangled_ports_r(offs_t offset);
	void spinner_select_w(u8 data);
	u8 spinner_input_r();
	u8 elim4_input_r();
	void multiply_w(offs_t offset, u8 data);
	u8 multiply_r();
	void coin_count_w(u8 data);
	void unknown_w(u8 data);
	void update_int();
	void vblank_callback(screen_device &screen, bool state);

	void usb_ram_w(offs_t offset, u8 data);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_segag80v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	inline bool adjust_xy(int rawx, int rawy, int &outx, int &outy);
	void sega_generate_vector_list();
	offs_t decrypt_offset(offs_t offset);
	inline u8 demangle(u8 d7d6, u8 d5d4, u8 d3d2, u8 d1d0);

	void main_map(address_map &map) ATTR_COLD;
	void opcodes_map(address_map &map) ATTR_COLD;
	void main_portmap(address_map &map) ATTR_COLD;
	void spacfurybl_speech_prg_map(address_map &map) ATTR_COLD;
	void spacfurybl_speech_io_map(address_map &map) ATTR_COLD;
};

#endif // MAME_SEGA_SEGAG80V_H
