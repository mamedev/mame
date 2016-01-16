// license:BSD-3-Clause
// copyright-holders:Alex Marshall,nimitz,austere
#pragma once

#ifndef __ICS2115_H__
#define __ICS2115_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ICS2115_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, ICS2115, _clock)

#define MCFG_ICS2115_IRQ_CB(_devcb) \
	devcb = &ics2115_device::set_irq_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct ics2115_voice {
	struct {
		INT32 left;
		UINT32 acc, start, end;
		UINT16 fc;
		UINT8 ctl, saddr;
	} osc;

	struct {
		INT32 left;
		UINT32 add;
		UINT32 start, end;
		UINT32 acc;
		UINT16 regacc;
		UINT8 incr;
		UINT8 pan, mode;
	} vol;

	union {
		struct {
			UINT8 ulaw       : 1;
			UINT8 stop       : 1;   //stops wave + vol envelope
			UINT8 eightbit   : 1;
			UINT8 loop       : 1;
			UINT8 loop_bidir : 1;
			UINT8 irq        : 1;
			UINT8 invert     : 1;
			UINT8 irq_pending: 1;
			//IRQ on variable?
		} bitflags;
		UINT8 value;
	} osc_conf;

	union {
		struct {
			UINT8 done       : 1;   //indicates ramp has stopped
			UINT8 stop       : 1;   //stops the ramp
			UINT8 rollover   : 1;   //rollover (TODO)
			UINT8 loop       : 1;
			UINT8 loop_bidir : 1;
			UINT8 irq        : 1;   //enable IRQ generation
			UINT8 invert     : 1;   //invert direction
			UINT8 irq_pending: 1;   //(read only) IRQ pending
			//noenvelope == (done | disable)
		} bitflags;
		UINT8 value;
	} vol_ctrl;

	//Possibly redundant state. => improvements of wavetable logic
	//may lead to its elimination.
	union {
		struct {
			UINT8 on         : 1;
			UINT8 ramp       : 7;       // 100 0000 = 0x40 maximum
		} bitflags;
		UINT8 value;
	} state;

	bool playing();
	int update_volume_envelope();
	int update_oscillator();
	void update_ramp();
};

// ======================> ics2115_device

class ics2115_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	ics2115_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_callback(device_t &device, _Object object) { return downcast<ics2115_device &>(device).m_irq_cb.set_callback(object); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	//UINT8 read(offs_t offset);
	//void write(offs_t offset, UINT8 data);
	TIMER_CALLBACK_MEMBER(timer_cb_0);
	TIMER_CALLBACK_MEMBER(timer_cb_1);

	sound_stream *m_stream;

	static const UINT16 revision = 0x1;

protected:

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal callbacks
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	// internal state
	required_region_ptr<UINT8> m_rom;
	devcb_write_line m_irq_cb;

	INT16 m_ulaw[256];
	UINT16 m_volume[4096];
	static const int volume_bits = 15;

	ics2115_voice m_voice[32];
	struct {
		UINT8 scale, preset;
		emu_timer *timer;
		UINT64 period;  /* in nsec */
	} m_timer[2];

	UINT8 m_active_osc;
	UINT8 m_osc_select;
	UINT8 m_reg_select;
	UINT8 m_irq_enabled, m_irq_pending;
	bool m_irq_on;

	//Unknown variable, seems to be effected by 0x12. Further investigation
	//Required.
	UINT8 m_vmode;

	//internal register helper functions
	UINT16 reg_read();
	void reg_write(UINT8 data, bool msb);
	void recalc_timer(int timer);
	void keyon();
	void recalc_irq();

	//stream helper functions
	int fill_output(ics2115_voice& voice, stream_sample_t *outputs[2], int samples);
	stream_sample_t get_sample(ics2115_voice& voice);
};


// device type definition
extern const device_type ICS2115;

#endif /* __ICS2115_H__ */
