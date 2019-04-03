// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM040-B Remote Telecommunication controller emulation

**********************************************************************/

#include "emu.h"
#include "rtc.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define OPTION_ID       0x1c

#define Z80_TAG         "z80"
#define AM9517A_TAG     "am9517"
#define Z80CTC_0_TAG    "z80ctc_0"
#define Z80CTC_1_TAG    "z80ctc_1"
#define Z80SIO_TAG      "z80sio"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(WANGPC_RTC, wangpc_rtc_device, "wangpc_rtc", "Wang PC-PM040-B Remote Telecommunication Controller")


//-------------------------------------------------
//  ROM( wangpc_rtc )
//-------------------------------------------------

ROM_START( wangpc_rtc )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_LOAD( "remotecomms-l28.bin", 0x0000, 0x1000, CRC(c05a1bee) SHA1(6b3f0d787d014b1fd3925812c905ddb63c5055f1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *wangpc_rtc_device::device_rom_region() const
{
	return ROM_NAME( wangpc_rtc );
}


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_rtc_mem )
//-------------------------------------------------

void wangpc_rtc_device::wangpc_rtc_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(Z80_TAG, 0);
	map(0x1000, 0xffff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_rtc_io )
//-------------------------------------------------

void wangpc_rtc_device::wangpc_rtc_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_sio, FUNC(z80sio0_device::cd_ba_r), FUNC(z80sio0_device::cd_ba_w));
	map(0x10, 0x1f).rw(AM9517A_TAG, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x20, 0x23).rw(m_ctc0, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x30); //AM_WRITE(clear_char_w)
	map(0x31, 0x31); //AM_WRITE(set_char_w)
	map(0x40, 0x40).portr("SW1"); //AM_WRITE(control_w)
	map(0x44, 0x44); //AM_READ(i8086_status_r) AM_WRITE(reset_w)
	map(0x48, 0x48); //AM_WRITE(dte_ready_w)
	map(0x4c, 0x4c); //AM_READWRITE(8232_acu_r, 8232_acu_w)
	map(0x50, 0x50); //AM_READ(outbound_data_r)
	map(0x51, 0x52); //AM_WRITE(status_w)
	map(0x54, 0x54); //AM_WRITE(enable_inbound_data_w)
	map(0x51, 0x52); //AM_WRITE(inbound_data_w)
	map(0x60, 0x63).rw(m_ctc1, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x70, 0x70); //AM_READWRITE(led_toggle_r, odd_parity_w)
	map(0x71, 0x71); //AM_WRITE(even_parity_w)
}


//-------------------------------------------------
//  z80_daisy_config wangpc_rtc_daisy_chain
//-------------------------------------------------

static const z80_daisy_config wangpc_rtc_daisy_chain[] =
{
	{ Z80SIO_TAG },
	{ Z80CTC_0_TAG },
	{ Z80CTC_1_TAG },
	{ nullptr }
};

//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void wangpc_rtc_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, 2000000);
	m_maincpu->set_daisy_config(wangpc_rtc_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &wangpc_rtc_device::wangpc_rtc_mem);
	m_maincpu->set_addrmap(AS_IO, &wangpc_rtc_device::wangpc_rtc_io);

	AM9517A(config, m_dmac, 2000000);

	Z80CTC(config, m_ctc0, 2000000);
	m_ctc0->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80CTC(config, m_ctc1, 2000000);
	m_ctc1->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	Z80SIO0(config, m_sio, 2000000);
	m_sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}


//-------------------------------------------------
//  INPUT_PORTS( wangpc_rtc )
//-------------------------------------------------

INPUT_PORTS_START( wangpc_rtc )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor wangpc_rtc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( wangpc_rtc );
}




//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  wangpc_rtc_device - constructor
//-------------------------------------------------

wangpc_rtc_device::wangpc_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, WANGPC_RTC, tag, owner, clock),
	device_wangpcbus_card_interface(mconfig, *this),
	m_maincpu(*this, Z80_TAG),
	m_dmac(*this, AM9517A_TAG),
	m_ctc0(*this, Z80CTC_0_TAG),
	m_ctc1(*this, Z80CTC_1_TAG),
	m_sio(*this, Z80SIO_TAG),
	m_char_ram(*this, "char_ram")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void wangpc_rtc_device::device_start()
{
	m_char_ram.allocate(0x100);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void wangpc_rtc_device::device_reset()
{
}


//-------------------------------------------------
//  wangpcbus_mrdc_r - memory read
//-------------------------------------------------

uint16_t wangpc_rtc_device::wangpcbus_mrdc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_rtc_device::wangpcbus_amwc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

uint16_t wangpc_rtc_device::wangpcbus_iorc_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0xfe/2:
			data = 0xff00 | OPTION_ID;
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  wangpcbus_aiowc_w - I/O write
//-------------------------------------------------

void wangpc_rtc_device::wangpcbus_aiowc_w(offs_t offset, uint16_t mem_mask, uint16_t data)
{
	if (sad(offset))
	{
		switch (offset & 0x7f)
		{
		case 0xfc/2:
			device_reset();
			break;
		}
	}
}
