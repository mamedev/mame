// license:BSD-3-Clause
// copyright-holders:Couriersud
#include "emu.h"
#include "mario.h"

#include "cpu/mcs48/mcs48.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "nl_mario.h"

/****************************************************************
 *
 * Initialization
 *
 ****************************************************************/

void mario_state::sound_start()
{
	if (m_audiocpu->type() != Z80)
	{
		uint8_t *SND = memregion("audiocpu")->base();

		// Hack to bootstrap MCU program into external MB1
		SND[0x0000] = 0xf5;
		SND[0x0001] = 0x04;
		SND[0x0002] = 0x00;
	}

	save_item(NAME(m_last));
}

void mario_state::sound_reset()
{
	m_soundlatch[0]->clear_w();
	if (m_soundlatch[1]) m_soundlatch[1]->clear_w();
	if (m_soundlatch[2]) m_soundlatch[2]->clear_w();
	if (m_soundlatch[3]) m_soundlatch[3]->clear_w();

	m_last = 0;
}

/****************************************************************
 *
 * I/O Handlers - static
 *
 ****************************************************************/

uint8_t mario_state::mario_sh_tune_r(offs_t offset)
{
	uint8_t p2 = m_soundlatch[2]->read();

	if ((p2 >> 7) & 1)
		return m_soundlatch[0]->read();
	else
		return m_soundrom[(p2 & 0x0f) << 8 | offset];
}

void mario_state::mario_sh_sound_w(uint8_t data)
{
	m_audio_dac->write(data);
}

/****************************************************************
 *
 * I/O Handlers - global
 *
 ****************************************************************/

void mario_state::masao_sh_irqtrigger_w(uint8_t data)
{
	data &= 1;

	// setting bit 0 high then low triggers IRQ on the sound CPU
	if (m_last && !data)
		m_audiocpu->set_input_line(0, HOLD_LINE);

	m_last = data;
}

/* Misc samples */
void mario_state::mario_sh3_w(offs_t offset, uint8_t data)
{
	data &= 1;

	switch (offset)
	{
		// death
		case 0:
			m_audiocpu->set_input_line(0, data ? ASSERT_LINE : CLEAR_LINE);
			break;

		// get coin, ice
		case 1: case 2:
		{
			const uint8_t mask = 1 << (offset - 1);
			m_soundlatch[3]->write((m_soundlatch[3]->read() & ~mask) | (data ? mask : 0));
			break;
		}

		// crab, turtle, fly, coin
		case 3: case 4: case 5: case 6:
		{
			const uint8_t mask = 1 << (offset - 3);
			m_soundlatch[1]->write((m_soundlatch[1]->read() & ~mask) | (data ? mask : 0));
			break;
		}

		// skid
		case 7:
			m_audio_snd7->write(data ^ 1);
			break;
	}
}

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

void mario_state::mario_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(m_soundrom, 0);
}

void mario_state::mario_sound_io_map(address_map &map)
{
	map(0x00, 0xff).r(FUNC(mario_state::mario_sh_tune_r)).w(FUNC(mario_state::mario_sh_sound_w));
}

void mario_state::masao_sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x4000, 0x4000).rw("aysnd", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x6000, 0x6000).w("aysnd", FUNC(ay8910_device::address_w));
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mario_state::mario_audio(machine_config &config)
{
	m58715_device &audiocpu(M58715(config, m_audiocpu, 11_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &mario_state::mario_sound_map);
	audiocpu.set_addrmap(AS_IO, &mario_state::mario_sound_io_map);
	audiocpu.p1_in_cb().set(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	audiocpu.p1_out_cb().set(m_soundlatch[1], FUNC(generic_latch_8_device::write));
	audiocpu.p2_in_cb().set(m_soundlatch[2], FUNC(generic_latch_8_device::read)).mask(0xef); // bit 4 is GND!
	audiocpu.p2_out_cb().set(m_soundlatch[2], FUNC(generic_latch_8_device::write));
	audiocpu.p2_out_cb().append_inputline(m_audiocpu, MCS48_INPUT_EA).bit(5).invert();
	audiocpu.t0_in_cb().set(m_soundlatch[3], FUNC(generic_latch_8_device::read)).bit(0);
	audiocpu.t1_in_cb().set(m_soundlatch[3], FUNC(generic_latch_8_device::read)).bit(1);

	SPEAKER(config, "mono").front_center();

	for (int i = 0; i < 4; i++)
		GENERIC_LATCH_8(config, m_soundlatch[i]);

	NETLIST_SOUND(config, "snd_nl", 48000)
		.set_source(netlist_mario)
		.add_route(ALL_OUTPUTS, "mono", 0.5);

	NETLIST_LOGIC_INPUT(config, m_audio_snd0, "SOUND0.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_snd1, "SOUND1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_snd7, "SOUND7.IN", 0);
	NETLIST_INT_INPUT(config, m_audio_dac, "DAC.VAL", 0, 255);

	NETLIST_STREAM_OUTPUT(config, "snd_nl:cout0", 0, "ROUT.1").set_mult_offset(150000.0 / 32768.0, 0.0);
}

void mario_state::masao_audio(machine_config &config)
{
	Z80(config, m_audiocpu, 14'318'181 / 8); // 1.79MHz?
	m_audiocpu->set_addrmap(AS_PROGRAM, &mario_state::masao_sound_map);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);

	ay8910_device &aysnd(AY8910(config, "aysnd", 14'318'181 / 8)); // 1.79MHz?
	aysnd.port_a_read_callback().set(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}
