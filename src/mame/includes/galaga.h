#include "sound/discrete.h"

/*----------- defined in video/bosco.c -----------*/

extern UINT8 *bosco_videoram;
extern UINT8 *bosco_radarattr;
READ8_HANDLER( bosco_videoram_r );
WRITE8_HANDLER( bosco_videoram_w );
WRITE8_HANDLER( bosco_scrollx_w );
WRITE8_HANDLER( bosco_scrolly_w );
WRITE8_HANDLER( bosco_starcontrol_w );
WRITE8_HANDLER( bosco_starclr_w );
WRITE8_HANDLER( bosco_starblink_w );
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

extern UINT8 *galaga_videoram;
extern UINT8 *galaga_ram1,*galaga_ram2,*galaga_ram3;
extern UINT8 galaga_starcontrol[];
extern const struct star star_seed_tab[];

PALETTE_INIT( galaga );
READ8_HANDLER( galaga_videoram_r );
WRITE8_HANDLER( galaga_videoram_w );
WRITE8_HANDLER( galaga_starcontrol_w );
WRITE8_HANDLER( gatsbee_bank_w );
VIDEO_START( galaga );
VIDEO_UPDATE( galaga );
VIDEO_EOF( galaga );	/* update starfield */

/*----------- defined in video/xevious.c -----------*/

extern UINT8 *xevious_fg_videoram,*xevious_fg_colorram;
extern UINT8 *xevious_bg_videoram,*xevious_bg_colorram;
extern UINT8 *xevious_sr1,*xevious_sr2,*xevious_sr3;

READ8_HANDLER( xevious_fg_videoram_r );
READ8_HANDLER( xevious_fg_colorram_r );
READ8_HANDLER( xevious_bg_videoram_r );
READ8_HANDLER( xevious_bg_colorram_r );
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

extern UINT8 *digdug_videoram,*digdug_objram, *digdug_posram, *digdug_flpram;

READ8_HANDLER( digdug_videoram_r );
WRITE8_HANDLER( digdug_videoram_w );
WRITE8_HANDLER( digdug_PORT_w );
VIDEO_START( digdug );
VIDEO_UPDATE( digdug );
PALETTE_INIT( digdug );
