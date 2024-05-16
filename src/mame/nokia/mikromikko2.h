// license:BSD-3-Clause
// copyright-holders:Curt Coder
#ifndef MAME_NOKIA_MIKROMIKKO2_H
#define MAME_NOKIA_MIKROMIKKO2_H

#pragma once

#include "screen.h"
#include "bus/nscsi/devices.h"
#include "bus/scsi/s1410.h"
#include "bus/scsi/scsihd.h"
#include "cpu/i86/i186.h"
#include "imagedev/floppy.h"
#include "machine/nvram.h"
#include "machine/am9517a.h"
#include "machine/i8251.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/upd765.h"
#include "machine/z80sio.h"
#include "machine/x2212.h"
#include "video/crt9007.h"
#include "video/crt9212.h"

#define I80186_TAG      "maincpu"
#define UPD765_TAG	    "upd765"
#define SCREEN_TAG      "screen"

class mm2_state : public driver_device
{
public:
	mm2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, I80186_TAG),
		m_novram(*this, "x2212"),
		m_pic(*this, "pic8259"),
		m_pit(*this, "pit8253"),
		m_mpsc(*this, "i8274"),
		m_vpac(*this, "crt9007"),
		m_drb0(*this, "crt9212_0"),
		m_drb1(*this, "crt9212_1"),
		m_sio(*this, "i8251"),
		m_dmac(*this, "am9517a"),
		m_fdc(*this, UPD765_TAG),
		m_sasi(*this, "sasi:7:scsicb")
	{ }

	void mm2(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<x2212_device> m_novram;
	required_device<pic8259_device> m_pic;
	required_device<pit8253_device> m_pit;
	required_device<i8274_device> m_mpsc;
	required_device<crt9007_device> m_vpac;
	required_device<crt9212_device> m_drb0;
	required_device<crt9212_device> m_drb1;
	required_device<i8251_device> m_sio;
	required_device<am9517a_device> m_dmac;
	required_device<upd765a_device> m_fdc;
	required_device<nscsi_callback_device> m_sasi;

	void mm2_map(address_map &map);
	void mm2_io_map(address_map &map);
	void vpac_mem(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	static void floppy_formats(format_registration &fr);

	void novram_store(offs_t offset, uint8_t data);
	void novram_recall(offs_t offset, uint8_t data);
	uint8_t videoram_r(offs_t offset);
};

#endif // MAME_NOKIA_MIKROMIKKO2_H
