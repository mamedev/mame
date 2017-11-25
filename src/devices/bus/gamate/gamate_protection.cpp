// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "gamate_protection.h"

//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(GAMATE_PROT, gamate_protection_device, "gamate_prot", "Gamate Protection Mapper")

gamate_protection_device::gamate_protection_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GAMATE_PROT, tag, owner, clock),
	m_is_protection_passed(0),
	m_has_failed(0),
	m_passed_write(0),
	m_inpos(0),
	m_inbyte(0),
	m_inseq(0)
{
}

void gamate_protection_device::device_start()
{
	save_item(NAME(m_is_protection_passed));
	save_item(NAME(m_has_failed));
	save_item(NAME(m_passed_write));
	save_item(NAME(m_inpos));
	save_item(NAME(m_inbyte));
	save_item(NAME(m_inseq));
}

void gamate_protection_device::device_reset()
{
	m_is_protection_passed = 0;
	m_has_failed = 0;
	m_passed_write = 0;
	m_inpos = 0;
	m_inbyte = 0;
	m_inseq = 0;
}

bool gamate_protection_device::is_protection_passed()
{
	return m_is_protection_passed;
}

WRITE_LINE_MEMBER(gamate_protection_device::prot_w)
{
	LOG("write to protection %01x\n", state);

	if (m_inpos < 8)
	{
		m_inbyte |= state << (7 - m_inpos);
		m_inpos++;
	}
	else
	{
		LOG("byte in was %c\n", m_inbyte);

		if (!m_has_failed)
		{
			if (m_inseq < 15)
			{
				if (m_inbyte == m_prot_string[m_inseq])
				{
					LOG("OK\n");
				}
				else
				{
					m_has_failed = 1;
				}
			}
		}

		m_inpos = 0;
		m_inbyte = 0;

		m_inseq++;

		if (!m_has_failed && m_inseq == 15)
		{
			m_inbyte = 0x47;
			LOG("setting byte to output\n");
			m_passed_write = 1;
		}
	}
}

READ_LINE_MEMBER(gamate_protection_device::prot_r)
{

	if (m_passed_write)
	{
		int retval = (m_inbyte >> (7 - m_inpos)) & 1;
		m_inpos++;

		LOG("read from protection %01x\n", retval);

		if (m_inpos == 8)
		{
			LOG("unlocking ROM\n");
			m_is_protection_passed = 1;
			m_inpos = 0;
			m_inbyte = 0;
		}

		return retval;
	}
	else
	{
		LOG("read from protection when not ready\n");
	}

	return 0x0;
}
