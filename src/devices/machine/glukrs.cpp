// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/***************************************************************************

  Mr Gluk Reset Service

  Refs:
  https://zxart.ee/spa/software/prikladnoe-po/electronics/pzu/mr-gluk-reset-service-663/mr-gluk-reset-service-663/action:viewFile/id:250389/fileId:814961/

****************************************************************************/

#include "emu.h"
#include "glukrs.h"

glukrs_device::glukrs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GLUKRS, tag, owner, clock),
	  device_rtc_interface(mconfig, *this),
	  m_nvram(*this, "nvram") {}

void glukrs_device::device_add_mconfig(machine_config &config)
{
  NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);
}

void glukrs_device::device_start()
{
  m_nvram->set_base(&m_cmos, sizeof(m_cmos));

  m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(glukrs_device::timer_callback), this));
  m_timer->adjust(attotime::from_hz(clock() / XTAL(32'768)), 0, attotime::from_hz(clock() / XTAL(32'768)));
}

u8 glukrs_device::read(offs_t address)
{
  return m_cmos[address];
}

void glukrs_device::write(offs_t address, u8 data)
{
  m_cmos[address] = data;
}

void glukrs_device::rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second)
{
  m_cmos[0x00] = convert_to_bcd(second);
  m_cmos[0x02] = convert_to_bcd(minute);
  m_cmos[0x04] = convert_to_bcd(hour);
  m_cmos[0x06] = convert_to_bcd(day_of_week);
  m_cmos[0x07] = convert_to_bcd(day);
  m_cmos[0x08] = convert_to_bcd(month);
  m_cmos[0x09] = convert_to_bcd(year % 100);
}

TIMER_CALLBACK_MEMBER(glukrs_device::timer_callback)
{
  advance_seconds();
}

// device type definition
DEFINE_DEVICE_TYPE(GLUKRS, glukrs_device, "glukrs", "Mr Gluk Reset Service")
