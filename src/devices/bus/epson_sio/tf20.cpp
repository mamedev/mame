// license:GPL-2.0+
// copyright-holders:Dirk Best
/**********************************************************************

    EPSON TF-20

    Dual 5.25" floppy drive with HX-20 factory option

    Status: Needs testing.

    http://fjkraan.home.xs4all.nl/comp/tf20/index.html

**********************************************************************/

#include "tf20.h"

#define XTAL_CR1    XTAL_8MHz
#define XTAL_CR2    XTAL_4_9152MHz


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type EPSON_TF20 = &device_creator<epson_tf20_device>;

//-------------------------------------------------
//  address maps
//-------------------------------------------------

static ADDRESS_MAP_START( cpu_mem, AS_PROGRAM, 8, epson_tf20_device )
	AM_RANGE(0x0000, 0x7fff) AM_RAMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu_io, AS_IO, 8, epson_tf20_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf3) AM_DEVREADWRITE("3a", upd7201_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0xf6, 0xf6) AM_READ(rom_disable_r)
	AM_RANGE(0xf7, 0xf7) AM_READ_PORT("tf20_dip")
	AM_RANGE(0xf8, 0xf8) AM_READWRITE(upd765_tc_r, fdc_control_w)
	AM_RANGE(0xfa, 0xfb) AM_DEVICE("5a", upd765a_device, map)
ADDRESS_MAP_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( tf20 )
	ROM_REGION(0x0800, "rom", 0)
	ROM_LOAD("tfx.15e", 0x0000, 0x0800, CRC(af34f084) SHA1(c9bdf393f757ba5d8f838108ceb2b079be1d616e))
ROM_END

