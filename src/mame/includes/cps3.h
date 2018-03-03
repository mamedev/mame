// license:BSD-3-Clause
// copyright-holders:David Haywood, Andreas Naive, Tomasz Slanina, ElSemi
/***************************************************************************

    Capcom CPS-3 Hardware

****************************************************************************/

#include "machine/intelfsh.h"
#include "cpu/sh/sh2.h"
#include "audio/cps3.h"


class cps3_state : public driver_device
{
public:
	cps3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_cps3sound(*this, "cps3sound")
		, m_mainram(*this, "mainram")
		, m_spriteram(*this, "spriteram")
		, m_colourram(*this, "colourram")
		, m_tilemap20_regs_base(*this, "tmap20_regs")
		, m_tilemap30_regs_base(*this, "tmap30_regs")
		, m_tilemap40_regs_base(*this, "tmap40_regs")
		, m_tilemap50_regs_base(*this, "tmap50_regs")
		, m_fullscreenzoom(*this, "fullscreenzoom")
		, m_0xc0000000_ram(*this, "0xc0000000_ram")
		, m_decrypted_gamerom(*this, "decrypted_gamerom")
		, m_0xc0000000_ram_decrypted(*this, "0xc0000000_ram_decrypted")
		, m_user4_region(*this, "user4")
		, m_user5_region(*this, "user5")
	{
	}

	required_device<sh2_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<cps3_sound_device> m_cps3sound;

	required_shared_ptr<uint32_t> m_mainram;
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_colourram;
	required_shared_ptr<uint32_t> m_tilemap20_regs_base;
	required_shared_ptr<uint32_t> m_tilemap30_regs_base;
	required_shared_ptr<uint32_t> m_tilemap40_regs_base;
	required_shared_ptr<uint32_t> m_tilemap50_regs_base;
	required_shared_ptr<uint32_t> m_fullscreenzoom;
	required_shared_ptr<uint32_t> m_0xc0000000_ram;
	required_shared_ptr<uint32_t> m_decrypted_gamerom;
	required_shared_ptr<uint32_t> m_0xc0000000_ram_decrypted;

	optional_memory_region      m_user4_region;
	optional_memory_region      m_user5_region;

	fujitsu_29f016a_device *m_simm[7][8];
	uint32_t m_cram_gfxflash_bank;
	std::unique_ptr<uint32_t[]> m_char_ram;
	std::unique_ptr<uint32_t[]> m_eeprom;
	uint32_t m_ss_pal_base;
	uint32_t m_unk_vidregs[0x20/4];
	uint32_t m_ss_bank_base;
	uint32_t m_screenwidth;
	std::unique_ptr<uint32_t[]> m_mame_colours;
	bitmap_rgb32 m_renderbuffer_bitmap;
	rectangle m_renderbuffer_clip;
	uint8_t* m_user4;
	uint32_t m_key1;
	uint32_t m_key2;
	int m_altEncryption;
	std::unique_ptr<uint32_t[]> m_ss_ram;
	uint32_t m_cram_bank;
	uint16_t m_current_eeprom_read;
	uint32_t m_paldma_source;
	uint32_t m_paldma_realsource;
	uint32_t m_paldma_dest;
	uint32_t m_paldma_fade;
	uint32_t m_paldma_other2;
	uint32_t m_paldma_length;
	uint32_t m_chardma_source;
	uint32_t m_chardma_other;
	uint32_t m_current_table_address;
	int m_rle_length;
	int m_last_normal_byte;
	unsigned short m_lastb;
	unsigned short m_lastb2;
	uint8_t* m_user5;

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
	uint32_t screen_update_cps3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cps3_vbl_interrupt);
	INTERRUPT_GEN_MEMBER(cps3_other_interrupt);
	uint16_t rotate_left(uint16_t value, int n);
	uint16_t rotxor(uint16_t val, uint16_t xorval);
	uint32_t cps3_mask(uint32_t address, uint32_t key1, uint32_t key2);
	void cps3_decrypt_bios();
	void init_common(void);
	void init_crypt(uint32_t key1, uint32_t key2, int altEncryption);
	void cps3_set_mame_colours(int colournum, uint16_t data, uint32_t fadeval);
	void cps3_draw_tilemapsprite_line(int tmnum, int drawline, bitmap_rgb32 &bitmap, const rectangle &cliprect );
	uint32_t cps3_flashmain_r(address_space &space, int which, uint32_t offset, uint32_t mem_mask);
	void cps3_flashmain_w(int which, uint32_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t process_byte( uint8_t real_byte, uint32_t destination, int max_length );
	void cps3_do_char_dma( uint32_t real_source, uint32_t real_destination, uint32_t real_length );
	uint32_t ProcessByte8(uint8_t b,uint32_t dst_offset);
	void cps3_do_alt_char_dma( uint32_t src, uint32_t real_dest, uint32_t real_length );
	void cps3_process_character_dma(uint32_t address);
	void copy_from_nvram();
	inline void cps3_drawgfxzoom(bitmap_rgb32 &dest_bmp, const rectangle &clip, gfx_element *gfx,
		unsigned int code, unsigned int color, int flipx, int flipy, int sx, int sy,
		int transparency, int transparent_color,
		int scalex, int scaley, bitmap_ind8 *pri_buffer, uint32_t pri_mask);
	void cps3(machine_config &config);
	void jojo(machine_config &config);
	void redearth(machine_config &config);
	void sfiii2(machine_config &config);
	void sfiii3(machine_config &config);
	void sfiii(machine_config &config);
	void jojoba(machine_config &config);
	void simm1_64mbit(machine_config &config);
	void simm2_64mbit(machine_config &config);
	void simm3_128mbit(machine_config &config);
	void simm4_128mbit(machine_config &config);
	void simm5_128mbit(machine_config &config);
	void simm5_32mbit(machine_config &config);
	void simm6_128mbit(machine_config &config);
	void cps3_map(address_map &map);
	void decrypted_opcodes_map(address_map &map);
};
