// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    CL-PS7110 - Low-Power System-on-a-Chip

***************************************************************************/

#ifndef MAME_MACHINE_CLPS7110_H
#define MAME_MACHINE_CLPS7110_H

#pragma once

#include "cpu/arm7/arm7.h"


class clps711x_device : public device_t,
	public device_video_interface
{
public:
	void set_screen_origin(uint16_t x_offset, uint16_t y_offset) { m_lcd_x_offset = x_offset; m_lcd_y_offset = y_offset; }

	template <typename T> void set_cpu_tag(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

	// callbacks
	auto lcd_dma_cb() { return m_lcd_dma_cb.bind(); }
	auto buz_cb() { return m_buz_cb.bind(); }
	auto col_cb() { return m_col_cb.bind(); }
	auto adc_r() { return m_adc_r.bind(); }

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

	auto pcm_in() { return m_pcm_in.bind(); }
	auto pcm_out() { return m_pcm_out.bind(); }

	void extfiq_w(int state);
	void eint1_w(int state);
	void eint2_w(int state);
	void eint3_w(int state);

	uint32_t periphs_r(offs_t offset, uint32_t mem_mask = ~0);
	void periphs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	clps711x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t id);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	TIMER_CALLBACK_MEMBER(update_timer);
	TIMER_CALLBACK_MEMBER(update_rtc);

	void check_interrupts();

	// CL-PS7110/CL-PS7111
	static constexpr uint16_t REG_PADR  = 0x0000; // Port A Data register
	static constexpr uint16_t REG_PBDR  = 0x0001; // Port B Data register
	static constexpr uint16_t REG_PCDR  = 0x0002; // Port C Data register (CL-PS7110 only)
	static constexpr uint16_t REG_PDDR  = 0x0003; // Port D Data register
	static constexpr uint16_t REG_PADDR = 0x0040; // Port A Data Direction register
	static constexpr uint16_t REG_PBDDR = 0x0041; // Port B Data Direction register
	static constexpr uint16_t REG_PCDDR = 0x0042; // Port C Data Direction register
	static constexpr uint16_t REG_PDDDR = 0x0043; // Port D Data Direction register
	static constexpr uint16_t REG_PEDR  = 0x0080; // Port E Data register
	static constexpr uint16_t REG_PEDDR = 0x00c0; // Port E Data Direction register

	static constexpr uint16_t REG_SYSCON1 = 0x0100; // System Control register
	static constexpr uint16_t REG_SYSFLG1 = 0x0140; // System Status Flags register
	static constexpr uint16_t REG_MEMCFG1 = 0x0180; // Expansion and ROM Memory Configuration Register 1
	static constexpr uint16_t REG_MEMCFG2 = 0x01c0; // Expansion and ROM Memory Configuration Register 2

	static constexpr uint16_t REG_DRFPR  = 0x0200; // DRAM Refresh Period register
	static constexpr uint16_t REG_INTSR1 = 0x0240; // Interrupt Status register
	static constexpr uint16_t REG_INTMR1 = 0x0280; // Interrupt Mask register
	static constexpr uint16_t REG_LCDCON = 0x02c0; // LCD Control register

	static constexpr uint16_t REG_TC1D  = 0x0300; // Read/write data to TC1
	static constexpr uint16_t REG_TC2D  = 0x0340; // Read/write data to TC2
	static constexpr uint16_t REG_RTCDR = 0x0380; // Realtime Clock Data register
	static constexpr uint16_t REG_RTCMR = 0x03c0; // Realtime Clock Match register

	static constexpr uint16_t REG_PMPCON  = 0x0400; // DC-to-DC Pump Control register
	static constexpr uint16_t REG_CODR    = 0x0440; // Codec Data I/O register
	static constexpr uint16_t REG_UARTDR1 = 0x0480; // UART FIFO Data register
	static constexpr uint16_t REG_UBRLCR1 = 0x04c0; // UART Bit Rate and Line Control register

	static constexpr uint16_t REG_SYNCIO = 0x0500; // Synchronous Serial I/O Data register
	static constexpr uint16_t REG_PALLSW = 0x0540; // Least-significant 32-bit word of LCD Palette register
	static constexpr uint16_t REG_PALMSW = 0x0580; // Most-significant 32-bit word of LCD Palette register
	static constexpr uint16_t REG_STFCLR = 0x05c0; // Write to clear all start up reason flags

	static constexpr uint16_t REG_BLEOI  = 0x0600; // Write to clear Battery Low interrupt
	static constexpr uint16_t REG_MCEOI  = 0x0640; // Write to clear Media Changed interrupt
	static constexpr uint16_t REG_TEOI   = 0x0680; // Write to clear Tick and Watchdog interrupt
	static constexpr uint16_t REG_TC1EOI = 0x06c0; // Write to clear TC1 interrupt

	static constexpr uint16_t REG_TC2EOI = 0x0700; // Write to clear TC2 interrupt
	static constexpr uint16_t REG_RTCEOI = 0x0740; // Write to clear RTC Match interrupt
	static constexpr uint16_t REG_UMSEOI = 0x0780; // Write to clear UART Modem Status Changed interrupt
	static constexpr uint16_t REG_COEOI  = 0x07c0; // Write to clear Codec Sound interrupt

	// CL-PS7111 only
	static constexpr uint16_t REG_HALT    = 0x0800; // Write to enter idle state
	static constexpr uint16_t REG_STDBY   = 0x0840; // Write to enter standby state

	static constexpr uint16_t REG_FRBADDR = 0x1000; // LCD Frame Buffer Start Address register

	static constexpr uint16_t REG_SYSCON2 = 0x1100; // System Control register 2
	static constexpr uint16_t REG_SYSFLG2 = 0x1140; // System Status Flag register 2

	static constexpr uint16_t REG_INTSR2  = 0x1240; // Interrupt Status register 2
	static constexpr uint16_t REG_INTMR2  = 0x1280; // Interrupt Mask register 2

	static constexpr uint16_t REG_UARTDR2 = 0x1480; // UART2 Data register
	static constexpr uint16_t REG_UBRLCR2 = 0x14c0; // UART2 Control register

	static constexpr uint16_t REG_KBDEOI  = 0x1700; // Write to clear keyboard interrupt


	static constexpr int IRQ_EXTFIQ  = 0;  // External fast interrupt input (NEXTFIQ)
	static constexpr int IRQ_BLINT   = 1;  // Battery low interrupt
	static constexpr int IRQ_WEINT   = 2;  // Watch dog expired interrupt
	static constexpr int IRQ_MCINT   = 3;  // Media changed interrupt
	static constexpr int IRQ_CSINT   = 4;  // Codec sound interrupt
	static constexpr int IRQ_EINT1   = 5;  // External interrupt input 1 (NEINT1)
	static constexpr int IRQ_EINT2   = 6;  // External interrupt input 2 (NEINT2)
	static constexpr int IRQ_EINT3   = 7;  // External interrupt input 3 (EINT3)
	static constexpr int IRQ_TC1OI   = 8;  // TC1 under flow interrupt
	static constexpr int IRQ_TC2OI   = 9;  // TC2 under flow interrupt
	static constexpr int IRQ_RTCMI   = 10; // RTC compare match interrupt
	static constexpr int IRQ_TINT    = 11; // 64Hz tick interrupt
	static constexpr int IRQ_UTXINT1 = 12; // Internal UART1 transmit FIFO empty interrupt
	static constexpr int IRQ_URXINT1 = 13; // Internal UART1 receive FIFO full interrupt
	static constexpr int IRQ_UMSINT  = 14; // Internal UART1 modem status changed interrupt
	static constexpr int IRQ_SSEOTI  = 15; // Synchronous serial interface, end of transfer interrupt

	static constexpr int IRQ_KBDINT  = 16; // Key press interrupt
	static constexpr int IRQ_UTXINT2 = 28; // Internal UART2 transmit FIFO empty interrupt
	static constexpr int IRQ_URXINT2 = 29; // Internal UART2 receive FIFO full interrupt

	static constexpr uint32_t IRQ_FIQ_MASK = 0x0000000f;
	static constexpr uint32_t IRQ_IRQ_MASK = 0xfffffff0;

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
	devcb_read16 m_adc_r;

	devcb_read8::array<5> m_port_r;
	devcb_write8::array<5> m_port_w;

	devcb_read8 m_pcm_in;
	devcb_write8 m_pcm_out;

	emu_timer *m_timer[2];
	emu_timer *m_rtc_ticker;

	uint8_t  m_id; // 1 for CL-PS7111, 0 for CL-PS7110

	uint32_t m_memcfg[2];
	uint16_t m_dramcfg;
	uint32_t m_syscon[2];
	uint32_t m_sysflg1;
	uint16_t m_syncio;
	uint16_t m_pmpcon;

	uint16_t m_timer_reload[2];
	uint16_t m_timer_value[2];

	uint32_t m_int_status;
	uint32_t m_int_mask;

	uint32_t m_lcd_base_addr;
	uint32_t m_lcdcon;
	uint64_t m_lcdpal;
	uint16_t m_lcd_x_offset = 0;
	uint16_t m_lcd_y_offset = 0;

	uint32_t m_rtc;
	uint8_t  m_rtcdiv;

	int m_buzzer_tog;

	uint8_t m_port_data[5];
	uint8_t m_port_ddr[5];
};


class clps7110_device : public clps711x_device
{
public:
	clps7110_device(const machine_config &mconfig, const char *tag, device_t* owner, uint32_t clock);

	template <typename T>
	clps7110_device(const machine_config &mconfig, const char *tag, device_t* owner, uint32_t clock, T &&cpu_tag)
		: clps7110_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
};

class clps7111_device : public clps711x_device
{
public:
	clps7111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T>
	clps7111_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag)
		: clps7111_device(mconfig, tag, owner, clock)
	{
		set_cpu_tag(std::forward<T>(cpu_tag));
	}
};


DECLARE_DEVICE_TYPE(CLPS7110, clps7110_device)
DECLARE_DEVICE_TYPE(CLPS7111, clps7111_device)

#endif // MAME_MACHINE_CLPS7110_H
