// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 8050/8250/SFD-1001 Disk Drive emulation

**********************************************************************/

/*

    TODO:

    - Micropolis 8x50 stepper motor is same as 4040, except it takes 4 pulses to step a track instead of 1

    - BASIC program to set 8250/SFD-1001 to 8050 mode:

        10 OPEN 15,8,15
        20 PRINT#15,"M-W"CHR$(172)CHR$(16)CHR$(1)CHR$(1)
        30 PRINT#15,"M-W"CHR$(195)CHR$(16)CHR$(1)CHR$(0)
        40 PRINT#15,"U9"
        50 CLOSE 15

*/

#include "emu.h"
#include "c8050.h"

#include "formats/d80_dsk.h"
#include "formats/d82_dsk.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "un1"
#define M6532_0_TAG     "uc1"
#define M6532_1_TAG     "ue1"
#define M6504_TAG       "uh3"
#define M6530_TAG       "uk3"
#define FDC_TAG         "fdc"


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

DEFINE_DEVICE_TYPE(C8050,   c8050_device,   "c8050",   "Commodore 8050")
DEFINE_DEVICE_TYPE(C8250,   c8250_device,   "c8250",   "Commodore 8250")
DEFINE_DEVICE_TYPE(C8250LP, c8250lp_device, "c8250lp", "Commodore 8250LP")
DEFINE_DEVICE_TYPE(SFD1001, sfd1001_device, "sfd1001", "Commodore SFD-1001")


//-------------------------------------------------
//  ROM( c8050 )
//-------------------------------------------------

/*

    DOS/CONTROLLER ROMS FOR DIGITAL PCB #8050002

    DESCRIPTION     PCB PART NO.        UL1         UH1         UK3

    2.5 Micropolis  8050002-01      901482-07   901482-06   901483-03
    2.5 Tandon      8050002-02      901482-07   901482-06   901483-04
    2.7 Tandon      8050002-03      901887-01   901888-01   901884-01
    2.7 Micropolis  8050002-04      901887-01   901888-01   901885-04
    2.7 MPI 8050    8050002-05      901887-01   901888-01   901869-01
    2.7 MPI 8250    8050002-06      901887-01   901888-01   901869-01

*/

