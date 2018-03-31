// license:BSD-3-Clause
// copyright-holders:Alex Marshall,nimitz,austere
#ifndef MAME_SOUND_ICS2115_H
#define MAME_SOUND_ICS2115_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ICS2115_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, ICS2115, _clock)

#define MCFG_ICS2115_IRQ_CB(_devcb) \
	devcb = &downcast<ics2115_device &>(*device).set_irq_callback(DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ics2115_device

class ics2115_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	ics2115_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_irq_callback(Object &&cb) { return m_irq_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	//uint8_t read(offs_t offset);
	//void write(offs_t offset, uint8_t data);
	TIMER_CALLBACK_MEMBER(timer_cb_0);
	TIMER_CALLBACK_MEMBER(timer_cb_1);

protected:
	static constexpr uint16_t revision = 0x1;

	struct ics2115_voice {
		struct {
			int32_t left;
			uint32_t acc, start, end;
			uint16_t fc;
			uint8_t ctl, saddr;
		} osc;

		struct {
			int32_t left;
			uint32_t add;
			uint32_t start, end;
			uint32_t acc;
			uint16_t regacc;
			uint8_t incr;
			uint8_t pan, mode;
		} vol;

		union {
			struct {
				uint8_t ulaw       : 1;
				uint8_t stop       : 1;   //stops wave + vol envelope
				uint8_t eightbit   : 1;
				uint8_t loop       : 1;
				uint8_t loop_bidir : 1;
				uint8_t irq        : 1;
				uint8_t invert     : 1;
				uint8_t irq_pending: 1;
				//IRQ on variable?
			} bitflags;
			uint8_t value;
		} osc_conf;

		union {
			struct {
				uint8_t done       : 1;   //indicates ramp has stopped
				uint8_t stop       : 1;   //stops the ramp
				uint8_t rollover   : 1;   //rollover (TODO)
				uint8_t loop       : 1;
				uint8_t loop_bidir : 1;
				uint8_t irq        : 1;   //enable IRQ generation
				uint8_t invert     : 1;   //invert direction
				uint8_t irq_pending: 1;   //(read only) IRQ pending
				//noenvelope == (done | disable)
			} bitflags;
			uint8_t value;
		} vol_ctrl;

		//Possibly redundant state. => improvements of wavetable logic
		//may lead to its elimination.
		union {
			struct {
				uint8_t on         : 1;
				uint8_t ramp       : 7;       // 100 0000 = 0x40 maximum
			} bitflags;
			uint8_t value;
		} state;

		bool playing();
		int update_volume_envelope();
		int update_oscillator();
		void update_ramp();
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	//internal register helper functions
	uint16_t reg_read();
	void reg_write(uint8_t data, bool msb);
	void recalc_timer(int timer);
	void keyon();
	void recalc_irq();

	//stream helper functions
	int fill_output(ics2115_voice& voice, stream_sample_t *outputs[2], int samples);
	stream_sample_t get_sample(ics2115_voice& voice);

	sound_stream *m_stream;

	// internal state
	required_region_ptr<uint8_t> m_rom;
	devcb_write_line m_irq_cb;

	int16_t m_ulaw[256];
	uint16_t m_volume[4096];
	static const int volume_bits = 15;

	ics2115_voice m_voice[32];
	struct {
		uint8_t scale, preset;
		emu_timer *timer;
		uint64_t period;  /* in nsec */
	} m_timer[2];

	uint8_t m_active_osc;
	uint8_t m_osc_select;
	uint8_t m_reg_select;
	uint8_t m_irq_enabled, m_irq_pending;
	bool m_irq_on;

	//Unknown variable, seems to be effected by 0x12. Further investigation
	//Required.
	uint8_t m_vmode;
};


// device type definition
DECLARE_DEVICE_TYPE(ICS2115, ics2115_device)

#endif // MAME_SOUND_ICS2115_H
