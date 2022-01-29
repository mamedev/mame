// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Blue Alpha Sound Sampler for SAM Coupe

***************************************************************************/

#include "emu.h"
#include "machine/rescap.h"
#include "blue_sampler.h"
#include "speaker.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAM_BLUE_SOUND_SAMPLER, sam_blue_sound_sampler_device, "blue_sampler", "Blue Alpha Sound Sampler")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sam_blue_sound_sampler_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	I8255A(config, m_ppi);
	m_ppi->out_pa_callback().set("dac", FUNC(dac_byte_interface::data_w));
	m_ppi->out_pb_callback().set(FUNC(sam_blue_sound_sampler_device::ppi_portb_w));
	m_ppi->in_pc_callback().set(FUNC(sam_blue_sound_sampler_device::ppi_portc_r));

	ZN426E(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.5);

	// TODO: ZN449E ADC

	// 555 with variable resistor ~19.342hz
	TIMER(config, "timer").configure_periodic(FUNC(sam_blue_sound_sampler_device::clock_callback), PERIOD_OF_555_MONOSTABLE(RES_K(1), CAP_N(47)));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sam_blue_sound_sampler_device - constructor
//-------------------------------------------------

sam_blue_sound_sampler_device::sam_blue_sound_sampler_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAM_BLUE_SOUND_SAMPLER, tag, owner, clock),
	device_samcoupe_expansion_interface(mconfig, *this),
	m_ppi(*this, "ppi"),
	m_dac(*this, "dac"),
	m_portc(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sam_blue_sound_sampler_device::device_start()
{
	// register for savestates
	save_item(NAME(m_portc));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

TIMER_DEVICE_CALLBACK_MEMBER( sam_blue_sound_sampler_device::clock_callback )
{
	m_portc ^= 1;
}

void sam_blue_sound_sampler_device::ppi_portb_w(uint8_t data)
{
	// 765432--  not used
	// ------1-  enable for adc
	// -------0  enable for dac and ppi pc6

	m_ppi->pc6_w(BIT(data, 0));
}

uint8_t sam_blue_sound_sampler_device::ppi_portc_r()
{
	// 7-------  not used
	// -6------  from ppi pb0
	// --5-----  not used
	// ---4----  busy output from zn449e adc
	// ----321-  not used
	// --------  clock input from 555

	return m_portc;
}

uint8_t sam_blue_sound_sampler_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	if ((offset & 0xff) == 0x7f)
		return m_ppi->read(offset >> 8);

	return data;
}

void sam_blue_sound_sampler_device::iorq_w(offs_t offset, uint8_t data)
{
	if ((offset & 0xff) == 0x7f)
		m_ppi->write(offset >> 8, data);
}
