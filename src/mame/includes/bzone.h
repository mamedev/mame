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
	void bzone_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t analog_data_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void analog_select_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	ioport_value clock_r(ioport_field &field, void *param);
	uint8_t redbaron_joy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void redbaron_joysound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_bradley();
	virtual void machine_start() override;
	void machine_start_redbaron();
	void bzone_interrupt(device_t &device);
	void bzone_sounds_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
};


/*----------- defined in audio/bzone.c -----------*/
MACHINE_CONFIG_EXTERN( bzone_audio );
