// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    athensprime.cpp - Apple Athens Prime programmable timing controller.

***************************************************************************/

#include "emu.h"
#include "athensprime.h"

#define LOG_REGISTERS   (1U << 1)
#define LOG_OUTPUTS     (1U << 2)

#define VERBOSE (0)

//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(APPLE_ATHENSPRIME, athensprime_device, "aplenprime", "Apple Athens Prime")

static constexpr u8 P_DIVIDERS[4] = { 8, 4, 2, 1 };

athensprime_device::athensprime_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APPLE_ATHENSPRIME, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0x29),
	m_write_pclock(*this),
	m_write_sclock(*this),
	m_pclock(0),
	m_sclock(0)
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
	m_regs[0] = 0x41;   // ID for Athens Prime
}

void athensprime_device::device_start()
{
	save_item(NAME(m_regs));
	save_item(NAME(m_pclock));
	save_item(NAME(m_sclock));
}

void athensprime_device::write_data(u16 offset, u8 data)
{
	LOGMASKED(LOG_REGISTERS, "%s: write %02x (%d) to register %04x\n", tag(), data, data & 0x3f, offset);
	if (offset != REG_ID)
	{
		m_regs[offset] = data;
	}

	if (offset == REG_P1 || offset == REG_N1 || offset == REG_D1)
	{
		update_sclock();
	}

	if (offset == REG_P2 || offset == REG_N2 || offset == REG_D2)
	{
		update_pclock();
	}
}

u8 athensprime_device::read_data(u16 offset)
{
	LOGMASKED(LOG_REGISTERS, "%s: read from %04x\n", tag(), offset);
	return m_regs[offset];
}

void athensprime_device::update_pclock()
{
	if (m_regs[REG_N2] == 0 || m_regs[REG_D2] == 0)
	{
		return;
	}

	const u8 source = (m_regs[REG_P2] >> 4) & 3;
	const u8 pdiv = P_DIVIDERS[m_regs[REG_P2] & 0x3];

	double new_pclock = 0.0f;

	switch (source)
	{
		case 0: // dot clock PLL (input * N2 / D2) / pdiv
			new_pclock = clock() * (double(m_regs[REG_N2] & 0x3f) / double((m_regs[REG_D2] & 0x3f) * pdiv));
			break;

		case 1: // system clock PLL (input * N1 / D1) / pdiv
			update_sclock();
			new_pclock = double(m_sclock) / pdiv;
			break;

		case 2:
			new_pclock = XTAL(31'334'400).dvalue() / pdiv;
			break;
	}


	m_pclock = std::round(new_pclock);
	LOGMASKED(LOG_OUTPUTS, "%s: update pixel clock to %u Hz\n", tag(), m_pclock);
	m_write_pclock(m_pclock);
}

void athensprime_device::update_sclock()
{
	if (m_regs[REG_N1] == 0 || m_regs[REG_D1] == 0)
	{
		return;
	}

	const u8 pdiv = P_DIVIDERS[m_regs[REG_P1] & 0x3];

	double new_sclock = clock() * (double(m_regs[REG_N1] & 0x3f) / double((m_regs[REG_D1] & 0x3f) * pdiv));
	m_sclock = std::round(new_sclock);
	LOGMASKED(LOG_OUTPUTS, "%s: update system clock to %u Hz\n", tag(), m_sclock);
	m_write_sclock(m_sclock);
}
