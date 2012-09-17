/***************************************************************************

    tecmosys protection simulation

***************************************************************************/

class tecmosys_state : public driver_device
{
public:
	tecmosys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
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
		m_880000regs(*this, "880000regs"){ }

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
	DECLARE_WRITE8_MEMBER(tecmosys_z80_bank_w);
	DECLARE_WRITE8_MEMBER(tecmosys_oki_bank_w);
	DECLARE_READ16_MEMBER(tecmosys_prot_status_r);
	DECLARE_WRITE16_MEMBER(tecmosys_prot_status_w);
	DECLARE_READ16_MEMBER(tecmosys_prot_data_r);
	DECLARE_WRITE16_MEMBER(tecmosys_prot_data_w);
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
	DECLARE_DRIVER_INIT(tkdensha);
	DECLARE_DRIVER_INIT(deroon);
	DECLARE_DRIVER_INIT(tkdensho);
	TILE_GET_INFO_MEMBER(get_bg0tile_info);
	TILE_GET_INFO_MEMBER(get_bg1tile_info);
	TILE_GET_INFO_MEMBER(get_bg2tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void video_start();
	UINT32 screen_update_tecmosys(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/tecmosys.c -----------*/

void tecmosys_prot_init(running_machine &machine, int which);



/*----------- defined in video/tecmosys.c -----------*/




