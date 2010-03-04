class welltris_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, welltris_state(machine)); }

	welltris_state(running_machine &machine) { }

	int pending_command;

	UINT16 *spriteram;
	UINT16 *pixelram;
	UINT16 *charvideoram;

	tilemap_t *char_tilemap;
	UINT8 gfxbank[8];
	UINT16 charpalettebank;
	UINT16 spritepalettebank;
	UINT16 pixelpalettebank;
	int scrollx;
	int scrolly;
};


/*----------- defined in video/welltris.c -----------*/

//READ16_HANDLER( welltris_spriteram_r );
WRITE16_HANDLER( welltris_spriteram_w );
WRITE16_HANDLER( welltris_palette_bank_w );
WRITE16_HANDLER( welltris_gfxbank_w );
WRITE16_HANDLER( welltris_charvideoram_w );
WRITE16_HANDLER( welltris_scrollreg_w );

VIDEO_START( welltris );
VIDEO_UPDATE( welltris );
