// license:BSD-3-Clause
// copyright-holders:windyfairy
/***************************************************************************

  SH7014 Bus State Controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH7014_BSC_H
#define MAME_CPU_SH_SH7014_BSC_H

#pragma once


class sh7014_bsc_device : public device_t
{
public:
	sh7014_bsc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	void map(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t bcr1_r();
	void bcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t bcr2_r();
	void bcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t wcr1_r();
	void wcr1_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t wcr2_r();
	void wcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t dcr_r();
	void dcr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t rtcsr_r();
	void rtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t rtcor_r();
	void rtcor_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t rtcnt_r();
	void rtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t m_bcr1, m_bcr2;
	uint16_t m_wcr1, m_wcr2;
	uint16_t m_dcr;
	uint16_t m_rtcsr;
	uint16_t m_rtcnt;
	uint16_t m_rtcor;
};


DECLARE_DEVICE_TYPE(SH7014_BSC, sh7014_bsc_device)

#endif // MAME_CPU_SH_SH7014_BSC_H
