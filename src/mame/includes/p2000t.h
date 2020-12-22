// license:BSD-3-Clause
// copyright-holders:Paul Daniels
/*****************************************************************************
 *
 * includes/p2000t.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_P2000T_H
#define MAME_INCLUDES_P2000T_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/spkrdev.h"
#include "video/saa5050.h"
#include "machine/p2000t_mdcr.h"
#include "machine/ram.h"
#include "machine/z80pio.h"
#include "machine/z80daisy.h"
#include "emupal.h"
#include "screen.h"

class p2000t_state : public driver_device
{
public:
	p2000t_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_saa5050(*this, "saa5050")
		, m_screen(*this, "screen")
		, m_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_speaker(*this, "speaker")
		, m_mdcr(*this, "mdcr")
		, m_ram(*this, RAM_TAG)
		, m_bank(*this, "bank")
		, m_keyboard(*this, "KEY.%u", 0)
	{
	}

	void p2000t(machine_config &config);

protected:
	uint8_t p2000t_port_000f_r(offs_t offset);
	uint8_t p2000t_port_202f_r();

	void p2000t_port_00_w(uint8_t data);
	void p2000t_port_101f_w(uint8_t data);
	void p2000t_port_303f_w(uint8_t data);
	void p2000t_port_505f_w(uint8_t data);
	void p2000t_port_707f_w(uint8_t data);
	uint8_t p2000t_port_707f_r();
	void p2000t_port_888b_w(uint8_t data);
	void p2000t_port_8c90_w(uint8_t data);
	void p2000t_port_9494_w(uint8_t data);

	uint8_t videoram_r(offs_t offset);
	virtual void machine_start() override;

	INTERRUPT_GEN_MEMBER(p2000_interrupt);

	void p2000t_mem(address_map &map);
	void p2000t_io(address_map &map);
    bool in_80char_mode() { return BIT(m_port_707f, 0); }

	optional_device<saa5050_device> m_saa5050; // Only available on P2000T not on M-model
    required_device<screen_device> m_screen;

	required_shared_ptr<uint8_t> m_videoram;

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_device<mdcr_device> m_mdcr;
	required_device<ram_device> m_ram;
	
	required_memory_bank m_bank;
	

private:
	required_ioport_array<10> m_keyboard;
	
	uint8_t m_port_101f;
	uint8_t m_port_202f;
	uint8_t m_port_303f;
	uint8_t m_port_707f;
};

class p2000h_state : public p2000t_state
{
public:
	p2000h_state(const machine_config &mconfig, device_type type, const char *tag)
		: p2000t_state(mconfig, type, tag)
		, m_hirescpu(*this, "hirescpu") 
		, m_hiresram(*this, "hiresram")
		, m_mainpio(*this, "mainpio")
		, m_hirespio(*this, "hirespio")
		
	{
	}

	void p2000h(machine_config &config);

protected:
	uint32_t screen_update_p2000h(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
    void screen_update_p2000h_draw_pixel(bitmap_rgb32 &bitmap, int xpos, int ypos, uint32_t color, int xlen, int ylen );

	virtual void machine_start() override;
	void p2000h_mem(address_map &map);
	void p2000t_io(address_map &map);
	void p2000h_io(address_map &map);
	u8 memory_read(offs_t offset);
	void memory_write(offs_t offset, u8 data);

    /* P2000T CPU side */
    void p2000t_port_2c_w(uint8_t data);
	uint8_t mainpio_pa_r_cb();
	void mainpio_pa_w_cb(uint8_t data);
	uint8_t mainpio_pb_r_cb();
	void mainpio_pb_w_cb(uint8_t data);

	/* hires CPU side */
	void p2000h_port_808f_w(uint8_t data);
	void p2000h_port_909f_w(uint8_t data);
	void p2000h_port_a0af_w(uint8_t data);
	void p2000h_port_b0bf_w(uint8_t data);
	void p2000h_port_c0cf_w(uint8_t data);
	void p2000h_port_d0df_w(uint8_t data);
	void p2000h_port_e0ef_w(uint8_t data);

	uint8_t hirespio_pa_r_cb();
	void hirespio_pa_w_cb(uint8_t data);
	uint8_t hirespio_pb_r_cb();
	void hirespio_pb_w_cb(uint8_t data);

	required_device<z80_device> m_hirescpu;
	required_device<ram_device> m_hiresram;
	required_device<z80pio_device> m_mainpio;
	required_device<z80pio_device> m_hirespio;

private:
	/* Hires implementation */
	uint8_t m_channel_a_data;
	uint8_t m_channel_b_data;
	
  	void hirespio_emulate_sync();
	
	uint8_t m_hires_image_mode;
	uint8_t m_hires_image_select;
	uint8_t m_hires_scroll_reg;
 
    bool m_hiresmem_bank0_ROM = true;
	u8 *m_hiresrom = NULL;

	static const size_t LUT_TABLE_SIZE = 16;
	uint8_t m_hires_LutRed[LUT_TABLE_SIZE];
	uint8_t m_hires_LutRedCnt = 0;
	uint8_t m_hires_LutBlue[LUT_TABLE_SIZE];
	uint8_t m_hires_LutBlueCnt = 0;
	uint8_t m_hires_LutGreen[LUT_TABLE_SIZE];
	uint8_t m_hires_LutGreenCnt = 0;
};


class p2000m_state : public p2000t_state
{
public:
	p2000m_state(const machine_config &mconfig, device_type type, const char *tag)
		: p2000t_state(mconfig, type, tag)
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")

	{
	}

	void p2000m(machine_config &config);

protected:
	virtual void video_start() override;
	void p2000m_palette(palette_device &palette) const;
	uint32_t screen_update_p2000m(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	
	void p2000m_mem(address_map &map);

private:
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int8_t m_frame_count;
};


#endif // MAME_INCLUDES_P2000T_H
