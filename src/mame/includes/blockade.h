#include "sound/discrete.h"
#include "sound/samples.h"

class blockade_state : public driver_device
{
public:
	blockade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	/* input-related */
	UINT8 m_coin_latch;  /* Active Low */
	UINT8 m_just_been_reset;
	DECLARE_READ8_MEMBER(blockade_input_port_0_r);
	DECLARE_WRITE8_MEMBER(blockade_coin_latch_w);
	DECLARE_WRITE8_MEMBER(blockade_videoram_w);
	DECLARE_WRITE8_MEMBER(blockade_env_on_w);
	DECLARE_WRITE8_MEMBER(blockade_env_off_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};


/*----------- defined in video/blockade.c -----------*/


VIDEO_START( blockade );
SCREEN_UPDATE_IND16( blockade );

/*----------- defined in audio/blockade.c -----------*/

extern const samples_interface blockade_samples_interface;
DISCRETE_SOUND_EXTERN( blockade );

WRITE8_DEVICE_HANDLER( blockade_sound_freq_w );
