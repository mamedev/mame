// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Thermal Printer
    Model PHP1900

    Michael Zapf
    May 2026

*****************************************************************************/

#ifndef MAME_BUS_TI99_SIDECAR_THERMAL_H
#define MAME_BUS_TI99_SIDECAR_THERMAL_H

#pragma once

#include "bus/ti99/internal/ioport.h"
#include "machine/bitmap_printer.h"
#include "machine/74123.h"
#include "machine/74259.h"

namespace bus::ti99::sidecar {

class ti_thermal_printer_device : public bus::ti99::internal::ioport_attached_device
{
public:
	ti_thermal_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void readz(offs_t offset, uint8_t *value) override;
	void write(offs_t offset, uint8_t data) override;
	void setaddress_dbin(offs_t offset, int state) override;

	void crureadz(offs_t offset, uint8_t *value) override;
	void cruwrite(offs_t offset, uint8_t data) override;

	void memen_in(int state) override;
	void msast_in(int state) override;

	void clock_in(int state) override;
	void reset_in(int state) override;

	INPUT_CHANGED_MEMBER( feed_button );

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// Callbacks from the external port
	void extint(int state);
	void extready(int state);

private:
	required_device<bitmap_printer_device>                m_bitmap_printer;
	required_device<bus::ti99::internal::ioport_device>   m_port;

	required_device<ls259_device>    m_latch_u4;
	required_device<ls259_device>    m_latch_u5;
	required_device<ls259_device>    m_latch_u6;
	required_device<ls259_device>    m_latch_u7;
	required_device<ttl74123_device> m_heatmf_u8;

	bool m_feed_pressed;
	line_state m_ext_int;

	// DSR ROM
	uint8_t*          m_dsrrom;

	void write_line();
	void line_feed();
	void enable_column_latches(int state);
	void clear_latches(int state);
};

} // end namespace bus::ti99::sidecar

DECLARE_DEVICE_TYPE_NS(TI99_THERMAL, bus::ti99::sidecar, ti_thermal_printer_device)

#endif // MAME_BUS_TI99_SIDECAR_THERMAL_H
