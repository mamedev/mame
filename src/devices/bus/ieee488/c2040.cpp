// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 2040/3040/4040 Disk Drive emulation

**********************************************************************/

/*

    2040/3040 disk initialization
    -----------------------------
    You need to initialize each diskette before trying to access it
    or you will get a DISK ID MISMATCH error upon disk commands.
    On the 4040 this is done automatically by the DOS.

    open 15,8,15:print 15,"i":close 15

    List directory
    --------------
    directory / diR

    Format disk
    -----------
    header "label,id",d0,i01

    Load file
    ---------
    dload "name" / dL"name

    Save file
    ---------
    dsave "name" / dS"name

*/

/*

    TODO:

    - 2040/3040/4040 have a Shugart SA390 drive (FLOPPY_525_SSSD_35T)

    - 2040 DOS 1 FDC rom (jumps to 104d while getting block header)

        FE70: jsr  $104D
        104D: m6502_brk#$00

*/

#include "emu.h"
#include "c2040.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "un1"
#define M6532_0_TAG     "uc1"
#define M6532_1_TAG     "ue1"
#define M6504_TAG       "uh3"
#define M6522_TAG       "um3"
#define M6530_TAG       "uk3"
#define FDC_TAG         "fdc"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C2040, c2040_device, "c2040", "Commodore 2040")
DEFINE_DEVICE_TYPE(C3040, c3040_device, "c3040", "Commodore 3040")
DEFINE_DEVICE_TYPE(C4040, c4040_device, "c4040", "Commodore 4040")


//-------------------------------------------------
//  ROM( c2040 )
//-------------------------------------------------

