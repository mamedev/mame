// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Scandia Metric ABC FD2 floppy controller emulation

*********************************************************************/

#ifndef MAME_BUS_ABCBUS_FD2_H
#define MAME_BUS_ABCBUS_FD2_H

#pragma once

#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/z80pio.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc_fd2_device

class abc_fd2_device :  public device_t,
					public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc_fd2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;
	virtual uint8_t abcbus_xmemfl(offs_t offset) override;

private:
	uint8_t pio_pa_r();
	void pio_pa_w(uint8_t data);
	uint8_t pio_pb_r();
	void pio_pb_w(uint8_t data);

	void status_w(uint8_t data);

	static void floppy_formats(format_registration &fr);

	void abc_fd2_io(address_map &map) ATTR_COLD;
	void abc_fd2_mem(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<fd1771_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_memory_region m_dos_rom;

	bool m_cs;
	uint8_t m_status;
	uint8_t m_data;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_FD2, abc_fd2_device)

#endif // MAME_BUS_ABCBUS_FD2_H
