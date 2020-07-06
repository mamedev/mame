// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Cinematronics vector hardware

    Special thanks to Neil Bradley, Zonn Moore, and Jeff Mitchell of the
    Retrocade Alliance

    Update:
    6/27/99 Jim Hernandez -- 1st Attempt at Fixing Drone Star Castle sound and
                             pitch adjustments.
    6/30/99 MLR added Rip Off, Solar Quest, Armor Attack (no samples yet)
    11/04/08 Jim Hernandez -- Fixed Drone Star Castle sound again. It was
                              broken for a long time due to some changes.

    Bugs: Sometimes the death explosion (small explosion) does not trigger.

***************************************************************************/

#include "emu.h"
#include "includes/cinemat.h"

#include "audio/nl_spacewar.h"
#include "audio/nl_speedfrk.h"
#include "audio/nl_starcas.h"
#include "audio/nl_starhawk.h"
#include "audio/nl_sundance.h"
#include "audio/nl_tailg.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "speaker.h"


/*************************************
 *
 *  Base class
 *
 *************************************/

cinemat_audio_device::cinemat_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 inputs_mask)
	: device_t(mconfig, type, tag, owner, clock)
	, m_samples(*this, "samples")
	, m_out_input(*this, "sound_nl:out_%u", 0)
	, m_shiftreg_input(*this, "sound_nl:shiftreg_%u", 0)
	, m_shiftreg16_input(*this, "sound_nl:shiftreg16_%u", 0)
	, m_inputs_mask(inputs_mask)
{
}

void cinemat_audio_device::configure_latch_inputs(ls259_device &latch, u8 mask)
{
	if (BIT(mask, 0)) latch.q_out_cb<0>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<0>)));
	if (BIT(mask, 1)) latch.q_out_cb<1>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<1>)));
	if (BIT(mask, 2)) latch.q_out_cb<2>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<2>)));
	if (BIT(mask, 3)) latch.q_out_cb<3>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<3>)));
	if (BIT(mask, 4)) latch.q_out_cb<4>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<4>)));
	if (BIT(mask, 5)) latch.q_out_cb<5>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<5>)));
	if (BIT(mask, 6)) latch.q_out_cb<6>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<6>)));
	if (BIT(mask, 7)) latch.q_out_cb<7>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<7>)));
}

void cinemat_audio_device::device_start()
{
	save_item(NAME(m_inputs));
	save_item(NAME(m_shiftreg));
	save_item(NAME(m_shiftreg_accum));
	save_item(NAME(m_shiftreg16));
	save_item(NAME(m_shiftreg16_accum));
#if ENABLE_NETLIST_LOGGING
	m_logfile = fopen("cinemat.csv", "w");
#endif
}

void cinemat_audio_device::device_stop()
{
#if ENABLE_NETLIST_LOGGING
	if (m_logfile != nullptr)
		fclose(m_logfile);
#endif
}

void cinemat_audio_device::input_set(int bit, int state)
{
	u8 oldvals = m_inputs;
	m_inputs = (m_inputs & ~(1 << bit)) | ((state & 1) << bit);
	if (oldvals != m_inputs)
	{
		log_changes(m_inputs, oldvals, "I_OUT", m_inputs_mask);
		for (int index = 0; index < 8; index++)
			if (m_out_input[index] != nullptr)
				m_out_input[index]->write_line(BIT(m_inputs, index));
		inputs_changed(m_inputs, oldvals);
	}
}

void cinemat_audio_device::shiftreg_latch()
{
	u8 oldvals = m_shiftreg;
	m_shiftreg = shiftreg_swizzle(m_shiftreg_accum);
	if (oldvals != m_shiftreg)
	{
		log_changes(m_shiftreg, oldvals, "I_SHIFTREG");
		for (int index = 0; index < 8; index++)
			if (m_shiftreg_input[index] != nullptr)
				m_shiftreg_input[index]->write_line(BIT(m_shiftreg, index));
		shiftreg_changed(m_shiftreg, oldvals);
	}
}

