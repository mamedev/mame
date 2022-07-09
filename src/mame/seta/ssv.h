// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_SSV_H
#define MAME_INCLUDES_SSV_H

#pragma once

#include "cpu/upd7725/upd7725.h"
#include "st0020.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "machine/upd4701.h"
#include "machine/upd7001.h"
#include "sound/es5506.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

class ssv_state : public driver_device
{
public:
	ssv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ensoniq(*this, "ensoniq"),
		m_dsp(*this, "dsp"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_irq_vectors(*this, "irq_vectors"),
		m_input_sel(*this, "input_sel"),
		m_srmp7_esbank(*this, "esbank_%u", 2U),
		m_raster_interrupt_enabled(false),
		m_io_key(*this, "KEY%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void ssv(machine_config &config);
	void dynagear(machine_config &config);
	void hypreac2(machine_config &config);
	void meosism(machine_config &config);
	void drifto94(machine_config &config);
	void stmblade(machine_config &config);
	void srmp4(machine_config &config);
	void srmp7(machine_config &config);
	void twineag2(machine_config &config);
	void ryorioh(machine_config &config);
	void janjans1(machine_config &config);
	void survarts(machine_config &config);
	void ultrax(machine_config &config);
	void vasara(machine_config &config);
	void mslider(machine_config &config);
	void jsk(machine_config &config);
	void hypreact(machine_config &config);
	void keithlcy(machine_config &config);
	void pastelis(machine_config &config);
	void cairblad(machine_config &config);

	void init_ssv();
	void init_ssv_tilescram();
	void init_ssv_irq1();
	void init_srmp7();
	void init_jsk();
	void init_pastelis();

protected:
	required_device<cpu_device> m_maincpu;
	required_device<es5506_device> m_ensoniq;
	optional_device<upd96050_device> m_dsp;

	required_shared_ptr<uint16_t> m_mainram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_irq_vectors;
	optional_shared_ptr<uint16_t> m_input_sel;
	optional_memory_bank_array<2> m_srmp7_esbank;

	int m_tile_code[16]{};
	int m_enable_video = 0;
	int m_shadow_pen_mask = 0;
	int m_shadow_pen_shift = 0;
	uint8_t m_requested_int = 0;
	uint16_t m_irq_enable = 0;
	int m_interrupt_ultrax = 0;
	bool m_raster_interrupt_enabled;
	uint32_t m_latches[8]{};

	void irq_ack_w(offs_t offset, uint16_t data);
	void irq_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lockout_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void lockout_inv_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_dr_r();
	void dsp_dr_w(uint16_t data);
	uint16_t dsp_r(offs_t offset);
	[[maybe_unused]] uint16_t fake_r(offs_t offset);
	void dsp_w(offs_t offset, uint16_t data);
	uint16_t drifto94_unknown_r();
	uint16_t hypreact_input_r();
	uint16_t mainram_r(offs_t offset);
	void mainram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t srmp4_input_r();
	uint16_t srmp7_irqv_r();
	void srmp7_sound_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t srmp7_input_r();
	uint32_t latch32_r(offs_t offset);
	void latch32_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t latch16_r(offs_t offset);
	void latch16_w(offs_t offset, uint16_t data);
	uint16_t vblank_r();
	void scroll_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	void update_irq_state();
	IRQ_CALLBACK_MEMBER(irq_callback);

	void drawgfx_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int gfx, uint32_t code, uint32_t color, int flipx, int flipy, int base_sx, int base_sy, int shadow, int realline, int line);
	void drawgfx(bitmap_ind16 &bitmap, const rectangle &cliprect, int gfx,uint32_t code, uint32_t color, int flipx, int flipy, int base_sx, int base_sy,int shadow);

	void draw_16x16_tile_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int flipx, int flipy, int mode, int code, int color, int sx, int sy, int realline, int line);
	void get_tile(int x, int y, int size, int page, int& code, int& attr, int& flipx, int& flipy);
	void draw_row_64pixhigh(bitmap_ind16 &bitmap, const rectangle &cliprect, int in_sy, int scroll);
	void draw_layer(bitmap_ind16 &bitmap, const rectangle &cliprect, int  nr);

