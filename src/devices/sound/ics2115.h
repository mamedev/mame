// license:BSD-3-Clause
// copyright-holders:Alex Marshall,nimitz,austere
#ifndef MAME_SOUND_ICS2115_H
#define MAME_SOUND_ICS2115_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ics2115_device

class ics2115_device : public device_t, public device_sound_interface, public device_memory_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; } // Incorrect/Unverified interrupt, interpolation;
	// current ramping behavior is seems like incorrect?

	// construction/destruction
	ics2115_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq() { return m_irq_cb.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// 16-bit read / write handlers (when /IOCS16 is low)
	u16 word_r(offs_t offset, u16 mem_mask);
	void word_w(offs_t offset, u16 data, u16 mem_mask);
	TIMER_CALLBACK_MEMBER(timer_cb_0);
	TIMER_CALLBACK_MEMBER(timer_cb_1);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_memory_interface configuration
	virtual space_config_vector memory_space_config() const override;

	address_space_config m_data_config;

private:
	static constexpr u16 revision = 0x1;

	struct ics2115_voice {
		struct {
			s32 left;
			u32 acc, start, end; // address counters (20.9 fixed point)
			u16 fc;              // frequency (6.9 fixed point)
			u8 ctl, saddr;
		} osc;

		struct {
			s32 left;
			u32 add;
			u32 start, end;
			u32 acc;
			u16 regacc;
			u8 incr;
			u8 pan, mode;
		} vol;

		union {
			struct {
				u8 ulaw       : 1;   // compressed sample format
				u8 stop       : 1;   // stops wave + vol envelope
				u8 eightbit   : 1;   // 8 bit sample format
				u8 loop       : 1;   // loop enable
				u8 loop_bidir : 1;   // bi-directional loop enable
				u8 irq        : 1;   // enable IRQ generation
				u8 invert     : 1;   // invert direction
				u8 irq_pending: 1;   // (read only?) IRQ pending
				// IRQ on variable?
			} bitflags;
			u8 value;
		} osc_conf;

		union {
			struct {
				u8 done       : 1;   // indicates ramp has stopped
				u8 stop       : 1;   // stops the ramp
				u8 rollover   : 1;   // rollover (TODO)
				u8 loop       : 1;   // loop enable
				u8 loop_bidir : 1;   // bi-directional loop enable
				u8 irq        : 1;   // enable IRQ generation
				u8 invert     : 1;   // invert direction
				u8 irq_pending: 1;   // (read only?) IRQ pending
				// noenvelope == (done | stop)
			} bitflags;
			u8 value;
		} vol_ctrl;

		// Possibly redundant state. => improvements of wavetable logic
		// may lead to its elimination.
		struct {
			bool on;
			int ramp;       // 100 0000 = 0x40 maximum
		} state;

		u16 regs[0x20]; // channel registers
		bool playing();
		int update_volume_envelope();
		int update_oscillator();
		void update_ramp();
	};

	// internal register helper functions
	u16 reg_read();
	void reg_write(u16 data, u16 mem_mask);
	void recalc_timer(int timer);
	void keyon();
	void recalc_irq();

	// stream helper functions
	int fill_output(ics2115_voice& voice, sound_stream &stream);
	s32 get_sample(ics2115_voice& voice);
	u8 read_sample(ics2115_voice& voice, u32 addr) { return m_cache.read_byte((voice.osc.saddr << 20) | (addr & 0xfffff)); }

	sound_stream *m_stream;

	// internal state
	memory_access<24, 0, 0, ENDIANNESS_LITTLE>::cache m_cache;
	required_region_ptr<u8> m_rom;
	devcb_write_line m_irq_cb;

	s16 m_ulaw[256];
	u16 m_volume[4096];
	u16 m_panlaw[256];
	static const int volume_bits = 15;

	ics2115_voice m_voice[32];
	struct {
		u8 scale, preset;
		emu_timer *timer;
		u64 period;  /* in nsec */
	} m_timer[2];

	u8 m_active_osc;
	u8 m_osc_select;
	u8 m_reg_select;
	u8 m_irq_enabled, m_irq_pending;
	bool m_irq_on;

	u16 m_regs[0x40]; // global registers

	/*
	    Unknown variable, seems to be effected by 0x12. Further investigation
	    Required.
	*/
	u8 m_vmode;
};


// device type definition
DECLARE_DEVICE_TYPE(ICS2115, ics2115_device)

#endif // MAME_SOUND_ICS2115_H
