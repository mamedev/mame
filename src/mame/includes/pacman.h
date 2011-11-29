/*************************************************************************

    Namco PuckMan

**************************************************************************/

class pacman_state : public driver_device
{
public:
	pacman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_cannonb_bit_to_read;
	int m_mystery;
	UINT8 m_counter;
	int m_bigbucks_bank;
	UINT8 *m_rocktrv2_prot_data;
	UINT8 m_rocktrv2_question_bank;
	UINT8 *m_videoram;
	UINT8 *m_colorram;
	UINT8 *m_s2650games_spriteram;
	UINT8 *m_s2650games_tileram;
	tilemap_t *m_bg_tilemap;
	UINT8 m_charbank;
	UINT8 m_spritebank;
	UINT8 m_palettebank;
	UINT8 m_colortablebank;
	UINT8 m_flipscreen;
	UINT8 m_bgpriority;
	int m_xoffsethack;
	UINT8 m_inv_spr;
	UINT8 m_irq_mask;
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

VIDEO_START( birdiy );


/*----------- defined in machine/pacplus.c -----------*/

void pacplus_decode(running_machine &machine);


/*----------- defined in machine/jumpshot.c -----------*/

void jumpshot_decode(running_machine &machine);


/*----------- defined in machine/theglobp.c -----------*/

MACHINE_START( theglobp );
MACHINE_RESET( theglobp );
READ8_HANDLER( theglobp_decrypt_rom );


/*----------- defined in machine/acitya.c -------------*/

MACHINE_START( acitya );
MACHINE_RESET( acitya );
READ8_HANDLER( acitya_decrypt_rom );
