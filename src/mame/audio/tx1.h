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

	DECLARE_WRITE16_MEMBER( z80_busreq_w );
	DECLARE_READ16_MEMBER( dipswitches_r );
	DECLARE_READ16_MEMBER( z80_shared_r );
	DECLARE_WRITE16_MEMBER( z80_shared_w );

	DECLARE_WRITE8_MEMBER( z80_intreq_w );

	DECLARE_WRITE8_MEMBER( ts_w );
	DECLARE_READ8_MEMBER( ts_r );

	DECLARE_READ8_MEMBER( pit8253_r );
	DECLARE_WRITE8_MEMBER( pit8253_w );

	INTERRUPT_GEN_MEMBER( z80_irq );

protected:

	DECLARE_WRITE8_MEMBER( tx1_ppi_latch_w );
	DECLARE_WRITE8_MEMBER( tx1_coin_cnt_w );
	DECLARE_READ8_MEMBER( tx1_ppi_porta_r );
	DECLARE_READ8_MEMBER( tx1_ppi_portb_r );

	DECLARE_WRITE8_MEMBER( ay8910_a_w );
	DECLARE_WRITE8_MEMBER( ay8910_b_w );

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
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

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

	sound_stream *m_stream;
	uint32_t m_freq_to_step;
	uint32_t m_step0;
	uint32_t m_step1;
	uint32_t m_step2;

	pit8253_state m_pit8253;

	uint8_t m_ay_outputa;
	uint8_t m_ay_outputb;

	uint8_t m_ppi_latch_a;
	uint8_t m_ppi_latch_b;
	uint32_t m_ts;

	stream_sample_t m_pit0;
	stream_sample_t m_pit1;
	stream_sample_t m_pit2;

	double m_weights0[4];
	double m_weights1[3];
	double m_weights2[3];
	int m_eng0[4];
	int m_eng1[4];
	int m_eng2[4];

	int m_noise_lfsra;
	int m_noise_lfsrb;
	int m_noise_lfsrc;
	int m_noise_lfsrd;
	int m_noise_counter;
	uint8_t m_ym1_outputa;
	uint8_t m_ym2_outputa;
	uint8_t m_ym2_outputb;
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

	DECLARE_WRITE8_MEMBER( ym2_a_w );
	DECLARE_WRITE8_MEMBER( ym2_b_w );

protected:
	buggyboy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	DECLARE_WRITE8_MEMBER( ym1_a_w );

	DECLARE_READ8_MEMBER( bb_analog_r );
	DECLARE_WRITE8_MEMBER( bb_coin_cnt_w );

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

	DECLARE_READ8_MEMBER( bbjr_analog_r );

	virtual bool has_coin_counters() override { return true; }

	void buggybjr_sound_prg(address_map &map);
};

DECLARE_DEVICE_TYPE(TX1_SOUND, tx1_sound_device)
DECLARE_DEVICE_TYPE(TX1J_SOUND, tx1j_sound_device)
DECLARE_DEVICE_TYPE(BUGGYBOY_SOUND, buggyboy_sound_device)
DECLARE_DEVICE_TYPE(BUGGYBOYJR_SOUND, buggyboyjr_sound_device)

#endif // MAME_AUDIO_TX1_H
