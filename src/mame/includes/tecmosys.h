// license:BSD-3-Clause
// copyright-holders:Farfetch'd,David Haywood,Tomasz Slanina
/***************************************************************************

    tecmosys protection simulation

***************************************************************************/
#include "machine/eepromser.h"

class tecmosys_state : public driver_device
{
public:
	tecmosys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_tilemap_paletteram16(*this, "tmap_palette"),
		m_bg2tilemap_ram(*this, "bg2tilemap_ram"),
		m_bg1tilemap_ram(*this, "bg1tilemap_ram"),
		m_bg0tilemap_ram(*this, "bg0tilemap_ram"),
		m_fgtilemap_ram(*this, "fgtilemap_ram"),
		m_bg0tilemap_lineram(*this, "bg0_lineram"),
		m_bg1tilemap_lineram(*this, "bg1_lineram"),
		m_bg2tilemap_lineram(*this, "bg2_lineram"),
		m_a80000regs(*this, "a80000regs"),
		m_b00000regs(*this, "b00000regs"),
		m_c00000regs(*this, "c00000regs"),
		m_c80000regs(*this, "c80000regs"),
		m_880000regs(*this, "880000regs") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_tilemap_paletteram16;
	required_shared_ptr<UINT16> m_bg2tilemap_ram;
	required_shared_ptr<UINT16> m_bg1tilemap_ram;
	required_shared_ptr<UINT16> m_bg0tilemap_ram;
	required_shared_ptr<UINT16> m_fgtilemap_ram;
	required_shared_ptr<UINT16> m_bg0tilemap_lineram;
	required_shared_ptr<UINT16> m_bg1tilemap_lineram;
	required_shared_ptr<UINT16> m_bg2tilemap_lineram;
	required_shared_ptr<UINT16> m_a80000regs;
	required_shared_ptr<UINT16> m_b00000regs;
	required_shared_ptr<UINT16> m_c00000regs;
	required_shared_ptr<UINT16> m_c80000regs;
	required_shared_ptr<UINT16> m_880000regs;

	int m_spritelist;
	bitmap_ind16 m_sprite_bitmap;
	bitmap_ind16 m_tmp_tilemap_composebitmap;
	bitmap_ind16 m_tmp_tilemap_renderbitmap;
	tilemap_t *m_bg0tilemap;
	tilemap_t *m_bg1tilemap;
	tilemap_t *m_bg2tilemap;
	tilemap_t *m_txt_tilemap;
	UINT8 m_device_read_ptr;
	UINT8 m_device_status;
	const struct prot_data* m_device_data;
	UINT8 m_device_value;

	DECLARE_READ16_MEMBER(sound_r);
	DECLARE_WRITE16_MEMBER(sound_w);
	DECLARE_WRITE16_MEMBER(unk880000_w);
	DECLARE_READ16_MEMBER(unk880000_r);
	DECLARE_WRITE8_MEMBER(z80_bank_w);
	DECLARE_WRITE8_MEMBER(oki_bank_w);
	DECLARE_READ16_MEMBER(prot_status_r);
	DECLARE_WRITE16_MEMBER(prot_status_w);
	DECLARE_READ16_MEMBER(prot_data_r);
	DECLARE_WRITE16_MEMBER(prot_data_w);
	DECLARE_WRITE16_MEMBER(bg0_tilemap_w);
	DECLARE_WRITE16_MEMBER(bg1_tilemap_w);
	DECLARE_WRITE16_MEMBER(bg2_tilemap_w);
	DECLARE_WRITE16_MEMBER(fg_tilemap_w);
	DECLARE_WRITE16_MEMBER(tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w);
	DECLARE_WRITE16_MEMBER(bg0_tilemap_lineram_w);
	DECLARE_WRITE16_MEMBER(bg1_tilemap_lineram_w);
	DECLARE_WRITE16_MEMBER(bg2_tilemap_lineram_w);
	DECLARE_READ16_MEMBER(eeprom_r);
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_WRITE_LINE_MEMBER(sound_irq);

	DECLARE_DRIVER_INIT(tkdensha);
	DECLARE_DRIVER_INIT(deroon);
	DECLARE_DRIVER_INIT(tkdensho);
	virtual void machine_start() override;
	virtual void video_start() override;

	TILE_GET_INFO_MEMBER(get_bg0tile_info);
	TILE_GET_INFO_MEMBER(get_bg1tile_info);
	TILE_GET_INFO_MEMBER(get_bg2tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prot_init(int which);
	void prot_reset();
	inline void set_color_555(pen_t color, int rshift, int gshift, int bshift, UINT16 data);
	void render_sprites_to_bitmap(bitmap_rgb32 &bitmap, UINT16 extrax, UINT16 extray );
	void tilemap_copy_to_compose(UINT16 pri);
	void do_final_mix(bitmap_rgb32 &bitmap);
	void descramble();
};
