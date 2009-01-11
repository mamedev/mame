/*************************************************************************

    Entertainment Sciences RIP System hardware

*************************************************************************/

/* TODO */
#define ESRIPSYS_PIXEL_CLOCK	(XTAL_25MHz / 2)
#define ESRIPSYS_HTOTAL			(512 + 141 + 2)
#define ESRIPSYS_HBLANK_START	(512)
#define ESRIPSYS_HBLANK_END		(0)
#define ESRIPSYS_VTOTAL			(384 + 20)
#define ESRIPSYS_VBLANK_START	(384)
#define ESRIPSYS_VBLANK_END		(0)

#define CMOS_RAM_SIZE			(2048)
#define FDT_RAM_SIZE			(2048 * sizeof(UINT16))

enum
{
	ESRIPSYS_GAME_CPU  = 0,
	ESRIPSYS_FRAME_CPU = 1,
	ESRIPSYS_VIDEO_CPU = 2,
	ESRIPSYS_SOUND_CPU = 3,
};

/*----------- defined in video/esripsys.c -----------*/

extern int esripsys_hblank;
extern UINT8 *esripsys_pal_ram;
extern int esripsys_frame_vbl;
extern int esripsys__12sel;
extern int esripsys_video_firq_en;

VIDEO_START( esripsys );
VIDEO_UPDATE( esripsys );

WRITE8_HANDLER( esripsys_bg_intensity_w );
INTERRUPT_GEN( esripsys_vblank_irq );

int esripsys_draw(running_machine *machine, int l, int r, int fig, int attr, int addr, int col, int x_scale, int line_latch);
