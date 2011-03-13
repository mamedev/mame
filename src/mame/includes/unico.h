class unico_state : public driver_device
{
public:
	unico_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *vram;
	UINT16 *scroll;
	UINT32 *vram32;
	UINT32 *scroll32;
	tilemap_t *tilemap[3];
	int sprites_scrolldx;
	int sprites_scrolldy;
	UINT16 *spriteram;
	size_t spriteram_size;
};


/*----------- defined in video/unico.c -----------*/

WRITE16_HANDLER( unico_vram_w );
WRITE16_HANDLER( unico_palette_w );

WRITE32_HANDLER( unico_vram32_w );
WRITE32_HANDLER( unico_palette32_w );

VIDEO_START( unico );
SCREEN_UPDATE( unico );

VIDEO_START( zeropnt2 );
SCREEN_UPDATE( zeropnt2 );

