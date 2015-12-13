// license:???
// copyright-holders:Derrick Renaud, Couriersud
#pragma once

#ifndef __FILTER_H__
#define __FILTER_H__

/* Max filter order */
#define FILTER_ORDER_MAX 51

/* Define to use integer calculation */
#define FILTER_USE_INT

#ifdef FILTER_USE_INT
typedef int filter_real;
#define FILTER_INT_FRACT 15 /* fractional bits */
#else
typedef double filter_real;
#endif

struct filter
{
	filter_real xcoeffs[(FILTER_ORDER_MAX+1)/2];
	unsigned order;
};

struct filter_state
{
	unsigned prev_mac;
	filter_real xprev[FILTER_ORDER_MAX];
};

/* Allocate a FIR Low Pass filter */
filter* filter_lp_fir_alloc(double freq, int order);
void filter_free(filter* f);

/* Allocate a filter state */
filter_state* filter_state_alloc(void);

/* Free the filter state */
void filter_state_free(filter_state* s);

/* Clear the filter state */
void filter_state_reset(filter* f, filter_state* s);

/* Insert a value in the filter state */
static inline void filter_insert(filter* f, filter_state* s, filter_real x) {
	/* next state */
	++s->prev_mac;
	if (s->prev_mac >= f->order)
		s->prev_mac = 0;

	/* set x[0] */
	s->xprev[s->prev_mac] = x;
}

/* Compute the filter output */
filter_real filter_compute(filter* f, filter_state* s);


/* Filter types */
#define FILTER_LOWPASS      0
#define FILTER_HIGHPASS     1
#define FILTER_BANDPASS     2

#define Q_TO_DAMP(q)    (1.0/q)

struct filter2_context
{
	filter2_context() :
		x0(0.0),
		x1(0.0),
		x2(0.0),
		y0(0.0),
		y1(0.0),
		y2(0.0),
		a1(0.0),
		a2(0.0),
		b0(0.0),
		b1(0.0),
		b2(0.0)
	{}

	double x0, x1, x2;  /* x[k], x[k-1], x[k-2], current and previous 2 input values */
	double y0, y1, y2;  /* y[k], y[k-1], y[k-2], current and previous 2 output values */
	double a1, a2;      /* digital filter coefficients, denominator */
	double b0, b1, b2;  /* digital filter coefficients, numerator */
};


/* Setup the filter context based on the passed filter type info.
 * type - 1 of the 3 defined filter types
 * fc   - center frequency
 * d    - damp = 1/Q
 * gain - overall filter gain. Set to 1 if not needed.
 */
void filter2_setup(device_t *device, int type, double fc, double d, double gain,
					filter2_context *filter2);


/* Reset the input/output voltages to 0. */
void filter2_reset(filter2_context *filter2);


/* Step the filter.
 * x0 is the new input, which needs to be set before stepping.
 * y0 is the new filter output.
 */
void filter2_step(filter2_context *filter2);


/* Setup a filter2 structure based on an op-amp multipole bandpass circuit.
 * NOTE: If r2 is not used then set to 0.
 *       vRef is not needed to setup filter.
 *
 *                             .--------+---------.
 *                             |        |         |
 *                            --- c1    Z         |
 *                            ---       Z r3      |
 *                             |        Z         |
 *            r1               |  c2    |  |\     |
 *   In >----ZZZZ----+---------+--||----+  | \    |
 *                   Z                  '--|- \   |
 *                   Z r2                  |   >--+------> out
 *                   Z                  .--|+ /
 *                   |                  |  | /
 *                  gnd        vRef >---'  |/
 *
 */
void filter_opamp_m_bandpass_setup(device_t *device, double r1, double r2, double r3, double c1, double c2,
					filter2_context *filter2);

#endif /* __FILTER_H__ */
