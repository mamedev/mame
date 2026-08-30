// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1540/1541/1541C/1541-II Single Disk Drive emulation

**********************************************************************/

/*

    1540/1541/1541A/SX-64 Parts

    Location       Part Number    Description
                                  2016 2K x 8 bit Static RAM (short board)
    UA2-UB3                       2114 (4) 1K x 4 bit Static RAM (long board)
                   325572-01      64H105 40 pin Gate Array (short board)
                   325302-01      2364-130 ROM DOS 2.6 C000-DFFF
                   325303-01      2364-131 ROM DOS 2.6 (1540) E000-FFFF
                   901229-01      2364-173 ROM DOS 2.6 rev. 0 E000-FFFF
                   901229-03      2364-197 ROM DOS 2.6 rev. 1 E000-FFFF
                   901229-05      8K ROM DOS 2.6 rev. 2 E000-FFFF
                                  6502 CPU
                                  6522 (2) VIA
    drive                         Alps DFB111M25A
    drive                         Alps FDM2111-B2
    drive                         Newtronics D500

    1541B/1541C Parts

    Location       Part Number    Description
    UA3                           2016 2K x 8 bit Static RAM
    UC2                           6502 CPU
    UC1, UC3                      6522 (2) CIA
    UC4            251828-02      64H156 42 pin Gate Array
    UC5            251829-01      64H157 20 pin Gate Array
    UD1          * 251853-01      R/W Hybrid
    UD1          * 251853-02      R/W Hybrid
    UA2            251968-01      27128 EPROM DOS 2.6 rev. 3 C000-FFFF
    drive                         Newtronics D500
      * Not interchangeable.

    1541-II Parts

    Location       Part Number    Description
    U5                            2016-15 2K x 8 bit Static RAM
    U12                           SONY CX20185 R/W Amp.
    U3                            6502A CPU
    U6, U8                        6522 (2) CIA
    U10            251828-01      64H156 40 pin Gate Array
    U4             251968-03      16K ROM DOS 2.6 rev. 4 C000-FFFF
    drive                         Chinon FZ-501M REV A
    drive                         Digital System DS-50F
    drive                         Newtronics D500
    drive                         Safronic DS-50F

    ...

    PCB Assy # 1540008-01
    Schematic # 1540001
    Original "Long" Board
    Has 4 discreet 2114 RAMs
    ALPS Drive only

    PCB Assy # 1540048
    Schematic # 1540049
    Referred to as the CR board
    Changed to 2048 x 8 bit RAM pkg.
    A 40 pin Gate Array is used
    Alps Drive (-01)
    Newtronics Drive (-03)

    PCB Assy # 250442-01
    Schematic # 251748
    Termed the 1541 A
    Just one jumper change to accommodate both types of drive

    PCB Assy # 250446-01
    Schematic # 251748 (See Notes)
    Termed the 1541 A-2
    Just one jumper change to accommodate both types of drive

    ...

    VIC1541 1540001-01   Very early version, long board.
            1540001-03   As above, only the ROMs are different.
            1540008-01

    1541    1540048-01   Shorter board with a 40 pin gate array, Alps mech.
            1540048-03   As above, but Newtronics mech.
            1540049      Similar to above
            1540050      Similar to above, Alps mechanism.

    SX64    250410-01    Design most similar to 1540048-01, Alps mechanism.

    1541    251777       Function of bridge rects. reversed, Newtronics mech.
            251830       Same as above

    1541A   250442-01    Alps or Newtronics drive selected by a jumper.
    1541A2  250446-01    A 74LS123 replaces the 9602 at UD4.
    1541B   250448-01    Same as the 1541C, but in a case like the 1541.
    1541C   250448-01    Short board, new 40/42 pin gate array, 20 pin gate
                         array and a R/W hybrid chip replace many components.
                         Uses a Newtronics drive with optical trk 0 sensor.
    1541C   251854       As above, single DOS ROM IC, trk 0 sensor, 30 pin
                         IC for R/W ampl & stepper motor control (like 1541).

    1541-II              A complete redesign using the 40 pin gate array
                         from the 1451C and a Sony R/W hybrid, but not the
                         20 pin gate array, single DOS ROM IC.

    NOTE: These system boards are the 60 Hz versions.
          The -02 and -04 boards are probably the 50 Hz versions.

    The ROMs appear to be completely interchangeable. For instance, the
    first version of ROM for the 1541-II contained the same code as the
    last version of the 1541. I copy the last version of the 1541-II ROM
    into two 68764 EPROMs and use them in my original 1541 (long board).
    Not only do they work, but they work better than the originals.


    http://www.amiga-stuff.com/hardware/cbmboards.html

*/

