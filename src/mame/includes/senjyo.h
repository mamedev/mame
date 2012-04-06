#include "sound/samples.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"

class senjyo_state : public driver_device
{
public:
	senjyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_int_delay_kludge;
	UINT8 m_sound_cmd;
	INT16 *m_single_data;
	int m_single_rate;
	int m_single_volume;

	size_t m_spriteram_size;
	UINT8 *m_spriteram;
	UINT8 *m_fgscroll;
	UINT8 *m_scrollx1;
	UINT8 *m_scrolly1;
	UINT8 *m_scrollx2;
	UINT8 *m_scrolly2;
	UINT8 *m_scrollx3;
	UINT8 *m_scrolly3;
	UINT8 *m_fgvideoram;
	UINT8 *m_fgcolorram;
	UINT8 *m_bg1videoram;
	UINT8 *m_bg2videoram;
	UINT8 *m_bg3videoram;
	UINT8 *m_radarram;
	UINT8 *m_bgstripesram;
	int m_is_senjyo;
	int m_scrollhack;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg1_tilemap;
	tilemap_t *m_bg2_tilemap;
	tilemap_t *m_bg3_tilemap;

	int m_bgstripes;
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(senjyo_paletteram_w);
	DECLARE_WRITE8_MEMBER(starforb_scrolly2);
	DECLARE_WRITE8_MEMBER(starforb_scrollx2);
	DECLARE_WRITE8_MEMBER(senjyo_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(senjyo_fgcolorram_w);
	DECLARE_WRITE8_MEMBER(senjyo_bg1videoram_w);
	DECLARE_WRITE8_MEMBER(senjyo_bg2videoram_w);
	DECLARE_WRITE8_MEMBER(senjyo_bg3videoram_w);
	DECLARE_WRITE8_MEMBER(senjyo_bgstripes_w);
};


/*----------- defined in audio/senjyo.c -----------*/

extern const z80_daisy_config senjyo_daisy_chain[];
extern const z80pio_interface senjyo_pio_intf;
extern const z80ctc_interface senjyo_ctc_intf;

SAMPLES_START( senjyo_sh_start );
WRITE8_HANDLER( senjyo_volume_w );


/*----------- defined in video/senjyo.c -----------*/


VIDEO_START( senjyo );
SCREEN_UPDATE_IND16( senjyo );
