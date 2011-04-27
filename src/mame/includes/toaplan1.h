/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
****************************************************************************/


class toaplan1_state : public driver_device
{
public:
	toaplan1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_unk_reset_port;
	UINT16 *m_colorram1;
	UINT16 *m_colorram2;
	size_t m_colorram1_size;
	size_t m_colorram2_size;

	UINT8 *m_sharedram;

	int m_coin_count; /* coin count increments on startup ? , so dont count it */
	int m_intenable;

	/* Demon world */
	int m_dsp_on;
	int m_dsp_BIO;
	int m_dsp_execute;
	UINT32 m_dsp_addr_w;
	UINT32 m_main_ram_seg;

	UINT8 m_vimana_coins[2];
	UINT8 m_vimana_credits;
	UINT8 m_vimana_latch;

	UINT16 *m_pf4_tilevram16;	/*  ||  Drawn in this order */
	UINT16 *m_pf3_tilevram16;	/*  ||  */
	UINT16 *m_pf2_tilevram16;	/* \||/ */
	UINT16 *m_pf1_tilevram16;	/*  \/  */

	size_t m_spriteram_size;
	UINT16 *m_spriteram;
	UINT16 *m_buffered_spriteram;
	UINT16 *m_spritesizeram16;
	UINT16 *m_buffered_spritesizeram16;

	INT32 m_bcu_flipscreen;		/* Tile   controller flip flag */
	INT32 m_fcu_flipscreen;		/* Sprite controller flip flag */

	INT32 m_pf_voffs;
	INT32 m_spriteram_offs;

	INT32 m_pf1_scrollx;
	INT32 m_pf1_scrolly;
	INT32 m_pf2_scrollx;
	INT32 m_pf2_scrolly;
	INT32 m_pf3_scrollx;
	INT32 m_pf3_scrolly;
	INT32 m_pf4_scrollx;
	INT32 m_pf4_scrolly;
	INT32 m_scrollx_offs1;
	INT32 m_scrollx_offs2;
	INT32 m_scrollx_offs3;
	INT32 m_scrollx_offs4;
	INT32 m_scrolly_offs;


#ifdef MAME_DEBUG
	int m_display_pf1;
	int m_display_pf2;
	int m_display_pf3;
	int m_display_pf4;
	int m_displog;
#endif

	INT32 m_tiles_offsetx;
	INT32 m_tiles_offsety;

	int m_reset;		/* Hack! See toaplan1_bcu_control below */

	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_pf2_tilemap;
	tilemap_t *m_pf3_tilemap;
	tilemap_t *m_pf4_tilemap;

	// an empty tile, so that we can safely disable tiles
	UINT8        m_empty_tile[8*8];
};


/*----------- defined in machine/toaplan1.c -----------*/

INTERRUPT_GEN( toaplan1_interrupt );
WRITE16_HANDLER( toaplan1_intenable_w );
READ16_HANDLER ( toaplan1_shared_r );
WRITE16_HANDLER( toaplan1_shared_w );
WRITE16_HANDLER( toaplan1_reset_sound );
WRITE16_HANDLER( demonwld_dsp_addrsel_w );
READ16_HANDLER ( demonwld_dsp_r );
WRITE16_HANDLER( demonwld_dsp_w );
WRITE16_HANDLER( demonwld_dsp_bio_w );
WRITE16_HANDLER( demonwld_dsp_ctrl_w );
READ16_HANDLER ( demonwld_BIO_r );
READ16_HANDLER ( samesame_port_6_word_r );
READ16_HANDLER ( vimana_system_port_r );
READ16_HANDLER ( vimana_mcu_r );
WRITE16_HANDLER( vimana_mcu_w );

WRITE8_HANDLER( rallybik_coin_w );
WRITE8_HANDLER( toaplan1_coin_w );
WRITE16_HANDLER( samesame_coin_w );

MACHINE_RESET( toaplan1 );
MACHINE_RESET( demonwld );
MACHINE_RESET( vimana );
MACHINE_RESET( zerowing );	/* hack for ZeroWing/OutZone. See video */

void toaplan1_driver_savestate(running_machine &machine);
void demonwld_driver_savestate(running_machine &machine);
void vimana_driver_savestate(running_machine &machine);


/*----------- defined in video/toaplan1.c -----------*/

READ16_HANDLER ( toaplan1_frame_done_r );
WRITE16_HANDLER( toaplan1_bcu_control_w );
WRITE16_HANDLER( rallybik_bcu_flipscreen_w );
WRITE16_HANDLER( toaplan1_bcu_flipscreen_w );
WRITE16_HANDLER( toaplan1_fcu_flipscreen_w );

READ16_HANDLER ( rallybik_tileram16_r );
READ16_HANDLER ( toaplan1_tileram16_r );
WRITE16_HANDLER( toaplan1_tileram16_w );
READ16_HANDLER ( toaplan1_spriteram16_r );
WRITE16_HANDLER( toaplan1_spriteram16_w );
READ16_HANDLER ( toaplan1_spritesizeram16_r );
WRITE16_HANDLER( toaplan1_spritesizeram16_w );
READ16_HANDLER ( toaplan1_colorram1_r );
WRITE16_HANDLER( toaplan1_colorram1_w );
READ16_HANDLER ( toaplan1_colorram2_r );
WRITE16_HANDLER( toaplan1_colorram2_w );

READ16_HANDLER ( toaplan1_scroll_regs_r );
WRITE16_HANDLER( toaplan1_scroll_regs_w );
WRITE16_HANDLER( toaplan1_tile_offsets_w );
READ16_HANDLER ( toaplan1_tileram_offs_r );
WRITE16_HANDLER( toaplan1_tileram_offs_w );
READ16_HANDLER ( toaplan1_spriteram_offs_r );
WRITE16_HANDLER( toaplan1_spriteram_offs_w );

SCREEN_EOF( rallybik );
SCREEN_EOF( toaplan1 );
SCREEN_EOF( samesame );
VIDEO_START( rallybik );
VIDEO_START( toaplan1 );
SCREEN_UPDATE( rallybik );
SCREEN_UPDATE( toaplan1 );
