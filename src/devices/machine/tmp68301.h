// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_MACHINE_TMP68301_H
#define MAME_MACHINE_TMP68301_H

#pragma once

#include "cpu/m68000/m68000.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

/* TODO: serial ports, frequency & hook it up with m68k */

// FIXME: M68000 ought to be a parent class, not an external object
#define MCFG_TMP68301_CPU(_tag) \
	tmp68301_device::set_cpu_tag(*device, "^" _tag);

#define MCFG_TMP68301_IN_PARALLEL_CB(cb) \
	devcb = &tmp68301_device::set_in_parallel_callback(*device, (DEVCB_##cb));

#define MCFG_TMP68301_OUT_PARALLEL_CB(cb) \
	devcb = &tmp68301_device::set_out_parallel_callback(*device, (DEVCB_##cb));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************



class tmp68301_device : public device_t,
						public device_memory_interface
{
public:
	tmp68301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_cpu_tag(device_t &device, const char *tag) { downcast<tmp68301_device &>(device).m_cpu.set_tag(tag); }
	template <class Object> static devcb_base &set_in_parallel_callback(device_t &device, Object &&cb) { return downcast<tmp68301_device &>(device).m_in_parallel_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_out_parallel_callback(device_t &device, Object &&cb) { return downcast<tmp68301_device &>(device).m_out_parallel_cb.set_callback(std::forward<Object>(cb)); }

	// Hardware Registers
	DECLARE_READ16_MEMBER( regs_r );
	DECLARE_WRITE16_MEMBER( regs_w );

	// Interrupts
	void external_interrupt_0();
	void external_interrupt_1();
	void external_interrupt_2();

	IRQ_CALLBACK_MEMBER(irq_callback);

private:
	DECLARE_READ16_MEMBER(imr_r);
	DECLARE_WRITE16_MEMBER(imr_w);
	DECLARE_READ16_MEMBER(iisr_r);
	DECLARE_WRITE16_MEMBER(iisr_w);
	DECLARE_READ16_MEMBER(scr_r);
	DECLARE_WRITE16_MEMBER(scr_w);
	DECLARE_READ16_MEMBER(pdr_r);
	DECLARE_WRITE16_MEMBER(pdr_w);
	DECLARE_READ16_MEMBER(pdir_r);
	DECLARE_WRITE16_MEMBER(pdir_w);
	DECLARE_READ8_MEMBER(icr_r);
	DECLARE_WRITE8_MEMBER(icr_w);

	void tmp68301_regs(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;

private:
	TIMER_CALLBACK_MEMBER( timer_callback );
	void update_timer( int i );
	void update_irq_state(uint16_t cause);
	void update_irq_serial(uint16_t cause, uint8_t type);

	static constexpr uint16_t EXT_IRQ0 = 1 << 0;
	static constexpr uint16_t EXT_IRQ1 = 1 << 1;
	static constexpr uint16_t EXT_IRQ2 = 1 << 2;
	static constexpr uint16_t SERIAL_IRQ_CH0 = 1 << 4;
	static constexpr uint16_t SERIAL_IRQ_CH1 = 1 << 5;
	static constexpr uint16_t SERIAL_IRQ_CH2 = 1 << 6;
	static constexpr uint16_t PARALLEL_IRQ = 1 << 7;
	static constexpr uint16_t TIMER0_IRQ = 1 << 8;
	static constexpr uint16_t TIMER1_IRQ = 1 << 9;
	static constexpr uint16_t TIMER2_IRQ = 1 << 10;

	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);

	required_device<m68000_base_device> m_cpu;

	devcb_read16         m_in_parallel_cb;
	devcb_write16        m_out_parallel_cb;

	// internal state
	uint16_t m_regs[0x400];

	emu_timer *m_tmp68301_timer[3];        // 3 Timers

	uint16_t m_irq_vector[8];

	uint16_t m_imr;
	uint16_t m_iisr;
	uint16_t m_scr;
	uint16_t m_pdir;
	uint16_t m_pdr;
	uint8_t m_icr[10];

	const address_space_config      m_space_config;
};

DECLARE_DEVICE_TYPE(TMP68301, tmp68301_device)

#endif // MAME_MACHINE_TMP68301_H
