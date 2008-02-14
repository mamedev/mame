/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	/* Early F3 class games, these are not cartridge games and system features may be different */
	RINGRAGE=0,	/* D21 */
	ARABIANM,	/* D29 */
	RIDINGF,	/* D34 */
	GSEEKER,	/* D40 */
	TRSTAR,		/* D53 */
	GUNLOCK,	/* D66 */
	TWINQIX,
	UNDRFIRE,	/* D67 - Heavily modified F3 hardware (different memory map) */
	SCFINALS,
	LIGHTBR,	/* D69 */

	/* D77 - F3 motherboard proms, all following games are 'F3 package system' */
	/* D78 I CUP */
	KAISERKN,	/* D84 */
	DARIUSG,	/* D87 */
	BUBSYMPH,	/* D90 */
	SPCINVDX,	/* D93 */
	HTHERO95,	/* D94 */
	QTHEATER,	/* D95 */
	EACTION2,	/* E02 */
	SPCINV95,	/* E06 */
	QUIZHUHU,	/* E08 */
	PBOBBLE2,	/* E10 */
	GEKIRIDO,	/* E11 */
	KTIGER2,	/* E15 */
	BUBBLEM,	/* E21 */
	CLEOPATR,	/* E28 */
	PBOBBLE3,	/* E29 */
	ARKRETRN,	/* E36 */
	KIRAMEKI,	/* E44 */
	PUCHICAR,	/* E46 */
	PBOBBLE4,	/* E49 */
	POPNPOP,	/* E51 */
	LANDMAKR,	/* E61 */
	RECALH,		/* prototype */
	COMMANDW	/* prototype */
};

/*----------- defined in drivers/taito_f3.c -----------*/

extern UINT32 *f3_shared_ram;
extern int f3_game;

/*----------- defined in video/taito_f3.c -----------*/

extern UINT32 *f3_vram,*f3_line_ram;
extern UINT32 *f3_pf_data,*f3_pivot_ram;

VIDEO_START( f3 );
VIDEO_UPDATE( f3 );
VIDEO_EOF( f3 );

WRITE32_HANDLER( f3_control_0_w );
WRITE32_HANDLER( f3_control_1_w );
WRITE32_HANDLER( f3_palette_24bit_w );
WRITE32_HANDLER( f3_pf_data_w );
WRITE32_HANDLER( f3_vram_w );
WRITE32_HANDLER( f3_pivot_w );
WRITE32_HANDLER( f3_lineram_w );
WRITE32_HANDLER( f3_videoram_w );


