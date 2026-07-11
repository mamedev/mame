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

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	u16 nand_7850_status_r();
	u16 nand_data_r();
	void nand_data_w(u16 data);
	void nand_dma_ctrl_w(u16 data);
	void nand_7850_w(u16 data);
	void nand_command_w(u16 data);
	void nand_addr_low_w(u16 data);
	void nand_addr_high_w(u16 data);
	u16 nand_ecc_err1_lb_r();
	void nand_bch_ctrl_w(u16 data);
	void nand_ecc_ctrl_w(u16 data);
	void nand_ecc_lpr_ckl_lb_w(u16 data);
	void nand_ecc_lpr_ckh_lb_w(u16 data);
	void nand_ecc_cpckr_lb_w(u16 data);
	u16 nand_ecc_err0_lb_r();

	u16 spi_rxstatus_r();

	u16 efuse2_r();

	u16 m_nand_addr_low;
	u16 m_nand_addr_high;

	u16 m_nand_dma_ctrl;
	u16 m_nand_ctrl;
	u16 m_nand_ecc_cpckr_lb;
	u16 m_nand_ecc_lpr_ckh_lb;
	u16 m_nand_ecc_lpr_ckl_lb;
	u16 m_nand_bch_ctrl;
	u16 m_nand_ecc_ctrl;
};

DECLARE_DEVICE_TYPE(GPL16250, generalplus_gpac800_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL1625X_SOC_H
