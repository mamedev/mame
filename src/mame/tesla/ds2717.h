// license:BSD-3-Clause
// copyright-holders:Felipe Corrêa da Silva Sanches
/*******************************************************************************

    Consul 2717 DS 2717 floppy disk controller

*******************************************************************************/

#ifndef MAME_TESLA_DS2717_H
#define MAME_TESLA_DS2717_H

#pragma once

#include "imagedev/floppy.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"


class ds2717_device : public device_t
{
public:
	ds2717_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_cb() { return m_int_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	void tc_w(int state);
	void update_int(bool old_int);
	void count_byte();

	static void floppy_formats(format_registration &fr);

	required_device<i8272a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<pit8253_device> m_pit;

	devcb_write_line m_int_cb;

	bool m_drq;
	bool m_intrq;
	bool m_tc;
};

DECLARE_DEVICE_TYPE(DS2717, ds2717_device)

#endif // MAME_TESLA_DS2717_H
