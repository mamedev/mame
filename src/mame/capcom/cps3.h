// license:BSD-3-Clause
// copyright-holders:David Haywood, Andreas Naive, Tomasz Slanina, ElSemi
#ifndef MAME_CAPCOM_CPS3_H
#define MAME_CAPCOM_CPS3_H

#pragma once

/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "machine/intelfsh.h"
#include "cpu/sh/sh7604.h"
#include "cps3_a.h"
#include "machine/timer.h"
#include "emupal.h"


class cps3_state : public driver_device
{
public:
	cps3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_cps3sound(*this, "cps3sound")
		, m_dma_timer(*this, "dma_timer")
		, m_spritelist_dma_timer(*this, "spritelist_dma_timer")
		, m_simm{{*this, "simm1.%u", 0U},
				 {*this, "simm2.%u", 0U},
				 {*this, "simm3.%u", 0U},
				 {*this, "simm4.%u", 0U},
				 {*this, "simm5.%u", 0U},
				 {*this, "simm6.%u", 0U},
				 {*this, "simm7.%u", 0U}}
		, m_mainram(*this, "mainram")
		, m_spriteram(*this, "spriteram")
		, m_colourram(*this, "colourram", 0x40000, ENDIANNESS_BIG)
		, m_ppu_gscroll(*this, "ppu_gscroll_regs")
		, m_tilemap_regs(*this, "ppu_tmap_regs")
		, m_ppu_crtc_zoom(*this, "ppu_crtc_zoom")
		, m_sh2cache_ram(*this, "sh2cache_ram")
		, m_decrypted_gamerom(*this, "decrypted_gamerom")
		, m_sh2cache_ram_decrypted(*this, "sh2cache_ram_decrypted")
		, m_user4_region(*this, "user4")
		, m_user5_region(*this, "user5")
	{
	}

	void init_sfiii3();
	void init_sfiii();
	void init_redearth();
	void init_jojo();
	void init_jojoba();
	void init_sfiii2();
	void init_cps3boot();

	void cps3(machine_config &config);
	void jojo(machine_config &config);
	void redearth(machine_config &config);
	void sfiii2(machine_config &config);
	void sfiii3(machine_config &config);
	void sfiii(machine_config &config);
	void jojoba(machine_config &config);
	void simm_config(machine_config &config, unsigned slot, unsigned count);
	void simm1_64mbit(machine_config &config);
	void simm2_64mbit(machine_config &config);
	void simm3_128mbit(machine_config &config);
	void simm4_128mbit(machine_config &config);
	void simm5_128mbit(machine_config &config);
	void simm5_32mbit(machine_config &config);
	void simm6_128mbit(machine_config &config);

protected:
	virtual void device_post_load() override;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void copy_from_nvram();
	u32 m_current_table_address;
	required_device<sh7604_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<cps3_sound_device> m_cps3sound;
	required_device<timer_device> m_dma_timer;
	required_device<timer_device> m_spritelist_dma_timer;
	optional_device_array<fujitsu_29f016a_device, 8> m_simm[7];

	required_shared_ptr<u32> m_mainram;
	required_shared_ptr<u32> m_spriteram;
	memory_share_creator<u16> m_colourram;
	required_shared_ptr<u32> m_ppu_gscroll;
	required_shared_ptr<u32> m_tilemap_regs;
	required_shared_ptr<u32> m_ppu_crtc_zoom;
	required_shared_ptr<u32> m_sh2cache_ram;
	required_shared_ptr<u32> m_decrypted_gamerom;
	required_shared_ptr<u32> m_sh2cache_ram_decrypted;

	optional_memory_region      m_user4_region;
	optional_memory_region      m_user5_region;

private:
	u32 m_cram_gfxflash_bank = 0;
	std::unique_ptr<u32[]> m_char_ram;
	std::unique_ptr<u32[]> m_eeprom;
	std::unique_ptr<u8[]>  m_ss_ram;
	std::unique_ptr<u32[]> m_spritelist;
	u32 m_ppu_gscroll_buff[0x20/4]{};
	s16 m_ss_hscroll = 0;
	s16 m_ss_vscroll = 0;
	u8  m_ss_pal_base = 0;
	u32 m_screenwidth = 0;
	std::unique_ptr<u32[]> m_mame_colours;
	bitmap_rgb32 m_renderbuffer_bitmap;
	rectangle m_renderbuffer_clip;
	u8* m_user4 = nullptr;
	std::unique_ptr<u8[]> m_user4_allocated;
	u32 m_key1 = 0;
	u32 m_key2 = 0;
	int m_altEncryption = 0;
	u16 m_dma_status = 0;
	u16 m_spritelist_dma = 0;
	u32 m_cram_bank = 0;
	u16 m_current_eeprom_read = 0;
	u32 m_paldma_source = 0;
	u32 m_paldma_realsource = 0;
	u32 m_paldma_dest = 0;
	u32 m_paldma_fade = 0;
	u32 m_paldma_other2 = 0;
	u32 m_paldma_length = 0;
	u32 m_chardma_source = 0;
	u32 m_chardma_other = 0;
	int m_rle_length = 0;
	int m_last_normal_byte = 0;
	u16 m_lastb = 0;
	u16 m_lastb2 = 0;
	u8* m_user5 = nullptr;
	std::unique_ptr<u8[]> m_user5_allocated;

	u8 ssram_r(offs_t offset);
	void ssram_w(offs_t offset, u8 data);
	void ssregs_w(offs_t offset, u8 data);
	void sh2cache_ram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void cram_bank_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 cram_data_r(offs_t offset);
	void cram_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 gfxflash_r(offs_t offset, u32 mem_mask = ~0);
	void gfxflash_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 flash1_r(offs_t offset, u32 mem_mask = ~0);
	u32 flash2_r(offs_t offset, u32 mem_mask = ~0);
	void flash1_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void flash2_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void cram_gfxflash_bank_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u16 dma_status_r();
	u16 dev_dipsw_r();
	u32 eeprom_r(offs_t offset, u32 mem_mask = ~0);
	void eeprom_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void palettedma_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void characterdma_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u16 colourram_r(offs_t offset);
	void colourram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void outport_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void spritedma_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	SH2_DMA_KLUDGE_CB(dma_callback);
	void draw_fg_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vbl_interrupt(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(dma_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(sprite_dma_cb);
	u16 rotate_left(u16 value, int n);
	u16 rotxor(u16 val, u16 xorval);
	u32 cps3_mask(u32 address, u32 key1, u32 key2);
	void decrypt_bios();
	void init_crypt(u32 key1, u32 key2, int altEncryption);
	void set_mame_colours(int colournum, u16 data, u32 fadeval);
	void draw_tilemapsprite_line(u32* regs, int drawline, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	u32 flashmain_r(int which, u32 offset, u32 mem_mask);
	void flashmain_w(int which, u32 offset, u32 data, u32 mem_mask);
	u32 process_byte( u8 real_byte, u32 destination, int max_length );
	void do_char_dma( u32 real_source, u32 real_destination, u32 real_length );
	u32 ProcessByte8(u8 b,u32 dst_offset);
	void do_alt_char_dma( u32 src, u32 real_dest, u32 real_length );
	void process_character_dma(u32 address);
	inline void cps3_drawgfxzoom(bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx,
		u32 code, u32 color, int flipx, int flipy, int sx, int sy,
		int transparency, int transparent_color,
		int scalex, int scaley);
	void cps3_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
};

#endif // MAME_CAPCOM_CPS3_H
