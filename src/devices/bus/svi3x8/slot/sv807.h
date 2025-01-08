// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-807 64k memory expansion for SVI-318/328

***************************************************************************/

#ifndef MAME_BUS_SVI3X8_SLOT_SV807_H
#define MAME_BUS_SVI3X8_SLOT_SV807_H

#pragma once

#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv807_device

class sv807_device : public device_t, public device_svi_slot_interface
{
public:
	// construction/destruction
	sv807_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t mreq_r(offs_t offset) override;
	virtual void mreq_w(offs_t offset, uint8_t data) override;

	virtual void bk21_w(int state) override;
	virtual void bk22_w(int state) override;
	virtual void bk31_w(int state) override;
	virtual void bk32_w(int state) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_switch;

	std::unique_ptr<uint8_t[]> m_ram_bank1;
	std::unique_ptr<uint8_t[]> m_ram_bank2;

	int m_bk21;
	int m_bk22;
	int m_bk31;
	int m_bk32;
};

// device type definition
DECLARE_DEVICE_TYPE(SV807, sv807_device)

#endif // MAME_BUS_SVI3X8_SLOT_SV807_H
