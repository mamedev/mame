/*************************************************************************

    Meadows S2650 hardware

*************************************************************************/

#include "sound/samples.h"

class meadows_state : public driver_device
{
public:
	meadows_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"){ }

	optional_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_videoram;
	UINT8 m_dac;
	int m_dac_enable;
	int m_channel;
	int m_freq1;
	int m_freq2;
	UINT8 m_latched_0c01;
	UINT8 m_latched_0c02;
	UINT8 m_latched_0c03;
	UINT8 m_main_sense_state;
	UINT8 m_audio_sense_state;
	UINT8 m_0c00;
	UINT8 m_0c01;
	UINT8 m_0c02;
	UINT8 m_0c03;
	tilemap_t *m_bg_tilemap;
	DECLARE_READ8_MEMBER(hsync_chain_r);
	DECLARE_READ8_MEMBER(vsync_chain_hi_r);
	DECLARE_READ8_MEMBER(vsync_chain_lo_r);
	DECLARE_WRITE8_MEMBER(meadows_audio_w);
	DECLARE_WRITE8_MEMBER(audio_hardware_w);
	DECLARE_READ8_MEMBER(audio_hardware_r);
	DECLARE_WRITE8_MEMBER(meadows_videoram_w);
	DECLARE_WRITE8_MEMBER(meadows_spriteram_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(minferno);
	DECLARE_DRIVER_INIT(gypsyjug);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_meadows(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in audio/meadows.c -----------*/

SAMPLES_START( meadows_sh_start );
void meadows_sh_dac_w(running_machine &machine, int data);
void meadows_sh_update(running_machine &machine);
