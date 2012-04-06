
#define STARFIRE_MASTER_CLOCK	(20000000)
#define STARFIRE_CPU_CLOCK		(STARFIRE_MASTER_CLOCK / 8)
#define STARFIRE_PIXEL_CLOCK	(STARFIRE_MASTER_CLOCK / 4)
#define STARFIRE_HTOTAL			(0x13f)  /* could be 0x140, but I think this is right */
#define STARFIRE_HBEND			(0x000)
#define STARFIRE_HBSTART		(0x100)
#define STARFIRE_VTOTAL			(0x106)
#define STARFIRE_VBEND			(0x020)
#define STARFIRE_VBSTART		(0x100)
#define	STARFIRE_NUM_PENS       (0x40)

class starfire_state : public driver_device
{
public:
	starfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

    read8_delegate m_input_read;

	UINT8 m_fireone_select;

    UINT8 m_starfire_vidctrl;
    UINT8 m_starfire_vidctrl1;
    UINT8 m_starfire_color;
    UINT16 m_starfire_colors[STARFIRE_NUM_PENS];

    UINT8 *m_starfire_videoram;
    UINT8 *m_starfire_colorram;

    emu_timer* m_scanline_timer;
    bitmap_rgb32 m_starfire_screen;
	DECLARE_WRITE8_MEMBER(starfire_scratch_w);
	DECLARE_READ8_MEMBER(starfire_scratch_r);
	DECLARE_READ8_MEMBER(starfire_input_r);
	DECLARE_READ8_MEMBER(fireone_input_r);
	DECLARE_WRITE8_MEMBER(starfire_colorram_w);
	DECLARE_READ8_MEMBER(starfire_colorram_r);
	DECLARE_WRITE8_MEMBER(starfire_videoram_w);
	DECLARE_READ8_MEMBER(starfire_videoram_r);
};

/*----------- defined in video/starfire.c -----------*/

SCREEN_UPDATE_RGB32( starfire );
VIDEO_START( starfire );


