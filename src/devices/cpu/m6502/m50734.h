// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Mitsubishi M50734

**********************************************************************
                            _____   _____
            P04/STBout   1 |*    \_/     | 64  Vcc
              P05/_DME   2 |             | 63  P03/BUSYout
               P06/TxD   3 |             | 62  P02/CNTR
               P07/RxD   4 |             | 61  P01/_INT2
                   P10   5 |             | 60  P00/_INT1
                   P11   6 |             | 59  WDout
                   P12   7 |             | 58  A0/D0
                   P13   8 |             | 57  A1/D1
                   P14   9 |             | 56  A2/D2
                   P15  10 |             | 55  A3/D3
                   P16  11 |             | 54  A4/D4
                   P17  12 |             | 53  A5/D5
                P20/Ha  13 |             | 52  A6/D6
                P21/Hb  14 |             | 51  A7/D7
                P22/Hc  15 |   M50734SP  | 50  ALE
                P23/Hd  16 |             | 49  A8
                P24/Va  17 |             | 48  A9
                P25/Vb  18 |             | 47  A10
                P26/Vc  19 |             | 46  A11
                P27/Vd  20 |             | 45  A12
            P30/PWMout  21 |             | 44  A13
              P31/Sclk  22 |             | 43  A14
               P32/Sio  23 |             | 42  A15
                   P33  24 |             | 41  SYNC
                   P34  25 |             | 40  ɸ
                   P35  26 |             | 39  _WR
                   P36  27 |             | 38  _RD
                   P37  28 |             | 37  Vref
                _RESET  29 |             | 36  P43/AN3
                   Xin  30 |             | 35  P42/AN2
                  Xout  31 |             | 34  P41/AN1
                   Vss  32 |_____________| 33  P40/AN0

**********************************************************************/

#ifndef MAME_CPU_M6502_M50734_H
#define MAME_CPU_M6502_M50734_H

#pragma once

#include "m740.h"

class m50734_device : public m740_device
{
public:
	m50734_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// port callback configuration
	auto p0_in_cb() { return m_port_in_cb[0].bind(); }
	auto p0_out_cb() { return m_port_out_cb[0].bind(); }
	auto p1_in_cb() { return m_port_in_cb[1].bind(); }
	auto p1_out_cb() { return m_port_out_cb[1].bind(); }
	auto p2_in_cb() { return m_port_in_cb[2].bind(); }
	auto p2_out_cb() { return m_port_out_cb[2].bind(); }
	auto p3_in_cb() { return m_port_in_cb[3].bind(); }
	auto p3_out_cb() { return m_port_out_cb[3].bind(); }
	auto p4_in_cb() { return m_port_in_cb[4].bind(); }
	auto an0_in_cb() { return m_analog_in_cb[0].bind(); }
	auto an1_in_cb() { return m_analog_in_cb[1].bind(); }
	auto an2_in_cb() { return m_analog_in_cb[2].bind(); }
	auto an3_in_cb() { return m_analog_in_cb[3].bind(); }

	// port three-state output configuration
	void set_p0_3state(u8 value) { assert(!configured()); m_port_3state[0] = value; }
	void set_p1_3state(u8 value) { assert(!configured()); m_port_3state[1] = value; }
	void set_p2_3state(u8 value) { assert(!configured()); m_port_3state[2] = value; }
	void set_p3_3state(u8 value) { assert(!configured()); m_port_3state[3] = value; }

protected:
	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_execute_interface implementation
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept override { return (clocks + 4 - 1) / 4; }
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept override { return (cycles * 4); }

	// device_memory_interface implementation
	space_config_vector memory_space_config() const override;

	// m740_device overrides
	virtual void read_dummy(u16 adr) override;
	virtual uint8_t read_data(u16 adr) override;
	virtual void write_data(u16 adr, u8 val) override;

private:
	u8 interrupt_control_r(offs_t offset);
	void interrupt_control_w(offs_t offset, u8 data);

	template <int N> u8 port_r(offs_t offset);
	template <int N> void port_w(offs_t offset, u8 data);
	u8 p4_r();
	u8 p0_function_r();
	void p0_function_w(u8 data);
	u8 p2_p3_function_r();
	void p2_p3_function_w(u8 data);

	u8 ad_control_r();
	void ad_control_w(u8 data);
	u8 ad_r();
	TIMER_CALLBACK_MEMBER(ad_complete);

	u8 timer_r(offs_t offset);
	void timer_w(offs_t offset, u8 data);
	void step_motor(int which);
	template <int N> TIMER_CALLBACK_MEMBER(timer_interrupt);
	u8 step_counter_r(offs_t offset);
	void step_counter_w(offs_t offset, u8 data);
	u8 phase_counter_r();
	void phase_counter_w(u8 data);
	u8 smcon_r(offs_t offset);
	void smcon_w(offs_t offset, u8 data);
	TIMER_CALLBACK_MEMBER(timer_x_interrupt);
	u16 get_timer_x() const;
	void set_timer_x(u16 count);
	u8 timer_x_r(offs_t offset);
	void timer_x_w(offs_t offset, u8 data);

	void internal_map(address_map &map) ATTR_COLD;

	const address_space_config m_data_config;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::specific m_data;

	devcb_read8::array<5> m_port_in_cb;
	devcb_write8::array<4> m_port_out_cb;
	devcb_read8::array<4> m_analog_in_cb;

	emu_timer *m_ad_timer;
	emu_timer *m_timer[3];
	emu_timer *m_timer_x;

	u8 m_port_latch[4];
	u8 m_port_direction[4];
	u8 m_port_3state[4];
	u8 m_p0_function;
	u8 m_p2_p3_function;
	u8 m_ad_control;
	u8 m_ad_register;
	u8 m_prescaler_reload[3];
	u8 m_timer_reload[3];
	u8 m_step_counter[2];
	u8 m_phase_counter;
	u8 m_smcon[2];
	u16 m_tx_count;
	u16 m_tx_reload;
	u8 m_interrupt_control[3];
};

DECLARE_DEVICE_TYPE(M50734, m50734_device)

#endif // MAME_CPU_M6502_M50734_H
