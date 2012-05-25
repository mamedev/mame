/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/

#include "devlegcy.h"

#define BZONE_MASTER_CLOCK (XTAL_12_096MHz)
#define BZONE_CLOCK_3KHZ  (MASTER_CLOCK / 4096)

class bzone_state : public driver_device
{
public:
	bzone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_analog_data;
	UINT8 m_rb_input_select;
	DECLARE_WRITE8_MEMBER(bzone_coin_counter_w);
	DECLARE_READ8_MEMBER(analog_data_r);
	DECLARE_WRITE8_MEMBER(analog_select_w);
	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);
	DECLARE_READ8_MEMBER(redbaron_joy_r);
	DECLARE_WRITE8_MEMBER(redbaron_joysound_w);
};


/*----------- defined in audio/bzone.c -----------*/

WRITE8_DEVICE_HANDLER( bzone_sounds_w );

MACHINE_CONFIG_EXTERN( bzone_audio );

/*----------- defined in audio/redbaron.c -----------*/

WRITE8_DEVICE_HANDLER( redbaron_sounds_w );

DECLARE_LEGACY_SOUND_DEVICE(REDBARON, redbaron_sound);
