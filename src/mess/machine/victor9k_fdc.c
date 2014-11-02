// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 floppy disk controller emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - everything

*/

#include "victor9k_fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0

#define I8048_TAG       "5d"
#define M6522_4_TAG     "1f"
#define M6522_5_TAG     "1k"
#define M6522_6_TAG     "1h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VICTOR_9000_FDC = &device_creator<victor_9000_fdc_t>;


//-------------------------------------------------
//  ROM( victor_9000_fdc )
//-------------------------------------------------

ROM_START( victor_9000_fdc )
	ROM_REGION( 0x400, I8048_TAG, 0)
	ROM_LOAD( "36080.5d", 0x000, 0x400, CRC(9bf49f7d) SHA1(b3a11bb65105db66ae1985b6f482aab6ea1da38b) )

	ROM_REGION( 0x800, "gcr", 0 )
	ROM_LOAD( "100836-001.4k", 0x000, 0x800, CRC(adc601bd) SHA1(6eeff3d2063ae2d97452101aa61e27ef83a467e5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *victor_9000_fdc_t::device_rom_region() const
{
	return ROM_NAME( victor_9000_fdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( floppy_io )
//-------------------------------------------------

static ADDRESS_MAP_START( floppy_io, AS_IO, 8, victor_9000_fdc_t )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(floppy_p1_r) AM_WRITENOP
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(floppy_p2_r, floppy_p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(tach0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(tach1_r)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_WRITE(da_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  SLOT_INTERFACE( victor9k_floppies )
//-------------------------------------------------

void victor_9000_fdc_t::ready0_cb(floppy_image_device *device, int state)
{
	m_rdy0 = state;

	m_via5->write_ca2(m_rdy0);
}

int victor_9000_fdc_t::load0_cb(floppy_image_device *device)
{
	m_ds0 = 0;

	m_via4->write_ca1(m_ds0);

	return IMAGE_INIT_PASS;
}

void victor_9000_fdc_t::unload0_cb(floppy_image_device *device)
{
	m_ds0 = 1;

	m_via4->write_ca1(m_ds0);
}

void victor_9000_fdc_t::ready1_cb(floppy_image_device *device, int state)
{
	m_rdy1 = state;

	m_via5->write_cb2(m_rdy1);
}

int victor_9000_fdc_t::load1_cb(floppy_image_device *device)
{
	m_ds1 = 0;

	m_via4->write_cb1(m_ds1);

	return IMAGE_INIT_PASS;
}

void victor_9000_fdc_t::unload1_cb(floppy_image_device *device)
{
	m_ds1 = 1;

	m_via4->write_cb1(m_ds1);
}

static SLOT_INTERFACE_START( victor9k_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( victor_9000_fdc_t::floppy_formats )
	FLOPPY_VICTOR_9000_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( victor_9000_fdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( victor_9000_fdc )
	MCFG_CPU_ADD(I8048_TAG, I8048, XTAL_30MHz/6)
	MCFG_CPU_IO_MAP(floppy_io)

	MCFG_DEVICE_ADD(M6522_4_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(victor_9000_fdc_t, via4_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(victor_9000_fdc_t, via4_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(victor_9000_fdc_t, mode_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via4_irq_w))

	MCFG_DEVICE_ADD(M6522_5_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via5_irq_w))

	MCFG_DEVICE_ADD(M6522_6_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_READPA_HANDLER(READ8(victor_9000_fdc_t, via6_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(victor_9000_fdc_t, via6_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(victor_9000_fdc_t, via6_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(victor_9000_fdc_t, via6_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(victor_9000_fdc_t, drw_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(victor_9000_fdc_t, erase_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via6_irq_w))

	MCFG_FLOPPY_DRIVE_ADD(I8048_TAG":0", victor9k_floppies, "525qd", victor_9000_fdc_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(I8048_TAG":1", victor9k_floppies, "525qd", victor_9000_fdc_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor victor_9000_fdc_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( victor_9000_fdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  victor_9000_fdc_t - constructor
//-------------------------------------------------

victor_9000_fdc_t::victor_9000_fdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VICTOR_9000_FDC, "Victor 9000 FDC", tag, owner, clock, "victor9k_fdc", __FILE__),
	m_irq_cb(*this),
	m_maincpu(*this, I8048_TAG),
	m_via4(*this, M6522_4_TAG),
	m_via5(*this, M6522_5_TAG),
	m_via6(*this, M6522_6_TAG),
	m_floppy0(*this, I8048_TAG":0:525qd"),
	m_floppy1(*this, I8048_TAG":1:525qd"),
	m_gcr_rom(*this, "gcr"),
	m_da(0),
	m_da0(0),
	m_da1(0),
	m_sel0(0),
	m_sel1(0),
	m_tach0(0),
	m_tach1(0),
	m_rdy0(0),
	m_rdy1(0),
	m_ds0(1),
	m_ds1(1),
	m_lms(0),
	m_st0(0),
	m_st1(0),
	m_stp0(0),
	m_stp1(0),
	m_drive(0),
	m_side(0),
	m_brdy(1),
	m_sync(1),
	m_gcrerr(0),
	m_via4_irq(CLEAR_LINE),
	m_via5_irq(CLEAR_LINE),
	m_via6_irq(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victor_9000_fdc_t::device_start()
{
	// state saving
	save_item(NAME(m_da));
	save_item(NAME(m_da0));
	save_item(NAME(m_da1));
	save_item(NAME(m_sel0));
	save_item(NAME(m_sel1));
	save_item(NAME(m_tach0));
	save_item(NAME(m_tach1));
	save_item(NAME(m_rdy0));
	save_item(NAME(m_rdy1));
	save_item(NAME(m_ds0));
	save_item(NAME(m_ds1));
	save_item(NAME(m_lms));
	save_item(NAME(m_st0));
	save_item(NAME(m_st1));
	save_item(NAME(m_stp0));
	save_item(NAME(m_stp1));
	save_item(NAME(m_drive));
	save_item(NAME(m_side));
	save_item(NAME(m_brdy));
	save_item(NAME(m_sync));
	save_item(NAME(m_gcrerr));
	save_item(NAME(m_via4_irq));
	save_item(NAME(m_via5_irq));
	save_item(NAME(m_via6_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victor_9000_fdc_t::device_reset()
{
	// resolve callbacks
	m_irq_cb.resolve_safe();

	// set floppy callbacks
	m_floppy0->setup_ready_cb(floppy_image_device::ready_cb(FUNC(victor_9000_fdc_t::ready0_cb), this));
	m_floppy0->setup_load_cb(floppy_image_device::load_cb(FUNC(victor_9000_fdc_t::load0_cb), this));
	m_floppy0->setup_unload_cb(floppy_image_device::unload_cb(FUNC(victor_9000_fdc_t::unload0_cb), this));
	m_floppy1->setup_ready_cb(floppy_image_device::ready_cb(FUNC(victor_9000_fdc_t::ready1_cb), this));
	m_floppy1->setup_load_cb(floppy_image_device::load_cb(FUNC(victor_9000_fdc_t::load1_cb), this));
	m_floppy1->setup_unload_cb(floppy_image_device::unload_cb(FUNC(victor_9000_fdc_t::unload1_cb), this));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void victor_9000_fdc_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}


//-------------------------------------------------
//  floppy_p1_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::floppy_p1_r )
{
	/*

	    bit     description

	    0       L0MS0
	    1       L0MS1
	    2       L0MS2
	    3       L0MS3
	    4       L1MS0
	    5       L1MS1
	    6       L1MS2
	    7       L1MS3

	*/

	return m_lms;
}


//-------------------------------------------------
//  floppy_p2_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::floppy_p2_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6       RDY0
	    7       RDY1

	*/

	UINT8 data = 0;

	data |= m_rdy0 << 6;
	data |= m_rdy1 << 7;

	return data;
}


//-------------------------------------------------
//  floppy_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_fdc_t::floppy_p2_w )
{
	/*

	    bit     description

	    0       START0
	    1       STOP0
	    2       START1
	    3       STOP1
	    4       SEL1
	    5       SEL0
	    6
	    7

	*/

	if (BIT(data, 0)) m_floppy0->mon_w(0);
	if (BIT(data, 1)) m_floppy0->mon_w(1);
	if (BIT(data, 2)) m_floppy1->mon_w(0);
	if (BIT(data, 3)) m_floppy1->mon_w(1);

	int sel0 = BIT(data, 5);

	if (m_sel0 && !sel0)
	{
		m_da0 = m_da;
		//m_floppy0->set_rpm();
	}

	m_sel0 = sel0;

	int sel1 = BIT(data, 4);

	if (m_sel1 && !sel1)
	{
		m_da1 = m_da;
		//m_floppy1->set_rpm();
	}

	m_sel1 = sel1;
}


//-------------------------------------------------
//  tach0_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::tach0_r )
{
	return m_tach0;
}


//-------------------------------------------------
//  tach1_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::tach1_r )
{
	return m_tach1;
}


//-------------------------------------------------
//  da_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_fdc_t::da_w )
{
	m_da = data;
}

WRITE8_MEMBER( victor_9000_fdc_t::via4_pa_w )
{
	/*

	    bit     description

	    PA0     L0MS0
	    PA1     L0MS1
	    PA2     L0MS2
	    PA3     L0MS3
	    PA4     ST0A
	    PA5     ST0B
	    PA6     ST0C
	    PA7     ST0D

	*/

	m_lms = (m_lms & 0xf0) | (data & 0x0f);
	m_st0 = data >> 4;
}

WRITE8_MEMBER( victor_9000_fdc_t::via4_pb_w )
{
	/*

	    bit     description

	    PB0     L1MS0
	    PB1     L1MS1
	    PB2     L1MS2
	    PB3     L1MS3
	    PB4     ST1A
	    PB5     ST1B
	    PB6     ST1C
	    PB7     ST1D

	*/

	m_lms = (data << 4) | (m_lms & 0x0f);
	m_st1 = data >> 4;
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::mode_w )
{
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via4_irq_w )
{
	m_via4_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}


/*

    bit     description

    PA0     E0
    PA1     E1
    PA2     I1
    PA3     E2
    PA4     E4
    PA5     E5
    PA6     I7
    PA7     E6

*/

WRITE8_MEMBER( victor_9000_fdc_t::via5_pb_w )
{
	/*

	    bit     description

	    PB0     WD0
	    PB1     WD1
	    PB2     WD2
	    PB3     WD3
	    PB4     WD4
	    PB5     WD5
	    PB6     WD6
	    PB7     WD7

	*/
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via5_irq_w )
{
	m_via5_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}


READ8_MEMBER( victor_9000_fdc_t::via6_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1     _TRK0D0
	    PA2
	    PA3     _TRK0D1
	    PA4
	    PA5
	    PA6     WPS
	    PA7     _SYNC

	*/

	UINT8 data = 0;

	// track 0 drive A sense
	data |= m_floppy0->trk00_r() << 1;

	// track 0 drive B sense
	data |= m_floppy1->trk00_r() << 3;

	// write protect sense
	data |= (m_drive ? m_floppy1->wpt_r() : m_floppy0->wpt_r()) << 6;

	// disk sync detect
	data |= m_sync << 7;

	return data;
}

WRITE8_MEMBER( victor_9000_fdc_t::via6_pa_w )
{
	/*

	    bit     description

	    PA0     LED0A
	    PA1
	    PA2     LED1A
	    PA3
	    PA4     SIDE SELECT
	    PA5     DRIVE SELECT
	    PA6
	    PA7

	*/

	// LED, drive A
	output_set_led_value(LED_A, BIT(data, 0));

	// LED, drive B
	output_set_led_value(LED_B, BIT(data, 2));

	// dual side select
	m_side = BIT(data, 4);

	// select drive A/B
	m_drive = BIT(data, 5);
}

READ8_MEMBER( victor_9000_fdc_t::via6_pb_r )
{
	/*

	    bit     description

	    PB0     RDY0
	    PB1     RDY1
	    PB2
	    PB3     _DS1
	    PB4     _DS0
	    PB5     SINGLE/_DOUBLE SIDED
	    PB6
	    PB7

	*/

	UINT8 data = 0;

	// motor speed status, drive A
	data |= m_rdy0;

	// motor speed status, drive B
	data |= m_rdy1 << 1;

	// door B sense
	data |= m_ds1 << 3;

	// door A sense
	data |= m_ds0 << 4;

	// single/double sided
	data |= (m_drive ? m_floppy1->twosid_r() : m_floppy0->twosid_r()) << 5;

	return data;
}

WRITE8_MEMBER( victor_9000_fdc_t::via6_pb_w )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2     _SCRESET
	    PB3
	    PB4
	    PB5
	    PB6     STP0
	    PB7     STP1

	*/

	// motor speed controller reset
	if (!BIT(data, 2))
		m_maincpu->reset();

	// stepper enable A
	m_stp0 = BIT(data, 6);

	// stepper enable B
	m_stp1 = BIT(data, 7);
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::drw_w )
{
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::erase_w )
{
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via6_irq_w )
{
	m_via6_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}
