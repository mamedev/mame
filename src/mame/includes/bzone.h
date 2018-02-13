// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Nicola Salmoria
/*************************************************************************

    Atari Battle Zone hardware

*************************************************************************/

#include "audio/redbaron.h"
#include "machine/mathbox.h"
#include "sound/discrete.h"

#define BZONE_MASTER_CLOCK (XTAL(12'096'000))
#define BZONE_CLOCK_3KHZ   (BZONE_MASTER_CLOCK / 4096)

class bzone_state : public driver_device
{
public:
	bzone_state(const machine_config &mconfig, device_type type, const char *tag)
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

	uint8_t m_analog_data;
	uint8_t m_rb_input_select;
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
	void bzone_base(machine_config &config);
	void redbaron(machine_config &config);
	void bzone(machine_config &config);
	void bzone_audio(machine_config &config);
	void bzone_map(address_map &map);
	void redbaron_map(address_map &map);
};
