// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2echoii.c

    Implementation of the Street Electronics Echo II speech card
    Ready logic traced by Jonathan Gevaryahu

*********************************************************************/

#include "emu.h"
#include "a2echoii.h"
#include "sound/tms5220.h"
#include "speaker.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_ECHOII, a2bus_echoii_device, "a2echoii", "Street Electronics Echo II")

#define TMS_TAG         "tms5220"

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(a2bus_echoii_device::device_add_mconfig)
	SPEAKER(config, "echoii").front_center();
	MCFG_DEVICE_ADD(TMS_TAG, TMS5220, 640000) // Note the Echo II card has a "FREQ" potentiometer which can be used to adjust the tms5220's clock frequency; 640khz is the '8khz' value according to the tms5220 datasheet
	MCFG_TMS52XX_READYQ_HANDLER(WRITELINE(*this, a2bus_echoii_device, ready_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "echoii", 1.0)
MACHINE_CONFIG_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_echoii_device::a2bus_echoii_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_echoii_device::a2bus_echoii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_echoii_device(mconfig, A2BUS_ECHOII, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_echoii_device::device_start()
{
	m_timer = timer_alloc(0);
	m_timer->adjust(attotime::never);

	save_item(NAME(m_latch));
	save_item(NAME(m_ready));
	save_item(NAME(m_byte_in_latch));
}

void a2bus_echoii_device::device_reset()
{
	m_byte_in_latch = false;
	m_ready = 0;
	m_latch = 0;
}

uint8_t a2bus_echoii_device::read_c0nx(uint8_t offset)
{
	switch (offset)
	{
		case 0:
			return 0x1f | m_tms->read_status();
	}

	return 0;
}

void a2bus_echoii_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_latch = data;
			if (!m_ready)
				m_tms->write_data(m_latch);
			else
				m_byte_in_latch = true;
			break;
	}
}

bool a2bus_echoii_device::take_c800()
{
	return false;
}

void a2bus_echoii_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_tms->write_data(m_latch);
	m_byte_in_latch = false;
	m_timer->adjust(attotime::never);
}

WRITE_LINE_MEMBER( a2bus_echoii_device::ready_w )
{
	// if /Ready falls, write the byte in the latch if there is one
	if ((m_ready) && (!state) && (m_byte_in_latch))
	{
		m_timer->adjust(attotime::zero);
	}

	m_ready = state;
}
