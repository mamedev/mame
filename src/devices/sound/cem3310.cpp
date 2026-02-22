// license:BSD-3-Clause
// copyright-holders:m1macrophage

#include "emu.h"
#include "cem3310.h"
#include "machine/rescap.h"

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

DEFINE_DEVICE_TYPE(CEM3310, cem3310_device, "cem3310", "CEM3310 Envelope Generator")
