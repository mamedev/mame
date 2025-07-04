// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    JMB-X1 sound card

    Used for PC-8001mkII/PC-8801 NRTDRV88J sound program
    http://upd780c1.g1.xrea.com/pc-8001/j80-25.html

    TODO:
    - Source claims it runs on INT3, but the vector that uses NRTDRV is clearly INT4.
      Is it configurable?

**************************************************************************************************/

#include "emu.h"
#include "jmbx1.h"


DEFINE_DEVICE_TYPE(JMBX1, jmbx1_device, "jmbx1", "JMB-X1 \"Sound Board X\"")

jmbx1_device::jmbx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc8801_exp_device(mconfig, JMBX1, tag, owner, clock)
	, m_opm1(*this, "opm1")
	, m_opm2(*this, "opm2")
	, m_ssg(*this, "ssg")
{
}

void jmbx1_device::io_map(address_map &map)
{
	map(0xc8, 0xc9).rw(m_opm1, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xca, 0xcb).rw(m_opm2, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xcc, 0xcd).w(m_ssg, FUNC(ym2149_device::address_data_w));
	map(0xcd, 0xcd).r(m_ssg, FUNC(ym2149_device::data_r));
}

void jmbx1_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL jmb_x1_clock = XTAL(8'000'000);

	// TODO: OPM mixing
	// (SSG is mono)
	// doesn't seem to have irq mask
	YM2151(config, m_opm1, jmb_x1_clock / 2);
	m_opm1->irq_handler().set(FUNC(jmbx1_device::int4_w));
	m_opm1->add_route(ALL_OUTPUTS, "^^speaker", 0.25, 0);
	m_opm1->add_route(ALL_OUTPUTS, "^^speaker", 0.25, 1);

	YM2151(config, m_opm2, jmb_x1_clock / 2);
	m_opm2->add_route(ALL_OUTPUTS, "^^speaker", 0.25, 0);
	m_opm2->add_route(ALL_OUTPUTS, "^^speaker", 0.25, 1);

	YM2149(config, m_ssg, jmb_x1_clock / 4);
	// TODO: adds a non-negligible DC offset, likely needs high pass filter
	m_ssg->add_route(ALL_OUTPUTS, "^^speaker", 0.20, 0);
	m_ssg->add_route(ALL_OUTPUTS, "^^speaker", 0.20, 1);
}
