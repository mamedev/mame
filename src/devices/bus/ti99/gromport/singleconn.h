// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
    Single cartridge connector.
*****************************************************************************/

#ifndef MAME_BUS_TI99_GROMPORT_SINGLECONN_H
#define MAME_BUS_TI99_GROMPORT_SINGLECONN_H

#pragma once

#include "cartridges.h"

namespace bus::ti99::gromport {

class ti99_single_cart_conn_device : public cartridge_connector_device
{
public:
	ti99_single_cart_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;
	void romgq_line(int state) override;
	void set_gromlines(line_state mline, line_state moline, line_state gsq) override;
	void gclock_in(int state) override;

	bool is_grom_idle() override;

protected:
	virtual void device_start() override { };
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ti99_cartridge_device> m_cartridge;
};

} // end namespace bus::ti99::gromport

DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT_SINGLE, bus::ti99::gromport, ti99_single_cart_conn_device)

#endif // MAME_BUS_TI99_GROMPORT_SINGLECONN_H