void cinemat_audio_device::shiftreg16_latch()
{
	u16 oldvals = m_shiftreg16;
	m_shiftreg16 = m_shiftreg16_accum;
	if (oldvals != m_shiftreg16)
	{
		log_changes(m_shiftreg16, oldvals, "I_SHIFTREG16");
		for (int index = 0; index < 16; index++)
			if (m_shiftreg16_input[index] != nullptr)
				m_shiftreg16_input[index]->write_line(BIT(m_shiftreg16, index));
		shiftreg16_changed(m_shiftreg16, oldvals);
	}
}

void cinemat_audio_device::shiftreg_set(int bit, int val)
{
	u8 oldvals = m_shiftreg;
	u8 mask = 1 << bit;
	m_shiftreg = (m_shiftreg & ~mask) | ((val & 1) << bit);
	if (oldvals != m_shiftreg)
	{
		log_changes(m_shiftreg, oldvals, "I_SHIFTREG");
		shiftreg_changed(m_shiftreg, oldvals);
	}
}

void cinemat_audio_device::inputs_changed(u8 newvals, u8 oldvals)
{
	// overridden by base class if needed
}

void cinemat_audio_device::shiftreg_changed(u8 newvals, u8 oldvals)
{
	// overridden by base class if needed
}

void cinemat_audio_device::shiftreg16_changed(u16 newvals, u16 oldvals)
{
	// overridden by base class if needed
}

u8 cinemat_audio_device::shiftreg_swizzle(u8 rawvals)
{
	// overridden if needed by base class
	return rawvals;
}



/*************************************
 *
 *  Space Wars
 *
 *************************************/

DEFINE_DEVICE_TYPE(SPACE_WARS_AUDIO, spacewar_audio_device, "spacewar_audio", "Space Wars Sound Board")

spacewar_audio_device::spacewar_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, SPACE_WARS_AUDIO, tag, owner, clock, 0x1f)
{
}

void spacewar_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(spacewar))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_3", "I_OUT_3.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_4", "I_OUT_4.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(150000.0, 0.0);
}



/*************************************
 *
 *  Barrier
 *
 *************************************/

DEFINE_DEVICE_TYPE(BARRIER_AUDIO, barrier_audio_device, "barrier_audio", "Barrier Sound Board")

barrier_audio_device::barrier_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, BARRIER_AUDIO, tag, owner, clock, 0x07)
{
}

void barrier_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(barrier))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(200000.0, 0.0);
}



/*************************************
 *
 *  Speed Freak
 *
 *************************************/

DEFINE_DEVICE_TYPE(SPEED_FREAK_AUDIO, speedfrk_audio_device, "speedfrk_audio", "Speed Freak Sound Board")

speedfrk_audio_device::speedfrk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, SPEED_FREAK_AUDIO, tag, owner, clock, 0x9f)
{
}

void speedfrk_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(speedfrk))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_3", "I_OUT_3.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_4", "I_OUT_4.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_7", "I_OUT_7.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(12000.0, 0.0);
}



/*************************************
 *
 *  Star Hawk
 *
 *************************************/

DEFINE_DEVICE_TYPE(STAR_HAWK_AUDIO, starhawk_audio_device, "starhawk_audio", "Star Hawk Sound Board")

starhawk_audio_device::starhawk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, STAR_HAWK_AUDIO, tag, owner, clock, 0x9f)
{
}

void starhawk_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(starhawk))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_3", "I_OUT_3.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_4", "I_OUT_4.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_7", "I_OUT_7.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(50000.0, 0.0);
}



/*************************************
 *
 *  Sundance
 *
 *************************************/

DEFINE_DEVICE_TYPE(SUNDANCE_AUDIO, sundance_audio_device, "sundance_audio", "Sundance Sound Board")

sundance_audio_device::sundance_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, SUNDANCE_AUDIO, tag, owner, clock, 0x9f)
{
}

void sundance_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(sundance))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_3", "I_OUT_3.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_4", "I_OUT_4.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_7", "I_OUT_7.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(30000.0 * 4.0, 0.0);
}



/*************************************
 *
 *  Tail Gunner
 *
 *************************************/

