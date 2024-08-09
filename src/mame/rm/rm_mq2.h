// license:BSD-3-Clause
// copyright-holders:Robin Sergeant

#ifndef MAME_BUS_RM_MQ2_H
#define MAME_BUS_RM_MQ2_H

#pragma once

#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80sio.h"

class rmMQ2_device : public device_t, public device_rs232_port_interface
{
public:
	rmMQ2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

private:
	bool m_8inch_sel = false;

	required_device<z80_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80sio_device> m_sio;
	required_device<fd1793_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;

	const tiny_rom_entry *device_rom_region() const override;
	void device_add_mconfig(machine_config &config) override;
	void device_start() override;

	void input_txd(int state) override;
	void input_rts(int state) override;
	void input_dtr(int state) override;

	void rmMQ2_mem(address_map &map);
	void rmMQ2_io(address_map &map);
	void port0_w(uint8_t data);
	void port1_w(uint8_t data);
	uint8_t status_r();
	uint8_t drive_status_r();
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	uint8_t fdc_read(offs_t offset);
};

DECLARE_DEVICE_TYPE(RM_MQ2, rmMQ2_device)

#endif // MAME_BUS_RM_MQ2_H
