// license:BSD-3-Clause
/***************************************************************************

    Zilog Z84C015, MPUZ80/Z8400/84C00 Family
    Z80 CPU, SIO, CTC, CGC, PIO, WDT

***************************************************************************/

#include "emu.h"
#include "z84c015.h"

DEFINE_DEVICE_TYPE(Z84C015, z84c015_device, "z84c015", "Zilog Z84C015")

void z84c015_device::internal_io_map(address_map &map)
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
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void z84c015_device::device_reset()
{
	scrp_w(0);

	m_wcr = 0x00; // 0xff, then 0x00 on 16th M1
    m_mwbr = 0xf0;
	m_csbr |= 0x0f;
	m_mcr = 0x01;

	tmpz84c015_device::device_reset();
}

int z84c015_device::cs0_r(u16 addr)
{
	int cs0 = BIT(~m_mcr, 0);
	if (!cs0)
	{
		const u8 at = BIT(addr, 12, 4);
		cs0 = ((m_csbr & 0x0f) >= at) && (at >= 0);
    }

    return cs0;
}

int z84c015_device::cs1_r(u16 addr)
{
    int cs1 = BIT(~m_mcr, 1);
    if (!cs1)
    {
		const u8 at = BIT(addr, 12, 4);
		cs1 = ((m_csbr >> 4) >= at) && (at > (m_csbr & 0x0f));
    }

    return cs1;
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
