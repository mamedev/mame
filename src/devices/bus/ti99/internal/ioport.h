// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
     I/O port
*****************************************************************************/

#ifndef MAME_BUS_TI99_INTERNAL_IOPORT_H
#define MAME_BUS_TI99_INTERNAL_IOPORT_H

#pragma once

#define TI99_IOPORT_TAG      "ioport"

namespace bus::ti99::internal {

extern const device_type IOPORT;

class ioport_device;

/********************************************************************
    Common parent class of all devices attached to the I/O port
********************************************************************/
class ioport_attached_device : public device_t
{
public:
	ioport_attached_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		m_ioport(nullptr)
	{ }

	// Methods called from the console / ioport
	virtual void readz(offs_t offset, uint8_t *value) { }
	virtual void write(offs_t offset, uint8_t data) { }
	virtual void setaddress_dbin(offs_t offset, int state) { }
	virtual void crureadz(offs_t offset, uint8_t *value) { }
	virtual void cruwrite(offs_t offset, uint8_t data) { }
	virtual void memen_in(int state) { }
	virtual void msast_in(int state) { }
	virtual void clock_in(int state) { }
	virtual void reset_in(int state) { }

	void set_ioport(ioport_device* ioport) { m_ioport = ioport; }

protected:
	// Methods called from the external device
	void set_extint(int state);
	void set_ready(int state);
private:
	ioport_device* m_ioport;
};

/********************************************************************
    I/O port
********************************************************************/

class ioport_device : public device_t, public device_single_card_slot_interface<ioport_attached_device>
{
	friend class ioport_attached_device;

public:
	template <typename U>
	ioport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, U &&opts, const char *dflt)
		: ioport_device(mconfig, tag, owner, clock)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	ioport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// Methods called from the console
	void readz(offs_t offset, uint8_t *value);
	void write(offs_t offset, uint8_t data);
	void setaddress_dbin(offs_t offset, int state);
	void crureadz(offs_t offset, uint8_t *value);
	void cruwrite(offs_t offset, uint8_t data);
	void memen_in(int state);
	void msast_in(int state);
	void clock_in(int state);
	void reset_in(int state);

	// Callbacks
	auto extint_cb() { return m_console_extint.bind(); }
	auto ready_cb() { return m_console_ready.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_config_complete() override;

	// Methods called back from the external device
	devcb_write_line m_console_extint;   // EXTINT line
	devcb_write_line m_console_ready;  // READY line

private:
	ioport_attached_device*    m_connected;
};

} // end namespace bus::ti99::internal

DECLARE_DEVICE_TYPE_NS(TI99_IOPORT, bus::ti99::internal, ioport_device)

void ti99_ioport_options_plain(device_slot_interface &device);
void ti99_ioport_options_evpc(device_slot_interface &device);

#endif /* __TI99IOPORT__ */
