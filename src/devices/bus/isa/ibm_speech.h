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
	// construction/destruction
	isa8_ibm_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t audio_control_latch_r();
	void audio_control_latch_w(uint8_t data);

	uint8_t shift_register_r();
	void shift_register_w(uint8_t data);

	uint8_t rom_r(offs_t offset);
	void rom_w(offs_t offset, uint8_t data) {};

	uint8_t pit_r(offs_t offset);
	void pit_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_reset_after_children() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	required_device<tms5220_device> m_lpc;
	required_device<mc3418_device>  m_cvsd;
	required_device<speaker_sound_device> m_8254_audio;
	required_device<pit8254_device> m_timers;
	required_device<i8255_device> m_ppi;
	required_device<speaker_device> m_speaker;

	required_region_ptr<uint8_t> m_rom;

	uint8_t porta_r();
	uint8_t portc_r();

	void porta_w(uint8_t data);
	void portc_w(uint8_t data);

	void rom_page_w(uint8_t data);
	void channel_mux_w(uint8_t data);

	DECLARE_WRITE_LINE_MEMBER(cvsd_clock_w);    // PIT CH0
	DECLARE_WRITE_LINE_MEMBER(cvsd_frame_w);    // PIT CH1
	DECLARE_WRITE_LINE_MEMBER(int_clock_w);     // PIT CH2
	DECLARE_WRITE_LINE_MEMBER(lpc_interrupt_w);

	bool    m_lpc_interrupt;
	bool    m_lpc_running;
	bool    m_cvsd_clock;
	bool    m_cvsd_frame;
	bool    m_cvsd_ed;
	bool    m_sr_clk;
	bool    m_sr_sck;
	bool    m_beeper_gate;

	bool    m_acl_int_ena;
	bool    m_acl_chan_ena;

	uint8_t m_cvsd_sr_bits_remaining;

	uint8_t m_rom_page;
	uint8_t m_channel_mux;

	// CVSD serialization
	uint8_t m_cvsd_sr_parallel;
	uint8_t m_cvsd_sr_serial;
	DECLARE_WRITE_LINE_MEMBER(cvsd_shiftreg_clk_w);
};

DECLARE_DEVICE_TYPE(ISA8_IBM_SPEECH, isa8_ibm_speech_device)

#endif // MAME_BUS_ISA_IBM_SPEECH_H
