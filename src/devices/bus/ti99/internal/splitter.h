// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    I/O port splitter

    This plugs into the I/O port of the console as an
    ioport_attached_device, and it offers two new I/O ports.

*****************************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_SPLITTER_H
#define MAME_BUS_TI99_INTERNAL_SPLITTER_H

#pragma once

#include "ioport.h"

namespace bus::ti99::internal {

class ioport_splitter_device : public ioport_attached_device
{

public:
	ioport_splitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Next methods are called from the console
	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	void memen_in(int state) override;
	void msast_in(int state) override;

	void clock_in(int state) override;
	void reset_in(int state) override;

protected:
	ioport_splitter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// Callbacks from the two ports
	template<int port> void extint(int state);
	template<int port> void ready(int state);

	int m_inta_flag;
	int m_ready_flag;

private:
	required_device<ioport_device>     m_port1;
	required_device<ioport_device>     m_port2;
};

} // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(TI99_IOSPLIT, bus::ti99::internal, ioport_splitter_device)

#endif /* MAME_BUS_TI99_INTERNAL_SPLITTER_H */
