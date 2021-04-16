// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**********************************************************************

    Tasc SmartBoard

*********************************************************************/

#ifndef MAME_MACHINE_SMARTBOARD_H
#define MAME_MACHINE_SMARTBOARD_H

#pragma once

#include "machine/sensorboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tasc_sb30_device

class tasc_sb30_device : public device_t
{
public:
	// construction/destruction
	tasc_sb30_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u8 read();
	void write(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	void out_led(int pos);
	bool piece_available(u8 id);
	void init_cb(int state);
	u8 spawn_cb(offs_t offset);

	required_device<sensorboard_device> m_board;
	output_finder<8,8> m_out_leds;

	u8 m_data;
	u32 m_position;
	u8 m_shift;
};


// device type definition
DECLARE_DEVICE_TYPE(TASC_SB30, tasc_sb30_device)


#endif // MAME_MACHINE_SMARTBOARD_H
