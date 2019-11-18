// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Vas Crabb
/*****************************************************************************
 *
 * includes/osborne1.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_OSBORNE1_H
#define MAME_INCLUDES_OSBORNE1_H

#pragma once

#include "bus/ieee488/ieee488.h"

#include "cpu/z80/z80.h"

#include "imagedev/floppy.h"

#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"

#include "sound/spkrdev.h"

#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


class osborne1_state : public driver_device
{
public:
	osborne1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this, RAM_TAG),
		m_screen(*this, "screen"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_speaker(*this, "speaker"),
		m_pia0(*this, "pia_0"),
		m_pia1(*this, "pia_1"),
		m_acia(*this, "acia"),
		m_fdc(*this, "mb8877"),
		m_ieee(*this, IEEE488_TAG),
		m_floppy0(*this, "mb8877:0"),
		m_floppy1(*this, "mb8877:1"),
		m_keyb_row(*this, "ROW%u", 0),
		m_btn_reset(*this, "RESET"),
		m_cnf(*this, "CNF"),
		m_region_maincpu(*this, "maincpu"),
		m_bank_0xxx(*this, "bank_0xxx"),
		m_bank_1xxx(*this, "bank_1xxx"),
		m_bank_fxxx(*this, "bank_fxxx"),
		m_p_chargen(*this, "chargen"),
		m_video_timer(nullptr),
		m_tilemap(nullptr),
		m_acia_rxc_txc_timer(nullptr)
	{
	}

	void osborne1(machine_config &config);

	void init_osborne1();

	DECLARE_INPUT_CHANGED_MEMBER(reset_key);

protected:
	virtual void machine_reset() override;
	virtual void video_start() override;

	void osborne1nv_io(address_map &map);

	required_device<ram_device>             m_ram;
	required_device<screen_device>          m_screen;
	required_device<z80_device>             m_maincpu;

private:
	DECLARE_WRITE8_MEMBER(bank_0xxx_w);
	DECLARE_WRITE8_MEMBER(bank_1xxx_w);
	DECLARE_READ8_MEMBER(bank_2xxx_3xxx_r);
	DECLARE_WRITE8_MEMBER(bank_2xxx_3xxx_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_READ8_MEMBER(opcode_r);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE_LINE_MEMBER(irqack_w);

	DECLARE_READ8_MEMBER(ieee_pia_pb_r);
	DECLARE_WRITE8_MEMBER(ieee_pia_pb_w);
	DECLARE_WRITE_LINE_MEMBER(ieee_pia_irq_a_func);

	DECLARE_WRITE8_MEMBER(video_pia_port_a_w);
	DECLARE_WRITE8_MEMBER(video_pia_port_b_w);
	DECLARE_WRITE_LINE_MEMBER(video_pia_out_cb2_dummy);
	DECLARE_WRITE_LINE_MEMBER(video_pia_irq_a_func);

	DECLARE_WRITE_LINE_MEMBER(serial_acia_irq_func);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<gfxdecode_device>       m_gfxdecode;
	required_device<speaker_sound_device>   m_speaker;
	required_device<pia6821_device>         m_pia0;
	required_device<pia6821_device>         m_pia1;
	required_device<acia6850_device>        m_acia;
	required_device<mb8877_device>          m_fdc;
	required_device<ieee488_device>         m_ieee;
	required_device<floppy_connector>       m_floppy0;
	required_device<floppy_connector>       m_floppy1;

	void osborne1_mem(address_map &map);
	void osborne1_op(address_map &map);
	void osborne1_io(address_map &map);

	TIMER_CALLBACK_MEMBER(video_callback);
	TIMER_CALLBACK_MEMBER(acia_rxc_txc_callback);

	TILE_GET_INFO_MEMBER(get_tile_info);

	bool set_rom_mode(u8 value);
	bool set_bit_9(u8 value);
	void update_irq();
	void update_acia_rxc_txc();

	// user inputs
	required_ioport_array<8>    m_keyb_row;
	required_ioport             m_btn_reset;

	// fake inputs for hardware configuration and things that need rewiring
	required_ioport             m_cnf;

	// pieces of memory
	required_memory_region  m_region_maincpu;
	required_memory_bank    m_bank_0xxx;
	required_memory_bank    m_bank_1xxx;
	required_memory_bank    m_bank_fxxx;
	required_region_ptr<u8> m_p_chargen;

	// configuration (reloaded on reset)
	u8              m_screen_pac;
	u8              m_acia_rxc_txc_div;
	u8              m_acia_rxc_txc_p_low;
	u8              m_acia_rxc_txc_p_high;

	// bank switch control bits
	u8              m_ub4a_q;
	u8              m_ub6a_q;
	u8              m_rom_mode;
	u8              m_bit_9;

	// onboard video state
	u8              m_scroll_x;
	u8              m_scroll_y;
	u8              m_beep_state;
	emu_timer       *m_video_timer;
	bitmap_ind16    m_bitmap;
	tilemap_t       *m_tilemap;

	// SCREEN-PAC registers
	u8              m_resolution;
	u8              m_hc_left;

	// serial state
	u8              m_acia_irq_state;
	u8              m_acia_rxc_txc_state;
	emu_timer       *m_acia_rxc_txc_timer;
};


class osborne1nv_state : public osborne1_state
{
public:
	osborne1nv_state(const machine_config &mconfig, device_type type, const char *tag) :
		osborne1_state(mconfig, type, tag),
		m_palette(*this, "palette"),
		m_p_nuevo(*this, "nuevo")
	{
	}

	void osborne1nv(machine_config &config);

private:
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr_changed);

	required_device<palette_device> m_palette;
	required_region_ptr<u8>         m_p_nuevo;
};

#endif // MAME_INCLUDES_OSBORNE1_H
