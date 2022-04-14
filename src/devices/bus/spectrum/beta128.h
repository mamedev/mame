// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Technology Research Beta 128 Disk interface

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_BETA128_H
#define MAME_BUS_SPECTRUM_BETA128_H

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

	static void floppy_formats(format_registration &fr);
	DECLARE_INPUT_CHANGED_MEMBER(magic_button);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual void pre_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	// passthru
	virtual void post_opcode_fetch(offs_t offset) override { m_exp->post_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual void mreq_w(offs_t offset, uint8_t data) override { if (m_exp->romcs()) m_exp->mreq_w(offset, data); }

	required_memory_region m_rom;
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<spectrum_expansion_slot_device> m_exp;
	required_ioport m_switch;

	int m_romcs;
	u8 m_control;
	bool m_motor_active;
	bool m_128rom_bit;
	void fdc_hld_w(int state);
	void motors_control();
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_BETA128, spectrum_beta128_device)


#endif // MAME_BUS_SPECTRUM_BETA128_H
