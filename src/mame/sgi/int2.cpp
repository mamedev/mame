// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics Local I/O Interrupt Multiplexor (INT2).
 *
 * TODO:
 *  - bus error
 *  - fpu interrupt
 */

/*
 * #  Source
 * 0  floating-point unit
 * 1  local I/O 0
 * 2  local I/O 1
 * 3  timer 0
 * 4  timer 1
 * 5  bus error
 */

#include "emu.h"
#include "int2.h"

//#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(SGI_INT2, sgi_int2_device, "sgi_int2", "SGI INT2")

sgi_int2_device::sgi_int2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_INT2, tag, owner, clock)
	, m_pit(*this, "pit")
	, m_vme(*this, "vme%u_int", 0U)
	, m_led(*this)
	, m_poweroff(*this)
	, m_intr(*this)
	, m_intr_state{}
{
}

void sgi_int2_device::device_add_mconfig(machine_config &config)
{
	PIT8254(config, m_pit);
	m_pit->set_clk<2>(clock() / 10);
	m_pit->out_handler<0>().set([this](int state) { if (state) { m_intr_state[3] = true; m_intr[3](m_intr_state[3]); } });
	m_pit->out_handler<1>().set([this](int state) { if (state) { m_intr_state[4] = true; m_intr[4](m_intr_state[4]); } });
	m_pit->out_handler<2>().append(m_pit, FUNC(pit8254_device::write_clk0));
	m_pit->out_handler<2>().append(m_pit, FUNC(pit8254_device::write_clk1));

	INPUT_MERGER_ANY_HIGH(config, m_vme[0]);
	m_vme[0]->output_handler().set(FUNC(sgi_int2_device::lio0_w<LIO0_VME0>));

	INPUT_MERGER_ANY_HIGH(config, m_vme[1]);
	m_vme[1]->output_handler().set(FUNC(sgi_int2_device::lio1_w<LIO1_VME1>));
}

void sgi_int2_device::device_start()
{
	save_item(NAME(m_intr_state));
	save_item(NAME(m_lstatus));
	save_item(NAME(m_lmask));
	save_item(NAME(m_vstatus));
	save_item(NAME(m_vmask));
	save_item(NAME(m_config));

	m_lstatus[0] = 0;
	m_lstatus[1] = 0;
	m_vstatus = 0;
	m_config = 0;
}

void sgi_int2_device::device_reset()
{
	config_w(0);

	m_lmask[0] = 0;
	m_lmask[1] = 0;
	m_vmask[0] = 0;
	m_vmask[1] = 0;

	lio_update();
}

void sgi_int2_device::map(address_map &map)
{
	map(0x0, 0x0).r(FUNC(sgi_int2_device::lstatus_r<0>));
	map(0x1, 0x1).rw(FUNC(sgi_int2_device::lmask_r<0>), FUNC(sgi_int2_device::lmask_w<0>));
	map(0x2, 0x2).r(FUNC(sgi_int2_device::lstatus_r<1>));
	map(0x3, 0x3).rw(FUNC(sgi_int2_device::lmask_r<1>), FUNC(sgi_int2_device::lmask_w<1>));
	map(0x4, 0x4).r(FUNC(sgi_int2_device::vstatus_r));
	map(0x5, 0x5).rw(FUNC(sgi_int2_device::vmask_r<0>), FUNC(sgi_int2_device::vmask_w<0>));
	map(0x6, 0x6).rw(FUNC(sgi_int2_device::vmask_r<1>), FUNC(sgi_int2_device::vmask_w<1>));
	map(0x7, 0x7).rw(FUNC(sgi_int2_device::config_r), FUNC(sgi_int2_device::config_w));
	map(0x8, 0x8).w(FUNC(sgi_int2_device::tclear_w));

	map(0xc, 0xf).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
}

template <unsigned N> void sgi_int2_device::lmask_w(u8 data)
{
	m_lmask[N] = data;

	lio_update();
}

template <unsigned N> void sgi_int2_device::vmask_w(u8 data)
{
	m_vmask[N] = data;

	lio_update();
}

void sgi_int2_device::config_w(u8 data)
{
	for (unsigned led = 0; led < 4; led++)
		if (BIT(data ^ m_config, led))
			m_led[led](BIT(data, led));

	if (BIT(data ^ m_config, 4))
		m_poweroff(BIT(data, 4));

	m_config = data & 0x1f;
}

void sgi_int2_device::tclear_w(u8 data)
{
	// clear timer 0 level 3
	if (BIT(data, 0) && m_intr_state[3])
	{
		m_intr_state[3] = false;
		m_intr[3](m_intr_state[3]);
	}

	// clear timer 1 level 4
	if (BIT(data, 1) && m_intr_state[4])
	{
		m_intr_state[4] = false;
		m_intr[4](m_intr_state[4]);
	}
}

template <unsigned N> void sgi_int2_device::lio0_w(int state)
{
	LOG("lio0 interrupt %u input state %d\n", N, state);
	if (state)
		m_lstatus[0] |= 1U << N;
	else
		m_lstatus[0] &= ~(1U << N);

	lio_update();
}

template <unsigned N> void sgi_int2_device::lio1_w(int state)
{
	LOG("lio1 interrupt %u input state %d\n", N, state);
	if (state)
		m_lstatus[1] |= 1U << N;
	else
		m_lstatus[1] &= ~(1U << N);

	lio_update();
}

template <unsigned N> void sgi_int2_device::vme_w(int state)
{
	LOG("vme interrupt %u input state %d\n", N, state);
	if (state)
		m_vstatus |= 1U << N;
	else
		m_vstatus &= ~(1U << N);

	m_vme[0]->in_w<N>(state);
	m_vme[1]->in_w<N>(state);
}

void sgi_int2_device::lio_update()
{
	for (unsigned l = 0; l < 2; l++)
	{
		if (bool(m_lstatus[l] & m_lmask[l]) != m_intr_state[l + 1])
		{
			LOG("lio%d output state %d\n", l, bool(m_lstatus[l] & m_lmask[l]));
			m_intr_state[l + 1] = m_lstatus[l] & m_lmask[l];
			m_intr[l + 1](m_intr_state[l + 1]);
		}
	}
}

template void sgi_int2_device::lio0_w<0>(int state);
template void sgi_int2_device::lio0_w<1>(int state);
template void sgi_int2_device::lio0_w<2>(int state);
template void sgi_int2_device::lio0_w<3>(int state);
template void sgi_int2_device::lio0_w<4>(int state);
template void sgi_int2_device::lio0_w<5>(int state);
template void sgi_int2_device::lio0_w<6>(int state);
template void sgi_int2_device::lio0_w<7>(int state);

template void sgi_int2_device::lio1_w<0>(int state);
template void sgi_int2_device::lio1_w<1>(int state);
template void sgi_int2_device::lio1_w<2>(int state);
template void sgi_int2_device::lio1_w<3>(int state);
template void sgi_int2_device::lio1_w<4>(int state);
template void sgi_int2_device::lio1_w<5>(int state);
template void sgi_int2_device::lio1_w<6>(int state);
template void sgi_int2_device::lio1_w<7>(int state);

template void sgi_int2_device::vme_w<1>(int state);
template void sgi_int2_device::vme_w<2>(int state);
template void sgi_int2_device::vme_w<3>(int state);
template void sgi_int2_device::vme_w<4>(int state);
template void sgi_int2_device::vme_w<5>(int state);
template void sgi_int2_device::vme_w<6>(int state);
template void sgi_int2_device::vme_w<7>(int state);
