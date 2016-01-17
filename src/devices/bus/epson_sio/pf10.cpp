// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON PF-10

    Battery operated portable 3.5" floppy drive

    Status: Needs lots of work.

**********************************************************************/

#include "pf10.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type EPSON_PF10 = &device_creator<epson_pf10_device>;


//-------------------------------------------------
//  address maps
//-------------------------------------------------

static ADDRESS_MAP_START( cpu_mem, AS_PROGRAM, 8, epson_pf10_device )
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE("maincpu", hd6303y_cpu_device, m6801_io_r, m6801_io_w)
	AM_RANGE(0x0040, 0x00ff) AM_RAM /* 192 bytes internal ram */
	AM_RANGE(0x0800, 0x0fff) AM_RAM /* external 2k ram */
	AM_RANGE(0x1000, 0x17ff) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0x1800, 0x1fff) AM_WRITE(fdc_tc_w)
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu_io, AS_IO, 8, epson_pf10_device )
	AM_RANGE(M6801_PORT1, M6801_PORT1) AM_READWRITE(port1_r, port1_w)
	AM_RANGE(M6801_PORT2, M6801_PORT2) AM_READWRITE(port2_r, port2_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( pf10 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("k3pf1.bin", 0x0000, 0x2000, CRC(eef4593a) SHA1(bb176e4baf938fe58c2d32f7c46d7bb7b0627755))
ROM_END

const rom_entry *epson_pf10_device::device_rom_region() const
{
	return ROM_NAME( pf10 );
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static SLOT_INTERFACE_START( pf10_floppies )
	SLOT_INTERFACE( "smd165", EPSON_SMD_165 )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( pf10 )
	MCFG_CPU_ADD("maincpu", HD6303Y, XTAL_4_9152MHz) // HD63A03XF
	MCFG_CPU_PROGRAM_MAP(cpu_mem)
	MCFG_CPU_IO_MAP(cpu_io)
	MCFG_M6801_SER_TX(DEVWRITELINE(DEVICE_SELF, epson_pf10_device, hd6303_tx_w))

	MCFG_UPD765A_ADD("upd765a", false, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765a:0", pf10_floppies, "smd165", floppy_image_device::default_floppy_formats)

	MCFG_EPSON_SIO_ADD("sio", nullptr)
	MCFG_EPSON_SIO_RX(DEVWRITELINE(DEVICE_SELF, epson_pf10_device, rxc_w))
	MCFG_EPSON_SIO_PIN(DEVWRITELINE(DEVICE_SELF, epson_pf10_device, pinc_w))
MACHINE_CONFIG_END

machine_config_constructor epson_pf10_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pf10 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_pf10_device - constructor
//-------------------------------------------------

epson_pf10_device::epson_pf10_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EPSON_PF10, "EPSON PF-10 Portable Floppy Unit", tag, owner, clock, "epson_pf10", __FILE__),
	device_epson_sio_interface(mconfig, *this),
	m_cpu(*this, "maincpu"),
	m_fdc(*this, "upd765a"),
	m_sio_output(*this, "sio"), m_floppy(nullptr), m_timer(nullptr),
	m_port1(0xff),
	m_port2(0xff),
	m_rxc(1), m_hd6303_tx(0), m_pinc(0)
{
	m_sio_input = dynamic_cast<epson_sio_device *>(owner);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_pf10_device::device_start()
{
	m_timer = timer_alloc(0, nullptr);
	m_floppy = subdevice<floppy_connector>("upd765a:0")->get_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_pf10_device::device_reset()
{
	m_timer->adjust(attotime::zero, 0, attotime::from_hz(38400 * 8));
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void epson_pf10_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case 0:
		m_cpu->m6801_clock_serial();
		break;
	}
}


//**************************************************************************
//  CPU
//**************************************************************************

READ8_MEMBER( epson_pf10_device::port1_r )
{
	logerror("%s: port1_r(%02x)\n", tag().c_str(), m_port1);
	return m_port1;
}

WRITE8_MEMBER( epson_pf10_device::port1_w )
{
	logerror("%s: port1_w(%02x)\n", tag().c_str(), data);
}

READ8_MEMBER( epson_pf10_device::port2_r )
{
	logerror("%s: port2_r(%02x)\n", tag().c_str(), m_port2);
	return m_port2;
}

WRITE8_MEMBER( epson_pf10_device::port2_w )
{
	m_floppy->mon_w(data & PORT2_MON);
	logerror("%s: port2_w(%02x)\n", tag().c_str(), data);
}

READ8_MEMBER( epson_pf10_device::fdc_r )
{
	logerror("%s: fdc_r @ %04x\n", tag().c_str(), offset);
	return 0xff;
}

WRITE8_MEMBER( epson_pf10_device::fdc_w )
{
	logerror("%s: fdc_w @ %04x (%02x)\n", tag().c_str(), offset, data);
}

WRITE8_MEMBER( epson_pf10_device::fdc_tc_w )
{
	logerror("%s: fdc_tc_w(%02x)\n", tag().c_str(), data);
}


//**************************************************************************
//  SIO INTERFACE
//**************************************************************************

//-------------------------------------------------
//  rxc_w - rx input
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_pf10_device::rxc_w )
{
	m_rxc = state;
	m_sio_input->rx_w(m_hd6303_tx & m_rxc);
}

//-------------------------------------------------
//  pinc_w - pin input
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_pf10_device::pinc_w )
{
	m_pinc = state;
	m_sio_input->pin_w(m_pinc);
}

//-------------------------------------------------
//  hd6303_tx_w - rx output
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_pf10_device::hd6303_tx_w )
{
	m_hd6303_tx = state;
	m_sio_input->rx_w(m_hd6303_tx & m_rxc);
}

//-------------------------------------------------
//  tx_w - tx input
//-------------------------------------------------

void epson_pf10_device::tx_w(int level)
{
	if (level)
		m_port2 |= PORT2_RXD;
	else
		m_port2 &= ~PORT2_RXD;

	m_sio_output->tx_w(level);
}

//-------------------------------------------------
//  pout_w - pout input
//-------------------------------------------------

void epson_pf10_device::pout_w(int level)
{
	m_sio_output->pout_w(level);
}
