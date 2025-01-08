// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Gromport (Cartridge port) of the TI-99 consoles
    For details see gromport.cpp

    Michael Zapf
***************************************************************************/

#ifndef MAME_BUS_TI99_GROMPORT_GROMPORT_H
#define MAME_BUS_TI99_GROMPORT_GROMPORT_H

#pragma once

#define TI99_GROMPORT_TAG    "gromport"

namespace bus::ti99::gromport {

struct pcb_type
{
	int id;
	const char* name;
};

class ti99_cartridge_device;
class cartridge_connector_device;

class gromport_device : public device_t, public device_single_card_slot_interface<cartridge_connector_device>
{
public:
	template <typename U>
	gromport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, U &&opts, const char *dflt)
		: gromport_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	gromport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void readz(offs_t offset, uint8_t *value);
	void write(offs_t offset, uint8_t data);
	void crureadz(offs_t offset, uint8_t *value);
	void cruwrite(offs_t offset, uint8_t data);
	void ready_line(int state);
	void romgq_line(int state);
	void set_gromlines(line_state mline, line_state moline, line_state gsq);
	void gclock_in(int state);

	void    cartridge_inserted();
	bool    is_grom_idle();

	auto ready_cb() { return m_console_ready.bind(); }
	auto reset_cb() { return m_console_reset.bind(); }

	// Configure for 16K ROM space (TI-99/8 only)
	gromport_device& extend() { m_mask = 0x3fff; return *this; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_config_complete() override;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	cartridge_connector_device*    m_connector;
	bool                m_reset_on_insert;
	devcb_write_line   m_console_ready;
	devcb_write_line   m_console_reset;
	int m_romgq;
	int m_mask;
};

class cartridge_connector_device : public device_t
{
	friend class gromport_device;

public:
	virtual void readz(offs_t offset, uint8_t *value) = 0;
	virtual void write(offs_t offset, uint8_t data) = 0;
	virtual void setaddress_dbin(offs_t offset, int state) { }

	virtual void crureadz(offs_t offset, uint8_t *value) = 0;
	virtual void cruwrite(offs_t offset, uint8_t data) = 0;

	virtual void romgq_line(int state) = 0;
	virtual void set_gromlines(line_state mline, line_state moline, line_state gsq) =0;

	virtual void gclock_in(int state) = 0;

	void ready_line(int state);

	virtual void insert() { m_gromport->cartridge_inserted(); }
	virtual void remove() { }
	virtual bool is_grom_idle() = 0;

protected:
	cartridge_connector_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	gromport_device*    m_gromport;
	bool     m_grom_selected;

	void set_port(gromport_device* gromport) { m_gromport = gromport; }
};

} // end namespace bus::ti99::gromport

DECLARE_DEVICE_TYPE_NS(TI99_GROMPORT, bus::ti99::gromport, gromport_device)

void ti99_gromport_options(device_slot_interface &device);
void ti99_gromport_options_998(device_slot_interface &device);

#endif // MAME_BUS_TI99_GROMPORT_GROMPORT_H
