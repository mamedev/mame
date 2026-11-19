// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_BUS_SEGA8_SF7000_H
#define MAME_BUS_SEGA8_SF7000_H

#pragma once

#include "sega8_slot.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/output_latch.h"
#include "machine/upd765.h"


// ======================> sega8_sf7000_device

class sega8_sf7000_device : public device_t, public device_sega8_cart_interface
{
public:
	// construction/destruction
	sega8_sf7000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_sega8_cart_interface implementation
	virtual uint8_t read_cart(offs_t offset) override;
	virtual void write_cart(offs_t offset, uint8_t data) override;
	virtual uint8_t read_ram(offs_t offset) override;
	virtual void write_ram(offs_t offset, uint8_t data) override;
	virtual uint8_t read_io(offs_t offset) override;
	virtual void write_io(offs_t offset, uint8_t data) override;
	virtual bool dsram() const override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<upd765a_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<i8255_device> m_ppi;
	required_device<i8251_device> m_usart;
	required_device<clock_device> m_baudclk;
	required_device<rs232_port_device> m_rs232;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_region_ptr<uint8_t> m_rom;
	memory_share_creator<uint8_t> m_ram;
	required_ioport m_baud;

	bool m_romsel;
	uint8_t m_centronics_busy;

	void write_centronics_busy(int state);
	uint8_t ppi_pa_r();
	void ppi_pc_w(uint8_t data);

	static void floppy_formats(format_registration &fr) ATTR_COLD;
};


// device type declaration
DECLARE_DEVICE_TYPE(SEGA8_SF7000, sega8_sf7000_device)

#endif // MAME_BUS_SEGA8_SF7000_H
