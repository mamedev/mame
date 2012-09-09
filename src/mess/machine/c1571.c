/**********************************************************************

    Commodore 1570/1571/1571CR Single Disk Drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - fast serial
    - 1541/1571 Alignment shows drive speed as 266 rpm, should be 310
    - CP/M disks

*/

#include "c1541.h"
#include "c1571.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG		"u1"
#define M6522_0_TAG		"u9"
#define M6522_1_TAG		"u4"
#define M6526_TAG		"u20"
#define WD1770_TAG		"u11"
#define C64H156_TAG		"u6"


enum
{
	LED_POWER = 0,
	LED_ACT
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C1570 = &device_creator<c1570_device>;
const device_type C1571 = &device_creator<c1571_device>;
const device_type C1571CR = &device_creator<c1571cr_device>;
const device_type MINI_CHIEF = &device_creator<mini_chief_device>;


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void base_c1571_device::device_config_complete()
{
	switch (m_variant)
	{
	case TYPE_1570:
		m_shortname = "c1570";
		break;

	default:
	case TYPE_1571:
		m_shortname = "c1571";
		break;

	case TYPE_1571CR:
		m_shortname = "c1571cr";
		break;

	case TYPE_MINI_CHIEF:
		m_shortname = "minichif";
		break;
	}
}


//-------------------------------------------------
//  ROM( c1570 )
//-------------------------------------------------

ROM_START( c1570 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "315090-01.u2", 0x0000, 0x8000, CRC(5a0c7937) SHA1(5fc06dc82ff6840f183bd43a4d9b8a16956b2f56) )
ROM_END


//-------------------------------------------------
//  ROM( c1571 )
//-------------------------------------------------

ROM_START( c1571 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
    ROM_DEFAULT_BIOS("r5")
    ROM_SYSTEM_BIOS( 0, "r3", "Revision 3" )
	ROMX_LOAD( "310654-03.u2", 0x0000, 0x8000, CRC(3889b8b8) SHA1(e649ef4419d65829d2afd65e07d31f3ce147d6eb), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r5", "Revision 5" )
	ROMX_LOAD( "310654-05.u2", 0x0000, 0x8000, CRC(5755bae3) SHA1(f1be619c106641a685f6609e4d43d6fc9eac1e70), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1571.u2", 0x0000, 0x8000, CRC(fe6cac6d) SHA1(d4b79b60cf1eaa399d0932200eb7811e00455249), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  ROM( c1571cr )
//-------------------------------------------------

ROM_START( c1571cr )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("cbm")
    ROM_SYSTEM_BIOS( 0, "cbm", "Commodore" )
	ROMX_LOAD( "318047-01.u102", 0x0000, 0x8000, CRC(f24efcc4) SHA1(14ee7a0fb7e1c59c51fbf781f944387037daa3ee), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1571d.u102", 0x0000, 0x8000, CRC(9cba146d) SHA1(823b178561302b403e6bfd8dd741d757efef3958), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  ROM( minichief )
//-------------------------------------------------

ROM_START( minichief )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "ictdos710.u2", 0x0000, 0x8000, CRC(aaacf7e9) SHA1(c1296995238ef23f18e7fec70a144a0566a25a27) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *base_c1571_device::device_rom_region() const
{
	switch (m_variant)
	{
	case TYPE_1570:
		return ROM_NAME( c1570 );

	default:
	case TYPE_1571:
		return ROM_NAME( c1571 );

	case TYPE_1571CR:
		return ROM_NAME( c1571cr );

	case TYPE_MINI_CHIEF:
		return ROM_NAME( minichief );
	}
}


//-------------------------------------------------
//  ADDRESS_MAP( c1571_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c1571_mem, AS_PROGRAM, 8, base_c1571_device )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1800, 0x180f) AM_MIRROR(0x03f0) AM_DEVREADWRITE(M6522_0_TAG, via6522_device, read, write)
	AM_RANGE(0x1c00, 0x1c0f) AM_MIRROR(0x03f0) AM_DEVREADWRITE(M6522_1_TAG, via6522_device, read, write)
	AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE_LEGACY(WD1770_TAG, wd17xx_r, wd17xx_w)
	AM_RANGE(0x4000, 0x400f) AM_MIRROR(0x3ff0) AM_DEVREADWRITE_LEGACY(M6526_TAG, mos6526_r, mos6526_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  via6522_interface via0_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( base_c1571_device::via0_irq_w )
{
	m_via0_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER( base_c1571_device::via0_pa_r )
{
	/*

        bit     description

        PA0     TRK0 SNS
        PA1
        PA2
        PA3
        PA4
        PA5
        PA6
        PA7     BYTE RDY

    */

	UINT8 data = 0;

	// track 0 sense
	data |= floppy_tk00_r(m_image);

	// byte ready
	data |= m_ga->byte_r() << 7;

	return data;
}

WRITE8_MEMBER( base_c1571_device::via0_pa_w )
{
	/*

        bit     description

        PA0
        PA1     SER DIR
        PA2     SIDE
        PA3
        PA4
        PA5     _1/2 MHZ
        PA6     ATN OUT
        PA7

    */

	// 1/2 MHz
	int clock_1_2 = BIT(data, 5);

	if (m_1_2mhz != clock_1_2)
	{
		UINT32 clock = clock_1_2 ? XTAL_16MHz/8 : XTAL_16MHz/16;

		m_maincpu->set_unscaled_clock(clock);
		m_cia->set_unscaled_clock(clock);
		m_via0->set_unscaled_clock(clock);
		m_via1->set_unscaled_clock(clock);
		m_ga->accl_w(clock_1_2);

		m_1_2mhz = clock_1_2;
	}

	// fast serial direction
	int ser_dir = BIT(data, 1);

	if (m_ser_dir != ser_dir)
	{
		m_ser_dir = ser_dir;

		set_iec_data();
		set_iec_srq();

		m_cia->cnt_w(m_ser_dir || m_bus->srq_r());
		m_cia->sp_w(m_ser_dir || m_bus->data_r());
	}

	// side select
	m_ga->set_side(BIT(data, 2));

	// attention out
	m_bus->atn_w(this, !BIT(data, 6));
}

READ8_MEMBER( base_c1571_device::via0_pb_r )
{
	/*

        bit     description

        PB0     DATA IN
        PB1
        PB2     CLK IN
        PB3
        PB4
        PB5     DEV# SEL
        PB6     DEV# SEL
        PB7     ATN IN

    */

	UINT8 data = 0;

	// data in
	data = !m_bus->data_r();

	// clock in
	data |= !m_bus->clk_r() << 2;

	// serial bus address
	data |= (m_address - 8) << 5;

	// attention in
	data |= !m_bus->atn_r() << 7;

	return data;
}

WRITE8_MEMBER( base_c1571_device::via0_pb_w )
{
	/*

        bit     description

        PB0
        PB1     DATA OUT
        PB2
        PB3     CLK OUT
        PB4     ATN ACK
        PB5
        PB6
        PB7

    */

	// data out
	m_data_out = BIT(data, 1);

	// attention acknowledge
	m_ga->atna_w(BIT(data, 4));

	// clock out
	m_bus->clk_w(this, !BIT(data, 3));
}

READ_LINE_MEMBER( base_c1571_device::atn_in_r )
{
	return !m_bus->atn_r();
}

READ_LINE_MEMBER( base_c1571_device::wprt_r )
{
	return !floppy_wpt_r(m_image);
}

static const via6522_interface via0_intf =
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, via0_pa_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, via0_pb_r),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, atn_in_r),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, wprt_r),
	DEVCB_NULL,

	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, via0_pa_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, via0_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, via0_irq_w)
};


//-------------------------------------------------
//  via6522_interface via1_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( base_c1571_device::via1_irq_w )
{
	m_via1_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER( base_c1571_device::via1_pb_r )
{
	/*

        bit     signal      description

        PB0
        PB1
        PB2
        PB3
        PB4     _WPRT       write protect sense
        PB5
        PB6
        PB7     _SYNC       SYNC detect line

    */

	UINT8 data = 0;

	// write protect sense
	data |= !floppy_wpt_r(m_image) << 4;

	// SYNC detect line
	data |= m_ga->sync_r() << 7;

	return data;
}

WRITE8_MEMBER( base_c1571_device::via1_pb_w )
{
	/*

        bit     signal      description

        PB0     STP0        stepping motor bit 0
        PB1     STP1        stepping motor bit 1
        PB2     MTR         motor ON/OFF
        PB3     ACT         drive 0 LED
        PB4
        PB5     DS0         density select 0
        PB6     DS1         density select 1
        PB7

    */

	// spindle motor
	m_ga->mtr_w(BIT(data, 2));

	// stepper motor
	m_ga->stp_w(data & 0x03); // TODO actually STP1=0, STP0=!(PB0^PB1), Y0=PB1, Y2=!PB1

	// activity LED
	output_set_led_value(LED_ACT, BIT(data, 3));

	// density select
	m_ga->ds_w((data >> 5) & 0x03);
}

static const via6522_interface via1_intf =
{
	DEVCB_DEVICE_MEMBER(C64H156_TAG, c64h156_device, yb_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, via1_pb_r),
	DEVCB_DEVICE_LINE_MEMBER(C64H156_TAG, c64h156_device, byte_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_DEVICE_MEMBER(C64H156_TAG, c64h156_device, yb_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, via1_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(C64H156_TAG, c64h156_device, soe_w),
	DEVCB_DEVICE_LINE_MEMBER(C64H156_TAG, c64h156_device, oe_w),

	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, via1_irq_w)
};


//-------------------------------------------------
//  mos6526_interface cia_intf
//-------------------------------------------------

WRITE_LINE_MEMBER( base_c1571_device::cia_irq_w )
{
	m_cia_irq = state;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_via0_irq || m_via1_irq || m_cia_irq) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( base_c1571_device::cia_pc_w )
{
	if (m_other != NULL)
    {
    	m_other->parallel_strobe_w(state);
    }
}

WRITE_LINE_MEMBER( base_c1571_device::cia_cnt_w )
{
	// fast serial clock out
	m_cnt_out = state;
	set_iec_srq();
}

WRITE_LINE_MEMBER( base_c1571_device::cia_sp_w )
{
	// fast serial data out
	m_sp_out = state;
	set_iec_data();
}

READ8_MEMBER( base_c1571_device::cia_pb_r )
{
	return m_parallel_data;
}

WRITE8_MEMBER( base_c1571_device::cia_pb_w )
{
	if (m_other != NULL)
    {
		m_other->parallel_data_w(data);
	}
}

static MOS6526_INTERFACE( cia_intf )
{
	0,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, cia_irq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, cia_pc_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, cia_cnt_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, cia_sp_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, cia_pb_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, cia_pb_w)
};


//-------------------------------------------------
//  C64H156_INTERFACE( ga_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( base_c1571_device::atn_w )
{
	set_iec_data();
}

WRITE_LINE_MEMBER( base_c1571_device::byte_w )
{
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, state);

	m_via1->write_ca1(state);
}

static C64H156_INTERFACE( ga_intf )
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, atn_w),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, byte_w)
};