#include "emu.h"
#include "c1541.h"

#include "formats/d64_dsk.h"
#include "formats/g64_dsk.h"
#include "formats/fs_cbmdos.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "ucd5"
#define M6522_0_TAG     "uab1"
#define M6522_1_TAG     "ucd4"
#define C64H156_TAG     "uc4"
#define C64H157_TAG     "uc5"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1540,                      c1540_device,                      "c1540",    "Commodore 1540 Disk Drive")
DEFINE_DEVICE_TYPE(C1541,                      c1541_device,                      "c1541",    "Commodore 1541 Disk Drive")
DEFINE_DEVICE_TYPE(C1541C,                     c1541c_device,                     "c1541c",   "Commodore 1541C Disk Drive")
DEFINE_DEVICE_TYPE(C1541II,                    c1541ii_device,                    "c1541ii",  "Commodore 1541-II Disk Drive")
DEFINE_DEVICE_TYPE(SX1541,                     sx1541_device,                     "sx1541",   "SX1541 Disk Drive")


//-------------------------------------------------
//  ROM( c1540 )
//-------------------------------------------------

ROM_START( c1540 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "325302-01.uab4", 0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11) )
	ROM_LOAD( "325303-01.uab5", 0x2000, 0x2000, CRC(10b39158) SHA1(56dfe79b26f50af4e83fd9604857756d196516b9) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1540_device::device_rom_region() const
{
	return ROM_NAME( c1540 );
}


//-------------------------------------------------
//  ROM( c1541 )
//-------------------------------------------------

ROM_START( c1541 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "325302-01.uab4", 0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11) )

	ROM_DEFAULT_BIOS("r6")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "901229-01.uab5", 0x2000, 0x2000, CRC(9a48d3f0) SHA1(7a1054c6156b51c25410caec0f609efb079d3a77), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "901229-02.uab5", 0x2000, 0x2000, CRC(b29bab75) SHA1(91321142e226168b1139c30c83896933f317d000), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r3", "Revision 3" )
	ROMX_LOAD( "901229-03.uab5", 0x2000, 0x2000, CRC(9126e74a) SHA1(03d17bd745066f1ead801c5183ac1d3af7809744), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "r4", "Revision 4" )
	ROMX_LOAD( "901229-04.uab5", 0x2000, 0x2000, NO_DUMP, ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "r5", "Revision 5" )
	ROMX_LOAD( "901229-05 ae.uab5", 0x2000, 0x2000, CRC(361c9f37) SHA1(f5d60777440829e46dc91285e662ba072acd2d8b), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "r6", "Revision 6" )
	ROMX_LOAD( "901229-06 aa.uab5", 0x2000, 0x2000, CRC(3a235039) SHA1(c7f94f4f51d6de4cdc21ecbb7e57bb209f0530c0), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1541.uab5", 0x2000, 0x2000, CRC(bc7e4aeb) SHA1(db6cfaa6d9b78d58746c811de29f3b1f44d99ddf), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "speeddos", "SpeedDOS-Plus+" )
	ROMX_LOAD( "speed-dosplus.uab5", 0x0000, 0x4000, CRC(f9db1eac) SHA1(95407e59a9c1d26a0e4bcf2c244cfe8942576e2c), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "rolo27", "Rolo DOS v2.7" )
	ROMX_LOAD( "rolo27.uab5", 0x2000, 0x2000, CRC(171c7962) SHA1(04c892c4b3d7c74750576521fa081f07d8ca8557), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS( 9, "digidos", "DigiDOS" )
	ROMX_LOAD( "digidos.uab5", 0x2000, 0x2000, CRC(b3f05ea3) SHA1(99d3d848344c68410b686cda812f3788b41fead3), ROM_BIOS(9) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541_device::device_rom_region() const
{
	return ROM_NAME( c1541 );
}


//-------------------------------------------------
//  ROM( c1541c )
//-------------------------------------------------

ROM_START( c1541c )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("r2")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "251968-01.ua2", 0x0000, 0x4000, CRC(1b3ca08d) SHA1(8e893932de8cce244117fcea4c46b7c39c6a7765), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r2", "Revision 2" )
	ROMX_LOAD( "251968-02.ua2", 0x0000, 0x4000, CRC(2d862d20) SHA1(38a7a489c7bbc8661cf63476bf1eb07b38b1c704), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541c_device::device_rom_region() const
{
	return ROM_NAME( c1541c );
}


//-------------------------------------------------
//  ROM( c1541ii )
//-------------------------------------------------

ROM_START( c1541ii )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "251968-03.u4", 0x0000, 0x4000, CRC(899fa3c5) SHA1(d3b78c3dbac55f5199f33f3fe0036439811f7fb3) )

	ROM_DEFAULT_BIOS("r1")
	ROM_SYSTEM_BIOS( 0, "r1", "Revision 1" )
	ROMX_LOAD( "355640-01.u4", 0x0000, 0x4000, CRC(57224cde) SHA1(ab16f56989b27d89babe5f89c5a8cb3da71a82f0), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1541-ii.u4", 0x0000, 0x4000, CRC(dd409902) SHA1(b1a5b826304d3df2a27d7163c6a81a532e040d32), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1541ii_device::device_rom_region() const
{
	return ROM_NAME( c1541ii );
}


//-------------------------------------------------
//  ROM( sx1541 )
//-------------------------------------------------

ROM_START( sx1541 )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "325302-01.uab4",    0x0000, 0x2000, CRC(29ae9752) SHA1(8e0547430135ba462525c224e76356bd3d430f11) )

	ROM_DEFAULT_BIOS("r5")
	ROM_SYSTEM_BIOS( 0, "r5", "Revision 5" )
	ROMX_LOAD( "901229-05 ae.uab5", 0x2000, 0x2000, CRC(361c9f37) SHA1(f5d60777440829e46dc91285e662ba072acd2d8b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos sx1541",   0x0000, 0x4000, CRC(783575f6) SHA1(36ccb9ff60328c4460b68522443ecdb7f002a234), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "flash", "1541 FLASH!" )
	ROMX_LOAD( "1541 flash.uab5",   0x2000, 0x2000, CRC(22f7757e) SHA1(86a1e43d3d22b35677064cca400a6bd06767a3dc), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *sx1541_device::device_rom_region() const
{
	return ROM_NAME( sx1541 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1541_mem )
//-------------------------------------------------

void c1541_device_base::c1541_mem(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x6000).ram();
	map(0x1800, 0x180f).mirror(0x63f0).m(m_via0, FUNC(via6522_device::map));
	map(0x1c00, 0x1c0f).mirror(0x63f0).m(m_via1, FUNC(via6522_device::map));
	map(0x8000, 0xbfff).mirror(0x4000).rom().region(M6502_TAG, 0);
}



uint8_t c1541_device_base::via0_pb_r()
{
	/*

	    bit     description

	    PB0     DATA IN
	    PB1
	    PB2     CLK IN
	    PB3
	    PB4
	    PB5     J1
	    PB6     J2
	    PB7     ATN IN

	*/

	u8 data;

	// data in
	data = !m_bus->data_r() && !m_ga->atn_r();

	// clock in
	data |= !m_bus->clk_r() << 2;

	// serial bus address
	data |= ((m_slot->get_address() - 8) & 0x03) << 5;

	// attention in
	data |= !m_bus->atn_r() << 7;

	return data;
}

TIMER_CALLBACK_MEMBER(c1541_device_base::iec_sync_tick)
{
	m_via0->write_ca1(!m_bus->atn_r());

	m_bus->clk_w(this, m_iec_clk);

	bool data = m_iec_data && !m_ga->atn_r();
	m_bus->data_w(this, data);
}

void c1541_device_base::via0_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0
	    PB1     DATA OUT
	    PB2
	    PB3     CLK OUT
	    PB4     ATNA
	    PB5
	    PB6
	    PB7

	*/

	m_iec_clk = !BIT(data, 3);
	m_iec_data = !BIT(data, 1);

	m_ga->atna_w(BIT(data, 4)); // triggers IEC sync
}

uint8_t c1541c_device::via0_pa_r()
{
	/*

	    bit     description

	    PA0     TR00 SENCE
	    PA1
	    PA2
	    PA3
	    PA4
	    PA5
	    PA6
	    PA7

	*/

	return !m_floppy->trk00_r();
}


void c1541_device_base::via1_pb_w(uint8_t data)
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

	if (machine().phase() != machine_phase::RUNNING)
		return;

	// spindle motor
	m_ga->mtr_w(BIT(data, 2));

	// stepper motor
	m_ga->stp_w(data & 0x03);

	// activity LED
	m_leds[LED_ACT] = BIT(data, 3);

	// density select
	m_ga->ds_w((data >> 5) & 0x03);
}


//-------------------------------------------------
//  C64H156_INTERFACE( ga_intf )
//-------------------------------------------------

void c1541_device_base::atn_w(int state)
{
	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void c1541_device_base::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D64_FORMAT);
	fr.add(FLOPPY_G64_FORMAT);
	fr.add(fs::CBMDOS);
}

void c1541_device_base::wpt_callback(floppy_image_device *floppy, int state)
{
	m_via1->write_pb4(!state);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1541_device_base::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, XTAL(16'000'000)/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &c1541_device_base::c1541_mem);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	MOS6522(config, m_via0, XTAL(16'000'000)/16);
	m_via0->readpb_handler().set(FUNC(c1541_device_base::via0_pb_r));
	m_via0->writepb_handler().set(FUNC(c1541_device_base::via0_pb_w));
	m_via0->irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	MOS6522(config, m_via1, XTAL(16'000'000)/16);
	m_via1->writepa_handler().set(C64H156_TAG, FUNC(c64h156_device::yb_w));
	m_via1->writepb_handler().set(FUNC(c1541_device_base::via1_pb_w));
	m_via1->ca2_handler().set(C64H156_TAG, FUNC(c64h156_device::soe_w));
	m_via1->cb2_handler().set(C64H156_TAG, FUNC(c64h156_device::oe_w));
	m_via1->irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));

	C64H156(config, m_ga, XTAL(16'000'000));
	m_ga->atn_callback().set(FUNC(c1541_device_base::atn_w));
	m_ga->sync_callback().set(m_via1, FUNC(via6522_device::write_pb7));
	m_ga->byte_callback().set_inputline(m_maincpu, M6502_SET_OVERFLOW).invert();
	m_ga->byte_callback().append(m_via1, FUNC(via6522_device::write_ca1));
	m_ga->yb_wr_cb().set(m_via1, FUNC(via6522_device::write_pa));

	floppy_connector &connector(FLOPPY_CONNECTOR(config, C64H156_TAG":0"));
	connector.option_add("525ssqd", ALPS_3255190X);
	connector.set_default_option("525ssqd");
	connector.set_fixed(true);
	connector.set_formats(c1541_device_base::floppy_formats);
	connector.enable_sound(true);
}


