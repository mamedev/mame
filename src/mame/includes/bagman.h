
class bagman_state : public driver_device
{
public:
	bagman_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 ls259_buf[8];
	UINT8 p1_res;
	UINT8 p1_old_val;
	UINT8 p2_res;
	UINT8 p2_old_val;
	UINT8 *videoram;
	UINT8 *colorram;
	UINT8 *video_enable;

	/*table holds outputs of all ANDs (after AND map)*/
	UINT8 andmap[64];

	/*table holds inputs (ie. not x, x, not q, q) to the AND map*/
	UINT8 columnvalue[32];

	/*8 output pins (actually 6 output and 2 input/output)*/
	UINT8 outvalue[8];

	tilemap_t *bg_tilemap;
	UINT8 *spriteram;
	size_t spriteram_size;
};


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

WRITE8_HANDLER( bagman_videoram_w );
WRITE8_HANDLER( bagman_colorram_w );
WRITE8_HANDLER( bagman_flipscreen_w );

PALETTE_INIT( bagman );
VIDEO_START( bagman );
SCREEN_UPDATE( bagman );
