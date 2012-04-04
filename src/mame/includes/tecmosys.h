/***************************************************************************

    tecmosys protection simulation

***************************************************************************/

class tecmosys_state : public driver_device
{
public:
	tecmosys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16* m_spriteram;
	UINT16* m_tilemap_paletteram16;
	UINT16* m_bg2tilemap_ram;
	UINT16* m_bg1tilemap_ram;
	UINT16* m_bg0tilemap_ram;
	UINT16* m_fgtilemap_ram;
	UINT16* m_bg0tilemap_lineram;
	UINT16* m_bg1tilemap_lineram;
	UINT16* m_bg2tilemap_lineram;
	UINT16* m_a80000regs;
	UINT16* m_b00000regs;
	UINT16* m_c00000regs;
	UINT16* m_c80000regs;
	UINT16* m_880000regs;
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
};


/*----------- defined in machine/tecmosys.c -----------*/

void tecmosys_prot_init(running_machine &machine, int which);

READ16_HANDLER(tecmosys_prot_status_r);
WRITE16_HANDLER(tecmosys_prot_status_w);
READ16_HANDLER(tecmosys_prot_data_r);
WRITE16_HANDLER(tecmosys_prot_data_w);


/*----------- defined in video/tecmosys.c -----------*/

WRITE16_HANDLER( bg0_tilemap_w );
WRITE16_HANDLER( bg1_tilemap_w );
WRITE16_HANDLER( bg2_tilemap_w );
WRITE16_HANDLER( fg_tilemap_w );
WRITE16_HANDLER( tilemap_paletteram16_xGGGGGRRRRRBBBBB_word_w );
WRITE16_HANDLER( bg0_tilemap_lineram_w );
WRITE16_HANDLER( bg1_tilemap_lineram_w );
WRITE16_HANDLER( bg2_tilemap_lineram_w );

SCREEN_UPDATE_RGB32(tecmosys);
VIDEO_START(tecmosys);
