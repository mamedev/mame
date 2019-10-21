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

#ifndef MAME_BUS_PET_USER_H
#define MAME_BUS_PET_USER_H

#pragma once


DECLARE_DEVICE_TYPE(PET_USER_PORT, pet_user_port_device)

class device_pet_user_port_interface;

class pet_user_port_device : public device_t,
	public device_slot_interface
{
	friend class device_pet_user_port_interface;

public:
	template <typename T>
	pet_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: pet_user_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	pet_user_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto p2_handler() { return m_2_handler.bind(); }
	auto p3_handler() { return m_3_handler.bind(); }
	auto p4_handler() { return m_4_handler.bind(); }
	auto p5_handler() { return m_5_handler.bind(); }
	auto p6_handler() { return m_6_handler.bind(); }
	auto p7_handler() { return m_7_handler.bind(); }
	auto p8_handler() { return m_8_handler.bind(); }
	auto p9_handler() { return m_9_handler.bind(); }
	auto p10_handler() { return m_10_handler.bind(); }
	auto pb_handler() { return m_b_handler.bind(); }
	auto pc_handler() { return m_c_handler.bind(); }
	auto pd_handler() { return m_d_handler.bind(); }
	auto pe_handler() { return m_e_handler.bind(); }
	auto pf_handler() { return m_f_handler.bind(); }
	auto ph_handler() { return m_h_handler.bind(); }
	auto pj_handler() { return m_j_handler.bind(); }
	auto pk_handler() { return m_k_handler.bind(); }
	auto pl_handler() { return m_l_handler.bind(); }
	auto pm_handler() { return m_m_handler.bind(); }

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
	device_pet_user_port_interface(const machine_config &mconfig, device_t &device);

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


void pet_user_port_cards(device_slot_interface &device);

#endif // MAME_BUS_PET_USER_H
