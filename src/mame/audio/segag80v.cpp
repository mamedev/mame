// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega vector hardware

*************************************************************************/

#include "emu.h"
#include "includes/segag80v.h"

#include "audio/nl_elim.h"
#include "sound/samples.h"



/*************************************
 *
 *  Base class
 *
 *************************************/

segag80_audio_device::segag80_audio_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, void (*netlist)(netlist::nlparse_t &), double output_scale)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this)
	, m_lo_input(*this, "sound_nl:lo_%u", 0)
	, m_hi_input(*this, "sound_nl:hi_%u", 0)
	, m_netlist(netlist)
	, m_output_scale(output_scale)
{
}

void segag80_audio_device::device_add_mconfig(machine_config &config)
{
	if (m_netlist != nullptr)
	{
		NETLIST_SOUND(config, "sound_nl", 48000)
			.set_source(m_netlist)
			.add_route(ALL_OUTPUTS, *this, 1.0);

		NETLIST_LOGIC_INPUT(config, m_lo_input[1], "I_LO_D1.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_lo_input[2], "I_LO_D2.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_lo_input[3], "I_LO_D3.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_lo_input[4], "I_LO_D4.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_lo_input[5], "I_LO_D5.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_lo_input[6], "I_LO_D6.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_lo_input[7], "I_LO_D7.IN", 0);

		NETLIST_LOGIC_INPUT(config, m_hi_input[0], "I_HI_D0.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_hi_input[1], "I_HI_D1.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_hi_input[2], "I_HI_D2.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_hi_input[3], "I_HI_D3.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_hi_input[4], "I_HI_D4.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_hi_input[5], "I_HI_D5.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_hi_input[6], "I_HI_D6.IN", 0);
		NETLIST_LOGIC_INPUT(config, m_hi_input[7], "I_HI_D7.IN", 0);

		NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(m_output_scale, 0.0);
	}
}

void segag80_audio_device::device_start()
{
#if ENABLE_NETLIST_LOGGING
	m_logfile = fopen("netlist.csv", "w");
#endif
}

void segag80_audio_device::device_stop()
{
#if ENABLE_NETLIST_LOGGING
	if (m_logfile != nullptr)
		fclose(m_logfile);
#endif
}

void segag80_audio_device::write(offs_t addr, uint8_t data)
{
	addr &= 1;

	auto &inputs = (addr == 0) ? m_lo_input : m_hi_input;
	auto &oldvals = (addr == 0) ? m_lo_vals : m_hi_vals;

	for (int bit = (addr == 0) ? 1 : 0; bit < 8; bit++)
	{
		inputs[bit]->write_line(BIT(data, bit));
#if ENABLE_NETLIST_LOGGING
		if (BIT((data ^ oldvals), bit) != 0)
		{
			attotime time = machine().scheduler().time();
			fprintf(m_logfile, "%s,I_%s_%u.IN,%d\n", time.as_string(), (addr == 0) ? "LO" : "HI", bit, BIT(data, bit));
			printf("%s,I_%s_%u.IN,%d\n", time.as_string(), (addr == 0) ? "LO" : "HI", bit, BIT(data, bit));
		}
#endif
	}
	oldvals = data;
}



/*************************************
 *
 *  Eliminator
 *
 *************************************/

DEFINE_DEVICE_TYPE(ELIMINATOR_AUDIO, elim_audio_device, "elim_audio", "Eliminator Sound Board")

elim_audio_device::elim_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: segag80_audio_device(mconfig, ELIMINATOR_AUDIO, tag, owner, clock, NETLIST_NAME(elim), 5000.0)
{
}



/*************************************
 *
 *  Zektor
 *
 *************************************/

DEFINE_DEVICE_TYPE(ZEKTOR_AUDIO, zektor_audio_device, "zektor_audio", "Zektor Sound Board")

zektor_audio_device::zektor_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: segag80_audio_device(mconfig, ZEKTOR_AUDIO, tag, owner, clock, NETLIST_NAME(elim), 5000.0)
{
}



/*************************************
 *
 *  Space Fury
 *
 *************************************/

DEFINE_DEVICE_TYPE(SPACE_FURY_AUDIO, spcfury_audio_device, "spcfury_audio", "Space Fury Sound Board")

spcfury_audio_device::spcfury_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: segag80_audio_device(mconfig, SPACE_FURY_AUDIO, tag, owner, clock, nullptr, 5000.0)
{
}




