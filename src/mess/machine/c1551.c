/**********************************************************************

    Commodore 1551 Single Disk Drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

	- paddle expansion port passthru
    - byte latching does not match hardware behavior
      (CPU skips data bytes if implemented per schematics)

*/

#include "c1541.h"
#include "c1551.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6510T_TAG		"u2"
#define M6523_0_TAG		"u3"
#define M6523_1_TAG		"ci_u2"
#define C64H156_TAG		"u6"
#define PLA_TAG			"u1"

enum
{
	LED_POWER = 0,
	LED_ACT
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C1551 = &device_creator<c1551_device>;


//-------------------------------------------------
//  ROM( c1551 )
//-------------------------------------------------

ROM_START( c1551 ) // schematic 251860
	ROM_REGION( 0x4000, M6510T_TAG, 0 )
	ROM_LOAD( "318001-01.u4", 0x0000, 0x4000, CRC(6d16d024) SHA1(fae3c788ad9a6cc2dbdfbcf6c0264b2ca921d55e) )

	ROM_REGION( 0xf5, PLA_TAG, 0 ) // schematic 251925
	ROM_LOAD( "251641-03.u1", 0x00, 0xf5, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c1551_device::device_rom_region() const
{
	return ROM_NAME( c1551 );
}


//-------------------------------------------------
//  m6502_interface m6510t_intf
//-------------------------------------------------

READ8_MEMBER( c1551_device::port_r )
{
	/*

        bit     description

        P0
        P1
        P2
        P3
        P4      WPS
        P5
        P6
        P7      BYTE LTCHED

    */

	UINT8 data = 0;

	// write protect sense
	data |= !floppy_wpt_r(m_image) << 4;

	// byte latched
	data |= m_ga->atn_r() << 7;

	return data;
}

WRITE8_MEMBER( c1551_device::port_w )
{
	/*

        bit     description

        P0      STP0A
        P1      STP0B
        P2      MTR0
        P3      ACT0
        P4
        P5      DS0
        P6      DS1
        P7

    */

	// spindle motor
	m_ga->mtr_w(BIT(data, 2));

	// stepper motor
	m_ga->stp_w(data & 0x03);

	// activity LED
	output_set_led_value(LED_ACT, BIT(data, 3));

	// density select
	m_ga->ds_w((data >> 5) & 0x03);
}

static const m6502_interface m6510t_intf =
{
	NULL,			// read_indexed_func
	NULL,			// write_indexed_func
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, port_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, port_w)
};


//-------------------------------------------------
//  tpi6525_interface tpi0_intf
//-------------------------------------------------

READ8_MEMBER( c1551_device::tcbm_data_r )
{
	/*

        bit     description

        PA0     TCBM PA0
        PA1     TCBM PA1
        PA2     TCBM PA2
        PA3     TCBM PA3
        PA4     TCBM PA4
        PA5     TCBM PA5
        PA6     TCBM PA6
        PA7     TCBM PA7

    */

	return m_tcbm_data;
}

WRITE8_MEMBER( c1551_device::tcbm_data_w )
{
	/*

        bit     description

        PA0     TCBM PA0
        PA1     TCBM PA1
        PA2     TCBM PA2
        PA3     TCBM PA3
        PA4     TCBM PA4
        PA5     TCBM PA5
        PA6     TCBM PA6
        PA7     TCBM PA7

    */

	m_tcbm_data = data;
}

READ8_MEMBER( c1551_device::tpi0_pc_r )
{
	/*

        bit     description

        PC0
        PC1
        PC2
        PC3
        PC4
        PC5     JP1
        PC6     _SYNC
        PC7     TCBM DAV

    */

	UINT8 data = 0;

	// JP1
	data |= ioport("JP1")->read() << 5;

	// SYNC detect line
	data |= m_ga->sync_r() << 6;

	// TCBM data valid
	data |= m_dav << 7;

	return data;
}

WRITE8_MEMBER( c1551_device::tpi0_pc_w )
{
	/*

        bit     description

        PC0     TCBM STATUS0
        PC1     TCBM STATUS1
        PC2     TCBM DEV
        PC3     TCBM ACK
        PC4     MODE
        PC5
        PC6
        PC7

    */

	// TCBM status
	m_status = data & 0x03;

	// TCBM device number
	m_dev = BIT(data, 2);

	// TCBM acknowledge
	m_ack = BIT(data, 3);

	// read/write mode
	m_ga->oe_w(BIT(data, 4));
}

static const tpi6525_interface tpi0_intf =
{
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tcbm_data_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tcbm_data_w),
	DEVCB_DEVICE_MEMBER(C64H156_TAG, c64h156_device, yb_r),
	DEVCB_DEVICE_MEMBER(C64H156_TAG, c64h156_device, yb_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tpi0_pc_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tpi0_pc_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  tpi6525_interface tpi1_intf
//-------------------------------------------------

READ8_MEMBER( c1551_device::tpi1_pb_r )
{
	/*

        bit     description

        PB0     STATUS0
        PB1     STATUS1
        PB2
        PB3
        PB4
        PB5
        PB6
        PB7

    */

	return m_status & 0x03;
}

READ8_MEMBER( c1551_device::tpi1_pc_r )
{
	/*

        bit     description

        PC0
        PC1
        PC2
        PC3
        PC4
        PC5
        PC6
        PC7     TCBM ACK

    */

	UINT8 data = 0;

	// TCBM acknowledge
	data |= m_ack << 7;

	return data;
}

WRITE8_MEMBER( c1551_device::tpi1_pc_w )
{
	/*

        bit     description

        PC0
        PC1
        PC2
        PC3
        PC4
        PC5
        PC6     TCBM DAV
        PC7

    */

	// TCBM data valid
	m_dav = BIT(data, 6);
}

static const tpi6525_interface tpi1_intf =
{
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tcbm_data_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tcbm_data_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tpi1_pb_r),
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tpi1_pc_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, c1551_device, tpi1_pc_w),
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  ADDRESS_MAP( c1551_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c1551_mem, AS_PROGRAM, 8, c1551_device )
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x0800) AM_RAM
	AM_RANGE(0x4000, 0x4007) AM_MIRROR(0x3ff8) AM_DEVREADWRITE_LEGACY(M6523_0_TAG, tpi6525_r, tpi6525_w)
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(M6510T_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  C64H156_INTERFACE( ga_intf )
//-------------------------------------------------

static C64H156_INTERFACE( ga_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(C64H156_TAG, c64h156_device, atni_w)
};


//-------------------------------------------------
//  MACHINE_DRIVER( c1551 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c1551 )
	MCFG_CPU_ADD(M6510T_TAG, M6510T, XTAL_16MHz/8)
	MCFG_CPU_PROGRAM_MAP(c1551_mem)
	MCFG_CPU_CONFIG(m6510t_intf)
	MCFG_QUANTUM_PERFECT_CPU(M6510T_TAG)

	MCFG_PLS100_ADD(PLA_TAG)
	MCFG_TPI6525_ADD(M6523_0_TAG, tpi0_intf)
	MCFG_TPI6525_ADD(M6523_1_TAG, tpi1_intf)

	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, c1541_floppy_interface)
	MCFG_64H156_ADD(C64H156_TAG, XTAL_16MHz, ga_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c1551_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c1551 );
}


