// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/***************************************************************************

  Nintendo 8080 sound emulation

***************************************************************************/

#include "emu.h"
#include "n8080_a.h"

#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/sn76477.h"

#include "speaker.h"

#include <cmath>


namespace {

template <unsigned Monostables>
class n8080_csg_sound_device_base : public n8080_sound_device_base
{
protected:
	n8080_csg_sound_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *parent,
			u32 clock) :
		n8080_sound_device_base(mconfig, type, tag, parent, clock),
		m_dac(*this, "dac"),
		m_sn(*this, "snsnd")
	{
	}

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	bool mono_flop(unsigned n) const { return m_mono_flop[n]; }
	attotime mono_elapsed(unsigned n) { return m_mono_timer[n]->elapsed(); }
	void start_mono_flop(unsigned n, attotime const &expiry);
	void stop_mono_flop(unsigned n);

	required_device<dac_bit_interface> m_dac;
	required_device<sn76477_device> m_sn;

private:
	u8 p1_r()
	{
		return bitswap<8>(current_pins(), 1, 2, 3, 5, 8, 9, 10, 11);
	}

	int t0_r()
	{
		return BIT(current_pins(), 7);
	}

	int t1_r()
	{
		return BIT(current_pins(), 12);
	}

	void dac_w(u8 data)
	{
		m_dac->write(BIT(data, 7));
	}

	TIMER_CALLBACK_MEMBER(stop_mono_flop_callback)
	{
		stop_mono_flop(param);
	}

	virtual void update_sn_status() = 0;

	emu_timer *m_mono_timer[Monostables];
	bool m_mono_flop[Monostables];
};


template <unsigned Monostables>
void n8080_csg_sound_device_base<Monostables>::device_add_mconfig(machine_config &config)
{
	n8080_sound_device_base::device_add_mconfig(config);

	m_cpu->t0_in_cb().set(FUNC(n8080_csg_sound_device_base::t0_r));
	m_cpu->t1_in_cb().set(FUNC(n8080_csg_sound_device_base::t1_r));
	m_cpu->p1_in_cb().set(FUNC(n8080_csg_sound_device_base::p1_r));
	m_cpu->p2_out_cb().set(FUNC(n8080_csg_sound_device_base::dac_w));

	SPEAKER(config, "speaker").front_center();

	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.15);

	SN76477(config, m_sn);
	m_sn->set_attack_params(CAP_U(1.0), RES_K(20));
	m_sn->set_amp_res(RES_K(150));
	m_sn->set_feedback_res(RES_K(47));
	m_sn->set_vco_params(0, CAP_N(1), RES_M(1.5));
	m_sn->set_pitch_voltage(0);
	m_sn->set_vco_mode(0);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->set_enable(1);
	m_sn->add_route(ALL_OUTPUTS, "speaker", 0.35);
}

template <unsigned Monostables>
void n8080_csg_sound_device_base<Monostables>::device_start()
{
	n8080_sound_device_base::device_start();

	for (unsigned i = 0; Monostables > i; ++i)
		m_mono_timer[i] = timer_alloc(FUNC(n8080_csg_sound_device_base::stop_mono_flop_callback), this);

	save_item(NAME(m_mono_flop));
}

template <unsigned Monostables>
void n8080_csg_sound_device_base<Monostables>::device_reset()
{
	n8080_sound_device_base::device_reset();

	for (unsigned i = 0; Monostables > i; ++i)
		m_mono_flop[i] = false;
}


template <unsigned Monostables>
void n8080_csg_sound_device_base<Monostables>::start_mono_flop(unsigned n, attotime const &expiry)
{
	m_mono_flop[n] = true;

	update_sn_status();

	m_mono_timer[n]->adjust(expiry, n);
}

template <unsigned Monostables>
void n8080_csg_sound_device_base<Monostables>::stop_mono_flop(unsigned n)
{
	m_mono_flop[n] = false;

	update_sn_status();

	m_mono_timer[n]->adjust(attotime::never, n);
}



