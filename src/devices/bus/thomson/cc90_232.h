// license:BSD-3-Clause
// copyright-holders:Antoine Mine

#ifndef MAME_BUS_THOMSON_CC90_232_H
#define MAME_BUS_THOMSON_CC90_232_H

#include "extension.h"
#include "bus/rs232/rs232.h"
#include "machine/6821pia.h"

class cc90_232_device : public device_t, public thomson_extension_interface
{
public:
	cc90_232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	// read data register
	uint8_t porta_in();

	// write data register
	void porta_out(uint8_t data);

	void write_rxd(int state);
	void write_cts(int state);
	void write_dsr(int state);
	void write_centronics_busy(int state);

	required_device<pia6821_device> m_pia_io;
	required_device<rs232_port_device> m_rs232;

	int m_last_low = 0;
	int m_centronics_busy = 0;
	int m_rxd = 0;
	int m_cts = 0;
	int m_dsr = 0;
};

// device type declaration
DECLARE_DEVICE_TYPE(CC90_232, cc90_232_device)

#endif // MAME_BUS_THOMSON_CC90_232_H
