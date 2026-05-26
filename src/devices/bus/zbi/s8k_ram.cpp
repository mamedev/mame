// license:BSD-3-Clause
// copyright-holders:A. Lenard
/**********************************************************************

    S8000 ECC Controller (+ Memory Array)
    S8000 Parity Memory

NOTE: Due to limitations, the configurable RAM device needs to be added to
the root driver device. It is only referenced here using finders.

TODO: Actually perform the parity & ECC checks

**********************************************************************/

#include "emu.h"
#include "s8k_ram.h"

zbi_s8k_ecc_ram_card_device::zbi_s8k_ecc_ram_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZBI_S8K_ECC_RAM, tag, owner, clock)
	, device_zbi_card_interface(mconfig, *this)
	, m_ram(*this, ":" RAM_TAG)
{
}

uint16_t zbi_s8k_ecc_ram_card_device::regs_r(offs_t offset)
{
	logerror("%s: ECC REGISTER READ @ %04x\n", machine().describe_context(), offset);
	return offset == 1 ? m_reg2 : m_reg1;
}

void zbi_s8k_ecc_ram_card_device::regs_w(offs_t offset, uint16_t data)
{
	logerror("%s: ECC REGISTER WRITTE @ %04x = %04x\n", machine().describe_context(), offset, data);
	if (offset == 1)
		m_reg2 = data;
	else
		m_reg1 = data;
}

uint8_t zbi_s8k_ecc_ram_card_device::card_ram8_r(offs_t offset)
{
	return (offset < m_ram->size()) ? *(m_ram->pointer() + offset) : 0;
}

void zbi_s8k_ecc_ram_card_device::card_ram8_w(offs_t offset, uint8_t data)
{
	if (offset < m_ram->size())
		m_ram->pointer()[offset] = data;
}

uint16_t zbi_s8k_ecc_ram_card_device::card_ram16_r(offs_t offset, uint16_t mask)
{
	if (offset < m_ram->size())
	{
		const uint8_t *memptr = m_ram->pointer() + offset;

		return (mask & big_endianize_int16(reinterpret_cast<const uint16_t*>(memptr)[0]));
	}
	else
		return 0;
}

void zbi_s8k_ecc_ram_card_device::card_ram16_w(offs_t offset, uint16_t data, uint16_t mask)
{
	if (offset < m_ram->size())
	{
		uint8_t *memptr = m_ram->pointer() + offset;

		if (mask & 0xff00)
			memptr[0] = (uint8_t)(data >> 8);
		if (mask & 0x00ff)
			memptr[1] = (uint8_t)(data);
	}
}

uint32_t zbi_s8k_ecc_ram_card_device::card_ram32_r(offs_t offset, uint32_t mask)
{
	if (offset < m_ram->size())
	{
		const uint8_t *memptr = m_ram->pointer() + offset;

		return (mask & big_endianize_int32(reinterpret_cast<const uint32_t*>(memptr)[0]));
	}
	else
		return 0;
}

void zbi_s8k_ecc_ram_card_device::card_ram32_w(offs_t offset, uint32_t data, uint32_t mask)
{
	if (offset < m_ram->size())
	{
		uint8_t *memptr = m_ram->pointer() + offset;

		if (mask & 0xff000000)
			memptr[0] = (uint8_t)(data >> 24);
		if (mask & 0x00ff0000)
			memptr[1] = (uint8_t)(data >> 16);
		if (mask & 0x0000ff00)
			memptr[2] = (uint8_t)(data >> 8);
		if (mask & 0x000000ff)
			memptr[3] = (uint8_t)(data);
	}
}

void zbi_s8k_ecc_ram_card_device::device_start()
{
	if (!m_ram.lookup() || !m_ram->started())
		throw device_missing_dependencies();

	save_item(NAME(m_reg1));
	save_item(NAME(m_reg2));
}

void zbi_s8k_ecc_ram_card_device::device_reset()
{
	m_bus->iospace()->install_readwrite_handler(0x0000, 0x0003,
		read16sm_delegate(*this, FUNC(zbi_s8k_ecc_ram_card_device::regs_r)),
		write16sm_delegate(*this, FUNC(zbi_s8k_ecc_ram_card_device::regs_w)));
}


