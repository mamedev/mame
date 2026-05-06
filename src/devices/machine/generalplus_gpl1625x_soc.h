// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_GENERALPLUS_GPL1625X_SOC_H
#define MAME_MACHINE_GENERALPLUS_GPL1625X_SOC_H

#pragma once

#include "generalplus_gpl162xx_soc.h"


class generalplus_gpac800_device : public sunplus_gcm394_base_device
{
public:
	template <typename T>
	generalplus_gpac800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag) :
		generalplus_gpac800_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
		m_csbase = 0x30000;
	}

	generalplus_gpac800_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	void gpac800_internal_map(address_map &map) ATTR_COLD;

	//virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void recalculate_calculate_effective_nand_address();

	u16 nand_7850_status_r();
	u16 nand_7854_r();
	void nand_dma_ctrl_w(u16 data);
	void nand_7850_w(u16 data);
	void nand_command_w(u16 data);
	void nand_addr_low_w(u16 data);
	void nand_addr_high_w(u16 data);
	u16 nand_ecc_low_byte_error_flag_1_r();
	void nand_7856_type_w(u16 data);
	void nand_7857_w(u16 data);
	void nand_785b_w(u16 data);
	void nand_785c_w(u16 data);
	void nand_785d_w(u16 data);
	u16 nand_785e_r();

	u16 m_nandcommand;

	u16 m_nand_addr_low;
	u16 m_nand_addr_high;

	u16 m_nand_dma_ctrl;
	u16 m_nand_7850;
	u16 m_nand_785d;
	u16 m_nand_785c;
	u16 m_nand_785b;
	u16 m_nand_7856;
	u16 m_nand_7857;

	int m_curblockaddr;
	u32 m_effectiveaddress;
};


class generalplus_gpspispi_device : public sunplus_gcm394_base_device
{
public:
	template <typename T>
	generalplus_gpspispi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag) :
		generalplus_gpspispi_device(mconfig, tag, owner, clock)
	{
		m_screen.set_tag(std::forward<T>(screen_tag));
		m_csbase = 0x30000;
	}

	generalplus_gpspispi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	void gpspispi_internal_map(address_map &map) ATTR_COLD;

	//virtual void device_start() override ATTR_COLD;
	//virtual void device_reset() override ATTR_COLD;

private:
	u16 spi_unk_7943_r();
};

DECLARE_DEVICE_TYPE(GPAC800, generalplus_gpac800_device)
DECLARE_DEVICE_TYPE(GP_SPISPI, generalplus_gpspispi_device)


#endif // MAME_MACHINE_GENERALPLUS_GPL1625X_SOC_H
