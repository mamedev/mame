/*************************************************************************

    Jaleco Moero Pro Yakyuu Homerun hardware

*************************************************************************/

#include "sound/upd7759.h"
#include "sound/samples.h"

class homerun_state : public driver_device
{
public:
	homerun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_d7756(*this, "d7756"),
		m_samples(*this, "samples")
	{ }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	optional_device<upd7756_device> m_d7756;
	optional_device<samples_device> m_samples;

	UINT8 m_control;
	UINT8 m_sample;

	tilemap_t *m_tilemap;
	int m_gfx_ctrl;
	int m_scrollx;
	int m_scrolly;
	
	DECLARE_WRITE8_MEMBER(homerun_control_w);
	DECLARE_WRITE8_MEMBER(homerun_d7756_sample_w);
	DECLARE_WRITE8_MEMBER(homerun_videoram_w);
	DECLARE_WRITE8_MEMBER(homerun_color_w);
	DECLARE_WRITE8_MEMBER(homerun_scrollhi_w);
	DECLARE_WRITE8_MEMBER(homerun_scrolly_w);
	DECLARE_WRITE8_MEMBER(homerun_scrollx_w);

	DECLARE_CUSTOM_INPUT_MEMBER(homerun_40_r);
	DECLARE_CUSTOM_INPUT_MEMBER(homerun_d7756_busy_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ganjaja_d7756_busy_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ganjaja_hopper_status_r);

	TILE_GET_INFO_MEMBER(get_homerun_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in video/homerun.c -----------*/

WRITE8_DEVICE_HANDLER( homerun_banking_w );


SCREEN_UPDATE_IND16(homerun);
