// license:BSD-3-Clause
// copyright-holders:Devin Acker
/***************************************************************************

    gew12.h

    Yamaha GEW12 (65c02-based)

***************************************************************************/
#ifndef MAME_CPU_M6502_GEW12_H
#define MAME_CPU_M6502_GEW12_H

#include "m65c02.h"
#include "machine/gew12_uart.h"

class gew12_device : public m65c02_device, public device_mixer_interface {
public:
	gew12_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	template<offs_t i> auto port_in_cb() { return m_in_cb[i].bind(); }
	template<offs_t i> auto port_out_cb() { return m_out_cb[i].bind(); }

	u8 irq_stat_r();
	void irq_en_w(u8 data);
	void timer_stat_w(u8 data);

	u8 timer_count_r(offs_t offset);
	void timer_count_w(offs_t offset, u8 data);

	u8 port_r(offs_t offset);
	void port_w(offs_t offset, u8 data);
	u8 port_ddr_r(offs_t offset);
	void port_ddr_w(offs_t offset, u8 data);

protected:
	virtual void device_add_mconfig(machine_config& config) override;

	virtual void device_start() override;
	virtual void device_reset() override;

	enum
	{
		INTERNAL_IRQ_TIMER0,
		INTERNAL_IRQ_TIMER1,
		INTERNAL_IRQ_MIDI_TX,
		INTERNAL_IRQ_MIDI_RX
	};

	template<int Num>
	void internal_irq(int state) { internal_irq(Num, state); }
	void internal_irq(int num, int state);

	void irq_update();

	TIMER_CALLBACK_MEMBER(timer_tick);

	void internal_map(address_map& map);

	u8 m_irq_pending, m_irq_enable;

	emu_timer* m_timer[2];
	u16 m_timer_count[2];

	devcb_read8::array<6> m_in_cb;
	devcb_write8::array<6> m_out_cb;
	u8 m_port_data[6];
	u8 m_port_ddr[6];

	required_memory_region        m_rom;
	required_memory_bank_array<2> m_bank;

	required_device<gew12_uart_device> m_uart;
};

DECLARE_DEVICE_TYPE(GEW12, gew12_device)

#endif // MAME_CPU_M6502_GEW12_H
