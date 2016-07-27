// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 sound card.  DAC, ADC, MIDI in/out and 6 music channels.

    Memory-mapped, uses IRQ3 and IRQ7, no DMA.

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "p1_sound.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type P1_SOUND = &device_creator<p1_sound_device>;


//-------------------------------------------------
//  Machine config
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( p1_sound )
	MCFG_DEVICE_ADD("midi", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("mdout", midi_port_device, write_txd))
	MCFG_I8251_RXRDY_HANDLER(DEVWRITELINE(":isa", isa8_device, irq3_w))

	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_RX_HANDLER(DEVWRITELINE("midi", i8251_device, write_rxd))

	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")

	MCFG_DEVICE_ADD("d14", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_12_5MHz/10)
//  sampler at 10 KHz
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(p1_sound_device, sampler_sync))
	MCFG_PIT8253_CLK1(XTAL_12_5MHz/10)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("midi", i8251_device, write_txc))
	MCFG_PIT8253_CLK2(XTAL_12_5MHz/10)
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("midi", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("d16", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_12_5MHz/10)
//  MCFG_PIT8253_OUT0_HANDLER(XXX)
	MCFG_PIT8253_CLK1(XTAL_12_5MHz/10)
//  MCFG_PIT8253_OUT1_HANDLER(XXX)
	MCFG_PIT8253_CLK2(XTAL_12_5MHz/10)
//  MCFG_PIT8253_OUT2_HANDLER(XXX)

	MCFG_DEVICE_ADD("d17", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_12_5MHz/10)
//  MCFG_PIT8253_OUT0_HANDLER(XXX)
	MCFG_PIT8253_CLK1(XTAL_12_5MHz/10)
//  MCFG_PIT8253_OUT1_HANDLER(XXX)
	MCFG_PIT8253_CLK2(XTAL_12_5MHz/10)
//  MCFG_PIT8253_OUT2_HANDLER(XXX)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "filter", 1.0)
	MCFG_FILTER_RC_ADD("filter", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor p1_sound_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( p1_sound );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  p1_sound_device - constructor
//-------------------------------------------------

p1_sound_device::p1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, P1_SOUND, "Poisk-1 sound card (B623)", tag, owner, clock, "p1_sound", __FILE__),
	device_isa8_card_interface( mconfig, *this ),
	m_dac(*this, "dac"),
	m_filter(*this, "filter"),
	m_midi(*this, "midi"),
	m_d14(*this, "d14"),
	m_d16(*this, "d16"),
	m_d17(*this, "d17")
{
}

READ8_MEMBER( p1_sound_device::d14_r )
{
	return m_d14->read(space, offset>>1);
}

WRITE8_MEMBER( p1_sound_device::d14_w )
{
	m_d14->write(space, offset>>1, data);
}

READ8_MEMBER( p1_sound_device::d16_r )
{
	return m_d16->read(space, offset>>1);
}

WRITE8_MEMBER( p1_sound_device::d16_w )
{
	m_d16->write(space, offset>>1, data);
}

READ8_MEMBER( p1_sound_device::d17_r )
{
	return m_d17->read(space, offset>>1);
}

WRITE8_MEMBER( p1_sound_device::d17_w )
{
	m_d17->write(space, offset>>1, data);
}

READ8_MEMBER( p1_sound_device::adc_r )
{
	return 0;
}

WRITE8_MEMBER( p1_sound_device::dac_w )
{
//  logerror("DAC write: %02x <- %02x\n", offset>>1, data);
	m_dac_data[offset>>1] = data;
	m_isa->irq7_w(CLEAR_LINE);
}

WRITE_LINE_MEMBER( p1_sound_device::sampler_sync )
{
	if (state) {
		m_dac->write_unsigned8(m_dac_data[m_dac_ptr++]);
		m_dac_ptr &= 7;
		if ((m_dac_ptr % 8) == 0) {
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
		read8_delegate(FUNC(i8251_device::data_r), (i8251_device*)m_midi),
		write8_delegate(FUNC(i8251_device::data_w), (i8251_device*)m_midi));
	m_isa->install_memory(0xee002, 0xee002,
		read8_delegate(FUNC(i8251_device::status_r), (i8251_device*)m_midi),
		write8_delegate(FUNC(i8251_device::control_w), (i8251_device*)m_midi));

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
	m_filter->filter_rc_set_RC(FLT_RC_LOWPASS, 330, 0, 0, CAP_N(100) );
}
