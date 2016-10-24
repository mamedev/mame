// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Poisk-1 sound card

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __P1_SOUND__
#define __P1_SOUND__

#include "emu.h"

#include "bus/midi/midi.h"
#include "isa.h"
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

	// Optional information overrides
	virtual machine_config_constructor  device_mconfig_additions() const override;

	uint8_t d14_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t d16_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t d17_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void d14_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void d16_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void d17_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void sampler_sync(int state);
	uint8_t adc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint8_t                           m_dac_data[16];
	int                             m_dac_ptr;

	required_device<dac_byte_interface> m_dac;
	optional_device<filter_rc_device>   m_filter;
	required_device<i8251_device>   m_midi;
	required_device<pit8253_device> m_d14;
	required_device<pit8253_device> m_d16;
	required_device<pit8253_device> m_d17;
};


// device type definition
extern const device_type P1_SOUND;


#endif
