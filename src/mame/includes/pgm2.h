// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_INCLUDES_PGM2_H
#define MAME_INCLUDES_PGM2_H

#pragma once

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "sound/ymz770.h"
#include "machine/igs036crypt.h"
#include "screen.h"
#include "speaker.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/atmel_arm_aic.h"
#include "machine/pgm2_memcard.h"
#include "emupal.h"

struct kov3_module_key
{
	uint8_t key[8];
	uint8_t sum[8];
	uint32_t addr_xor; // 22bit
	uint16_t data_xor;
};

class pgm2_state : public driver_device
{
public:
	pgm2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_lineram(*this, "lineram"),
		m_sp_zoom(*this, "sp_zoom"),
		m_mainram(*this, "mainram"),
		m_romboard_ram(*this, "romboard_ram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_sp_videoram(*this, "sp_videoram"),
		m_bgscroll(*this, "bgscroll"),
		m_fgscroll(*this, "fgscroll"),
		m_vidmode(*this, "vidmode"),
		m_gfxdecode(*this, "gfxdecode%u", 2U),
		m_arm_aic(*this, "arm_aic"),
		m_sprites_mask(*this, "sprites_mask"),
		m_sprites_colour(*this, "sprites_colour"),
		m_sp_palette(*this, "sp_palette"),
		m_bg_palette(*this, "bg_palette"),
		m_tx_palette(*this, "tx_palette"),
		m_mcu_timer(*this, "mcu_timer"),
		m_maindata(*this, "maindata"),
		m_memcard(*this, "memcard_p%u", 1U) { }

	DECLARE_READ32_MEMBER(unk_startup_r);
	DECLARE_READ32_MEMBER(rtc_r);
	DECLARE_READ32_MEMBER(mcu_r);
	DECLARE_WRITE32_MEMBER(fg_videoram_w);
	DECLARE_WRITE32_MEMBER(bg_videoram_w);
	DECLARE_WRITE32_MEMBER(mcu_w);
	DECLARE_WRITE16_MEMBER(share_bank_w);
	DECLARE_READ8_MEMBER(shareram_r);
	DECLARE_WRITE8_MEMBER(shareram_w);
	DECLARE_WRITE16_MEMBER(vbl_ack_w);
	DECLARE_WRITE16_MEMBER(unk30120014_w);

	DECLARE_WRITE32_MEMBER(pio_sodr_w);
	DECLARE_WRITE32_MEMBER(pio_codr_w);
	DECLARE_READ32_MEMBER(pio_pdsr_r);
	DECLARE_WRITE16_MEMBER(module_rom_w);
	DECLARE_READ16_MEMBER(module_rom_r);
	DECLARE_READ_LINE_MEMBER(module_data_r);
	DECLARE_WRITE_LINE_MEMBER(module_data_w);
	DECLARE_WRITE_LINE_MEMBER(module_clk_w);

	DECLARE_READ32_MEMBER(orleg2_speedup_r);
	DECLARE_READ32_MEMBER(kov2nl_speedup_r);
	DECLARE_READ32_MEMBER(kof98umh_speedup_r);
	DECLARE_READ32_MEMBER(ddpdojt_speedup_r);
	DECLARE_READ32_MEMBER(ddpdojt_speedup2_r);
	DECLARE_READ32_MEMBER(kov3_speedup_r);

	DECLARE_READ8_MEMBER(encryption_r);
	DECLARE_WRITE8_MEMBER(encryption_w);
	DECLARE_WRITE32_MEMBER(encryption_do_w);
	DECLARE_WRITE32_MEMBER(sprite_encryption_w);

	void init_kov2nl();
	void init_orleg2();
	void init_ddpdojt();
	void init_kov3();
	void init_kov3_104();
	void init_kov3_102();
	void init_kov3_101();
	void init_kov3_100();
	void init_kof98umh();

	uint32_t screen_update_pgm2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_pgm2);

