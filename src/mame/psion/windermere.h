// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Nigel Barnes
// thanks-to:Ash Wolf
/***************************************************************************

    Psion Windermere peripheral emulation

***************************************************************************/

#ifndef MAME_PSION_WINDERMERE_H
#define MAME_PSION_WINDERMERE_H

#pragma once

#include "cpu/arm7/arm7.h"


class windermere_device : public device_t,
	public device_video_interface
{
public:
	windermere_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	windermere_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: windermere_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
	}

	void set_screen_origin(uint16_t x_offset, uint16_t y_offset) { m_lcd_x_offset = x_offset; m_lcd_y_offset = y_offset; }

	// callbacks
	auto lcd_dma_cb() { return m_lcd_dma_cb.bind(); }
	auto buz_cb() { return m_buz_cb.bind(); }
	auto col_cb() { return m_col_cb.bind(); }
	auto ssp_r() { return m_ssp_r.bind(); }

	auto pcm_in() { return m_pcm_in.bind(); }
	auto pcm_out() { return m_pcm_out.bind(); }

	auto porta_r() { return m_port_r[PORTA].bind(); }
	auto porta_w() { return m_port_w[PORTA].bind(); }
	auto portb_r() { return m_port_r[PORTB].bind(); }
	auto portb_w() { return m_port_w[PORTB].bind(); }
	auto portc_r() { return m_port_r[PORTC].bind(); }
	auto portc_w() { return m_port_w[PORTC].bind(); }
	auto portd_r() { return m_port_r[PORTD].bind(); }
	auto portd_w() { return m_port_w[PORTD].bind(); }
	auto porte_r() { return m_port_r[PORTE].bind(); }
	auto porte_w() { return m_port_w[PORTE].bind(); }

	void extfiq_w(int state);
	void eint1_w(int state);
	void eint2_w(int state);
	void eint3_w(int state);

	uint32_t periphs_r(offs_t offset, uint32_t mem_mask = ~0);
	void periphs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(update_timer);
	TIMER_CALLBACK_MEMBER(update_rtc);
	TIMER_CALLBACK_MEMBER(update_codec);

	void set_timer_ctrl(int timer, uint8_t value);
	void check_interrupts();

	// Static Memory Interface
	static constexpr uint16_t REG_MEMCFG1 = 0x0000;
	static constexpr uint16_t REG_MEMCFG2 = 0x0004;

	// Dynamic Memory Interface
	static constexpr uint16_t REG_DRAMCFG = 0x0100;

	// LCD Internal Registers
	static constexpr uint16_t REG_LCDCTL    = 0x0200;
	static constexpr uint16_t REG_LCDST     = 0x0204;
	static constexpr uint16_t REG_LCD_DBAR1 = 0x0210;
	static constexpr uint16_t REG_LCD_DBAR2 = 0x0218;
	static constexpr uint16_t REG_LCDT0     = 0x0220;
	static constexpr uint16_t REG_LCDT1     = 0x0224;
	static constexpr uint16_t REG_LCDT2     = 0x0228;

	// Power/State Control
	static constexpr uint16_t REG_PWRSR  = 0x0400;
	static constexpr uint16_t REG_PWRCNT = 0x0404;
	static constexpr uint16_t REG_HALT   = 0x0408;
	static constexpr uint16_t REG_STBY   = 0x040c;
	static constexpr uint16_t REG_BLEOI  = 0x0410;
	static constexpr uint16_t REG_MCEOI  = 0x0414;
	static constexpr uint16_t REG_TEOI   = 0x0418;
	static constexpr uint16_t REG_STFCLR = 0x041c;
	static constexpr uint16_t REG_E2EOI  = 0x0420;

	// Interrupt Controller
	static constexpr uint16_t REG_INTSR    = 0x0500;
	static constexpr uint16_t REG_INTRSR   = 0x0504;
	static constexpr uint16_t REG_INTENS   = 0x0508;
	static constexpr uint16_t REG_INTENC   = 0x050c;
	static constexpr uint16_t REG_INTTEST1 = 0x0514;
	static constexpr uint16_t REG_INTTEST2 = 0x0518;

	// UART1 and SIR
	static constexpr uint16_t REG_UART1DR   = 0x0600;
	static constexpr uint16_t REG_UART1FCR  = 0x0604;
	static constexpr uint16_t REG_UART1BR   = 0x0608;
	static constexpr uint16_t REG_UART1CON  = 0x060c;
	static constexpr uint16_t REG_UART1FLG  = 0x0610;
	static constexpr uint16_t REG_UART1INT  = 0x0614;
	static constexpr uint16_t REG_UART1INTM = 0x0618;
	static constexpr uint16_t REG_UART1INTR = 0x061c;

	// UART2
	static constexpr uint16_t REG_UART2DR   = 0x0700;
	static constexpr uint16_t REG_UART2FCR  = 0x0704;
	static constexpr uint16_t REG_UART2BR   = 0x0708;
	static constexpr uint16_t REG_UART2CON  = 0x070c;
	static constexpr uint16_t REG_UART2FLG  = 0x0710;
	static constexpr uint16_t REG_UART2INT  = 0x0714;
	static constexpr uint16_t REG_UART2INTM = 0x0718;
	static constexpr uint16_t REG_UART2INTR = 0x071c;

	// DC-DC Converter
	static constexpr uint16_t REG_PUMPCON = 0x0900;

	// Codec Interface
	static constexpr uint16_t REG_CODR   = 0x0a00;
	static constexpr uint16_t REG_CONFG  = 0x0a04;
	static constexpr uint16_t REG_COLFG  = 0x0a08;
	static constexpr uint16_t REG_COEOI  = 0x0a0c;
	static constexpr uint16_t REG_COTEST = 0x0a10;

	// Synchronous Serial Interface Port
	static constexpr uint16_t REG_SSCR0 = 0x0b00;
	static constexpr uint16_t REG_SSCR1 = 0x0b04;
	static constexpr uint16_t REG_SSDR  = 0x0b0c;
	static constexpr uint16_t REG_SSSR  = 0x0b14;

	// Counter Timer 1 & 2
	static constexpr uint16_t REG_TC1LOAD = 0x0c00;
	static constexpr uint16_t REG_TC1VAL  = 0x0c04;
	static constexpr uint16_t REG_TC1CTRL = 0x0c08;
	static constexpr uint16_t REG_TC1EOI  = 0x0c0c;
	static constexpr uint16_t REG_TC2LOAD = 0x0c20;
	static constexpr uint16_t REG_TC2VAL  = 0x0c24;
	static constexpr uint16_t REG_TC2CTRL = 0x0c28;
	static constexpr uint16_t REG_TC2EOI  = 0x0c2c;
	static constexpr uint16_t REG_BZCONT  = 0x0c40;

	// Real Time Clock
	static constexpr uint16_t REG_RTCDRL = 0x0d00;
	static constexpr uint16_t REG_RTCDRU = 0x0d04;
	static constexpr uint16_t REG_RTCMRL = 0x0d08;
	static constexpr uint16_t REG_RTCMRU = 0x0d0c;
	static constexpr uint16_t REG_RTCEOI = 0x0d10;

	// Programmable I/O, Keyboard Scan
	static constexpr uint16_t REG_PADR  = 0x0e00;
	static constexpr uint16_t REG_PBDR  = 0x0e04;
	static constexpr uint16_t REG_PCDR  = 0x0e08;
	static constexpr uint16_t REG_PDDR  = 0x0e0c;
	static constexpr uint16_t REG_PADDR = 0x0e10;
	static constexpr uint16_t REG_PBDDR = 0x0e14;
	static constexpr uint16_t REG_PCDDR = 0x0e18;
	static constexpr uint16_t REG_PDDDR = 0x0e1c;
	static constexpr uint16_t REG_PEDR  = 0x0e20;
	static constexpr uint16_t REG_PEDDR = 0x0e24;
	static constexpr uint16_t REG_KSCAN = 0x0e28;
	static constexpr uint16_t REG_LCDMUX = 0x0e2c;

	static constexpr int IRQ_EXTFIQ = 0;  // FiqExternal
	static constexpr int IRQ_BLINT  = 1;  // FiqBatLow
	static constexpr int IRQ_WEINT  = 2;  // FiqWatchDog
	static constexpr int IRQ_MCINT  = 3;  // FiqMediaChg
	static constexpr int IRQ_CSINT  = 4;  // IrqCodec
	static constexpr int IRQ_EINT1  = 5;  // IrqExt1
	static constexpr int IRQ_EINT2  = 6;  // IrqExt2
	static constexpr int IRQ_EINT3  = 7;  // IrqExt3
	static constexpr int IRQ_TC1OI  = 8;  // IrqTimer1
	static constexpr int IRQ_TC2OI  = 9;  // IrqTimer2
	static constexpr int IRQ_RTCMI  = 10; // IrqRtcMatch
	static constexpr int IRQ_TINT   = 11; // IrqTick
	static constexpr int IRQ_UART1  = 12; // IrqUart1
	static constexpr int IRQ_UART2  = 13; // IrqUart2
	static constexpr int IRQ_LCDINT = 14; // IrqLcd
	static constexpr int IRQ_SSEOTI = 15; // IrqSpi

	static constexpr uint16_t IRQ_FIQ_MASK = 0x000f;
	static constexpr uint16_t IRQ_IRQ_MASK = 0xfff0;

	enum
	{
		PORTA,
		PORTB,
		PORTC,
		PORTD,
		PORTE
	};

	required_device<arm7_cpu_device> m_maincpu;

	devcb_read8 m_lcd_dma_cb;
	devcb_write_line m_buz_cb;
	devcb_write8 m_col_cb;
	devcb_read16 m_ssp_r;

	devcb_read8::array<5> m_port_r;
	devcb_write8::array<5> m_port_w;

	devcb_read8 m_pcm_in;
	devcb_write8 m_pcm_out;

	emu_timer *m_timer[2];

	uint32_t m_memcfg[2];
	uint16_t m_dramcfg;

	uint32_t m_lcd_base_addr[2];
	uint32_t m_lcd_control;
	uint32_t m_lcd_timing[3]{};
	uint16_t m_lcd_x_offset = 0;
	uint16_t m_lcd_y_offset = 0;

	uint16_t m_timer_load[2];
	uint16_t m_timer_ctrl[2];
	uint16_t m_timer_value[2];

	uint16_t m_pending_ints;
	uint16_t m_int_mask;

	uint8_t m_uartfcr[2]{};
	uint8_t m_uartcon[2]{};

	uint32_t m_rtc;
	uint16_t m_pwrsr;
	uint16_t m_pwrcnt;
	uint16_t m_pumpcon;
	uint8_t  m_kscan;
	uint16_t m_last_ssi_request;
	uint16_t m_ssi_read_counter;
	uint8_t  m_buzzer_ctrl;
	int m_buzzer_tog;
	uint8_t  m_confg;
	util::fifo<uint8_t, 16> m_codec_rx;
	util::fifo<uint8_t, 16> m_codec_tx;

	uint8_t m_port_data[5];
	uint8_t m_port_ddr[5];

	emu_timer *m_rtc_ticker;
	emu_timer *m_codec_timer;
};

DECLARE_DEVICE_TYPE(WINDERMERE, windermere_device)

#endif // MAME_PSION_WINDERMERE_H
