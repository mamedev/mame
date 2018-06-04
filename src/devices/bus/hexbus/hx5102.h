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
#include "machine/rescap.h"
#include "machine/ram.h"
#include "imagedev/floppy.h"
#include "tp0370.h"

namespace bus { namespace hexbus {

class hx5102_device : public hexbus_chained_device
{
public:
	hx5102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	ioport_constructor device_input_ports() const override;

	void crumap(address_map &map);
	void memmap(address_map &map);
	DECLARE_WRITE8_MEMBER(external_operation);
	DECLARE_WRITE_LINE_MEMBER( clock_out );
	DECLARE_WRITE_LINE_MEMBER( board_ready );
	DECLARE_WRITE_LINE_MEMBER( board_reset );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void device_start() override;
	void device_reset() override;
	virtual void hexbus_value_changed(uint8_t data) override;

private:
	required_device<tms9995_device> m_flopcpu;
	line_state m_ready_old;

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE_LINE_MEMBER(motor_w);
	DECLARE_WRITE_LINE_MEMBER(mspeed_w);

	DECLARE_READ8_MEMBER(fdc_read);
	DECLARE_WRITE8_MEMBER(fdc_write);
	DECLARE_READ8_MEMBER(ibc_read);
	DECLARE_WRITE8_MEMBER(ibc_write);
	DECLARE_WRITE8_MEMBER(hexbus_out);
	DECLARE_WRITE_LINE_MEMBER(hsklatch_out);

	DECLARE_READ8_MEMBER(cruread);
	DECLARE_WRITE8_MEMBER(cruwrite);

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
	floppy_image_device*    m_floppy[2];
	floppy_image_device*    m_current_floppy;
	int m_floppy_select, m_floppy_select_last;

	required_device<ibc_device> m_hexbus_ctrl;
	required_device<i8272a_device> m_floppy_ctrl;
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

}   } // end namespace bus::hexbus

DECLARE_DEVICE_TYPE_NS(HX5102, bus::hexbus, hx5102_device)

#endif // MAME_BUS_HEXBUS_HX5102_H
