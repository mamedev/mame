// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Schneider NLQ 401 Matrix Printer (skeleton)

    This appears to be an OEM version of a Centronics Printer Corp.
    product. The hardware (but not the firmware) is also said to be
    identical with the Brother M1009.

**********************************************************************/

#include "emu.h"
#include "nlq401.h"

#include "cpu/upd7810/upd7810.h"


//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(NLQ401, nlq401_device, "nlq401", "Schneider NLQ 401 Matrix Printer")


//-------------------------------------------------
//  nlq401_device - constructor
//-------------------------------------------------

nlq401_device::nlq401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NLQ401, tag, owner, clock)
	, device_centronics_peripheral_interface(mconfig, *this)
	, m_inpexp(*this, "inpexp")
	, m_outexp(*this, "outexp")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nlq401_device::device_start()
{
}


//-------------------------------------------------
//  input_data0 - DATA1 line handler
//-------------------------------------------------

void nlq401_device::input_data0(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_data1 - DATA2 line handler
//-------------------------------------------------

void nlq401_device::input_data1(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_data2 - DATA3 line handler
//-------------------------------------------------

void nlq401_device::input_data2(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_data3 - DATA4 line handler
//-------------------------------------------------

void nlq401_device::input_data3(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_data4 - DATA5 line handler
//-------------------------------------------------

void nlq401_device::input_data4(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_data5 - DATA6 line handler
//-------------------------------------------------

void nlq401_device::input_data5(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_data6 - DATA7 line handler
//-------------------------------------------------

void nlq401_device::input_data6(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_data7 - DATA8 line handler
//-------------------------------------------------

void nlq401_device::input_data7(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_strobe - DATA STROBE line handler
//-------------------------------------------------

void nlq401_device::input_strobe(int state)
{
	// TODO
}

//-------------------------------------------------
//  input_init - INIT line handler
//-------------------------------------------------

void nlq401_device::input_init(int state)
{
	// TODO
}


//-------------------------------------------------
//  expander_w - write from MCU to expanders
//-------------------------------------------------

void nlq401_device::expander_w(u8 data)
{
	m_outexp->write_h(BIT(data, 0, 4));
	m_outexp->write_s(BIT(data, 4, 3));
	m_inpexp->write_s(BIT(data, 4, 3));
	m_outexp->write_std(BIT(data, 7));
}

//-------------------------------------------------
//  expander_r - read from expander port
//-------------------------------------------------

u8 nlq401_device::expander_r()
{
	return 0x87 | (m_inpexp->read_h() << 3);
}


//-------------------------------------------------
//  mem_map - address map for microcontroller
//-------------------------------------------------

void nlq401_device::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("prom", 0);
}


//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------

void nlq401_device::device_add_mconfig(machine_config &config)
{
	upd7810_device &mcu(UPD7810(config, "mcu", 11_MHz_XTAL));
	mcu.set_addrmap(AS_PROGRAM, &nlq401_device::mem_map);
	mcu.pb_out_cb().set(FUNC(nlq401_device::expander_w));
	mcu.pc_in_cb().set(FUNC(nlq401_device::expander_r));

	TMS1025(config, m_inpexp); // B8 (labeled M50780 on schematic)
	m_inpexp->set_ms(0);
	m_inpexp->read_port1_callback().set_ioport("P1");
	m_inpexp->read_port2_callback().set_ioport("P2");
	m_inpexp->read_port3_callback().set_ioport("P3");
	m_inpexp->read_port4_callback().set_ioport("P4");
	m_inpexp->read_port5_callback().set_ioport("P5");
	m_inpexp->read_port6_callback().set_ioport("P6");

	TMS1025(config, m_outexp); // B2 (labeled M50780 on schematic)
	m_outexp->set_ms(1); // tied to _RESET
	//m_outexp->write_port1_callback().set(FUNC(nlq401_device::cr_w));
	//m_outexp->write_port2_callback().set(FUNC(nlq401_device::lf_w));
	//m_outexp->write_port3_callback().set(FUNC(nlq401_device::ack_w));
	//m_outexp->write_port4_callback().set(FUNC(nlq401_device::head_5_8_w));
	//m_outexp->write_port5_callback().set(FUNC(nlq401_device::head_1_4_w));
	//m_outexp->write_port7_callback().set(FUNC(nlq401_device::led_w));
}


//**************************************************************************
//  INPUT PORTS
//**************************************************************************

static INPUT_PORTS_START(nlq401)
	PORT_START("SW1")
	PORT_DIPNAME(0x01, 0x01, "DIP11") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x02, 0x02, "DIP12") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x04, 0x04, "DIP13") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x08, 0x08, "DIP14") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x10, 0x10, "DIP15") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("P1")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("HP SW")
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("PE SW")
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P2")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("LF SW")
	PORT_DIPNAME(2, 2, "DIP28") PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(2, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(4, 4, "DIP27") PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(4, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(8, 8, "DIP16") PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(8, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))

	PORT_START("P3")
	PORT_DIPNAME(1, 1, "DIP17") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(1, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(2, 2, "DIP18") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(2, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(4, 4, "DIP26") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(4, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(8, 8, "DIP25") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(8, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))

	PORT_START("P4")
	PORT_DIPNAME(1, 1, "DIP24") PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(1, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(2, 2, "DIP23") PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(2, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(4, 4, "DIP22") PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(4, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))
	PORT_DIPNAME(8, 8, "DIP21") PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(8, DEF_STR(Off))
	PORT_DIPSETTING(0, DEF_STR(On))

	PORT_START("P5")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("DSR")
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("CTS")
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("P6")
	PORT_BIT(1, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ONL SW")
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------

ioport_constructor nlq401_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(nlq401);
}


//**************************************************************************
//  ROM DEFINITION
//**************************************************************************

ROM_START(nlq401)
	ROM_REGION(0x4000, "prom", 0)
	ROM_LOAD("schneider_nlq401_rev004.bin", 0x0000, 0x4000, CRC(5c331aed) SHA1(b6374abaebb8e484e573caa21b1cc87f1554c8d6))
ROM_END

//-------------------------------------------------
//  device_rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nlq401_device::device_rom_region() const
{
	return ROM_NAME(nlq401);
}
