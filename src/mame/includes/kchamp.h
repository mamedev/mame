/*************************************************************************

    Karate Champ

*************************************************************************/

class kchamp_state : public driver_device
{
public:
	kchamp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_nmi_enable;
	int        m_sound_nmi_enable;
	int        m_msm_data;
	int        m_msm_play_lo_nibble;
	int        m_counter;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(sound_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(sound_msm_w);
	DECLARE_READ8_MEMBER(sound_reset_r);
	DECLARE_WRITE8_MEMBER(kc_sound_control_w);
	DECLARE_WRITE8_MEMBER(kchamp_videoram_w);
	DECLARE_WRITE8_MEMBER(kchamp_colorram_w);
	DECLARE_WRITE8_MEMBER(kchamp_flipscreen_w);
};


/*----------- defined in video/kchamp.c -----------*/


PALETTE_INIT( kchamp );
VIDEO_START( kchamp );
SCREEN_UPDATE_IND16( kchamp );
SCREEN_UPDATE_IND16( kchampvs );
