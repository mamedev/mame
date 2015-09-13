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
	pet_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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

	DECLARE_WRITE_LINE_MEMBER( write_2 );
	DECLARE_WRITE_LINE_MEMBER( write_3 );
	DECLARE_WRITE_LINE_MEMBER( write_4 );
	DECLARE_WRITE_LINE_MEMBER( write_5 );
	DECLARE_WRITE_LINE_MEMBER( write_6 );
	DECLARE_WRITE_LINE_MEMBER( write_7 );
	DECLARE_WRITE_LINE_MEMBER( write_8 );
	DECLARE_WRITE_LINE_MEMBER( write_9 );
	DECLARE_WRITE_LINE_MEMBER( write_10 );
	DECLARE_WRITE_LINE_MEMBER( write_b );
	DECLARE_WRITE_LINE_MEMBER( write_c );
	DECLARE_WRITE_LINE_MEMBER( write_d );
	DECLARE_WRITE_LINE_MEMBER( write_e );
	DECLARE_WRITE_LINE_MEMBER( write_f );
	DECLARE_WRITE_LINE_MEMBER( write_h );
	DECLARE_WRITE_LINE_MEMBER( write_j );
	DECLARE_WRITE_LINE_MEMBER( write_k );
	DECLARE_WRITE_LINE_MEMBER( write_l );
	DECLARE_WRITE_LINE_MEMBER( write_m );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

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

	DECLARE_WRITE_LINE_MEMBER( output_2 ) { m_slot->m_2_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_3 ) { m_slot->m_3_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_4 ) { m_slot->m_4_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_5 ) { m_slot->m_5_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_6 ) { m_slot->m_6_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_7 ) { m_slot->m_7_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_8 ) { m_slot->m_8_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_9 ) { m_slot->m_9_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_10 ) { m_slot->m_10_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_b ) { m_slot->m_b_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_c ) { m_slot->m_c_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_d ) { m_slot->m_d_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_e ) { m_slot->m_e_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_f ) { m_slot->m_f_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_h ) { m_slot->m_h_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_j ) { m_slot->m_j_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_k ) { m_slot->m_k_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_l ) { m_slot->m_l_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( output_m ) { m_slot->m_m_handler(state); }

protected:
	virtual DECLARE_WRITE_LINE_MEMBER( input_2 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_3 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_4 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_5 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_6 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_7 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_8 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_9 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_10 ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_b ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_c ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_d ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_e ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_f ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_h ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_j ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_k ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_l ) {}
	virtual DECLARE_WRITE_LINE_MEMBER( input_m ) {}

	pet_user_port_device *m_slot;
};


SLOT_INTERFACE_EXTERN( pet_user_port_cards );

#endif