class spacefev_sound_device : public n8080_csg_sound_device_base<3>
{
public:
	spacefev_sound_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *parent,
			u32 clock) :
		n8080_csg_sound_device_base<3>(mconfig, SPACEFEV_SOUND, tag, parent, clock)
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	virtual void update_sn_status() override;
	virtual void pins_changed(u16 curr, u16 prev) override;

	TIMER_DEVICE_CALLBACK_MEMBER(vco_voltage_timer);
};


void spacefev_sound_device::device_add_mconfig(machine_config &config)
{
	n8080_csg_sound_device_base<3>::device_add_mconfig(config);

	m_sn->set_noise_params(RES_K(36), RES_K(150), CAP_N(1));
	m_sn->set_decay_res(RES_M(1));
	m_sn->set_slf_params(CAP_N(47), RES_M(1));
	m_sn->set_oneshot_params(CAP_N(47), RES_K(820));

	TIMER(config, "vco_timer").configure_periodic(FUNC(spacefev_sound_device::vco_voltage_timer), attotime::from_hz(1000));
}


void spacefev_sound_device::update_sn_status()
{
	double dblR0 = RES_M(1.0);
	if (!mono_flop(0))
		dblR0 = 1.0 / (1.0 / RES_K(150) + 1.0 / dblR0); // ?

	double dblR1 = RES_M(1.5);
	if (!mono_flop(1))
		dblR1 = 1.0 / (1.0 / RES_K(620) + 1.0 / dblR1); // ?

	m_sn->decay_res_w(dblR0);
	m_sn->vco_res_w(dblR1);
	m_sn->enable_w((!mono_flop(0) && !mono_flop(1) && !mono_flop(2)) ? 1 : 0);
	m_sn->vco_w(mono_flop(1) ? 1 : 0);
	m_sn->mixer_b_w(mono_flop(0) ? 1 : 0);
}

