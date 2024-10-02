// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Ash Wolf
/***************************************************************************

        Psion 5mx (EPOC R5) series and peripherals

        Skeleton driver by Ryan Holtz, ported from work by Ash Wolf

****************************************************************************/

#ifndef MAME_PSION_PSION5_H
#define MAME_PSION_PSION5_H

#pragma once

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "etna.h"
#include "sound/spkrdev.h"

#include "screen.h"
#include "emupal.h"
#include "speaker.h"

class psion5mx_state : public driver_device
{
public:
	psion5mx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_etna(*this, "etna")
		, m_lcd_ram(*this, "lcd_ram")
		, m_palette(*this, "palette")
		, m_speaker(*this, "speaker")
		, m_touchx(*this, "TOUCHX")
		, m_touchy(*this, "TOUCHY")
		, m_touch(*this, "TOUCH")
		, m_kbd_cols(*this, "COL%u", 0U)
	{
	}

	void psion5mx(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(touch_down);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_timer1);
	TIMER_CALLBACK_MEMBER(update_timer2);
	TIMER_CALLBACK_MEMBER(update_periodic_irq);
	TIMER_CALLBACK_MEMBER(update_rtc);

private:
	void palette_init(palette_device &palette);

	uint32_t periphs_r(offs_t offset, uint32_t mem_mask = ~0);
	void periphs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void update_timer(int timer);
	void set_timer_ctrl(int timer, uint32_t value);
	void check_interrupts();

	enum
	{
		REG_MEMCFG1   = 0x0000,
		REG_MEMCFG2   = 0x0004,

		REG_DRAMCFG   = 0x0100,

		REG_LCDCTL    = 0x0200,
		REG_LCDST     = 0x0204,
		REG_LCD_DBAR1 = 0x0210,
		REG_LCDT0     = 0x0220,
		REG_LCDT1     = 0x0224,
		REG_LCDT2     = 0x0228,

		REG_PWRSR     = 0x0400,
		REG_PWRCNT    = 0x0404,
		REG_HALT      = 0x0408,
		REG_STBY      = 0x040c,
		REG_BLEOI     = 0x0410,
		REG_MCEOI     = 0x0414,
		REG_TEOI      = 0x0418,
		REG_STFCLR    = 0x041c,
		REG_E2EOI     = 0x0420,

		REG_INTSR     = 0x0500,
		REG_INTRSR    = 0x0504,
		REG_INTENS    = 0x0508,
		REG_INTENC    = 0x050c,
		REG_INTTEST1  = 0x0514,
		REG_INTTEST2  = 0x0518,

		REG_PUMPCON   = 0x0900,

		REG_CODR      = 0x0a00,
		REG_CONFG     = 0x0a04,
		REG_COLFG     = 0x0a08,
		REG_COEOI     = 0x0a0c,
		REG_COTEST    = 0x0a10,

		REG_SSCR0     = 0x0b00,
		REG_SSCR1     = 0x0b04,
		REG_SSDR      = 0x0b0c,
		REG_SSSR      = 0x0b14,

		REG_TC1LOAD   = 0x0c00,
		REG_TC1VAL    = 0x0c04,
		REG_TC1CTRL   = 0x0c08,
		REG_TC1EOI    = 0x0c0c,
		REG_TC2LOAD   = 0x0c20,
		REG_TC2VAL    = 0x0c24,
		REG_TC2CTRL   = 0x0c28,
		REG_TC2EOI    = 0x0c2c,

		REG_BZCONT    = 0x0c40,

		REG_RTCDRL    = 0x0d00,
		REG_RTCDRU    = 0x0d04,
		REG_RTCMRL    = 0x0d08,
		REG_RTCMRU    = 0x0d0c,
		REG_RTCEOI    = 0x0d10,

		REG_PADR      = 0x0e00,
		REG_PBDR      = 0x0e04,
		REG_PCDR      = 0x0e08,
		REG_PDDR      = 0x0e0c,
		REG_PADDR     = 0x0e10,
		REG_PBDDR     = 0x0e14,
		REG_PCDDR     = 0x0e18,
		REG_PDDDR     = 0x0e1c,
		REG_PEDR      = 0x0e20,
		REG_PEDDR     = 0x0e24,

		REG_KSCAN     = 0x0e28,
		REG_LCDMUX    = 0x0e2c
	};

	enum
	{
		IRQ_EXTFIQ    = 0,  // FiqExternal
		IRQ_BLINT     = 1,  // FiqBatLow
		IRQ_WEINT     = 2,  // FiqWatchDog
		IRQ_MCINT     = 3,  // FiqMediaChg
		IRQ_CSINT     = 4,  // IrqCodec
		IRQ_EINT1     = 5,  // IrqExt1
		IRQ_EINT2     = 6,  // IrqExt2
		IRQ_EINT3     = 7,  // IrqExt3
		IRQ_TC1OI     = 8,  // IrqTimer1
		IRQ_TC2OI     = 9,  // IrqTimer2
		IRQ_RTCMI     = 10, // IrqRtcMatch
		IRQ_TINT      = 11, // IrqTick
		IRQ_UART1     = 12, // IrqUart1
		IRQ_UART2     = 13, // IrqUart2
		IRQ_LCDINT    = 14, // IrqLcd
		IRQ_SSEOTI    = 15, // IrqSpi
		IRQ_FIQ_MASK  = 0x000f,
		IRQ_IRQ_MASK  = 0xfff0
	};

	enum
	{
		PORTA,
		PORTB,
		PORTC,
		PORTD,
		PORTE
	};

	void main_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t read_keyboard();

	required_device<arm710t_cpu_device> m_maincpu;
	required_device<etna_device> m_etna;
	required_shared_ptr<uint32_t> m_lcd_ram;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_speaker;
	required_ioport m_touchx;
	required_ioport m_touchy;
	required_ioport m_touch;
	required_ioport_array<8> m_kbd_cols;

	emu_timer *m_timers[2]{};

	uint32_t m_memcfg[2]{};
	uint16_t m_dramcfg = 0;

	uint16_t m_timer_reload[2]{};
	uint16_t m_timer_ctrl[2]{};
	uint16_t m_timer_value[2]{};

	uint32_t m_pending_ints = 0;
	uint32_t m_int_mask = 0;

	uint32_t m_lcd_display_base_addr = 0;

	uint32_t m_rtc = 0;
	uint32_t m_pwrsr = 0;
	uint32_t m_last_ssi_request = 0;
	uint32_t m_ssi_read_counter = 0;
	uint8_t m_buzzer_ctrl = 0;

	uint8_t m_kbd_scan = 0;

	uint8_t m_ports[5]{};

	emu_timer *m_periodic = nullptr;
	emu_timer *m_rtc_ticker = nullptr;
};

#endif // MAME_PSION_PSION5_H
