// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8280 Dual 8" Disk Drive emulation

**********************************************************************/

#include "c8280.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_DOS_TAG   "5c"
#define M6502_FDC_TAG   "9e"
#define M6532_0_TAG     "9f"
#define M6532_1_TAG     "9g"
#define WD1797_TAG      "5e"


enum
{
	LED_POWER = 0,
	LED_ACT0,
	LED_ACT1,
	LED_ERR
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C8280 = &device_creator<c8280_t>;


//-------------------------------------------------
//  ROM( c8280 )
//-------------------------------------------------

ROM_START( c8280 )
	ROM_REGION( 0x4000, M6502_DOS_TAG, 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "300542-001.10c", 0x0000, 0x2000, CRC(3c6eee1e) SHA1(0726f6ab4de4fc9c18707fe87780ffd9f5ed72ab), ROM_BIOS(1) )
	ROMX_LOAD( "300543-001.10d", 0x2000, 0x2000, CRC(f58e665e) SHA1(9e58b47c686c91efc6ef1a27f72dbb5e26c485ec), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "300542-reva.10c", 0x0000, 0x2000, CRC(6f32ccfb) SHA1(6926c049f1635e6769ec69891de8c92941ff880e), ROM_BIOS(2) )
	ROMX_LOAD( "300543-reva.10d", 0x2000, 0x2000, CRC(1af93f2c) SHA1(ad197b1d5dfa273487b33f473403ebd20dd15b2b), ROM_BIOS(2) )

	ROM_REGION( 0x800, M6502_FDC_TAG, 0 )
	ROMX_LOAD( "300541-001.3c", 0x000, 0x800, BAD_DUMP CRC(cb07b2db) SHA1(a1f9c5a7bd3798f5a97dc0b465c3bf5e3513e148), ROM_BIOS(1) )
	ROMX_LOAD( "300541-revb.3c", 0x000, 0x800, CRC(403e632c) SHA1(a0994c80025240d2b49ffd209dbfe8a4de3975b0), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c8280_t::device_rom_region() const
{
	return ROM_NAME( c8280 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c8280_main_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c8280_main_mem, AS_PROGRAM, 8, c8280_t )
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x100) AM_DEVICE(M6532_0_TAG, mos6532_t, ram_map)
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x100) AM_DEVICE(M6532_1_TAG, mos6532_t, ram_map)
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0xd60) AM_DEVICE(M6532_0_TAG, mos6532_t, io_map)
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0xd60) AM_DEVICE(M6532_1_TAG, mos6532_t, io_map)
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0xc00) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0xc00) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0xc00) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0xc00) AM_RAM AM_SHARE("share4")
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(M6502_DOS_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( c8280_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c8280_fdc_mem, AS_PROGRAM, 8, c8280_t )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x300) AM_RAM
	AM_RANGE(0x0080, 0x0083) AM_MIRROR(0x37c) AM_DEVREADWRITE(WD1797_TAG, fd1797_t, read, write)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1400, 0x1400) AM_MIRROR(0x3ff) AM_READWRITE(fk5_r, fk5_w)
	AM_RANGE(0x1800, 0x1fff) AM_ROM AM_REGION(M6502_FDC_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  riot6532 0
//-------------------------------------------------

READ8_MEMBER( c8280_t::dio_r )
{
	/*

	    bit     description

	    PA0     DI0
	    PA1     DI1
	    PA2     DI2
	    PA3     DI3
	    PA4     DI4
	    PA5     DI5
	    PA6     DI6
	    PA7     DI7

	*/

	return m_bus->dio_r();
}

WRITE8_MEMBER( c8280_t::dio_w )
{
	/*

	    bit     description

	    PB0     DO0
	    PB1     DO1
	    PB2     DO2
	    PB3     DO3
	    PB4     DO4
	    PB5     DO5
	    PB6     DO6
	    PB7     DO7

	*/

	m_bus->dio_w(this, data);
}


//-------------------------------------------------
//  riot6532 1
//-------------------------------------------------

READ8_MEMBER( c8280_t::riot1_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2
	    PA3
	    PA4
	    PA5     EOII
	    PA6     DAVI
	    PA7     _ATN

	*/

	UINT8 data = 0;

	// end or identify in
	data |= m_bus->eoi_r() << 5;

	// data valid in
	data |= m_bus->dav_r() << 6;

	// attention
	data |= !m_bus->atn_r() << 7;

	return data;
}

WRITE8_MEMBER( c8280_t::riot1_pa_w )
{
	/*

	    bit     description

	    PA0     ATNA
	    PA1     DACO
	    PA2     RFDO
	    PA3     EOIO
	    PA4     DAVO
	    PA5
	    PA6
	    PA7

	*/

	// attention acknowledge
	m_atna = BIT(data, 0);

	// data accepted out
	m_daco = BIT(data, 1);

	// not ready for data out
	m_rfdo = BIT(data, 2);

	// end or identify out
	m_bus->eoi_w(this, BIT(data, 3));

	// data valid out
	m_bus->dav_w(this, BIT(data, 4));

	update_ieee_signals();
}

READ8_MEMBER( c8280_t::riot1_pb_r )
{
	/*

	    bit     description

	    PB0     DEVICE NUMBER SELECTION
	    PB1     DEVICE NUMBER SELECTION
	    PB2     DEVICE NUMBER SELECTION
	    PB3
	    PB4
	    PB5
	    PB6     DACI
	    PB7     RFDI

	*/

	UINT8 data = 0;

	// device number selection
	data |= m_slot->get_address() - 8;

	// data accepted in
	data |= m_bus->ndac_r() << 6;

	// ready for data in
	data |= m_bus->nrfd_r() << 7;

	return data;
}

WRITE8_MEMBER( c8280_t::riot1_pb_w )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3     ACT LED 1
	    PB4     ACT LED 0
	    PB5     ERR LED
	    PB6
	    PB7

	*/

	// activity led 1
	output_set_led_value(LED_ACT1, BIT(data, 3));

	// activity led 0
	output_set_led_value(LED_ACT0, BIT(data, 4));

	// error led
	output_set_led_value(LED_ERR, BIT(data, 5));
}