//-------------------------------------------------
//  wd17xx_interface fdc_intf
//-------------------------------------------------

static const wd17xx_interface fdc_intf =
{
	DEVCB_LINE_GND,
	DEVCB_NULL,
	DEVCB_NULL,
	{ FLOPPY_0, NULL, NULL, NULL }
};


//-------------------------------------------------
//  LEGACY_FLOPPY_OPTIONS( c1571 )
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( c1571 )
	LEGACY_FLOPPY_OPTION( c1571, "g64", "Commodore 1541 GCR Disk Image", g64_dsk_identify, g64_dsk_construct, NULL, NULL )
	LEGACY_FLOPPY_OPTION( c1571, "d64", "Commodore 1541 Disk Image", d64_dsk_identify, d64_dsk_construct, NULL, NULL )
	LEGACY_FLOPPY_OPTION( c1571, "d71", "Commodore 1571 Disk Image", d71_dsk_identify, d64_dsk_construct, NULL, NULL )
LEGACY_FLOPPY_OPTIONS_END


//-------------------------------------------------
//  floppy_interface c1571_floppy_interface
//-------------------------------------------------

WRITE_LINE_MEMBER( base_c1571_device::wpt_w )
{
	m_via0->write_ca2(!state);
}

static const floppy_interface c1571_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, wpt_w),
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(c1571),
	"floppy_5_25",
	NULL
};


