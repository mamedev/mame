// license: BSD-3-Clause
// copyright-holders: Aaron Giles
/***************************************************************************

    Cheap Squeak Deluxe / Artificial Artist Sound Board

***************************************************************************/
#ifndef MAME_MIDWAY_CSD_H
#define MAME_MIDWAY_CSD_H

#pragma once


#include "cpu/m68000/m68000.h"
#include "machine/6821pia.h"
#include "sound/dac.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> midway_cheap_squeak_deluxe_device

class midway_cheap_squeak_deluxe_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	midway_cheap_squeak_deluxe_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 16'000'000);

	// helpers
	void suspend_cpu();

	// read/write
	u8 stat_r();
	void sr_w(u8 data);
	void sirq_w(int state);
	void reset_w(int state);

	void csdeluxe_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(sync_pia);

private:
	// devices
	required_device<m68000_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device<dac_word_interface> m_dac;

	// internal state
	uint8_t    m_status;
	uint16_t   m_dacval;
	emu_timer *m_pia_sync_timer;

	// internal communications
	void porta_w(uint8_t data);
	void portb_w(uint8_t data);
	void irq_w(int state);
};

// device type definition
DECLARE_DEVICE_TYPE(MIDWAY_CHEAP_SQUEAK_DELUXE, midway_cheap_squeak_deluxe_device)

#endif // MAME_MIDWAY_CSD_H
