// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_eg.h"
#include "machine/rescap.h"

#define LOG_PARAMS      (1U << 1)
#define LOG_CONVERGENCE (1U << 2)

#define VERBOSE (0)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


// The envelope is considered completed after this many time constants.
static constexpr const float TIME_CONSTANTS_TO_END = 10;


va_rc_eg_device::va_rc_eg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VA_RC_EG, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	// Initialize to a valid state.
	, m_r(RES_M(1))
	, m_c(CAP_U(1))
	, m_rc_inv(1.0F / (m_r * m_c))
	, m_v_start(0)
	, m_v_end(0)
{
}

va_rc_eg_device &va_rc_eg_device::set_r(float r)
{
	assert(r > 0);
	if (r == m_r)
		return *this;
	if (m_stream != nullptr)
		m_stream->update();

	snapshot();  // Snapshots voltage using the old `r` value.
	m_r = r;
	m_rc_inv = 1.0F / (m_r * m_c);
	return *this;
}

va_rc_eg_device &va_rc_eg_device::set_c(float c)
{
	assert(c > 0);
	if (c == m_c)
		return *this;
	if (m_stream != nullptr)
		m_stream->update();

	snapshot();  // Snapshots voltage using the old `c` value.
	m_c = c;
	m_rc_inv = 1.0F / (m_r * m_c);
	return *this;
}

va_rc_eg_device &va_rc_eg_device::set_target_v(float v)
{
	if (v == m_v_end)
		return *this;
	if (m_stream != nullptr)
		m_stream->update();

	snapshot();
	m_v_end = v;
	return *this;
}

va_rc_eg_device &va_rc_eg_device::set_instant_v(float v)
{
	if (m_stream != nullptr)
		m_stream->update();

	m_v_start = v;
	m_v_end = v;
	m_t_start = has_running_machine() ? machine().time() : attotime::zero;
	m_t_end_approx = m_t_start;
	return *this;
}

float va_rc_eg_device::get_v(const attotime &t) const
{
	assert(t >= m_t_start);
	const float delta_t = float((t - m_t_start).as_double());
	return m_v_start + (m_v_end - m_v_start) * (1.0F - expf(-delta_t * m_rc_inv));
}

float va_rc_eg_device::get_v() const
{
	return get_v(machine().time());
}

attotime va_rc_eg_device::get_dt(float v) const
{
	if (m_v_start == m_v_end)
	{
		// This only happens after a set_instant_v. If v != m_v_start, then it
		// is unreachable. If v == m_vstart, then we (somewhat arbitrarily)
		// consider it as having been reached in the past.
		return attotime::never;
	}
	if (m_v_start < m_v_end && (v < m_v_start || v >= m_v_end))
		return attotime::never;
	if (m_v_start > m_v_end && (v > m_v_start || v <= m_v_end))
		return attotime::never;

	const double t_from_start = -m_r * m_c * log((m_v_end - v) / (m_v_end - m_v_start));
	const attotime t_abs = m_t_start + attotime::from_double(t_from_start);
	const attotime now = has_running_machine() ? machine().time() : attotime::zero;
	if (t_abs < now)
		return attotime::never;
	return t_abs - now;
}

void va_rc_eg_device::device_start()
{
	if (get_sound_requested_outputs() > 0)
		m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	else
		m_stream = nullptr;

	save_item(NAME(m_r));
	save_item(NAME(m_c));
	save_item(NAME(m_rc_inv));
	save_item(NAME(m_v_start));
	save_item(NAME(m_v_end));
	save_item(NAME(m_t_start));
	save_item(NAME(m_t_end_approx));
}

void va_rc_eg_device::sound_stream_update(sound_stream &stream)
{
	assert(stream.input_count() == 0 && stream.output_count() == 1);
	attotime t = stream.start_time();

	if (converged(t))
	{
		// Avoid expensive get_v() calls if the envelope stage has completed.
		stream.fill(0, m_v_end);
		return;
	}

	const int n = stream.samples();
	const attotime dt = stream.sample_period();
	for (int i = 0; i < n; ++i, t += dt)
		stream.put(0, i, get_v(t));
}