ROM_START( c8050 ) // schematic 8050001
	ROM_DEFAULT_BIOS("dos27mpi")
	ROM_SYSTEM_BIOS( 0, "dos25m", "DOS 2.5 (Micropolis)" )
	ROM_SYSTEM_BIOS( 1, "dos25r2m", "DOS 2.5 rev 2 (Micropolis)" )
	ROM_SYSTEM_BIOS( 2, "dos25r3m", "DOS 2.5 rev 3 (Micropolis)" )
	ROM_SYSTEM_BIOS( 3, "dos25t", "DOS 2.5 rev 3 (Tandon)" )
	ROM_SYSTEM_BIOS( 4, "dos27t", "DOS 2.7 (Tandon)" )
	ROM_SYSTEM_BIOS( 5, "dos27mo", "DOS 2.7 (Micropolis, older)" )
	ROM_SYSTEM_BIOS( 6, "dos27m", "DOS 2.7 (Micropolis)" )
	ROM_SYSTEM_BIOS( 7, "dos27mpi", "DOS 2.7 (MPI)" )

	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROMX_LOAD( "901482-01.ul1", 0x0000, 0x2000, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "901482-02.uh1", 0x2000, 0x2000, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "901482-03.ul1", 0x0000, 0x2000, CRC(09a609b9) SHA1(166d8bfaaa9c4767f9b17ad63fc7ae77c199a64e), ROM_BIOS(1) )
	ROMX_LOAD( "901482-04.uh1", 0x2000, 0x2000, CRC(1bcf9df9) SHA1(217f4a8b348658bb365f4a1de21ecbaa6402b1c0), ROM_BIOS(1) )
	ROMX_LOAD( "901482-06.ul1", 0x0000, 0x2000, CRC(3cbd2756) SHA1(7f5fbed0cddb95138dd99b8fe84fddab900e3650), ROM_BIOS(2) )
	ROMX_LOAD( "901482-07.uh1", 0x2000, 0x2000, CRC(c7532d90) SHA1(0b6d1e55afea612516df5f07f4a6dccd3bd73963), ROM_BIOS(2) )
	ROMX_LOAD( "901482-06.ul1", 0x0000, 0x2000, CRC(3cbd2756) SHA1(7f5fbed0cddb95138dd99b8fe84fddab900e3650), ROM_BIOS(3) )
	ROMX_LOAD( "901482-07.uh1", 0x2000, 0x2000, CRC(c7532d90) SHA1(0b6d1e55afea612516df5f07f4a6dccd3bd73963), ROM_BIOS(3) )
	ROMX_LOAD( "901887-01.ul1", 0x0000, 0x2000, CRC(0073b8b2) SHA1(b10603195f240118fe5fb6c6dfe5c5097463d890), ROM_BIOS(4) )
	ROMX_LOAD( "901888-01.uh1", 0x2000, 0x2000, CRC(de9b6132) SHA1(2e6c2d7ca934e5c550ad14bd5e9e7749686b7af4), ROM_BIOS(4) )
	ROMX_LOAD( "901887-01.ul1", 0x0000, 0x2000, CRC(0073b8b2) SHA1(b10603195f240118fe5fb6c6dfe5c5097463d890), ROM_BIOS(5) )
	ROMX_LOAD( "901888-01.uh1", 0x2000, 0x2000, CRC(de9b6132) SHA1(2e6c2d7ca934e5c550ad14bd5e9e7749686b7af4), ROM_BIOS(5) )
	ROMX_LOAD( "901887-01.ul1", 0x0000, 0x2000, CRC(0073b8b2) SHA1(b10603195f240118fe5fb6c6dfe5c5097463d890), ROM_BIOS(6) )
	ROMX_LOAD( "901888-01.uh1", 0x2000, 0x2000, CRC(de9b6132) SHA1(2e6c2d7ca934e5c550ad14bd5e9e7749686b7af4), ROM_BIOS(6) )
	ROMX_LOAD( "901887-01.ul1", 0x0000, 0x2000, CRC(0073b8b2) SHA1(b10603195f240118fe5fb6c6dfe5c5097463d890), ROM_BIOS(7) )
	ROMX_LOAD( "901888-01.uh1", 0x2000, 0x2000, CRC(de9b6132) SHA1(2e6c2d7ca934e5c550ad14bd5e9e7749686b7af4), ROM_BIOS(7) )

	ROM_REGION( 0x400, M6530_TAG, 0 )
	ROMX_LOAD( "901483-02.uk3", 0x000, 0x400, CRC(d7277f95) SHA1(7607f9357f3a08f2a9f20931058d60d9e3c17d39), ROM_BIOS(0) ) // 6530-036 DOS 2.5
	ROMX_LOAD( "901483-02.uk3", 0x000, 0x400, CRC(d7277f95) SHA1(7607f9357f3a08f2a9f20931058d60d9e3c17d39), ROM_BIOS(1) ) // 6530-036 DOS 2.5
	ROMX_LOAD( "901483-03.uk3", 0x000, 0x400, CRC(9e83fa70) SHA1(e367ea8a5ddbd47f13570088427293138a10784b), ROM_BIOS(2) ) // 6530-038 DOS 2.5 Micropolis
	ROMX_LOAD( "901483-04.uk3", 0x000, 0x400, CRC(ae1c7866) SHA1(13bdf0bb387159167534c07a4554964734373f11), ROM_BIOS(3) ) // 6530-039 DOS 2.5 Tandon
	ROMX_LOAD( "901884-01.uk3", 0x000, 0x400, CRC(9e9a9f90) SHA1(39498d7369a31ea7527b5044071acf35a84ea2ac), ROM_BIOS(4) ) // 6530-40 DOS 2.7 Tandon
	ROMX_LOAD( "901885-01.uk3", 0x000, 0x400, NO_DUMP, ROM_BIOS(5) ) // 6530-044 DOS 2.7 Micropolis
	ROMX_LOAD( "901885-04.uk3", 0x000, 0x400, CRC(bab998c9) SHA1(0dc9a3b60f1b866c63eebd882403532fc59fe57f), ROM_BIOS(6) ) // 6530-47 DOS 2.7 Micropolis
	ROMX_LOAD( "901869-01.uk3", 0x000, 0x400, CRC(2915327a) SHA1(3a9a80f72ce76e5f5c72513f8ef7553212912ae3), ROM_BIOS(7) ) // 6530-48 DOS 2.7 MPI
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c8050_device::device_rom_region() const
{
	return ROM_NAME( c8050 );
}


