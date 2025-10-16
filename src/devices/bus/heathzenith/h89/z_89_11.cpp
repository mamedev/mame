// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heath/Zenith Z-89-11    Multi-Mode Interface Card

****************************************************************************/

#include "emu.h"

#include "z_89_11.h"

#include "bus/rs232/rs232.h"
#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/scn_pci.h"

namespace {

class z_89_11_device : public device_t, public device_h89bus_right_card_interface
{
public:

	z_89_11_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = XTAL(1'843'200).value());

protected:

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	void update_intr(u8 level);

	void aux_int(int data);

	required_device<i8255_device>    m_lp;
	required_device<ins8250_device>  m_aux;
	required_device<scn2661c_device> m_modem;
	required_ioport                  m_cfg_lp;
	required_ioport                  m_cfg_aux;
	required_ioport                  m_cfg_modem;

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

};


z_89_11_device::z_89_11_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, H89BUS_Z_89_11, tag, owner, clock)
	, device_h89bus_right_card_interface(mconfig, *this)
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


void z_89_11_device::update_intr(u8 level)
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

void z_89_11_device::aux_int(int data)
{
	if (!m_aux_enabled)
	{
		return;
	}

	m_aux_intr = data;

	update_intr(m_aux_int_idx);
}

void z_89_11_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
	save_item(NAME(m_lp_intr));
	save_item(NAME(m_aux_intr));
	save_item(NAME(m_modem_intr));
}

void z_89_11_device::device_reset()
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
				read8sm_delegate(m_lp, FUNC(i8255_device::read)),
				write8sm_delegate(m_lp, FUNC(i8255_device::write)));
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
					read8sm_delegate(m_modem, FUNC(scn_pci_device::read)),
					write8sm_delegate(m_modem, FUNC(scn_pci_device::write)));
			}
		}

		m_installed = true;
	}
}

void z_89_11_device::device_add_mconfig(machine_config &config)
{
	// Parallel 0xD0-0xD7 (320 - 327 octal)
	I8255(config, m_lp, m_clock);

	// AUX DCE 0xE0-0xE7 (340 - 347 octal)
	//
	INS8250(config, m_aux, XTAL(1'843'200));
	m_aux->out_tx_callback().set("dce2", FUNC(rs232_port_device::write_txd));
	m_aux->out_dtr_callback().set("dce2", FUNC(rs232_port_device::write_dtr));
	m_aux->out_rts_callback().set("dce2", FUNC(rs232_port_device::write_rts));
	m_aux->out_int_callback().set(FUNC(z_89_11_device::aux_int));

	rs232_port_device &dce2(RS232_PORT(config, "dce2", default_rs232_devices, "loopback"));
	dce2.rxd_handler().set(m_aux, FUNC(ins8250_device::rx_w));
	dce2.dcd_handler().set(m_aux, FUNC(ins8250_device::dcd_w));
	dce2.dsr_handler().set(m_aux, FUNC(ins8250_device::dsr_w));
	dce2.cts_handler().set(m_aux, FUNC(ins8250_device::cts_w));
	dce2.ri_handler().set(m_aux, FUNC(ins8250_device::ri_w));

	// Modem DTE 0xD7-0xDF (330 - 337 octal)
	SCN2661C(config, m_modem, XTAL(5'068'800));
	m_modem->txd_handler().set("dte", FUNC(rs232_port_device::write_txd));
	m_modem->dtr_handler().set("dte", FUNC(rs232_port_device::write_dtr));
	m_modem->rts_handler().set("dte", FUNC(rs232_port_device::write_rts));


	rs232_port_device &dte(RS232_PORT(config, "dte", default_rs232_devices, "loopback"));
	dte.rxd_handler().set(m_modem, FUNC(scn2661c_device::rxd_w));
	dte.dcd_handler().set(m_modem, FUNC(scn2661c_device::dcd_w));
	dte.dsr_handler().set(m_modem, FUNC(scn2661c_device::dsr_w));
	dte.cts_handler().set(m_modem, FUNC(scn2661c_device::cts_w));
}


static INPUT_PORTS_START( z_89_11_device )

	// INS8250
	PORT_START("CFG_LP")
	PORT_CONFNAME(0x01, 0x01, "LP - DCE (340 octal) Enabled" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x06, 0x00, "LP - DCE (340 octal) Interrupt level")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x02, "3")
	PORT_CONFSETTING(   0x04, "4")
	PORT_CONFSETTING(   0x06, "5")

	// 8255 chip
	PORT_START("CFG_AUX")
	PORT_CONFNAME(0x01, 0x01, "AUX - DCE (320 octal) Enabled" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x06, 0x00, "AUX - DCE (320 octal) Interrupt level")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x02, "3")
	PORT_CONFSETTING(   0x04, "4")
	PORT_CONFSETTING(   0x06, "5")

	// 2661
	PORT_START("CFG_MODEM")
	PORT_CONFNAME(0x01, 0x01, "MODEM - DTE (330 octal) Enabled" )
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x01, DEF_STR( Yes ))
	PORT_CONFNAME(0x06, 0x00, "MODEM - DTE (330 octal) Interrupt level")
	PORT_CONFSETTING(   0x00, DEF_STR( None ))
	PORT_CONFSETTING(   0x02, "3")
	PORT_CONFSETTING(   0x04, "4")
	PORT_CONFSETTING(   0x06, "5")
	PORT_CONFNAME(0x08, 0x00, "Interrupt on Rx Ready Enabled")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x08, DEF_STR( Yes ))
	PORT_CONFNAME(0x10, 0x00, "Interrupt on Tx Ready Enabled")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x10, DEF_STR( Yes ))
	PORT_CONFNAME(0x20, 0x00, "Interrupt on Tx Empty Enabled")
	PORT_CONFSETTING(   0x00, DEF_STR( No ))
	PORT_CONFSETTING(   0x20, DEF_STR( Yes ))

INPUT_PORTS_END


ioport_constructor z_89_11_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(z_89_11_device);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(H89BUS_Z_89_11, device_h89bus_right_card_interface, z_89_11_device, "z_89_11", "Heath/Zenith Z-89-11 Multi-Function I/O Card");
