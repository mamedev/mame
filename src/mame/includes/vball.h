class vball_state : public driver_device
{
public:
	vball_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_vb_attribram;
	UINT8 *m_vb_videoram;
	UINT8 *m_vb_scrolly_lo;
	int m_vb_scrollx_hi;
	int m_vb_scrolly_hi;
	int m_vb_scrollx_lo;
	int m_gfxset;
	int m_vb_scrollx[256];
	int m_vb_bgprombank;
	int m_vb_spprombank;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/vball.c -----------*/

VIDEO_START( vb );
SCREEN_UPDATE( vb );
void vb_bgprombank_w(running_machine &machine, int bank);
void vb_spprombank_w(running_machine &machine, int bank);
WRITE8_HANDLER( vb_attrib_w );
WRITE8_HANDLER( vb_videoram_w );
void vb_mark_all_dirty(running_machine &machine);
