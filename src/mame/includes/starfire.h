
#define STARFIRE_MASTER_CLOCK	(20000000)
#define STARFIRE_CPU_CLOCK		(STARFIRE_MASTER_CLOCK / 8)
#define STARFIRE_PIXEL_CLOCK	(STARFIRE_MASTER_CLOCK / 4)
#define STARFIRE_HTOTAL			(0x13f)  /* could be 0x140, but I think this is right */
#define STARFIRE_HBEND			(0x000)
#define STARFIRE_HBSTART		(0x100)
#define STARFIRE_VTOTAL			(0x106)
#define STARFIRE_VBEND			(0x020)
#define STARFIRE_VBSTART		(0x100)


/*----------- defined in drivers/starfire.c -----------*/

extern UINT8 *starfire_videoram;
extern UINT8 *starfire_colorram;


/*----------- defined in video/starfire.c -----------*/

VIDEO_UPDATE( starfire );
VIDEO_START( starfire );
void starfire_video_update(int scanline, int count);

WRITE8_HANDLER( starfire_videoram_w );
READ8_HANDLER( starfire_videoram_r );
WRITE8_HANDLER( starfire_colorram_w );
READ8_HANDLER( starfire_colorram_r );
WRITE8_HANDLER( starfire_vidctrl_w );
WRITE8_HANDLER( starfire_vidctrl1_w );

