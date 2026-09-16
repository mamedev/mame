// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "generalplus_gpl951xx_rtc.h"

#define LOG_RTC (1U << 1)

#define VERBOSE     (LOG_RTC)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(GPL951XX_RTC, gpl951xx_rtc_device, "gpl951xx_rtc", "GPL951XX Real Time Clock")

gpl951xx_rtc_device::gpl951xx_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, GPL951XX_RTC, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("rtc", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(gpl951xx_rtc_device::rtc_regs), this))
{
}

device_memory_interface::space_config_vector gpl951xx_rtc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

inline uint8_t gpl951xx_rtc_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}

inline void gpl951xx_rtc_device::writebyte(offs_t address, uint8_t data)
{
	space().write_byte(address, data);
}

void gpl951xx_rtc_device::device_start()
{
	save_item(NAME(m_rtc_addr));
	save_item(NAME(m_read_dat));
	save_item(NAME(m_write_dat));

	save_item(NAME(m_reg40));
	save_item(NAME(m_reg50));

	save_item(NAME(m_loadcnt));
	save_item(NAME(m_alarmdat));
	save_item(NAME(m_timerval));

	// not on reset? (should be saved as NV?)
	m_loadcnt = 0;
	m_alarmdat = 0;
	m_timerval = 0;
}

void gpl951xx_rtc_device::device_reset()
{
	m_rtc_addr = 0;
	m_read_dat = 0;
	m_write_dat = 0;

	m_reg40 = 0;
	m_reg50 = 0;
}

// Reg00
//
// 4  Rtcclken
// 3  Rtcrst
// 2  Timerdir
// 1  SWreset
// 0  Busy

u8 gpl951xx_rtc_device::reg00_r()
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::reg00_r\n", machine().describe_context());
	return 0x00;
}

// Reg01
//
// 3  Slow_clk
// 2  Reserved
// 1  Strong
// 0  Reserved

u8 gpl951xx_rtc_device::reg01_r()
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::reg01_r\n", machine().describe_context());
	return 0x01;
}

// Reg02
//
// 'reliable'

u8 gpl951xx_rtc_device::reg02_r()
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::reg02_r\n", machine().describe_context());
	return m_reg02;
}

void gpl951xx_rtc_device::reg02_w(u8 data)
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::reg02_w %02x\n", machine().describe_context(), data);
	m_reg02 = data;
}

// Reg40
//
// 3  HalfSecint
// 2  Rtcwakeup
// 1  Alarmint
// 0  Secint

u8 gpl951xx_rtc_device::reg40_r()
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::reg40_r\n", machine().describe_context());
	return m_reg40;
}

void gpl951xx_rtc_device::reg40_w(u8 data)
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::reg40_w %02x\n", machine().describe_context(), data);
	m_reg40 = data;
}

// Reg50
//
// 3  HalfSecint_en
// 2  Rtcwakeup_en
// 1  Alarmint_en
// 0  Secint_en

u8 gpl951xx_rtc_device::reg50_r()
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::reg50_r\n", machine().describe_context());
	return m_reg50;
}

void gpl951xx_rtc_device::reg50_w(u8 data)
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::reg50_w %02x\n", machine().describe_context(), data);
	m_reg50 = data;
}

void gpl951xx_rtc_device::loadcnt_w(offs_t offset, u8 data)
{
	u64 bits = (u64)0xff << (8 * offset);
	u64 databits = (u64)data << (8 * offset);
	m_loadcnt = m_loadcnt & ~bits;
	m_loadcnt |= databits;

	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::loadcnt_w (offset %02x) %02x (loadcnt is now %08x%08x)\n", machine().describe_context(), offset, data, (u32)(m_loadcnt >> 32), (u32)(m_loadcnt & 0xffffffff));

	// TODO: not sure this is how it's reloaded
	if (offset == 5)
		m_timerval = m_loadcnt;
}