void va_rc_eg_device::snapshot()
{
	if (has_running_machine())
	{
		const attotime now = machine().time();
		m_v_start = get_v(now);
		m_t_start = now;
	}
	else
	{
		m_v_start = 0;
		m_t_start = attotime::zero;
	}
	m_t_end_approx = m_t_start + attotime::from_double(TIME_CONSTANTS_TO_END * m_r * m_c);
}


va_ota_eg_device::va_ota_eg_device(const machine_config &mconfig, const char *tag, device_t *owner, ota_type ota, float c)
	: device_t(mconfig, VA_OTA_EG, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_c(c)
	, m_max_iout_scale(1)
	, m_plus_scale(1)
	, m_minus_scale(1)
	, m_stream(nullptr)
	, m_converged(true)
	, m_g(0)
	, m_target_v(0)
	, m_iabc(1E-3F)
	, m_v(0)
{
	// Based on the "Peak Output Current" specs in the datasheets.
	switch (ota)
	{
		case ota_type::LM13600:
		case ota_type::LM13700:
		case ota_type::CA3080:
			m_max_iout_scale = 1.0F;
			break;
		case ota_type::CA3280:
			m_max_iout_scale = 4.1F / 5.0F;
			break;
		default:
			fatalerror("%s: Unrecognized ota_type.\n", tag);
	}
}

va_ota_eg_device::va_ota_eg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: va_ota_eg_device(mconfig, tag, owner, ota_type::CA3280, CAP_U(1))
{
}

va_ota_eg_device &va_ota_eg_device::configure_plus_divider(float r_in, float r_gnd)
{
	m_plus_scale = RES_VOLTAGE_DIVIDER(r_in, r_gnd);
	return *this;
}

va_ota_eg_device &va_ota_eg_device::configure_minus_divider(float r_in, float r_gnd)
{
	m_minus_scale = RES_VOLTAGE_DIVIDER(r_in, r_gnd);
	return *this;
}

void va_ota_eg_device::set_target_v(float v)
{
	if (v == m_target_v)
		return;
	m_stream->update();
	m_target_v = v;
	m_converged = false;
	LOGMASKED(LOG_PARAMS, "%s: Target V: %f\n", tag(), m_target_v);
}

void va_ota_eg_device::set_iabc(float iabc)
{
	if (iabc == m_iabc)
		return;
	m_stream->update();
	m_iabc = iabc;
	recalc();
	LOGMASKED(LOG_PARAMS, "%s: Iabc: %e\n", tag(), m_iabc);
}

void va_ota_eg_device::device_start()
{
	m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	save_item(NAME(m_converged));
	save_item(NAME(m_g));
	save_item(NAME(m_target_v));
	save_item(NAME(m_iabc));
	save_item(NAME(m_v));
}

void va_ota_eg_device::device_reset()
{
	recalc();
}

void va_ota_eg_device::recalc()
{
	const float h = 1.0F / float(m_stream->sample_rate());
	m_g = (h / 2.0F) * m_iabc * m_max_iout_scale / m_c;
}

void va_ota_eg_device::sound_stream_update(sound_stream &stream)
{
	/*
	For an OTA: Iout = s * Iabc * tanh((vp - vm) / (2 * VT))
	Where:
	  s ~ A device-specific scaling factor. Typically 1.
	  Iabc ~ Control current.
	  vp ~ Voltage at the '+' input.
	  vm ~ Voltage at the '-' input.
	  VT ~ Thermal voltage. A temperature-dependent constant.

	In this EG application, the target voltage (Vtarget) is applied to 'vp', and
	the EG output (v(t)) is fed back to 'vm'.
	Let x(t) = (Vtarget - v(t)) / (2 * VT)
	Therefore: Iout(t) = s * Iabc * tanh(x(t))

	Simulate using the trapezoidal rule:
	v(t+1) = v(t) + (h/2) * (v'(t) + v'(t+1)), where 'h' is the timestep.

	In this EG application, the OTA's output current is (dis)charging a
	capacitor. Therefore:
	v'(t) = Iout(t) / C = (s * Iabc / C) * tanh(x(t))

	Let G = (s * Iabc / C) * (h / 2). Now:
	v(t+1) = v(t) + G * tanh(x(t)) + G * tanh(x(t+1))

	Solving for v(t+1) is only possible with iterative methods, since it also
	appears inside a tanh (recall that x(t+1) is a function of v(t+1)). Instead,
	use the "linearization at operating point" approximation:
	tanh(x(t+1)) = tanh(x(t)) + (x(t+1) - x(t)) * tanh'(x(t))

	Substitute on the simulation equation to get:
	v(t+1) = v(t) + G * tanh(x(t)) + G * (tanh(x(t)) + (x(t+1) - x(t)) * tanh'(x(t)))

	Substitute the `x()`s that are outside the `tanh()`s and simplify to get:
	v(t+1) = v(t) + 2 * G * tanh(x(t)) + (G / (2 * VT)) * (-v(t+1) + v(t)) * tanh'(x(t))

	Now it is possible to solve for v(t+1). After some algebra:
	v(t+1) = v(t) + (2 * G * tanh(x(t))) / (1 + (G / (2 * VT)) * tanh'(x(t)))


	Note that tanh((Vtarget - v(t)) / (2 * VT)) will saturate when
	abs(Vtarget - v(t)) > ~10 * VT. When that happens:
	tan(x(t)) = tan(x(t+1)) = +/- 1 (+ or - depending on whether Vtarget is
	larger or smaller than v(t) respectively).

	During saturation, the simulation equation simplifies to a linear ramp:
	v(t+1) = v(t) + G * tanh(x(t)) + G * tanh(x(t+1))  ==>
	v(t+1) = v(t) +/- 2 * G
	*/

	constexpr float VT = 25.7E-3F;  // Thermal voltage at 25 degrees C.
	constexpr float INV_2VT = 1.0F / (2.0F * VT);
	constexpr float V_OTA_SAT = 10 * VT;

	// If the two inputs are scaled by different amounts, the EG will converge
	// off target.
	const float conv_v = m_target_v * m_plus_scale / m_minus_scale;
	if (m_converged)
	{
		stream.fill(0, conv_v);
		return;
	}

	// Cache some computations for the loop.
	const float scaled_target_v = m_plus_scale * m_target_v;
	const float g2 = m_g * 2.0F;
	const float gvt = m_g * INV_2VT;

	float v_step = 0;
	const float last_v = m_v;
	const int n = stream.samples();

	for (int i = 0; i < n; ++i)
	{
		const float dv = scaled_target_v - m_minus_scale * m_v;
		if (dv > V_OTA_SAT)
		{
			v_step = g2;
		}
		else if (dv < -V_OTA_SAT)
		{
			v_step = -g2;
		}
		else
		{
			const float tanhv = tanhf(INV_2VT * dv);
			const float dtanhv = 1.0F - tanhv * tanhv;  // tanh'(v) = sech(v) ^ 2 = 1 - tanh(v) ^ 2
			v_step = (g2 * tanhv) / (1.0F + gvt * dtanhv);
		}
		m_v += v_step;
		stream.put(0, i, m_v);
	}

	if (fabsf(m_v - last_v) < 1E-15 || fabsf(conv_v - m_v) < 1E-6)
		m_converged = true;

	LOGMASKED(LOG_CONVERGENCE, "%s: converged %d, target: %e %e, deltas %e %e, step: %e %e, current: %e %d\n",
			  tag(), m_converged, m_target_v, conv_v, conv_v - m_v, m_v - last_v, v_step, g2, m_v, n);
}


DEFINE_DEVICE_TYPE(VA_RC_EG, va_rc_eg_device, "va_rc_eg", "RC-based Envelope Generator")
DEFINE_DEVICE_TYPE(VA_OTA_EG, va_ota_eg_device, "va_ota_eg", "OTA-based Envelope Generator")
