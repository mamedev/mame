// license:BSD-3-Clause
// copyright-holders:Tim Schuerewegen
#ifndef MAME_MACHINE_S3C24XX_H
#define MAME_MACHINE_S3C24XX_H

#pragma once

#include "cpu/arm7/arm7.h"

class s3c24xx_peripheral_types // TODO: better name for this
{
protected:
	template <typename A, typename B, typename C> static constexpr auto BITS(A &&x, B &&m, C &&n)
	{
		return (x >> n) & ((uint32_t(1) << (m - n + 1)) - 1);
	}

	template <typename A, typename B, typename C> static constexpr auto CLR_BITS(A &&x, B &&m, C &&n)
	{
		return x & ~(((uint32_t(1) << (m - n + 1)) - 1) << n);
	}

	struct memcon_regs_t
	{
		uint32_t data[0x34/4];
	};

	struct usbhost_regs_t
	{
		uint32_t data[0x5c/4];
	};

	struct lcdpal_regs_t
	{
		uint32_t data[0x400/4];
	};

	struct uart_regs_t
	{
		uint32_t ulcon;
		uint32_t ucon;
		uint32_t ufcon;
		uint32_t umcon;
		uint32_t utrstat;
		uint32_t uerstat;
		uint32_t ufstat;
		uint32_t umstat;
		uint32_t utxh;
		uint32_t urxh;
		uint32_t ubrdiv;
	};

	struct pwm_regs_t
	{
		uint32_t tcfg0;
		uint32_t tcfg1;
		uint32_t tcon;
		uint32_t tcntb0;
		uint32_t tcmpb0;
		uint32_t tcnto0;
		uint32_t tcntb1;
		uint32_t tcmpb1;
		uint32_t tcnto1;
		uint32_t tcntb2;
		uint32_t tcmpb2;
		uint32_t tcnto2;
		uint32_t tcntb3;
		uint32_t tcmpb3;
		uint32_t tcnto3;
		uint32_t tcntb4;
		uint32_t tcnto4;
	};

	struct wdt_regs_t
	{
		uint32_t wtcon;
		uint32_t wtdat;
		uint32_t wtcnt;
	};

	struct iis_regs_t
	{
		uint32_t iiscon;
		uint32_t iismod;
		uint32_t iispsr;
		uint32_t iisfcon;
		uint32_t iisfifo;
	};

	struct rtc_regs_t
	{
		uint32_t rtccon;
		uint32_t ticnt;
		uint32_t reserved[2];
		uint32_t rtcalm;
		uint32_t almsec;
		uint32_t almmin;
		uint32_t almhour;
		uint32_t almday;
		uint32_t almmon;
		uint32_t almyear;
		uint32_t rtcrst;
		uint32_t bcdsec;
		uint32_t bcdmin;
		uint32_t bcdhour;
		uint32_t bcdday;
		uint32_t bcddow;
		uint32_t bcdmon;
		uint32_t bcdyear;
	};

	struct mmc_regs_t
	{
		uint32_t data[0x40/4];
	};

	struct spi_regs_t
	{
		uint32_t spcon;
		uint32_t spsta;
		uint32_t sppin;
		uint32_t sppre;
		uint32_t sptdat;
		uint32_t sprdat;
	};

	struct sdi_regs_t
	{
		uint32_t data[0x44/4];
	};

	struct cam_regs_t
	{
		uint32_t data[0xa4/4];
	};

	struct ac97_regs_t
	{
		uint32_t data[0x20/4];
	};


	struct memcon_t
	{
		void reset();

		memcon_regs_t regs;
	};

	struct usbhost_t
	{
		void reset();

		usbhost_regs_t regs;
	};

	struct lcdpal_t
	{
		lcdpal_regs_t regs;
	};

	struct uart_t
	{
		static constexpr offs_t ULCON   = 0x00 / 4; // UART Line Control
		static constexpr offs_t UCON    = 0x04 / 4; // UART Control
		static constexpr offs_t UFCON   = 0x08 / 4; // UART FIFO Control
		static constexpr offs_t UMCON   = 0x0c / 4; // UART Modem Control
		static constexpr offs_t UTRSTAT = 0x10 / 4; // UART Tx/Rx Status
		static constexpr offs_t UERSTAT = 0x14 / 4; // UART Rx Error Status
		static constexpr offs_t UFSTAT  = 0x18 / 4; // UART FIFO Status
		static constexpr offs_t UMSTAT  = 0x1c / 4; // UART Modem Status
		static constexpr offs_t UTXH    = 0x20 / 4; // UART Transmission Hold
		static constexpr offs_t URXH    = 0x24 / 4; // UART Receive Buffer
		static constexpr offs_t UBRDIV  = 0x28 / 4; // UART Baud Rate Divisor

		void reset();

		uart_regs_t regs;
	};