void gpl951xx_rtc_device::alarmdat_w(offs_t offset, u8 data)
{
	u64 bits = (u64)0xff << (8 * offset);
	u64 databits = (u64)data << (8 * offset);
	m_alarmdat = m_alarmdat & ~bits;
	m_alarmdat |= databits;

	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::alarmdat_w (offset %02x) %02x (alarmdat is now %08x%08x)\n", machine().describe_context(), offset, data, (u32)(m_loadcnt >> 32), (u32)(m_loadcnt & 0xffffffff));
}

u8 gpl951xx_rtc_device::timerval_r(offs_t offset)
{
	LOGMASKED(LOG_RTC, "%s: gpl951xx_rtc_device::timerval_r (offset %02x)\n", machine().describe_context(), offset);
	return m_timerval >> (offset * 8);
}

void gpl951xx_rtc_device::rtc_regs(address_map &map)
{
	if (!has_configured_map(0))
	{
		map(0x00, 0x00).r(FUNC(gpl951xx_rtc_device::reg00_r));
		map(0x01, 0x01).r(FUNC(gpl951xx_rtc_device::reg01_r));
		map(0x02, 0x02).rw(FUNC(gpl951xx_rtc_device::reg02_r), FUNC(gpl951xx_rtc_device::reg02_w));

		map(0x10, 0x15).w(FUNC(gpl951xx_rtc_device::loadcnt_w));
		map(0x20, 0x25).w(FUNC(gpl951xx_rtc_device::alarmdat_w));
		map(0x30, 0x35).r(FUNC(gpl951xx_rtc_device::timerval_r));

		map(0x40, 0x40).rw(FUNC(gpl951xx_rtc_device::reg40_r), FUNC(gpl951xx_rtc_device::reg40_w));
		map(0x50, 0x50).rw(FUNC(gpl951xx_rtc_device::reg50_r), FUNC(gpl951xx_rtc_device::reg50_w));
		// 0x80 - 0x8f reserved
		map(0x90, 0xff).ram();
	}
}


u16 gpl951xx_rtc_device::rtc_readdata_r()
{
	LOGMASKED(LOG_RTC, "%s: rtc_readdata_r\n", machine().describe_context());
	return m_read_dat;
}

// 15-1  unused
//  0    RDY
u16 gpl951xx_rtc_device::rtc_ready_r()
{
	LOGMASKED(LOG_RTC, "%s: rtc_ready_r\n", machine().describe_context());
	return 0x0001;
}


// 15-1  unused
//  0    SIEN    (Serial interface enabled)
void gpl951xx_rtc_device::rtc_ctrl_w(u16 data)
{
	LOGMASKED(LOG_RTC, "%s: rtc_ctrl_w %04x\n", machine().describe_context(), data);
}

void gpl951xx_rtc_device::rtc_addr_w(u16 data)
{
	LOGMASKED(LOG_RTC, "%s: rtc_addr_w %04x\n", machine().describe_context(), data);
	m_rtc_addr = data & 0xff;
}

void gpl951xx_rtc_device::rtc_writedata_w(u16 data)
{
	LOGMASKED(LOG_RTC, "%s: rtc_writedata_w %04x\n", machine().describe_context(), data);
	m_write_dat = data;
}

// 15-2 unused
//  1   Read Request
//  0   Write Request
void gpl951xx_rtc_device::rtc_request_w(u16 data)
{
	LOGMASKED(LOG_RTC, "%s: rtc_request_w %04x\n", machine().describe_context(), data);
	if (data & 1)
		writebyte(m_rtc_addr, m_write_dat);
	if (data & 2)
		m_read_dat = readbyte(m_rtc_addr);
}

TIMER_DEVICE_CALLBACK_MEMBER( gpl951xx_rtc_device::rtc_cb )
{
	m_timerval++;
}

void gpl951xx_rtc_device::device_add_mconfig(machine_config &config)
{
	TIMER(config, "rtc_timer").configure_periodic(FUNC(gpl951xx_rtc_device::rtc_cb), attotime::from_hz(32768));
}
