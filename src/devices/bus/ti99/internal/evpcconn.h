// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Connector from EVPC
    For details see evpcconn.cpp

****************************************************************************/
#ifndef MAME_BUS_TI99_INTERNAL_EVPCCONN_H
#define MAME_BUS_TI99_INTERNAL_EVPCCONN_H

#pragma once

#include "bus/ti99/ti99defs.h"

namespace bus { namespace ti99 { namespace internal {

class evpc_clock_connector : public device_t
{
public:
	evpc_clock_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE_LINE_MEMBER( vclock_line );
	void device_start() override;
	auto vdpint_cb() { return m_vdpint.bind(); }

private:
	// VDPINT line to the CPU
	devcb_write_line m_vdpint;
};

} } } // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(TI99_EVPCCONN, bus::ti99::internal, evpc_clock_connector)

#endif // MAME_BUS_TI99_INTERNAL_EVPCCONN_H
