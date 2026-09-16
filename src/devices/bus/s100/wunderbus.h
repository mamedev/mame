// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Wunderbus I/O card emulation

**********************************************************************/

#ifndef MAME_BUS_S100_WUNDERBUS_H
#define MAME_BUS_S100_WUNDERBUS_H

#pragma once

#include "s100.h"
#include "machine/ins8250.h"
#include "machine/pic8259.h"
#include "machine/upd1990a.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_wunderbus_device

class s100_wunderbus_device : public device_t,
								public device_s100_card_interface
{
public:
	// construction/destruction
	s100_wunderbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_s100_card_interface overrides
	virtual void s100_vi0_w(int state) override;
	virtual void s100_vi1_w(int state) override;
	virtual void s100_vi2_w(int state) override;
	virtual uint8_t s100_sinp_r(offs_t offset) override;
	virtual void s100_sout_w(offs_t offset, uint8_t data) override;

private:
	void pic_int_w(int state);
	void rtc_tp_w(int state);

	required_device<pic8259_device> m_pic;
	required_device<ins8250_device> m_ace1;
	required_device<ins8250_device> m_ace2;
	required_device<ins8250_device> m_ace3;
	required_device<upd1990a_device> m_rtc;
	required_ioport m_7c;
	required_ioport m_10a;

	uint8_t m_group;
	int m_rtc_tp;
};


// device type definition
DECLARE_DEVICE_TYPE(S100_WUNDERBUS, s100_wunderbus_device)

#endif // MAME_BUS_S100_WUNDERBUS_H
