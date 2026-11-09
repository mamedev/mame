// license:BSD-3-Clause
// copyright-holders:rfka01
/**********************************************************************

    HP LaserJet IIIP on a Centronics port

    The printer itself is machine/hp_lj3p.cpp; select I/O PARALEL in its
    control panel (the default).  The layout puts the computer's first
    screen next to the paper, with the control panel under the paper.

**********************************************************************/

#include "emu.h"
#include "hp_lj3p.h"

#include "machine/hp_lj3p.h"


namespace {

class centronics_hp_lj3p_device : public device_t, public device_centronics_peripheral_interface
{
public:
	centronics_hp_lj3p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		device_t(mconfig, CENTRONICS_HP_LJ3P, tag, owner, clock),
		device_centronics_peripheral_interface(mconfig, *this),
		m_printer(*this, "printer"),
		m_data(0)
	{
	}

	virtual void input_strobe(int state) override { m_printer->strobe_w(state); }
	virtual void input_init(int state) override { m_printer->init_w(state); }
	virtual void input_data0(int state) override { data_bit(0, state); }
	virtual void input_data1(int state) override { data_bit(1, state); }
	virtual void input_data2(int state) override { data_bit(2, state); }
	virtual void input_data3(int state) override { data_bit(3, state); }
	virtual void input_data4(int state) override { data_bit(4, state); }
	virtual void input_data5(int state) override { data_bit(5, state); }
	virtual void input_data6(int state) override { data_bit(6, state); }
	virtual void input_data7(int state) override { data_bit(7, state); }

protected:
	virtual void device_start() override ATTR_COLD
	{
		save_item(NAME(m_data));
	}

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		HP_LJ3P(config, m_printer);
		m_printer->busy_cb().set(FUNC(centronics_hp_lj3p_device::output_busy));
		m_printer->ack_cb().set(FUNC(centronics_hp_lj3p_device::output_ack));
		m_printer->perror_cb().set(FUNC(centronics_hp_lj3p_device::output_perror));
		m_printer->fault_cb().set(FUNC(centronics_hp_lj3p_device::output_fault));
		m_printer->select_cb().set(FUNC(centronics_hp_lj3p_device::output_select));

		hp_lj3p_device::host_layout(config);
	}

	virtual bool supports_pin35_5v() override { return true; }

private:
	required_device<hp_lj3p_device> m_printer;
	u8 m_data;

	void data_bit(int bit, int state)
	{
		if (state)
			m_data |= 1 << bit;
		else
			m_data &= ~(1 << bit);
		m_printer->data_w(m_data);
	}
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(CENTRONICS_HP_LJ3P, device_centronics_peripheral_interface, centronics_hp_lj3p_device, "centronics_hplj3p", "HP LaserJet IIIP (Centronics)")
