// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
#include "emu.h"
#include "s3c24xx.h"

#include "coreutil.h"

#include <cstdio>
#include <cstring>

//#define VERBOSE 1
#define LOG_OUTPUT_FUNC std::printf
#include "logmacro.h"


void s3c24xx_peripheral_types::usbhost_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
}


void s3c24xx_peripheral_types::memcon_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
	regs.data[0x04 / 4] = 0x00000700;
	regs.data[0x08 / 4] = 0x00000700;
	regs.data[0x0c / 4] = 0x00000700;
	regs.data[0x10 / 4] = 0x00000700;
	regs.data[0x14 / 4] = 0x00000700;
	regs.data[0x18 / 4] = 0x00000700;
	regs.data[0x1c / 4] = 0x00018008;
	regs.data[0x20 / 4] = 0x00018008;
	regs.data[0x24 / 4] = 0x00AC0000;
}


void s3c24xx_peripheral_types::uart_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
	regs.utrstat = 6;
}


void s3c24xx_peripheral_types::pwm_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
	for (emu_timer *&tmr : timer)
		tmr->adjust(attotime::never);
}

uint16_t s3c24xx_peripheral_types::pwm_t::calc_observation(unsigned ch) const
{
	double const timeleft = timer[ch]->remaining().as_double();
	LOG("timeleft %f freq %d cntb %d cmpb %d\n", timeleft, freq[ch], cnt[ch], cmp[ch]);
	double const x1 = 1 / (double(freq[ch]) / (cnt[ch] - cmp[ch] + 1));
	double const x2 = x1 / timeleft;
	LOG("x1 %f\n", x1);
	uint32_t cnto = cmp[ch] + ((cnt[ch]- cmp[ch]) / x2);
	LOG("cnto %u\n", cnto);
	return cnto;
}


void s3c24xx_peripheral_types::wdt_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
	regs.wtcon = 0x8021;
	regs.wtdat = 0x8000;
	regs.wtcnt = 0x8000;
	timer->adjust(attotime::never);
}

uint16_t s3c24xx_peripheral_types::wdt_t::calc_current_count() const
{
	double const timeleft = timer->remaining().as_double();
	LOG("timeleft %f freq %d cnt %d\n", timeleft, freq, cnt);
	double const x1 = 1 / (double(freq) / cnt);
	double const x2 = x1 / timeleft;
	printf("x1 %f\n", x1);
	uint32_t const res = cnt / x2;
	printf("cnt %d\n", res);
	return res;
}


void s3c24xx_peripheral_types::iis_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
	fifo_index = 0;
	regs.iiscon = 0x0100;
	timer->adjust(attotime::never);
}


void s3c24xx_peripheral_types::rtc_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
	regs.almday = 1;
	regs.almmon = 1;
	timer_update->adjust(attotime::never);
	timer_update->adjust(attotime::from_msec(1000), 0, attotime::from_msec(1000));
}

void s3c24xx_peripheral_types::rtc_t::recalc()
{
	if (BIT(regs.ticnt, 7))
	{
		uint32_t const ttc = BITS(regs.ticnt, 6, 0);
		double const freq = 128.0 / (ttc + 1);
		LOG("ttc %u freq %f\n", ttc, freq);
		timer_tick_count->adjust(attotime::from_hz(freq), 0, attotime::from_hz(freq));
	}
	else
	{
		timer_tick_count->adjust(attotime::never);
	}
}

void s3c24xx_peripheral_types::rtc_t::update()
{
	// increase second
	regs.bcdsec = bcd_adjust(regs.bcdsec + 1);
	if (regs.bcdsec >= 0x60)
	{
		regs.bcdsec = 0;
		// increase minute
		regs.bcdmin = bcd_adjust(regs.bcdmin + 1);
		if (regs.bcdmin >= 0x60)
		{
			regs.bcdmin = 0;
			// increase hour
			regs.bcdhour = bcd_adjust(regs.bcdhour + 1);
			if (regs.bcdhour >= 0x24)
			{
				regs.bcdhour = 0;
				// increase day-of-week
				regs.bcddow = (regs.bcddow % 7) + 1;
				// increase day
				regs.bcdday = bcd_adjust(regs.bcdday + 1);
				uint32_t const bcdday_max = dec_2_bcd(gregorian_days_in_month(bcd_2_dec(regs.bcdmon), bcd_2_dec(regs.bcdyear) + 2000));
				if (regs.bcdday > bcdday_max)
				{
					regs.bcdday = 1;
					// increase month
					regs.bcdmon = bcd_adjust(regs.bcdmon + 1);
					if (regs.bcdmon >= 0x12)
					{
						regs.bcdmon = 1;
						// increase year
						regs.bcdyear = bcd_adjust(regs.bcdyear + 1);
						if (regs.bcdyear >= 0x100)
						{
							regs.bcdyear = 0;
						}
					}
				}
			}
		}
	}
}

bool s3c24xx_peripheral_types::rtc_t::check_alarm() const
{
	return
			(regs.rtcalm & 0x40) &&
			(!(regs.rtcalm & 0x20) || (regs.almyear == regs.bcdyear)) &&
			(!(regs.rtcalm & 0x10) || (regs.almmon == regs.bcdmon)) &&
			(!(regs.rtcalm & 0x08) || (regs.almday == regs.bcdday)) &&
			(!(regs.rtcalm & 0x04) || (regs.almhour == regs.bcdhour)) &&
			(!(regs.rtcalm & 0x02) || (regs.almmin == regs.bcdmin)) &&
			(!(regs.rtcalm & 0x01) || (regs.almsec == regs.bcdsec));
}


void s3c24xx_peripheral_types::mmc_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
}


void s3c24xx_peripheral_types::cam_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
}


void s3c24xx_peripheral_types::ac97_t::reset()
{
	std::memset(&regs, 0, sizeof(regs));
}
