// license:BSD-3-Clause
// copyright-holders:David Haywood, Andreas Naive, Tomasz Slanina, ElSemi
/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "machine/intelfsh.h"
#include "cpu/sh2/sh2.h"
#include "audio/cps3.h"


class cps3_state : public driver_device
{
public:
	cps3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_cps3sound(*this, "cps3sound"),
		m_mainram(*this, "mainram"),
		m_spriteram(*this, "spriteram"),
		m_colourram(*this, "colourram"),
		m_tilemap20_regs_base(*this, "tmap20_regs"),
		m_tilemap30_regs_base(*this, "tmap30_regs"),
		m_tilemap40_regs_base(*this, "tmap40_regs"),
		m_tilemap50_regs_base(*this, "tmap50_regs"),
		m_fullscreenzoom(*this, "fullscreenzoom"),
		m_0xc0000000_ram(*this, "0xc0000000_ram"),
		m_decrypted_gamerom(*this, "decrypted_gamerom"),
		m_0xc0000000_ram_decrypted(*this, "0xc0000000_ram_decrypted")
	{ }

	required_device<sh2_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<cps3_sound_device> m_cps3sound;

	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_spriteram;
	required_shared_ptr<UINT32> m_colourram;
	required_shared_ptr<UINT32> m_tilemap20_regs_base;
	required_shared_ptr<UINT32> m_tilemap30_regs_base;
	required_shared_ptr<UINT32> m_tilemap40_regs_base;
	required_shared_ptr<UINT32> m_tilemap50_regs_base;
	required_shared_ptr<UINT32> m_fullscreenzoom;
	required_shared_ptr<UINT32> m_0xc0000000_ram;
	required_shared_ptr<UINT32> m_decrypted_gamerom;
	required_shared_ptr<UINT32> m_0xc0000000_ram_decrypted;

	fujitsu_29f016a_device *m_simm[7][8];
	UINT32 m_cram_gfxflash_bank;
	UINT32* m_nops;
	UINT32* m_char_ram;
	UINT32* m_eeprom;
	UINT32 m_ss_pal_base;
	UINT32 m_unk_vidregs[0x20/4];
	UINT32 m_ss_bank_base;
	UINT32 m_screenwidth;
	UINT32* m_mame_colours;
	bitmap_rgb32 m_renderbuffer_bitmap;
	rectangle m_renderbuffer_clip;
	UINT8* m_user4region;
	UINT32 m_key1;
	UINT32 m_key2;
	int m_altEncryption;
	UINT32* m_ss_ram;
	UINT32 m_cram_bank;
	UINT16 m_current_eeprom_read;
	UINT32 m_paldma_source;
	UINT32 m_paldma_realsource;
	UINT32 m_paldma_dest;
	UINT32 m_paldma_fade;
	UINT32 m_paldma_other2;
	UINT32 m_paldma_length;
	UINT32 m_chardma_source;
	UINT32 m_chardma_other;
	UINT32 m_current_table_address;
	int m_rle_length;
	int m_last_normal_byte;
	unsigned short m_lastb;
	unsigned short m_lastb2;
	UINT8* m_user5region;

	DECLARE_READ32_MEMBER(cps3_ssram_r);
	DECLARE_WRITE32_MEMBER(cps3_ssram_w);
	DECLARE_WRITE32_MEMBER(cps3_0xc0000000_ram_w);
	DECLARE_WRITE32_MEMBER(cram_bank_w);
	DECLARE_READ32_MEMBER(cram_data_r);
	DECLARE_WRITE32_MEMBER(cram_data_w);
	DECLARE_READ32_MEMBER(cps3_gfxflash_r);
	DECLARE_WRITE32_MEMBER(cps3_gfxflash_w);
	DECLARE_READ32_MEMBER(cps3_flash1_r);
	DECLARE_READ32_MEMBER(cps3_flash2_r);
	DECLARE_WRITE32_MEMBER(cps3_flash1_w);
	DECLARE_WRITE32_MEMBER(cps3_flash2_w);
	DECLARE_WRITE32_MEMBER(cram_gfxflash_bank_w);
	DECLARE_READ32_MEMBER(cps3_vbl_r);
	DECLARE_READ32_MEMBER(cps3_unk_io_r);
	DECLARE_READ32_MEMBER(cps3_40C0000_r);
	DECLARE_READ32_MEMBER(cps3_40C0004_r);
	DECLARE_READ32_MEMBER(cps3_eeprom_r);
	DECLARE_WRITE32_MEMBER(cps3_eeprom_w);
	DECLARE_WRITE32_MEMBER(cps3_ss_bank_base_w);
	DECLARE_WRITE32_MEMBER(cps3_ss_pal_base_w);
	DECLARE_WRITE32_MEMBER(cps3_palettedma_w);
	DECLARE_WRITE32_MEMBER(cps3_characterdma_w);
	DECLARE_WRITE32_MEMBER(cps3_irq10_ack_w);
	DECLARE_WRITE32_MEMBER(cps3_irq12_ack_w);
	DECLARE_WRITE32_MEMBER(cps3_unk_vidregs_w);
	DECLARE_READ32_MEMBER(cps3_colourram_r);
	DECLARE_WRITE32_MEMBER(cps3_colourram_w);
	DECLARE_DRIVER_INIT(sfiii3);
	DECLARE_DRIVER_INIT(sfiii);
	DECLARE_DRIVER_INIT(redearth);
	DECLARE_DRIVER_INIT(jojo);
	DECLARE_DRIVER_INIT(jojoba);
	DECLARE_DRIVER_INIT(sfiii2);
	DECLARE_DRIVER_INIT(cps3boot);
	SH2_DMA_KLUDGE_CB(dma_callback);
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_cps3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cps3_vbl_interrupt);
	INTERRUPT_GEN_MEMBER(cps3_other_interrupt);
	UINT16 rotate_left(UINT16 value, int n);
	UINT16 rotxor(UINT16 val, UINT16 xorval);
	UINT32 cps3_mask(UINT32 address, UINT32 key1, UINT32 key2);
	void cps3_decrypt_bios();
	void init_common(void);
	void init_crypt(UINT32 key1, UINT32 key2, int altEncryption);
	void cps3_set_mame_colours(int colournum, UINT16 data, UINT32 fadeval);
	void cps3_draw_tilemapsprite_line(int tmnum, int drawline, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	UINT32 cps3_flashmain_r(address_space &space, int which, UINT32 offset, UINT32 mem_mask);
	void cps3_flashmain_w(int which, UINT32 offset, UINT32 data, UINT32 mem_mask);
	UINT32 process_byte( UINT8 real_byte, UINT32 destination, int max_length );
	void cps3_do_char_dma( UINT32 real_source, UINT32 real_destination, UINT32 real_length );
	UINT32 ProcessByte8(UINT8 b,UINT32 dst_offset);
	void cps3_do_alt_char_dma( UINT32 src, UINT32 real_dest, UINT32 real_length );
	void cps3_process_character_dma(UINT32 address);
	void copy_from_nvram();
	inline void cps3_drawgfxzoom(bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx,
		unsigned int code, unsigned int color, int flipx, int flipy, int sx, int sy,
		int transparency, int transparent_color,
		int scalex, int scaley, bitmap_ind8 *pri_buffer, UINT32 pri_mask);
};