#if 0

void segag80v_state::elim1_sh_w(uint8_t data)
{
#if 0
	write_log(machine(), 35, data);

	data ^= 0xff;

	/* Play fireball sample */
	if (data & 0x02)
		m_samples->start(0, 0);

	/* Play explosion samples */
	if (data & 0x04)
		m_samples->start(1, 10);
	if (data & 0x08)
		m_samples->start(1, 9);
	if (data & 0x10)
		m_samples->start(1, 8);

	/* Play bounce sample */
	if (data & 0x20)
	{
		if (m_samples->playing(2))
			m_samples->stop(2);
		m_samples->start(2, 1);
	}

	/* Play lazer sample */
	if (data & 0xc0)
	{
		if (m_samples->playing(3))
			m_samples->stop(3);
		m_samples->start(3, 5);
	}
#endif
}

void segag80v_state::elim2_sh_w(uint8_t data)
{
#if 0
	write_log(machine(), 34, data);

	data ^= 0xff;

	/* Play thrust sample */
	if (data & 0x0f)
		m_samples->start(4, 6);
	else
		m_samples->stop(4);

	/* Play skitter sample */
	if (data & 0x10)
		m_samples->start(5, 2);

	/* Play eliminator sample */
	if (data & 0x20)
		m_samples->start(6, 3);

	/* Play electron samples */
	if (data & 0x40)
		m_samples->start(7, 7);
	if (data & 0x80)
		m_samples->start(7, 4);
#endif
}


void segag80v_state::zektor1_sh_w(uint8_t data)
{
#if 0
	data ^= 0xff;

	/* Play fireball sample */
	if (data & 0x02)
				m_samples->start(0, 0);

	/* Play explosion samples */
	if (data & 0x04)
				m_samples->start(1, 10);
	if (data & 0x08)
					m_samples->start(1, 9);
	if (data & 0x10)
					m_samples->start(1, 8);

	/* Play bounce sample */
	if (data & 0x20)
	{
				if (m_samples->playing(2))
						m_samples->stop(2);
				m_samples->start(2, 1);
	}

	/* Play lazer sample */
	if (data & 0xc0)
	{
		if (m_samples->playing(3))
			m_samples->stop(3);
				m_samples->start(3, 5);
	}
#endif
}

void segag80v_state::zektor2_sh_w(uint8_t data)
{
#if 0
	data ^= 0xff;

	/* Play thrust sample */
	if (data & 0x0f)
			m_samples->start(4, 6);
	else
		m_samples->stop(4);

	/* Play skitter sample */
	if (data & 0x10)
				m_samples->start(5, 2);

	/* Play eliminator sample */
	if (data & 0x20)
				m_samples->start(6, 3);

	/* Play electron samples */
	if (data & 0x40)
				m_samples->start(7, 40);
	if (data & 0x80)
				m_samples->start(7, 41);
#endif
}



void segag80v_state::spacfury1_sh_w(uint8_t data)
{
	data ^= 0xff;

	/* craft growing */
	if (data & 0x01)
		m_samples->start(1, 0);

	/* craft moving */
	if (data & 0x02)
	{
		if (!m_samples->playing(2))
			m_samples->start(2, 1, true);
	}
	else
		m_samples->stop(2);

	/* Thrust */
	if (data & 0x04)
	{
		if (!m_samples->playing(3))
			m_samples->start(3, 4, true);
	}
	else
		m_samples->stop(3);

	/* star spin */
	if (data & 0x40)
		m_samples->start(4, 8);

	/* partial warship? */
	if (data & 0x80)
		m_samples->start(4, 9);

}

void segag80v_state::spacfury2_sh_w(uint8_t data)
{
	data ^= 0xff;

	/* craft joining */
	if (data & 0x01)
		m_samples->start(5, 2);

	/* ship firing */
	if (data & 0x02)
	{
		if (m_samples->playing(6))
			m_samples->stop(6);
		m_samples->start(6, 3);

	}

	/* fireball */
	if (data & 0x04)
		m_samples->start(7, 6);

	/* small explosion */
	if (data & 0x08)
		m_samples->start(7, 6);
	/* large explosion */
	if (data & 0x10)
		m_samples->start(7, 5);

	/* docking bang */
	if (data & 0x20)
		m_samples->start(0, 7);

}
#endif
