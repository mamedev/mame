// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Wang PC-PM040-B Remote Telecommunication controller emulation

**********************************************************************/

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

const device_type WANGPC_RTC = &device_creator<wangpc_rtc_device>;


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

const rom_entry *wangpc_rtc_device::device_rom_region() const
{
	return ROM_NAME( wangpc_rtc );
}


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_rtc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( wangpc_rtc_mem, AS_PROGRAM, 8, wangpc_rtc_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( wangpc_rtc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( wangpc_rtc_io, AS_IO, 8, wangpc_rtc_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE(Z80SIO_TAG, z80sio0_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x10, 0x1f) AM_DEVREADWRITE(AM9517A_TAG, am9517a_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE(Z80CTC_0_TAG, z80ctc_device, read, write)
	AM_RANGE(0x30, 0x30) //AM_WRITE(clear_char_w)
	AM_RANGE(0x31, 0x31) //AM_WRITE(set_char_w)
	AM_RANGE(0x40, 0x40) AM_READ_PORT("SW1") //AM_WRITE(control_w)
	AM_RANGE(0x44, 0x44) //AM_READ(i8086_status_r) AM_WRITE(reset_w)
	AM_RANGE(0x48, 0x48) //AM_WRITE(dte_ready_w)
	AM_RANGE(0x4c, 0x4c) //AM_READWRITE(8232_acu_r, 8232_acu_w)
	AM_RANGE(0x50, 0x50) //AM_READ(outbound_data_r)
	AM_RANGE(0x51, 0x52) //AM_WRITE(status_w)
	AM_RANGE(0x54, 0x54) //AM_WRITE(enable_inbound_data_w)
	AM_RANGE(0x51, 0x52) //AM_WRITE(inbound_data_w)
	AM_RANGE(0x60, 0x63) AM_DEVREADWRITE(Z80CTC_1_TAG, z80ctc_device, read, write)
	AM_RANGE(0x70, 0x70) //AM_READWRITE(led_toggle_r, odd_parity_w)
	AM_RANGE(0x71, 0x71) //AM_WRITE(even_parity_w)
ADDRESS_MAP_END


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
//  MACHINE_CONFIG_FRAGMENT( wangpc_rtc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( wangpc_rtc )
	MCFG_CPU_ADD(Z80_TAG, Z80, 2000000)
	MCFG_CPU_CONFIG(wangpc_rtc_daisy_chain)
	MCFG_CPU_PROGRAM_MAP(wangpc_rtc_mem)
	MCFG_CPU_IO_MAP(wangpc_rtc_io)

	MCFG_DEVICE_ADD(AM9517A_TAG, AM9517A, 2000000)

	MCFG_DEVICE_ADD(Z80CTC_0_TAG, Z80CTC, 2000000)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_DEVICE_ADD(Z80CTC_1_TAG, Z80CTC, 2000000)
	MCFG_Z80CTC_INTR_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))

	MCFG_Z80SIO0_ADD(Z80SIO_TAG, 2000000, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor wangpc_rtc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wangpc_rtc );
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

wangpc_rtc_device::wangpc_rtc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WANGPC_RTC, "Wang PC-PM040-B", tag, owner, clock, "wangpc_rtc", __FILE__),
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

UINT16 wangpc_rtc_device::wangpcbus_mrdc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

	return data;
}


//-------------------------------------------------
//  wangpcbus_amwc_w - memory write
//-------------------------------------------------

void wangpc_rtc_device::wangpcbus_amwc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
{
}


//-------------------------------------------------
//  wangpcbus_iorc_r - I/O read
//-------------------------------------------------

UINT16 wangpc_rtc_device::wangpcbus_iorc_r(address_space &space, offs_t offset, UINT16 mem_mask)
{
	UINT16 data = 0xffff;

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

void wangpc_rtc_device::wangpcbus_aiowc_w(address_space &space, offs_t offset, UINT16 mem_mask, UINT16 data)
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
