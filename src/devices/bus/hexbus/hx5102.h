// license:BSD-3-Clause
// copyright-holders:Michael Zapf
/****************************************************************************

    Hexbus floppy disk drive
    HX5102

    See hx5102.cpp for documentation

    Michael Zapf
    June 2017

*****************************************************************************/

#ifndef MAME_BUS_HEXBUS_HX5102_H
#define MAME_BUS_HEXBUS_HX5102_H

#pragma once

#include "hexbus.h"
#include "cpu/tms9900/tms9995.h"
#include "machine/upd765.h"
#include "machine/7474.h"
#include "machine/74123.h"
#include "machine/74259.h"
#include "machine/rescap.h"
#include "machine/ram.h"
#include "imagedev/floppy.h"
#include "tp0370.h"

namespace bus::hexbus {

class hx5102_device : public hexbus_chained_device
{
public:
	hx5102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void crumap(address_map &map) ATTR_COLD;
	void memmap(address_map &map) ATTR_COLD;
	void external_operation(offs_t offset, uint8_t data);
	void clock_out(int state);
	void board_ready(int state);
	void board_reset(int state);
	static void floppy_formats(format_registration &fr);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void hexbus_value_changed(uint8_t data) override;

private:
	required_device<tms9995_device> m_flopcpu;
	line_state m_ready_old;

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void motor_w(int state);
	void mspeed_w(int state);

	uint8_t fdc_read(offs_t offset);
	void fdc_write(offs_t offset, uint8_t data);
	uint8_t ibc_read(offs_t offset);
	void ibc_write(offs_t offset, uint8_t data);
	void hexbus_out(uint8_t data);
	void hsklatch_out(int state);

	uint8_t cruread(offs_t offset);
	void nocomp_w(int state);
	void diren_w(int state);
	void dacken_w(int state);
	void stepen_w(int state);
	void ds1_w(int state);
	void ds2_w(int state);
	void ds3_w(int state);
	void ds4_w(int state);
	void aux_motor_w(int state);
	void wait_w(int state);
	void update_drive_select();

	// Operate the floppy motors
	bool m_motor_on;
	bool m_mspeed_on;

	bool m_pending_int;
	bool m_pending_drq;

	bool m_dcs;

	bool m_dack;
	bool m_dacken;

	bool m_wait;

	void update_readyff_input();

	// Link to the attached floppy drives
	required_device<floppy_connector> m_flopcon0;
	required_device<floppy_connector> m_flopcon1;
	floppy_image_device*    m_floppy[2];
	floppy_image_device*    m_current_floppy;
	int m_floppy_select, m_floppy_select_last;

	required_device<ibc_device> m_hexbus_ctrl;
	required_device<i8272a_device> m_floppy_ctrl;
	required_device_array<ls259_device, 2> m_crulatch;
	required_device<ttl74123_device> m_motormf;
	required_device<ttl74123_device> m_speedmf;
	required_device<ttl7474_device> m_readyff;

	// RAM
	required_device<ram_device> m_ram1;
	required_device<ram_device> m_ram2;

	// System ROM
	uint8_t* m_rom1;
	uint8_t* m_rom2;
};

} // end namespace bus::hexbus

DECLARE_DEVICE_TYPE_NS(HX5102, bus::hexbus, hx5102_device)

#endif // MAME_BUS_HEXBUS_HX5102_H
