// license:BSD-3-Clause
// copyright-holders:MetalliC
/*********************************************************************

    FloppyOne DOS Interface
    (c) 1984/5 R.P.Gush

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_FLPONE_H
#define MAME_BUS_SPECTRUM_FLPONE_H

#include "exp.h"
#include "softlist.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "formats/fl1_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_flpone_device :
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_flpone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);
	DECLARE_INPUT_CHANGED_MEMBER(snapshot_button) { m_slot->nmi_w(newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void post_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	// passthru
	virtual void pre_opcode_fetch(offs_t offset) override { m_exp->pre_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual uint8_t iorq_r(offs_t offset) override { return m_exp->iorq_r(offset); }
	virtual void iorq_w(offs_t offset, uint8_t data) override { m_exp->iorq_w(offset, data); }

	DECLARE_WRITE_LINE_MEMBER(busy_w) { m_busy = state; }

	required_memory_region m_rom;
	required_memory_region m_prom;
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<spectrum_expansion_slot_device> m_exp;
	required_device<centronics_device> m_centronics;
	required_device<rs232_port_device> m_rs232;
	required_ioport m_sw1;
	required_ioport m_sw2;

	int m_romcs, m_if1cs;
	u8 m_ram[0x1000];
	int m_busy;
	uint8_t m_shifter;
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_FLPONE, spectrum_flpone_device)


#endif // MAME_BUS_SPECTRUM_FLPONE_H
