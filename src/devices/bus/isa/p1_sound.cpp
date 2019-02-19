// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 sound card.  DAC, ADC, MIDI in/out and 6 music channels.

    Memory-mapped, uses IRQ3 and IRQ7, no DMA.

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "emu.h"
#include "p1_sound.h"

#include "sound/volt_reg.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P1_SOUND, p1_sound_device, "p1_sound", "Poisk-1 sound card (B623)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void p1_sound_device::device_add_mconfig(machine_config &config)
{
	I8251(config, m_midi, 0);
	m_midi->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_midi->rxrdy_handler().set(":isa", FUNC(isa8_device::irq3_w));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_midi, FUNC(i8251_device::write_rxd));

	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	PIT8253(config, m_d14, 0);
	m_d14->set_clk<0>(XTAL(12'500'000)/10);
//  sampler at 10 KHz
	m_d14->out_handler<0>().set(FUNC(p1_sound_device::sampler_sync));
	m_d14->set_clk<1>(XTAL(12'500'000)/10);
	m_d14->out_handler<1>().set(m_midi, FUNC(i8251_device::write_txc));
	m_d14->set_clk<2>(XTAL(12'500'000)/10);
	m_d14->out_handler<2>().set(m_midi, FUNC(i8251_device::write_rxc));

	PIT8253(config, m_d16, 0);
	m_d16->set_clk<0>(XTAL(12'500'000)/10);
//  m_d16->out_handler<0>().set(FUNC(XXX));
	m_d16->set_clk<1>(XTAL(12'500'000)/10);
//  m_d16->out_handler<1>().set(FUNC(XXX));
	m_d16->set_clk<2>(XTAL(12'500'000)/10);
//  m_d16->out_handler<2>().set(FUNC(XXX));

	PIT8253(config, m_d17, 0);
	m_d17->set_clk<0>(XTAL(12'500'000)/10);
//  m_d17->out_handler<0>().set(FUNC(XXX));
	m_d17->set_clk<1>(XTAL(12'500'000)/10);
//  m_d17->out_handler<1>().set(FUNC(XXX));
	m_d17->set_clk<2>(XTAL(12'500'000)/10);
//  m_d17->out_handler<2>().set(FUNC(XXX));

	SPEAKER(config, "speaker").front_center();
	FILTER_RC(config, m_filter).add_route(ALL_OUTPUTS, "speaker", 1.0);
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "filter", 0.5); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  p1_sound_device - constructor
//-------------------------------------------------

p1_sound_device::p1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P1_SOUND, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_dac(*this, "dac")
	, m_filter(*this, "filter")
	, m_midi(*this, "midi")
	, m_d14(*this, "d14")
	, m_d16(*this, "d16")
	, m_d17(*this, "d17")
{
}

READ8_MEMBER(p1_sound_device::d14_r)
{
	return m_d14->read(offset >> 1);
}

WRITE8_MEMBER(p1_sound_device::d14_w)
{
	m_d14->write(offset >> 1, data);
}

READ8_MEMBER(p1_sound_device::d16_r)
{
	return m_d16->read(offset >> 1);
}

WRITE8_MEMBER(p1_sound_device::d16_w)
{
	m_d16->write(offset >> 1, data);
}

READ8_MEMBER(p1_sound_device::d17_r)
{
	return m_d17->read(offset >> 1);
}

WRITE8_MEMBER(p1_sound_device::d17_w)
{
	m_d17->write(offset >> 1, data);
}

READ8_MEMBER(p1_sound_device::adc_r)
{
	return 0;
}

WRITE8_MEMBER(p1_sound_device::dac_w)
{
//  logerror("DAC write: %02x <- %02x\n", offset>>1, data);
	m_dac_data[offset >> 1] = data;
	m_isa->irq7_w(CLEAR_LINE);
}

WRITE_LINE_MEMBER(p1_sound_device::sampler_sync)
{
	if (state)
	{
		m_dac->write(m_dac_data[m_dac_ptr++]);
		m_dac_ptr &= 7;
		if ((m_dac_ptr % 8) == 0)
		{
			m_isa->irq7_w(state);
		}
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void p1_sound_device::device_start()
{
	set_isa_device();

	// E8000..E9FFF -- ROM (not installed on any known board)
	// EE400 -- ?? root tone measurement ??
	// EFC00 -- ADC output

	m_isa->install_memory(0xea000, 0xea01f,
		read8_delegate(FUNC(p1_sound_device::adc_r), this), // XXX not really
		write8_delegate(FUNC(p1_sound_device::dac_w), this));

	m_isa->install_memory(0xee000, 0xee000,
		read8smo_delegate(FUNC(i8251_device::data_r), m_midi.target()),
		write8smo_delegate(FUNC(i8251_device::data_w), m_midi.target()));
	m_isa->install_memory(0xee002, 0xee002,
		read8smo_delegate(FUNC(i8251_device::status_r), m_midi.target()),
		write8smo_delegate(FUNC(i8251_device::control_w), m_midi.target()));

	// sync generator
	m_isa->install_memory(0xef000, 0xef007,
		read8_delegate(FUNC(p1_sound_device::d14_r), this),
		write8_delegate(FUNC(p1_sound_device::d14_w), this));

	// 6 music channels
	m_isa->install_memory(0xef400, 0xef407,
		read8_delegate(FUNC(p1_sound_device::d16_r), this),
		write8_delegate(FUNC(p1_sound_device::d16_w), this));
	m_isa->install_memory(0xef800, 0xef807,
		read8_delegate(FUNC(p1_sound_device::d17_r), this),
		write8_delegate(FUNC(p1_sound_device::d17_w), this));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void p1_sound_device::device_reset()
{
	memset(m_dac_data, 0, sizeof(m_dac_data));
	m_dac_ptr = 0;

	// 5 kHz lowpass filter.  XXX check schematics
	m_filter->filter_rc_set_RC(filter_rc_device::LOWPASS, 330, 0, 0, CAP_N(100));
}
