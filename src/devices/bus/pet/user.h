// license:BSD-3-Clause
// copyright-holders:smf
/**********************************************************************

    Commodore PET User Port emulation

**********************************************************************

                    GND       1      A       GND
                  VIDEO       2      B       CA1
                _SRQ IN       3      C       PA0
                    EOI       4      D       PA1
                   DIAG       5      E       PA2
           #2 CASS READ       6      F       PA3
             CASS WRITE       7      H       PA4
           #1 CASS READ       8      J       PA5
             VERT DRIVE       9      K       PA6
             HORZ DRIVE      10      L       PA7
                    GND      11      M       CB2
                    GND      12      N       GND

**********************************************************************/

#pragma once

#ifndef __PET_USER_PORT__
#define __PET_USER_PORT__

#include "emu.h"


#define MCFG_PET_USER_PORT_ADD(_tag, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PET_USER_PORT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_PET_USER_PORT_2_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_2_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_3_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_3_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_4_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_4_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_5_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_5_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_6_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_6_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_7_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_7_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_8_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_8_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_9_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_9_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_10_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_10_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_B_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_b_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_C_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_c_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_D_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_d_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_E_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_e_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_F_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_f_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_H_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_h_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_J_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_j_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_K_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_k_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_L_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_l_handler(*device, DEVCB_##_devcb);

#define MCFG_PET_USER_PORT_M_HANDLER(_devcb) \
	devcb = &pet_user_port_device::set_m_handler(*device, DEVCB_##_devcb);


extern const device_type PET_USER_PORT;

class device_pet_user_port_interface;

class pet_user_port_device : public device_t,
	public device_slot_interface
{
	friend class device_pet_user_port_interface;

public:
	pet_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &set_2_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_2_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_3_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_3_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_4_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_4_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_5_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_5_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_6_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_6_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_7_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_7_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_8_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_8_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_9_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_9_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_10_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_10_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_b_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_b_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_c_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_c_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_d_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_d_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_e_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_e_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_f_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_f_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_h_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_h_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_j_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_j_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_k_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_k_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_l_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_l_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_m_handler(device_t &device, _Object object) { return downcast<pet_user_port_device &>(device).m_m_handler.set_callback(object); }

	void write_2(int state);
	void write_3(int state);
	void write_4(int state);
	void write_5(int state);
	void write_6(int state);
	void write_7(int state);
	void write_8(int state);
	void write_9(int state);
	void write_10(int state);
	void write_b(int state);
	void write_c(int state);
	void write_d(int state);
	void write_e(int state);
	void write_f(int state);
	void write_h(int state);
	void write_j(int state);
	void write_k(int state);
	void write_l(int state);
	void write_m(int state);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	devcb_write_line m_2_handler;
	devcb_write_line m_3_handler;
	devcb_write_line m_4_handler;
	devcb_write_line m_5_handler;
	devcb_write_line m_6_handler;
	devcb_write_line m_7_handler;
	devcb_write_line m_8_handler;
	devcb_write_line m_9_handler;
	devcb_write_line m_10_handler;
	devcb_write_line m_b_handler;
	devcb_write_line m_c_handler;
	devcb_write_line m_d_handler;
	devcb_write_line m_e_handler;
	devcb_write_line m_f_handler;
	devcb_write_line m_h_handler;
	devcb_write_line m_j_handler;
	devcb_write_line m_k_handler;
	devcb_write_line m_l_handler;
	devcb_write_line m_m_handler;

private:
	device_pet_user_port_interface *m_card;
};


class device_pet_user_port_interface : public device_slot_card_interface
{
	friend class pet_user_port_device;

public:
	device_pet_user_port_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_pet_user_port_interface();

	void output_2(int state) { m_slot->m_2_handler(state); }
	void output_3(int state) { m_slot->m_3_handler(state); }
	void output_4(int state) { m_slot->m_4_handler(state); }
	void output_5(int state) { m_slot->m_5_handler(state); }
	void output_6(int state) { m_slot->m_6_handler(state); }
	void output_7(int state) { m_slot->m_7_handler(state); }
	void output_8(int state) { m_slot->m_8_handler(state); }
	void output_9(int state) { m_slot->m_9_handler(state); }
	void output_10(int state) { m_slot->m_10_handler(state); }
	void output_b(int state) { m_slot->m_b_handler(state); }
	void output_c(int state) { m_slot->m_c_handler(state); }
	void output_d(int state) { m_slot->m_d_handler(state); }
	void output_e(int state) { m_slot->m_e_handler(state); }
	void output_f(int state) { m_slot->m_f_handler(state); }
	void output_h(int state) { m_slot->m_h_handler(state); }
	void output_j(int state) { m_slot->m_j_handler(state); }
	void output_k(int state) { m_slot->m_k_handler(state); }
	void output_l(int state) { m_slot->m_l_handler(state); }
	void output_m(int state) { m_slot->m_m_handler(state); }

protected:
	virtual void input_2(int state) {}
	virtual void input_3(int state) {}
	virtual void input_4(int state) {}
	virtual void input_5(int state) {}
	virtual void input_6(int state) {}
	virtual void input_7(int state) {}
	virtual void input_8(int state) {}
	virtual void input_9(int state) {}
	virtual void input_10(int state) {}
	virtual void input_b(int state) {}
	virtual void input_c(int state) {}
	virtual void input_d(int state) {}
	virtual void input_e(int state) {}
	virtual void input_f(int state) {}
	virtual void input_h(int state) {}
	virtual void input_j(int state) {}
	virtual void input_k(int state) {}
	virtual void input_l(int state) {}
	virtual void input_m(int state) {}

	pet_user_port_device *m_slot;
};


SLOT_INTERFACE_EXTERN( pet_user_port_cards );

#endif
