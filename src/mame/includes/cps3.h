// license:BSD-3-Clause
// copyright-holders:David Haywood, Andreas Naive, Tomasz Slanina, ElSemi
#ifndef MAME_INCLUDES_CPS3_H
#define MAME_INCLUDES_CPS3_H

#pragma once

/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "machine/intelfsh.h"
#include "cpu/sh/sh2.h"
#include "audio/cps3.h"
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
		, m_simm{{*this, "simm1.%u", 0U},
				 {*this, "simm2.%u", 0U},
				 {*this, "simm3.%u", 0U},
				 {*this, "simm4.%u", 0U},
				 {*this, "simm5.%u", 0U},
				 {*this, "simm6.%u", 0U},
				 {*this, "simm7.%u", 0U}}
		, m_mainram(*this, "mainram")
		, m_spriteram(*this, "spriteram")
		, m_colourram(*this, "colourram", 0)
		, m_tilemap20_regs_base(*this, "tmap20_regs")
		, m_tilemap30_regs_base(*this, "tmap30_regs")
		, m_tilemap40_regs_base(*this, "tmap40_regs")
		, m_tilemap50_regs_base(*this, "tmap50_regs")
		, m_ss_ram(*this, "ss_ram")
		, m_fullscreenzoom(*this, "fullscreenzoom")
		, m_0xc0000000_ram(*this, "0xc0000000_ram")
		, m_decrypted_gamerom(*this, "decrypted_gamerom")
		, m_0xc0000000_ram_decrypted(*this, "0xc0000000_ram_decrypted")
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
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void copy_from_nvram();
	u32 m_current_table_address;
	required_device<sh2_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<cps3_sound_device> m_cps3sound;
	optional_device_array<fujitsu_29f016a_device, 8> m_simm[7];

	required_shared_ptr<u32> m_mainram;
	required_shared_ptr<u32> m_spriteram;
	required_shared_ptr<u16> m_colourram;
	required_shared_ptr<u32> m_tilemap20_regs_base;
	required_shared_ptr<u32> m_tilemap30_regs_base;
	required_shared_ptr<u32> m_tilemap40_regs_base;
	required_shared_ptr<u32> m_tilemap50_regs_base;
	required_shared_ptr<u32> m_ss_ram;
	required_shared_ptr<u32> m_fullscreenzoom;
	required_shared_ptr<u32> m_0xc0000000_ram;
	required_shared_ptr<u32> m_decrypted_gamerom;
	required_shared_ptr<u32> m_0xc0000000_ram_decrypted;

	optional_memory_region      m_user4_region;
	optional_memory_region      m_user5_region;

private:
	u32 m_cram_gfxflash_bank;
	std::unique_ptr<u32[]> m_char_ram;
	std::unique_ptr<u32[]> m_eeprom;
	u32 m_ss_pal_base;
	u32 m_unk_vidregs[0x20/4];
	u32 m_ss_bank_base;
	u32 m_screenwidth;
	std::unique_ptr<u32[]> m_mame_colours;
	bitmap_rgb32 m_renderbuffer_bitmap;
	rectangle m_renderbuffer_clip;
	u8* m_user4;
	u32 m_key1;
	u32 m_key2;
	int m_altEncryption;
	u32 m_cram_bank;
	u16 m_current_eeprom_read;
	u32 m_paldma_source;
	u32 m_paldma_realsource;
	u32 m_paldma_dest;
	u32 m_paldma_fade;
	u32 m_paldma_other2;
	u32 m_paldma_length;
	u32 m_chardma_source;
	u32 m_chardma_other;
	int m_rle_length;
	int m_last_normal_byte;
	u16 m_lastb;
	u16 m_lastb2;
	u8* m_user5;

	DECLARE_READ32_MEMBER(ssram_r);
	DECLARE_WRITE32_MEMBER(ssram_w);
	DECLARE_WRITE32_MEMBER(_0xc0000000_ram_w);
	DECLARE_WRITE32_MEMBER(cram_bank_w);
	DECLARE_READ32_MEMBER(cram_data_r);
	DECLARE_WRITE32_MEMBER(cram_data_w);
	DECLARE_READ32_MEMBER(gfxflash_r);
	DECLARE_WRITE32_MEMBER(gfxflash_w);
	DECLARE_READ32_MEMBER(flash1_r);
	DECLARE_READ32_MEMBER(flash2_r);
	DECLARE_WRITE32_MEMBER(flash1_w);
	DECLARE_WRITE32_MEMBER(flash2_w);
	DECLARE_WRITE32_MEMBER(cram_gfxflash_bank_w);
	DECLARE_READ32_MEMBER(vbl_r);
	DECLARE_READ32_MEMBER(unk_io_r);
	DECLARE_READ32_MEMBER(_40C0000_r);
	DECLARE_READ32_MEMBER(_40C0004_r);
	DECLARE_READ32_MEMBER(eeprom_r);
	DECLARE_WRITE32_MEMBER(eeprom_w);
	DECLARE_WRITE32_MEMBER(ss_bank_base_w);
	DECLARE_WRITE32_MEMBER(ss_pal_base_w);
	DECLARE_WRITE32_MEMBER(palettedma_w);
	DECLARE_WRITE32_MEMBER(characterdma_w);
	DECLARE_WRITE32_MEMBER(irq10_ack_w);
	DECLARE_WRITE32_MEMBER(irq12_ack_w);
	DECLARE_WRITE32_MEMBER(unk_vidregs_w);
	DECLARE_READ16_MEMBER(colourram_r);
	DECLARE_WRITE16_MEMBER(colourram_w);
	SH2_DMA_KLUDGE_CB(dma_callback);
	void draw_fg_layer(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vbl_interrupt);
	INTERRUPT_GEN_MEMBER(other_interrupt);
	u16 rotate_left(u16 value, int n);
	u16 rotxor(u16 val, u16 xorval);
	u32 cps3_mask(u32 address, u32 key1, u32 key2);
	void decrypt_bios();
	void init_crypt(u32 key1, u32 key2, int altEncryption);
	void set_mame_colours(int colournum, u16 data, u32 fadeval);
	void draw_tilemapsprite_line(int tmnum, int drawline, bitmap_rgb32 &bitmap, const rectangle &cliprect );
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
	void cps3_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
};

#endif // MAME_INCLUDES_CPS3_H
