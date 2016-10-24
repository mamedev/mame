// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Intel 80130 iRMX Operating System Processor emulation

**********************************************************************/

#pragma once

#ifndef __I80130__
#define __I80130__

#include "emu.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"



///*************************************************************************
//  INTERFACE CONFIGURATION MACROS
///*************************************************************************

#define MCFG_I80130_IRQ_CALLBACK(_write) \
	devcb = &i80130_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_I80130_ACK_CALLBACK(_write) \
	devcb = &i80130_device::set_ack_wr_callback(*device, DEVCB_##_write);

#define MCFG_I80130_LIR_CALLBACK(_write) \
	devcb = &i80130_device::set_lir_wr_callback(*device, DEVCB_##_write);

#define MCFG_I80130_SYSTICK_CALLBACK(_write) \
	devcb = &i80130_device::set_systick_wr_callback(*device, DEVCB_##_write);

#define MCFG_I80130_DELAY_CALLBACK(_write) \
	devcb = &i80130_device::set_delay_wr_callback(*device, DEVCB_##_write);

#define MCFG_I80130_BAUD_CALLBACK(_write) \
	devcb = &i80130_device::set_baud_wr_callback(*device, DEVCB_##_write);



///*************************************************************************
//  TYPE DEFINITIONS
///*************************************************************************

// ======================> i80130_device

class i80130_device :  public device_t
{
public:
	// construction/destruction
	i80130_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<i80130_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_ack_wr_callback(device_t &device, _Object object) { return downcast<i80130_device &>(device).m_write_ack.set_callback(object); }
	template<class _Object> static devcb_base &set_lir_wr_callback(device_t &device, _Object object) { return downcast<i80130_device &>(device).m_write_lir.set_callback(object); }
	template<class _Object> static devcb_base &set_systick_wr_callback(device_t &device, _Object object) { return downcast<i80130_device &>(device).m_write_systick.set_callback(object); }
	template<class _Object> static devcb_base &set_delay_wr_callback(device_t &device, _Object object) { return downcast<i80130_device &>(device).m_write_delay.set_callback(object); }
	template<class _Object> static devcb_base &set_baud_wr_callback(device_t &device, _Object object) { return downcast<i80130_device &>(device).m_write_baud.set_callback(object); }

	virtual DECLARE_ADDRESS_MAP(rom_map, 16);
	virtual DECLARE_ADDRESS_MAP(io_map, 16);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	uint8_t inta_r() { return m_pic->acknowledge(); }

	void ir0_w(int state) { m_pic->ir0_w(state); }
	void ir1_w(int state) { m_pic->ir1_w(state); }
	void ir2_w(int state) { m_pic->ir2_w(state); }
	void ir3_w(int state) { m_pic->ir3_w(state); }
	void ir4_w(int state) { m_pic->ir4_w(state); }
	void ir5_w(int state) { m_pic->ir5_w(state); }
	void ir6_w(int state) { m_pic->ir6_w(state); }
	void ir7_w(int state) { m_pic->ir7_w(state); }

	void irq_w(int state) { m_write_irq(state); }
	void systick_w(int state) { m_write_systick(state); }
	void delay_w(int state) { m_write_delay(state); }
	void baud_w(int state) { m_write_baud(state); }

	uint16_t io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void io_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<pic8259_device> m_pic;
	required_device<pit8254_device> m_pit;

	devcb_write_line m_write_irq;
	devcb_write_line m_write_ack;
	devcb_write_line m_write_lir;
	devcb_write_line m_write_systick;
	devcb_write_line m_write_delay;
	devcb_write_line m_write_baud;
};


// device type definition
extern const device_type I80130;



#endif
