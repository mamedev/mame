// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Tatsumi TX-1/Buggy Boy sound hardware

***************************************************************************/

#ifndef MAME_AUDIO_TX1_H
#define MAME_AUDIO_TX1_H

#pragma once

#include "machine/i8255.h"
#include "sound/ay8910.h"

class tx1_sound_device : public device_t, public device_sound_interface
{
public:
	tx1_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void z80_busreq_w(uint16_t data);
	uint16_t dipswitches_r();
	uint16_t z80_shared_r(offs_t offset);
	void z80_shared_w(offs_t offset, uint16_t data);

	void z80_intreq_w(uint8_t data);

	void ts_w(offs_t offset, uint8_t data);
	uint8_t ts_r(offs_t offset);

	uint8_t pit8253_r(offs_t offset);
	void pit8253_w(offs_t offset, uint8_t data);

	INTERRUPT_GEN_MEMBER( z80_irq );

protected:

	void tx1_ppi_latch_w(uint8_t data);
	void tx1_coin_cnt_w(uint8_t data);
	uint8_t tx1_ppi_porta_r();
	uint8_t tx1_ppi_portb_r();

	void ay8910_a_w(uint8_t data);
	void ay8910_b_w(uint8_t data);

	/*************************************
	 *
	 *  8253 Programmable Interval Timer
	 *
	 *************************************/
	struct pit8253_state
	{
		union
		{
#ifdef LSB_FIRST
			struct { uint8_t LSB; uint8_t MSB; } as8bit;
#else
			struct { uint8_t MSB; uint8_t LSB; } as8bit;
#endif
			uint16_t val;
		} counts[3];

		int idx[3];
	};

	tx1_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	void tx1_sound_io(address_map &map);
	void tx1_sound_prg(address_map &map);

	// internal state
	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<uint8_t> m_z80_ram;
	optional_device<i8255_device> m_ppi;

	required_ioport m_dsw;
	required_ioport m_steering;
	required_ioport m_accelerator;
	required_ioport m_brake;
	optional_ioport m_ppi_portd;

	sound_stream *m_stream = nullptr;
	uint32_t m_freq_to_step = 0;
	uint32_t m_step0 = 0;
	uint32_t m_step1 = 0;
	uint32_t m_step2 = 0;

	pit8253_state m_pit8253;

	uint8_t m_ay_outputa = 0;
	uint8_t m_ay_outputb = 0;

	uint8_t m_ppi_latch_a = 0;
	uint8_t m_ppi_latch_b = 0;
	uint32_t m_ts = 0;

	s32 m_pit0 = 0;
	s32 m_pit1 = 0;
	s32 m_pit2 = 0;

	double m_weights0[4] = { 0, 0, 0, 0 };
	double m_weights1[3] = { 0, 0, 0 };
	double m_weights2[3] = { 0, 0, 0 };
	int m_eng0[4] = { 0, 0, 0, 0 };
	int m_eng1[4] = { 0, 0, 0, 0 };
	int m_eng2[4] = { 0, 0, 0, 0 };

	int m_noise_lfsra = 0;
	int m_noise_lfsrb = 0;
	int m_noise_lfsrc = 0;
	int m_noise_lfsrd = 0;
	int m_noise_counter = 0;
	uint8_t m_ym1_outputa = 0;
	uint8_t m_ym2_outputa = 0;
	uint8_t m_ym2_outputb = 0;
	uint16_t m_eng_voltages[16];
};

class tx1j_sound_device : public tx1_sound_device
{
public:
	tx1j_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
};

class buggyboy_sound_device : public tx1_sound_device
{
public:
	buggyboy_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void ym2_a_w(uint8_t data);
	void ym2_b_w(uint8_t data);

protected:
	buggyboy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	void ym1_a_w(uint8_t data);

	uint8_t bb_analog_r(offs_t offset);
	void bb_coin_cnt_w(uint8_t data);

	virtual bool has_coin_counters() { return false; }

	void buggyboy_sound_io(address_map &map);
	void buggyboy_sound_prg(address_map &map);

	required_device_array<ym2149_device, 2> m_ym;
};

class buggyboyjr_sound_device : public buggyboy_sound_device
{
public:
	buggyboyjr_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	uint8_t bbjr_analog_r(offs_t offset);

	virtual bool has_coin_counters() override { return true; }

	void buggybjr_sound_prg(address_map &map);
};

DECLARE_DEVICE_TYPE(TX1_SOUND, tx1_sound_device)
DECLARE_DEVICE_TYPE(TX1J_SOUND, tx1j_sound_device)
DECLARE_DEVICE_TYPE(BUGGYBOY_SOUND, buggyboy_sound_device)
DECLARE_DEVICE_TYPE(BUGGYBOYJR_SOUND, buggyboyjr_sound_device)

#endif // MAME_AUDIO_TX1_H
