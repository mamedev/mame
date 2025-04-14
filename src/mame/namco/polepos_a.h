#ifndef MAME_NAMCO_POLEPOS_A_H
#define MAME_NAMCO_POLEPOS_A_H

#pragma once

#include "sound/discrete.h"


class polepos_sound_device : public device_t,
								public device_sound_interface
{
public:
	polepos_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~polepos_sound_device() { }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

public:
	void clson_w(int state);
	void polepos_engine_sound_lsb_w(uint8_t data);
	void polepos_engine_sound_msb_w(uint8_t data);

private:
	struct filter2_context
	{
		filter2_context() { }

		void setup(device_t *device, int type, double fc, double d, double gain);
		void opamp_m_bandpass_setup(device_t *device, double r1, double r2, double r3, double c1, double c2);
		void reset();
		void step();

		double x0 = 0.0, x1 = 0.0, x2 = 0.0;  /* x[k], x[k-1], x[k-2], current and previous 2 input values */
		double y0 = 0.0, y1 = 0.0, y2 = 0.0;  /* y[k], y[k-1], y[k-2], current and previous 2 output values */
		double a1 = 0.0, a2 = 0.0;            /* digital filter coefficients, denominator */
		double b0 = 0.0, b1 = 0.0, b2 = 0.0;  /* digital filter coefficients, numerator */
	};

	uint32_t m_current_position;
	int m_sample_msb;
	int m_sample_lsb;
	int m_sample_enable;
	sound_stream *m_stream;
	filter2_context m_filter_engine[3];
};

DECLARE_DEVICE_TYPE(POLEPOS_SOUND, polepos_sound_device)

DISCRETE_SOUND_EXTERN( polepos_discrete );

#endif // MAME_NAMCO_POLEPOS_A_H
