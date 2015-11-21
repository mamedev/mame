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

#include "c8050.h"



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

const device_type C8050 = &device_creator<c8050_t>;
const device_type C8250 = &device_creator<c8250_t>;
const device_type C8250LP = &device_creator<c8250lp_t>;
const device_type SFD1001 = &device_creator<sfd1001_t>;


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
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("dos27")
	ROM_SYSTEM_BIOS( 0, "dos25r1", "DOS 2.5 Revision 1" )
	ROMX_LOAD( "901482-01.ul1", 0x0000, 0x2000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "901482-02.uh1", 0x2000, 0x2000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "dos25r2", "DOS 2.5 Revision 2" )
	ROMX_LOAD( "901482-03.ul1", 0x0000, 0x2000, CRC(09a609b9) SHA1(166d8bfaaa9c4767f9b17ad63fc7ae77c199a64e), ROM_BIOS(2) )
	ROMX_LOAD( "901482-04.uh1", 0x2000, 0x2000, CRC(1bcf9df9) SHA1(217f4a8b348658bb365f4a1de21ecbaa6402b1c0), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "dos25r3", "DOS 2.5 Revision 3" )
	ROMX_LOAD( "901482-06.ul1", 0x0000, 0x2000, CRC(3cbd2756) SHA1(7f5fbed0cddb95138dd99b8fe84fddab900e3650), ROM_BIOS(3) )
	ROMX_LOAD( "901482-07.uh1", 0x2000, 0x2000, CRC(c7532d90) SHA1(0b6d1e55afea612516df5f07f4a6dccd3bd73963), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "dos27", "DOS 2.7" )// 2364 ROM DOS 2.7
	ROMX_LOAD( "901887-01.ul1", 0x0000, 0x2000, CRC(0073b8b2) SHA1(b10603195f240118fe5fb6c6dfe5c5097463d890), ROM_BIOS(4) )
	ROMX_LOAD( "901888-01.uh1", 0x2000, 0x2000, CRC(de9b6132) SHA1(2e6c2d7ca934e5c550ad14bd5e9e7749686b7af4), ROM_BIOS(4) )

	ROM_REGION( 0x400, M6530_TAG, 0 )
	ROM_LOAD_OPTIONAL( "901483-02.uk3", 0x000, 0x400, CRC(d7277f95) SHA1(7607f9357f3a08f2a9f20931058d60d9e3c17d39) ) // 6530-036
	ROM_LOAD_OPTIONAL( "901483-03.uk3", 0x000, 0x400, CRC(9e83fa70) SHA1(e367ea8a5ddbd47f13570088427293138a10784b) ) // 6530-038 RIOT DOS 2.5 Micropolis
	ROM_LOAD_OPTIONAL( "901483-04.uk3", 0x000, 0x400, CRC(ae1c7866) SHA1(13bdf0bb387159167534c07a4554964734373f11) ) // 6530-039 RIOT DOS 2.5 Tandon
	ROM_LOAD_OPTIONAL( "901884-01.uk3", 0x000, 0x400, CRC(9e9a9f90) SHA1(39498d7369a31ea7527b5044071acf35a84ea2ac) ) // 6530-40 RIOT DOS 2.7 Tandon
	ROM_LOAD_OPTIONAL( "901885-01.uk3", 0x000, 0x400, NO_DUMP ) // 6530-044
	ROM_LOAD_OPTIONAL( "901885-04.uk3", 0x000, 0x400, CRC(bab998c9) SHA1(0dc9a3b60f1b866c63eebd882403532fc59fe57f) ) // 6530-47 RIOT DOS 2.7 Micropolis
	ROM_LOAD( "901869-01.uk3", 0x000, 0x400, CRC(2915327a) SHA1(3a9a80f72ce76e5f5c72513f8ef7553212912ae3) ) // 6530-48 RIOT DOS 2.7 MPI
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c8050_t::device_rom_region() const
{
	return ROM_NAME( c8050 );
}


//-------------------------------------------------
//  ROM( c8250lp )
//-------------------------------------------------