static SLOT_INTERFACE_START( c8280_floppies )
	SLOT_INTERFACE( "8dsdd", FLOPPY_8_DSDD )
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( c8280_t::floppy_formats )
	FLOPPY_C8280_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c8280 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c8280 )
	MCFG_CPU_ADD(M6502_DOS_TAG, M6502, XTAL_12MHz/8)
	MCFG_CPU_PROGRAM_MAP(c8280_main_mem)

	MCFG_DEVICE_ADD(M6532_0_TAG, MOS6532n, XTAL_12MHz/8)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8280_t, dio_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8280_t, dio_w))

	MCFG_DEVICE_ADD(M6532_1_TAG, MOS6532n, XTAL_12MHz/8)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8280_t, riot1_pa_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(c8280_t, riot1_pa_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(c8280_t, riot1_pb_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8280_t, riot1_pb_w))
	MCFG_MOS6530n_IRQ_CB(INPUTLINE(M6502_DOS_TAG, INPUT_LINE_IRQ0))

	MCFG_CPU_ADD(M6502_FDC_TAG, M6502, XTAL_12MHz/8)
	MCFG_CPU_PROGRAM_MAP(c8280_fdc_mem)

	MCFG_FD1797_ADD(WD1797_TAG, XTAL_12MHz/6)
	MCFG_WD_FDC_INTRQ_CALLBACK(INPUTLINE(M6502_FDC_TAG, M6502_IRQ_LINE))
	MCFG_WD_FDC_DRQ_CALLBACK(INPUTLINE(M6502_FDC_TAG, M6502_SET_OVERFLOW))
	MCFG_FLOPPY_DRIVE_ADD(WD1797_TAG ":0", c8280_floppies, "8dsdd", c8280_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD1797_TAG ":1", c8280_floppies, "8dsdd", c8280_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c8280_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c8280 );
}


//-------------------------------------------------
//  INPUT_PORTS( c8280 )
//-------------------------------------------------

static INPUT_PORTS_START( c8280 )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x07, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x05, "13" )
	PORT_DIPSETTING(    0x06, "14" )
	PORT_DIPSETTING(    0x07, "15" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c8280_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( c8280 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void c8280_t::update_ieee_signals()
{
	int atn = m_bus->atn_r();
	int nrfd = !(!(!(atn && m_atna) && m_rfdo) || !(atn || m_atna));
	int ndac = !(m_daco || !(atn || m_atna));

	m_bus->nrfd_w(this, nrfd);
	m_bus->ndac_w(this, ndac);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c8280_t - constructor
//-------------------------------------------------

c8280_t::c8280_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C8280, "C8280", tag, owner, clock, "c8280", __FILE__),
	device_ieee488_interface(mconfig, *this),
	m_maincpu(*this, M6502_DOS_TAG),
	m_fdccpu(*this, M6502_FDC_TAG),
	m_riot0(*this, M6532_0_TAG),
	m_riot1(*this, M6532_1_TAG),
	m_fdc(*this, WD1797_TAG),
	m_floppy0(*this, WD1797_TAG ":0"),
	m_floppy1(*this, WD1797_TAG ":1"),
	m_address(*this, "ADDRESS"),
	m_rfdo(1),
	m_daco(1),
	m_atna(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c8280_t::device_start()
{
	// state saving
	save_item(NAME(m_rfdo));
	save_item(NAME(m_daco));
	save_item(NAME(m_atna));
	save_item(NAME(m_ifc));
	save_item(NAME(m_fk5));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c8280_t::device_reset()
{
	m_maincpu->reset();

	// toggle M6502 SO
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, ASSERT_LINE);
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, CLEAR_LINE);

	m_fdccpu->reset();

	m_riot0->reset();
	m_riot1->reset();
	m_fdc->reset();

	m_riot1->pa7_w(1);

	m_fk5 = 0;
	m_floppy = NULL;
	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(0);
}


//-------------------------------------------------
//  ieee488_atn -
//-------------------------------------------------

void c8280_t::ieee488_atn(int state)
{
	update_ieee_signals();

	m_riot1->pa7_w(state);
}


//-------------------------------------------------
//  ieee488_ifc -
//-------------------------------------------------

void c8280_t::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}

READ8_MEMBER( c8280_t::fk5_r )
{
	/*

	    bit     description

	    0
	    1
	    2
	    3       DCHG
	    4       TSID
	    5
	    6       0
	    7       0

	*/

	UINT8 data = m_fk5;

	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 3;
	data |= (m_floppy ? m_floppy->twosid_r() : 1) << 4;

	return data;
}

WRITE8_MEMBER( c8280_t::fk5_w )
{
	/*

	    bit     description

	    0       DS1
	    1       DS2
	    2       _DDEN
	    3
	    4
	    5       MOTOR ENABLE
	    6
	    7

	*/

	m_fk5 = data & 0x27;

	// drive select
	m_floppy = NULL;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy) m_floppy->mon_w(!BIT(data, 5));

	// density select
	m_fdc->dden_w(BIT(data, 2));
}
