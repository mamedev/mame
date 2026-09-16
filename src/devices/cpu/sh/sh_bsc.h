// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_bsc.h

    SH Bus State Controller

***************************************************************************/

#ifndef MAME_CPU_SH_SH_BSC_H
#define MAME_CPU_SH_SH_BSC_H

#pragma once

class sh_bsc_device : public device_t {
public:
	sh_bsc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	u16 bcr1_r();
	void bcr1_w(offs_t, u16 data, u16 mem_mask);
	u16 bcr2_r();
	void bcr2_w(offs_t, u16 data, u16 mem_mask);
	u16 wcr1_r();
	void wcr1_w(offs_t, u16 data, u16 mem_mask);
	u16 wcr2_r();
	void wcr2_w(offs_t, u16 data, u16 mem_mask);
	u16 dcr_r();
	void dcr_w(offs_t, u16 data, u16 mem_mask);
	u16 rtcsr_r();
	void rtcsr_w(offs_t, u16 data, u16 mem_mask);
	u16 rtcnt_r();
	void rtcnt_w(offs_t, u16 data, u16 mem_mask);
	u16 rtcor_r();
	void rtcor_w(offs_t, u16 data, u16 mem_mask);


protected:
	u16 m_bcr1, m_bcr2, m_wcr1, m_wcr2, m_dcr, m_rtcsr, m_rtcnt, m_rtcor;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SH_BSC, sh_bsc_device)

#endif
