// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD FD-2000/FD-4000 disk drive emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_FD2000_H
#define MAME_BUS_CBMIEC_FD2000_H

#pragma once

#include "cbmiec.h"
#include "cpu/m6502/m65c02.h"
#include "formats/d81_dsk.h"
#include "imagedev/floppy.h"
#include "machine/6522via.h"
#include "machine/upd765.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> fd2000_device

class fd2000_device :  public device_t,
						public device_cbm_iec_interface
{
public:
	// construction/destruction
	fd2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( via_pa_r );
	DECLARE_WRITE8_MEMBER( via_pa_w );
	DECLARE_READ8_MEMBER( via_pb_r );
	DECLARE_WRITE8_MEMBER( via_pb_w );

	//DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	fd2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_cbm_iec_interface overrides
	void cbm_iec_srq(int state) override;
	void cbm_iec_atn(int state) override;
	void cbm_iec_data(int state) override;
	void cbm_iec_reset(int state) override;

	void add_common_devices(machine_config &config);

	required_device<m65c02_device> m_maincpu;
	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy0;

private:
	void fd2000_mem(address_map &map);
};


// ======================> fd4000_device

class fd4000_device :  public fd2000_device
{
public:
	// construction/destruction
	fd4000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void fd4000_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(FD2000, fd2000_device)
DECLARE_DEVICE_TYPE(FD4000, fd4000_device)


#endif // MAME_BUS_CBMIEC_FD2000_H
