// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/******************************************************************************

    Heath H19 Terminal(Serial interface)

******************************************************************************/

#include "emu.h"
#include "heath_h19.h"

#include "bus/heathzenith/h19/tlb.h"


namespace {

class serial_heath_h19_device : public device_t, public device_rs232_port_interface
{
public:
	serial_heath_h19_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
		: device_t(mconfig, SERIAL_TERMINAL_H19, tag, owner, clock)
		, device_rs232_port_interface(mconfig, *this)
		, m_tlbc(*this, "tlbc")
	{
	}

	virtual void input_txd(int state) override { m_tlbc->serial_in_w(state); }
	virtual void input_rts(int state) override { m_tlbc->cts_in_w(state); }
	virtual void input_dtr(int state) override { m_tlbc->dsr_in_w(state); }

protected:

	virtual void device_start() override ATTR_COLD { }
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<heath_tlb_connector> m_tlbc;

	static void tlb_options(device_slot_interface &device);
};

void serial_heath_h19_device::tlb_options(device_slot_interface &device)
{
	device.option_add("heath",      HEATH_TLB);
	device.option_add("gp19",       HEATH_GP19);
	device.option_add("imaginator", HEATH_IMAGINATOR);
	device.option_add("super19",    HEATH_SUPER19);
	device.option_add("superset",   HEATH_SUPERSET);
	device.option_add("ultrarom",   HEATH_ULTRA);
	device.option_add("watzman",    HEATH_WATZ);
}

void serial_heath_h19_device::device_add_mconfig(machine_config &config)
{
	HEATH_TLB_CONNECTOR(config, m_tlbc, tlb_options, "heath");
	m_tlbc->serial_data_callback().set(FUNC(serial_heath_h19_device::output_rxd));
	m_tlbc->rts_callback().set(FUNC(serial_heath_h19_device::output_cts));
	m_tlbc->dtr_callback().set(FUNC(serial_heath_h19_device::input_dtr));
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SERIAL_TERMINAL_H19, device_rs232_port_interface, serial_heath_h19_device, "serial_heath_h19", "Heath H19 Terminal (Serial Port)")
