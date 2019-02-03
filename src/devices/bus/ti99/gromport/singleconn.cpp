// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    Single: the standard console connector, one cartridge

***************************************************************************/

#include "emu.h"
#include "singleconn.h"

DEFINE_DEVICE_TYPE_NS(TI99_GROMPORT_SINGLE, bus::ti99::gromport, ti99_single_cart_conn_device, "ti99_scartconn", "TI-99 Standard cartridge connector")

namespace bus { namespace ti99 { namespace gromport {

ti99_single_cart_conn_device::ti99_single_cart_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cartridge_connector_device(mconfig, TI99_GROMPORT_SINGLE, tag, owner, clock),
	m_cartridge(nullptr)
{
}

READ8Z_MEMBER(ti99_single_cart_conn_device::readz)
{
	// Pass through
	m_cartridge->readz(space, offset, value);
}

WRITE8_MEMBER(ti99_single_cart_conn_device::write)
{
	// Pass through
	m_cartridge->write(space, offset, data);
}

READ8Z_MEMBER(ti99_single_cart_conn_device::crureadz)
{
	// Pass through
	m_cartridge->crureadz(space, offset, value);
}

WRITE8_MEMBER(ti99_single_cart_conn_device::cruwrite)
{
	// Pass through
	m_cartridge->cruwrite(space, offset, data);
}

WRITE_LINE_MEMBER(ti99_single_cart_conn_device::romgq_line)
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


WRITE_LINE_MEMBER(ti99_single_cart_conn_device::gclock_in)
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

void ti99_single_cart_conn_device::device_start()
{
	m_cartridge = static_cast<ti99_cartridge_device*>(subdevices().first());
}

void ti99_single_cart_conn_device::device_reset()
{
	m_cartridge->set_slot(0);
}

void ti99_single_cart_conn_device::device_add_mconfig(machine_config &config)
{
	TI99_CART(config, "cartridge", 0);
}

} } } // end namespace bus::ti99::gromport