//-------------------------------------------------
//  ROM( c8250lp )
//-------------------------------------------------

ROM_START( c8250lp )
	ROM_DEFAULT_BIOS("dos27b")
	ROM_SYSTEM_BIOS( 0, "dos27", "DOS 2.7" )
	ROM_SYSTEM_BIOS( 1, "dos27b", "DOS 2.7B" )
	ROM_SYSTEM_BIOS( 2, "speeddos", "SpeedDOS" )

	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROMX_LOAD( "251165-01.ua11",  0x0000, 0x2000, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "251166-01.ua13",  0x2000, 0x2000, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "dos-2.7b.bin", 0x0000, 0x4000, CRC(96e3b209) SHA1(9849300be9f2e0143c2ed2564d26a4ba3b27526c), ROM_BIOS(1) ) // CBM DOS 2.7B from the 8250LP inside 8296D
	ROMX_LOAD( "speeddos-c000.ua11", 0x0000, 0x2000, CRC(46cc260f) SHA1(e9838635d6868e35ec9c161b6e5c1ad92a4a241a), ROM_BIOS(2) )
	ROMX_LOAD( "speeddos-e000.ua13", 0x2000, 0x2000, CRC(88cfd505) SHA1(0fb570b180504cd1fcb7d203d8d37ea3d7e72ab4), ROM_BIOS(2) )

	ROM_REGION( 0x800, M6504_TAG, 0 )
	ROMX_LOAD( "251256-02", 0x000, 0x400, NO_DUMP, ROM_BIOS(0) ) // 6530-050
	ROMX_LOAD( "251474-01b", 0x000, 0x400, CRC(9e9a9f90) SHA1(39498d7369a31ea7527b5044071acf35a84ea2ac), ROM_BIOS(0) ) // Matsushita
	ROMX_LOAD( "fdc-2.7b.bin", 0x000, 0x800, CRC(13a24482) SHA1(1cfa52d2ed245a95e6369b46a36c6c7aa3929931), ROM_BIOS(1) ) // CBM DOS 2.7B FDC ROM from the 8250LP inside 8296D
	ROMX_LOAD( "speeddos-fdc-f800.bin", 0x000, 0x800, CRC(253e760f) SHA1(3f7892a9bab84b633f45686bbbbe66bc2948c8e5), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c8250lp_device::device_rom_region() const
{
	return ROM_NAME( c8250lp );
}


//-------------------------------------------------
//  ROM( sfd1001 )
//-------------------------------------------------

ROM_START( sfd1001 ) // schematic 251406
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "901887-01.1j", 0x0000, 0x2000, CRC(0073b8b2) SHA1(b10603195f240118fe5fb6c6dfe5c5097463d890) )
	ROM_LOAD( "901888-01.3j", 0x2000, 0x2000, CRC(de9b6132) SHA1(2e6c2d7ca934e5c550ad14bd5e9e7749686b7af4) )

	ROM_REGION( 0x400, M6530_TAG, 0 )
	ROM_LOAD( "901885-04.u1", 0x000, 0x400, CRC(bab998c9) SHA1(0dc9a3b60f1b866c63eebd882403532fc59fe57f) )

	ROM_REGION( 0x800, M6504_TAG, 0 )
	ROM_LOAD( "251257-02a.u2", 0x000, 0x800, CRC(b51150de) SHA1(3b954eb34f7ea088eed1d33ebc6d6e83a3e9be15) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *sfd1001_device::device_rom_region() const
{
	return ROM_NAME( sfd1001 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c8050_main_mem )
//-------------------------------------------------

void c8050_device::c8050_main_mem(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x0100).m(M6532_0_TAG, FUNC(mos6532_device::ram_map));
	map(0x0080, 0x00ff).mirror(0x0100).m(M6532_1_TAG, FUNC(mos6532_device::ram_map));
	map(0x0200, 0x021f).mirror(0x0d60).m(M6532_0_TAG, FUNC(mos6532_device::io_map));
	map(0x0280, 0x029f).mirror(0x0d60).m(M6532_1_TAG, FUNC(mos6532_device::io_map));
	map(0x1000, 0x13ff).mirror(0x0c00).ram().share("share1");
	map(0x2000, 0x23ff).mirror(0x0c00).ram().share("share2");
	map(0x3000, 0x33ff).mirror(0x0c00).ram().share("share3");
	map(0x4000, 0x43ff).mirror(0x0c00).ram().share("share4");
	map(0xc000, 0xffff).rom().region(M6502_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( c8050_fdc_mem )
//-------------------------------------------------

void c8050_device::c8050_fdc_mem(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x003f).mirror(0x0300).m(m_miot, FUNC(mos6530_device::ram_map));
	map(0x0040, 0x004f).mirror(0x0330).m(m_via, FUNC(via6522_device::map));
	map(0x0080, 0x008f).mirror(0x0330).m(m_miot, FUNC(mos6530_device::io_map));
	map(0x0400, 0x07ff).ram().share("share1");
	map(0x0800, 0x0bff).ram().share("share2");
	map(0x0c00, 0x0fff).ram().share("share3");
	map(0x1000, 0x13ff).ram().share("share4");
	map(0x1c00, 0x1fff).m(m_miot, FUNC(mos6530_device::rom_map));
}


//-------------------------------------------------
//  ADDRESS_MAP( c8250lp_fdc_mem )
//-------------------------------------------------

void c8050_device::c8250lp_fdc_mem(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x003f).mirror(0x0300).m(m_miot, FUNC(mos6530_device::ram_map));
	map(0x0040, 0x004f).mirror(0x0330).m(m_via, FUNC(via6522_device::map));
	map(0x0080, 0x008f).mirror(0x0330).m(m_miot, FUNC(mos6530_device::io_map));
	map(0x0400, 0x07ff).ram().share("share1");
	map(0x0800, 0x0bff).ram().share("share2");
	map(0x0c00, 0x0fff).ram().share("share3");
	map(0x1000, 0x13ff).ram().share("share4");
	map(0x1800, 0x1fff).rom().region(M6504_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( sfd1001_fdc_mem )
//-------------------------------------------------

void c8050_device::sfd1001_fdc_mem(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x003f).mirror(0x0300).m(m_miot, FUNC(mos6530_device::ram_map));
	map(0x0040, 0x004f).mirror(0x0330).m(m_via, FUNC(via6522_device::map));
	map(0x0080, 0x008f).mirror(0x0330).m(m_miot, FUNC(mos6530_device::io_map));
	map(0x0400, 0x07ff).ram().share("share1");
	map(0x0800, 0x0bff).ram().share("share2");
	map(0x0c00, 0x0fff).ram().share("share3");
	map(0x1000, 0x13ff).ram().share("share4");
	map(0x1800, 0x1fff).rom().region(M6504_TAG, 0);
}

//-------------------------------------------------
//  riot6532 ue1
//-------------------------------------------------

uint8_t c8050_device::riot1_pa_r()
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

void c8050_device::riot1_pa_w(uint8_t data)
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

uint8_t c8050_device::riot1_pb_r()
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

void c8050_device::riot1_pb_w(uint8_t data)
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

void c8050_device::via_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     S1A
	    PB1     S1B
	    PB2     S0A
	    PB3     S0B
	    PB4     MTR1
	    PB5     MTR0
	    PB6     PULL SYNC
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

	// PLL sync
	m_fdc->pull_sync_w(!BIT(data, 6));
}


//-------------------------------------------------
//  SLOT_INTERFACE( c8050_floppies )
//-------------------------------------------------

static void c8050_floppies(device_slot_interface &device)
{
	device.option_add("525ssqd", FLOPPY_525_SSQD);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void c8050_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D80_FORMAT);
}


