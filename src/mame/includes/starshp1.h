/*
 *  The schematics don't seem to make a lot of sense when it
 *  comes to the video timing chain::
 *
 *    * there are clear typos -- what is H132???
 *    * there are two HBLANK/HSYNC periods as the horizontal
 *      chain is drawn, which would give an alternating long
 *      line followed by a much shorter one.  This cannot be right
 *    * the carry-out/load circuit on LS161@J8 is superflous
 *
 *  These values also give a frame rate of about 45Hz, which is
 *  probably too low.  I suspect that screen is not really
 *  512 pixels wide -- most likely 384, which would give 60Hz
 *
 *  Some of the graphics, like the starfield, is clocked with the
 *  12MHz signal, effecitvely doubling the horizontal resolution
 */

#include "sound/discrete.h"


#define STARSHP1_MASTER_CLOCK		(12096000)
#define STARSHP1_CPU_CLOCK			(STARSHP1_MASTER_CLOCK / 16)
#define STARSHP1_PIXEL_CLOCK		(STARSHP1_MASTER_CLOCK / 2)
#define STARSHP1_HTOTAL				(0x200)
#define STARSHP1_HBEND				(0x000)
#define STARSHP1_HBSTART			(0x200)
#define STARSHP1_VTOTAL				(0x106)
#define STARSHP1_VBEND				(0x000)
#define STARSHP1_VBSTART			(0x0f0)


class starshp1_state : public driver_device
{
public:
	starshp1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int analog_in_select;
	int attract;
	UINT8 *playfield_ram;
	UINT8 *hpos_ram;
	UINT8 *vpos_ram;
	UINT8 *obj_ram;
	int ship_explode;
	int ship_picture;
	int ship_hoffset;
	int ship_voffset;
	int ship_size;
	int circle_hpos;
	int circle_vpos;
	int circle_size;
	int circle_mod;
	int circle_kill;
	int phasor;
	int collision_latch;
	int starfield_kill;
	int mux;
	int inverse;
	UINT16 *LSFR;
	bitmap_t *helper;
	tilemap_t *bg_tilemap;
};


/*----------- defined in video/starshp1.c -----------*/

READ8_HANDLER( starshp1_rng_r );

WRITE8_HANDLER( starshp1_sspic_w );
WRITE8_HANDLER( starshp1_ssadd_w );
WRITE8_HANDLER( starshp1_playfield_w );

PALETTE_INIT( starshp1 );
VIDEO_UPDATE( starshp1 );
VIDEO_EOF( starshp1 );
VIDEO_START( starshp1 );


/*----------- defined in audio/starshp1.c -----------*/

DISCRETE_SOUND_EXTERN( starshp1 );

/* Discrete Sound Input Nodes */
#define STARSHP1_NOISE_AMPLITUDE	NODE_01
#define STARSHP1_TONE_PITCH         NODE_02
#define	STARSHP1_MOTOR_SPEED		NODE_03
#define	STARSHP1_NOISE_FREQ			NODE_04
#define	STARSHP1_MOLVL				NODE_05
#define	STARSHP1_SL2				NODE_06
#define	STARSHP1_SL1				NODE_07
#define	STARSHP1_KICKER				NODE_08
#define	STARSHP1_PHASOR_ON			NODE_09
#define	STARSHP1_ATTRACT			NODE_10
