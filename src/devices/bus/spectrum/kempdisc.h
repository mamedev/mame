// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    Kempston Disc Interface

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_KEMPDISC_H
#define MAME_BUS_SPECTRUM_KEMPDISC_H

#include "exp.h"
#include "softlist.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "formats/dsk_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_kempdisc_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_kempdisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	spectrum_kempdisc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	// passthru
	virtual void pre_opcode_fetch(offs_t offset) override { m_exp->pre_opcode_fetch(offset); }
	virtual void post_opcode_fetch(offs_t offset) override { m_exp->post_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual void mreq_w(offs_t offset, uint8_t data) override { if (m_exp->romcs()) m_exp->mreq_w(offset, data); }

	required_memory_region m_rom;
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<spectrum_expansion_slot_device> m_exp;

	int m_romcs;
//  u8 m_control;
};

class spectrum_spdos_device :
	public spectrum_kempdisc_device
{
public:
	// construction/destruction
	spectrum_spdos_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t mreq_r(offs_t offset) override;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_KEMPDISC, spectrum_kempdisc_device)
DECLARE_DEVICE_TYPE(SPECTRUM_SPDOS, spectrum_spdos_device)


#endif // MAME_BUS_SPECTRUM_KEMPDISC_H
