// license:BSD-3-Clause
// copyright-holders:Mark Garlanger, R. Belmont
/***************************************************************************

  Heath H-88-3 3-port serial port card
    Came with 2 serial ports and space to add an additional serial port.

  Heath HA-88-3 3-port serial port card
    Came standard with 3 serial ports.

****************************************************************************/

#include "emu.h"

#include "h_88_3.h"

#include "bus/rs232/rs232.h"
#include "machine/ins8250.h"

namespace {

class h_88_3_device : public device_t, public device_h89bus_right_card_interface
{
public:
	h_88_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = XTAL(1'843'200).value());

protected:
	h_88_3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	devcb_write_line::array<4> m_int_cb;

	required_device<ins8250_device> m_lp;
	required_device<ins8250_device> m_aux;
	required_device<ins8250_device> m_modem;
	required_ioport m_cfg_lp;
	required_ioport m_cfg_aux;
	required_ioport m_cfg_modem;

	bool m_installed;

	bool m_lp_enabled;
	bool m_aux_enabled;
	bool m_modem_enabled;

	u8 m_lp_int_idx;
	u8 m_aux_int_idx;
	u8 m_modem_int_idx;

	int m_lp_intr;
	int m_aux_intr;
	int m_modem_intr;

private:
	void update_intr(u8 level);

	void lp_int(int data);
	void aux_int(int data);
	void modem_int(int data);
};

class ha_88_3_device : public h_88_3_device
{
public:
	ha_88_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = XTAL(1'843'200).value());

protected:
	virtual ioport_constructor device_input_ports() const override;
};

h_88_3_device::h_88_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h_88_3_device(mconfig, H89BUS_H_88_3, tag, owner, clock)
{
}

h_88_3_device::h_88_3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_h89bus_right_card_interface(mconfig, *this)
	, m_int_cb(*this)
	, m_lp(*this, "lp")
	, m_aux(*this, "aux")
	, m_modem(*this, "modem")
	, m_cfg_lp(*this, "CFG_LP")
	, m_cfg_aux(*this, "CFG_AUX")
	, m_cfg_modem(*this, "CFG_MODEM")
	, m_lp_intr(0)
	, m_aux_intr(0)
	, m_modem_intr(0)
{
}

void h_88_3_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
	save_item(NAME(m_lp_intr));
	save_item(NAME(m_aux_intr));
	save_item(NAME(m_modem_intr));
}

void h_88_3_device::device_reset()
{
	ioport_value const cfg_lp(m_cfg_lp->read());
	ioport_value const cfg_aux(m_cfg_aux->read());
	ioport_value const cfg_modem(m_cfg_modem->read());

	m_lp_enabled    = bool(BIT(cfg_lp, 0));
	m_aux_enabled   = bool(BIT(cfg_aux, 0));
	m_modem_enabled = bool(BIT(cfg_modem, 0));

	// LP Interrupt level
	m_lp_int_idx = BIT(cfg_lp, 1, 2);

	// AUX Interrupt level
	m_aux_int_idx = BIT(cfg_aux, 1, 2);

	// MODEM Interrupt level
	m_modem_int_idx = BIT(cfg_modem, 1, 2);

	if (!m_installed)
	{
		if (m_lp_enabled)
		{
			h89bus::addr_ranges  addr_ranges = h89bus().get_address_ranges(h89bus::IO_LP);

			if (addr_ranges.size() == 1)
			{
				h89bus::addr_range range = addr_ranges.front();

				h89bus().install_io_device(range.first, range.second,
					read8sm_delegate(m_lp, FUNC(ins8250_uart_device::ins8250_r)),
					write8sm_delegate(m_lp, FUNC(ins8250_uart_device::ins8250_w)));
			}
		}

		if (m_aux_enabled)
		{
			h89bus::addr_ranges  addr_ranges = h89bus().get_address_ranges(h89bus::IO_SER0);

			if (addr_ranges.size() == 1)
			{
				h89bus::addr_range range = addr_ranges.front();

				h89bus().install_io_device(range.first, range.second,
					read8sm_delegate(m_aux, FUNC(ins8250_uart_device::ins8250_r)),
					write8sm_delegate(m_aux, FUNC(ins8250_uart_device::ins8250_w)));
			}
		}

		if (m_modem_enabled)
		{
			h89bus::addr_ranges  addr_ranges = h89bus().get_address_ranges(h89bus::IO_SER1);

			if (addr_ranges.size() == 1)
			{
				h89bus::addr_range range = addr_ranges.front();

				h89bus().install_io_device(range.first, range.second,
					read8sm_delegate(m_modem, FUNC(ins8250_uart_device::ins8250_r)),
					write8sm_delegate(m_modem, FUNC(ins8250_uart_device::ins8250_w)));

			}
		}

		m_installed = true;
	}
}

void h_88_3_device::update_intr(u8 level)
{
	// check for interrupt disabled
	if (level == 0)
	{
		return;
	}

	// Handle multiple ports set to the same interrupt. If any are set, interrupt should be triggered
	int data = 0;

	if (m_lp_enabled && (m_lp_int_idx == level))
	{
		data |= m_lp_intr;
	}
	if (m_aux_enabled && (m_aux_int_idx == level))
	{
		data |= m_aux_intr;
	}
	if (m_modem_enabled && (m_modem_int_idx == level))
	{
		data |= m_modem_intr;
	}

	switch (level)
	{
		case 1:
			set_slot_int3(data);
			break;
		case 2:
			set_slot_int4(data);
			break;
		case 3:
			set_slot_int5(data);
			break;
	}
}

