#include "sound/samples.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"

class senjyo_state : public driver_device
{
public:
	senjyo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int int_delay_kludge;
	UINT8 sound_cmd;
	INT16 *single_data;
	int single_rate;
	int single_volume;

	size_t spriteram_size;
	UINT8 *spriteram;
	UINT8 *fgscroll;
	UINT8 *scrollx1;
	UINT8 *scrolly1;
	UINT8 *scrollx2;
	UINT8 *scrolly2;
	UINT8 *scrollx3;
	UINT8 *scrolly3;
	UINT8 *fgvideoram;
	UINT8 *fgcolorram;
	UINT8 *bg1videoram;
	UINT8 *bg2videoram;
	UINT8 *bg3videoram;
	UINT8 *radarram;
	UINT8 *bgstripesram;
	int is_senjyo;
	int scrollhack;
	tilemap_t *fg_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *bg2_tilemap;
	tilemap_t *bg3_tilemap;

	int bgstripes;
};


/*----------- defined in audio/senjyo.c -----------*/

extern const z80_daisy_config senjyo_daisy_chain[];
extern const z80pio_interface senjyo_pio_intf;
extern const z80ctc_interface senjyo_ctc_intf;

SAMPLES_START( senjyo_sh_start );
WRITE8_HANDLER( senjyo_volume_w );


/*----------- defined in video/senjyo.c -----------*/

WRITE8_HANDLER( senjyo_fgvideoram_w );
WRITE8_HANDLER( senjyo_fgcolorram_w );
WRITE8_HANDLER( senjyo_bg1videoram_w );
WRITE8_HANDLER( senjyo_bg2videoram_w );
WRITE8_HANDLER( senjyo_bg3videoram_w );
WRITE8_HANDLER( senjyo_bgstripes_w );

VIDEO_START( senjyo );
VIDEO_UPDATE( senjyo );
