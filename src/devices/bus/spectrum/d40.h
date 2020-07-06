// license:BSD-3-Clause
// copyright-holders:MetalliC
/**********************************************************************

    Didaktik D40/D80 interface

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_D40_H
#define MAME_BUS_SPECTRUM_D40_H

#pragma once

#include "exp.h"
//#include "softlist.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "formats/d40_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_d40_device: public device_t, public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_d40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_INPUT_CHANGED_MEMBER(snapshot_button);

protected:
	spectrum_d40_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	required_memory_region m_rom;
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	int m_romcs;
	uint8_t m_ram[2 * 1024];
	uint8_t m_control;
	int m_snap_flag;
};

class spectrum_d80_device :
	public spectrum_d40_device
{
public:
	// construction/destruction
	spectrum_d80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual void device_add_mconfig(machine_config &config) override;
};

// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_D40, spectrum_d40_device)
DECLARE_DEVICE_TYPE(SPECTRUM_D80, spectrum_d80_device)

#endif // MAME_BUS_SPECTRUM_D40_H
