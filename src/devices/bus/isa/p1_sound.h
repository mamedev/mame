// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 sound card

**********************************************************************/

#ifndef MAME_BUS_P1_SOUND_H
#define MAME_BUS_P1_SOUND_H

#pragma once


#include "isa.h"
#include "bus/midi/midi.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "sound/dac.h"
#include "sound/flt_rc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class p1_sound_device : public device_t,
	public device_isa8_card_interface
{
public:
	// construction/destruction
	p1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t d14_r(offs_t offset);
	uint8_t d16_r(offs_t offset);
	uint8_t d17_r(offs_t offset);
	void d14_w(offs_t offset, uint8_t data);
	void d16_w(offs_t offset, uint8_t data);
	void d17_w(offs_t offset, uint8_t data);

	uint8_t adc_r(offs_t offset);
	void dac_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// Optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void sampler_sync(int state);

	uint8_t m_dac_data[16];
	int m_dac_ptr;

	required_device<dac_byte_interface> m_dac;
	optional_device<filter_rc_device> m_filter;
	required_device<i8251_device> m_midi;
	required_device<pit8253_device> m_d14;
	required_device<pit8253_device> m_d16;
	required_device<pit8253_device> m_d17;
};


// device type definition
DECLARE_DEVICE_TYPE(P1_SOUND, p1_sound_device)


#endif // MAME_BUS_P1_SOUND_H
