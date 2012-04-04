/***************************************************************************

    Astro Fighter hardware

****************************************************************************/

#include "sound/samples.h"

class astrof_state : public driver_device
{
public:
	astrof_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* video-related */
	UINT8 *    m_videoram;
	size_t     m_videoram_size;

	UINT8 *    m_colorram;
	UINT8 *    m_tomahawk_protection;

	UINT8 *    m_astrof_color;
	UINT8      m_astrof_palette_bank;
	UINT8      m_red_on;
	UINT8      m_flipscreen;
	UINT8      m_screen_off;
	UINT16     m_abattle_count;

	/* sound-related */
	UINT8      m_port_1_last;
	UINT8      m_port_2_last;
	UINT8      m_astrof_start_explosion;
	UINT8      m_astrof_death_playing;
	UINT8      m_astrof_bosskill_playing;

	/* devices */
	device_t *m_maincpu;
	samples_device *m_samples;	// astrof & abattle
	device_t *m_sn;	// tomahawk
	DECLARE_READ8_MEMBER(irq_clear_r);
	DECLARE_WRITE8_MEMBER(astrof_videoram_w);
	DECLARE_WRITE8_MEMBER(tomahawk_videoram_w);
	DECLARE_WRITE8_MEMBER(video_control_1_w);
	DECLARE_WRITE8_MEMBER(astrof_video_control_2_w);
	DECLARE_WRITE8_MEMBER(spfghmk2_video_control_2_w);
	DECLARE_WRITE8_MEMBER(tomahawk_video_control_2_w);
	DECLARE_READ8_MEMBER(shoot_r);
	DECLARE_READ8_MEMBER(abattle_coin_prot_r);
	DECLARE_READ8_MEMBER(afire_coin_prot_r);
	DECLARE_READ8_MEMBER(tomahawk_protection_r);
};

/*----------- defined in audio/astrof.c -----------*/

MACHINE_CONFIG_EXTERN( astrof_audio );
WRITE8_HANDLER( astrof_audio_1_w );
WRITE8_HANDLER( astrof_audio_2_w );

MACHINE_CONFIG_EXTERN( spfghmk2_audio );
WRITE8_HANDLER( spfghmk2_audio_w );

MACHINE_CONFIG_EXTERN( tomahawk_audio );
WRITE8_HANDLER( tomahawk_audio_w );
