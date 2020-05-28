// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    Sirius JoyPort

*********************************************************************/

#ifndef MAME_BUS_A2GAMEIO_JOYPORT_H
#define MAME_BUS_A2GAMEIO_JOYPORT_H

#pragma once

#include "bus/a2gameio/gameio.h"

// ======================> apple2_joyport_device

class apple2_joyport_device : public device_t, public device_a2gameio_interface
{
public:
	// construction/destruction
	apple2_joyport_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;

	// device_a2gameio_interface overrides
	virtual DECLARE_READ_LINE_MEMBER(sw0_r) override;
	virtual DECLARE_READ_LINE_MEMBER(sw1_r) override;
	virtual DECLARE_READ_LINE_MEMBER(sw2_r) override;
	virtual DECLARE_WRITE_LINE_MEMBER(an0_w) override;
	virtual DECLARE_WRITE_LINE_MEMBER(an1_w) override;

private:
	// input ports
	required_ioport m_player1, m_player2;
	int m_an0, m_an1;
};

// device type declaration
DECLARE_DEVICE_TYPE(APPLE2_JOYPORT, apple2_joyport_device)

#endif // MAME_BUS_A2GAMEIO_JOYPORT_H
