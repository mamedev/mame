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

#include "audio/nl_armora.h"
#include "audio/nl_barrier.h"
#include "audio/nl_boxingb.h"
#include "audio/nl_ripoff.h"
#include "audio/nl_solarq.h"
#include "audio/nl_spacewar.h"
#include "audio/nl_speedfrk.h"
#include "audio/nl_starcas.h"
#include "audio/nl_starhawk.h"
#include "audio/nl_sundance.h"
#include "audio/nl_tailg.h"
#include "audio/nl_warrior.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "speaker.h"


/*************************************
 *
 *  Base class
 *
 *************************************/

cinemat_audio_device::cinemat_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, u8 inputs_mask, void (*netlist)(netlist::nlparse_t &), double output_scale)
	: device_t(mconfig, type, tag, owner, clock)
	, m_samples(*this, "samples")
	, m_out_input(*this, "sound_nl:out_%u", 0)
	, m_inputs_mask(inputs_mask)
	, m_netlist(netlist)
	, m_output_scale(output_scale)
{
}

void cinemat_audio_device::configure_latch_inputs(ls259_device &latch, u8 mask)
{
	if (mask == 0)
		mask = m_inputs_mask;
	if (BIT(mask, 0))
		latch.q_out_cb<0>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<0>)));
	if (BIT(mask, 1))
		latch.q_out_cb<1>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<1>)));
	if (BIT(mask, 2))
		latch.q_out_cb<2>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<2>)));
	if (BIT(mask, 3))
		latch.q_out_cb<3>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<3>)));
	if (BIT(mask, 4))
		latch.q_out_cb<4>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<4>)));
	if (BIT(mask, 7))
		latch.q_out_cb<7>().set(write_line_delegate(*this, FUNC(cinemat_audio_device::sound_w<7>)));
}

void cinemat_audio_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	if (m_netlist != nullptr)
	{
		NETLIST_SOUND(config, "sound_nl", 48000)
			.set_source(m_netlist)
			.add_route(ALL_OUTPUTS, "mono", 1.0);

		if ((m_inputs_mask & 0x01) != 0)
			NETLIST_LOGIC_INPUT(config, "sound_nl:out_0", "I_OUT_0.IN", 0);
		if ((m_inputs_mask & 0x02) != 0)
			NETLIST_LOGIC_INPUT(config, "sound_nl:out_1", "I_OUT_1.IN", 0);
		if ((m_inputs_mask & 0x04) != 0)
			NETLIST_LOGIC_INPUT(config, "sound_nl:out_2", "I_OUT_2.IN", 0);
		if ((m_inputs_mask & 0x08) != 0)
			NETLIST_LOGIC_INPUT(config, "sound_nl:out_3", "I_OUT_3.IN", 0);
		if ((m_inputs_mask & 0x10) != 0)
			NETLIST_LOGIC_INPUT(config, "sound_nl:out_4", "I_OUT_4.IN", 0);
		if ((m_inputs_mask & 0x80) != 0)
			NETLIST_LOGIC_INPUT(config, "sound_nl:out_7", "I_OUT_7.IN", 0);

		NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(m_output_scale, 0.0);
	}
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
	m_shiftreg = m_shiftreg_accum;
	if (oldvals != m_shiftreg)
		shiftreg_changed(m_shiftreg, oldvals);
}

void cinemat_audio_device::shiftreg16_latch()
{
	u16 oldvals = m_shiftreg16;
	m_shiftreg16 = m_shiftreg16_accum;
	if (oldvals != m_shiftreg16)
		shiftreg16_changed(m_shiftreg16, oldvals);
}

void cinemat_audio_device::shiftreg_set(int bit, int val)
{
	u8 oldvals = m_shiftreg;
	u8 mask = 1 << bit;
	m_shiftreg = (m_shiftreg & ~mask) | ((val & 1) << bit);
		shiftreg_changed(m_shiftreg, oldvals);
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



/*************************************
 *
 *  Space Wars
 *
 *************************************/

DEFINE_DEVICE_TYPE(SPACE_WARS_AUDIO, spacewar_audio_device, "spacewar_audio", "Space Wars Sound Board")

spacewar_audio_device::spacewar_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, SPACE_WARS_AUDIO, tag, owner, clock, 0x1f, NETLIST_NAME(spacewar), 150000.0)
{
}



/*************************************
 *
 *  Barrier
 *
 *************************************/