ROM_START( c8250lp )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("dos27")
	ROM_SYSTEM_BIOS( 0, "dos27", "DOS 2.7" )
	ROMX_LOAD( "251165-01.ua11",  0x0000, 0x2000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "251166-01.ua13",  0x2000, 0x2000, NO_DUMP, ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "dos27b", "DOS 2.7B" )
	ROMX_LOAD( "dos-2.7b.bin", 0x0000, 0x4000, CRC(96e3b209) SHA1(9849300be9f2e0143c2ed2564d26a4ba3b27526c), ROM_BIOS(2) ) // CBM DOS 2.7B from the 8250LP inside 8296D
	ROM_SYSTEM_BIOS( 2, "speeddos", "SpeedDOS" )
	ROMX_LOAD( "speeddos-c000.ua11", 0x0000, 0x2000, CRC(46cc260f) SHA1(e9838635d6868e35ec9c161b6e5c1ad92a4a241a), ROM_BIOS(3) )
	ROMX_LOAD( "speeddos-e000.ua13", 0x2000, 0x2000, CRC(88cfd505) SHA1(0fb570b180504cd1fcb7d203d8d37ea3d7e72ab4), ROM_BIOS(3) )

	ROM_REGION( 0x800, M6504_TAG, 0 )
	ROMX_LOAD( "251256-02", 0x000, 0x400, NO_DUMP, ROM_BIOS(1) ) // 6530-050
	ROMX_LOAD( "251474-01b", 0x000, 0x400, CRC(9e9a9f90) SHA1(39498d7369a31ea7527b5044071acf35a84ea2ac), ROM_BIOS(1) ) // Matsushita
	ROMX_LOAD( "fdc-2.7b.bin", 0x000, 0x800, CRC(13a24482) SHA1(1cfa52d2ed245a95e6369b46a36c6c7aa3929931), ROM_BIOS(2) ) // CBM DOS 2.7B FDC ROM from the 8250LP inside 8296D
	ROMX_LOAD( "speeddos-fdc-f800.bin", 0x000, 0x800, CRC(253e760f) SHA1(3f7892a9bab84b633f45686bbbbe66bc2948c8e5), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c8250lp_t::device_rom_region() const
{
	return ROM_NAME( c8250lp );
}


//-------------------------------------------------
//  ROM( sfd1001 )
//-------------------------------------------------

ROM_START( sfd1001 ) // schematic 251406
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "901887-01.1j",  0x0000, 0x2000, CRC(0073b8b2) SHA1(b10603195f240118fe5fb6c6dfe5c5097463d890) )
	ROM_LOAD( "901888-01.3j",  0x2000, 0x2000, CRC(de9b6132) SHA1(2e6c2d7ca934e5c550ad14bd5e9e7749686b7af4) )

	ROM_REGION( 0x400, M6530_TAG, 0 )
	ROM_LOAD( "901885-04.u1", 0x000, 0x400, CRC(bab998c9) SHA1(0dc9a3b60f1b866c63eebd882403532fc59fe57f) )

	ROM_REGION( 0x800, M6504_TAG, 0 )
	ROM_LOAD( "251257-02a.u2", 0x000, 0x800, CRC(b51150de) SHA1(3b954eb34f7ea088eed1d33ebc6d6e83a3e9be15) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467-01.5c",  0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *sfd1001_t::device_rom_region() const
{
	return ROM_NAME( sfd1001 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c8050_main_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c8050_main_mem, AS_PROGRAM, 8, c8050_t )
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0100) AM_DEVICE(M6532_0_TAG, mos6532_t, ram_map)
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x0100) AM_DEVICE(M6532_1_TAG, mos6532_t, ram_map)
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0x0d60) AM_DEVICE(M6532_0_TAG, mos6532_t, io_map)
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0x0d60) AM_DEVICE(M6532_1_TAG, mos6532_t, io_map)
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share4")
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( c8050_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c8050_fdc_mem, AS_PROGRAM, 8, c8050_t )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x003f) AM_MIRROR(0x0300) AM_DEVICE(M6530_TAG, mos6530_t, ram_map)
	AM_RANGE(0x0040, 0x004f) AM_MIRROR(0x0330) AM_DEVICE(M6522_TAG, via6522_device, map)
	AM_RANGE(0x0080, 0x008f) AM_MIRROR(0x0330) AM_DEVICE(M6530_TAG, mos6530_t, io_map)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1c00, 0x1fff) AM_DEVICE(M6530_TAG, mos6530_t, rom_map)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( c8250lp_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c8250lp_fdc_mem, AS_PROGRAM, 8, c8050_t )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x003f) AM_MIRROR(0x0300) AM_DEVICE(M6530_TAG, mos6530_t, ram_map)
	AM_RANGE(0x0040, 0x004f) AM_MIRROR(0x0330) AM_DEVICE(M6522_TAG, via6522_device, map)
	AM_RANGE(0x0080, 0x008f) AM_MIRROR(0x0330) AM_DEVICE(M6530_TAG, mos6530_t, io_map)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1800, 0x1fff) AM_ROM AM_REGION(M6504_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( sfd1001_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( sfd1001_fdc_mem, AS_PROGRAM, 8, c8050_t )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x003f) AM_MIRROR(0x0300) AM_DEVICE(M6530_TAG, mos6530_t, ram_map)
	AM_RANGE(0x0040, 0x004f) AM_MIRROR(0x0330) AM_DEVICE(M6522_TAG, via6522_device, map)
	AM_RANGE(0x0080, 0x008f) AM_MIRROR(0x0330) AM_DEVICE(M6530_TAG, mos6530_t, io_map)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1800, 0x1fff) AM_ROM AM_REGION(M6504_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  riot6532 uc1
//-------------------------------------------------

READ8_MEMBER( c8050_t::dio_r )
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

WRITE8_MEMBER( c8050_t::dio_w )
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

READ8_MEMBER( c8050_t::riot1_pa_r )
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

	UINT8 data = 0;

	// end or identify in
	data |= m_bus->eoi_r() << 5;

	// data valid in
	data |= m_bus->dav_r() << 6;

	// attention
	data |= !m_bus->atn_r() << 7;

	return data;
}

