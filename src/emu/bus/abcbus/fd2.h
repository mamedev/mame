// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Scandia Metric FD2 floppy controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#pragma once

#ifndef __ABC_FD2__
#define __ABC_FD2__

#include "emu.h"
#include "abcbus.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
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
	abc_fd2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data);
	virtual UINT8 abcbus_xmemfl(offs_t offset);

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<fd1771_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_memory_region m_rom;
};


// device type definition
extern const device_type ABC_FD2;



#endif
