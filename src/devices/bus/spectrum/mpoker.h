// license:BSD-3-Clause
// copyright-holders:TwistedTom
/*********************************************************************

    MICRO-POKEer

    (Micro-Studio, Hungary 1988-1989)

*********************************************************************/
#ifndef MAME_BUS_SPECTRUM_MPOKER_H
#define MAME_BUS_SPECTRUM_MPOKER_H

#include "exp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class spectrum_mpoker_device : public device_t, public device_spectrum_expansion_interface
{
public:
	// construction/destruction
	spectrum_mpoker_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(freeze_button);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void iorq_w(offs_t offset, uint8_t data) override;
	virtual bool romcs() override;

	required_memory_region m_rom;

	bool m_romcs;
};

// device type definition
DECLARE_DEVICE_TYPE(SPECTRUM_MPOKER, spectrum_mpoker_device)

#endif // MAME_BUS_SPECTRUM_MPOKER_H
