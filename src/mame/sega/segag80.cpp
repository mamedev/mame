// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega vector hardware

*************************************************************************/

#include "emu.h"
#include "segag80.h"

#include "segag80v.h" // FIXME: eliminate the need for this dependency

#include "nl_astrob.h"
#include "nl_elim.h"
#include "nl_spacfury.h"

#include "sound/samples.h"



/*************************************
 *
 *  Base class
 *
 *************************************/

segag80_audio_device_base::segag80_audio_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 lomask, u8 himask, bool haspsg, netlist_ptr netlist, double output_scale) :
	device_t(mconfig, type, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_lo_input(*this, "sound_nl:lo_%u", 0),
	m_hi_input(*this, "sound_nl:hi_%u", 0),
	m_psg(*this, "psg"),
	m_lo_mask(lomask),
	m_hi_mask(himask),
	m_has_psg(haspsg),
	m_netlist(netlist),
	m_output_scale(output_scale)
{
}

void segag80_audio_device_base::device_add_mconfig(machine_config &config)
{
	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(m_netlist)
		.add_route(ALL_OUTPUTS, *this, 1.0);

	if (BIT(m_lo_mask, 0))
		NETLIST_LOGIC_INPUT(config, m_lo_input[0], "I_LO_D0.IN", 0);
	if (BIT(m_lo_mask, 1))
		NETLIST_LOGIC_INPUT(config, m_lo_input[1], "I_LO_D1.IN", 0);
	if (BIT(m_lo_mask, 2))
		NETLIST_LOGIC_INPUT(config, m_lo_input[2], "I_LO_D2.IN", 0);
	if (BIT(m_lo_mask, 3))
		NETLIST_LOGIC_INPUT(config, m_lo_input[3], "I_LO_D3.IN", 0);
	if (BIT(m_lo_mask, 4))
		NETLIST_LOGIC_INPUT(config, m_lo_input[4], "I_LO_D4.IN", 0);
	if (BIT(m_lo_mask, 5))
		NETLIST_LOGIC_INPUT(config, m_lo_input[5], "I_LO_D5.IN", 0);
	if (BIT(m_lo_mask, 6))
		NETLIST_LOGIC_INPUT(config, m_lo_input[6], "I_LO_D6.IN", 0);
	if (BIT(m_lo_mask, 7))
		NETLIST_LOGIC_INPUT(config, m_lo_input[7], "I_LO_D7.IN", 0);

	if (BIT(m_hi_mask, 0))
		NETLIST_LOGIC_INPUT(config, m_hi_input[0], "I_HI_D0.IN", 0);
	if (BIT(m_hi_mask, 1))
		NETLIST_LOGIC_INPUT(config, m_hi_input[1], "I_HI_D1.IN", 0);
	if (BIT(m_hi_mask, 2))
		NETLIST_LOGIC_INPUT(config, m_hi_input[2], "I_HI_D2.IN", 0);
	if (BIT(m_hi_mask, 3))
		NETLIST_LOGIC_INPUT(config, m_hi_input[3], "I_HI_D3.IN", 0);
	if (BIT(m_hi_mask, 4))
		NETLIST_LOGIC_INPUT(config, m_hi_input[4], "I_HI_D4.IN", 0);
	if (BIT(m_hi_mask, 5))
		NETLIST_LOGIC_INPUT(config, m_hi_input[5], "I_HI_D5.IN", 0);
	if (BIT(m_hi_mask, 6))
		NETLIST_LOGIC_INPUT(config, m_hi_input[6], "I_HI_D6.IN", 0);
	if (BIT(m_hi_mask, 7))
		NETLIST_LOGIC_INPUT(config, m_hi_input[7], "I_HI_D7.IN", 0);

	if (m_has_psg)
	{
		AY8912(config, m_psg, VIDEO_CLOCK/4/2);
		m_psg->set_flags(AY8910_RESISTOR_OUTPUT);
		m_psg->set_resistors_load(10000.0, 10000.0, 10000.0);
		m_psg->add_route(0, "sound_nl", 1.0, 0);
		m_psg->add_route(1, "sound_nl", 1.0, 1);
		m_psg->add_route(2, "sound_nl", 1.0, 2);

		NETLIST_STREAM_INPUT(config, "sound_nl:cin0", 0, "R_PSG_1.R");
		NETLIST_STREAM_INPUT(config, "sound_nl:cin1", 1, "R_PSG_2.R");
		NETLIST_STREAM_INPUT(config, "sound_nl:cin2", 2, "R_PSG_3.R");
	}

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(m_output_scale, 0.0);
}

void segag80_audio_device_base::device_start()
{
}

void segag80_audio_device_base::write(offs_t addr, uint8_t data)
{
	addr &= 1;

	auto &inputs = (addr == 0) ? m_lo_input : m_hi_input;
	auto &mask = (addr == 0) ? m_lo_mask : m_hi_mask;

	for (int bit = 0; bit < 8; bit++)
		if (BIT(mask, bit))
			inputs[bit]->write_line(BIT(data, bit));
}

void segag80_audio_device_base::write_ay(offs_t addr, uint8_t data)
{
	assert(m_has_psg);
	m_psg->address_data_w(addr, data);
}



/*************************************
 *
 *  Eliminator
 *
 *************************************/

DEFINE_DEVICE_TYPE(ELIMINATOR_AUDIO, elim_audio_device, "elim_audio", "Eliminator Sound Board")

elim_audio_device::elim_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	segag80_audio_device_base(mconfig, ELIMINATOR_AUDIO, tag, owner, clock, 0xfe, 0xff, false, NETLIST_NAME(elim), 0.15)
{
}



/*************************************
 *
 *  Zektor
 *
 *************************************/

DEFINE_DEVICE_TYPE(ZEKTOR_AUDIO, zektor_audio_device, "zektor_audio", "Zektor Sound Board")

zektor_audio_device::zektor_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	segag80_audio_device_base(mconfig, ZEKTOR_AUDIO, tag, owner, clock, 0xfe, 0xff, true, NETLIST_NAME(zektor), 0.15)
{
}



/*************************************
 *
 *  Space Fury
 *
 *************************************/

DEFINE_DEVICE_TYPE(SPACE_FURY_AUDIO, spacfury_audio_device, "spcfury_audio", "Space Fury Sound Board")

spacfury_audio_device::spacfury_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	segag80_audio_device_base(mconfig, SPACE_FURY_AUDIO, tag, owner, clock, 0xc7, 0x3f, false, NETLIST_NAME(spacfury), 2.0)
{
}



/*************************************
 *
 *  Astro Blaster
 *
 *************************************/

DEFINE_DEVICE_TYPE(ASTRO_BLASTER_AUDIO, astrob_audio_device, "astrob_audio", "Astro Blaster Sound Board")

astrob_audio_device::astrob_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	segag80_audio_device_base(mconfig, ASTRO_BLASTER_AUDIO, tag, owner, clock, 0xff, 0xff, false, NETLIST_NAME(astrob), 1.0)
{
}
