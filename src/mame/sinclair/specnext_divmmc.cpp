// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    Spectrum Next DivMMC
**********************************************************************/

#include "emu.h"
#include "specnext_divmmc.h"


specnext_divmmc_device::specnext_divmmc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SPECNEXT_DIVMMC, tag, owner, clock)
{
}

bool specnext_divmmc_device::conmem() const noexcept
{
	return BIT(m_divmmc_reg, 7);
}

bool specnext_divmmc_device::mapram() const noexcept
{
	return BIT(m_divmmc_reg, 6);
}

bool specnext_divmmc_device::page0() const noexcept
{
	return m_cpu_a_15_13 == 0;
}

bool specnext_divmmc_device::page1() const noexcept
{
	return m_cpu_a_15_13 == 1;
}

bool specnext_divmmc_device::rom_en() const noexcept
{
	return page0() && (conmem() || automap()) && !mapram();
}

bool specnext_divmmc_device::ram_en() const noexcept
{
	return (page0() && (conmem() || automap()) && mapram()) || (page1() && (conmem() || automap()));
}

u8 specnext_divmmc_device::ram_bank() const noexcept
{
	return page0() ? 3 : (m_divmmc_reg & 0x0f);
}

bool specnext_divmmc_device::automap_nmi_instant_on() const noexcept
{
	return m_automap_nmi_instant_on && m_button_nmi;
}

bool specnext_divmmc_device::automap_nmi_delayed_on() const noexcept
{
	return m_automap_nmi_delayed_on && m_button_nmi;
}

bool specnext_divmmc_device::automap() const noexcept
{
	return !m_automap_reset && (m_automap_held
			|| (m_automap_active && (m_automap_instant_on || automap_nmi_instant_on()))
			|| (m_automap_rom3_active && m_automap_rom3_instant_on));
}

void specnext_divmmc_device::clock_w() noexcept
{
	// Rising edge
	if (m_automap_reset || m_retn_seen)
		m_button_nmi = 0;
	else if (m_divmmc_button)
		m_button_nmi = 1;
	else if (m_automap_held)
		m_button_nmi = 0;

	if (m_automap_reset || m_retn_seen)
	{
		m_automap_hold = 0;
	}
	else if (!m_cpu_mreq_n && !m_cpu_m1_n)
	{
		m_automap_hold = (m_automap_active && (m_automap_instant_on || m_automap_delayed_on || automap_nmi_instant_on() || automap_nmi_delayed_on()))
				|| (m_automap_rom3_active && (m_automap_rom3_instant_on || m_automap_rom3_delayed_on))
				|| (m_automap_held && !(m_automap_active && m_automap_delayed_off));
	}

	if (m_automap_reset || m_retn_seen)
		m_automap_held = 0;
	else if (m_cpu_mreq_n)
		m_automap_held = m_automap_hold;
}


void specnext_divmmc_device::device_start()
{
	save_item(NAME(m_cpu_a_15_13));
	save_item(NAME(m_cpu_mreq_n));
	save_item(NAME(m_cpu_m1_n));
	save_item(NAME(m_en));
	save_item(NAME(m_automap_reset));
	save_item(NAME(m_automap_active));
	save_item(NAME(m_automap_rom3_active));
	save_item(NAME(m_retn_seen));
	save_item(NAME(m_divmmc_button));
	save_item(NAME(m_divmmc_reg));
	save_item(NAME(m_automap_instant_on));
	save_item(NAME(m_automap_delayed_on));
	save_item(NAME(m_automap_delayed_off));
	save_item(NAME(m_automap_rom3_instant_on));
	save_item(NAME(m_automap_rom3_delayed_on));
	save_item(NAME(m_automap_nmi_instant_on));
	save_item(NAME(m_automap_nmi_delayed_on));
	save_item(NAME(m_button_nmi));
	save_item(NAME(m_automap_hold));
	save_item(NAME(m_automap_held));
}

void specnext_divmmc_device::device_reset()
{
	m_cpu_mreq_n = 0;
	m_cpu_m1_n = 0;

	m_automap_instant_on = 0;
	m_automap_delayed_on = 0;
	m_automap_delayed_off = 0;
	m_automap_rom3_instant_on = 0;
	m_automap_rom3_delayed_on = 0;
	m_automap_nmi_instant_on = 0;
	m_automap_nmi_delayed_on = 0;

	m_automap_hold = 0;
	m_automap_held = 0;
	m_button_nmi = 0;
}


// device type definition
DEFINE_DEVICE_TYPE(SPECNEXT_DIVMMC, specnext_divmmc_device, "specnext_divmmc", "Spectrum Next DivMMC")
