// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************
                            _____    _____
                   Vdd   1 |*    \__/     | 40  RD0/AD8
               RC0/AD0   2 |              | 39  RD1/AD9
               RC1/AD1   3 |              | 38  RD2/AD10
               RC2/AD2   4 |              | 37  RD3/AD11
               RC3/AD3   5 |              | 36  RD4/AD12
               RC4/AD4   6 |              | 35  RD5/AD13
               RC5/AD5   7 |              | 34  RD6/AD14
               RC6/AD6   8 |              | 33  RD7/AD15
               RC7/AD7   9 |              | 32  _MCLR/Vpp
                   Vss  10 |   PIC17C4X   | 31  Vss
              RB0/CAP1  11 |              | 30  RE0/ALE
              RB1/CAP2  12 |              | 29  RE1/_OE
              RB2/PWM1  13 |              | 28  RE2/_WR
              RB3/PWM2  14 |              | 27  TEST
            RB4/TCLK12  15 |              | 26  RA0/INT
             RB5/TCLK3  16 |              | 25  RA1/T0CKI
                   RB6  17 |              | 24  RA2
                   RB7  18 |              | 23  RA3
            OSC1/CLKIN  19 |              | 22  RA4/RX/DT
           OSC2/CLKOUT  20 |______________| 21  RA5/TX/CK

**********************************************************************/

#ifndef MAME_CPU_PIC17_PIC17C4X_H
#define MAME_CPU_PIC17_PIC17C4X_H

#pragma once

#include "pic17.h"

class pic17c4x_device : public pic17_cpu_device
{
public:
	enum {
		RA0_LINE = 0,
		INT_LINE = RA0_LINE,
		RA1_LINE = 1,
		T0CKI_LINE = RA1_LINE,
		RA2_LINE = 2,
		RA3_LINE = 3,
		RA4_LINE = 4,
		RX_LINE = RA4_LINE,
		RA5_LINE = 5
	};

	enum {
		PIC17_LATA = PIC17_PS + 1,
		PIC17_DDRB,
		PIC17_LATB,
		PIC17_TMR1,
		PIC17_TMR2,
		PIC17_TMR3,
		PIC17_PR1,
		PIC17_PR2,
		PIC17_CA1,
		PIC17_CA2,
		PIC17_TCON1,
		PIC17_TCON2,
		PIC17_RCSTA,
		PIC17_TXSTA,
		PIC17_SPBRG,
		PIC17_PIE,
		PIC17_PIR
	};

	// callback configuration
	auto ra_out_cb() { return m_port_out_cb[0].bind(); }
	auto rb_out_cb() { return m_port_out_cb[1].bind(); }
	auto rc_out_cb() { return m_port_out_cb[2].bind(); }
	auto rd_out_cb() { return m_port_out_cb[3].bind(); }
	auto re_out_cb() { return m_port_out_cb[4].bind(); }

protected:
	pic17c4x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 rom_size, address_map_constructor data_map);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual bool execute_input_edge_triggered(int linenum) const noexcept override { return linenum == INT_LINE || linenum == T0CKI_LINE; }
	virtual void execute_set_input(int linenum, int state) override;

	void data_map(address_map &map) ATTR_COLD;

	virtual void increment_timers() override;

private:
	u8 porta_r();
	void porta_w(u8 data);
	u8 ddrb_r();
	void ddrb_w(u8 data);
	u8 portb_r();
	void portb_w(u8 data);
	u8 tmr1_r();
	void tmr1_w(u8 data);
	u8 tmr2_r();
	void tmr2_w(u8 data);
	u8 tmr3l_r();
	void tmr3l_w(u8 data);
	u8 tmr3h_r();
	void tmr3h_w(u8 data);
	u8 pr1_r();
	void pr1_w(u8 data);
	u8 pr2_r();
	void pr2_w(u8 data);
	u8 pr3l_ca1l_r();
	void pr3l_ca1l_w(u8 data);
	u8 pr3h_ca1h_r();
	void pr3h_ca1h_w(u8 data);
	u8 ca2l_r();
	u8 ca2h_r();
	u8 tcon1_r();
	void tcon1_w(u8 data);
	u8 tcon2_r();
	void tcon2_w(u8 data);
	u8 rcsta_r();
	void rcsta_w(u8 data);
	u8 txsta_r();
	void txsta_w(u8 data);
	void txreg_w(u8 data);
	u8 spbrg_r();
	void spbrg_w(u8 data);
	u8 pir_r();
	void pir_w(u8 data);
	u8 pie_r();
	void pie_w(u8 data);

	// callback objects
	devcb_write8::array<5> m_port_out_cb;

	// internal state
	u8 m_rain;
	u8 m_lata;
	bool m_rbpu;
	u8 m_ddrb;
	u8 m_latb;
	u8 m_tmr8bit[2];
	u8 m_pr8bit[2];
	u16 m_tmr3;
	u16 m_ca16bit[2];
	u8 m_tcon1;
	u8 m_tcon2;
	u8 m_rcsta;
	u8 m_txsta;
	u8 m_spbrg;
	u8 m_pir;
	u8 m_pie;
};

class pic17c43_device : public pic17c4x_device
{
public:
	// device type constructor
	pic17c43_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	pic17c43_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u16 rom_size);

private:
	void data_map(address_map &map) ATTR_COLD;
};

class pic17c44_device : public pic17c43_device
{
public:
	// device type constructor
	pic17c44_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
};

// device type declarations
DECLARE_DEVICE_TYPE(PIC17C43, pic17c43_device)
DECLARE_DEVICE_TYPE(PIC17C44, pic17c44_device)

#endif // MAME_CPU_PIC17_PIC17C4X_H
