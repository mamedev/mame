// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Multi cartridge connector.

    We set the number of slots to 4, although we may have up to 16. From a
    logical point of view we could have 256, but the operating system only checks
    the first 16 banks.
*****************************************************************************/

#ifndef MAME_BUS_TI99_GROMPORT_MULTICONN_H
#define MAME_BUS_TI99_GROMPORT_MULTICONN_H

#pragma once
#include "bus/ti99/ti99defs.h"
#include "cartridges.h"

namespace bus { namespace ti99 { namespace gromport {

class ti99_multi_cart_conn_device : public cartridge_connector_device
{
public:
	ti99_multi_cart_conn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;
	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;
	DECLARE_WRITE_LINE_MEMBER(romgq_line) override;
	void set_gromlines(line_state mline, line_state moline, line_state gsq) override;
	DECLARE_WRITE_LINE_MEMBER(gclock_in) override;

	void insert(int index, ti99_cartridge_device* cart) override;
	void remove(int index) override;
	DECLARE_INPUT_CHANGED_MEMBER( switch_changed );

	bool is_grom_idle() override;

protected:
	static constexpr unsigned NUMBER_OF_CARTRIDGE_SLOTS = 4;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

private:
	bool m_readrom;
	int     m_active_slot;
	int     m_fixed_slot;
	int     m_next_free_slot;
	ti99_cartridge_device*  m_cartridge[NUMBER_OF_CARTRIDGE_SLOTS];

	void    set_slot(int slotnumber);
	int     get_active_slot(bool changebase, offs_t offset);
};

} } } // end namespace bus::ti99::gromport

DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT_MULTI,  bus::ti99::gromport, ti99_multi_cart_conn_device)

#endif // MAME_BUS_TI99_GROMPORT_MULTICONN_H