DEFINE_DEVICE_TYPE(TAIL_GUNNER_AUDIO, tailg_audio_device, "tailg_audio", "Tail Gunner Sound Board")

tailg_audio_device::tailg_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, TAIL_GUNNER_AUDIO, tag, owner, clock, 0x1f)
{
}

void tailg_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(tailg))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_3", "I_OUT_3.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_4", "I_OUT_4.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(75000.0, 0.0);
}



/*************************************
 *
 *  Warrior
 *
 *************************************/

DEFINE_DEVICE_TYPE(WARRIOR_AUDIO, warrior_audio_device, "warrior_audio", "Warrior Sound Board")

warrior_audio_device::warrior_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, WARRIOR_AUDIO, tag, owner, clock, 0x1f)
{
}

void warrior_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	static const char *const sample_names[] =
	{
		"*warrior",
		"bgmhum1",
		"bgmhum2",
		"killed",
		"fall",
		"appear",
		nullptr
	};

	SAMPLES(config, m_samples);
	m_samples->set_channels(5);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void warrior_audio_device::inputs_changed(u8 curvals, u8 oldvals)
{
	// normal level - 0=on, 1=off
	if (falling_edge(curvals, oldvals, 0))
		m_samples->start(0, 0, true);
	if (rising_edge(curvals, oldvals, 0))
		m_samples->stop(0);

	// hi level - 0=on, 1=off
	if (falling_edge(curvals, oldvals, 1))
		m_samples->start(1, 1, true);
	if (rising_edge(curvals, oldvals, 1))

	// explosion - falling edge
	if (falling_edge(curvals, oldvals, 2))
		m_samples->start(2, 2);

	// fall - falling edge
	if (falling_edge(curvals, oldvals, 3))
		m_samples->start(3, 3);

	// appear - falling edge
	if (falling_edge(curvals, oldvals, 4))
		m_samples->start(4, 4);
}



/*************************************
 *
 *  Armor Attack
 *
 *************************************/

DEFINE_DEVICE_TYPE(ARMOR_ATTACK_AUDIO, armora_audio_device, "armora_audio", "Armor Atrack Sound Board")

armora_audio_device::armora_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, ARMOR_ATTACK_AUDIO, tag, owner, clock, 0x0e)
{
}

void armora_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	static const char *const sample_names[] =
	{
		"*armora",
		"loexp",
		"jeepfire",
		"hiexp",
		"tankfire",
		"tankeng",
		"beep",
		"chopper",
		nullptr
	};

	SAMPLES(config, m_samples);
	m_samples->set_channels(7);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void armora_audio_device::inputs_changed(u8 curvals, u8 oldvals)
{
	// execute on the rising edge of bit 0x01
	if (rising_edge(curvals, oldvals, 0))
		shiftreg_latch();

	// on the rising edge of bit 4, clock bit 0x80 into the shift register
	if (rising_edge(curvals, oldvals, 4))
		shiftreg_clock(curvals >> 7);

	// tank sound - 0=on, 1=off
	// still not totally correct - should be multiple speeds based on remaining bits in shift reg
	if (falling_edge(curvals, oldvals, 1))
		m_samples->start(4, 4, true);
	if (rising_edge(curvals, oldvals, 1))
		m_samples->stop(4);

	// beep sound - 0=on, 1=off
	if (falling_edge(curvals, oldvals, 2))
		m_samples->start(5, 5, true);
	if (rising_edge(curvals, oldvals, 2))
		m_samples->stop(5);

	// chopper sound - 0=on, 1=off */
	if (falling_edge(curvals, oldvals, 3))
		m_samples->start(6, 6, true);
	if (rising_edge(curvals, oldvals, 3))
		m_samples->stop(6);
}

void armora_audio_device::shiftreg_changed(u8 curvals, u8 oldvals)
{
	// bits 0-4 control the tank sound speed

	// lo explosion - falling edge
	if (falling_edge(curvals, oldvals, 4))
		m_samples->start(0, 0);

	// jeep fire - falling edge
	if (falling_edge(curvals, oldvals, 5))
		m_samples->start(1, 1);

	// hi explosion - falling edge
	if (falling_edge(curvals, oldvals, 6))
		m_samples->start(2, 2);

	// tank fire - falling edge
	if (falling_edge(curvals, oldvals, 7))
		m_samples->start(3, 3);
}



