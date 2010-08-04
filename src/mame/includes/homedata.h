
class homedata_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, homedata_state(machine)); }

	homedata_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *  vreg;
	UINT8 *  videoram;

	/* video-related */
	tilemap_t *bg_tilemap[2][4];
	int      visible_page;
	int      priority;
	UINT8    reikaids_which;
	int      flipscreen;
	UINT8	   gfx_bank[2];	// pteacher only uses the first one
	UINT8	   blitter_bank;
	int      blitter_param_count;
	UINT8	   blitter_param[4];		/* buffers last 4 writes to 0x8006 */


	/* misc */
	int      vblank;
	int      sndbank;
	int      keyb;
	int      snd_command;
	int      upd7807_porta, upd7807_portc;
	int      to_cpu, from_cpu;

	/* device */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *dac;
	running_device *ym;
	running_device *sn;
};



/*----------- defined in video/homedata.c -----------*/

WRITE8_HANDLER( mrokumei_videoram_w );
WRITE8_HANDLER( reikaids_videoram_w );
WRITE8_HANDLER( reikaids_gfx_bank_w );
WRITE8_HANDLER( pteacher_gfx_bank_w );
WRITE8_HANDLER( homedata_blitter_param_w );
WRITE8_HANDLER( mrokumei_blitter_bank_w );
WRITE8_HANDLER( reikaids_blitter_bank_w );
WRITE8_HANDLER( pteacher_blitter_bank_w );
WRITE8_HANDLER( mrokumei_blitter_start_w );
WRITE8_HANDLER( reikaids_blitter_start_w );
WRITE8_HANDLER( pteacher_blitter_start_w );

PALETTE_INIT( mrokumei );
PALETTE_INIT( reikaids );
PALETTE_INIT( pteacher );
PALETTE_INIT( mirderby );

VIDEO_START( mrokumei );
VIDEO_START( reikaids );
VIDEO_START( pteacher );
VIDEO_START( lemnangl );
VIDEO_START( mirderby );
VIDEO_UPDATE( mrokumei );
VIDEO_UPDATE( reikaids );
VIDEO_UPDATE( pteacher );
VIDEO_UPDATE( mirderby );
VIDEO_EOF( homedata );