void c1541c_device::device_add_mconfig(machine_config &config)
{
	c1541_device_base::device_add_mconfig(config);
}


//-------------------------------------------------
//  INPUT_PORTS( c1541 )
//-------------------------------------------------

static INPUT_PORTS_START( c1541 )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1541_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1541 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1541_device_base - constructor
//-------------------------------------------------

c1541_device_base::c1541_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_floppy(*this, C64H156_TAG":0:525ssqd"),
	m_via0(*this, M6522_0_TAG),
	m_via1(*this, M6522_1_TAG),
	m_ga(*this, C64H156_TAG),
	m_address(*this, "ADDRESS"),
	m_leds(*this, "led%u", 0U)
{
}


//-------------------------------------------------
//  c1540_device - constructor
//-------------------------------------------------

c1540_device::c1540_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1540, tag, owner, clock) { }


//-------------------------------------------------
//  c1541_device - constructor
//-------------------------------------------------

c1541_device::c1541_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541, tag, owner, clock) { }


//-------------------------------------------------
//  c1541c_device - constructor
//-------------------------------------------------

c1541c_device::c1541c_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541C, tag, owner, clock) {  }


//-------------------------------------------------
//  c1541ii_device - constructor
//-------------------------------------------------

c1541ii_device::c1541ii_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, C1541II, tag, owner, clock) {  }


//-------------------------------------------------
//  sx1541_device - constructor
//-------------------------------------------------

sx1541_device::sx1541_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: c1541_device_base(mconfig, SX1541, tag, owner, clock) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1541_device_base::device_start()
{
	m_iec_sync_timer = timer_alloc(FUNC(c1541_device_base::iec_sync_tick), this);

	// install image callbacks
	m_ga->set_floppy(m_floppy);
	m_floppy->setup_wpt_cb(floppy_image_device::wpt_cb(&c1541_device_base::wpt_callback, this));

	// register for state saving
	save_item(NAME(m_iec_clk));
	save_item(NAME(m_iec_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c1541_device_base::device_reset()
{
	// initialize gate array
	m_ga->accl_w(0);
	m_ga->ted_w(1);
}


//-------------------------------------------------
//  iec_atn_w -
//-------------------------------------------------

void c1541_device_base::cbm_iec_atn(int state)
{
	m_ga->atni_w(!state); // triggers IEC sync
}


//-------------------------------------------------
//  iec_reset_w -
//-------------------------------------------------

void c1541_device_base::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}