/*************************************
 *
 *  Ripoff
 *
 *************************************/

DEFINE_DEVICE_TYPE(RIP_OFF_AUDIO, ripoff_audio_device, "ripoff_audio", "Rip Off Sound Board")

ripoff_audio_device::ripoff_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, RIP_OFF_AUDIO, tag, owner, clock, 0x98)
{
}

void ripoff_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	static const char *const sample_names[] =
	{
		"*ripoff",
		"bonuslvl",
		"eattack",
		"shipfire",
		"efire",
		"explosn",
		"bg1",
		"bg2",
		"bg3",
		"bg4",
		"bg5",
		"bg6",
		"bg7",
		"bg8",
		nullptr
	};

	SAMPLES(config, m_samples);
	m_samples->set_channels(6);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void ripoff_audio_device::inputs_changed(u8 curvals, u8 oldvals)
{
	// execute on the rising edge of bit 2, latch the shift register
	if (rising_edge(curvals, oldvals, 2))
		shiftreg_latch();

	// on the rising edge of bit 1, clock bit 0 into the shift register
	if (rising_edge(curvals, oldvals, 1))
		shiftreg_clock(curvals & 1);

	// torpedo - falling edge
	if (falling_edge(curvals, oldvals, 3))
		m_samples->start(2, 2);

	// laser - falling edge
	if (falling_edge(curvals, oldvals, 4))
		m_samples->start(3, 3);

	// explosion - falling edge
	if (falling_edge(curvals, oldvals, 7))
		m_samples->start(4, 4);
}

void ripoff_audio_device::shiftreg_changed(u8 curvals, u8 oldvals)
{
	// background - 0=on, 1=off, selected by bits 0x38
	if (falling_edge(curvals, oldvals, 2) || (((curvals ^ oldvals) & 0x38) && !BIT(curvals, 2)))
		m_samples->start(5, 5 + ((curvals >> 5) & 7), true);
	if (rising_edge(curvals, oldvals, 2))
		m_samples->stop(5);

	// beep - falling edge
	if (falling_edge(curvals, oldvals, 1))
		m_samples->start(0, 0);

	// motor - 0=on, 1=off
	if (falling_edge(curvals, oldvals, 0))
		m_samples->start(1, 1, true);
	if (rising_edge(curvals, oldvals, 0))
		m_samples->stop(1);
}



/*************************************
 *
 *  Star Castle
 *
 *************************************/

DEFINE_DEVICE_TYPE(STAR_CASTLE_AUDIO, starcas_audio_device, "starcas_audio", "Star Castle Sound Board")

starcas_audio_device::starcas_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, STAR_CASTLE_AUDIO, tag, owner, clock, 0x9f)
{
}

void starcas_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(starcas))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_3", "I_OUT_3.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_4", "I_OUT_4.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_7", "I_OUT_7.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(5000.0, 0.0);
}



/*************************************
 *
 *  Solar Quest
 *
 *************************************/

DEFINE_DEVICE_TYPE(SOLAR_QUEST_AUDIO, solarq_audio_device, "solarq_audio", "Solar Quest Sound Board")

solarq_audio_device::solarq_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, SOLAR_QUEST_AUDIO, tag, owner, clock, 0x00)
{
}

void solarq_audio_device::device_start()
{
	cinemat_audio_device::device_start();

	save_item(NAME(m_last_frame));
	save_item(NAME(m_current_volume));
	save_item(NAME(m_target_volume));
}

void solarq_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	static const char *const sample_names[] =
	{
		"*solarq",
		"bigexpl",
		"smexpl",
		"lthrust",
		"slaser",
		"pickup",
		"nuke2",
		"nuke1",
		"music",
		nullptr
	};

	SAMPLES(config, m_samples);
	m_samples->set_channels(8);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void solarq_audio_device::inputs_changed(u8 curvals, u8 oldvals)
{
	// execute on the rising edge of bit 1, latch the shift register
	if (rising_edge(curvals, oldvals, 1))
		shiftreg_latch();

	// clock music data on the rising edge of bit 0
	if (rising_edge(curvals, oldvals, 0))
		shiftreg16_latch();

	// on the rising edge of bit 4, clock bit 7 into the shift register */
	if (rising_edge(curvals, oldvals, 4))
	{
		shiftreg_clock(curvals >> 7);
		shiftreg16_clock(curvals >> 7);
	}
}

