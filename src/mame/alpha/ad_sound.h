// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Nicola Salmoria

//****************************************************************************
//    Functions to emulate the Alpha Denshi "59MC07" audio board
//****************************************************************************

#ifndef MAME_ALPHA_AD_SOUND_H
#define MAME_ALPHA_AD_SOUND_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "machine/gen_latch.h"
#include "machine/i8155.h"
#include "sound/dac.h"
#include "sound/msm5232.h"
#include "sound/samples.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(AD_59MC07, ad_59mc07_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ad_59mc07_device

class ad_59mc07_device : public device_t
{
public:
	// construction/destruction
	ad_59mc07_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	void sound_command_w(uint8_t data) { m_soundlatch->write(data); }

protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<i8085a_cpu_device> m_audiocpu;
	required_device<i8155_device> m_audio8155;
	required_device<samples_device> m_samples;
	required_device<msm5232_device> m_msm;
	required_device<dac_byte_interface> m_dac_1;
	required_device<dac_byte_interface> m_dac_2;
	required_device<generic_latch_8_device> m_soundlatch;
	required_ioport m_frq_adjuster;

	int       m_sound_prom_address;
	uint8_t   m_dac_latch;
	uint8_t   m_8155_port_b;
	uint8_t   m_8155_port_a;
	uint8_t   m_8155_port_c;
	uint8_t   m_ay_port_a;
	uint8_t   m_ay_port_b;
	uint8_t   m_cymbal_ctrl;
	float     m_cymvol;
	float     m_hihatvol;
	emu_timer *m_adjuster_timer;

	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;

	void c0f8_w(offs_t offset, uint8_t data);
	void cymbal_ctrl_w(uint8_t data);
	void dac_latch_w(uint8_t data);
	void i8155_porta_w(uint8_t data);
	void i8155_portb_w(uint8_t data);
	void i8155_portc_w(uint8_t data);
	void ay8910_porta_w(uint8_t data);
	void ay8910_portb_w(uint8_t data);
	void i8155_timer_pulse(int state);
	TIMER_CALLBACK_MEMBER(frq_adjuster_callback);
	void update_dac();
	void msm5232_gate(int state);
};


//****************************************************************************
//    Functions to emulate the Alpha Denshi "60MC01" audio board
//****************************************************************************

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(AD_60MC01, ad_60mc01_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ad_60mc01_device

class ad_60mc01_device : public device_t
{
public:
	// construction/destruction
	ad_60mc01_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	static constexpr feature_type imperfect_features() { return feature::SOUND; } // TODO: fix interrupts, missing music?

	void sound_command_w(uint8_t data) { m_soundlatch->write(data); }

protected:
	// device level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;

	void sound_map(address_map &map) ATTR_COLD;
	void sound_portmap(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(sound_irq) { m_audiocpu->set_input_line(0, HOLD_LINE); }
};

#endif // MAME_ALPHA_AD_SOUND_H
