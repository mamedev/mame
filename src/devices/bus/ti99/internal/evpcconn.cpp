// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Connector from EVPC

    We need this for the TI-99/4A console; the SGCPU uses a separate line
    in the PEB.

    This is actually a separate cable lead going from
    the EPVC in the PEB to a pin inside the console. This cable sends the
    video interrupt from the v9938 on the EVPC into the console.
    This workaround must be done on the real system because the peripheral
    box and its connector were not designed to deliver a video interrupt signal.
    This was fixed with the EVPC2 which uses the external interrupt EXTINT
    with a special firmware (DSR).

    May 2017, Michael Zapf

****************************************************************************/

#include "emu.h"
#include "evpcconn.h"

DEFINE_DEVICE_TYPE(TI99_EVPCCONN, bus::ti99::internal::evpc_clock_connector, "ti99_evpc_clock", "EVPC clock connector")

namespace bus::ti99::internal {

evpc_clock_connector::evpc_clock_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TI99_EVPCCONN, tag, owner, clock),
		m_vdpint(*this)
{
}

WRITE_LINE_MEMBER( evpc_clock_connector::vclock_line )
{
	m_vdpint(state);
}

void evpc_clock_connector::device_start()
{
	m_vdpint.resolve();
}

} // end namespace bus::ti99::internal