//-------------------------------------------------
//  SLOT_INTERFACE( c8250_floppies )
//-------------------------------------------------

static void c8250_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}


//-------------------------------------------------
//  SLOT_INTERFACE( sfd1001_floppies )
//-------------------------------------------------

static void sfd1001_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD); // Matsushita JU-570 / JU-570-2
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void c8250_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D80_FORMAT);
	fr.add(FLOPPY_D82_FORMAT);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void c8250lp_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D80_FORMAT);
	fr.add(FLOPPY_D82_FORMAT);
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void sfd1001_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D80_FORMAT);
	fr.add(FLOPPY_D82_FORMAT);
}


void c8050_device::add_common_devices(machine_config &config)
{
	// DOS
	M6502(config, m_maincpu, XTAL(12'000'000)/12);
	m_maincpu->set_addrmap(AS_PROGRAM, &c8050_device::c8050_main_mem);

	MOS6532(config, m_riot0, XTAL(12'000'000)/12);
	m_riot0->pa_rd_callback().set(FUNC(c8050_device::dio_r));
	m_riot0->pb_wr_callback().set(FUNC(c8050_device::dio_w));

	MOS6532(config, m_riot1, XTAL(12'000'000)/12);
	m_riot1->pa_rd_callback().set(FUNC(c8050_device::riot1_pa_r));
	m_riot1->pa_wr_callback().set(FUNC(c8050_device::riot1_pa_w));
	m_riot1->pb_rd_callback().set(FUNC(c8050_device::riot1_pb_r));
	m_riot1->pb_wr_callback().set(FUNC(c8050_device::riot1_pb_w));
	m_riot1->irq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// controller
	M6504(config, m_fdccpu, XTAL(12'000'000)/12);

	MOS6522(config, m_via, XTAL(12'000'000)/12);
	m_via->readpa_handler().set(m_fdc, FUNC(c8050_fdc_device::read));
	m_via->writepb_handler().set(FUNC(c8050_device::via_pb_w));
	m_via->ca2_handler().set(m_fdc, FUNC(c8050_fdc_device::mode_sel_w));
	m_via->cb2_handler().set(m_fdc, FUNC(c8050_fdc_device::rw_sel_w));

	MOS6530(config, m_miot, XTAL(12'000'000)/12);
	m_miot->pa_wr_callback().set(m_fdc, FUNC(c8050_fdc_device::write));
	m_miot->pb_wr_callback<1>().set(m_fdc, FUNC(c8050_fdc_device::ds0_w));
	m_miot->pb_wr_callback<2>().set(m_fdc, FUNC(c8050_fdc_device::ds1_w));
	m_miot->pb_rd_callback<3>().set(m_fdc, FUNC(c8050_fdc_device::wps_r));
	m_miot->pb_rd_callback<6>().set_constant(1); // SINGLE SIDED
	m_miot->irq_wr_callback().set_inputline(m_fdccpu, M6502_IRQ_LINE);

	C8050_FDC(config, m_fdc, XTAL(12'000'000)/2);
	m_fdc->sync_wr_callback().set(m_via, FUNC(via6522_device::write_pb7));
	m_fdc->ready_wr_callback().set(m_via, FUNC(via6522_device::write_ca1));
	m_fdc->brdy_wr_callback().set_inputline(m_fdccpu, M6502_SET_OVERFLOW).invert();
	m_fdc->error_wr_callback().set(m_via, FUNC(via6522_device::write_cb1));
}

