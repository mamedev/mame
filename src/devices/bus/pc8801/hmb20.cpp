// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

    HAL HMB-20

    Used by NRTDRV88 for PC-8801

    TODO:
    - Not extensively tested beyond on-disk songs, which in turn barely has an
      OPM-driven "TESTTONE.NRD";
    - Implement HIBIKI-8800 (same as this plus extra YM3802-X MIDI controller?)

**************************************************************************************************/

#include "emu.h"
#include "hmb20.h"


DEFINE_DEVICE_TYPE(HMB20, hmb20_device, "hmb20", "HAL HMB-20")

hmb20_device::hmb20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc8801_exp_device(mconfig, HMB20, tag, owner, clock)
	, m_opm(*this, "opm")
{
}

void hmb20_device::io_map(address_map &map)
{
	map(0x88, 0x89).rw(m_opm, FUNC(ym2151_device::read), FUNC(ym2151_device::write));
}

void hmb20_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL hmb20_x1_clock = XTAL(4'000'000);

	// TODO: OPM mixing
	YM2151(config, m_opm, hmb20_x1_clock);
//  m_opm->irq_handler().set(FUNC(hmb20_device::int4_w));
	m_opm->add_route(ALL_OUTPUTS, "^^speaker", 0.50, 0);
	m_opm->add_route(ALL_OUTPUTS, "^^speaker", 0.50, 1);
}