//-------------------------------------------------
//  INPUT_PORTS( c1551 )
//-------------------------------------------------

static INPUT_PORTS_START( c1551 )
	PORT_START("JP1")
	PORT_DIPNAME( 0x01, 0x00, "Device Number" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1551_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1551 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1551_device - constructor
//-------------------------------------------------

c1551_device::c1551_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, C1551, "C1551", tag, owner, clock),
      device_plus4_expansion_card_interface(mconfig, *this),
	  m_maincpu(*this, M6510T_TAG),
	  m_tpi0(*this, M6523_0_TAG),
	  m_tpi1(*this, M6523_1_TAG),
	  m_ga(*this, C64H156_TAG),
	  m_pla(*this, PLA_TAG),
	  m_image(*this, FLOPPY_0),
	  m_tcbm_data(0xff),
	  m_status(1),
	  m_dav(1),
	  m_ack(1),
	  m_dev(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1551_device::device_start()
{
	// allocate timers
	m_irq_timer = timer_alloc();
	m_irq_timer->adjust(attotime::zero, CLEAR_LINE);

	// install image callbacks
	floppy_install_unload_proc(m_image, c1551_device::on_disk_change);
	floppy_install_load_proc(m_image, c1551_device::on_disk_change);

	// register for state saving
	save_item(NAME(m_tcbm_data));
	save_item(NAME(m_status));
	save_item(NAME(m_dav));
	save_item(NAME(m_ack));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1551_device::device_reset()
{
	m_maincpu->reset();

	m_tpi0->reset();

	// initialize gate array
	m_ga->test_w(1);
	m_ga->soe_w(1);
	m_ga->accl_w(1);
	m_ga->atna_w(1);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void c1551_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_maincpu->set_input_line(M6502_IRQ_LINE, param);

	if (param == ASSERT_LINE)
	{
		// Ts = 0.7*R2*C1 = 0.7*100R*0.1uF = 7us
		m_irq_timer->adjust(attotime::from_usec(7), CLEAR_LINE);
	}
	else
	{
		// Tm = 0.7*(R1+R2)*C1 = 0.7*(120K+100R)*0.1uF = 0.008407s
		m_irq_timer->adjust(attotime::from_usec(8407), ASSERT_LINE);
	}
}


//-------------------------------------------------
//  tpi1_selected -
//-------------------------------------------------

bool c1551_device::tpi1_selected(offs_t offset)
{
#ifdef PLA_DUMPED
	int mux = 0, ras = 0, phi0 = 0, f7 = 0;
	UINT16 input = A5 << 15 | A6 << 14 | A7 << 13 | A8 << 12 | A9 << 11 | mux << 10 | A10 << 9 | m_dev << 8 | ras << 7 | phi0 << 6 | A15 << 5 | A14 << 4 | A13 << 3 | A12 << 2 | A11 << 1 | f7;
	UINT8 data = m_pla->read(input);
	return BIT(data, 0) ? true : false;
#endif

	offs_t start_address = m_dev ? 0xfee0 : 0xfec0;

	if (offset >= start_address && offset < (start_address + 0x20))
	{
		return true;
	}

	return false;
}


//-------------------------------------------------
//  plus4_cd_r - cartridge data read
//-------------------------------------------------

UINT8 c1551_device::plus4_cd_r(address_space &space, offs_t offset, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	UINT8 data = 0;

	if (tpi1_selected(offset))
	{
		data = tpi6525_r(m_tpi1, offset & 0x07);
	}

	return data;
}


//-------------------------------------------------
//  plus4_cd_w - cartridge data write
//-------------------------------------------------

void c1551_device::plus4_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h)
{
	if (tpi1_selected(offset))
	{
		tpi6525_w(m_tpi1, offset & 0x07, data);
	}
}


//-------------------------------------------------
//  plus4_breset_w - buffered reset write
//-------------------------------------------------

void c1551_device::plus4_breset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  on_disk_change -
//-------------------------------------------------

void c1551_device::on_disk_change(device_image_interface &image)
{
    c1551_device *c1551 = static_cast<c1551_device *>(image.device().owner());

    int wp = floppy_wpt_r(image);
	c1551->m_ga->on_disk_changed(wp);
}
