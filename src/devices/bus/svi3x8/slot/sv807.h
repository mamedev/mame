// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-807 64k memory expansion for SVI-318/328

***************************************************************************/

#pragma once

#ifndef __SVI3X8_SLOT_SV807_H__
#define __SVI3X8_SLOT_SV807_H__

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sv807_device

class sv807_device : public device_t, public device_svi_slot_interface
{
public:
	// construction/destruction
	sv807_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override;

	virtual DECLARE_READ8_MEMBER( mreq_r ) override;
	virtual DECLARE_WRITE8_MEMBER( mreq_w ) override;

	virtual void bk21_w(int state) override;
	virtual void bk22_w(int state) override;
	virtual void bk31_w(int state) override;
	virtual void bk32_w(int state) override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport m_switch;

	std::unique_ptr<UINT8[]> m_ram_bank1;
	std::unique_ptr<UINT8[]> m_ram_bank2;

	int m_bk21;
	int m_bk22;
	int m_bk31;
	int m_bk32;
};

// device type definition
extern const device_type SV807;

#endif // __SVI3X8_SLOT_SV807_H__
