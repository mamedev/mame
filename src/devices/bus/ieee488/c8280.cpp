// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8280 Dual 8" Disk Drive emulation

**********************************************************************/

#include "emu.h"
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

DEFINE_DEVICE_TYPE(C8280, c8280_device, "c8280", "Commodore 8280")


//-------------------------------------------------
//  ROM( c8280 )
//-------------------------------------------------

ROM_START( c8280 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )

	ROM_REGION( 0x4000, M6502_DOS_TAG, 0 )
	ROMX_LOAD( "300542-001.10c", 0x0000, 0x2000, CRC(3c6eee1e) SHA1(0726f6ab4de4fc9c18707fe87780ffd9f5ed72ab), ROM_BIOS(0) )
	ROMX_LOAD( "300543-001.10d", 0x2000, 0x2000, CRC(f58e665e) SHA1(9e58b47c686c91efc6ef1a27f72dbb5e26c485ec), ROM_BIOS(0) )
	ROMX_LOAD( "300542-reva.10c", 0x0000, 0x2000, CRC(6f32ccfb) SHA1(6926c049f1635e6769ec69891de8c92941ff880e), ROM_BIOS(1) )
	ROMX_LOAD( "300543-reva.10d", 0x2000, 0x2000, CRC(1af93f2c) SHA1(ad197b1d5dfa273487b33f473403ebd20dd15b2b), ROM_BIOS(1) )

	ROM_REGION( 0x800, M6502_FDC_TAG, 0 )
	ROMX_LOAD( "300541-001.3c", 0x000, 0x800, BAD_DUMP CRC(cb07b2db) SHA1(a1f9c5a7bd3798f5a97dc0b465c3bf5e3513e148), ROM_BIOS(0) )
	ROMX_LOAD( "300541-revb.3c", 0x000, 0x800, CRC(403e632c) SHA1(a0994c80025240d2b49ffd209dbfe8a4de3975b0), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c8280_device::device_rom_region() const
{
	return ROM_NAME( c8280 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c8280_main_mem )
//-------------------------------------------------

void c8280_device::c8280_main_mem(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x100).m(M6532_0_TAG, FUNC(mos6532_new_device::ram_map));
	map(0x0080, 0x00ff).mirror(0x100).m(M6532_1_TAG, FUNC(mos6532_new_device::ram_map));
	map(0x0200, 0x021f).mirror(0xd60).m(M6532_0_TAG, FUNC(mos6532_new_device::io_map));
	map(0x0280, 0x029f).mirror(0xd60).m(M6532_1_TAG, FUNC(mos6532_new_device::io_map));
	map(0x1000, 0x13ff).mirror(0xc00).ram().share("share1");
	map(0x2000, 0x23ff).mirror(0xc00).ram().share("share2");
	map(0x3000, 0x33ff).mirror(0xc00).ram().share("share3");
	map(0x4000, 0x43ff).mirror(0xc00).ram().share("share4");
	map(0xc000, 0xffff).rom().region(M6502_DOS_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( c8280_fdc_mem )
//-------------------------------------------------

void c8280_device::c8280_fdc_mem(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x007f).mirror(0x300).ram();
	map(0x0080, 0x0083).mirror(0x37c).rw(WD1797_TAG, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0x0400, 0x07ff).ram().share("share1");
	map(0x0800, 0x0bff).ram().share("share2");
	map(0x0c00, 0x0fff).ram().share("share3");
	map(0x1000, 0x13ff).ram().share("share4");
	map(0x1400, 0x1400).mirror(0x3ff).rw(FUNC(c8280_device::fk5_r), FUNC(c8280_device::fk5_w));
	map(0x1800, 0x1fff).rom().region(M6502_FDC_TAG, 0);
}


//-------------------------------------------------
//  riot6532 0
//-------------------------------------------------

READ8_MEMBER( c8280_device::dio_r )
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

	return m_bus->read_dio();
}

WRITE8_MEMBER( c8280_device::dio_w )
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

READ8_MEMBER( c8280_device::riot1_pa_r )
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

	uint8_t data = 0;

	// end or identify in
	data |= m_bus->eoi_r() << 5;

	// data valid in
	data |= m_bus->dav_r() << 6;

	// attention
	data |= !m_bus->atn_r() << 7;

	return data;
}

