// license:BSD-3-Clause
// copyright-holders:TwistedTom
/*********************************************************************

    Spec-Mate back-up interface

    (A, T & Y Computing Ltd.)

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_SPECMATE_H
#define MAME_BUS_SPECTRUM_SPECMATE_H

#include "exp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_specmate_device : public device_t, public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_specmate_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(freeze_button) { m_slot->nmi_w(newval ? CLEAR_LINE : ASSERT_LINE); }

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
	virtual DECLARE_READ_LINE_MEMBER(romcs) override;

	// passthru
	virtual void post_opcode_fetch(offs_t offset) override { m_exp->post_opcode_fetch(offset); }
	virtual void pre_data_fetch(offs_t offset) override { m_exp->pre_data_fetch(offset); }
	virtual void post_data_fetch(offs_t offset) override { m_exp->post_data_fetch(offset); }
	virtual void mreq_w(offs_t offset, uint8_t data) override { if (m_exp->romcs()) m_exp->mreq_w(offset, data); }
	virtual uint8_t iorq_r(offs_t offset) override { return m_exp->iorq_r(offset); }
	virtual void iorq_w(offs_t offset, uint8_t data) override { m_exp->iorq_w(offset, data); }

	required_memory_region m_rom;
	required_device<spectrum_expansion_slot_device> m_exp;

	int m_romcs;
};

// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_SPECMATE, spectrum_specmate_device)

#endif // MAME_BUS_SPECTRUM_SPECMATE_H
