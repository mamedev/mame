// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_MACHINE_GENERALPLUS_GPL951XX_RTC_H
#define MAME_MACHINE_GENERALPLUS_GPL951XX_RTC_H

#pragma once

class gpl951xx_rtc_device :   public device_t,
						public device_memory_interface
{
public:

	gpl951xx_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void rtc_regs(address_map &map) ATTR_COLD;

	u16 rtc_readdata_r();
	u16 rtc_ready_r();
	void rtc_ctrl_w(u16 data);
	void rtc_addr_w(u16 data);
	void rtc_writedata_w(u16 data);
	void rtc_request_w(u16 data);

protected:
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	const address_space_config      m_space_config;

	inline uint8_t readbyte(offs_t address);
	inline void writebyte(offs_t address, uint8_t data);

	u8 reg00_r();
	u8 reg01_r();

	void reg40_w(u8 data);

	u8 reg50_r();
	void reg50_w(u8 data);

	u8 m_rtc_addr;
	u8 m_read_dat;
	u8 m_write_dat;

	u8 m_reg40;
	u8 m_reg50;
};

DECLARE_DEVICE_TYPE(GPL951XX_RTC, gpl951xx_rtc_device)

#endif // MAME_MACHINE_GENERALPLUS_GPL951XX_RTC_H
