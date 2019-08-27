// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Technology Research Beta 128 Disk interface

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_BETA_H
#define MAME_BUS_SPECTRUM_BETA_H

#include "exp.h"
#include "softlist.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "formats/trd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_beta128_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_beta128_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_FLOPPY_FORMATS(floppy_formats);
	DECLARE_INPUT_CHANGED_MEMBER(magic_button);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	required_memory_region m_rom;
	required_device<fd1793_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<spectrum_expansion_slot_device> m_exp;
	required_ioport m_switch;

	int m_romcs;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_BETA128, spectrum_beta128_device)


#endif // MAME_BUS_SPECTRUM_BETA_H