void solarq_audio_device::shiftreg_changed(u8 curvals, u8 oldvals)
{
	// loud explosion - falling edge
	if (falling_edge(curvals, oldvals, 7))
		m_samples->start(0, 0);

	// soft explosion - falling edge
	if (falling_edge(curvals, oldvals, 6))
		m_samples->start(1, 1);

	// thrust - 0=on, 1=off
	if (falling_edge(curvals, oldvals, 5))
	{
		m_target_volume = 1.0;
		if (!m_samples->playing(2))
			m_samples->start(2, 2, true);
	}
	if (rising_edge(curvals, oldvals, 5))
		m_target_volume = 0;

	// ramp the thrust volume
	u64 curframe = framenum();
	if (m_samples->playing(2) && curframe > m_last_frame)
	{
		if (m_current_volume > m_target_volume)
			m_current_volume -= 0.078f;
		if (m_current_volume < m_target_volume)
			m_current_volume += 0.078f;
		if (m_current_volume > 0)
			m_samples->set_volume(2, m_current_volume);
		else
			m_samples->stop(2);
		m_last_frame = curframe;
	}

	// fire - falling edge
	if (falling_edge(curvals, oldvals, 4))
		m_samples->start(3, 3);

	// capture - falling edge
	if (falling_edge(curvals, oldvals, 3))
		m_samples->start(4, 4);

	// nuke - 1=on, 0=off
	if (rising_edge(curvals, oldvals, 2))
		m_samples->start(5, 5, true);
	if (falling_edge(curvals, oldvals, 2))
		m_samples->stop(5);

	// photon - falling edge
	if (falling_edge(curvals, oldvals, 1))
		m_samples->start(6, 6);
}

void solarq_audio_device::shiftreg16_changed(u16 curvals, u16 oldvals)
{
	// start/stop the music sample on the high bit
	if (rising_edge(curvals, oldvals, 15))
		m_samples->start(7, 7, true);
	if (falling_edge(curvals, oldvals, 15))
		m_samples->stop(7);

	/* set the frequency */
	int freq = 56818.181818 / (4096 - (curvals & 0xfff));
	m_samples->set_frequency(7, 44100 * freq / 1050);

	/* set the volume */
	int vol = (~curvals >> 12) & 7;
	m_samples->set_volume(7, vol / 7.0);
}



/*************************************
 *
 *  Boxing Bugs
 *
 *************************************/

DEFINE_DEVICE_TYPE(BOXING_BUGS_AUDIO, boxingb_audio_device, "boxingb_audio", "Boxing Bugs Sound Board")

boxingb_audio_device::boxingb_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, RIP_OFF_AUDIO, tag, owner, clock, 0x0c)
{
}

void boxingb_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	static const char *const sample_names[] =
	{
		"*boxingb",
		"softexpl",
		"loudexpl",
		"chirp",
		"eggcrack",
		"bugpusha",
		"bugpushb",
		"bugdie",
		"beetle",
		"music",
		"cannon",
		"bounce",
		"bell",
		nullptr
	};

	SAMPLES(config, m_samples);
	m_samples->set_channels(12);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void boxingb_audio_device::inputs_changed(u8 curvals, u8 oldvals)
{
	// execute on the rising edge of bit 1
	if (rising_edge(curvals, oldvals, 1))
		shiftreg_latch();

	// execute on the rising edge of bit 0
	if (rising_edge(curvals, oldvals, 0))
		shiftreg16_latch();

	// on the rising edge of bit 4, clock bit 7 into the shift register
	if (rising_edge(curvals, oldvals, 4))
	{
		shiftreg_clock(curvals >> 7);
		shiftreg16_clock(curvals >> 7);
	}

	// bounce - rising edge
	if (rising_edge(curvals, oldvals, 2))
		m_samples->start(10, 10);

	// bell - falling edge
	if (falling_edge(curvals, oldvals, 3))
		m_samples->start(11, 11);
}

