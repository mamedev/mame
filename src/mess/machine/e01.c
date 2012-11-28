/**********************************************************************

    Acorn FileStore E01/E01S network hard disk emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

    http://acorn.chriswhy.co.uk/Network/Econet.html
    http://acorn.chriswhy.co.uk/Network/Pics/Acorn_FileStoreE01.html
    http://acorn.chriswhy.co.uk/8bit_Upgrades/Acorn_FileStoreE01S.html
    http://www.heyrick.co.uk/econet/fs/emulator.html
    http://www.pdfio.com/k-1019481.html#

**********************************************************************/

/*

    The FileStore E01 is an Econet station in its own right which acts as a fileserver when connected to a network. It is a single unit
    which does not require a monitor or keyboard. Communication with the FileStore is done via another Econet station when the FileStore
    is in one of two "maintenance modes"

    The E01 can be seen as a slimmed-down BBC computer tailored for its function as a fileserver. It has a 6502 processor at its heart
    along with a 6522 VIA just like the BBC. It requires a Master series Econet module to be plugged in and connects to the network via
    an Econet port in the same way as any other station.

    The FileStore E01S was Acorns second generation Filestore replacing the FileStore E01. The FileStore is a dedicated Econet fileserver,
    it does not support a keyboard and monitor, instead you use an Econet attached station to logon and perform administrative tasks.

            Hitachi HD146818P Real Time Clock
            Rockwell R65C102P3 CPU
            2 x TMM27256D-20 white labelled EPROMs, TMSE01 MOS on left and E01 FS on the right
    IC20    WD2793-APL-02 floppy disc controller
            2 x NEC D41464C-12 64k x 4bit NMOS RAM ICs giving 64K memory
    IC21    Rockwell RC6522AP VIA behind to the right

*/

/*

    TODO:

    - memory_bank::set_entry called for bank ':econet254:e01s:bank2' with invalid bank entry 0
    - centronics strobe
    - econet clock speed select
    - ADLC interrupts
    - ECONET device
    - artwork
    - hard disk

        E20: Rodime RO652 (-chs 306,4,17,512)
        E40S: Rodime RO3057S (-chs 680,5,26,512)
        E60S:

*/

#include "e01.h"
#include "machine/scsibus.h"
#include "machine/scsicb.h"
#include "machine/scsihd.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define R65C102_TAG		"r65c102"
#define R6522_TAG		"ic21"
#define WD2793_TAG		"ic20"
#define MC6854_TAG		"mc6854"
#define HD146818_TAG	"hd146818"
#define CENTRONICS_TAG	"centronics"
#define SCSIBUS_TAG		"scsi"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type E01 = &device_creator<e01_device>;
const device_type E01S = &device_creator<e01s_device>;

e01s_device::e01s_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:e01_device(mconfig, E01S, "Acorn FileStore E01S", tag, owner, clock) { m_variant = TYPE_E01S; }


//-------------------------------------------------
//  ROM( e01 )
//-------------------------------------------------

ROM_START( e01 )
    ROM_REGION( 0x10000, R65C102_TAG, 0 )
	//ROM_DEFAULT_BIOS("v131")
	//ROM_SYSTEM_BIOS( 0, "v131", "V 1.31" )
	ROM_LOAD( "0254,205-04 e01 fs",  0x0000, 0x8000, CRC(ae666c76) SHA1(0954119eb5cd09cdbadf76d60d812aa845838d5a) )
	ROM_LOAD( "0254,205-03 e01 mos", 0x8000, 0x8000, CRC(a13e8014) SHA1(6f44a1a48108c60a64a1774cb30c1a59c4a6a199) )
ROM_END


//-------------------------------------------------
//  ROM( e01s )
//-------------------------------------------------

