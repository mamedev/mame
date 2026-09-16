// license:BSD-3-Clause
// copyright-holders:Katherine Rohl

#ifndef MAME_BUS_ISA_IBM_SPEECH_H
#define MAME_BUS_ISA_IBM_SPEECH_H

#pragma once

#include "isa.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "sound/tms5220.h"
#include "sound/hc55516.h"
#include "sound/spkrdev.h"
#include "speaker.h"

class isa8_ibm_speech_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	isa8_ibm_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t audio_control_latch_r();
	void    audio_control_latch_w(uint8_t data);

	uint8_t shift_register_r();
	void    shift_register_w(uint8_t data);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_device<tms5220_device>         m_lpc;
	required_device<mc3418_device>          m_cvsd;
	required_device<speaker_sound_device>   m_dac1bit;
	required_device<pit8254_device>         m_pit;
	required_device<i8255_device>           m_ppi;
	required_device<speaker_device>         m_speaker;
	required_region_ptr<uint8_t>            m_rom;

	void cvsd_clock_w(int state);        // PIT CH0
	void cvsd_frame_w(int state);        // PIT CH1
	void int_clock_w(int state);         // PIT CH2
	void lpc_interrupt_w(int state);     // 5220 INT#
	void cvsd_shiftreg_clk_w(int state); // SRCLK

	uint8_t porta_r();
	uint8_t portc_r();

	void    porta_w(uint8_t data);
	void    portc_w(uint8_t data);

	void    rom_page_w(uint8_t data);
	void    channel_mux_w(uint8_t data);

	bool    m_lpc_interrupt;
	bool    m_lpc_running;

	bool    m_cvsd_clock;
	bool    m_cvsd_frame;
	bool    m_cvsd_ed;

	bool    m_sr_clk;
	bool    m_beeper_gate;

	bool    m_acl_int_ena;
	bool    m_acl_chan_ena;

	uint8_t m_channel_mux;
	uint8_t m_cvsd_sr_bits_remaining;
	uint8_t m_cvsd_sr_parallel;
	uint8_t m_cvsd_sr_serial;
};

DECLARE_DEVICE_TYPE(ISA8_IBM_SPEECH, isa8_ibm_speech_device)

#endif // MAME_BUS_ISA_IBM_SPEECH_H
