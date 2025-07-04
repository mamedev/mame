// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Sharp MZ-1R37 640KB EMM for MZ-2500

**************************************************************************************************/


#include "emu.h"
#include "mz1r37.h"

#include "speaker.h"

DEFINE_DEVICE_TYPE(MZ1R37, mz1r37_device, "mz1r37", "Sharp MZ-1R37 EMM")

mz1r37_device::mz1r37_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: mz80_exp_device(mconfig, MZ1R37, tag, owner, clock)
{
}

void mz1r37_device::device_start()
{
	m_emm_ram = make_unique_clear<u8[]>(MAX_EMM_SIZE);
	save_pointer(NAME(m_emm_ram), MAX_EMM_SIZE);
}

void mz1r37_device::io_map(address_map &map)
{
	map(0xac, 0xac).select(0xff00).w(FUNC(mz1r37_device::address_w));
	map(0xad, 0xad).select(0xff00).r(FUNC(mz1r37_device::data_r)).w(FUNC(mz1r37_device::data_w));
}

void mz1r37_device::address_w(offs_t offset, u8 data)
{
	const u8 emm_hi_index = (offset >> 8) & 0xff;

	m_emm_offset = ((emm_hi_index & 0xff) << 16) | ((data & 0xff) << 8) | (m_emm_offset & 0xff);
}

u8 mz1r37_device::data_r(offs_t offset)
{
	const u8 emm_lo_index = (offset >> 8) & 0xff;

	m_emm_offset = (m_emm_offset & 0xffff00) | (emm_lo_index & 0xff);

	if(m_emm_offset < MAX_EMM_SIZE)
		return m_emm_ram[m_emm_offset];

	return 0xff;
}

void mz1r37_device::data_w(offs_t offset, u8 data)
{
	const u8 emm_lo_index = (offset >> 8) & 0xff;

	m_emm_offset = (m_emm_offset & 0xffff00) | (emm_lo_index & 0xff);

	if(m_emm_offset < MAX_EMM_SIZE)
		m_emm_ram[m_emm_offset] = data;
}