ROM_START( e01s )
    ROM_REGION( 0x10000, R65C102_TAG, 0 )
	//ROM_DEFAULT_BIOS("v140")
	//ROM_SYSTEM_BIOS( 0, "v133", "V 1.33" ) // 0282,008-02 e01s rom
	ROM_LOAD( "e01sv133.rom",  0x0000, 0x10000, CRC(2a4a0032) SHA1(54ad68ceae44992293ccdd64ec88ad8520deec22) ) // which label?
	//ROM_SYSTEM_BIOS( 1, "v140", "V 1.40" )
	ROM_LOAD( "e01sv140.rom",  0x0000, 0x10000, CRC(5068fe86) SHA1(9b8740face15b5541e2375b3054988af00757931) ) // which label?
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *e01_device::device_rom_region() const
{
	switch (m_variant)
	{
	default:
	case TYPE_E01:
		return ROM_NAME( e01 );

	case TYPE_E01S:
		return ROM_NAME( e01s );
	}
}


//-------------------------------------------------
//  MC146818_INTERFACE( rtc_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( e01_device::rtc_irq_w )
{
    m_rtc_irq = state;

    update_interrupts();
}

static mc146818_interface rtc_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, e01_device, rtc_irq_w)
};


//-------------------------------------------------
//  mc6854_interface adlc_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( e01_device::adlc_irq_w )
{
    m_adlc_irq = state;

    update_interrupts();
}

READ_LINE_MEMBER( e01_device::econet_data_r )
{
	return m_econet->data_r();
}

WRITE_LINE_MEMBER( e01_device::econet_data_w )
{
	m_econet->data_w(this, state);
}

static const mc6854_interface adlc_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, e01_device, adlc_irq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, e01_device, econet_data_r),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, e01_device, econet_data_w),
	NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  via6522_interface via_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( e01_device::via_irq_w )
{
	m_via_irq = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( e01_device::clk_en_w )
{
	m_clk_en = state;
}

static const via6522_interface via_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, write),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, e01_device, via_irq_w)
};


//-------------------------------------------------
//  floppy_interface e01_floppy_interface
//-------------------------------------------------

static const floppy_interface e01_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_3_5_DSDD, // NEC FD1036 A
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	NULL,
	NULL
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static SLOT_INTERFACE_START( e01_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

void e01_device::fdc_irq_w(bool state)
{
	m_fdc_irq = state;

	update_interrupts();
}

void e01_device::fdc_drq_w(bool state)
{
	m_fdc_drq = state;

	update_interrupts();
}

WRITE_LINE_MEMBER( e01_device::scsi_bsy_w )
{
	if (state)
	{
		m_scsibus->scsi_sel_w(0);
	}
}

WRITE_LINE_MEMBER( e01_device::scsi_req_w )
{
	if (!state)
	{
		m_scsibus->scsi_ack_w(0);
	}

	m_hdc_irq = !state;
	update_interrupts();
}


//-------------------------------------------------
//  centronics_interface e01_centronics_intf
//-------------------------------------------------

static centronics_interface e01_centronics_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(R6522_TAG, via6522_device, write_ca1),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  ADDRESS_MAP( e01_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( e01_mem, AS_PROGRAM, 8, e01_device )
	AM_RANGE(0x0000, 0xfbff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank2")
	AM_RANGE(0xfc00, 0xfc00) AM_MIRROR(0x00c3) AM_READWRITE(rtc_address_r, rtc_address_w)
	AM_RANGE(0xfc04, 0xfc04) AM_MIRROR(0x00c3) AM_READWRITE(rtc_data_r, rtc_data_w)
	AM_RANGE(0xfc08, 0xfc08) AM_MIRROR(0x00c0) AM_READ(ram_select_r) AM_WRITE(floppy_w)
	AM_RANGE(0xfc0c, 0xfc0f) AM_MIRROR(0x00c0) AM_DEVREADWRITE(WD2793_TAG, wd2793_t, read, write)
	AM_RANGE(0xfc10, 0xfc1f) AM_MIRROR(0x00c0) AM_DEVREADWRITE(R6522_TAG, via6522_device, read, write)
	AM_RANGE(0xfc20, 0xfc23) AM_MIRROR(0x00c0) AM_DEVREADWRITE_LEGACY(MC6854_TAG, mc6854_r, mc6854_w)
	AM_RANGE(0xfc24, 0xfc24) AM_MIRROR(0x00c3) AM_READWRITE(network_irq_disable_r, network_irq_disable_w)
	AM_RANGE(0xfc28, 0xfc28) AM_MIRROR(0x00c3) AM_READWRITE(network_irq_enable_r, network_irq_enable_w)
	AM_RANGE(0xfc2c, 0xfc2c) AM_MIRROR(0x00c3) AM_READ_PORT("FLAP")
	AM_RANGE(0xfc30, 0xfc30) AM_MIRROR(0x00c0) AM_READWRITE(hdc_data_r, hdc_data_w)
	AM_RANGE(0xfc31, 0xfc31) AM_MIRROR(0x00c0) AM_READ(hdc_status_r)
	AM_RANGE(0xfc32, 0xfc32) AM_MIRROR(0x00c0) AM_WRITE(hdc_select_w)
	AM_RANGE(0xfc33, 0xfc33) AM_MIRROR(0x00c0) AM_WRITE(hdc_irq_enable_w)
	AM_RANGE(0xfd00, 0xffff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank4")
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( e01 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( e01 )
    // basic machine hardware
	MCFG_CPU_ADD(R65C102_TAG, M65C02, XTAL_8MHz/4) // Rockwell R65C102P3
    MCFG_CPU_PROGRAM_MAP(e01_mem)

	MCFG_MC146818_IRQ_ADD(HD146818_TAG, MC146818_STANDARD, rtc_intf)

	// devices
	MCFG_VIA6522_ADD(R6522_TAG, XTAL_8MHz/4, via_intf)
	MCFG_MC6854_ADD(MC6854_TAG, adlc_intf)
	MCFG_WD2793x_ADD(WD2793_TAG, XTAL_8MHz/4 *8)
	MCFG_FLOPPY_DRIVE_ADD(WD2793_TAG":0", e01_floppies, "35dd", NULL, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2793_TAG":1", e01_floppies, "35dd", NULL, floppy_image_device::default_floppy_formats)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, e01_centronics_intf)

	MCFG_SCSIBUS_ADD(SCSIBUS_TAG)
	MCFG_SCSIDEV_ADD(SCSIBUS_TAG ":harddisk0", SCSIHD, SCSI_ID_0)
	MCFG_SCSICB_ADD(SCSIBUS_TAG ":host")
	MCFG_SCSICB_BSY_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, e01_device, scsi_bsy_w))
	MCFG_SCSICB_REQ_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, e01_device, scsi_req_w))

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor e01_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( e01 );
}


