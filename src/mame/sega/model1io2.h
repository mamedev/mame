// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega Model 1 I/O Board (Advanced)

***************************************************************************/

#ifndef MAME_SEGA_MODEL1IO2_H
#define MAME_SEGA_MODEL1IO2_H

#pragma once

#include "machine/eepromser.h"
#include "machine/mb3773.h"
#include "video/hd44780.h"
#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class model1io2_device : public device_t
{
public:
	// construction/destruction
	model1io2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto read_callback() { return m_read_cb.bind(); }
	auto write_callback() { return m_write_cb.bind(); }
	template <unsigned N> auto in_callback() { return m_in_cb[N].bind(); }
	auto drive_read_callback() { return m_drive_read_cb.bind(); }
	auto drive_write_callback() { return m_drive_write_cb.bind(); }
	template <unsigned N> auto an_callback() { return m_an_cb[N].bind(); }
	auto output_callback() { return m_output_cb.bind(); }

	template <typename T> void set_lightgun_p1y_tag(T && tag) { m_lightgun_ports[0].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_lightgun_p1x_tag(T && tag) { m_lightgun_ports[1].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_lightgun_p2y_tag(T && tag) { m_lightgun_ports[2].set_tag(std::forward<T>(tag)); }
	template <typename T> void set_lightgun_p2x_tag(T && tag) { m_lightgun_ports[3].set_tag(std::forward<T>(tag)); }

protected:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void lcd_palette(palette_device &palette) const;
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<mb3773_device> m_watchdog;
	required_device<hd44780_device> m_lcd;
	output_finder<> m_led_comm_err;
	optional_ioport_array<4> m_lightgun_ports;

	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);
	uint8_t io_pa_r();
	uint8_t io_pb_r();
	uint8_t io_pc_r();
	void io_pd_w(uint8_t data);
	uint8_t io_pe_r();
	void io_pe_w(uint8_t data);
	void io_pf_w(uint8_t data);
	void io_pg_w(uint8_t data);
	uint8_t fpga_r(offs_t offset);
	void fpga_w(uint8_t data);

	ioport_value analog0_r();
	ioport_value analog1_r();
	ioport_value analog2_r();
	ioport_value analog3_r();

	devcb_read8 m_read_cb;
	devcb_write8 m_write_cb;
	devcb_read8::array<3> m_in_cb;
	devcb_read8 m_drive_read_cb;
	devcb_write8 m_drive_write_cb;
	devcb_read8::array<8> m_an_cb;
	devcb_write8 m_output_cb;

	bool m_secondary_controls;
	uint8_t m_lcd_data;
	int m_fpga_counter;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_MODEL1IO2, model1io2_device)

#endif // MAME_SEGA_MODEL1IO2_H
