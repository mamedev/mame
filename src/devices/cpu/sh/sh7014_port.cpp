// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 I/O Port

  TODO list (not comprehensive):
  - Special functions specified by control registers are not implemented

***************************************************************************/

#include "emu.h"
#include "sh7014_port.h"

#define LOG_READ (1U << 1)
#define LOG_WRITE (1U << 2)

// #define VERBOSE (LOG_GENERAL | LOG_READ | LOG_WRITE)

#include "logmacro.h"


DEFINE_DEVICE_TYPE(SH7014_PORT, sh7014_port_device, "sh7014port", "SH7014 I/O Port")


sh7014_port_device::sh7014_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SH7014_PORT, tag, owner, clock)
	, m_port_a_read_cb(*this, 0)
	, m_port_a_write_cb(*this)
	, m_port_b_read_cb(*this, 0)
	, m_port_b_write_cb(*this)
	, m_port_e_read_cb(*this, 0)
	, m_port_e_write_cb(*this)
	, m_port_f_read_cb(*this, 0)
{
}

void sh7014_port_device::device_start()
{
	save_item(NAME(m_padr));
	save_item(NAME(m_paior));
	save_item(NAME(m_pacr1));
	save_item(NAME(m_pacr2));

	save_item(NAME(m_pbdr));
	save_item(NAME(m_pbior));
	save_item(NAME(m_pbcr1));
	save_item(NAME(m_pbcr2));

	save_item(NAME(m_pedr));
	save_item(NAME(m_peior));
	save_item(NAME(m_pecr1));
	save_item(NAME(m_pecr2));

	save_item(NAME(m_pfdr));
}

void sh7014_port_device::device_reset()
{
	m_padr = 0;
	m_paior = 0;
	m_pacr1 = m_pacr2 = 0;

	m_pbdr = 0;
	m_pbior = 0;
	m_pbcr1 = m_pbcr2 = 0;

	m_pedr = 0;
	m_peior = 0;
	m_pecr1 = m_pecr2 = 0;

	m_pfdr = 0;
}

///

uint16_t sh7014_port_device::padrl_r()
{
	uint16_t pins = m_port_a_read_cb();
	uint16_t r = ((m_padr & m_paior) | (pins & ~m_paior)) & 0x83ff;
	LOGMASKED(LOG_READ, "padrl_r %04x (pins: %04x %04x)\n", r, m_paior, pins);
	return r;
}

void sh7014_port_device::padrl_w(uint16_t data)
{
	m_padr = data & 0x83ff;
	LOGMASKED(LOG_WRITE, "padrl_w %04x\n", m_padr);

	if (m_paior)
		m_port_a_write_cb(m_padr & m_paior);
}

uint16_t sh7014_port_device::paiorl_r()
{
	LOGMASKED(LOG_READ, "paiorl_r %04x\n", m_paior);
	return m_paior;
}

void sh7014_port_device::paiorl_w(uint16_t data)
{
	m_paior = data & 0x83ff;
	LOGMASKED(LOG_WRITE, "paiorl_w %04x\n", m_paior);
}

uint16_t sh7014_port_device::pacrl1_r()
{
	LOGMASKED(LOG_READ, "m_pacr1 %04x\n", m_pacr1);
	return m_pacr1;
}

void sh7014_port_device::pacrl1_w(uint16_t data)
{
	m_pacr1 = data & 0x400f;
	LOGMASKED(LOG_WRITE, "pacrl1_w %04x\n", m_pacr1);
}

uint16_t sh7014_port_device::pacrl2_r()
{
	LOGMASKED(LOG_READ, "pacrl2_r %04x\n", m_pacr2);
	return m_pacr2;
}

void sh7014_port_device::pacrl2_w(uint16_t data)
{
	m_pacr2 = data & 0xfd75;
	LOGMASKED(LOG_WRITE, "pacrl2_w %04x\n", m_pacr2);
}

