/*************************************************************************

    SNK/Alpha 68000 based games

*************************************************************************/

class alpha68k_state : public driver_device
{
public:
	alpha68k_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    videoram;
	UINT16 *    spriteram;
	UINT16 *    shared_ram;
	UINT16 *    paletteram;

	/* video-related */
	tilemap_t     *fix_tilemap;
	int         bank_base, flipscreen, last_bank;
	int         buffer_28, buffer_60, buffer_68;

	/* misc */
	int         invert_controls;
	int         microcontroller_id;
	int         coin_id;
	unsigned    trigstate, deposits1, deposits2, credits;
	unsigned    coinvalue;
	unsigned    microcontroller_data;
	int         latch;
	unsigned    game_id;	// see below

	/* devices */
	device_t *audiocpu;
};

/* game_id - used to deal with a few game specific situations */
enum
{
	ALPHA68K_BTLFIELDB = 1,		// used in alpha_II_trigger_r
	ALPHA68K_JONGBOU,			// used in kyros_alpha_trigger_r & kyros_draw_sprites
	ALPHA68K_KYROS			// used in kyros_draw_sprites
};


/*----------- defined in video/alpha68k.c -----------*/

PALETTE_INIT( kyros );
PALETTE_INIT( paddlem );

VIDEO_START( alpha68k );

VIDEO_UPDATE( kyros );
VIDEO_UPDATE( sstingry );
VIDEO_UPDATE( alpha68k_I );
VIDEO_UPDATE( alpha68k_II );
VIDEO_UPDATE( alpha68k_V );
VIDEO_UPDATE( alpha68k_V_sb );

void alpha68k_V_video_bank_w(running_machine *machine, int bank);
void alpha68k_flipscreen_w(running_machine *machine, int flip);

WRITE16_HANDLER( alpha68k_paletteram_w );
WRITE16_HANDLER( alpha68k_videoram_w );
WRITE16_HANDLER( alpha68k_II_video_bank_w );
WRITE16_HANDLER( alpha68k_V_video_control_w );