	TIMER_DEVICE_CALLBACK_MEMBER(igs_interrupt);

	void pgm2_ramrom(machine_config &config);
	void pgm2_lores(machine_config &config);
	void pgm2(machine_config &config);
	void pgm2_hires(machine_config &config);
	void pgm2_map(address_map &map);
	void pgm2_module_rom_map(address_map &map);
	void pgm2_ram_rom_map(address_map &map);
	void pgm2_rom_map(address_map &map);
private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void decrypt_kov3_module(uint32_t addrxor, uint16_t dataxor);

	tilemap_t    *m_fg_tilemap;
	tilemap_t    *m_bg_tilemap;

	std::unique_ptr<uint32_t[]>     m_spritebufferram; // buffered spriteram

	void skip_sprite_chunk(int &palette_offset, uint32_t maskdata, int reverse);
	void draw_sprite_pixel(uint32_t* dst, uint8_t* dstpri, const rectangle &cliprect, const pen_t *pen, int palette_offset, int realx, int realy, int pri);
	void draw_sprite_chunk(uint32_t* dst, uint8_t* dstpri, const rectangle &cliprect, const pen_t *pen,
	                       int &palette_offset, int x, int realy, int sizex, int xdraw, int pri, uint32_t maskdata, uint32_t zoomx_bits, int repeats, int &realxdraw, int realdraw_inc, int palette_inc);
	void draw_sprite_line(bitmap_rgb32 &bitmap, bitmap_ind8 &primap, const rectangle &cliprect, const pen_t *pen,
	                       int &mask_offset, int &palette_offset, int x, int realy, int flipx, int reverse, int sizex, int pri, int zoomybit, int zoomx_bits, int xrepeats);
	void draw_sprites(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap, const uint32_t* spriteram);

	void common_encryption_init();
	std::unique_ptr<uint8_t[]> m_encryption_table;
	int m_has_decrypted;    // so we only do it once.
	int m_has_decrypted_kov3_module;
	uint32_t m_spritekey;
	uint32_t m_realspritekey;
	int m_sprite_predecrypted;

	std::unique_ptr<uint8_t[]> m_shareram;
	uint16_t m_share_bank;
	uint32_t m_mcu_regs[8];
	uint32_t m_mcu_result0;
	uint32_t m_mcu_result1;
	uint8_t m_mcu_last_cmd;
	void mcu_command(address_space &space, bool is_command);

	std::vector<uint8_t> m_encrypted_copy;

	uint32_t pio_out_data;
	const kov3_module_key *module_key;
	bool module_sum_read;
	uint32_t module_in_latch;
	uint32_t module_out_latch;
	int module_prev_state;
	int module_clk_cnt;
	uint8_t module_rcv_buf[10];
	uint8_t module_send_buf[9];

	void postload();

	// devices
	required_device<igs036_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_lineram;
	required_shared_ptr<uint32_t> m_sp_zoom;
	required_shared_ptr<uint32_t> m_mainram;
	optional_shared_ptr<uint32_t> m_romboard_ram;
	required_shared_ptr<uint32_t> m_fg_videoram;
	required_shared_ptr<uint32_t> m_bg_videoram;
	required_shared_ptr<uint32_t> m_sp_videoram;
	required_shared_ptr<uint32_t> m_bgscroll;
	required_shared_ptr<uint32_t> m_fgscroll;
	required_shared_ptr<uint32_t> m_vidmode;
	required_device_array<gfxdecode_device, 2> m_gfxdecode;
	required_device<arm_aic_device> m_arm_aic;
	required_region_ptr<uint8_t> m_sprites_mask;
	required_region_ptr<uint8_t> m_sprites_colour;
	required_device<palette_device> m_sp_palette;
	required_device<palette_device> m_bg_palette;
	required_device<palette_device> m_tx_palette;
	required_device<timer_device> m_mcu_timer;

	required_memory_region m_maindata;

	optional_device_array<pgm2_memcard_device, 4> m_memcard;
};

#endif