uint16_t sh7014_port_device::pbdr_r()
{
	uint16_t pins = m_port_b_read_cb();
	uint16_t r = ((m_pbdr & m_pbior) | (pins & ~m_pbior)) & 0x3ff;
	LOGMASKED(LOG_READ, "pbdr_r %04x (pins: %04x %04x)\n", r, m_pbior, pins);
	return r;
}

void sh7014_port_device::pbdr_w(uint16_t data)
{
	m_pbdr = data;
	LOGMASKED(LOG_WRITE, "pbdr_w %04x\n", m_pbdr);

	if (m_pbior)
		m_port_b_write_cb(m_pbdr & m_pbior);
}

uint16_t sh7014_port_device::pbior_r()
{
	LOGMASKED(LOG_READ, "pbior_r %04x\n", m_pbior);
	return m_pbior;
}

void sh7014_port_device::pbior_w(uint16_t data)
{
	// For SH7014 specifically, bits 0 and 1 are read only
	m_pbior = data & 0x3fc;
	LOGMASKED(LOG_WRITE, "pbior_w %04x\n", m_pbior);
}

uint16_t sh7014_port_device::pbcr1_r()
{
	LOGMASKED(LOG_READ, "pbcr1_r %04x\n", m_pbcr1);
	return m_pbcr1;
}

void sh7014_port_device::pbcr1_w(uint16_t data)
{
	m_pbcr1 = data & 0xf;
	LOGMASKED(LOG_WRITE, "pbcr1_w %04x\n", m_pbcr1);
}

uint16_t sh7014_port_device::pbcr2_r()
{
	LOGMASKED(LOG_READ, "pbcr2_r %04x\n", m_pbcr2);
	return m_pbcr2;
}

void sh7014_port_device::pbcr2_w(uint16_t data)
{
	m_pbcr2 = data & 0xfff5;
	LOGMASKED(LOG_WRITE, "pbcr2_w %04x\n", m_pbcr2);
}

uint16_t sh7014_port_device::pedr_r()
{
	uint16_t pins = m_port_e_read_cb();
	uint16_t r = ((m_pedr & m_peior) | (pins & ~m_peior));
	LOGMASKED(LOG_READ, "pedr_r %04x (pins: %04x %04x)\n", r, m_peior, pins);
	return r;
}

void sh7014_port_device::pedr_w(uint16_t data)
{
	m_pedr = data;
	LOGMASKED(LOG_WRITE, "pedr_w %04x\n", m_pedr);

	if (m_peior)
		m_port_e_write_cb(m_pedr & m_peior);
}

uint16_t sh7014_port_device::peior_r()
{
	LOGMASKED(LOG_READ, "peior_r %04x\n", m_peior);
	return m_peior;
}

void sh7014_port_device::peior_w(uint16_t data)
{
	m_peior = data;
	LOGMASKED(LOG_WRITE, "peior_w %04x\n", m_peior);
}

uint16_t sh7014_port_device::pecr1_r()
{
	LOGMASKED(LOG_READ, "pecr1_r %04x\n", m_pecr1);
	return m_pecr1;
}

void sh7014_port_device::pecr1_w(uint16_t data)
{
	m_pecr1 = data & 0xf000;
	LOGMASKED(LOG_WRITE, "pecr1_w %04x\n", m_pecr1);
}

uint16_t sh7014_port_device::pecr2_r()
{
	LOGMASKED(LOG_READ, "pecr2_r %04x\n", m_pecr2);
	return m_pecr2;
}

void sh7014_port_device::pecr2_w(uint16_t data)
{
	m_pecr2 = data & 0x55ff;
	LOGMASKED(LOG_WRITE, "pecr2_w %04x\n", m_pecr2);
}


uint8_t sh7014_port_device::pfdr_r()
{
	auto r = m_port_f_read_cb();
	LOGMASKED(LOG_READ, "pfdr_r %02x\n", r);
	return r;
}
