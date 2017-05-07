// license:LGPL-2.1+
// copyright-holders:Michael Zapf
#include "emu.h"
#include "ti99defs.h"

DEFINE_DEVICE_TYPE(EVPC_CONN, evpc_clock_connector, "ti99_evpc_clock", "EVPC clock connector")

evpc_clock_connector::evpc_clock_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, EVPC_CONN, tag, owner, clock)
	, m_vdpint(*this)
{
}

void evpc_clock_connector::device_start()
{
	m_vdpint.resolve();
}