	void draw_sprites_tiles(bitmap_ind16 &bitmap, const rectangle &cliprect, int code, int flipx, int flipy, int gfx, int shadow, int color, int sx, int sy, int xnum, int ynum);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void enable_video(int enable);

	void cairblad_map(address_map &map);
	void drifto94_map(address_map &map);
	void dsp_data_map(address_map &map);
	void dsp_prg_map(address_map &map);
	void hypreac2_map(address_map &map);
	void hypreact_map(address_map &map);
	void janjans1_map(address_map &map);
	void jsk_map(address_map &map);
	void jsk_v810_mem(address_map &map);
	void keithlcy_map(address_map &map);
	void meosism_map(address_map &map);
	void mslider_map(address_map &map);
	void ryorioh_map(address_map &map);
	void srmp4_map(address_map &map);
	void srmp7_map(address_map &map);
	void srmp7_es5506_bank2_map(address_map &map);
	void srmp7_es5506_bank3_map(address_map &map);
	void survarts_map(address_map &map);
	void twineag2_map(address_map &map);
	void ultrax_map(address_map &map);

	optional_ioport_array<4> m_io_key;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void ssv_map(address_map &map, u32 rom);
};

class gdfs_state : public ssv_state
{
public:
	gdfs_state(const machine_config &mconfig, device_type type, const char *tag) :
		ssv_state(mconfig, type, tag),
		m_adc(*this, "adc"),
		m_eeprom(*this, "eeprom"),
		m_st0020(*this, "st0020_spr"),
		m_tmapram(*this, "gdfs_tmapram"),
		m_tmapscroll(*this, "gdfs_tmapscroll")
	{ }

	void gdfs(machine_config &config);

protected:
	virtual void video_start() override;

private:
	DECLARE_WRITE_LINE_MEMBER(adc_int_w);

	uint16_t eeprom_r();
	void eeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void gdfs_map(address_map &map);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	void tmapram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<adc0808_device> m_adc;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<st0020_device> m_st0020;
	required_shared_ptr<uint16_t> m_tmapram;
	required_shared_ptr<uint16_t> m_tmapscroll;

	tilemap_t *m_tmap = nullptr;
};

class eaglshot_state : public ssv_state
{
public:
	eaglshot_state(const machine_config &mconfig, device_type type, const char *tag) :
		ssv_state(mconfig, type, tag),
		m_upd4701(*this, "upd4701")
	{ }

	void eaglshot(machine_config &config);

	void init_eaglshot();

protected:
	virtual void video_start() override;

private:
	void gfxrom_bank_w(uint8_t data);
	void trackball_w(uint8_t data);
	uint16_t gfxram_r(offs_t offset, uint16_t mem_mask);
	void gfxram_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void eaglshot_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<upd4701_device> m_upd4701;

	std::unique_ptr<uint16_t[]> m_gfxram;
};

class sxyreact_state : public ssv_state
{
public:
	sxyreact_state(const machine_config &mconfig, device_type type, const char *tag) :
		ssv_state(mconfig, type, tag),
		m_sxyreact_adc(*this, "adc"),
		m_io_service(*this, "SERVICE")
	{ }

	void sxyreact(machine_config &config);
	void sxyreac2(machine_config &config);

	void init_sexy();

private:
	uint16_t ballswitch_r();
	uint8_t dial_r();
	void dial_w(uint8_t data);
	void motor_w(uint16_t data);

	void sxyreact_map(address_map &map);

	required_device<upd7001_device> m_sxyreact_adc;
	required_ioport m_io_service;
};

#endif // MAME_INCLUDES_SSV_H
