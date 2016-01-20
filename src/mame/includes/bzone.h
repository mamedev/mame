// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Nicola Salmoria
/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/

#include "audio/redbaron.h"
#include "machine/mathbox.h"
#include "sound/discrete.h"

#define BZONE_MASTER_CLOCK (XTAL_12_096MHz)
#define BZONE_CLOCK_3KHZ   ((double)BZONE_MASTER_CLOCK / 4096)

class bzone_state : public driver_device
{
public:
	bzone_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mathbox(*this, "mathbox"),
		m_discrete(*this, "discrete"),
		m_redbaronsound(*this, "custom")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<mathbox_device> m_mathbox;
	optional_device<discrete_device> m_discrete;
	optional_device<redbaron_sound_device> m_redbaronsound;

	UINT8 m_analog_data;
	UINT8 m_rb_input_select;
	DECLARE_WRITE8_MEMBER(bzone_coin_counter_w);
	DECLARE_READ8_MEMBER(analog_data_r);
	DECLARE_WRITE8_MEMBER(analog_select_w);
	DECLARE_CUSTOM_INPUT_MEMBER(clock_r);
	DECLARE_READ8_MEMBER(redbaron_joy_r);
	DECLARE_WRITE8_MEMBER(redbaron_joysound_w);
	DECLARE_DRIVER_INIT(bradley);
	virtual void machine_start() override;
	DECLARE_MACHINE_START(redbaron);
	INTERRUPT_GEN_MEMBER(bzone_interrupt);
	DECLARE_WRITE8_MEMBER(bzone_sounds_w);
};


/*----------- defined in audio/bzone.c -----------*/
MACHINE_CONFIG_EXTERN( bzone_audio );
