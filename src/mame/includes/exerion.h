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


class exerion_state : public driver_device
{
public:
	exerion_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_main_ram;
	UINT8 *  m_videoram;
	UINT8 *  m_spriteram;
	size_t   m_videoram_size;
	size_t   m_spriteram_size;

	/* video-related */
	UINT8    m_cocktail_flip;
	UINT8    m_char_palette;
	UINT8    m_sprite_palette;
	UINT8    m_char_bank;
	UINT16   *m_background_gfx[4];
	UINT8    *m_background_mixer;
	UINT8    m_background_latches[13];

	/* protection? */
	UINT8 m_porta;
	UINT8 m_portb;

	/* devices */
	device_t *m_maincpu;
	DECLARE_READ8_MEMBER(exerion_protection_r);
	DECLARE_WRITE8_MEMBER(exerion_videoreg_w);
	DECLARE_WRITE8_MEMBER(exerion_video_latch_w);
	DECLARE_READ8_MEMBER(exerion_video_timing_r);
};



/*----------- defined in video/exerion.c -----------*/

PALETTE_INIT( exerion );
VIDEO_START( exerion );
SCREEN_UPDATE_IND16( exerion );

