// license:BSD-3-Clause
// copyright-holders:rfka01
/**********************************************************************

    HP LaserJet IIIP on an RS-232 port

    The printer itself is machine/hp_lj3p.cpp; select I/O SERIAL and the
    computer's baud rate in its control panel (Menu, DEVICE CONFIG).  The
    layout puts the computer's first screen next to the paper, with the
    control panel under the paper.

**********************************************************************/

#include "emu.h"
#include "hp_lj3p.h"

#include "machine/hp_lj3p.h"


namespace {

class rs232_hp_lj3p_device : public device_t, public device_rs232_port_interface
{
public:
	rs232_hp_lj3p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
		device_t(mconfig, RS232_HP_LJ3P, tag, owner, clock),
		device_rs232_port_interface(mconfig, *this),
		m_printer(*this, "printer")
	{
	}

	// data from the computer arrives on the printer's RxD
	virtual void input_txd(int state) override { m_printer->rxd_w(state); }

protected:
	virtual void device_start() override ATTR_COLD { }

	virtual void device_reset() override ATTR_COLD
	{
		output_rxd(1);
		output_dcd(0);
	}

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD
	{
		HP_LJ3P(config, m_printer);
		m_printer->txd_cb().set(FUNC(rs232_hp_lj3p_device::output_rxd));
		// the printer's DTR (pin 20) goes to the computer as DSR and CTS
		m_printer->dtr_cb().set(FUNC(rs232_hp_lj3p_device::output_dsr));
		m_printer->dtr_cb().append(FUNC(rs232_hp_lj3p_device::output_cts));

		hp_lj3p_device::host_layout(config);
	}

private:
	required_device<hp_lj3p_device> m_printer;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(RS232_HP_LJ3P, device_rs232_port_interface, rs232_hp_lj3p_device, "rs232_hplj3p", "HP LaserJet IIIP (RS-232)")