	struct pwm_t
	{
		static constexpr offs_t TCFG0  = 0x00 / 4; // Timer Configuration
		static constexpr offs_t TCFG1  = 0x04 / 4; // Timer Configuration
		static constexpr offs_t TCON   = 0x08 / 4; // Timer Control
		static constexpr offs_t TCNTB0 = 0x0c / 4; // Timer Count Buffer 0
		static constexpr offs_t TCMPB0 = 0x10 / 4; // Timer Compare Buffer 0
		static constexpr offs_t TCNTO0 = 0x14 / 4; // Timer Count Observation 0
		static constexpr offs_t TCNTB1 = 0x18 / 4; // Timer Count Buffer 1
		static constexpr offs_t TCMPB1 = 0x1c / 4; // Timer Compare Buffer 1
		static constexpr offs_t TCNTO1 = 0x20 / 4; // Timer Count Observation 1
		static constexpr offs_t TCNTB2 = 0x24 / 4; // Timer Count Buffer 2
		static constexpr offs_t TCMPB2 = 0x28 / 4; // Timer Compare Buffer 2
		static constexpr offs_t TCNTO2 = 0x2c / 4; // Timer Count Observation 2
		static constexpr offs_t TCNTB3 = 0x30 / 4; // Timer Count Buffer 3
		static constexpr offs_t TCMPB3 = 0x34 / 4; // Timer Compare Buffer 3
		static constexpr offs_t TCNTO3 = 0x38 / 4; // Timer Count Observation 3
		static constexpr offs_t TCNTB4 = 0x3c / 4; // Timer Count Buffer 4
		static constexpr offs_t TCNTO4 = 0x40 / 4; // Timer Count Observation 4

		void reset();
		uint16_t calc_observation(unsigned ch) const;

		pwm_regs_t regs;
		emu_timer *timer[5];
		uint32_t cnt[5];
		uint32_t cmp[5];
		uint32_t freq[5];
	};

	struct wdt_t
	{
		static constexpr offs_t WTCON = 0x00 / 4; // Watchdog Timer Mode
		static constexpr offs_t WTDAT = 0x04 / 4; // Watchdog Timer Data
		static constexpr offs_t WTCNT = 0x08 / 4; // Watchdog Timer Count

		void reset();
		uint16_t calc_current_count() const;

		wdt_regs_t regs;
		emu_timer *timer;
		uint32_t freq, cnt;
	};

	struct iis_t
	{
		static constexpr offs_t IISCON  = 0x00 / 4; // IIS Control
		static constexpr offs_t IISMOD  = 0x04 / 4; // IIS Mode
		static constexpr offs_t IISPSR  = 0x08 / 4; // IIS Prescaler
		static constexpr offs_t IISFCON = 0x0c / 4; // IIS FIFO Control
		static constexpr offs_t IISFIFO = 0x10 / 4; // IIS FIFO Entry

		void reset();

		iis_regs_t regs;
		emu_timer *timer;
		uint16_t fifo[16/2];
		int fifo_index;
	};

	struct rtc_t
	{
		static constexpr offs_t RTCCON  = 0x00 / 4; // RTC Control
		static constexpr offs_t TICNT   = 0x04 / 4; // Tick Time count
		static constexpr offs_t RTCALM  = 0x10 / 4; // RTC Alarm Control
		static constexpr offs_t ALMSEC  = 0x14 / 4; // Alarm Second
		static constexpr offs_t ALMMIN  = 0x18 / 4; // Alarm Minute
		static constexpr offs_t ALMHOUR = 0x1c / 4; // Alarm Hour
		static constexpr offs_t ALMDAY  = 0x20 / 4; // Alarm Day
		static constexpr offs_t ALMMON  = 0x24 / 4; // Alarm Month
		static constexpr offs_t ALMYEAR = 0x28 / 4; // Alarm Year
		static constexpr offs_t RTCRST  = 0x2c / 4; // RTC Round Reset
		static constexpr offs_t BCDSEC  = 0x30 / 4; // BCD Second
		static constexpr offs_t BCDMIN  = 0x34 / 4; // BCD Minute
		static constexpr offs_t BCDHOUR = 0x38 / 4; // BCD Hour
		static constexpr offs_t BCDDAY  = 0x3c / 4; // BCD Day
		static constexpr offs_t BCDDOW  = 0x40 / 4; // BCD Day of Week
		static constexpr offs_t BCDMON  = 0x44 / 4; // BCD Month
		static constexpr offs_t BCDYEAR = 0x48 / 4; // BCD Year

		void reset();
		void recalc();
		void update();
		bool check_alarm() const;

		rtc_regs_t regs;
		emu_timer *timer_tick_count;
		emu_timer *timer_update;
	};

	struct mmc_t
	{
		void reset();

		mmc_regs_t regs;
	};

	struct spi_t
	{
		static constexpr offs_t SPCON  = 0x00 / 4; // SPI Control
		static constexpr offs_t SPSTA  = 0x04 / 4; // SPI Status
		static constexpr offs_t SPPIN  = 0x08 / 4; // SPI Pin Control
		static constexpr offs_t SPPRE  = 0x0c / 4; // SPI Baud Rate Prescaler
		static constexpr offs_t SPTDAT = 0x10 / 4; // SPI Tx Data
		static constexpr offs_t SPRDAT = 0x14 / 4; // SPI Rx Data

		spi_regs_t regs;
	};

	struct sdi_t
	{
		sdi_regs_t regs;
	};

	struct cam_t
	{
		void reset();

		cam_regs_t regs;
	};

	struct ac97_t
	{
		void reset();

		ac97_regs_t regs;
	};
};

#endif // MAME_MACHINE_S3C24XX_H
