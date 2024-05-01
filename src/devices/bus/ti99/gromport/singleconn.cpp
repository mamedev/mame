// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    Single: the standard console connector, one cartridge

***************************************************************************/

#include "emu.h"
#include "singleconn.h"

DEFINE_DEVICE_TYPE(TI99_GROMPORT_SINGLE, bus::ti99::gromport::ti99_single_cart_conn_device, "ti99_scartconn", "TI-99 Standard cartridge connector")

namespace bus::ti99::gromport {

ti99_single_cart_conn_device::ti99_single_cart_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cartridge_connector_device(mconfig, TI99_GROMPORT_SINGLE, tag, owner, clock),
	m_cartridge(*this, "cartridge")
{
}

void ti99_single_cart_conn_device::readz(offs_t offset, uint8_t *value)
{
	// Pass through
	m_cartridge->readz(offset, value);
}

void ti99_single_cart_conn_device::write(offs_t offset, uint8_t data)
{
	// Pass through
	m_cartridge->write(offset, data);
}

void ti99_single_cart_conn_device::crureadz(offs_t offset, uint8_t *value)
{
	// Pass through
	m_cartridge->crureadz(offset, value);
}

void ti99_single_cart_conn_device::cruwrite(offs_t offset, uint8_t data)
{
	// Pass through
	m_cartridge->cruwrite(offset, data);
}

void ti99_single_cart_conn_device::romgq_line(int state)
{
	// Pass through
	m_cartridge->romgq_line(state);
}

/*
    Combined select lines
*/
void ti99_single_cart_conn_device::set_gromlines(line_state mline, line_state moline, line_state gsq)
{
	// Pass through
	m_cartridge->set_gromlines(mline, moline, gsq);
}


void ti99_single_cart_conn_device::gclock_in(int state)
{
	// Pass through
	m_cartridge->gclock_in(state);
}

/*
    Check whether the GROMs are idle.
*/
bool ti99_single_cart_conn_device::is_grom_idle()
{
	return m_cartridge->is_grom_idle();
}

void ti99_single_cart_conn_device::device_add_mconfig(machine_config &config)
{
	TI99_CART(config, m_cartridge, 0);
	m_cartridge->set_connector(this);
}

} // end namespace bus::ti99::gromport