//-------------------------------------------------
//  floppy_interface c1570_floppy_interface
//-------------------------------------------------

static const floppy_interface c1570_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1571_device, wpt_w),
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(c1541),
	"floppy_5_25",
	NULL
};


//-------------------------------------------------
//  MACHINE_DRIVER( c1571 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c1571 )
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c1571_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

	MCFG_VIA6522_ADD(M6522_0_TAG, XTAL_16MHz/16, via0_intf)
	MCFG_VIA6522_ADD(M6522_1_TAG, XTAL_16MHz/16, via1_intf)
	MCFG_MOS6526R1_ADD(M6526_TAG, XTAL_16MHz/16, cia_intf)
	MCFG_WD1770_ADD(WD1770_TAG, /* XTAL_16MHz/2, */ fdc_intf)

	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, c1571_floppy_interface)
	MCFG_64H156_ADD(C64H156_TAG, XTAL_16MHz, ga_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_DRIVER( c1570 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c1570 )
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c1571_mem)
	MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

	MCFG_VIA6522_ADD(M6522_0_TAG, XTAL_16MHz/16, via0_intf)
	MCFG_VIA6522_ADD(M6522_1_TAG, XTAL_16MHz/16, via1_intf)
	MCFG_MOS6526R1_ADD(M6526_TAG, XTAL_16MHz/16, cia_intf)
	MCFG_WD1770_ADD(WD1770_TAG, /* XTAL_16MHz/2, */ fdc_intf)

	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, c1570_floppy_interface)
	MCFG_64H156_ADD(C64H156_TAG, XTAL_16MHz, ga_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor base_c1571_device::device_mconfig_additions() const
{
	switch (m_variant)
	{
	case TYPE_1570:
		return MACHINE_CONFIG_NAME( c1570 );

	default:
		return MACHINE_CONFIG_NAME( c1571 );
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  base_c1571_device - constructor
//-------------------------------------------------

inline void base_c1571_device::set_iec_data()
{
	int data = !m_data_out && !m_ga->atn_r();

	// fast serial data
	if (m_ser_dir) data &= m_sp_out;

	m_bus->data_w(this, data);
}


//-------------------------------------------------
//  base_c1571_device - constructor
//-------------------------------------------------

inline void base_c1571_device::set_iec_srq()
{
	int srq = 1;

	// fast serial clock
	if (m_ser_dir) srq &= m_cnt_out;

	m_bus->srq_w(this, srq);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  base_c1571_device - constructor
//-------------------------------------------------

base_c1571_device::base_c1571_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant)
    : device_t(mconfig, type, name, tag, owner, clock),
	  device_cbm_iec_interface(mconfig, *this),
      device_c64_floppy_parallel_interface(mconfig, *this),
	  m_maincpu(*this, M6502_TAG),
	  m_via0(*this, M6522_0_TAG),
	  m_via1(*this, M6522_1_TAG),
	  m_cia(*this, M6526_TAG),
	  m_fdc(*this, WD1770_TAG),
	  m_ga(*this, C64H156_TAG),
	  m_image(*this, FLOPPY_0),
	  m_1_2mhz(0),
	  m_data_out(1),
	  m_ser_dir(0),
	  m_sp_out(1),
	  m_cnt_out(1),
	  m_via0_irq(CLEAR_LINE),
	  m_via1_irq(CLEAR_LINE),
	  m_cia_irq(CLEAR_LINE),
	  m_variant(variant)
{
}


//-------------------------------------------------
//  c1570_device - constructor
//-------------------------------------------------

c1570_device::c1570_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c1571_device(mconfig, C1570, "C1570", tag, owner, clock, TYPE_1570) { }


//-------------------------------------------------
//  c1571_device - constructor
//-------------------------------------------------

c1571_device::c1571_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c1571_device(mconfig, C1571, "C1571", tag, owner, clock, TYPE_1571) { }


//-------------------------------------------------
//  c1571cr_device - constructor
//-------------------------------------------------

c1571cr_device::c1571cr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c1571_device(mconfig, C1571CR, "C1571CR", tag, owner, clock, TYPE_1571CR) { }


//-------------------------------------------------
//  mini_chief_device - constructor
//-------------------------------------------------

mini_chief_device::mini_chief_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c1571_device(mconfig, MINI_CHIEF, "ICT Mini Chief", tag, owner, clock, TYPE_MINI_CHIEF) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void base_c1571_device::device_start()
{
	// install image callbacks
	floppy_install_unload_proc(m_image, base_c1571_device::on_disk_change);
	floppy_install_load_proc(m_image, base_c1571_device::on_disk_change);

	// register for state saving
	save_item(NAME(m_1_2mhz));
	save_item(NAME(m_data_out));
	save_item(NAME(m_ser_dir));
	save_item(NAME(m_sp_out));
	save_item(NAME(m_cnt_out));
	save_item(NAME(m_via0_irq));
	save_item(NAME(m_via1_irq));
	save_item(NAME(m_cia_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void base_c1571_device::device_reset()
{
    m_maincpu->reset();

    m_via0->reset();
    m_via1->reset();
    m_cia->reset();

	wd17xx_mr_w(m_fdc, 0);
	wd17xx_mr_w(m_fdc, 1);

	m_sp_out = 1;
	set_iec_data();

	m_cnt_out = 1;
	set_iec_srq();
}


//-------------------------------------------------
//  cbm_iec_srq -
//-------------------------------------------------

void base_c1571_device::cbm_iec_srq(int state)
{
	m_cia->cnt_w(m_ser_dir || state);
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void base_c1571_device::cbm_iec_atn(int state)
{
	m_via0->write_ca1(!state);
	m_ga->atni_w(!state);

	set_iec_data();
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void base_c1571_device::cbm_iec_data(int state)
{
	m_cia->sp_w(m_ser_dir || state);
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void base_c1571_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  parallel_data_w -
//-------------------------------------------------

void base_c1571_device::parallel_data_w(UINT8 data)
{
    m_parallel_data = data;
}


//-------------------------------------------------
//  parallel_strobe_w -
//-------------------------------------------------

void base_c1571_device::parallel_strobe_w(int state)
{
    m_cia->flag_w(state);
}


//-------------------------------------------------
//  on_disk_change -
//-------------------------------------------------

void base_c1571_device::on_disk_change(device_image_interface &image)
{
    base_c1571_device *c1571 = static_cast<base_c1571_device *>(image.device().owner());

    int wp = floppy_wpt_r(image);
	c1571->m_ga->on_disk_changed(wp);
}
