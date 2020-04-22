// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    National Semiconductor MM5837

    Digital Noise Source

***************************************************************************/

#include "emu.h"
#include "mm5837.h"


//**************************************************************************
//  CONSTEXPR DEFINITIONS
//**************************************************************************

constexpr int mm5837_device::m_frequency[];


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(MM5837, mm5837_device, "mm5837", "MM5837 Digital Noise Source")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mm5837_device - constructor
//-------------------------------------------------

mm5837_device::mm5837_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MM5837, tag, owner, clock),
	m_output_cb(*this),
	m_vdd(0),
	m_timer(nullptr),
	m_shift(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm5837_device::device_start()
{
	// resolve callbacks
	m_output_cb.resolve_safe();

	// get timer
	m_timer = timer_alloc(0);

	// register for save states
	save_item(NAME(m_shift));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mm5837_device::device_reset()
{
	// initialize with something
	m_shift = 123456;

	if (m_vdd < 16)
		m_timer->adjust(attotime::zero, 0, attotime::from_hz(m_frequency[m_vdd]));
	else
		throw emu_fatalerror("%s: Invalid voltage: %d\n", tag(), m_vdd);
}

//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void mm5837_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int tap_14 = BIT(m_shift, 13);
	int tap_17 = BIT(m_shift, 16);
	int zero = (m_shift == 0) ? 1 : 0;

	m_shift <<= 1;
	m_shift |= tap_14 ^ tap_17 ^ zero;

	m_output_cb(BIT(m_shift, 16));
}
