
/*----------- timings -----------*/

#define BAGMAN_MAIN_CLOCK	XTAL_18_432MHz
#define BAGMAN_HCLK			(BAGMAN_MAIN_CLOCK / 3)
#define BAGMAN_H0			(BAGMAN_HCLK / 2)
#define BAGMAN_H1			(BAGMAN_H0   / 2)
#define HTOTAL				((0x100-0x40)*2)
#define HBEND				(0x00)
#define HBSTART				(0x100)
#define VTOTAL				((0x100-0x7c)*2)

/* the following VBEND/VBSTART are used for compsync
 * #define VBEND                (0x08)
 * #define VBSTART              (0x100)
 *
 * However VBSYQ (and INTQ) is generated using the following values:
 */
#define VBEND				(0x0f)
#define VBSTART				(0xef)

/*----------- defined in machine/bagman.c -----------*/

READ8_HANDLER( bagman_pal16r6_r );
MACHINE_RESET( bagman );
WRITE8_HANDLER( bagman_pal16r6_w );


/*----------- defined in video/bagman.c -----------*/

extern UINT8 *bagman_videoram;
extern UINT8 *bagman_colorram;
extern UINT8 *bagman_video_enable;

WRITE8_HANDLER( bagman_videoram_w );
WRITE8_HANDLER( bagman_colorram_w );
WRITE8_HANDLER( bagman_flipscreen_w );

PALETTE_INIT( bagman );
VIDEO_START( bagman );
VIDEO_UPDATE( bagman );
