// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit H19
  Zenith Data Systems Z-19

    A smart terminal designed and manufactured by Heath Company. This
    is identical to the Zenith Data Systems Z-19.

****************************************************************************/

#include "emu.h"

#include "tlb.h"
#include "bus/rs232/rs232.h"

#include "h19.lh"


namespace {

class h19_state : public driver_device
{
public:
	h19_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tlbc(*this, "tlbc")
	{
	}

	void h19(machine_config &config);

private:
	required_device<heath_tlb_connector> m_tlbc;

};

static void tlb_options(device_slot_interface &device)
{
	device.option_add("heath",      HEATH_TLB);
	device.option_add("gp19",       HEATH_GP19);
	device.option_add("imaginator", HEATH_IMAGINATOR);
	device.option_add("super19",    HEATH_SUPER19);
	device.option_add("superset",   HEATH_SUPERSET);
	device.option_add("ultrarom",   HEATH_ULTRA);
	device.option_add("watzman",    HEATH_WATZ);
}

void h19_state::h19(machine_config &config)
{
	config.set_default_layout(layout_h19);

	HEATH_TLB_CONNECTOR(config, m_tlbc, tlb_options, "heath");
	m_tlbc->serial_data_callback().set("dte", FUNC(rs232_port_device::write_txd));
	m_tlbc->dtr_callback().set("dte", FUNC(rs232_port_device::write_dtr));
	m_tlbc->rts_callback().set("dte", FUNC(rs232_port_device::write_rts));

	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(m_tlbc, FUNC(heath_tlb_connector::serial_in_w));
	dte.dcd_handler().set(m_tlbc, FUNC(heath_tlb_connector::rlsd_in_w));
	dte.dsr_handler().set(m_tlbc, FUNC(heath_tlb_connector::dsr_in_w));
	dte.cts_handler().set(m_tlbc, FUNC(heath_tlb_connector::cts_in_w));
}

// ROM definition
ROM_START( h19 )
ROM_END

} // anonymous namespace

//    year  name  parent  compat  machine input  class       init         company                fullname          flags
COMP( 1979, h19,  0,      0,      h19,    0,     h19_state,  empty_init,  "Heath Company",       "H-19 Terminal",  MACHINE_SUPPORTS_SAVE )
