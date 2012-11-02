/**********************************************************************

    EPSON TF-20

    Dual 5.25" floppy drive with HX-20 factory option

    Status: Issues with new uPD765, missing uPD7201 emulation.

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "tf20.h"

#define XTAL_CR1	XTAL_8MHz
#define XTAL_CR2	XTAL_4_9152MHz


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
	PORT_DIPNAME(0x0f, 0x0f, "Drive extension")
	PORT_DIPLOCATION("TF-20 TFX:8,7,6,5")
	PORT_DIPSETTING(0x0f, "A & B Drive")
	PORT_DIPSETTING(0x07, "C & D Drive")
INPUT_PORTS_END

ioport_constructor epson_tf20_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( tf20 );
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static UPD7201_INTERFACE( tf20_upd7201_intf )
{
	DEVCB_NULL,				/* interrupt: nc */
	{
		{
			XTAL_CR2 / 128,		/* receive clock: 38400 baud (default) */
			XTAL_CR2 / 128,		/* transmit clock: 38400 baud (default) */
			DEVCB_NULL,			/* receive DRQ */
			DEVCB_NULL,			/* transmit DRQ */
			DEVCB_NULL,			/* receive data */
			DEVCB_NULL,			/* transmit data */
			DEVCB_NULL,			/* clear to send */
			DEVCB_LINE_GND,		/* data carrier detect */
			DEVCB_NULL,			/* ready to send */
			DEVCB_NULL,			/* data terminal ready */
			DEVCB_NULL,			/* wait */
			DEVCB_NULL			/* sync output: nc */
		}, {
			XTAL_CR2 / 128,		/* receive clock: 38400 baud (default) */
			XTAL_CR2 / 128,		/* transmit clock: 38400 baud (default) */
			DEVCB_NULL,			/* receive DRQ: nc */
			DEVCB_NULL,			/* transmit DRQ */
			DEVCB_NULL,			/* receive data */
			DEVCB_NULL,			/* transmit data */
			DEVCB_LINE_GND,		/* clear to send */
			DEVCB_LINE_GND,		/* data carrier detect */
			DEVCB_NULL,			/* ready to send */
			DEVCB_NULL,			/* data terminal ready: nc */
			DEVCB_NULL,			/* wait */
			DEVCB_NULL			/* sync output: nc */
		}
	}
};

static const floppy_format_type tf20_floppy_formats[] =
{
	FLOPPY_D88_FORMAT,
	FLOPPY_MFM_FORMAT,
	FLOPPY_MFI_FORMAT,
	NULL
};

static SLOT_INTERFACE_START( tf20_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( tf20 )
	MCFG_CPU_ADD("19b", Z80, XTAL_CR1 / 2) /* uPD780C */
	MCFG_CPU_PROGRAM_MAP(cpu_mem)
	MCFG_CPU_IO_MAP(cpu_io)

	// 64k internal ram
	MCFG_RAM_ADD("ram")
	MCFG_RAM_DEFAULT_SIZE("64k")

	// upd7201 serial interface
	MCFG_UPD7201_ADD("3a", XTAL_CR1 / 2, tf20_upd7201_intf)

	// floppy disk controller
	MCFG_UPD765A_ADD("5a", true, true)

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("5a:0", tf20_floppies, "525dd", 0, tf20_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("5a:1", tf20_floppies, "525dd", 0, tf20_floppy_formats)

	// serial interface to another device
	MCFG_EPSON_SIO_ADD("sio")
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

epson_tf20_device::epson_tf20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, EPSON_TF20, "EPSON TF-20 dual floppy drive", tag, owner, clock),
	device_epson_sio_interface(mconfig, *this),
	m_cpu(*this, "19b"),
	m_ram(*this, "ram"),
	m_fdc(*this, "5a"),
	m_mpsc(*this, "3a"),
	m_sio(*this, "sio")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void epson_tf20_device::device_start()
{
	// make sure the ram device is already running
	if (!m_ram->started())
		throw device_missing_dependencies();

	m_timer_serial = timer_alloc(0, NULL);
    m_timer_tc = timer_alloc(1, NULL);

    m_cpu->set_irq_acknowledge_callback(irq_callback);

	m_fd0 = subdevice<floppy_connector>("5a:0")->get_device();
	m_fd1 = subdevice<floppy_connector>("5a:1")->get_device();

	m_fdc->setup_intrq_cb(upd765a_device::line_cb(FUNC(epson_tf20_device::fdc_irq), this));

	// enable second half of ram
	m_cpu->space(AS_PROGRAM).install_ram(0x8000, 0xffff, m_ram->pointer() + 0x8000);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void epson_tf20_device::device_reset()
{
	// init timers
	m_timer_serial->adjust(attotime::from_hz(XTAL_CR2 / 128), 0, attotime::from_hz(XTAL_CR2 / 128));
	m_timer_tc->adjust(attotime::never);

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
		m_mpsc->txca_w(1);
		m_mpsc->rxcb_w(1);
		m_mpsc->txcb_w(1);
		break;

	case 1:
		logerror("%s: tc off\n", tag());
		m_fdc->tc_w(false);
		break;
	}
}


//-------------------------------------------------
//  irq vector callback
//-------------------------------------------------

IRQ_CALLBACK( epson_tf20_device::irq_callback )
{
	return 0x00;
}


//-------------------------------------------------
//  fdc interrupt
//-------------------------------------------------

void epson_tf20_device::fdc_irq(bool state)
{
	m_cpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}


//-------------------------------------------------
//  tx_w
//-------------------------------------------------

void epson_tf20_device::tx_w(int level)
{
	logerror("%s: tx_w(%d)\n", tag(), level);
	m_mpsc->rxda_w(level);
}


//-------------------------------------------------
//  pout_w
//-------------------------------------------------

void epson_tf20_device::pout_w(int level)
{
	logerror("%s: pout_w(%d)\n", tag(), level);
	m_mpsc->ctsa_w(level);
}


//-------------------------------------------------
//  rx_r
//-------------------------------------------------

int epson_tf20_device::rx_r()
{
	logerror("%s: rx_r\n", tag());
	return m_mpsc->txda_r();
}


//-------------------------------------------------
//  pin_r
//-------------------------------------------------

int epson_tf20_device::pin_r()
{
	logerror("%s: pin_r\n", tag());
	return m_mpsc->dtra_r();
}


// a read from this location disables the rom
READ8_MEMBER( epson_tf20_device::rom_disable_r )
{
	// switch in ram
	m_cpu->space(AS_PROGRAM).install_ram(0x0000, 0x7fff, m_ram->pointer());
	return 0xff;
}


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
