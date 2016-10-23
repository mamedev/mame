// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

    Fujitsu MB89363 Parallel Communication Interface
    (this acts as a trampoline to 2x i8255 chips)

***************************************************************************/

#pragma once

#ifndef __MB89363B__
#define __MB89363B__


#include "machine/i8255.h"


extern const device_type MB89363B;

#define MCFG_MB89363B_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MB89363B, 0)



#define MCFG_MB89363B_IN_PORTA_CB(_devcb) \
	devcb = &mb89363b_device::set_in_a_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_IN_PORTB_CB(_devcb) \
	devcb = &mb89363b_device::set_in_a_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_IN_PORTC_CB(_devcb) \
	devcb = &mb89363b_device::set_in_a_pc_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_OUT_PORTA_CB(_devcb) \
	devcb = &mb89363b_device::set_out_a_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_OUT_PORTB_CB(_devcb) \
	devcb = &mb89363b_device::set_out_a_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_OUT_PORTC_CB(_devcb) \
	devcb = &mb89363b_device::set_out_a_pc_callback(*device, DEVCB_##_devcb);


#define MCFG_MB89363B_IN_PORTD_CB(_devcb) \
	devcb = &mb89363b_device::set_in_b_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_IN_PORTE_CB(_devcb) \
	devcb = &mb89363b_device::set_in_b_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_IN_PORTF_CB(_devcb) \
	devcb = &mb89363b_device::set_in_b_pc_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_OUT_PORTD_CB(_devcb) \
	devcb = &mb89363b_device::set_out_b_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_OUT_PORTE_CB(_devcb) \
	devcb = &mb89363b_device::set_out_b_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_MB89363B_OUT_PORTF_CB(_devcb) \
	devcb = &mb89363b_device::set_out_b_pc_callback(*device, DEVCB_##_devcb);



class mb89363b_device :  public device_t
{
public:
	// construction/destruction
	mb89363b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t i8255_a_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t i8255_a_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t i8255_a_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8255_a_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8255_a_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8255_a_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t i8255_b_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t i8255_b_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t i8255_b_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void i8255_b_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8255_b_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void i8255_b_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);


	template<class _Object> static devcb_base &set_in_a_pa_callback(device_t &device, _Object object)  { return downcast<mb89363b_device &>(device).m_in_a_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_a_pb_callback(device_t &device, _Object object)  { return downcast<mb89363b_device &>(device).m_in_a_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_a_pc_callback(device_t &device, _Object object)  { return downcast<mb89363b_device &>(device).m_in_a_pc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_a_pa_callback(device_t &device, _Object object) { return downcast<mb89363b_device &>(device).m_out_a_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_a_pb_callback(device_t &device, _Object object) { return downcast<mb89363b_device &>(device).m_out_a_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_a_pc_callback(device_t &device, _Object object) { return downcast<mb89363b_device &>(device).m_out_a_pc_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_in_b_pa_callback(device_t &device, _Object object)  { return downcast<mb89363b_device &>(device).m_in_b_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_b_pb_callback(device_t &device, _Object object)  { return downcast<mb89363b_device &>(device).m_in_b_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_b_pc_callback(device_t &device, _Object object)  { return downcast<mb89363b_device &>(device).m_in_b_pc_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_b_pa_callback(device_t &device, _Object object) { return downcast<mb89363b_device &>(device).m_out_b_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_b_pb_callback(device_t &device, _Object object) { return downcast<mb89363b_device &>(device).m_out_b_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_b_pc_callback(device_t &device, _Object object) { return downcast<mb89363b_device &>(device).m_out_b_pc_cb.set_callback(object); }


	required_device<i8255_device> m_i8255_a;
	required_device<i8255_device> m_i8255_b;

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;



private:

	devcb_read8        m_in_a_pa_cb;
	devcb_read8        m_in_a_pb_cb;
	devcb_read8        m_in_a_pc_cb;

	devcb_write8       m_out_a_pa_cb;
	devcb_write8       m_out_a_pb_cb;
	devcb_write8       m_out_a_pc_cb;

	devcb_read8        m_in_b_pa_cb;
	devcb_read8        m_in_b_pb_cb;
	devcb_read8        m_in_b_pc_cb;

	devcb_write8       m_out_b_pa_cb;
	devcb_write8       m_out_b_pb_cb;
	devcb_write8       m_out_b_pc_cb;


};

#endif
