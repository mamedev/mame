#include "sound/discrete.h"

class _galaga_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, _galaga_state(machine)); }

	_galaga_state(running_machine &machine)
		: driver_data_t(machine)
	{
		xevious_bs[0] = 0;
		xevious_bs[1] = 0;
	}

	/* memory pointers */
	UINT8 *galaga_ram1,*galaga_ram2,*galaga_ram3;
	UINT8 *galaga_starcontrol;	// 6 addresses

	/* machine state */
	UINT32 stars_scrollx;
	UINT32 stars_scrolly;

	UINT32 galaga_gfxbank; // used by catsbee

	/* devices */

	/* bank support */

	/* shared */
	UINT8 *videoram;
	tilemap_t *fg_tilemap;
	tilemap_t *bg_tilemap;

	/* xevious */

	UINT8 *xevious_fg_videoram;
	UINT8 *xevious_fg_colorram;
	UINT8 *xevious_bg_videoram;
	UINT8 *xevious_bg_colorram;
	UINT8 *xevious_sr1;
	UINT8 *xevious_sr2;
	UINT8 *xevious_sr3;

	/* xevious static */
	INT32 xevious_bs[2];

	/* bosco */

	UINT8 *bosco_radarattr;

	/* bosco static */

	UINT8 *bosco_starcontrol;
	UINT8 *bosco_starblink;

	UINT8 *bosco_radarx;
	UINT8 *bosco_radary;

	/* digdug */

	UINT8 *digdug_objram;
	UINT8 *digdug_posram;
	UINT8 *digdug_flpram;

	/* digdug static*/

	UINT8 bg_select;
	UINT8 tx_color_mode;
	UINT8 bg_disable;
	UINT8 bg_color_bank;

};



/*----------- defined in video/bosco.c -----------*/

WRITE8_HANDLER( bosco_videoram_w );
WRITE8_HANDLER( bosco_scrollx_w );
WRITE8_HANDLER( bosco_scrolly_w );
WRITE8_HANDLER( bosco_starclr_w );
VIDEO_START( bosco );
VIDEO_UPDATE( bosco );
PALETTE_INIT( bosco );
VIDEO_EOF( bosco );	/* update starfield */

/*----------- defined in audio/galaga.c -----------*/

DISCRETE_SOUND_EXTERN( bosco );
DISCRETE_SOUND_EXTERN( galaga );


/*----------- defined in video/galaga.c -----------*/

struct star
{
	UINT16 x,y;
	UINT8 col,set;
};

extern const struct star star_seed_tab[];

PALETTE_INIT( galaga );
WRITE8_HANDLER( galaga_videoram_w );
WRITE8_HANDLER( gatsbee_bank_w );
VIDEO_START( galaga );
VIDEO_UPDATE( galaga );
VIDEO_EOF( galaga );	/* update starfield */

/*----------- defined in video/xevious.c -----------*/

WRITE8_HANDLER( xevious_fg_videoram_w );
WRITE8_HANDLER( xevious_fg_colorram_w );
WRITE8_HANDLER( xevious_bg_videoram_w );
WRITE8_HANDLER( xevious_bg_colorram_w );
WRITE8_HANDLER( xevious_vh_latch_w );
WRITE8_HANDLER( xevious_bs_w );
READ8_HANDLER( xevious_bb_r );
VIDEO_START( xevious );
PALETTE_INIT( xevious );
VIDEO_UPDATE( xevious );

PALETTE_INIT( battles );

/*----------- defined in machine/xevious.c -----------*/

void battles_customio_init(running_machine *machine);
TIMER_DEVICE_CALLBACK( battles_nmi_generate );

READ8_HANDLER( battles_customio0_r );
READ8_HANDLER( battles_customio_data0_r );
READ8_HANDLER( battles_customio3_r );
READ8_HANDLER( battles_customio_data3_r );
READ8_HANDLER( battles_input_port_r );

WRITE8_HANDLER( battles_customio0_w );
WRITE8_HANDLER( battles_customio_data0_w );
WRITE8_HANDLER( battles_customio3_w );
WRITE8_HANDLER( battles_customio_data3_w );
WRITE8_HANDLER( battles_CPU4_coin_w );
WRITE8_HANDLER( battles_noise_sound_w );

INTERRUPT_GEN( battles_interrupt_4 );

/*----------- defined in video/digdug.c -----------*/

WRITE8_HANDLER( digdug_videoram_w );
WRITE8_HANDLER( digdug_PORT_w );
VIDEO_START( digdug );
VIDEO_UPDATE( digdug );
PALETTE_INIT( digdug );