//-------------------------------------------------
//  INPUT_PORTS( e01 )
//-------------------------------------------------

static INPUT_PORTS_START( e01 )
	PORT_START("FLAP")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x40, 0x00, "Front Flap")
	PORT_CONFSETTING( 0x00, "Closed" )
	PORT_CONFSETTING( 0x40, "Open" )
	PORT_DIPNAME( 0x80, 0x00, "SW3")
	PORT_DIPSETTING( 0x00, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x80, DEF_STR( On ) )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor e01_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( e01 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_interrupts - update interrupt state
//-------------------------------------------------

inline void e01_device::update_interrupts()
{
	int irq = (m_via_irq || (m_hdc_ie & m_hdc_irq) || m_rtc_irq) ? ASSERT_LINE : CLEAR_LINE;
	int nmi = (m_fdc_irq || m_fdc_drq || (m_adlc_ie & m_adlc_irq)) ? ASSERT_LINE : CLEAR_LINE;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, irq);
	m_maincpu->set_input_line(INPUT_LINE_NMI, nmi);
}


//-------------------------------------------------
//   network_irq_enable - network interrupt enable
//-------------------------------------------------

inline void e01_device::network_irq_enable(int enabled)
{
	m_adlc_ie = enabled;

	update_interrupts();
}


//-------------------------------------------------
//   hdc_irq_enable - hard disk interrupt enable
//-------------------------------------------------

