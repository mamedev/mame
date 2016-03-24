// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Hiromitsu Shioya
#pragma once

#ifndef __MSM5232_H__
#define __MSM5232_H__


#define MCFG_MSM5232_SET_CAPACITORS(_a, _b, _c, _d, _e, _f, _g, _h) \
	msm5232_device::static_set_capacitors(*device, _a, _b, _c, _d, _e, _f, _g, _h);

#define MCFG_MSM5232_GATE_HANDLER_CB(_devcb) \
	devcb = &msm5232_device::set_gate_handler_callback(*device, DEVCB_##_devcb);

struct VOICE {
	UINT8 mode;

	int     TG_count_period;
	int     TG_count;

	UINT8   TG_cnt;     /* 7 bits binary counter (frequency output) */
	UINT8   TG_out16;   /* bit number (of TG_cnt) for 16' output */
	UINT8   TG_out8;    /* bit number (of TG_cnt) for  8' output */
	UINT8   TG_out4;    /* bit number (of TG_cnt) for  4' output */
	UINT8   TG_out2;    /* bit number (of TG_cnt) for  2' output */

	int     egvol;
	int     eg_sect;
	int     counter;
	int     eg;

	UINT8   eg_arm;     /* attack/release mode */

	double  ar_rate;
	double  dr_rate;
	double  rr_rate;

	int     pitch;          /* current pitch data */

	int GF;
};


class msm5232_device : public device_t,
									public device_sound_interface
{
public:
	msm5232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~msm5232_device() {}

	static void static_set_capacitors(device_t &device, double cap1, double cap2, double cap3, double cap4, double cap5, double cap6, double cap7, double cap8);
	template<class _Object> static devcb_base &set_gate_handler_callback(device_t &device, _Object object) { return downcast<msm5232_device &>(device).m_gate_handler_cb.set_callback(object); }

	DECLARE_WRITE8_MEMBER( write );
	void set_clock(int clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	private:
	// internal state
	sound_stream *m_stream;

	VOICE   m_voi[8];

	UINT32 m_EN_out16[2]; /* enable 16' output masks for both groups (0-disabled ; ~0 -enabled) */
	UINT32 m_EN_out8[2];  /* enable 8'  output masks */
	UINT32 m_EN_out4[2];  /* enable 4'  output masks */
	UINT32 m_EN_out2[2];  /* enable 2'  output masks */

	int m_noise_cnt;
	int m_noise_step;
	int m_noise_rng;
	int m_noise_clocks;   /* number of the noise_rng (output) level changes */

	unsigned int m_UpdateStep;

	/* rate tables */
	double  m_ar_tbl[8];
	double  m_dr_tbl[16];

	UINT8   m_control1;
	UINT8   m_control2;

	int     m_gate;       /* current state of the GATE output */

	int     m_chip_clock;      /* chip clock in Hz */
	int     m_rate;       /* sample rate in Hz */

	double  m_external_capacity[8]; /* in Farads, eg 0.39e-6 = 0.36 uF (microFarads) */
	devcb_write_line m_gate_handler_cb;/* callback called when the GATE output pin changes state */

	void init_tables();
	void init_voice(int i);
	void gate_update();
	void init(int clock, int rate);
	void EG_voices_advance();
	void TG_group_advance(int groupidx);
	void postload();
};

extern const device_type MSM5232;


#endif /* __MSM5232_H__ */