WRITE8_MEMBER( c8280_device::riot1_pa_w )
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

READ8_MEMBER( c8280_device::riot1_pb_r )
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

	uint8_t data = 0;

	// device number selection
	data |= m_slot->get_address() - 8;

	// data accepted in
	data |= m_bus->ndac_r() << 6;

	// ready for data in
	data |= m_bus->nrfd_r() << 7;

	return data;
}

WRITE8_MEMBER( c8280_device::riot1_pb_w )
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
	m_leds[LED_ACT1] = BIT(data, 3);

	// activity led 0
	m_leds[LED_ACT0] = BIT(data, 4);

	// error led
	m_leds[LED_ERR] = BIT(data, 5);
}

static void c8280_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

FLOPPY_FORMATS_MEMBER( c8280_device::floppy_formats )
	FLOPPY_C8280_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c8280_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, XTAL(12'000'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &c8280_device::c8280_main_mem);

	MOS6532_NEW(config, m_riot0, XTAL(12'000'000)/8);
	m_riot0->pa_rd_callback().set(FUNC(c8280_device::dio_r));
	m_riot0->pb_wr_callback().set(FUNC(c8280_device::dio_w));

	MOS6532_NEW(config, m_riot1, XTAL(12'000'000)/8);
	m_riot1->pa_rd_callback().set(FUNC(c8280_device::riot1_pa_r));
	m_riot1->pa_wr_callback().set(FUNC(c8280_device::riot1_pa_w));
	m_riot1->pb_rd_callback().set(FUNC(c8280_device::riot1_pb_r));
	m_riot1->pb_wr_callback().set(FUNC(c8280_device::riot1_pb_w));
	m_riot1->irq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	M6502(config, m_fdccpu, XTAL(12'000'000)/8);
	m_fdccpu->set_addrmap(AS_PROGRAM, &c8280_device::c8280_fdc_mem);

	FD1797(config, m_fdc, XTAL(12'000'000)/6);
	m_fdc->intrq_wr_callback().set_inputline(m_fdccpu, M6502_IRQ_LINE);
	m_fdc->drq_wr_callback().set_inputline(m_fdccpu, M6502_SET_OVERFLOW);
	FLOPPY_CONNECTOR(config, m_floppy0, c8280_floppies, "8dsdd", c8280_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy1, c8280_floppies, "8dsdd", c8280_device::floppy_formats);
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

ioport_constructor c8280_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c8280 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void c8280_device::update_ieee_signals()
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
//  c8280_device - constructor
//-------------------------------------------------

c8280_device::c8280_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C8280, tag, owner, clock),
	device_ieee488_interface(mconfig, *this),
	m_maincpu(*this, M6502_DOS_TAG),
	m_fdccpu(*this, M6502_FDC_TAG),
	m_riot0(*this, M6532_0_TAG),
	m_riot1(*this, M6532_1_TAG),
	m_fdc(*this, WD1797_TAG),
	m_floppy0(*this, WD1797_TAG ":0"),
	m_floppy1(*this, WD1797_TAG ":1"),
	m_address(*this, "ADDRESS"),
	m_floppy(nullptr),
	m_leds(*this, "led%u", 0U),
	m_rfdo(1),
	m_daco(1),
	m_atna(1), m_ifc(0), m_fk5(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c8280_device::device_start()
{
	m_leds.resolve();

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

void c8280_device::device_reset()
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
	m_floppy = nullptr;
	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(0);
}


//-------------------------------------------------
//  ieee488_atn -
//-------------------------------------------------

void c8280_device::ieee488_atn(int state)
{
	update_ieee_signals();

	m_riot1->pa7_w(state);
}


//-------------------------------------------------
//  ieee488_ifc -
//-------------------------------------------------

void c8280_device::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}

READ8_MEMBER( c8280_device::fk5_r )
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

	uint8_t data = m_fk5;

	data |= (m_floppy ? m_floppy->dskchg_r() : 1) << 3;
	data |= (m_floppy ? m_floppy->twosid_r() : 1) << 4;

	return data;
}

WRITE8_MEMBER( c8280_device::fk5_w )
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
	m_floppy = nullptr;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy) m_floppy->mon_w(!BIT(data, 5));

	// density select
	m_fdc->dden_w(BIT(data, 2));
}
