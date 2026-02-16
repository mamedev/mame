// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "va_eg.h"
#include "machine/rescap.h"

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


cem3310_device::cem3310_device(const machine_config &mconfig, const char *tag, device_t *owner, float rx, float cx)
	: device_t(mconfig, CEM3310, tag, owner, 0)
	, device_sound_interface(mconfig, *this)
	, m_rx(rx)
	, m_cx(cx)
	, m_stream(nullptr)
	, m_attack_timer(nullptr)
	, m_rc(*this, "rc")
	, m_phase(PHASE_RELEASE)
	, m_gate(false)
	, m_attack_cv(0)
	, m_decay_cv(0)
	, m_sustain_cv(0)
	, m_release_cv(0)
{
}

cem3310_device::cem3310_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cem3310_device(mconfig, tag, owner, RES_K(24), CAP_U(0.039))  // Example values in the datasheet.
{
}

void cem3310_device::attack_w(float cv)
{
	if (cv == m_attack_cv)
		return;
	m_stream->update();
	m_attack_cv = cv;
	update_rc();
}

void cem3310_device::decay_w(float cv)
{
	if (cv == m_decay_cv)
		return;
	m_stream->update();
	m_decay_cv = cv;
	update_rc();
}

void cem3310_device::sustain_w(float cv)
{
	if (cv == m_sustain_cv)
		return;
	m_stream->update();
	m_sustain_cv = cv;
	update_rc();
}

void cem3310_device::release_w(float cv)
{
	if (cv == m_release_cv)
		return;
	m_stream->update();
	m_release_cv = cv;
	update_rc();
}

void cem3310_device::gate_w(int state)
{
	const bool gate = bool(state);
	if (gate == m_gate)
		return;

	m_stream->update();
	if (!m_gate && gate)
		m_phase = PHASE_ATTACK;
	else
		m_phase = PHASE_RELEASE;
	m_gate = gate;
	update_rc();
}

void cem3310_device::device_add_mconfig(machine_config &config)
{
	VA_RC_EG(config, m_rc).set_c(m_cx);
}

void cem3310_device::device_start()
{
	m_stream = stream_alloc(0, 1, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	m_attack_timer = timer_alloc(FUNC(cem3310_device::attack_timer_tick), this);

	save_item(NAME(m_phase));
	save_item(NAME(m_gate));
	save_item(NAME(m_attack_cv));
	save_item(NAME(m_decay_cv));
	save_item(NAME(m_sustain_cv));
	save_item(NAME(m_release_cv));
}

void cem3310_device::device_reset()
{
	update_rc();
}

void cem3310_device::sound_stream_update(sound_stream &stream)
{
	attotime t = stream.start_time();
	if (m_rc->converged(t))
	{
		// Avoid repeated calls to get_v() if the envelope stage has completed.
		stream.fill(0, m_rc->get_v(t));
		return;
	}

	const int n = stream.samples();
	const attotime dt = stream.sample_period();
	for (int i = 0; i < n; ++i, t += dt)
		stream.put(0, i, m_rc->get_v(t));
}

void cem3310_device::update_rc()
{
	float target_v = 0;
	float rate_cv = 0;
	switch (m_phase)
	{
		case PHASE_ATTACK:
			target_v = VZ;
			rate_cv = m_attack_cv;
			break;
		case PHASE_DECAY:
			target_v = m_sustain_cv;
			rate_cv = m_decay_cv;
			break;
		case PHASE_RELEASE:
			target_v = 0;
			rate_cv = m_release_cv;
			break;
		default:
			fatalerror("%s: Unrecognized EG phase\n", tag());
	}

	// According to the datasheet, the equivalent RC constant is:
	//   RC = Rx * Cx * exp(-VC / VT)
	// where VC is the rate CV of the current EG phase, and VT is the thermal voltage.
	// m_rc is configured with Cx as its capacitor. So to achieve the required
	// time constant, we set its resistor to:
	//   R = Rx * exp(-VC / VT)
	m_rc->set_r(m_rx * expf(-rate_cv / VT));
	m_rc->set_target_v(target_v);

	// If in the attack phase, set up a timer to switch to the decay phase once
	// peak voltage is reached.
	m_attack_timer->reset();
	if (m_phase == PHASE_ATTACK)
	{
		const attotime dt = m_rc->get_dt(VP);
		if (!dt.is_never())
		{
			m_attack_timer->adjust(dt);
		}
		else
		{
			// Extremely unlikely. Requires update_rc() getting called at
			// exactly the time when the attack phase ends. Using 'assert' to
			// ensure this gets noticed in debug builds, as it likely points to
			// a bug.
			assert(false);
			logerror("Voltage (%f) greater than 5V during attack phase.\n", m_rc->get_v());
			attack_timer_tick(0);  // Enter decay phase immediately.
		}
	}
}

TIMER_CALLBACK_MEMBER(cem3310_device::attack_timer_tick)
{
	assert(m_phase == PHASE_ATTACK);
	if (m_phase != PHASE_ATTACK)
	{
		logerror("Attack timer elapsed when not in attack phase.\n");
		return;
	}

	m_stream->update();
	m_phase = PHASE_DECAY;
	update_rc();
}


DEFINE_DEVICE_TYPE(VA_RC_EG, va_rc_eg_device, "va_rc_eg", "RC-based Envelope Generator")
DEFINE_DEVICE_TYPE(CEM3310, cem3310_device, "cem3310", "CEM3310 Envelope Generator")
