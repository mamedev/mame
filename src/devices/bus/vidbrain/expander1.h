// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Expander 1 emulation

**********************************************************************/

#ifndef MAME_BUS_VIDBRAIN_EXPANDER1_H
#define MAME_BUS_VIDBRAIN_EXPANDER1_H

#pragma once

#include "accessory.h"
#include "bus/rs232/rs232.h"
#include "cpu/f8/f8.h"
#include "imagedev/cassette.h"
#include "machine/f3853.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> videobrain_expander1_device

class videobrain_expander1_device : public device_t, public device_videobrain_accessory_interface
{
public:
	// construction/destruction
	videobrain_expander1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_videobrain_accessory_interface implementation
	virtual void videobrain_io_w(uint8_t data) override;
	virtual uint8_t videobrain_io_r() override;
	virtual void videobrain_reset_w(int state) override;

private:
	required_device<f8_cpu_device> m_mcu;
	required_device<f38t56_device> m_psu;
	required_device<cassette_image_device> m_playback;
	required_device<cassette_image_device> m_record;
	required_device<rs232_port_device> m_printer;
	required_device<rs232_port_device> m_modem;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t p0_r();
	void p0_w(uint8_t data);
	uint8_t p1_r();
	void p1_w(uint8_t data);
	uint8_t p4_r();
	void p4_w(uint8_t data);
	uint8_t p5_r();
	void p5_w(uint8_t data);

	uint8_t m_p0;
	uint8_t m_p1;
	uint8_t m_p4;
	uint8_t m_p5;
	uint8_t m_host;
	uint8_t m_printer_rxd;
	uint8_t m_modem_rxd;
};


// device type declaration
DECLARE_DEVICE_TYPE(VIDEOBRAIN_EXPANDER1, videobrain_expander1_device)

#endif // MAME_BUS_VIDBRAIN_EXPANDER1_H
