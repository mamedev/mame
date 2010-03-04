#ifndef MARIO_H_
#define MARIO_H_

/*
 * From the schematics:
 *
 * Video generation like dkong/dkongjr. However, clock is 24MHZ
 * 7C -> 100 => 256 - 124 = 132 ==> 264 Scanlines
 */

#define MASTER_CLOCK			XTAL_24MHz
#define PIXEL_CLOCK				(MASTER_CLOCK / 4)
#define CLOCK_1H				(MASTER_CLOCK / 8)
#define CLOCK_16H				(CLOCK_1H / 16)
#define CLOCK_1VF				((CLOCK_16H) / 12 / 2)
#define CLOCK_2VF				((CLOCK_1VF) / 2)

#define HTOTAL					(384)
#define HBSTART					(256)
#define HBEND					(0)
#define VTOTAL					(264)
#define VBSTART					(240)
#define VBEND					(16)

#define Z80_MASTER_CLOCK		XTAL_8MHz
#define Z80_CLOCK				(Z80_MASTER_CLOCK / 2) /* verified on pcb */

#define I8035_MASTER_CLOCK		XTAL_11MHz /* verified on pcb: 730Khz */
#define I8035_CLOCK				(I8035_MASTER_CLOCK)

#define MARIO_PALETTE_LENGTH	(256)

class mario_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, mario_state(machine)); }

	mario_state(running_machine &machine) { }

	/* memory pointers */

	/* machine states */

	/* sound state */
	UINT8	last;
	UINT8	portT;
	const char *eabank;

	/* video state */
	UINT8	gfx_bank;
	UINT8	palette_bank;
	UINT16	gfx_scroll;
	UINT8	flip;

	/* driver general */

	UINT8	*spriteram;
	UINT8	*videoram;
	size_t	spriteram_size;
	tilemap_t *bg_tilemap;
	int monitor;
};

/*----------- defined in video/mario.c -----------*/

WRITE8_HANDLER( mario_videoram_w );
WRITE8_HANDLER( mario_gfxbank_w );
WRITE8_HANDLER( mario_palettebank_w );
WRITE8_HANDLER( mario_scroll_w );
WRITE8_HANDLER( mario_flip_w );

PALETTE_INIT( mario );
VIDEO_START( mario );
VIDEO_UPDATE( mario );


/*----------- defined in audio/mario.c -----------*/

WRITE8_DEVICE_HANDLER( mario_sh1_w );
WRITE8_DEVICE_HANDLER( mario_sh2_w );
WRITE8_HANDLER( mario_sh3_w );

WRITE8_HANDLER( mario_sh_tuneselect_w );
WRITE8_HANDLER( masao_sh_irqtrigger_w );

MACHINE_DRIVER_EXTERN( mario_audio );
MACHINE_DRIVER_EXTERN( masao_audio );

#endif /*MARIO_H_*/