void spacefev_sound_device::pins_changed(u16 curr, u16 prev)
{
	u16 const changes = ~curr & prev;

	if (BIT(changes, 3))
		stop_mono_flop(1);

	if (BIT(changes, 3) || BIT(changes, 6))
		stop_mono_flop(2);

	if (BIT(changes, 3))
		start_mono_flop(0, attotime::from_usec(550 * 36 * 100));

	if (BIT(changes, 6))
		start_mono_flop(1, attotime::from_usec(550 * 22 * 33));

	if (BIT(changes, 4))
		start_mono_flop(2, attotime::from_usec(550 * 22 * 33));

	bool const irq_active = BIT(~curr, 2) || BIT(~curr, 3) || BIT(~curr, 5);
	m_cpu->set_input_line(INPUT_LINE_IRQ0, irq_active ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(spacefev_sound_device::vco_voltage_timer)
{
	double voltage = 0;
	if (mono_flop(2))
		voltage = 5.0 * (1.0 - exp(-mono_elapsed(2).as_double() / 0.22));

	m_sn->vco_voltage_w(voltage);
}



class sheriff_sound_device : public n8080_csg_sound_device_base<2>
{
public:
	sheriff_sound_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *parent,
			u32 clock) :
		n8080_csg_sound_device_base<2>(mconfig, SHERIFF_SOUND, tag, parent, clock)
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	virtual void update_sn_status() override;
	virtual void pins_changed(u16 curr, u16 prev) override;
};


void sheriff_sound_device::device_add_mconfig(machine_config &config)
{
	n8080_csg_sound_device_base<2>::device_add_mconfig(config);

	m_sn->set_noise_params(RES_K(36), RES_K(100), CAP_N(1));
	m_sn->set_decay_res(RES_K(620));
	m_sn->set_slf_params(CAP_N(47), RES_M(1.5));
	m_sn->set_oneshot_params(CAP_N(47), RES_K(560));
}


void sheriff_sound_device::update_sn_status()
{
	m_sn->vco_voltage_w(mono_flop(1) ? 5 : 0);
	m_sn->enable_w((!mono_flop(0) && !mono_flop(1)) ? 1 : 0);
	m_sn->vco_w(mono_flop(0) ? 1 : 0);
	m_sn->mixer_b_w(mono_flop(0) ? 0 : 1);
}

void sheriff_sound_device::pins_changed(u16 curr, u16 prev)
{
	u16 const changes = ~curr & prev;

	if (BIT(changes, 6))
		stop_mono_flop(1);

	if (BIT(changes, 6))
		start_mono_flop(0, attotime::from_usec(550 * 33 * 33));

	if (BIT(changes, 4))
		start_mono_flop(1, attotime::from_usec(550 * 33 * 33));

	bool const irq_active = BIT(~curr, 2) || BIT(~curr, 3) || BIT(~curr, 5);
	m_cpu->set_input_line(INPUT_LINE_IRQ0, irq_active ? ASSERT_LINE : CLEAR_LINE);
}



class helifire_sound_device : public n8080_sound_device_base
{
public:
	helifire_sound_device(machine_config const &mconfig,
			char const *tag,
			device_t *parent,
			u32 clock) :
		n8080_sound_device_base(mconfig, HELIFIRE_SOUND, tag, parent, clock),
		m_dac(*this, "dac"),
		m_dac_volume(1.0),
		m_dac_timing(0.0),
		m_dac_phase(false)
	{
	}

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static inline constexpr double ATTACK_RATE = 10e-6 * 500;
	static inline constexpr double DECAY_RATE = 10e-6 * 16'000;

	u8 ram_r()
	{
		return bitswap<4>(current_pins(), 1, 9, 8, 7);
	}

	u8 p2_r()
	{
		return BIT(current_pins(), 12) ? 0x10 : 0x00; // not used
	}

	int t0_r()
	{
		return BIT(current_pins(), 3);
	}

	int t1_r()
	{
		return BIT(current_pins(), 4);
	}

	void ctrl_w(u8 data);

	virtual void pins_changed(u16 curr, u16 prev) override;

	TIMER_DEVICE_CALLBACK_MEMBER(volume_timer);

	void io_map(address_map &map) ATTR_COLD;

	required_device<dac_8bit_r2r_device> m_dac;

	double m_dac_volume;
	double m_dac_timing;
	bool m_dac_phase;
};


void helifire_sound_device::device_add_mconfig(machine_config &config)
{
	n8080_sound_device_base::device_add_mconfig(config);

	m_cpu->set_clock(5_MHz_XTAL);
	m_cpu->set_addrmap(AS_IO, &helifire_sound_device::io_map);
	m_cpu->t0_in_cb().set(FUNC(helifire_sound_device::t0_r));
	m_cpu->t1_in_cb().set(FUNC(helifire_sound_device::t1_r));
	m_cpu->p2_in_cb().set(FUNC(helifire_sound_device::p2_r));
	m_cpu->p1_out_cb().set(m_dac, FUNC(dac_byte_interface::data_w));
	m_cpu->p2_out_cb().set(FUNC(helifire_sound_device::ctrl_w));

	TIMER(config, "volume_timer").configure_periodic(FUNC(helifire_sound_device::volume_timer), attotime::from_hz(1000));

	SPEAKER(config, "speaker").front_center();

	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.15); // unknown DAC
}

void helifire_sound_device::device_start()
{
	n8080_sound_device_base::device_start();

	save_item(NAME(m_dac_volume));
	save_item(NAME(m_dac_timing));
	save_item(NAME(m_dac_phase));
}

void helifire_sound_device::device_reset()
{
	n8080_sound_device_base::device_reset();

	m_dac_volume = 1.0;
	m_dac_timing = 0.0;
	m_dac_phase = false;
}


void helifire_sound_device::ctrl_w(u8 data)
{
	m_dac_phase = BIT(data, 7);

	// data bit 6 not emulated
	// data bit 5 not emulated

	m_dac_timing = machine().time().as_double();
	if (m_dac_phase)
		m_dac_timing += ATTACK_RATE * log(1 - m_dac_volume);
	else
		m_dac_timing += DECAY_RATE * log(m_dac_volume);
}


