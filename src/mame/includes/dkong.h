#include "sound/discrete.h"

/*
 * From the schematics:
 *
 * XTAL is 61,44 MHZ. There is some oscillator logic around it. The oscillating circuit
 * transfers the signal with a transformator. Onwards, it is fed through a M(B/C)10136. This
 * is a programmable counter which is used as a divisor by 5.
 * Cascaded 74LS161 further divide the signal. The following signals are generated:
 * 1/2H: 61,44MHZ/5/2 - pixel clock
 * 1H  : 61,44MHZ/5/4 - cpu-clock
 * 2H  : 61,44MHZ/5/8
 * ....
 * 128H: 61,44MHZ/5/512
 * The horizontal circuit counts till 384=256+128, thus 256H only being high for 128H/2
 *
 * Signal 16H,32H,64H and 256H are combined using a LS00, LS04 and a D-Flipflop to produce
 * a signal with Freq 16H/12. This is only possible because a 220pf capacitor with the
 * impedance of the LS-Family of 10K delays the 16H signal by about half a cycle.
 * This signal is divided by two by another D-Flipflop(74LS74) to give:
 * 1VF: 61,44MHZ/5/64/12/2 = 8KHZ
 * 2VF: 1VF/2 - Noise frequency: 4Khz
 * ...
 * The vertical circuit counts from 248 till 512 giving 264 lines.
 * 256VF is not being used, so counting is from 248...255, 0...255, ....
 */

#define MASTER_CLOCK		 	61440000
#define CLOCK_1H				(MASTER_CLOCK / 5 / 4)
#define CLOCK_16H				(CLOCK_1H / 16)
#define CLOCK_1VF				((CLOCK_16H) / 12 / 2)
#define CLOCK_2VF				((CLOCK_1VF) / 2)

#define PIXEL_CLOCK				(MASTER_CLOCK/10)
#define HTOTAL					(384)
#define HBSTART					(256)
#define HBEND					(0)
#define VTOTAL					(264)
#define VBSTART					(240)
#define VBEND					(16)

#define I8035_MASTER_CLOCK		(6000000)
#define I8035_DIVIDER			(15)
#define I8035_CLOCK				(I8035_MASTER_CLOCK/I8035_DIVIDER)


/*----------- defined in video/dkong.c -----------*/

WRITE8_HANDLER( radarscp_snd02_w ); /* to daisy chain sound 02 signal */
WRITE8_HANDLER( radarsc1_ansn_w ); /* to daisy chain sound 02 signal */

WRITE8_HANDLER( radarscp_grid_enable_w );
WRITE8_HANDLER( radarscp_grid_color_w );
WRITE8_HANDLER( dkong_flipscreen_w );
WRITE8_HANDLER( dkongjr_gfxbank_w );
WRITE8_HANDLER( dkong3_gfxbank_w );
WRITE8_HANDLER( dkong_spritebank_w );
WRITE8_HANDLER( dkong_palettebank_w );

WRITE8_HANDLER( dkong_videoram_w );

PALETTE_INIT( dkong );
PALETTE_INIT( radarscp );
PALETTE_INIT( radarsc1 );
PALETTE_INIT( dkong3 );

VIDEO_START( dkong );
VIDEO_START( radarsc1 );
VIDEO_START( radarscp );
VIDEO_UPDATE( radarscp );
VIDEO_UPDATE( dkong );
VIDEO_UPDATE( pestplce );
VIDEO_UPDATE( spclforc );



/*----------- defined in machine/strtheat.c -----------*/

DRIVER_INIT( strtheat );


/*----------- defined in machine/drakton.c -----------*/

DRIVER_INIT( drakton );


/*----------- defined in audio/dkong.c -----------*/

READ8_HANDLER( dkong_sh_p1_r );
READ8_HANDLER( dkong_sh_p2_r );
READ8_HANDLER( dkong_sh_t0_r );
READ8_HANDLER( dkong_sh_t1_r );
READ8_HANDLER( dkong_sh_tune_r );
READ8_HANDLER( dkong_in2_r );

WRITE8_HANDLER( radarsc1_snd_disc_w );
WRITE8_HANDLER( dkong_sh_w );
WRITE8_HANDLER( dkong_sh_p1_w );
WRITE8_HANDLER( dkong_sh_p2_w );

WRITE8_HANDLER( dkong_snd_disc_w );
WRITE8_HANDLER( dkong_sh_tuneselect_w );

WRITE8_HANDLER( dkongjr_sh_test6_w );
WRITE8_HANDLER( dkongjr_sh_test5_w );
WRITE8_HANDLER( dkongjr_sh_test4_w );
WRITE8_HANDLER( dkongjr_sh_tuneselect_w );

WRITE8_HANDLER( dkongjr_snd_w1 );
WRITE8_HANDLER( dkongjr_snd_w2 );

READ8_HANDLER( radarsc1_sh_tune_r );
READ8_HANDLER( radarsc1_sh_p1_r );
WRITE8_HANDLER( radarsc1_sh_p1_w );
WRITE8_HANDLER( radarsc1_sh_p2_w );

SOUND_START( dkong );
SOUND_START( dkongjr );
SOUND_START( hunchbkd );
SOUND_START( radarsc1 );

DISCRETE_SOUND_EXTERN( dkong );
DISCRETE_SOUND_EXTERN( radarscp );

