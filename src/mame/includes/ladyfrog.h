/*************************************************************************

    Lady Frog

*************************************************************************/

class ladyfrog_state : public driver_device
{
public:
	ladyfrog_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_spriteram;
	UINT8 *    m_scrlram;
//      UINT8 *    m_paletteram;    // currently this uses generic palette handling
//      UINT8 *    m_paletteram2;   // currently this uses generic palette handling
	size_t     m_videoram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tilebank;
	int        m_palette_bank;
	int        m_spritetilebase;

	/* misc */
	int        m_sound_nmi_enable;
	int        m_pending_nmi;
	int        m_snd_flag;
	UINT8      m_snd_data;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_READ8_MEMBER(from_snd_r);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_WRITE8_MEMBER(sound_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_READ8_MEMBER(snd_flag_r);
};


/*----------- defined in video/ladyfrog.c -----------*/

WRITE8_HANDLER( ladyfrog_videoram_w );
WRITE8_HANDLER( ladyfrog_spriteram_w );
WRITE8_HANDLER( ladyfrog_palette_w );
WRITE8_HANDLER( ladyfrog_gfxctrl_w );
WRITE8_HANDLER( ladyfrog_gfxctrl2_w );
WRITE8_HANDLER( ladyfrog_scrlram_w );

READ8_HANDLER( ladyfrog_spriteram_r );
READ8_HANDLER( ladyfrog_palette_r );
READ8_HANDLER( ladyfrog_scrlram_r );
READ8_HANDLER( ladyfrog_videoram_r );

VIDEO_START( ladyfrog );
VIDEO_START( toucheme );
SCREEN_UPDATE_IND16( ladyfrog );
