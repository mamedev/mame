// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Opus Discovery disc system

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_OPUS_H
#define MAME_BUS_SPECTRUM_OPUS_H

#include "exp.h"
#include "softlist.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/6821pia.h"
#include "bus/centronics/ctronics.h"
#include "formats/opd_dsk.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_opus_device:
	public device_t,
	public device_spectrum_expansion_interface

{
public:
	// construction/destruction
	spectrum_opus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual void post_opcode_fetch(offs_t offset) override;
	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;
	virtual uint8_t iorq_r(offs_t offset) override;
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	// passthru
	virtual void pre_opcode_fetch(offs_t offset) override { m_exp->pre_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual void iorq_w(offs_t offset, uint8_t data) override { m_exp->iorq_w(offset, data); }

private:
	void pia_out_a(uint8_t data);
	void pia_out_b(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(busy_w);

	required_ioport m_joy;
	required_memory_region m_rom;
	required_device<pia6821_device> m_pia;
	required_device<wd_fdc_device_base> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<centronics_device> m_centronics;
	required_device<spectrum_expansion_slot_device> m_exp;

	int m_romcs;
	uint8_t m_ram[4 * 1024];
};


// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_OPUS, spectrum_opus_device)

#endif // MAME_BUS_SPECTRUM_OPUS_H
