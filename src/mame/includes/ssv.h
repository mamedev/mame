// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_SSV_H
#define MAME_INCLUDES_SSV_H

#pragma once

#include "cpu/upd7725/upd7725.h"
#include "video/st0020.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "machine/upd4701.h"
#include "sound/es5506.h"
#include "emupal.h"
#include "screen.h"

class ssv_state : public driver_device
{
public:
	ssv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ensoniq(*this, "ensoniq"),
		m_eeprom(*this, "eeprom"),
		m_dsp(*this, "dsp"),
		m_upd4701(*this, "upd4701"),
		m_adc(*this, "adc"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_irq_vectors(*this, "irq_vectors"),
		m_gdfs_tmapram(*this, "gdfs_tmapram"),
		m_gdfs_tmapscroll(*this, "gdfs_tmapscroll"),
		m_gdfs_st0020(*this, "st0020_spr"),
		m_input_sel(*this, "input_sel"),
		m_raster_interrupt_enabled(false),
		m_io_key(*this, "KEY%u", 0U),
		m_io_service(*this, "SERVICE"),
		m_io_paddle(*this, "PADDLE"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void ssv(machine_config &config);
	void gdfs(machine_config &config);
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
	void eaglshot(machine_config &config);
	void survarts(machine_config &config);
	void sxyreac2(machine_config &config);
	void ultrax(machine_config &config);
	void vasara(machine_config &config);
	void sxyreact(machine_config &config);
	void mslider(machine_config &config);
	void jsk(machine_config &config);
	void hypreact(machine_config &config);
	void keithlcy(machine_config &config);
	void pastelis(machine_config &config);
	void cairblad(machine_config &config);

	void init_ssv();
	void init_ssv_tilescram();
	void init_eaglshot();
	void init_sexy();
	void init_ssv_irq1();
	void init_jsk();
	void init_pastelis();

private:
	required_device<cpu_device> m_maincpu;
	required_device<es5506_device> m_ensoniq;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<upd96050_device> m_dsp;
	optional_device<upd4701_device> m_upd4701;
	optional_device<adc0808_device> m_adc;

	required_shared_ptr<uint16_t> m_mainram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_scroll;
	required_shared_ptr<uint16_t> m_irq_vectors;
	optional_shared_ptr<uint16_t> m_gdfs_tmapram;
	optional_shared_ptr<uint16_t> m_gdfs_tmapscroll;
	optional_device<st0020_device> m_gdfs_st0020;
	optional_shared_ptr<uint16_t> m_input_sel;

	int m_tile_code[16];
	int m_enable_video;
	int m_shadow_pen_mask;
	int m_shadow_pen_shift;
	uint8_t m_requested_int;
	uint16_t m_irq_enable;
	std::unique_ptr<uint16_t[]> m_eaglshot_gfxram;
	tilemap_t *m_gdfs_tmap;
	int m_interrupt_ultrax;
	bool m_raster_interrupt_enabled;
	uint16_t m_sxyreact_serial;
	int m_sxyreact_dial;
	uint32_t m_latches[8];

	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_WRITE16_MEMBER(irq_enable_w);
	DECLARE_WRITE16_MEMBER(lockout_w);
	DECLARE_WRITE16_MEMBER(lockout_inv_w);
	DECLARE_READ16_MEMBER(dsp_dr_r);
	DECLARE_WRITE16_MEMBER(dsp_dr_w);
	DECLARE_READ16_MEMBER(dsp_r);
	DECLARE_WRITE16_MEMBER(dsp_w);
	DECLARE_READ16_MEMBER(drifto94_unknown_r);
	DECLARE_READ16_MEMBER(hypreact_input_r);
	DECLARE_READ16_MEMBER(mainram_r);
	DECLARE_WRITE16_MEMBER(mainram_w);
	DECLARE_READ16_MEMBER(srmp4_input_r);
	DECLARE_READ16_MEMBER(srmp7_irqv_r);
	DECLARE_WRITE16_MEMBER(srmp7_sound_bank_w);
	DECLARE_READ16_MEMBER(srmp7_input_r);
	DECLARE_READ16_MEMBER(sxyreact_ballswitch_r);
	DECLARE_READ16_MEMBER(sxyreact_dial_r);
	DECLARE_WRITE16_MEMBER(sxyreact_dial_w);
	DECLARE_WRITE16_MEMBER(sxyreact_motor_w);
	DECLARE_READ32_MEMBER(latch32_r);
	DECLARE_WRITE32_MEMBER(latch32_w);
	DECLARE_READ16_MEMBER(latch16_r);
	DECLARE_WRITE16_MEMBER(latch16_w);
	DECLARE_WRITE8_MEMBER(eaglshot_gfxrom_bank_w);
	DECLARE_WRITE8_MEMBER(eaglshot_trackball_w);
	DECLARE_READ16_MEMBER(eaglshot_gfxram_r);
	DECLARE_WRITE16_MEMBER(eaglshot_gfxram_w);
	DECLARE_WRITE16_MEMBER(gdfs_tmapram_w);
	DECLARE_READ16_MEMBER(vblank_r);
	DECLARE_WRITE16_MEMBER(scroll_w);
	DECLARE_READ16_MEMBER(gdfs_eeprom_r);
	DECLARE_WRITE16_MEMBER(gdfs_eeprom_w);

	TILE_GET_INFO_MEMBER(get_tile_info_0);

	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_VIDEO_START(gdfs);
	DECLARE_VIDEO_START(eaglshot);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gdfs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_eaglshot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_DEVICE_CALLBACK_MEMBER(interrupt);
	DECLARE_WRITE_LINE_MEMBER(gdfs_adc_int_w);
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

	void drifto94_map(address_map &map);
	void dsp_data_map(address_map &map);
	void dsp_prg_map(address_map &map);
	void eaglshot_map(address_map &map);
	void gdfs_map(address_map &map);
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
	void survarts_map(address_map &map);
	void sxyreact_map(address_map &map);
	void twineag2_map(address_map &map);
	void ultrax_map(address_map &map);

	optional_ioport_array<4> m_io_key;
	optional_ioport m_io_service;
	optional_ioport m_io_paddle;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void ssv_map(address_map &map, u32 rom);
};

#endif // MAME_INCLUDES_SSV_H