ROM_START( c2040 ) // schematic 320806, DOS 1.0
	ROM_REGION( 0x3000, M6502_TAG, 0 )
	ROM_LOAD( "901468-xx.ul1", 0x1000, 0x1000, NO_DUMP )
	ROM_LOAD( "901468-xx.uh1", 0x2000, 0x1000, NO_DUMP )

	ROM_REGION( 0x400, M6504_TAG, 0 )
	ROM_LOAD( "901466-01.uk3", 0x000, 0x400, CRC(9d1e25ce) SHA1(d539858f839f96393f218307df7394362a84a26a) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6",    0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c2040_device::device_rom_region() const
{
	return ROM_NAME( c2040 );
}


//-------------------------------------------------
//  ROM( c3040 )
//-------------------------------------------------

ROM_START( c3040 ) // schematic 320806, DOS 1.2
	ROM_REGION( 0x3000, M6502_TAG, 0 )
	ROM_LOAD( "901468-06.ul1", 0x1000, 0x1000, CRC(25b5eed5) SHA1(4d9658f2e6ff3276e5c6e224611a66ce44b16fc7) )
	ROM_LOAD( "901468-07.uh1", 0x2000, 0x1000, CRC(9b09ae83) SHA1(6a51c7954938439ca8342fc295bda050c06e1791) )

	ROM_REGION( 0x400, M6504_TAG, 0 )
	ROM_LOAD( "901466-02.uk3", 0x000, 0x400, CRC(9d1e25ce) SHA1(d539858f839f96393f218307df7394362a84a26a) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6",    0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c3040_device::device_rom_region() const
{
	return ROM_NAME( c3040 );
}


//-------------------------------------------------
//  ROM( c4040 )
//-------------------------------------------------

ROM_START( c4040 ) // schematic ?
	ROM_REGION( 0x3000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("dos20r2")
	ROM_SYSTEM_BIOS( 0, "dos20r1", "DOS 2.0 Revision 1" )
	ROMX_LOAD( "901468-11.uj1", 0x0000, 0x1000, CRC(b7157458) SHA1(8415f3159dea73161e0cef7960afa6c76953b6f8), ROM_BIOS(0) )
	ROMX_LOAD( "901468-12.ul1", 0x1000, 0x1000, CRC(02c44ff9) SHA1(e8a94f239082d45f64f01b2d8e488d18fe659cbb), ROM_BIOS(0) )
	ROMX_LOAD( "901468-13.uh1", 0x2000, 0x1000, CRC(cbd785b3) SHA1(6ada7904ac9d13c3f1c0a8715f9c4be1aa6eb0bb), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "dos20r2", "DOS 2.0 Revision 2" )
	ROMX_LOAD( "901468-14.uj1", 0x0000, 0x1000, CRC(bc4d4872) SHA1(ffb992b82ec913ddff7be964d7527aca3e21580c), ROM_BIOS(1) )
	ROMX_LOAD( "901468-15.ul1", 0x1000, 0x1000, CRC(b6970533) SHA1(f702d6917fe8a798740ba4d467b500944ae7b70a), ROM_BIOS(1) )
	ROMX_LOAD( "901468-16.uh1", 0x2000, 0x1000, CRC(1f5eefb7) SHA1(04b918cf4adeee8015b43383d3cea7288a7d0aa8), ROM_BIOS(1) )

	ROM_REGION( 0x400, M6504_TAG, 0 )
	// RIOT DOS 2
	ROM_LOAD( "901466-04.uk3", 0x000, 0x400, CRC(0ab338dc) SHA1(6645fa40b81be1ff7d1384e9b52df06a26ab0bfb) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6",    0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c4040_device::device_rom_region() const
{
	return ROM_NAME( c4040 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c2040_main_mem )
//-------------------------------------------------

void c2040_device::c2040_main_mem(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x007f).mirror(0x0100).m(M6532_0_TAG, FUNC(mos6532_new_device::ram_map));
	map(0x0080, 0x00ff).mirror(0x0100).m(M6532_1_TAG, FUNC(mos6532_new_device::ram_map));
	map(0x0200, 0x021f).mirror(0x0d60).m(M6532_0_TAG, FUNC(mos6532_new_device::io_map));
	map(0x0280, 0x029f).mirror(0x0d60).m(M6532_1_TAG, FUNC(mos6532_new_device::io_map));
	map(0x1000, 0x13ff).mirror(0x0c00).ram().share("share1");
	map(0x2000, 0x23ff).mirror(0x0c00).ram().share("share2");
	map(0x3000, 0x33ff).mirror(0x0c00).ram().share("share3");
	map(0x4000, 0x43ff).mirror(0x0c00).ram().share("share4");
	map(0x5000, 0x7fff).rom().region(M6502_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( c2040_fdc_mem )
//-------------------------------------------------

void c2040_device::c2040_fdc_mem(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x003f).mirror(0x0300).m(M6530_TAG, FUNC(mos6530_new_device::ram_map));
	map(0x0040, 0x004f).mirror(0x0330).m(M6522_TAG, FUNC(via6522_device::map));
	map(0x0080, 0x008f).mirror(0x0330).m(M6530_TAG, FUNC(mos6530_new_device::io_map));
	map(0x0400, 0x07ff).ram().share("share1");
	map(0x0800, 0x0bff).ram().share("share2");
	map(0x0c00, 0x0fff).ram().share("share3");
	map(0x1000, 0x13ff).ram().share("share4");
	map(0x1c00, 0x1fff).rom().region(M6504_TAG, 0);
}


//-------------------------------------------------
//  riot6532 uc1
//-------------------------------------------------

READ8_MEMBER( c2040_device::dio_r )
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

WRITE8_MEMBER( c2040_device::dio_w )
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
//  riot6532 ue1
//-------------------------------------------------

READ8_MEMBER( c2040_device::riot1_pa_r )
{
	/*

	    bit     description

	    PA0     ATNA
	    PA1     DACO
	    PA2     RFDO
	    PA3     EOIO
	    PA4     DAVO
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

WRITE8_MEMBER( c2040_device::riot1_pa_w )
{
	/*

	    bit     description

	    PA0     ATNA
	    PA1     DACO
	    PA2     RFDO
	    PA3     EOIO
	    PA4     DAVO
	    PA5     EOII
	    PA6     DAVI
	    PA7     _ATN

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

READ8_MEMBER( c2040_device::riot1_pb_r )
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

WRITE8_MEMBER( c2040_device::riot1_pb_w )
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


WRITE8_MEMBER( c2040_device::via_pb_w )
{
	/*

	    bit     description

	    PB0     S1A
	    PB1     S1B
	    PB2     S0A
	    PB3     S0B
	    PB4     MTR1
	    PB5     MTR0
	    PB6
	    PB7

	*/

	// spindle motor 1
	m_fdc->mtr1_w(BIT(data, 4));

	// spindle motor 0
	m_fdc->mtr0_w(BIT(data, 5));

	// stepper motor 1
	m_fdc->stp1_w(data & 0x03);

	// stepper motor 0
	m_fdc->stp0_w((data >> 2) & 0x03);
}


//-------------------------------------------------
//  SLOT_INTERFACE( c2040_floppies )
//-------------------------------------------------

static void c2040_floppies(device_slot_interface &device)
{
	device.option_add("525ssqd", FLOPPY_525_SSQD);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c2040_device::floppy_formats )
	FLOPPY_C3040_FORMAT,
	FLOPPY_G64_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c3040_device::floppy_formats )
	FLOPPY_C3040_FORMAT,
	FLOPPY_G64_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c4040_device::floppy_formats )
	FLOPPY_C4040_FORMAT,
	FLOPPY_G64_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c2040_device::add_common_devices(machine_config &config)
{
	// DOS
	M6502(config, m_maincpu, XTAL(16'000'000)/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &c2040_device::c2040_main_mem);

	MOS6532_NEW(config, m_riot0, XTAL(16'000'000)/16);
	m_riot0->pa_rd_callback().set(FUNC(c2040_device::dio_r));
	m_riot0->pb_wr_callback().set(FUNC(c2040_device::dio_w));

	MOS6532_NEW(config, m_riot1, XTAL(16'000'000)/16);
	m_riot1->pa_rd_callback().set(FUNC(c2040_device::riot1_pa_r));
	m_riot1->pa_wr_callback().set(FUNC(c2040_device::riot1_pa_w));
	m_riot1->pb_rd_callback().set(FUNC(c2040_device::riot1_pb_r));
	m_riot1->pb_wr_callback().set(FUNC(c2040_device::riot1_pb_w));
	m_riot1->irq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// controller
	M6504(config, m_fdccpu, XTAL(16'000'000)/16);
	m_fdccpu->set_addrmap(AS_PROGRAM, &c2040_device::c2040_fdc_mem);

	VIA6522(config, m_via, XTAL(16'000'000)/16);
	m_via->readpa_handler().set(m_fdc, FUNC(c2040_fdc_device::read));
	m_via->writepb_handler().set(FUNC(c2040_device::via_pb_w));
	m_via->ca2_handler().set(m_fdc, FUNC(c2040_fdc_device::mode_sel_w));
	m_via->cb2_handler().set(m_fdc, FUNC(c2040_fdc_device::rw_sel_w));

	MOS6530_NEW(config, m_miot, XTAL(16'000'000)/16);
	m_miot->pa_wr_callback().set(m_fdc, FUNC(c2040_fdc_device::write));
	m_miot->pb_wr_callback<0>().set(m_fdc, FUNC(c2040_fdc_device::drv_sel_w));
	m_miot->pb_wr_callback<1>().set(m_fdc, FUNC(c2040_fdc_device::ds0_w));
	m_miot->pb_wr_callback<2>().set(m_fdc, FUNC(c2040_fdc_device::ds1_w));
	m_miot->pb_wr_callback<7>().set_inputline(m_fdccpu, M6502_IRQ_LINE);
	m_miot->pb_rd_callback<3>().set(m_fdc, FUNC(c2040_fdc_device::wps_r));

	C2040_FDC(config, m_fdc, XTAL(16'000'000));
	m_fdc->sync_wr_callback().set(m_miot, FUNC(mos6530_new_device::pb6_w));
	m_fdc->ready_wr_callback().set(m_via, FUNC(via6522_device::write_ca1));
	m_fdc->error_wr_callback().set(m_via, FUNC(via6522_device::write_cb1));
}

void c2040_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	FLOPPY_CONNECTOR(config, FDC_TAG":0", c2040_floppies, "525ssqd", c2040_device::floppy_formats, true);
	FLOPPY_CONNECTOR(config, FDC_TAG":1", c2040_floppies, "525ssqd", c2040_device::floppy_formats, true);
}

void c3040_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	FLOPPY_CONNECTOR(config, FDC_TAG":0", c2040_floppies, "525ssqd", c3040_device::floppy_formats, true);
	FLOPPY_CONNECTOR(config, FDC_TAG":1", c2040_floppies, "525ssqd", c3040_device::floppy_formats, true);
}

void c4040_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	FLOPPY_CONNECTOR(config, FDC_TAG":0", c2040_floppies, "525ssqd", c4040_device::floppy_formats, true);
	FLOPPY_CONNECTOR(config, FDC_TAG":1", c2040_floppies, "525ssqd", c4040_device::floppy_formats, true);
}


