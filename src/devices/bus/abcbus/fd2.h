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
#include "formats/abcfd2_dsk.h"
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
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(uint8_t data) override;
	virtual uint8_t abcbus_inp() override;
	virtual void abcbus_out(uint8_t data) override;
	virtual uint8_t abcbus_stat() override;
	virtual void abcbus_c1(uint8_t data) override;
	virtual void abcbus_c3(uint8_t data) override;
	virtual uint8_t abcbus_xmemfl(offs_t offset) override;

private:
	DECLARE_READ8_MEMBER( pio_pa_r );
	DECLARE_WRITE8_MEMBER( pio_pa_w );
	DECLARE_READ8_MEMBER( pio_pb_r );
	DECLARE_WRITE8_MEMBER( pio_pb_w );

	DECLARE_WRITE8_MEMBER( status_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );

	void abc_fd2_io(address_map &map);
	void abc_fd2_mem(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<fd1771_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_dos_rom;

	bool m_cs;
	uint8_t m_status;
	uint8_t m_data;
};


// device type definition
DECLARE_DEVICE_TYPE(ABC_FD2, abc_fd2_device)

#endif // MAME_BUS_ABCBUS_FD2_H
