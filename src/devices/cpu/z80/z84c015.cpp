// license:BSD-3-Clause
/***************************************************************************

    Zilog Z84C015, MPUZ80/Z8400/84C00 Family
    Z80 CPU, SIO, CTC, CGC, PIO, WDT

***************************************************************************/

#include "emu.h"
#include "z84c015.h"

DEFINE_DEVICE_TYPE(Z84C015, z84c015_device, "z84c015", "Zilog Z84C015")

void z84c015_device::internal_io_map(address_map &map) const
{
    tmpz84c015_device::internal_io_map(map);
	map(0xee, 0xee).mirror(0xff00).rw(FUNC(z84c015_device::scrp_r), FUNC(z84c015_device::scrp_w));
	map(0xef, 0xef).mirror(0xff00).rw(FUNC(z84c015_device::scdp_r), FUNC(z84c015_device::scdp_w));
}

z84c015_device::z84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	tmpz84c015_device(mconfig, Z84C015, tag, owner, clock, address_map_constructor(FUNC(z84c015_device::internal_io_map), this))
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void z84c015_device::device_start()
{
	tmpz84c015_device::device_start();

	// register for save states
	save_item(NAME(m_scrp));
	save_item(NAME(m_wcr));
	save_item(NAME(m_mwbr));
	save_item(NAME(m_csbr));
	save_item(NAME(m_mcr));

	scrp_w(0);
	m_wcr = 0x00; // 0xff, then 0x00 on 16th M1
	m_mwbr = 0xf0;
	m_csbr = 0xff; // Must be `|= 0x0f` but keep ff for reproducible startup
	m_mcr = 0x01;
}

u8 z84c015_device::scdp_r()
{
	if (m_scrp < 0x04)
	{
		const u8 regs[4] = { m_wcr, m_mwbr, m_csbr, m_mcr };
		return regs[m_scrp];
	}
	else
		return 0xff;
}

void z84c015_device::scdp_w(u8 data)
{
	if (m_scrp == 0x00)
		m_wcr = data;
	else if (m_scrp == 0x01)
		m_mwbr = data;
	else if (m_scrp == 0x02)
		m_csbr = data;
	else if (m_scrp == 0x03)
		m_mcr = data;
}
