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

#define STARSHP1_MASTER_CLOCK		(12096000)
#define STARSHP1_CPU_CLOCK			(STARSHP1_MASTER_CLOCK / 16)
#define STARSHP1_PIXEL_CLOCK		(STARSHP1_MASTER_CLOCK / 2)
#define STARSHP1_HTOTAL				(0x200)
#define STARSHP1_HBEND				(0x000)
#define STARSHP1_HBSTART			(0x200)
#define STARSHP1_VTOTAL				(0x106)
#define STARSHP1_VBEND				(0x000)
#define STARSHP1_VBSTART			(0x0f0)


/*----------- defined in drivers/starshp1.c -----------*/

extern int starshp1_attract;


/*----------- defined in video/starshp1.c -----------*/

extern UINT8 *starshp1_playfield_ram;
extern UINT8 *starshp1_hpos_ram;
extern UINT8 *starshp1_vpos_ram;
extern UINT8 *starshp1_obj_ram;

extern int starshp1_ship_explode;
extern int starshp1_ship_picture;
extern int starshp1_ship_hoffset;
extern int starshp1_ship_voffset;
extern int starshp1_ship_size;

extern int starshp1_circle_hpos;
extern int starshp1_circle_vpos;
extern int starshp1_circle_size;
extern int starshp1_circle_mod;
extern int starshp1_circle_kill;

extern int starshp1_phasor;
extern int starshp1_collision_latch;
extern int starshp1_starfield_kill;
extern int starshp1_mux;

READ8_HANDLER( starshp1_rng_r );

WRITE8_HANDLER( starshp1_sspic_w );
WRITE8_HANDLER( starshp1_ssadd_w );
WRITE8_HANDLER( starshp1_playfield_w );

VIDEO_UPDATE( starshp1 );
VIDEO_EOF( starshp1 );
VIDEO_START( starshp1 );
