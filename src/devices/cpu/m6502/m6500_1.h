// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    m6500_1.h

    MOS Technology 6500/1, original NMOS variant with onboard peripherals:
    * 6502 CPU
    * 2048*8 mask ROM
    * 64*8 static RAM
    * Four eight-bit open drain I/O ports
    * Sixteen-bit programmable counter/latch

                            _____   _____
                   Vrr   1 |*    \_/     | 40  /NMI
                   PD7   2 |             | 38  /RES
                   PD6   3 |             | 38  PA0
                   PD5   4 |             | 37  PA1
                   PD4   5 |             | 36  PA2
                   PD3   6 |             | 35  PA3
                   PD2   7 |             | 34  PA4
                   PD1   8 |             | 33  PA5
                   PD0   9 |             | 32  PA6
                  XTLI  10 |   6500/1    | 31  PA7
                  XTLO  11 |             | 30  Vcc
                   Vss  12 |             | 29  PB0
                   PC7  13 |             | 28  PB1
                   PC6  14 |             | 27  PB2
                   PC5  15 |             | 26  PB3
                   PC4  16 |             | 25  PB4
                   PC3  17 |             | 24  PB5
                   PC2  18 |             | 23  PB6
                   PC1  19 |             | 22  PB7
                   PC0  20 |_____________| 21  CNTR

***************************************************************************/
#ifndef MAME_CPU_M6502_M6500_1_H
#define MAME_CPU_M6502_M6500_1_H

#pragma once

#include "m6502.h"

class m6500_1_device : public m6502_mcu_device
{
public:
	m6500_1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	auto pa_in_cb() { return m_port_in_cb[0].bind(); }
	auto pb_in_cb() { return m_port_in_cb[1].bind(); }
	auto pc_in_cb() { return m_port_in_cb[2].bind(); }
	auto pd_in_cb() { return m_port_in_cb[3].bind(); }
	auto pa_out_cb() { return m_port_out_cb[0].bind(); }
	auto pb_out_cb() { return m_port_out_cb[1].bind(); }
	auto pc_out_cb() { return m_port_out_cb[2].bind(); }
	auto pd_out_cb() { return m_port_out_cb[3].bind(); }
	auto cntr_out_cb() { return m_cntr_out_cb.bind(); }

	DECLARE_READ8_MEMBER(pa_r) { return m_port_buf[0]; }
	DECLARE_READ8_MEMBER(pb_r) { return m_port_buf[1]; }
	DECLARE_READ8_MEMBER(pc_r) { return m_port_buf[2]; }
	DECLARE_READ8_MEMBER(pd_r) { return m_port_buf[3]; }
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_WRITE8_MEMBER(pc_w);
	DECLARE_WRITE8_MEMBER(pd_w);

	DECLARE_WRITE_LINE_MEMBER(cntr_w);

protected:
	enum
	{
		M6500_1_CR = M6502_IR + 1,
		M6500_1_UL,
		M6500_1_LL,
		M6500_1_UC,
		M6500_1_LC
	};

	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual u64 execute_clocks_to_cycles(u64 clocks) const override;
	virtual u64 execute_cycles_to_clocks(u64 cycles) const override;

	virtual void state_import(device_state_entry const &entry) override;
	virtual void state_export(device_state_entry const &entry) override;

	virtual void internal_update(u64 current_time) override;
	using m6502_mcu_device::internal_update;

	DECLARE_READ8_MEMBER(read_control_register);
	DECLARE_WRITE8_MEMBER(write_control_register);
	void update_irq();

	DECLARE_READ8_MEMBER(read_port);
	DECLARE_WRITE8_MEMBER(write_port);
	DECLARE_WRITE8_MEMBER(clear_edge);
	template <unsigned Port> TIMER_CALLBACK_MEMBER(set_port_in);

	DECLARE_READ8_MEMBER(read_upper_count);
	DECLARE_READ8_MEMBER(read_lower_count);
	template <bool Transfer> DECLARE_WRITE8_MEMBER(write_upper_latch);
	DECLARE_WRITE8_MEMBER(write_lower_latch);
	u64 update_counter(u64 current_time);
	bool should_count() const;
	bool pulse_generator_mode() const;
	bool event_counter_mode() const;
	TIMER_CALLBACK_MEMBER(set_cntr_in);
	void toggle_cntr();

	void memory_map(address_map &map);

private:
	devcb_read8         m_port_in_cb[4];
	devcb_write8        m_port_out_cb[4];
	devcb_write_line    m_cntr_out_cb;

	u8  m_cr;

	u8  m_port_in[4], m_port_buf[4];

	u64 m_counter_base;
	u16 m_counter, m_latch;
	u8  m_cntr_in, m_cntr_out;

	u8  m_ul, m_ll, m_uc, m_lc;
};

DECLARE_DEVICE_TYPE(M6500_1, m6500_1_device)

#endif // MAME_CPU_M6502_M6500_1_H
