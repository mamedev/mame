// license:LGPL-2.1+
// copyright-holders:Michael Zapf
#include "emu.h"
#include "ti99defs.h"

const device_type EVPC_CONN = device_creator<evpc_clock_connector>;

evpc_clock_connector::evpc_clock_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, EVPC_CONN, "EVPC clock connector", tag, owner, clock, "ti99_evpc_clock", __FILE__)
	, m_vdpint(*this)
{
}

void evpc_clock_connector::device_start()
{
	m_vdpint.resolve();
}