void boxingb_audio_device::shiftreg_changed(u8 curvals, u8 oldvals)
{
	// soft explosion - falling edge
	if (falling_edge(curvals, oldvals, 7))
		m_samples->start(0, 0);

	// loud explosion - falling edge
	if (falling_edge(curvals, oldvals, 6))
		m_samples->start(1, 1);

	// chirping birds - 0=on, 1=off
	if (falling_edge(curvals, oldvals, 5))
		m_samples->start(2, 2);
	if (rising_edge(curvals, oldvals, 5))
		m_samples->stop(2);

	// egg cracking - falling edge
	if (falling_edge(curvals, oldvals, 4))
		m_samples->start(3, 3);

	// bug pushing A - rising edge
	if (rising_edge(curvals, oldvals, 3))
		m_samples->start(4, 4);

	// bug pushing B - rising edge
	if (rising_edge(curvals, oldvals, 2))
		m_samples->start(5, 5);

	// bug dying - falling edge
	if (falling_edge(curvals, oldvals, 1))
		m_samples->start(6, 6);

	// beetle on screen - falling edge
	if (falling_edge(curvals, oldvals, 0))
		m_samples->start(7, 7);
}

void boxingb_audio_device::shiftreg16_changed(u16 curvals, u16 oldvals)
{
	// start/stop the music sample on the high bit
	if (rising_edge(curvals, oldvals, 15))
		m_samples->start(8, 8, true);
	if (falling_edge(curvals, oldvals, 15))
		m_samples->stop(8);

	// set the frequency
	int freq = 56818.181818 / (4096 - (curvals & 0xfff));
	m_samples->set_frequency(8, 44100 * freq / 1050);

	// set the volume
	int vol = (~curvals >> 12) & 3;
	m_samples->set_volume(8, vol / 3.0);

	// cannon - falling edge
	if (rising_edge(curvals, oldvals, 14))
		m_samples->start(9, 9);
}



/*************************************
 *
 *  War of the Worlds
 *
 *************************************/

DEFINE_DEVICE_TYPE(WAR_OF_THE_WORLDS_AUDIO, wotw_audio_device, "wotw_audio", "War of the Worlds Sound Board")

wotw_audio_device::wotw_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, WAR_OF_THE_WORLDS_AUDIO, tag, owner, clock, 0x0e)
{
}

void wotw_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(wotw))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_3", "I_OUT_3.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_4", "I_OUT_4.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:out_7", "I_OUT_7.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(5000.0, 0.0);
}



/*************************************
 *
 *  Demon
 *
 *************************************/

TIMER_CALLBACK_MEMBER( demon_state::synced_sound_w )
{
	m_sound_fifo[m_sound_fifo_in] = param;
	m_sound_fifo_in = (m_sound_fifo_in + 1) % 16;
}


WRITE_LINE_MEMBER(demon_state::demon_sound4_w)
{
	/* watch for a 0->1 edge on bit 4 ("shift in") to clock in the new data */
	if (state)
		machine().scheduler().synchronize(timer_expired_delegate(FUNC(demon_state::synced_sound_w), this), ~m_outlatch->output_state() & 0x0f);
}


uint8_t demon_state::sound_porta_r()
{
	/* bits 0-3 are the sound data; bit 4 is the data ready */
	return m_sound_fifo[m_sound_fifo_out] | ((m_sound_fifo_in != m_sound_fifo_out) << 4);
}


uint8_t demon_state::sound_portb_r()
{
	return m_last_portb_write;
}


void demon_state::sound_portb_w(uint8_t data)
{
	/* watch for a 0->1 edge on bit 0 ("shift out") to advance the data pointer */
	if ((data & 1) != (m_last_portb_write & 1) && (data & 1) != 0)
		m_sound_fifo_out = (m_sound_fifo_out + 1) % 16;

	/* watch for a 0->1 edge of bit 1 ("hard reset") to reset the FIFO */
	if ((data & 2) != (m_last_portb_write & 2) && (data & 2) != 0)
		m_sound_fifo_in = m_sound_fifo_out = 0;

	/* bit 2 controls the global mute */
	if ((data & 4) != (m_last_portb_write & 4))
		machine().sound().system_mute(data & 4);

	/* remember the last value written */
	m_last_portb_write = data;
}

