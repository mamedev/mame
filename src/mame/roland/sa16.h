// license:BSD-3-Clause
// copyright-holders:AJR, Carl Lom
/***************************************************************************

    Roland RF5C36 (15229840) & SA-16 (15229874) Sampler Custom ICs

***************************************************************************/

#ifndef MAME_ROLAND_SA16_H
#define MAME_ROLAND_SA16_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sa16_base_device

class sa16_base_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	// address space indices
	enum {
		AS_WRAM     = 0,
		AS_REGS     = 1,
		AS_CHANREGS = 2
	};

	// callback configuration
	auto int_callback() { return m_int_callback.bind(); }
	auto sh_callback() { return m_sh_callback.bind(); } // output sample-and-hold strobe (not driven yet)

	// CPU read/write handlers
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// base type constructor
	sa16_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device_config_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device-specific overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// drive ENVINT from the pending envelope events
	void update_int();

	// address space configurations
	const address_space_config m_space_config;
	const address_space_config m_space_regs_config;
	const address_space_config m_space_chanregs_config;

private:
	static constexpr int NUM_VOICES = 16;
	static constexpr int CLOCK_DIVIDER = 896; // 26.88 MHz / 896 = 30 kHz
	static constexpr int ENV_RATE_SHIFT = 16; // ARBITRARY: scale of the logarithmic envelope rate (real value unknown)

	void sa16(address_map &map) ATTR_COLD;
	void regs_map(address_map &map) ATTR_COLD;
	void chanregs_map(address_map &map) ATTR_COLD;

	// line callbacks
	devcb_write_line m_int_callback;
	devcb_write_line m_sh_callback;

	u16 wram_r16(offs_t offset) { return space(AS_WRAM).read_word(offset); }
	void wram_w16(offs_t offset, u16 data) { space(AS_WRAM).write_word(offset, data); }

	// byte address in wave RAM of the 4K-word block selected by c011
	offs_t block_base() const { return offs_t(m_block) << 13; }

	// sound output
	sound_stream *m_stream;
	// keeps the stream (and so the ENVINT output) current between the sound
	// system's ~10 ms batch updates; the firmware reacts to ENVINT within ~1 ms
	emu_timer *m_update_timer;
	TIMER_CALLBACK_MEMBER(update_tick);
	memory_access<21, 1, 0, ENDIANNESS_LITTLE>::cache m_wram_cache;

	// per-voice state that is not held in the channel registers
	struct voice_t {
		u8 m_env_level;     // current envelope amplitude (ramped by hardware)
		u32 m_env_frac;     // fractional accumulator for the envelope rate
		bool m_env_armed;   // target written, end-of-ramp event not yet raised
		bool m_alt_back;    // alternate loop: currently playing backwards
	};
	voice_t m_voice[NUM_VOICES];

	// voice register helpers (see the register description in sa16.cpp)
	static int voice_reg(int voice, int reg) { return voice * 0x10 + reg; }
	static int env_reg(int voice, int reg) { return (voice + 1) * 0x10 + reg; }
	s32 voice_counter(int voice) const;
	void set_voice_counter(int voice, s32 counter);
	void env_level_write(int voice, u8 level);
	void raise_env_event(int voice);

	// internal state
	u16 m_active_channels; // key-on mask
	u16 m_env_pending;     // voices with an unacknowledged end-of-ramp event

	int m_reg800_woffset;  // voice register selected for writes (d000+ writes)
	int m_reg800_roffset;  // voice register selected for reads (d000+ reads)
	int m_blockoffset;     // byte offset within the wave RAM block (e000+ access)
	int m_smpcounter;      // sample port word counter
	u16 m_port_smp16;      // sample port value (16-bit)
	u8 m_block;            // wave RAM block (c011)
	u16 m_regs800[2048];   // voice registers
};

// ======================> rf5c36_device

class rf5c36_device : public sa16_base_device
{
public:
	// device type constructor
	rf5c36_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// ======================> sa16_device

class sa16_device : public sa16_base_device
{
public:
	// device type constructor
	sa16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_reset() override ATTR_COLD;
};

// device type declarations
DECLARE_DEVICE_TYPE(RF5C36, rf5c36_device)
DECLARE_DEVICE_TYPE(SA16, sa16_device)

#endif // MAME_ROLAND_SA16_H
