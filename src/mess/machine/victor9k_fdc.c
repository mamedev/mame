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

	m_rdy0_cb(state);
}

int victor_9000_fdc_t::load0_cb(floppy_image_device *device)
{
	m_ds0 = 0;

	m_ds0_cb(0);

	return IMAGE_INIT_PASS;
}

void victor_9000_fdc_t::unload0_cb(floppy_image_device *device)
{
	m_ds0 = 1;

	m_ds0_cb(1);
}

void victor_9000_fdc_t::ready1_cb(floppy_image_device *device, int state)
{
	m_rdy1 = state;

	m_rdy1_cb(state);
}

int victor_9000_fdc_t::load1_cb(floppy_image_device *device)
{
	m_ds1 = 0;

	m_ds1_cb(0);

	return IMAGE_INIT_PASS;
}

void victor_9000_fdc_t::unload1_cb(floppy_image_device *device)
{
	m_ds1 = 1;

	m_ds1_cb(1);
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
	m_ds0_cb(*this),
	m_ds1_cb(*this),
	m_rdy0_cb(*this),
	m_rdy1_cb(*this),
	m_brdy_cb(*this),
	m_gcrerr_cb(*this),
	m_maincpu(*this, I8048_TAG),
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
	m_gcrerr(0)
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
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victor_9000_fdc_t::device_reset()
{
	// resolve callbacks
	m_ds0_cb.resolve_safe();
	m_ds1_cb.resolve_safe();
	m_rdy0_cb.resolve_safe();
	m_rdy1_cb.resolve_safe();
	m_brdy_cb.resolve_safe();
	m_gcrerr_cb.resolve_safe();

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