WRITE8_MEMBER( c8050_t::riot1_pa_w )
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

READ8_MEMBER( c8050_t::riot1_pb_r )
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

WRITE8_MEMBER( c8050_t::riot1_pb_w )
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

WRITE8_MEMBER( c8050_t::via_pb_w )
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

static SLOT_INTERFACE_START( c8050_floppies )
	SLOT_INTERFACE( "525ssqd", FLOPPY_525_SSQD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c8050_t::floppy_formats )
	FLOPPY_D80_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  SLOT_INTERFACE( c8250_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( c8250_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  SLOT_INTERFACE( sfd1001_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( sfd1001_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD ) // Matsushita JU-570 / JU-570-2
SLOT_INTERFACE_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c8250_t::floppy_formats )
	FLOPPY_D80_FORMAT,
	FLOPPY_D82_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c8250lp_t::floppy_formats )
	FLOPPY_D80_FORMAT,
	FLOPPY_D82_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( sfd1001_t::floppy_formats )
	FLOPPY_D80_FORMAT,
	FLOPPY_D82_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c8050 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c8050 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_main_mem)

	MCFG_DEVICE_ADD(M6532_0_TAG, MOS6532n, XTAL_12MHz/12)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8050_t, dio_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8050_t, dio_w))

	MCFG_DEVICE_ADD(M6532_1_TAG, MOS6532n, XTAL_12MHz/12)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8050_t, riot1_pa_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(c8050_t, riot1_pa_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(c8050_t, riot1_pb_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8050_t, riot1_pb_w))
	MCFG_MOS6530n_IRQ_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_fdc_mem)

	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_12MHz/12)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(FDC_TAG, c8050_fdc_t, read))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c8050_t, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(FDC_TAG, c8050_fdc_t, mode_sel_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(FDC_TAG, c8050_fdc_t, rw_sel_w))

	MCFG_DEVICE_ADD(M6530_TAG, MOS6530n, XTAL_12MHz/12)
	MCFG_MOS6530n_OUT_PA_CB(DEVWRITE8(FDC_TAG, c8050_fdc_t, write))
	MCFG_MOS6530n_OUT_PB0_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, drv_sel_w))
	MCFG_MOS6530n_OUT_PB1_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, ds0_w))
	MCFG_MOS6530n_OUT_PB2_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, ds1_w))
	MCFG_MOS6530n_IN_PB3_CB(DEVREADLINE(FDC_TAG, c8050_fdc_t, wps_r))
	MCFG_MOS6530n_IN_PB6_CB(VCC) // SINGLE SIDED
	MCFG_MOS6530n_OUT_PB7_CB(INPUTLINE(M6504_TAG, M6502_IRQ_LINE))

	MCFG_DEVICE_ADD(FDC_TAG, C8050_FDC, XTAL_12MHz/2)
	MCFG_C8050_SYNC_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_pb7))
	MCFG_C8050_READY_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_C8050_BRDY_CALLBACK(INPUTLINE(M6504_TAG, M6502_SET_OVERFLOW)) MCFG_DEVCB_XOR(1)
	MCFG_C8050_ERROR_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":0", c8050_floppies, "525ssqd", c8050_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":1", c8050_floppies, "525ssqd", c8050_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c8050_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c8050 );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c8250 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c8250 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_main_mem)

	MCFG_DEVICE_ADD(M6532_0_TAG, MOS6532n, XTAL_12MHz/12)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8050_t, dio_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8050_t, dio_w))

	MCFG_DEVICE_ADD(M6532_1_TAG, MOS6532n, XTAL_12MHz/12)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8050_t, riot1_pa_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(c8050_t, riot1_pa_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(c8050_t, riot1_pb_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8050_t, riot1_pb_w))
	MCFG_MOS6530n_IRQ_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_fdc_mem)

	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_12MHz/12)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(FDC_TAG, c8050_fdc_t, read))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c8050_t, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(FDC_TAG, c8050_fdc_t, mode_sel_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(FDC_TAG, c8050_fdc_t, rw_sel_w))

	MCFG_DEVICE_ADD(M6530_TAG, MOS6530n, XTAL_12MHz/12)
	MCFG_MOS6530n_OUT_PA_CB(DEVWRITE8(FDC_TAG, c8050_fdc_t, write))
	MCFG_MOS6530n_OUT_PB0_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, drv_sel_w))
	MCFG_MOS6530n_OUT_PB1_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, ds0_w))
	MCFG_MOS6530n_OUT_PB2_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, ds1_w))
	MCFG_MOS6530n_IN_PB3_CB(DEVREADLINE(FDC_TAG, c8050_fdc_t, wps_r))
	MCFG_MOS6530n_OUT_PB4_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, odd_hd_w))
	MCFG_MOS6530n_IN_PB6_CB(GND) // DOUBLE SIDED
	MCFG_MOS6530n_OUT_PB7_CB(INPUTLINE(M6504_TAG, M6502_IRQ_LINE))

	MCFG_DEVICE_ADD(FDC_TAG, C8050_FDC, XTAL_12MHz/2)
	MCFG_C8050_SYNC_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_pb7))
	MCFG_C8050_READY_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_C8050_BRDY_CALLBACK(INPUTLINE(M6504_TAG, M6502_SET_OVERFLOW)) MCFG_DEVCB_XOR(1)
	MCFG_C8050_ERROR_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":0", c8250_floppies, "525qd", c8250_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":1", c8250_floppies, "525qd", c8250_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c8250_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c8250 );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c8250lp )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c8250lp )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_main_mem)

	MCFG_DEVICE_ADD(M6532_0_TAG, MOS6532n, XTAL_12MHz/12)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8050_t, dio_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8050_t, dio_w))

	MCFG_DEVICE_ADD(M6532_1_TAG, MOS6532n, XTAL_12MHz/12)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8050_t, riot1_pa_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(c8050_t, riot1_pa_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(c8050_t, riot1_pb_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8050_t, riot1_pb_w))
	MCFG_MOS6530n_IRQ_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8250lp_fdc_mem)

	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_12MHz/12)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(FDC_TAG, c8050_fdc_t, read))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c8050_t, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(FDC_TAG, c8050_fdc_t, mode_sel_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(FDC_TAG, c8050_fdc_t, rw_sel_w))

	MCFG_DEVICE_ADD(M6530_TAG, MOS6530n, XTAL_12MHz/12)
	MCFG_MOS6530n_OUT_PA_CB(DEVWRITE8(FDC_TAG, c8050_fdc_t, write))
	MCFG_MOS6530n_OUT_PB0_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, drv_sel_w))
	MCFG_MOS6530n_OUT_PB1_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, ds0_w))
	MCFG_MOS6530n_OUT_PB2_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, ds1_w))
	MCFG_MOS6530n_IN_PB3_CB(DEVREADLINE(FDC_TAG, c8050_fdc_t, wps_r))
	MCFG_MOS6530n_OUT_PB4_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, odd_hd_w))
	MCFG_MOS6530n_IN_PB6_CB(GND) // DOUBLE SIDED
	MCFG_MOS6530n_OUT_PB7_CB(INPUTLINE(M6504_TAG, M6502_IRQ_LINE))

	MCFG_DEVICE_ADD(FDC_TAG, C8050_FDC, XTAL_12MHz/2)
	MCFG_C8050_SYNC_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_pb7))
	MCFG_C8050_READY_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_C8050_BRDY_CALLBACK(INPUTLINE(M6504_TAG, M6502_SET_OVERFLOW)) MCFG_DEVCB_XOR(1)
	MCFG_C8050_ERROR_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":0", c8250_floppies, "525qd", c8250lp_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":1", c8250_floppies, "525qd", c8250lp_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c8250lp_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c8250lp );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sfd1001 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sfd1001 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_main_mem)

	MCFG_DEVICE_ADD(M6532_0_TAG, MOS6532n, XTAL_12MHz/12)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8050_t, dio_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8050_t, dio_w))

	MCFG_DEVICE_ADD(M6532_1_TAG, MOS6532n, XTAL_12MHz/12)
	MCFG_MOS6530n_IN_PA_CB(READ8(c8050_t, riot1_pa_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(c8050_t, riot1_pa_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(c8050_t, riot1_pb_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c8050_t, riot1_pb_w))
	MCFG_MOS6530n_IRQ_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(sfd1001_fdc_mem)

	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_12MHz/12)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(FDC_TAG, c8050_fdc_t, read))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c8050_t, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(FDC_TAG, c8050_fdc_t, mode_sel_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(FDC_TAG, c8050_fdc_t, rw_sel_w))

	MCFG_DEVICE_ADD(M6530_TAG, MOS6530n, XTAL_12MHz/12)
	MCFG_MOS6530n_OUT_PA_CB(DEVWRITE8(FDC_TAG, c8050_fdc_t, write))
	MCFG_MOS6530n_OUT_PB1_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, ds0_w))
	MCFG_MOS6530n_OUT_PB2_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, ds1_w))
	MCFG_MOS6530n_IN_PB3_CB(DEVREADLINE(FDC_TAG, c8050_fdc_t, wps_r))
	MCFG_MOS6530n_OUT_PB4_CB(DEVWRITELINE(FDC_TAG, c8050_fdc_t, odd_hd_w))
	MCFG_MOS6530n_IN_PB6_CB(GND) // DOUBLE SIDED
	MCFG_MOS6530n_OUT_PB7_CB(INPUTLINE(M6504_TAG, M6502_IRQ_LINE))

	MCFG_DEVICE_ADD(FDC_TAG, C8050_FDC, XTAL_12MHz/2)
	MCFG_C8050_SYNC_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_pb7))
	MCFG_C8050_READY_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_C8050_BRDY_CALLBACK(INPUTLINE(M6504_TAG, M6502_SET_OVERFLOW)) MCFG_DEVCB_XOR(1)
	MCFG_C8050_ERROR_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG ":0", sfd1001_floppies, "525qd", sfd1001_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sfd1001_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( sfd1001 );
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