void c8050_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	m_fdccpu->set_addrmap(AS_PROGRAM, &c8050_device::c8050_fdc_mem);

	m_miot->pb_wr_callback<0>().set(m_fdc, FUNC(c8050_fdc_device::drv_sel_w));
	m_miot->pb_rd_callback<6>().set_constant(1); // SINGLE SIDED

	FLOPPY_CONNECTOR(config, FDC_TAG ":0", c8050_floppies, "525ssqd", c8050_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, FDC_TAG ":1", c8050_floppies, "525ssqd", c8050_device::floppy_formats).enable_sound(true);
}

void c8250_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	m_fdccpu->set_addrmap(AS_PROGRAM, &c8250_device::c8050_fdc_mem);

	m_miot->pb_wr_callback<0>().set(m_fdc, FUNC(c8050_fdc_device::drv_sel_w));
	m_miot->pb_wr_callback<4>().set(m_fdc, FUNC(c8050_fdc_device::odd_hd_w));
	m_miot->pb_rd_callback<6>().set_constant(0); // DOUBLE SIDED

	FLOPPY_CONNECTOR(config, FDC_TAG ":0", c8250_floppies, "525qd", c8250_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, FDC_TAG ":1", c8250_floppies, "525qd", c8250_device::floppy_formats).enable_sound(true);
}

