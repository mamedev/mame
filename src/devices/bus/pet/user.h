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

class pet_user_port_device : public device_t, public device_slot_interface
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
	virtual void device_start() override ATTR_COLD;

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


class device_pet_user_port_interface : public device_interface
{
	friend class pet_user_port_device;

public:
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
	device_pet_user_port_interface(const machine_config &mconfig, device_t &device);

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


void pet_user_port_cards(device_slot_interface &device);

#endif // MAME_BUS_PET_USER_H