zbi_s8k_parity_ram_card_device::zbi_s8k_parity_ram_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ZBI_S8K_PARITY_RAM, tag, owner, clock)
	, device_zbi_card_interface(mconfig, *this)
	, m_ram(*this, ":" RAM_TAG)
{
}

//-------------------------------------------------
//  calculate the byte parity
//-------------------------------------------------

bool zbi_s8k_parity_ram_card_device::check_parity(offs_t offset, uint8_t data, bool write)
{
	uint8_t &checkbyte = m_checkbits[offset / 8];
	int bit = offset % 8;
	uint8_t x = data;
	bool parity;

    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
	parity = (x & 1);

	if (write)
	{
		if (BIT(checkbyte, bit) ^ parity)
			checkbyte ^= 1 << bit;
	}
	else if (BIT(checkbyte, bit) != parity)
	{
		card_memerr_w(ASSERT_LINE);
		return false;
	}

	return true;
}

uint8_t zbi_s8k_parity_ram_card_device::card_ram8_r(offs_t offset)
{
	return (offset < m_ram->size()) ? m_ram->pointer()[offset] : 0;
}

void zbi_s8k_parity_ram_card_device::card_ram8_w(offs_t offset, uint8_t data)
{
	if (offset < m_ram->size())
		m_ram->pointer()[offset] = data;
}

uint16_t zbi_s8k_parity_ram_card_device::card_ram16_r(offs_t offset, uint16_t mask)
{
	if (offset < m_ram->size())
	{
		const uint8_t *memptr = m_ram->pointer() + offset;

		return (mask & big_endianize_int16(reinterpret_cast<const uint16_t*>(memptr)[0]));
	}
	else
		return 0;
}

void zbi_s8k_parity_ram_card_device::card_ram16_w(offs_t offset, uint16_t data, uint16_t mask)
{
	if (offset < m_ram->size())
	{
		uint8_t *memptr = m_ram->pointer() + offset;

		if (mask & 0xff00)
			memptr[0] = (uint8_t)(data >> 8);
		if (mask & 0x00ff)
			memptr[1] = (uint8_t)(data);
	}
}

uint32_t zbi_s8k_parity_ram_card_device::card_ram32_r(offs_t offset, uint32_t mask)
{
	if (offset < m_ram->size())
	{
		const uint8_t *memptr = m_ram->pointer() + offset;

		return (mask & big_endianize_int32(reinterpret_cast<const uint32_t*>(memptr)[0]));
	}
	else
		return 0;
}

void zbi_s8k_parity_ram_card_device::card_ram32_w(offs_t offset, uint32_t data, uint32_t mask)
{
	if (offset < m_ram->size())
	{
		uint8_t *memptr = m_ram->pointer() + offset;

		if (mask & 0xff000000)
			memptr[0] = (uint8_t)(data >> 24);
		if (mask & 0x00ff0000)
			memptr[1] = (uint8_t)(data >> 16);
		if (mask & 0x0000ff00)
			memptr[2] = (uint8_t)(data >> 8);
		if (mask & 0x000000ff)
			memptr[3] = (uint8_t)(data);
	}
}

void zbi_s8k_parity_ram_card_device::device_start()
{
	if (!m_ram.lookup() || !m_ram->started())
		throw device_missing_dependencies();

	m_checksize = m_ram->size() / 8;
	m_checkbits = std::make_unique<uint8_t[]>(m_checksize);

	save_pointer(NAME(m_checkbits), m_checksize);
}

void zbi_s8k_parity_ram_card_device::device_reset()
{
	memset(&m_checkbits[0], 0, m_checksize);
}


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ZBI_S8K_ECC_RAM, zbi_s8k_ecc_ram_card_device, "s8k_ecc_ram", "S8000 ECC Controller")
DEFINE_DEVICE_TYPE(ZBI_S8K_PARITY_RAM, zbi_s8k_parity_ram_card_device, "s8k_parity_ram", "S8000 Parity Memory")
