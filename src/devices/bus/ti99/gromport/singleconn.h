// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
    Single cartridge connector.
*****************************************************************************/

#ifndef MAME_BUS_TI99_GROMPORT_SINGLECONN_H
#define MAME_BUS_TI99_GROMPORT_SINGLECONN_H

#pragma once

#include "bus/ti99/ti99defs.h"
#include "cartridges.h"

namespace bus { namespace ti99 { namespace gromport {

class ti99_single_cart_conn_device : public cartridge_connector_device
{
public:
	ti99_single_cart_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
	DECLARE_WRITE_LINE_MEMBER(romgq_line) override;
	void set_gromlines(line_state mline, line_state moline, line_state gsq) override;
	DECLARE_WRITE_LINE_MEMBER(gclock_in) override;

	bool is_grom_idle() override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	ti99_cartridge_device *m_cartridge;
};
} } } // end namespace bus::ti99::gromport

DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT_SINGLE, bus::ti99::gromport, ti99_single_cart_conn_device)

#endif // MAME_BUS_TI99_GROMPORT_SINGLECONN_H
