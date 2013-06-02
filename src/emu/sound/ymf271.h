#pragma once

#ifndef __YMF271_H__
#define __YMF271_H__

#include "emu.h"

#define MCFG_YMF271_IRQ_HANDLER(_devcb) \
	devcb = &ymf271_device::set_irq_handler(*device, DEVCB2_##_devcb);

#define MCFG_YMF271_EXT_READ_HANDLER(_devcb) \
	devcb = &ymf271_device::set_ext_read_handler(*device, DEVCB2_##_devcb);

#define MCFG_YMF271_EXT_WRITE_HANDLER(_devcb) \
	devcb = &ymf271_device::set_ext_write_handler(*device, DEVCB2_##_devcb);

class ymf271_device : public device_t,
									public device_sound_interface
{
public:
	ymf271_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb2_base &set_irq_handler(device_t &device, _Object object) { return downcast<ymf271_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_ext_read_handler(device_t &device, _Object object) { return downcast<ymf271_device &>(device).m_ext_read_handler.set_callback(object); }
	template<class _Object> static devcb2_base &set_ext_write_handler(device_t &device, _Object object) { return downcast<ymf271_device &>(device).m_ext_write_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	struct YMF271Slot
	{
		INT8  ext_en;
		INT8  ext_out;
		UINT8 lfoFreq;
		INT8  lfowave;
		INT8  pms, ams;
		INT8  detune;
		INT8  multiple;
		INT8  tl;
		INT8  keyscale;
		INT8  ar;
		INT8  decay1rate, decay2rate;
		INT8  decay1lvl;
		INT8  relrate;
		INT32 fns;
		INT8  block;
		INT8  feedback;
		INT8  waveform;
		INT8  accon;
		INT8  algorithm;
		INT8  ch0_level, ch1_level, ch2_level, ch3_level;

		UINT32 startaddr;
		UINT32 loopaddr;
		UINT32 endaddr;
		INT8   altloop;
		INT8   fs, srcnote, srcb;

		INT64 step;
		INT64 stepptr;

		INT8 active;
		INT8 bits;

		// envelope generator
		INT32 volume;
		INT32 env_state;
		INT32 env_attack_step;      // volume increase step in attack state
		INT32 env_decay1_step;
		INT32 env_decay2_step;
		INT32 env_release_step;

		INT64 feedback_modulation0;
		INT64 feedback_modulation1;

		INT32 lfo_phase, lfo_step;
		INT32 lfo_amplitude;
		double lfo_phasemod;
	};

	struct YMF271Group
	{
		INT8 sync, pfm;
	};

	void init_state();
	void calculate_step(YMF271Slot *slot);
	void update_envelope(YMF271Slot *slot);
	void init_envelope(YMF271Slot *slot);
	void init_lfo(YMF271Slot *slot);
	void update_lfo(YMF271Slot *slot);
	int calculate_slot_volume(YMF271Slot *slot);
	void update_pcm(int slotnum, INT32 *mixp, int length);
	INT32 calculate_2op_fm_0(int slotnum1, int slotnum2);
	INT32 calculate_2op_fm_1(int slotnum1, int slotnum2);
	INT32 calculate_1op_fm_0(int slotnum, int phase_modulation);
	INT32 calculate_1op_fm_1(int slotnum);
	void write_register(int slotnum, int reg, int data);
	void ymf271_write_fm(int grp, int adr, int data);
	void ymf271_write_pcm(int data);
	UINT8 ymf271_read_memory(UINT32 offset);
	void ymf271_write_timer(int data);

	// internal state
	YMF271Slot m_slots[48];
	YMF271Group m_groups[12];

	INT32 m_timerA;
	INT32 m_timerB;
	INT32 m_irqstate;
	INT8  m_status;
	INT8  m_enable;

	INT8  m_reg0;
	INT8  m_reg1;
	INT8  m_reg2;
	INT8  m_reg3;
	INT8  m_pcmreg;
	INT8  m_timerreg;
	UINT32 m_ext_address;
	UINT8 m_ext_rw;
	UINT8 m_ext_readlatch;

	UINT8 *m_region_base;
	UINT32 m_region_size;
	UINT32 m_clock;

	emu_timer *m_timA, *m_timB;
	sound_stream *m_stream;

	devcb2_write_line m_irq_handler;
	devcb2_read8 m_ext_read_handler;
	devcb2_write8 m_ext_write_handler;
};

extern const device_type YMF271;


#endif /* __YMF271_H__ */