void demon_state::sound_output_w(uint8_t data)
{
	logerror("sound_output = %02X\n", data);
}


void demon_state::sound_start()
{
	cinemat_state::sound_start();

	/* register for save states */
	save_item(NAME(m_sound_fifo));
	save_item(NAME(m_sound_fifo_in));
	save_item(NAME(m_sound_fifo_out));
	save_item(NAME(m_last_portb_write));
}

void demon_state::sound_reset()
{
	/* generic init */
	cinemat_state::sound_reset();

	/* reset the FIFO */
	m_sound_fifo_in = m_sound_fifo_out = 0;
	m_last_portb_write = 0xff;

	/* turn off channel A on AY8910 #0 because it is used as a low-pass filter */
	m_ay1->set_volume(0, 0);
}


void demon_state::demon_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x3000, 0x33ff).ram();
	map(0x4000, 0x4001).r(m_ay1, FUNC(ay8910_device::data_r));
	map(0x4002, 0x4003).w(m_ay1, FUNC(ay8910_device::data_address_w));
	map(0x5000, 0x5001).r("ay2", FUNC(ay8910_device::data_r));
	map(0x5002, 0x5003).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x6000, 0x6001).r("ay3", FUNC(ay8910_device::data_r));
	map(0x6002, 0x6003).w("ay3", FUNC(ay8910_device::data_address_w));
	map(0x7000, 0x7000).nopw();  /* watchdog? */
}


void demon_state::demon_sound_ports(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).w("ctc", FUNC(z80ctc_device::write));
	map(0x1c, 0x1f).w("ctc", FUNC(z80ctc_device::write));
}


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


void demon_state::demon_sound(machine_config &config)
{
	/* basic machine hardware */
	z80_device& audiocpu(Z80(config, "audiocpu", 3579545));
	audiocpu.set_daisy_config(daisy_chain);
	audiocpu.set_addrmap(AS_PROGRAM, &demon_state::demon_sound_map);
	audiocpu.set_addrmap(AS_IO, &demon_state::demon_sound_ports);

	z80ctc_device& ctc(Z80CTC(config, "ctc", 3579545 /* same as "audiocpu" */));
	ctc.intr_callback().set_inputline("audiocpu", INPUT_LINE_IRQ0);

	m_outlatch->q_out_cb<4>().set(FUNC(demon_state::demon_sound4_w));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	AY8910(config, m_ay1, 3579545);
	m_ay1->port_a_read_callback().set(FUNC(demon_state::sound_porta_r));
	m_ay1->port_b_read_callback().set(FUNC(demon_state::sound_portb_r));
	m_ay1->port_b_write_callback().set(FUNC(demon_state::sound_portb_w));
	m_ay1->add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, "ay2", 3579545).add_route(ALL_OUTPUTS, "mono", 0.25);

	ay8910_device &ay3(AY8910(config, "ay3", 3579545));
	ay3.port_b_write_callback().set(FUNC(demon_state::sound_output_w));
	ay3.add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*************************************
 *
 *  QB3
 *
 *************************************/

void qb3_state::qb3_sound_fifo_w(uint8_t data)
{
	uint16_t rega = m_maincpu->state_int(ccpu_cpu_device::CCPU_A);
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(qb3_state::synced_sound_w), this), rega & 0x0f);
}


void qb3_state::sound_reset()
{
	demon_state::sound_reset();

	/* this patch prevents the sound ROM from eating itself when command $0A is sent */
	/* on a cube rotate */
	memregion("audiocpu")->base()[0x11dc] = 0x09;
}


void qb3_state::qb3_sound(machine_config &config)
{
	demon_sound(config);
	m_outlatch->q_out_cb<4>().set_nop(); // not mapped through LS259
}
