// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_GENERALPLUS_GPL951XX_SOC_H
#define MAME_MACHINE_GENERALPLUS_GPL951XX_SOC_H

#pragma once

#include "generalplus_gpl162xx_soc.h"

class generalplus_gpl951xx_device : public sunplus_gcm394_base_device
{
public:
	template <typename T>
	generalplus_gpl951xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&screen_tag) :
		generalplus_gpl951xx_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
		//m_csbase = 0x30000;
		// TODO: is cs_space even used by this type?
		m_csbase = 0xffffffff;
	}

	generalplus_gpl951xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	void gpspi_direct_internal_map(address_map &map) ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t ramread_r(offs_t offset);
	void ramwrite_w(offs_t offset, uint16_t data);
	uint16_t spi_direct_7b40_r();
	uint16_t spi_direct_7b46_r();
	uint16_t spi_direct_7af0_r();
	void spi_direct_7af0_w(uint16_t data);
	uint16_t spi_direct_79f5_r();
	uint16_t spi_direct_78e8_r();
	void spi_direct_78e8_w(uint16_t data);
	uint16_t spi_direct_79f4_r();

	uint16_t m_7af0;
};

DECLARE_DEVICE_TYPE(GPL951XX, generalplus_gpl951xx_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL951XX_SOC_H
