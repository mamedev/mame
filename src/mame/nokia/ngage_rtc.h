// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
#ifndef MAME_NOKIA_NGAGE_RTC_H
#define MAME_NOKIA_NGAGE_RTC_H

#pragma once

class ngage_rtc_device : public device_t, public device_nvram_interface
{
public:
	ngage_rtc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	u32 counter() const;
	void set_day(u8 day);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void nvram_default() override;
	virtual bool nvram_read(util::read_stream &file) override;
	virtual bool nvram_write(util::write_stream &file) override;

private:
	void set_counter(u32 value);
	u32 m_base = 0;
};

DECLARE_DEVICE_TYPE(NGAGE_RTC, ngage_rtc_device)

#endif // MAME_NOKIA_NGAGE_RTC_H
