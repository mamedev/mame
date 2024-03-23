// license:BSD-3-Clause
// copyright-holders:hap
/*

OKI MSM5001N CMOS LCD Watch IC

Decap shows it's not an MCU. No known documentation exists, but there are datasheets
available for equivalent Samsung chips: KS5198, KS5199A, KS5114.

These kind of chips were used a lot for cheap 2-button digital wristwatches.

TODO:
- add D/S inputs (other display modes, setup mode)
- datasheets mention a 4-year calendar, does it mean it supports leap years somehow?
- one of the Samsung datasheets mention 24hr mode, does the MSM5001N support that?

*/

#include "emu.h"
#include "machine/msm5001n.h"


DEFINE_DEVICE_TYPE(MSM5001N, msm5001n_device, "msm5001n", "OKI MSM5001N LCD Watch")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

msm5001n_device::msm5001n_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, MSM5001N, tag, owner, clock),
	device_rtc_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_write_segs(*this)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

// allow save_item on a non-fundamental type
ALLOW_SAVE_TYPE(msm5001n_device::mode);

void msm5001n_device::device_start()
{
	m_timer = timer_alloc(FUNC(msm5001n_device::clock_tick), this);

	m_power = true;
	initialize();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_mode));
	save_item(NAME(m_counter));
}

void msm5001n_device::initialize()
{
	m_mode = MODE_NORMAL_HRMIN;
	m_counter = 0;

	// 1 January, 1AM
	set_time(true, 0, 1, 1, 1, 1, 0, 0);
}


//-------------------------------------------------
//  input pins
//-------------------------------------------------

void msm5001n_device::d_w(int state)
{
}

void msm5001n_device::s_w(int state)
{
}

void msm5001n_device::power_w(int state)
{
	if (m_power && !state)
	{
		// reset chip when power goes off
		initialize();
		write_lcd(nullptr, false);
	}

	m_power = bool(state);
}

void msm5001n_device::device_clock_changed()
{
	// smallest interval (at default frequency of 32768Hz) is LCD refresh at 32Hz
	attotime period = attotime::from_ticks(1024, clock());
	m_timer->adjust(period, 0, period);

	// clear LCD if clock stopped
	if (clock() == 0)
		write_lcd(nullptr, false);
}


//-------------------------------------------------
//  process
//-------------------------------------------------

void msm5001n_device::write_lcd(u8 *digits, bool colon)
{
	u32 segs = 0;

	if (digits)
	{
		// 0-9, A, P, none
		static const u8 lut_segs[0x10] =
			{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x73,0x00,0x00,0x00,0x00 };

		for (int i = 0; i < 4; i++)
			segs |= lut_segs[digits[i]] << (8 * i);
	}
	segs |= colon ? 0x80 : 0;

	// COM1: BC1,F2,A2,B2,COLON,F3,AD3,B3,F4,A4,B4
	m_write_segs(0, bitswap<11>(segs,25,21,16,17,7,13,8,9,5,0,1));

	// COM2: D2,E2,G2,C2,D4,E3,G3,C3,E4,G4,C4
	m_write_segs(1, bitswap<11>(segs,19,20,22,18,3,12,14,10,4,6,2));
}

TIMER_CALLBACK_MEMBER(msm5001n_device::clock_tick)
{
	if (!m_power)
		return;

	m_counter++;
	if ((m_counter & 0x1f) == 0)
		advance_seconds();

	u8 digits[4];
	for (int i = 0; i < 4; i++)
		digits[i] = 0xf;
	bool colon = false;

	// convert current time to BCD
	u8 minute = convert_to_bcd(get_clock_register(RTC_MINUTE));
	u8 hour = get_clock_register(RTC_HOUR) % 12;
	hour = convert_to_bcd((hour == 0) ? 12 : hour);

	switch (m_mode)
	{
		case MODE_NORMAL_HRMIN:
			digits[0] = minute & 0xf;
			digits[1] = minute >> 4;
			digits[2] = hour & 0xf;
			if (hour & 0xf0)
				digits[3] = hour >> 4;

			colon = !BIT(m_counter, 4);
			break;

		default:
			break;
	}

	write_lcd(digits, colon);
}


//-------------------------------------------------
//  nvram
//-------------------------------------------------

bool msm5001n_device::nvram_write(util::write_stream &file)
{
	u8 buf[5];

	// current time
	for (int i = 0; i < 5; i++)
		buf[i] = get_clock_register(i);

	auto const [err, actual] = write(file, &buf, sizeof(buf));
	if (err)
		return false;

	return true;
}

bool msm5001n_device::nvram_read(util::read_stream &file)
{
	u8 buf[5];
	auto const [err, actual] = read(file, &buf, sizeof(buf));
	if (err || (sizeof(buf) != actual))
		return false;

	// current time
	for (int i = 0; i < 5; i++)
		set_clock_register(i, buf[i]);

	return true;
}
