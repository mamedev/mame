/*************************************************************************

    Jaleco Exerion

*************************************************************************/


#define EXERION_MASTER_CLOCK      (XTAL_19_968MHz)   /* verified on pcb */
#define EXERION_CPU_CLOCK         (EXERION_MASTER_CLOCK / 6)
#define EXERION_AY8910_CLOCK      (EXERION_CPU_CLOCK / 2)
#define EXERION_PIXEL_CLOCK       (EXERION_MASTER_CLOCK / 3)
#define EXERION_HCOUNT_START      (0x58)
#define EXERION_HTOTAL            (512-EXERION_HCOUNT_START)
#define EXERION_HBEND             (12*8)	/* ?? */
#define EXERION_HBSTART           (52*8)	/* ?? */
#define EXERION_VTOTAL            (256)
#define EXERION_VBEND             (16)
#define EXERION_VBSTART           (240)


class exerion_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, exerion_state(machine)); }

	exerion_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *  main_ram;
	UINT8 *  videoram;
	UINT8 *  spriteram;
	size_t   videoram_size;
	size_t   spriteram_size;

	/* video-related */
	UINT8    cocktail_flip;
	UINT8    char_palette, sprite_palette;
	UINT8    char_bank;
	UINT16   *background_gfx[4];
	UINT8    *background_mixer;
	UINT8    background_latches[13];

	/* protection? */
	UINT8 porta;
	UINT8 portb;

	/* devices */
	running_device *maincpu;
};



/*----------- defined in video/exerion.c -----------*/

PALETTE_INIT( exerion );
VIDEO_START( exerion );
VIDEO_UPDATE( exerion );

WRITE8_HANDLER( exerion_videoreg_w );
WRITE8_HANDLER( exerion_video_latch_w );
READ8_HANDLER( exerion_video_timing_r );