ioport_constructor c8050_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( c8050 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void c8050_t::update_ieee_signals()
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
//  c8050_t - constructor
//-------------------------------------------------

c8050_t::c8050_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_ieee488_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_fdccpu(*this, M6504_TAG),
	m_riot0(*this, M6532_0_TAG),
	m_riot1(*this, M6532_1_TAG),
	m_miot(*this, M6530_TAG),
	m_via(*this, M6522_TAG),
	m_floppy0(*this, FDC_TAG ":0"),
	m_floppy1(*this, FDC_TAG ":1"),
	m_fdc(*this, FDC_TAG),
	m_address(*this, "ADDRESS"),
	m_rfdo(1),
	m_daco(1),
	m_atna(1), m_ifc(0)
{
}

c8050_t::c8050_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C8050, "C8050", tag, owner, clock, "c8050", __FILE__),
	device_ieee488_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_fdccpu(*this, M6504_TAG),
	m_riot0(*this, M6532_0_TAG),
	m_riot1(*this, M6532_1_TAG),
	m_miot(*this, M6530_TAG),
	m_via(*this, M6522_TAG),
	m_floppy0(*this, FDC_TAG ":0"),
	m_floppy1(*this, FDC_TAG ":1"),
	m_fdc(*this, FDC_TAG),
	m_address(*this, "ADDRESS"),
	m_rfdo(1),
	m_daco(1),
	m_atna(1), m_ifc(0)
{
}


//-------------------------------------------------
//  c8250_t - constructor
//-------------------------------------------------

c8250_t::c8250_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	c8050_t(mconfig, C8250, "C8250", tag, owner, clock, "c8250", __FILE__) { }


//-------------------------------------------------
//  c8250lp_t - constructor
//-------------------------------------------------

c8250lp_t::c8250lp_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	c8050_t(mconfig, C8250LP, "C8250LP", tag, owner, clock, "c8250lp", __FILE__) { }


//-------------------------------------------------
//  sfd1001_t - constructor
//-------------------------------------------------

sfd1001_t::sfd1001_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	c8050_t(mconfig, SFD1001, "SFD1001", tag, owner, clock, "sfd1001", __FILE__) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c8050_t::device_start()
{
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

void c8050_t::device_reset()
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

	m_riot1->pa7_w(1);

	// turn off spindle motors
	m_fdc->mtr0_w(1);
	m_fdc->mtr1_w(1);
}


//-------------------------------------------------
//  ieee488_atn -
//-------------------------------------------------

void c8050_t::ieee488_atn(int state)
{
	update_ieee_signals();

	m_riot1->pa7_w(state);
}


//-------------------------------------------------
//  ieee488_ifc -
//-------------------------------------------------

void c8050_t::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}
