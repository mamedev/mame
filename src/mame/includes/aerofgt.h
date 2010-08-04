
class aerofgt_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, aerofgt_state(machine)); }

	aerofgt_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT16 *  bg1videoram;
	UINT16 *  bg2videoram;
	UINT16 *  rasterram;
	UINT16 *  bitmapram;
	UINT16 *  spriteram1;
	UINT16 *  spriteram2;
	UINT16 *  spriteram3;
	UINT16 *  tx_tilemap_ram;
//  UINT16 *  paletteram;   // currently this uses generic palette handling
	size_t    spriteram1_size;
	size_t    spriteram2_size;
	size_t    spriteram3_size;

	/* video-related */
	tilemap_t   *bg1_tilemap, *bg2_tilemap;
	UINT8     gfxbank[8];
	UINT16    bank[4];
	UINT16    bg1scrollx, bg1scrolly, bg2scrollx, bg2scrolly, wbbc97_bitmap_enable;
	int       charpalettebank, spritepalettebank;
	int       sprite_gfx;
	int       spikes91_lookup;

	/* misc */
	int       pending_command;

	/* devices */
	running_device *audiocpu;
};


/*----------- defined in video/aerofgt.c -----------*/


WRITE16_HANDLER( aerofgt_bg1videoram_w );
WRITE16_HANDLER( aerofgt_bg2videoram_w );
WRITE16_HANDLER( pspikes_gfxbank_w );
WRITE16_HANDLER( pspikesb_gfxbank_w );
WRITE16_HANDLER( spikes91_lookup_w );
WRITE16_HANDLER( karatblz_gfxbank_w );
WRITE16_HANDLER( spinlbrk_gfxbank_w );
WRITE16_HANDLER( turbofrc_gfxbank_w );
WRITE16_HANDLER( aerofgt_gfxbank_w );
WRITE16_HANDLER( aerofgt_bg1scrollx_w );
WRITE16_HANDLER( aerofgt_bg1scrolly_w );
WRITE16_HANDLER( aerofgt_bg2scrollx_w );
WRITE16_HANDLER( aerofgt_bg2scrolly_w );
WRITE16_HANDLER( pspikes_palette_bank_w );
WRITE16_HANDLER( wbbc97_bitmap_enable_w );

VIDEO_START( pspikes );
VIDEO_START( karatblz );
VIDEO_START( spinlbrk );
VIDEO_START( turbofrc );
VIDEO_START( wbbc97 );
VIDEO_UPDATE( pspikes );
VIDEO_UPDATE( pspikesb );
VIDEO_UPDATE( spikes91 );
VIDEO_UPDATE( karatblz );
VIDEO_UPDATE( spinlbrk );
VIDEO_UPDATE( turbofrc );
VIDEO_UPDATE( aerofgt );
VIDEO_UPDATE( aerfboot );
VIDEO_UPDATE( aerfboo2 );
VIDEO_UPDATE( wbbc97 );
