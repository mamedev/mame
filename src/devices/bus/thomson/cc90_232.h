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

	virtual void rom_map(address_map &map) override;
	virtual void io_map(address_map &map) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	// read data register
	uint8_t porta_in();

	// write data register
	void porta_out(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(write_rxd);
	DECLARE_WRITE_LINE_MEMBER(write_cts);
	DECLARE_WRITE_LINE_MEMBER(write_dsr);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);

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
