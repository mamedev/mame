// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    SID Master 99
    Original circuit copyright 2010 by Marc Hull
    Board layout and board artwork copyright 2010 by Jim Fetzner

    This expansion card adds a SID sound generator chip to the TI system.

    The circuitry has an unconventional selection mechanism: It assumes to be
    active at all times, unless a CRU address 1xxx and higher appears on the
    bus. This requires that the 9901 addresses be visible on the bus. Also, it
    has to be verified whether the real Geneve works with the SID Master.

    The SID is mapped to addresses 5800-5836 (even addresses only), incompletely
    decoded (5800-5FFF).

    Michael Zapf
    September 2020

*****************************************************************************/

#include "emu.h"
#include "sidmaster.h"

#define LOG_WARN       (1U<<1)
#define LOG_WRITE      (1U<<2)

#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_SIDMASTER, bus::ti99::peb::sidmaster_device, "ti99_sidmaster", "SID Master 99")

namespace bus::ti99::peb {

sidmaster_device::sidmaster_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_SIDMASTER, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_sid(*this, "sid")
{
}

void sidmaster_device::write(offs_t offset, uint8_t data)
{
	if (m_selected && ((offset & 0xf800)==0x5800))
	{
		LOGMASKED(LOG_WRITE, "%04x <- %02x\n", offset, data);
		m_sid->write((offset >> 1)& 0x1f, data);
	}
}

/*
    CRU read access
    There is actually no CRU access; the card is deselected when CRU devices
    0x1000 and higher are accessed.
*/
void sidmaster_device::crureadz(offs_t offset, uint8_t *value)
{
	bool prevsel = m_selected;
	m_selected = ((offset & 0xf000) == 0x0000);
	if (prevsel && !m_selected)
		m_sid->reset();
}

/*
    CRU write access
    There is actually no CRU access; the card is deselected when CRU devices
    0x1000 and higher are accessed.
*/
void sidmaster_device::cruwrite(offs_t offset, uint8_t data)
{
	bool prevsel = m_selected;
	m_selected = ((offset & 0xf000) == 0x0000);
	if (prevsel && !m_selected)
		m_sid->reset();
}

void sidmaster_device::device_add_mconfig(machine_config &config)
{
	// sound hardware
	SPEAKER(config, "mono").front_center();
	MOS6581(config, m_sid, XTAL(17'734'472)/18);
	m_sid->add_route(ALL_OUTPUTS, "mono", 1.00);
}

void sidmaster_device::device_start()
{
}

void sidmaster_device::device_reset()
{
	m_sid->reset();
}


} // end namespace bus::ti99::peb
