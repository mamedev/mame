class vball_state : public driver_device
{
public:
	vball_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *vb_attribram;
	UINT8 *vb_videoram;
	UINT8 *vb_scrolly_lo;
	int vb_scrollx_hi;
	int vb_scrolly_hi;
	int vb_scrollx_lo;
	int gfxset;
	int vb_scrollx[256];
	int vb_bgprombank;
	int vb_spprombank;
	tilemap_t *bg_tilemap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/vball.c -----------*/

VIDEO_START( vb );
SCREEN_UPDATE( vb );
void vb_bgprombank_w(running_machine &machine, int bank);
void vb_spprombank_w(running_machine &machine, int bank);
WRITE8_HANDLER( vb_attrib_w );
WRITE8_HANDLER( vb_videoram_w );
void vb_mark_all_dirty(running_machine &machine);
