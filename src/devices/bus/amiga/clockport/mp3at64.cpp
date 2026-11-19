// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MP3@64 MPEG audio decoder card

***************************************************************************/

#include "emu.h"
#include "mp3at64.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(AMIGA_CLOCKPORT_MP3AT64, amiga_clockport_mp3at64_device, "amiga_clockport_mp3at64", "MP3@64")

amiga_clockport_mp3at64_device::amiga_clockport_mp3at64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_CLOCKPORT_MP3AT64, tag, owner, clock),
	device_amiga_clockport_interface(mconfig, *this),
	m_mas(*this, "mas3507d"),
	m_demand(true)
{
}

void amiga_clockport_mp3at64_device::device_start()
{
	save_item(NAME(m_demand));
}

void amiga_clockport_mp3at64_device::device_reset()
{
	m_demand = true;
}

void amiga_clockport_mp3at64_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	MAS3507D(config, m_mas);
	m_mas->demand_cb().set(FUNC(amiga_clockport_mp3at64_device::demand_w));
	m_mas->add_route(0, "speaker", 1.0, 0);
	m_mas->add_route(1, "speaker", 1.0, 1);
}

void amiga_clockport_mp3at64_device::demand_w(int state)
{
	m_demand = state;
}

uint8_t amiga_clockport_mp3at64_device::spare_r(offs_t offset)
{
	switch (offset)
	{
	case 4: return m_demand ? 0x80 : 0x00;
	case 5: return m_mas->i2c_sda_r();
	case 6: return m_mas->i2c_scl_r();
	}

	return 0xff;
}

void amiga_clockport_mp3at64_device::spare_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 4: m_mas->sid_w(data); break;
	case 5: m_mas->i2c_sda_w(BIT(data, 0)); break;
	case 6: m_mas->i2c_scl_w(BIT(data, 0)); break;
	}
}