//-------------------------------------------------
//  INPUT_PORTS( c2040 )
//-------------------------------------------------

static INPUT_PORTS_START( c2040 )
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

ioport_constructor c2040_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c2040 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void c2040_device::update_ieee_signals()
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
//  c2040_device - constructor
//-------------------------------------------------

c2040_device::c2040_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_ieee488_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_fdccpu(*this, M6504_TAG),
	m_riot0(*this, M6532_0_TAG),
	m_riot1(*this, M6532_1_TAG),
	m_miot(*this, M6530_TAG),
	m_via(*this, M6522_TAG),
	m_floppy0(*this, FDC_TAG":0:525ssqd"),
	m_floppy1(*this, FDC_TAG":1:525ssqd"),
	m_fdc(*this, FDC_TAG),
	m_gcr(*this, "gcr"),
	m_address(*this, "ADDRESS"),
	m_leds(*this, "led%u", 0U),
	m_rfdo(1),
	m_daco(1),
	m_atna(1),
	m_ifc(0)
{
}

c2040_device::c2040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c2040_device(mconfig, C2040, tag, owner, clock)
{
}


//-------------------------------------------------
//  c3040_device - constructor
//-------------------------------------------------

c3040_device::c3040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c2040_device(mconfig, C3040, tag, owner, clock)
{
}


//-------------------------------------------------
//  c4040_device - constructor
//-------------------------------------------------

c4040_device::c4040_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c2040_device(mconfig, C4040, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c2040_device::device_start()
{
	m_leds.resolve();
	// install image callbacks
	m_fdc->set_floppy(m_floppy0, m_floppy1);

	// register for state saving
	save_item(NAME(m_rfdo));
	save_item(NAME(m_daco));
	save_item(NAME(m_atna));
	save_item(NAME(m_ifc));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c2040_device::device_reset()
{
	m_maincpu->reset();

	// toggle M6502 SO
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, ASSERT_LINE);
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, CLEAR_LINE);

	m_fdccpu->reset();

	m_riot0->reset();
	m_riot1->reset();
	m_miot->reset();
	m_via->reset();

	m_riot1->pa7_w(0);

	// turn off spindle motors
	m_fdc->mtr0_w(1);
	m_fdc->mtr1_w(1);
}


//-------------------------------------------------
//  ieee488_atn -
//-------------------------------------------------

void c2040_device::ieee488_atn(int state)
{
	update_ieee_signals();

	m_riot1->pa7_w(!state);
}


//-------------------------------------------------
//  ieee488_ifc -
//-------------------------------------------------

void c2040_device::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}
