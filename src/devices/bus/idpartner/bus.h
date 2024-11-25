// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*********************************************************************

        Iskra Delta Partner BUS

**********************************************************************

    Pinout:

                      *********
        GND       1   * . | . *    1 GND
        GND       2   * . | . *    2 GND
        -12V      3   * . | . *    3 -12V
        +5V       4   * . | . *    4 +5V
        +5V       5   * . | . *    5 +5V
        +12V      6   * . | . *    6 +12V
        NC        7   * . | . *    7 NC
        NC        8   * . | . *    8 IEDB
        /BUSACKB  9   * . | . *    9 /DMARQ
        NC       10   * . | . *   10 NC
        /RESETB  11   * . | . *   11 NC
        NC       12   * . | . *   12 /M1B
        /MREQB   13   * . | . *   13 /NMI
        NC       14   * . | . *   14 /BUSRQ
        /INTB    15   * . | . *   15 /HALTB
        /WAITB   16   * . | . *   16 /RDB
        phiB     17   * . | . *   17 A0B
        /IORQB   18   * . | . *   18 A1B
        /WTB     19   * . | . *   19 A2B
        /RFSHB   20   * . | . *   20 A3B
        NC       21   * . | . *   21 A4B
        /BINB    22   * . | . *   22 A5B
        NC       23   * . | . *   23 A6B
        NC       24   * . | . *   24 A7B
        /D0B     25   * . | . *   25 NC
        /D1B     26   * . | . *   26 NC
        /D2B     27   * . | . *   27 NC
        /D3B     28   * . | . *   28 NC
        /D4B     29   * . | . *   29 NC
        /D5B     30   * . | . *   30 NC
        /D6B     31   * . | . *   31 NC
        /D7B     32   * . | . *   32 NC
                      *********

**********************************************************************/

#ifndef MAME_BUS_IDPARTNER_BUS_H
#define MAME_BUS_IDPARTNER_BUS_H

#pragma once

namespace bus::idpartner {

class bus_device;
class device_exp_card_interface;

class bus_connector_device : public device_t, public device_single_card_slot_interface<device_exp_card_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	bus_connector_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&bus_tag, U &&opts, const char *dflt)
		: bus_connector_device(mconfig, tag, owner)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
		m_bus.set_tag(std::forward<T>(bus_tag));
	}

	bus_connector_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<bus_device> m_bus;
};

class bus_device : public device_t
{
public:
	// construction/destruction
	bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	template <typename T> void set_io_space(T &&tag, int spacenum) { m_io.set_tag(std::forward<T>(tag), spacenum); }

	// callbacks
	auto int_handler() { return m_int_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }

	// called from expansion device
	void int_w(int state) { m_int_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }
	void drq_w(int state) { m_drq_handler(state); }

	address_space &io() { return *m_io; }

private:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// internal state
	required_address_space m_io;

	devcb_write_line m_int_handler;
	devcb_write_line m_nmi_handler;
	devcb_write_line m_drq_handler;
};

// ======================> device_exp_card_interface

class device_exp_card_interface : public device_interface
{
protected:
	// construction/destruction
	device_exp_card_interface(const machine_config &mconfig, device_t &device);

public:
	// inline configuration
	void set_bus(bus_device *bus) { m_bus = bus; }

protected:
	bus_device *m_bus;
};

} // namespace bus::idpartner

DECLARE_DEVICE_TYPE_NS(IDPARTNER_BUS, bus::idpartner, bus_device)
DECLARE_DEVICE_TYPE_NS(IDPARTNER_BUS_CONNECTOR, bus::idpartner, bus_connector_device)

void idpartner_exp_devices(device_slot_interface &device);

#endif // MAME_BUS_IDPARTNER_BUS_H
