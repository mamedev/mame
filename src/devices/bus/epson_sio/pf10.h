// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON PF-10

    Battery operated portable 3.5" floppy drive

**********************************************************************/

#ifndef MAME_BUS_EPSON_SIO_PF10_H
#define MAME_BUS_EPSON_SIO_PF10_H

#pragma once

#include "epson_sio.h"
#include "cpu/m6800/m6801.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class epson_pf10_device : public device_t,
							public device_epson_sio_interface
{
public:
	// construction/destruction
	epson_pf10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_epson_sio_interface overrides
	virtual void tx_w(int level) override;
	virtual void pout_w(int level) override;

private:
	// serial output from main cpu
	void hd6303_tx_w(int state);

	// from sio output
	void rxc_w(int state);
	void pinc_w(int state);

	// floppy disk controller
	uint8_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint8_t data);
	void fdc_tc_w(uint8_t data);

	// hd6303 i/o
	uint8_t port1_r();
	void port1_w(uint8_t data);
	uint8_t port2_r();
	void port2_w(uint8_t data);

	TIMER_CALLBACK_MEMBER( serial_clk_tick );

	void cpu_mem(address_map &map) ATTR_COLD;

	required_device<hd6303x_cpu_device> m_cpu;
	required_device<upd765a_device> m_fdc;
	required_device<epson_sio_device> m_sio_output;
	required_device<floppy_connector> m_floppy;

	epson_sio_device *m_sio_input;

	emu_timer *m_timer;

	uint8_t m_port1;
	uint8_t m_port2;

	int m_rxc;
	int m_hd6303_tx;
	int m_pinc;

	// port definitions
	enum
	{
		PORT2_SEEK    = 0x01,
		PORT2_SWCOM   = 0x02, // ?
		PORT2_RS232ON = 0x04, // to nmi?
		PORT2_RXD     = 0x08,
		PORT2_TXD     = 0x10,
		PORT2_FDCRST  = 0x20,
		PORT2_MON     = 0x40,
		PORT2_BATCKEN = 0x80  // ?
	};
};


// device type definition
DECLARE_DEVICE_TYPE(EPSON_PF10, epson_pf10_device)


#endif // MAME_BUS_EPSON_SIO_PF10_H