void h_88_3_device::lp_int(int data)
{
	if (!m_lp_enabled)
	{
		return;
	}

	m_lp_intr = data;

	update_intr(m_lp_int_idx);
}

void h_88_3_device::aux_int(int data)
{
	if (!m_aux_enabled)
	{
		return;
	}

	m_aux_intr = data;

	update_intr(m_aux_int_idx);
}

void h_88_3_device::modem_int(int data)
{
	if (!m_modem_enabled)
	{
		return;
	}

	m_modem_intr = data;

	update_intr(m_modem_int_idx);
}

void h_88_3_device::device_add_mconfig(machine_config &config)
{
	// LP DCE 0xE0-0xE7 (340 - 347 octal)
	INS8250(config, m_lp, m_clock);
	m_lp->out_tx_callback().set("dce1", FUNC(rs232_port_device::write_txd));
	m_lp->out_dtr_callback().set("dce1", FUNC(rs232_port_device::write_dtr));
	m_lp->out_rts_callback().set("dce1", FUNC(rs232_port_device::write_rts));
	m_lp->out_int_callback().set(FUNC(h_88_3_device::lp_int));

	rs232_port_device &dce1(RS232_PORT(config, "dce1", default_rs232_devices, "printer"));
	dce1.rxd_handler().set(m_lp, FUNC(ins8250_device::rx_w));
	dce1.dcd_handler().set(m_lp, FUNC(ins8250_device::dcd_w));
	dce1.dsr_handler().set(m_lp, FUNC(ins8250_device::dsr_w));
	dce1.cts_handler().set(m_lp, FUNC(ins8250_device::cts_w));
	dce1.ri_handler().set(m_lp, FUNC(ins8250_device::ri_w));


	// AUX DCE 0xD0-0xD7 (320 - 327 octal)
	INS8250(config, m_aux, m_clock);
	m_aux->out_tx_callback().set("dce2", FUNC(rs232_port_device::write_txd));
	m_aux->out_dtr_callback().set("dce2", FUNC(rs232_port_device::write_dtr));
	m_aux->out_rts_callback().set("dce2", FUNC(rs232_port_device::write_rts));
	m_aux->out_int_callback().set(FUNC(h_88_3_device::aux_int));

	rs232_port_device &dce2(RS232_PORT(config, "dce2", default_rs232_devices, "loopback"));
	dce2.rxd_handler().set(m_aux, FUNC(ins8250_device::rx_w));
	dce2.dcd_handler().set(m_aux, FUNC(ins8250_device::dcd_w));
	dce2.dsr_handler().set(m_aux, FUNC(ins8250_device::dsr_w));
	dce2.cts_handler().set(m_aux, FUNC(ins8250_device::cts_w));
	dce2.ri_handler().set(m_aux, FUNC(ins8250_device::ri_w));


	// Modem DTE 0xD7-0xDF (330 - 337 octal)
	INS8250(config, m_modem, m_clock);
	m_modem->out_tx_callback().set("dte", FUNC(rs232_port_device::write_txd));
	m_modem->out_dtr_callback().set("dte", FUNC(rs232_port_device::write_dtr));
	m_modem->out_rts_callback().set("dte", FUNC(rs232_port_device::write_rts));
	m_modem->out_int_callback().set(FUNC(h_88_3_device::modem_int));

	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(m_modem, FUNC(ins8250_device::rx_w));
	dte.dcd_handler().set(m_modem, FUNC(ins8250_device::dcd_w));
	dte.dsr_handler().set(m_modem, FUNC(ins8250_device::dsr_w));
	dte.cts_handler().set(m_modem, FUNC(ins8250_device::cts_w));
	dte.ri_handler().set(m_modem, FUNC(ins8250_device::ri_w));
}


static INPUT_PORTS_START( h_88_3_device )

	PORT_START("CFG_LP")
	PORT_CONFNAME(0x01, 0x01, "LP - DCE (340 octal) Enabled" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x06, 0x00, "LP - DCE (340 octal) Interrupt level")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x02, "3")
	PORT_CONFSETTING(   0x04, "4")
	PORT_CONFSETTING(   0x06, "5")

	PORT_START("CFG_AUX")
	PORT_CONFNAME(0x01, 0x00, "AUX - DCE (320 octal) Enabled" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x06, 0x00, "AUX - DCE (320 octal) Interrupt level")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x02, "3")
	PORT_CONFSETTING(   0x04, "4")
	PORT_CONFSETTING(   0x06, "5")

	PORT_START("CFG_MODEM")
	PORT_CONFNAME(0x01, 0x01, "MODEM - DTE (330 octal) Enabled" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x06, 0x00, "MODEM - DTE (330 octal) Interrupt level")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x02, "3")
	PORT_CONFSETTING(   0x04, "4")
	PORT_CONFSETTING(   0x06, "5")

INPUT_PORTS_END

static INPUT_PORTS_START( ha_88_3_device )
	PORT_INCLUDE( h_88_3_device )

	PORT_MODIFY("CFG_AUX")
	PORT_CONFNAME(0x01, 0x01, "AUX - DCE (320 octal) Enabled" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))

INPUT_PORTS_END


ioport_constructor h_88_3_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(h_88_3_device);
}

ioport_constructor ha_88_3_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ha_88_3_device);
}

ha_88_3_device::ha_88_3_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	h_88_3_device(mconfig, H89BUS_HA_88_3, tag, owner, clock)
{
}

}   // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_H_88_3,  device_h89bus_right_card_interface, h_88_3_device,  "h89h_88_3",  "Heath H-88-3 3-port Serial Board");
DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_HA_88_3, device_h89bus_right_card_interface, ha_88_3_device, "h89ha_88_3", "Heath HA-88-3 3-port Serial Board");