DEFINE_DEVICE_TYPE(BARRIER_AUDIO, barrier_audio_device, "barrier_audio", "Barrier Sound Board")

barrier_audio_device::barrier_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, BARRIER_AUDIO, tag, owner, clock, 0x07, NETLIST_NAME(barrier), 200000.0)
{
}



/*************************************
 *
 *  Speed Freak
 *
 *************************************/

DEFINE_DEVICE_TYPE(SPEED_FREAK_AUDIO, speedfrk_audio_device, "speedfrk_audio", "Speed Freak Sound Board")

speedfrk_audio_device::speedfrk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, SPEED_FREAK_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(speedfrk), 12000.0)
{
}



/*************************************
 *
 *  Star Hawk
 *
 *************************************/

DEFINE_DEVICE_TYPE(STAR_HAWK_AUDIO, starhawk_audio_device, "starhawk_audio", "Star Hawk Sound Board")

starhawk_audio_device::starhawk_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, STAR_HAWK_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(starhawk), 50000.0)
{
}



/*************************************
 *
 *  Sundance
 *
 *************************************/

DEFINE_DEVICE_TYPE(SUNDANCE_AUDIO, sundance_audio_device, "sundance_audio", "Sundance Sound Board")

sundance_audio_device::sundance_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, SUNDANCE_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(sundance), 45000.0)
{
}



/*************************************
 *
 *  Tail Gunner
 *
 *************************************/

DEFINE_DEVICE_TYPE(TAIL_GUNNER_AUDIO, tailg_audio_device, "tailg_audio", "Tail Gunner Sound Board")

tailg_audio_device::tailg_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, TAIL_GUNNER_AUDIO, tag, owner, clock, 0x1f, NETLIST_NAME(tailg), 75000.0)
{
}



/*************************************
 *
 *  Warrior
 *
 *************************************/

DEFINE_DEVICE_TYPE(WARRIOR_AUDIO, warrior_audio_device, "warrior_audio", "Warrior Sound Board")

warrior_audio_device::warrior_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, WARRIOR_AUDIO, tag, owner, clock, 0x1f, NETLIST_NAME(warrior), 50000.0)
{
}



/*************************************
 *
 *  Armor Attack
 *
 *************************************/

DEFINE_DEVICE_TYPE(ARMOR_ATTACK_AUDIO, armora_audio_device, "armora_audio", "Armor Atrack Sound Board")

armora_audio_device::armora_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, ARMOR_ATTACK_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(armora), 5000.0)
{
}



/*************************************
 *
 *  Ripoff
 *
 *************************************/

DEFINE_DEVICE_TYPE(RIPOFF_AUDIO, ripoff_audio_device, "ripoff_audio", "Rip Off Sound Board")

ripoff_audio_device::ripoff_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, RIPOFF_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(ripoff), 20000.0)
{
}



/*************************************
 *
 *  Star Castle
 *
 *************************************/

DEFINE_DEVICE_TYPE(STAR_CASTLE_AUDIO, starcas_audio_device, "starcas_audio", "Star Castle Sound Board")

starcas_audio_device::starcas_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, STAR_CASTLE_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(starcas), 5000.0)
{
}



/*************************************
 *
 *  Solar Quest
 *
 *************************************/

DEFINE_DEVICE_TYPE(SOLAR_QUEST_AUDIO, solarq_audio_device, "solarq_audio", "Solar Quest Sound Board")

solarq_audio_device::solarq_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, SOLAR_QUEST_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(solarq), 5000.0)
{
}



/*************************************
 *
 *  Boxing Bugs
 *
 *************************************/

DEFINE_DEVICE_TYPE(BOXING_BUGS_AUDIO, boxingb_audio_device, "boxingb_audio", "Boxing Bugs Sound Board")

boxingb_audio_device::boxingb_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, BOXING_BUGS_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(boxingb), 5000.0)
{
}



/*************************************
 *
 *  War of the Worlds
 *
 *************************************/

DEFINE_DEVICE_TYPE(WAR_OF_THE_WORLDS_AUDIO, wotw_audio_device, "wotw_audio", "War of the Worlds Sound Board")

wotw_audio_device::wotw_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cinemat_audio_device(mconfig, WAR_OF_THE_WORLDS_AUDIO, tag, owner, clock, 0x9f, NETLIST_NAME(wotw), 5000.0)
{
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