void c8250lp_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	m_fdccpu->set_addrmap(AS_PROGRAM, &c8250lp_device::c8250lp_fdc_mem);

	m_miot->pb_wr_callback<0>().set(m_fdc, FUNC(c8050_fdc_device::drv_sel_w));
	m_miot->pb_wr_callback<4>().set(m_fdc, FUNC(c8050_fdc_device::odd_hd_w));
	m_miot->pb_rd_callback<6>().set_constant(0); // DOUBLE SIDED

	FLOPPY_CONNECTOR(config, FDC_TAG ":0", c8250_floppies, "525qd", c8250lp_device::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, FDC_TAG ":1", c8250_floppies, "525qd", c8250lp_device::floppy_formats).enable_sound(true);
}

void sfd1001_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);
	m_fdccpu->set_addrmap(AS_PROGRAM, &sfd1001_device::sfd1001_fdc_mem);

	m_miot->pb_wr_callback<4>().set(m_fdc, FUNC(c8050_fdc_device::odd_hd_w));
	m_miot->pb_rd_callback<6>().set_constant(0); // DOUBLE SIDED

	FLOPPY_CONNECTOR(config, FDC_TAG ":0", sfd1001_floppies, "525qd", sfd1001_device::floppy_formats).enable_sound(true);
}


//-------------------------------------------------
//  INPUT_PORTS( c8050 )
//-------------------------------------------------

static INPUT_PORTS_START( c8050 )
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

ioport_constructor c8050_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c8050 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void c8050_device::update_ieee_signals()
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
//  c8050_device - constructor
//-------------------------------------------------

c8050_device::c8050_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_ieee488_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_fdccpu(*this, M6504_TAG),
	m_riot0(*this, M6532_0_TAG),
	m_riot1(*this, M6532_1_TAG),
	m_miot(*this, M6530_TAG),
	m_via(*this, "um3"),
	m_floppy0(*this, FDC_TAG ":0"),
	m_floppy1(*this, FDC_TAG ":1"),
	m_fdc(*this, FDC_TAG),
	m_address(*this, "ADDRESS"),
	m_leds(*this, "led%u", 0U),
	m_rfdo(1),
	m_daco(1),
	m_atna(1),
	m_ifc(0)
{
}

c8050_device::c8050_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c8050_device(mconfig, C8050, tag, owner, clock)
{
}


//-------------------------------------------------
//  c8250_device - constructor
//-------------------------------------------------

c8250_device::c8250_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c8050_device(mconfig, C8250, tag, owner, clock)
{
}


//-------------------------------------------------
//  c8250lp_device - constructor
//-------------------------------------------------

c8250lp_device::c8250lp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c8050_device(mconfig, C8250LP, tag, owner, clock)
{
}


//-------------------------------------------------
//  sfd1001_device - constructor
//-------------------------------------------------

sfd1001_device::sfd1001_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	c8050_device(mconfig, SFD1001, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c8050_device::device_start()
{
	m_leds.resolve();

	// install image callbacks
	m_fdc->set_floppy(m_floppy0, m_floppy1);

	// register for state saving
	save_item(NAME(m_rfdo));
	save_item(NAME(m_daco));
	save_item(NAME(m_atna));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c8050_device::device_reset()
{
	// toggle M6502 SO
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, ASSERT_LINE);
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, CLEAR_LINE);

	// release ATN
	m_riot1->pa_bit_w<7>(0);

	// turn off spindle motors
	m_fdc->mtr0_w(1);
	m_fdc->mtr1_w(1);
}


//-------------------------------------------------
//  ieee488_atn -
//-------------------------------------------------

void c8050_device::ieee488_atn(int state)
{
	update_ieee_signals();

	m_riot1->pa_bit_w<7>(!state);
}


//-------------------------------------------------
//  ieee488_ifc -
//-------------------------------------------------

void c8050_device::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}
