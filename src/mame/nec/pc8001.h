// license:BSD-3-Clause
// copyright-holders:Curt Coder, Angelo Salese
#ifndef MAME_NEC_PC8001_H
#define MAME_NEC_PC8001_H

#pragma once

#include "bus/centronics/ctronics.h"
#include "bus/nec_fdd/pc80s31k.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "machine/bankdev.h"
#include "machine/buffer.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/upd1990a.h"
#include "sound/beep.h"
#include "sound/ymopn.h"
#include "video/upd3301.h"

#include "pc88_alu.h"

#include "emupal.h"
#include "screen.h"

#define Z80_TAG         "maincpu"
#define N80SR_ROM_TAG   "n80sr_rom"
#define I8251_TAG       "i8251"
#define I8257_TAG       "i8257"
#define UPD1990A_TAG    "upd1990a"
#define UPD3301_TAG     "upd3301"
#define CGROM_TAG       "cgrom"
#define CENTRONICS_TAG  "centronics"

class pc8001_base_state : public driver_device
{
public:
	pc8001_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_picu(*this, "picu")
		, m_rtc(*this, UPD1990A_TAG)
		, m_centronics(*this, CENTRONICS_TAG)
		, m_cent_data_out(*this, "cent_data_out")
		, m_crtc(*this, UPD3301_TAG)
		, m_crtc_palette(*this, "crtc_palette")
		, m_dma(*this, I8257_TAG)
		, m_cassette(*this, "cassette")
		, m_cgrom(*this, CGROM_TAG)
	{}

	// feature::PRINTER more to do with lacking specific PC-88 options
	// cfr. -flop1 multipla:flop2 setprt.com
	static constexpr feature_type unemulated_features() { return feature::TAPE | feature::PRINTER; }


protected:
	required_device<cpu_device> m_maincpu;
	required_device<i8214_device> m_picu;
	required_device<upd1990a_device> m_rtc;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<upd3301_device> m_crtc;
	required_device<palette_device> m_crtc_palette;
	required_device<i8257_device> m_dma;
	required_device<cassette_image_device> m_cassette;
	required_memory_region m_cgrom;

	void port10_w(uint8_t data);

	void port30_w(u8 data);
	virtual void machine_start() override ATTR_COLD;
	void set_screen_frequency(bool is_24KHz) { m_screen_is_24KHz = is_24KHz; }
	bool get_screen_frequency() { return m_screen_is_24KHz; }

	void crtc_reverse_w(int state);
	UPD3301_DRAW_CHARACTER_MEMBER( draw_text );
	virtual UPD3301_FETCH_ATTRIBUTE( attr_fetch );
	void hrq_w(int state);
	virtual uint8_t dma_mem_r(offs_t offset);

	void write_centronics_busy(int state);
	void write_centronics_ack(int state);

	int m_centronics_busy = 0;
	int m_centronics_ack = 0;
private:
	bool m_screen_reverse = false;
	bool m_screen_is_24KHz = false;

	/* video state */
	int m_width80 = 0;
	int m_color = 0;
	u8 m_attr_color = 0xe8, m_attr_decoration = 0;

	// irq section
protected:
	void irq_level_w(uint8_t data);
	void irq_mask_w(uint8_t data);

	void int4_irq_w(int state);

	void rxrdy_irq_w(int state);
	void vrtc_irq_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(clock_irq_w);
	IRQ_CALLBACK_MEMBER(int_ack_cb);

	struct {
		u8 enable = 0, pending = 0;
	} m_irq_state;

	bool m_sound_irq_enable = false;
	bool m_sound_irq_pending = false;

	enum {
		RXRDY_IRQ_LEVEL = 0,
		VRTC_IRQ_LEVEL,
		CLOCK_IRQ_LEVEL,
		INT3_IRQ_LEVEL,
		INT4_IRQ_LEVEL,
		INT5_IRQ_LEVEL,
		FDCINT1_IRQ_LEVEL,
		FDCINT2_IRQ_LEVEL
	};

	void assert_irq(u8 level);
	void check_irq(u8 level);

	void picu_reset();
};

class pc8001_state : public pc8001_base_state
{
public:
	pc8001_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8001_base_state(mconfig, type, tag)
		, m_pc80s31(*this, "pc80s31")
		, m_screen(*this, "screen")
		, m_beep(*this, "beeper")
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, Z80_TAG)
		, m_exp_view(*this, "exp_view")
	{ }

	void pc8001(machine_config &config);

