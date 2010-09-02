/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
****************************************************************************/


class toaplan1_state : public driver_device
{
public:
	toaplan1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int unk_reset_port;
	UINT16 *colorram1;
	UINT16 *colorram2;
	size_t colorram1_size;
	size_t colorram2_size;

	UINT8 *sharedram;

	int coin_count; /* coin count increments on startup ? , so dont count it */
	int intenable;

	/* Demon world */
	int dsp_on;
	int dsp_BIO;
	int dsp_execute;
	UINT32 dsp_addr_w;
	UINT32 main_ram_seg;

	UINT8 vimana_coins[2];
	UINT8 vimana_credits;
	UINT8 vimana_latch;

	UINT16 *pf4_tilevram16;	/*  ||  Drawn in this order */
	UINT16 *pf3_tilevram16;	/*  ||  */
	UINT16 *pf2_tilevram16;	/* \||/ */
	UINT16 *pf1_tilevram16;	/*  \/  */

	size_t spriteram_size;
	UINT16 *spriteram;
	UINT16 *buffered_spriteram;
	UINT16 *spritesizeram16;
	UINT16 *buffered_spritesizeram16;

	INT32 bcu_flipscreen;		/* Tile   controller flip flag */
	INT32 fcu_flipscreen;		/* Sprite controller flip flag */

	INT32 pf_voffs;
	INT32 spriteram_offs;

	INT32 pf1_scrollx;
	INT32 pf1_scrolly;
	INT32 pf2_scrollx;
	INT32 pf2_scrolly;
	INT32 pf3_scrollx;
	INT32 pf3_scrolly;
	INT32 pf4_scrollx;
	INT32 pf4_scrolly;
	INT32 scrollx_offs1;
	INT32 scrollx_offs2;
	INT32 scrollx_offs3;
	INT32 scrollx_offs4;
	INT32 scrolly_offs;


#ifdef MAME_DEBUG
	int display_pf1;
	int display_pf2;
	int display_pf3;
	int display_pf4;
	int displog;
#endif

	INT32 tiles_offsetx;
	INT32 tiles_offsety;

	int reset;		/* Hack! See toaplan1_bcu_control below */

	tilemap_t *pf1_tilemap, *pf2_tilemap, *pf3_tilemap, *pf4_tilemap;

	// an empty tile, so that we can safely disable tiles
	UINT8        empty_tile[8*8];
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

void toaplan1_driver_savestate(running_machine *machine);
void demonwld_driver_savestate(running_machine *machine);
void vimana_driver_savestate(running_machine *machine);


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

VIDEO_EOF( rallybik );
VIDEO_EOF( toaplan1 );
VIDEO_EOF( samesame );
VIDEO_START( rallybik );
VIDEO_START( toaplan1 );
VIDEO_UPDATE( rallybik );
VIDEO_UPDATE( toaplan1 );
