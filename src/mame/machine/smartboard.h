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
	tasc_sb30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t read();
	void write(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;

private:
	TIMER_CALLBACK_MEMBER(leds_off_cb);
	void out_led(int pos);
	bool piece_available(uint8_t id);
	void init_cb(int state);
	uint8_t spawn_cb(offs_t offset);

	required_device<sensorboard_device> m_board;
	output_finder<9,9>        m_out_leds;
	emu_timer *               m_leds_off_timer;
	uint8_t                   m_data;
	uint8_t                   m_position;
	uint8_t                   m_shift;
};


// device type definition
DECLARE_DEVICE_TYPE(TASC_SB30, tasc_sb30_device)


#endif // MAME_MACHINE_SMARTBOARD_H