protected:
	void pc8001_io(address_map &map) ATTR_COLD;
	void pc8001_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<pc80s31_device> m_pc80s31;
	required_device<screen_device> m_screen;
	required_device<beep_device> m_beep;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	memory_view m_exp_view;

	DECLARE_SNAPSHOT_LOAD_MEMBER(snapshot_cb);

	// TODO: these two should really be inside ram_device instead
	template <unsigned StartBase> uint8_t ram_r(address_space &space, offs_t offset)
	{
		const offs_t memory_offset = StartBase + offset;

		if (memory_offset < m_ram->size())
			return m_ram->pointer()[memory_offset];

		// TODO: verify what happens on unmapped access
		return space.unmap();
	}

	template <unsigned StartBase> void ram_w(offs_t offset, uint8_t data)
	{
		const offs_t memory_offset = StartBase + offset;

		if (memory_offset < m_ram->size())
			m_ram->pointer()[memory_offset] = data;
	}

private:
	uint8_t port40_r();
	void port40_w(uint8_t data);
};

class pc8001mk2_state : public pc8001_state
{
public:
	pc8001mk2_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8001_state(mconfig, type, tag)
		, m_gvram_bank(*this, "gvram_bank")
		, m_kanji_rom(*this, "kanji")
		, m_dsw(*this, "DSW%d", 1U)
	{ }

	void pc8001mk2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

	void pc8001mk2_io(address_map &map) ATTR_COLD;
	void pc8001mk2_map(address_map &map) ATTR_COLD;

	required_device<address_map_bank_device> m_gvram_bank;
	required_memory_region m_kanji_rom;
	required_ioport_array<2> m_dsw;

	std::unique_ptr<uint8_t[]> m_gvram;
	u8 m_port31;
	u8 m_vram_sel;
	bool m_text_layer_mask;
	u8 m_bitmap_layer_mask;
	bitmap_rgb32 m_text_bitmap;

	void port31_w(uint8_t data);
	virtual void flush_low_bank();
	virtual void flush_gvram_access();

	virtual void gvram_map(address_map &map);
	u8 gvram_r(offs_t offset);
	void gvram_w(offs_t offset, u8 data);

	void draw_bitmap_2bpp(bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device *palette, std::function<u8(u32 bitmap_offset, int y, int x, int xi)> dot_func);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

private:
};

class pc8001mk2sr_state : public pc8001mk2_state
{
public:
	pc8001mk2sr_state(const machine_config &mconfig, device_type type, const char *tag)
		: pc8001mk2_state(mconfig, type, tag)
		, m_n80sr_rom(*this, N80SR_ROM_TAG)
		, m_alu(*this, "alu")
		, m_palette(*this, "palette")
		, m_opn(*this, "opn")
		, m_alu_view(*this, "alu_view")
		, m_extram_view(*this, "extram_view")
	{ }

	void pc8001mk2sr(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

	void pc8001mk2sr_map(address_map &map) ATTR_COLD;
	void pc8001mk2sr_io(address_map &map) ATTR_COLD;

	required_memory_region m_n80sr_rom;
	required_device<pc88_alu_device> m_alu;
	required_device<palette_device> m_palette;
	required_device<ym2203_device> m_opn;
	memory_view m_alu_view;
	memory_view m_extram_view;

	u8 port33_r();
	void port33_w(u8 data);
	u8 port71_r();
	void port71_w(u8 data);
	void alu_ctrl2_w(u8 data);

	u8 m_n80sr_bank;
	u8 m_port32;
	u8 m_port33;
	u8 m_alu_gam;
	u8 m_extram_mode;
	struct { uint8_t r, g, b; } m_palram[8];
	bitmap_rgb32 m_graph_bitmap;

	virtual void flush_low_bank() override;
	virtual void flush_gvram_access() override;

	virtual void gvram_map(address_map &map) override;

	void draw_bitmap_w80(bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device *palette, std::function<u8(u32 bitmap_offset, int y, int x, int xi)> dot_func);
	void draw_bitmap_w40(bitmap_rgb32 &bitmap, const rectangle &cliprect, palette_device *palette, std::function<u8(int layer_n, u32 bitmap_offset, int y, int x, int xi)> dot_func);

	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
};

#endif // MAME_NEC_PC8001_H