inline void e01_device::hdc_irq_enable(int enabled)
{
	m_hdc_ie = enabled;

	update_interrupts();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  e01_device - constructor
//-------------------------------------------------

e01_device::e01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, E01, "Acorn FileStore E01", tag, owner, clock),
      device_econet_interface(mconfig, *this),
	  m_maincpu(*this, R65C102_TAG),
	  m_fdc(*this, WD2793_TAG),
	  m_adlc(*this, MC6854_TAG),
	  m_rtc(*this, HD146818_TAG),
	  m_ram(*this, RAM_TAG),
	  m_scsibus(*this, SCSIBUS_TAG ":host"),
	  m_floppy0(*this, WD2793_TAG":0"),
	  m_floppy1(*this, WD2793_TAG":1"),
	  m_adlc_ie(0),
	  m_hdc_ie(0),
	  m_rtc_irq(CLEAR_LINE),
	  m_via_irq(CLEAR_LINE),
	  m_hdc_irq(CLEAR_LINE),
	  m_fdc_irq(CLEAR_LINE),
	  m_fdc_drq(CLEAR_LINE),
	  m_adlc_irq(CLEAR_LINE),
	  m_clk_en(0)
{
}


e01_device::e01_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, type, name, tag, owner, clock),
      device_econet_interface(mconfig, *this),
	  m_maincpu(*this, R65C102_TAG),
	  m_fdc(*this, WD2793_TAG),
	  m_adlc(*this, MC6854_TAG),
	  m_rtc(*this, HD146818_TAG),
	  m_ram(*this, RAM_TAG),
	  m_scsibus(*this, SCSIBUS_TAG ":host"),
	  m_floppy0(*this, WD2793_TAG":0"),
	  m_floppy1(*this, WD2793_TAG":1"),
	  m_adlc_ie(0),
	  m_hdc_ie(0),
	  m_rtc_irq(CLEAR_LINE),
	  m_via_irq(CLEAR_LINE),
	  m_hdc_irq(CLEAR_LINE),
	  m_fdc_irq(CLEAR_LINE),
	  m_fdc_drq(CLEAR_LINE),
	  m_adlc_irq(CLEAR_LINE),
	  m_clk_en(0)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void e01_device::device_config_complete()
{
	switch (m_variant)
	{
	default:
	case TYPE_E01:
		m_shortname = "e01";
		break;

	case TYPE_E01S:
		m_shortname = "e01s";
		break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e01_device::device_start()
{
	// floppy callbacks
	m_fdc->setup_intrq_cb(wd2793_t::line_cb(FUNC(e01_device::fdc_irq_w), this));
	m_fdc->setup_drq_cb(wd2793_t::line_cb(FUNC(e01_device::fdc_drq_w), this));

	// setup memory banking
	UINT8 *ram = m_ram->pointer();
	UINT8 *rom = memregion(R65C102_TAG)->base();

	membank("bank1")->configure_entry(0, ram);
	membank("bank1")->configure_entry(1, rom);
	membank("bank1")->set_entry(1);

	membank("bank2")->configure_entry(0, ram);
//  membank("bank2")->set_entry(0);

	membank("bank3")->configure_entry(0, ram + 0xfd00);
	membank("bank3")->configure_entry(1, rom + 0xfd00);
	membank("bank3")->set_entry(1);

	membank("bank4")->configure_entry(0, ram + 0xfd00);
	membank("bank4")->set_entry(0);

	// allocate timers
	m_clk_timer = timer_alloc();

	// register for state saving
	save_item(NAME(m_adlc_ie));
	save_item(NAME(m_hdc_ie));
	save_item(NAME(m_rtc_irq));
	save_item(NAME(m_via_irq));
	save_item(NAME(m_hdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_adlc_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e01_device::device_reset()
{
	m_clk_timer->adjust(attotime::zero, 0, attotime::from_hz(200000));

	membank("bank1")->set_entry(1);
	membank("bank3")->set_entry(1);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void e01_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (m_clk_en)
	{
		m_econet->clk_w(this, 1);
		m_econet->clk_w(this, 0);
	}
}


//-------------------------------------------------
//  eprom_r - ROM/RAM select read
//-------------------------------------------------

READ8_MEMBER( e01_device::ram_select_r )
{
	membank("bank1")->set_entry(0);
	membank("bank3")->set_entry(0);

	return 0;
}


//-------------------------------------------------
//  floppy_w - floppy control write
//-------------------------------------------------

WRITE8_MEMBER( e01_device::floppy_w )
{
	/*

        bit     description

        0       floppy 1 select
        1       floppy 2 select
        2       floppy side select
        3       NVRAM select
        4       floppy density
        5       floppy master reset
        6       floppy test
        7       mode LED

    */

	// floppy select
	floppy_image_device *floppy = NULL;

	if (!BIT(data, 0)) floppy = m_floppy0->get_device();
	if (!BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	// floppy side select
	if (floppy) floppy->ss_w(BIT(data, 2));

	// TODO NVRAM select
	//mc146818_stby_w(m_rtc, BIT(data, 3));

	// floppy density
	m_fdc->dden_w(BIT(data, 4));

	// floppy master reset
	if (!BIT(data, 5)) m_fdc->reset();

	// TODO floppy test

	// mode LED
	output_set_value("led_0", BIT(data, 7));
}


//-------------------------------------------------
//  network_irq_disable_r -
//-------------------------------------------------

READ8_MEMBER( e01_device::network_irq_disable_r )
{
	network_irq_enable(0);

	return 0;
}


//-------------------------------------------------
//  network_irq_disable_w -
//-------------------------------------------------

WRITE8_MEMBER( e01_device::network_irq_disable_w )
{
	network_irq_enable(0);
}


//-------------------------------------------------
//  network_irq_enable_r -
//-------------------------------------------------

READ8_MEMBER( e01_device::network_irq_enable_r )
{
	network_irq_enable(1);

	return 0;
}


//-------------------------------------------------
//  network_irq_enable_w -
//-------------------------------------------------

WRITE8_MEMBER( e01_device::network_irq_enable_w )
{
	network_irq_enable(1);
}


//-------------------------------------------------
//  hdc_data_r -
//-------------------------------------------------

READ8_MEMBER( e01_device::hdc_data_r )
{
	UINT8 data = m_scsibus->scsi_data_r(space, 0);

	m_scsibus->scsi_ack_w(1);

	return data;
}


//-------------------------------------------------
//  hdc_data_w -
//-------------------------------------------------

WRITE8_MEMBER( e01_device::hdc_data_w )
{
	m_scsibus->scsi_data_w(space, 0, data);

	m_scsibus->scsi_ack_w(1);
}


//-------------------------------------------------
//  hdc_status_r -
//-------------------------------------------------

READ8_MEMBER( e01_device::hdc_status_r )
{
	/*

        bit     description

        0       MSG
        1       BSY
        2       0
        3       0
        4       NIRQ
        5       REQ
        6       I/O
        7       C/D

    */

	UINT8 data = 0;

	// SCSI bus
	data |= m_scsibus->scsi_msg_r();
	data |= m_scsibus->scsi_bsy_r() << 1;
	data |= m_scsibus->scsi_req_r() << 5;
	data |= m_scsibus->scsi_io_r() << 6;
	data |= m_scsibus->scsi_cd_r() << 7;

	// TODO NIRQ

	return data;
}


//-------------------------------------------------
//  hdc_select_w -
//-------------------------------------------------

WRITE8_MEMBER( e01_device::hdc_select_w )
{
	m_scsibus->scsi_sel_w(1);
}


//-------------------------------------------------
//  hdc_irq_enable_w -
//-------------------------------------------------

WRITE8_MEMBER( e01_device::hdc_irq_enable_w )
{
	hdc_irq_enable(BIT(data, 0));
}


//-------------------------------------------------
//  rtc_address_r -
//-------------------------------------------------

READ8_MEMBER( e01_device::rtc_address_r )
{
	return m_rtc->read(space, 0);
}


//-------------------------------------------------
//  rtc_address_w -
//-------------------------------------------------

WRITE8_MEMBER( e01_device::rtc_address_w )
{
	m_rtc->write(space, 0, data);
}


//-------------------------------------------------
//  rtc_data_r -
//-------------------------------------------------

READ8_MEMBER( e01_device::rtc_data_r )
{
	return m_rtc->read(space, 1);
}


//-------------------------------------------------
//  rtc_data_w -
//-------------------------------------------------

WRITE8_MEMBER( e01_device::rtc_data_w )
{
	m_rtc->write(space, 1, data);
}


//-------------------------------------------------
//  econet_clk_w -
//-------------------------------------------------

void e01_device::econet_clk(int state)
{
	mc6854_rxc_w(m_adlc, state);
	mc6854_txc_w(m_adlc, state);
}
