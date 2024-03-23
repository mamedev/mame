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

z84c015_device::z84c015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: tmpz84c015_device(mconfig, Z84C015, tag, owner, clock, address_map_constructor(FUNC(z84c015_device::internal_io_map), this))
	, m_program_space_config("program", ENDIANNESS_LITTLE, 8, 18, 0, 16, 0)
	, m_opcodes_space_config("opcodes", ENDIANNESS_LITTLE, 8, 18, 0, 16, 0)
{
}

device_memory_interface::space_config_vector z84c015_device::memory_space_config() const
{
	auto r = z80_device::memory_space_config();
	for (auto it = r.begin(); it != r.end(); ++it)
	{
		if ((*it).first == AS_IO)
			(*it).second = &m_io_space_config;
		else if ((*it).first == AS_OPCODES)
			(*it).second = &m_opcodes_space_config;
		else if ((*it).first == AS_PROGRAM)
			(*it).second = &m_program_space_config;
	}

	return r;
}

bool z84c015_device::memory_translate(int spacenum, int intention, offs_t &address, address_space *&target_space)
{
	if (spacenum == AS_PROGRAM || spacenum == AS_OPCODES)
		address = translate_memory_address(address);

	target_space = &space(spacenum);
	return true;
}

u32 z84c015_device::translate_memory_address(u16 addr)
{
	const u8 csbr = csbr_r();
	const u8 at = BIT(addr, 12, 4);
	return ((BIT(m_mcr, 0) && ((csbr & 0x0f) >= at)) // cs0
		? 0x10000
		: (BIT(m_mcr, 1) && ((csbr >> 4) >= at) && (at > (csbr & 0x0f))) // cs1
			? 0x20000 : 0) | addr;
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

	state_add_divider(-1);
	state_add(Z84_WCR,  "WCR",   m_wcr);
	state_add(Z84_MWBR, "MWBR",  m_mwbr);
	state_add(Z84_CSBR, "CSBR",  m_csbr);
	state_add(Z84_MCR,  "MCR",   m_mcr);
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