void helifire_sound_device::pins_changed(u16 curr, u16 prev)
{
	// bit 10 not emulated
	// bit 11 not emulated
	// bit 12 not emulated

	bool const irq_active = BIT(~curr, 6);
	m_cpu->set_input_line(INPUT_LINE_IRQ0, irq_active ? ASSERT_LINE : CLEAR_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(helifire_sound_device::volume_timer)
{
	double const t = m_dac_timing - machine().time().as_double();

	if (m_dac_phase)
		m_dac_volume = 1.0 - exp(t / ATTACK_RATE);
	else
		m_dac_volume = exp(t / DECAY_RATE);

	m_dac->set_output_gain(ALL_OUTPUTS, m_dac_volume);
}


void helifire_sound_device::io_map(address_map &map)
{
	map(0x00, 0x00).mirror(0x7f).r(FUNC(helifire_sound_device::ram_r));
}

} // anonymous namespace



void n8080_sound_device_base::sound1_w(u8 data)
{
	u16 const prev = m_curr_pins;

	m_curr_pins &= ~((1 << 7) | (1 << 5) | (1 << 6) | (1 << 3) | (1 << 4) | (1 << 1));

	if (BIT(~data, 0)) m_curr_pins |= 1 << 7;
	if (BIT(~data, 1)) m_curr_pins |= 1 << 5; // pulse
	if (BIT(~data, 2)) m_curr_pins |= 1 << 6; // pulse
	if (BIT(~data, 3)) m_curr_pins |= 1 << 3; // pulse (except in HeliFire)
	if (BIT(~data, 4)) m_curr_pins |= 1 << 4; // pulse (except in HeliFire)
	if (BIT(~data, 5)) m_curr_pins |= 1 << 1;

	pins_changed(m_curr_pins, prev);
}

void n8080_sound_device_base::sound2_w(u8 data)
{
	u16 const prev = m_curr_pins;

	m_curr_pins &= ~((1 << 8) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 2) | (1 << 12));

	if (BIT(~data, 0)) m_curr_pins |= 1 << 8;
	if (BIT(~data, 1)) m_curr_pins |= 1 << 9;
	if (BIT(~data, 2)) m_curr_pins |= 1 << 10;
	if (BIT(~data, 3)) m_curr_pins |= 1 << 11;
	if (BIT(~data, 4)) m_curr_pins |= 1 << 2; // pulse
	if (BIT(~data, 5)) m_curr_pins |= 1 << 12;

	pins_changed(m_curr_pins, prev);
}


n8080_sound_device_base::n8080_sound_device_base(
		machine_config const &mconfig,
		device_type type,
		char const *tag,
		device_t *parent,
		u32 clock) :
	device_t(mconfig, type, tag, parent, clock),
	m_cpu(*this, "cpu"),
	m_curr_pins(0)
{
}


void n8080_sound_device_base::device_add_mconfig(machine_config &config)
{
	I8035(config, m_cpu, 6_MHz_XTAL);
	m_cpu->set_addrmap(AS_PROGRAM, &n8080_sound_device_base::prg_map);
}

void n8080_sound_device_base::device_start()
{
	save_item(NAME(m_curr_pins));
}


void n8080_sound_device_base::prg_map(address_map &map)
{
	map.global_mask(0x3ff);
	map(0x0000, 0x03ff).rom();
}



template class device_finder<n8080_sound_device_base, false>;
template class device_finder<n8080_sound_device_base, true>;



DEFINE_DEVICE_TYPE_PRIVATE(SPACEFEV_SOUND, n8080_sound_device_base, spacefev_sound_device, "spacefev_sound", "Nintendo Space Fever Sound Board")
DEFINE_DEVICE_TYPE_PRIVATE(SHERIFF_SOUND,  n8080_sound_device_base, sheriff_sound_device,  "sheriff_sound",  "Nintendo Sheriff Sound Board")
DEFINE_DEVICE_TYPE_PRIVATE(HELIFIRE_SOUND, n8080_sound_device_base, helifire_sound_device, "helifire_sound", "Nintendo HeliFire Sound Board")
