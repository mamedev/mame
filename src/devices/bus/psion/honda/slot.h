// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Honda Expansion slot emulation

**********************************************************************

    Pin  Name      Comments

    1    VEXT
    2    RTS     - RS232
    3    DTR     - RS232
    4    TXD     - RS232
    5    DSR     - RS232
    6    DCD     - RS232
    7    CTS     - RS232
    8    RXD     - RS232
    9    SDOE    - SIBO data direction control
    10   EXTSTAT - active low SIBO peripheral detect line
    11   EXON    - SIBO external turn-on
    12   INT     - SIBO peripheral interrupt
    13   SD      - SIBO serial data
    14   SCK     - SIBO data clock
    15   GND     - Signal and Power ground

**********************************************************************/

#ifndef MAME_BUS_PSION_HONDA_SLOT_H
#define MAME_BUS_PSION_HONDA_SLOT_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_honda_slot_device

class device_psion_honda_interface;

class psion_honda_slot_device : public device_t, public device_single_card_slot_interface<device_psion_honda_interface>
{
public:
	// construction/destruction
	template <typename T>
	psion_honda_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&slot_options, const char *default_option)
		: psion_honda_slot_device(mconfig, tag, owner)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	psion_honda_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// callbacks
	auto rxd_handler() { return m_rxd_handler.bind(); }
	auto dcd_handler() { return m_dcd_handler.bind(); }
	auto dsr_handler() { return m_dsr_handler.bind(); }
	auto cts_handler() { return m_cts_handler.bind(); }
	auto sdoe_handler() { return m_sdoe_handler.bind(); }

	uint8_t data_r();
	void data_w(uint16_t data);

	void write_txd(int state);
	void write_dtr(int state);
	void write_rts(int state);

	void write_rxd(int state) { m_rxd_handler(state); }
	void write_dcd(int state) { m_dcd_handler(state); }
	void write_dsr(int state) { m_dsr_handler(state); }
	void write_cts(int state) { m_cts_handler(state); }
	void write_sdoe(int state) { m_sdoe_handler(state); }

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_rxd_handler;
	devcb_write_line m_dcd_handler;
	devcb_write_line m_dsr_handler;
	devcb_write_line m_cts_handler;
	devcb_write_line m_sdoe_handler;

	device_psion_honda_interface *m_card;
};


// ======================> device_psion_honda_interface

class device_psion_honda_interface : public device_interface
{
public:
	virtual void write_txd(int state) { }
	virtual void write_dtr(int state) { }
	virtual void write_rts(int state) { }

	virtual uint8_t data_r() { return 0x00; }
	virtual void data_w(uint16_t data) { }

protected:
	device_psion_honda_interface(const machine_config &mconfig, device_t &device);

	psion_honda_slot_device *m_slot;
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_HONDA_SLOT, psion_honda_slot_device)

void psion_honda_devices(device_slot_interface &device);


#endif // MAME_BUS_PSION_HONDA_SLOT_H