const rom_entry *epson_tf20_device::device_rom_region() const
{
	return ROM_NAME( tf20 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

INPUT_PORTS_START( tf20 )
	PORT_START("tf20_dip")
	PORT_DIPNAME(0x0f, 0x00, "Drive extension")
	PORT_DIPLOCATION("TF-20:8,7,6,5")
	PORT_DIPSETTING(0x00, "A & B Drive")
	PORT_DIPSETTING(0x01, "C & D Drive")
INPUT_PORTS_END

ioport_constructor epson_tf20_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tf20 );
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static SLOT_INTERFACE_START( tf20_floppies )
	SLOT_INTERFACE( "sd320", EPSON_SD_320 )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( tf20 )
	MCFG_CPU_ADD("19b", Z80, XTAL_CR1 / 2) /* uPD780C */
	MCFG_CPU_PROGRAM_MAP(cpu_mem)
	MCFG_CPU_IO_MAP(cpu_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(DEVICE_SELF, epson_tf20_device,irq_callback)

	// 64k internal ram
	MCFG_RAM_ADD("ram")
	MCFG_RAM_DEFAULT_SIZE("64k")

	// upd7201 serial interface
	MCFG_UPD7201_ADD("3a", XTAL_CR1 / 2, 0, 0, 0, 0)
	MCFG_Z80DART_OUT_TXDA_CB(WRITELINE(epson_tf20_device, txda_w))
	MCFG_Z80DART_OUT_DTRA_CB(WRITELINE(epson_tf20_device, dtra_w))

	// floppy disk controller
	MCFG_UPD765A_ADD("5a", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE("19b", INPUT_LINE_IRQ0))

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("5a:0", tf20_floppies, "sd320", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("5a:1", tf20_floppies, "sd320", floppy_image_device::default_floppy_formats)

	// serial interface to another device
	MCFG_EPSON_SIO_ADD("sio", nullptr)
	MCFG_EPSON_SIO_RX(DEVWRITELINE(DEVICE_SELF, epson_tf20_device, rxc_w))
	MCFG_EPSON_SIO_PIN(DEVWRITELINE(DEVICE_SELF, epson_tf20_device, pinc_w))
MACHINE_CONFIG_END

machine_config_constructor epson_tf20_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tf20 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  epson_tf20_device - constructor
//-------------------------------------------------

epson_tf20_device::epson_tf20_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EPSON_TF20, "EPSON TF-20 Dual Floppy Disk Drive", tag, owner, clock, "epson_tf20", __FILE__),
	device_epson_sio_interface(mconfig, *this),
	m_cpu(*this, "19b"),
	m_ram(*this, "ram"),
	m_fdc(*this, "5a"),
	m_mpsc(*this, "3a"),
	m_sio_output(*this, "sio"), m_fd0(nullptr), m_fd1(nullptr), m_timer_serial(nullptr), m_timer_tc(nullptr),
	m_rxc(1), m_txda(0), m_dtra(0), m_pinc(0)
{
	m_sio_input = dynamic_cast<epson_sio_device *>(owner);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_tf20_device::device_start()
{
	// make sure the ram device is already running
	if (!m_ram->started())
		throw device_missing_dependencies();

	m_timer_serial = timer_alloc(0, nullptr);
	m_timer_tc = timer_alloc(1, nullptr);

	m_fd0 = subdevice<floppy_connector>("5a:0")->get_device();
	m_fd1 = subdevice<floppy_connector>("5a:1")->get_device();

	// enable second half of ram
	m_cpu->space(AS_PROGRAM).install_ram(0x8000, 0xffff, m_ram->pointer() + 0x8000);

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_tf20_device::device_reset()
{
	// init timers
	m_timer_serial->adjust(attotime::zero, 0, attotime::from_hz(XTAL_CR2 / 8));
	m_timer_tc->adjust(attotime::never);

	m_mpsc->rxa_w(1);
	m_mpsc->rxb_w(1);

	// enable rom
	m_cpu->space(AS_PROGRAM).install_rom(0x0000, 0x07ff, 0, 0x7800, memregion("rom")->base());
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void epson_tf20_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case 0:
		m_mpsc->rxca_w(1);
		m_mpsc->rxca_w(0);
		m_mpsc->txca_w(1);
		m_mpsc->txca_w(0);
		m_mpsc->rxcb_w(1);
		m_mpsc->rxcb_w(0);
		m_mpsc->txcb_w(1);
		m_mpsc->txcb_w(0);
		break;

	case 1:
		logerror("%s: tc off\n", tag().c_str());
		m_fdc->tc_w(false);
		break;
	}
}


//**************************************************************************
//  CPU & MEMORY
//**************************************************************************

//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK_MEMBER( epson_tf20_device::irq_callback )
{
	return 0x00;
}

// a read from this location disables the rom
READ8_MEMBER( epson_tf20_device::rom_disable_r )
{
	// switch in ram
	m_cpu->space(AS_PROGRAM).install_ram(0x0000, 0x7fff, m_ram->pointer());
	return 0xff;
}


//**************************************************************************
//  FLOPPY DISK CONTROLLER
//**************************************************************************

//-------------------------------------------------
//  fdc interrupt
//-------------------------------------------------

READ8_MEMBER( epson_tf20_device::upd765_tc_r )
{
	logerror("%s: upd765_tc_r\n", space.machine().describe_context());

	// toggle tc on read
	m_fdc->tc_w(true);
	m_timer_tc->adjust(attotime::zero);

	return 0xff;
}

WRITE8_MEMBER( epson_tf20_device::fdc_control_w )
{
	logerror("%s: tf20_fdc_control_w(%02x)\n", space.machine().describe_context(), data);

	// bit 0, motor on signal
	m_fd0->mon_w(!BIT(data, 0));
	m_fd1->mon_w(!BIT(data, 0));
}


//**************************************************************************
//  SIO INTERFACE
//**************************************************************************

//-------------------------------------------------
//  rxc_w - rx input
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_tf20_device::rxc_w )
{
	m_rxc = state;
	m_sio_input->rx_w(m_txda && m_rxc);
}

//-------------------------------------------------
//  pinc_w - pin input
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_tf20_device::pinc_w )
{
	m_pinc = state;
	m_sio_input->pin_w(!m_dtra || m_pinc);
}

//-------------------------------------------------
//  txda_w - rx output
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_tf20_device::txda_w )
{
	m_txda = state;
	m_sio_input->rx_w(m_txda && m_rxc);
}

//-------------------------------------------------
//  dtra_w - pin output
//-------------------------------------------------

WRITE_LINE_MEMBER( epson_tf20_device::dtra_w )
{
	m_dtra = state;
	m_sio_input->pin_w(!m_dtra || m_pinc);
}

//-------------------------------------------------
//  tx_w - tx input
//-------------------------------------------------

void epson_tf20_device::tx_w(int level)
{
	m_mpsc->rxa_w(level);
	m_sio_output->tx_w(level);
}

//-------------------------------------------------
//  pout_w - pout input
//-------------------------------------------------

void epson_tf20_device::pout_w(int level)
{
	m_mpsc->ctsa_w(!level);
	m_sio_output->pout_w(level);
}
