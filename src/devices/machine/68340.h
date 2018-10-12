// license:BSD-3-Clause
// copyright-holders:David Haywood
/* 68340 */
#ifndef MAME_MACHINE_68340_H
#define MAME_MACHINE_68340_H

#pragma once

#include "cpu/m68000/m68000.h"

#include "68340sim.h"
#include "68340dma.h"
#include "68340ser.h"
#include "68340tmu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************
#define MCFG_MC68340_PA_INPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_pa_in_callback(DEVCB_##_devcb);

#define MCFG_MC68340_PA_OUTPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_pa_out_callback(DEVCB_##_devcb);

#define MCFG_MC68340_PB_INPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_pb_in_callback(DEVCB_##_devcb);

#define MCFG_MC68340_PB_OUTPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_pb_out_callback(DEVCB_##_devcb);

#define MCFG_MC68340_ADD_CRYSTAL(_crystal) \
	downcast<m68340_cpu_device &>(*device).set_crystal(_crystal);

#define MCFG_MC68340_TOUT1_OUTPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_tout1_out_callback(DEVCB_##_devcb);

#define MCFG_MC68340_TIN1_INPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_tin1_out_callback(DEVCB_##_devcb);

#define MCFG_MC68340_TGATE1_INPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_tgate1_out_callback(DEVCB_##_devcb);

#define MCFG_MC68340_TOUT2_OUTPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_tout2_out_callback(DEVCB_##_devcb);

#define MCFG_MC68340_TIN2_INPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_tin2_out_callback(DEVCB_##_devcb);

#define MCFG_MC68340_TGATE2_INPUT_CB(_devcb) \
	downcast<m68340_cpu_device &>(*device).set_tgate2_out_callback(DEVCB_##_devcb);

class m68340_cpu_device : public fscpu32_device
{
	friend class mc68340_serial_module_device;
	friend class mc68340_timer_module_device;

public:
	m68340_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_pa_in_callback(Object &&cb){ return m_pa_in_cb.set_callback (std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_pa_out_callback(Object &&cb){ return m_pa_out_cb.set_callback (std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_pb_in_callback(Object &&cb){ return m_pb_in_cb.set_callback (std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_pb_out_callback(Object &&cb){ return m_pb_out_cb.set_callback (std::forward<Object>(cb)); }
	auto pa_in_callback() { return m_pa_in_cb.bind(); }
	auto pa_out_callback() { return m_pa_out_cb.bind(); }
	auto pb_in_callback() { return m_pb_in_cb.bind(); }
	auto pb_out_callback() { return m_pb_out_cb.bind(); }

	template <class Object> devcb_base &set_tout1_out_callback(Object &&cb){ return m_timer1->m_tout_out_cb.set_callback (std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_tin1_in_callback(Object &&cb)  { return m_timer1->m_tin_in_cb.set_callback (std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_tgate1_in_callback(Object &&cb){ return m_timer1->m_tgate_in_cb.set_callback (std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_tout2_out_callback(Object &&cb){ return m_timer2->m_tout_out_cb.set_callback (std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_tin2_in_callback(Object &&cb)  { return m_timer2->m_tin_in_cb.set_callback (std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_tgate2_in_callback(Object &&cb){ return m_timer2->m_tgate_in_cb.set_callback (std::forward<Object>(cb)); }
	auto tout1_out_callback() { return m_timer1->m_tout_out_cb.bind(); }
	auto tin1_in_callback() { return m_timer1->m_tin_in_cb.bind(); }
	auto tgate1_in_callback() { return m_timer1->m_tgate_in_cb.bind(); }
	auto tout2_out_callback() { return m_timer2->m_tout_out_cb.bind(); }
	auto tin2_in_callback() { return m_timer2->m_tin_in_cb.bind(); }
	auto tgate2_in_callback() { return m_timer2->m_tgate_in_cb.bind(); }

	uint16_t get_cs(offs_t address);

	void set_crystal(const XTAL &crystal) { set_crystal(crystal.value()); }

	// Timer input methods, can be used instead of the corresponding polling MCFG callbacks
	DECLARE_WRITE_LINE_MEMBER( tin1_w )  { m_timer1->tin_w(state);  }
	DECLARE_WRITE_LINE_MEMBER( tgate1_w ){ m_timer1->tgate_w(state); }
	DECLARE_WRITE_LINE_MEMBER( tin2_w )  { m_timer2->tin_w(state);  }
	DECLARE_WRITE_LINE_MEMBER( tgate2_w ){ m_timer2->tgate_w(state); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<mc68340_serial_module_device> m_serial;
	required_device<mc68340_timer_module_device> m_timer1;
	required_device<mc68340_timer_module_device> m_timer2;

	TIMER_CALLBACK_MEMBER(periodic_interrupt_timer_callback);

	void start_68340_sim();
	void do_pit_irq();
	void do_tick_pit();

	int calc_cs(offs_t address) const;
	int get_timer_index(mc68340_timer_module_device *timer) { return (timer == m_timer1) ? 0 : 1; }

	int m_currentcs;
	uint32_t m_clock_mode;
	uint32_t m_modck;
	uint32_t m_crystal;
	uint32_t m_extal;

	// TODO: Support Limp mode and external clock with no PLL
	void set_crystal(int crystal)
	{
		m_crystal = crystal;
		m_clock_mode |= (m68340_sim::CLOCK_MODCK | m68340_sim::CLOCK_PLL);
	}

	READ32_MEMBER( m68340_internal_base_r );
	WRITE32_MEMBER( m68340_internal_base_w );
	READ32_MEMBER( m68340_internal_dma_r );
	WRITE32_MEMBER( m68340_internal_dma_w );
	READ16_MEMBER( m68340_internal_sim_r );
	READ8_MEMBER( m68340_internal_sim_ports_r );
	READ32_MEMBER( m68340_internal_sim_cs_r );
	WRITE16_MEMBER( m68340_internal_sim_w );
	WRITE8_MEMBER( m68340_internal_sim_ports_w );
	WRITE32_MEMBER( m68340_internal_sim_cs_w );

	// Clock/VCO setting TODO: support external clock with PLL and Limp mode
	DECLARE_WRITE_LINE_MEMBER( set_modck );
	DECLARE_WRITE_LINE_MEMBER( extal_w );

	void m68340_internal_map(address_map &map);

	/* 68340 peripheral modules */
	m68340_sim*    m_m68340SIM;
	m68340_dma*    m_m68340DMA;

	uint32_t m_m68340_base;

	emu_timer *m_irq_timer;

	devcb_write8        m_pa_out_cb;
	devcb_read8         m_pa_in_cb;
	devcb_write8        m_pb_out_cb;
	devcb_read8         m_pb_in_cb;
};

DECLARE_DEVICE_TYPE(M68340, m68340_cpu_device)

#endif // MAME_MACHINE_68340_H
