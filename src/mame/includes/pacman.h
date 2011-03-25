/*************************************************************************

    Namco PuckMan

**************************************************************************/

class pacman_state : public driver_device
{
public:
	pacman_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 cannonb_bit_to_read;
	int mystery;
	UINT8 counter;
	int bigbucks_bank;
	UINT8 *rocktrv2_prot_data;
	UINT8 rocktrv2_question_bank;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *s2650games_spriteram;
	UINT8 *s2650games_tileram;
	tilemap_t *bg_tilemap;
	UINT8 charbank;
	UINT8 spritebank;
	UINT8 palettebank;
	UINT8 colortablebank;
	UINT8 flipscreen;
	UINT8 bgpriority;
	int xoffsethack;
};


/*----------- defined in video/pacman.c -----------*/

PALETTE_INIT( pacman );
VIDEO_START( pacman );
SCREEN_UPDATE( pacman );

WRITE8_HANDLER( pacman_videoram_w );
WRITE8_HANDLER( pacman_colorram_w );
WRITE8_HANDLER( pacman_flipscreen_w );


VIDEO_START( pengo );

WRITE8_HANDLER( pengo_palettebank_w );
WRITE8_HANDLER( pengo_colortablebank_w );
WRITE8_HANDLER( pengo_gfxbank_w );


VIDEO_START( s2650games );
SCREEN_UPDATE( s2650games );


WRITE8_HANDLER( s2650games_videoram_w );
WRITE8_HANDLER( s2650games_colorram_w );
WRITE8_HANDLER( s2650games_scroll_w );
WRITE8_HANDLER( s2650games_tilesbank_w );


VIDEO_START( jrpacman );

WRITE8_HANDLER( jrpacman_videoram_w );
WRITE8_HANDLER( jrpacman_charbank_w );
WRITE8_HANDLER( jrpacman_spritebank_w );
WRITE8_HANDLER( jrpacman_scroll_w );
WRITE8_HANDLER( jrpacman_bgpriority_w );


/*----------- defined in machine/pacplus.c -----------*/

void pacplus_decode(running_machine *machine);


/*----------- defined in machine/jumpshot.c -----------*/

void jumpshot_decode(running_machine *machine);


/*----------- defined in machine/theglobp.c -----------*/

MACHINE_START( theglobp );
MACHINE_RESET( theglobp );
READ8_HANDLER( theglobp_decrypt_rom );


/*----------- defined in machine/acitya.c -------------*/

MACHINE_START( acitya );
MACHINE_RESET( acitya );
READ8_HANDLER( acitya_decrypt_rom );
