// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON PF-10

    Battery operated portable 3.5" floppy drive

**********************************************************************/

#pragma once

#ifndef __PF10_H__
#define __PF10_H__

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/upd765.h"
#include "epson_sio.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class epson_pf10_device : public device_t,
							public device_epson_sio_interface
{
public:
	// construction/destruction
	epson_pf10_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// floppy disk controller
	uint8_t fdc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void fdc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fdc_tc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// hd6303 i/o
	uint8_t port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void port2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// serial output from main cpu
	void hd6303_tx_w(int state);

	// from sio output
	void rxc_w(int state);
	void pinc_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_epson_sio_interface overrides
	virtual void tx_w(int level) override;
	virtual void pout_w(int level) override;

private:
	required_device<hd6303y_cpu_device> m_cpu;
	required_device<upd765a_device> m_fdc;
	required_device<epson_sio_device> m_sio_output;

	epson_sio_device *m_sio_input;
	floppy_image_device *m_floppy;

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
extern const device_type EPSON_PF10;


#endif // __PF10_H__
