/**********************************************************************

    Commodore 2040/3040/4040/8050/8250/SFD-1001 Disk Drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - 8050/8250 only work when debugger is active!?

    - 2040 DOS 1 FDC rom (jumps to 104d while getting block header)

        FE70: jsr  $104D
        104D: m6502_brk#$00

    - Micropolis 8x50 stepper motor is same as 4040, except it takes 4 pulses to step a track instead of 1

    - BASIC program to set 8250/SFD-1001 to 8050 mode:

        10 OPEN 15,8,15
        20 PRINT#15,"M-W"CHR$(172)CHR$(16)CHR$(1)CHR$(1)
        30 PRINT#15,"M-W"CHR$(195)CHR$(16)CHR$(1)CHR$(0)
        40 PRINT#15,"U9"
        50 CLOSE 15

*/

#include "c2040.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG		"un1"
#define M6532_0_TAG		"uc1"
#define M6532_1_TAG		"ue1"

#define M6504_TAG		"uh3"
#define M6522_TAG		"um3"
#define M6530_TAG		"uk3"


enum
{
	LED_POWER = 0,
	LED_ACT0,
	LED_ACT1,
	LED_ERR
};


#define SYNC \
	(!(((m_sr & G64_SYNC_MARK) == G64_SYNC_MARK) & m_rw))

#define ERROR \
	(!(m_ready | BIT(m_e, 3)))



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C2040 = &device_creator<c2040_device>;
const device_type C3040 = &device_creator<c3040_device>;
const device_type C4040 = &device_creator<c4040_device>;
const device_type C8050 = &device_creator<c8050_device>;
const device_type C8250 = &device_creator<c8250_device>;
const device_type C8250LP = &device_creator<c8250lp_device>;
const device_type SFD1001 = &device_creator<sfd1001_device>;


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void base_c2040_device::device_config_complete()
{
	switch (m_variant)
	{
	default:
	case TYPE_2040:
		m_shortname = "c2040";
		break;

	case TYPE_3040:
		m_shortname = "c3040";
		break;

	case TYPE_4040:
		m_shortname = "c4040";
		break;

	case TYPE_8050:
		m_shortname = "c8050";
		break;

	case TYPE_8250:
		m_shortname = "c8250";
		break;

	case TYPE_8250LP:
		m_shortname = "c8250lp";
		break;

	case TYPE_SFD1001:
		m_shortname = "sfd1001";
		break;
	}
}


//-------------------------------------------------
//  ROM( c2040 )
//-------------------------------------------------

ROM_START( c2040 ) // schematic 320806
	ROM_REGION( 0x3000, M6502_TAG, 0 )
    ROM_DEFAULT_BIOS("dos12")
    ROM_SYSTEM_BIOS( 0, "dos10", "DOS 1.0" )
	ROMX_LOAD( "901468-xx.ul1", 0x1000, 0x1000, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "901468-xx.uh1", 0x2000, 0x1000, NO_DUMP, ROM_BIOS(1) )
    ROM_SYSTEM_BIOS( 1, "dos12", "DOS 1.2" )
	ROMX_LOAD( "901468-06.ul1", 0x1000, 0x1000, CRC(25b5eed5) SHA1(4d9658f2e6ff3276e5c6e224611a66ce44b16fc7), ROM_BIOS(2) )
	ROMX_LOAD( "901468-07.uh1", 0x2000, 0x1000, CRC(9b09ae83) SHA1(6a51c7954938439ca8342fc295bda050c06e1791), ROM_BIOS(2) )

	ROM_REGION( 0x400, M6504_TAG, 0 )
	ROMX_LOAD( "901466-01.uk3", 0x000, 0x400, NO_DUMP, ROM_BIOS(1) )
	ROMX_LOAD( "901466-02.uk3", 0x000, 0x400, NO_DUMP, ROM_BIOS(2) ) // 6530-028 BAD_DUMP CRC(e1c86c43) SHA1(d8209c66fde3f2937688ba934ba968678a9d2ebb) ) // parsed in from disassembly

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6",    0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  ROM( c4040 )
//-------------------------------------------------

ROM_START( c4040 ) // schematic ?
	ROM_REGION( 0x3000, M6502_TAG, 0 )
    ROM_DEFAULT_BIOS("dos20r2")
    ROM_SYSTEM_BIOS( 0, "dos20r1", "DOS 2.0 Revision 1" )
	ROMX_LOAD( "901468-11.uj1", 0x0000, 0x1000, CRC(b7157458) SHA1(8415f3159dea73161e0cef7960afa6c76953b6f8), ROM_BIOS(1) )
	ROMX_LOAD( "901468-12.ul1", 0x1000, 0x1000, CRC(02c44ff9) SHA1(e8a94f239082d45f64f01b2d8e488d18fe659cbb), ROM_BIOS(1) )
	ROMX_LOAD( "901468-13.uh1", 0x2000, 0x1000, CRC(cbd785b3) SHA1(6ada7904ac9d13c3f1c0a8715f9c4be1aa6eb0bb), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "dos20r2", "DOS 2.0 Revision 2" )
	ROMX_LOAD( "901468-14.uj1", 0x0000, 0x1000, CRC(bc4d4872) SHA1(ffb992b82ec913ddff7be964d7527aca3e21580c), ROM_BIOS(2) )
	ROMX_LOAD( "901468-15.ul1", 0x1000, 0x1000, CRC(b6970533) SHA1(f702d6917fe8a798740ba4d467b500944ae7b70a), ROM_BIOS(2) )
	ROMX_LOAD( "901468-16.uh1", 0x2000, 0x1000, CRC(1f5eefb7) SHA1(04b918cf4adeee8015b43383d3cea7288a7d0aa8), ROM_BIOS(2) )

	ROM_REGION( 0x400, M6504_TAG, 0 )
	// RIOT DOS 2
	ROM_LOAD( "901466-04.uk3", 0x000, 0x400, CRC(0ab338dc) SHA1(6645fa40b81be1ff7d1384e9b52df06a26ab0bfb) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6",    0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


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

	ROM_REGION( 0x400, M6504_TAG, 0 )
	ROM_LOAD_OPTIONAL( "901483-02.uk3", 0x000, 0x400, NO_DUMP )	// 6530-036
	ROM_LOAD_OPTIONAL( "901483-03.uk3", 0x000, 0x400, CRC(9e83fa70) SHA1(e367ea8a5ddbd47f13570088427293138a10784b) ) // 6530-038 RIOT DOS 2.5 Micropolis
	ROM_LOAD_OPTIONAL( "901483-04.uk3", 0x000, 0x400, NO_DUMP ) // 6530-039 RIOT DOS 2.5 Tandon
	ROM_LOAD_OPTIONAL( "901884-01.uk3", 0x000, 0x400, NO_DUMP ) // 6530-40 RIOT DOS 2.7 Tandon
	ROM_LOAD_OPTIONAL( "901885-01.uk3", 0x000, 0x400, NO_DUMP ) // 6530-044
	ROM_LOAD_OPTIONAL( "901885-04.uk3", 0x000, 0x400, CRC(bab998c9) SHA1(0dc9a3b60f1b866c63eebd882403532fc59fe57f) ) // 6530-47 RIOT DOS 2.7 Micropolis
	ROM_LOAD( "901869-01.uk3", 0x000, 0x400, CRC(2915327a) SHA1(3a9a80f72ce76e5f5c72513f8ef7553212912ae3) ) // 6530-48 RIOT DOS 2.7 MPI

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6", 0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


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

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "251167-01.uc1", 0x000, 0x800, BAD_DUMP CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  ROM( sfd1001 )
//-------------------------------------------------

ROM_START( sfd1001 ) // schematic 251406
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "901887-01.1j",  0x0000, 0x2000, CRC(0073b8b2) SHA1(b10603195f240118fe5fb6c6dfe5c5097463d890) )
	ROM_LOAD( "901888-01.3j",  0x2000, 0x2000, CRC(de9b6132) SHA1(2e6c2d7ca934e5c550ad14bd5e9e7749686b7af4) )

	ROM_REGION( 0x800, M6504_TAG, 0 )
	ROM_LOAD( "901885-04.u1", 0x000, 0x400, CRC(bab998c9) SHA1(0dc9a3b60f1b866c63eebd882403532fc59fe57f) )
	ROM_LOAD( "251257-02a.u2", 0x000, 0x800, CRC(b51150de) SHA1(3b954eb34f7ea088eed1d33ebc6d6e83a3e9be15) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467-01.5c",  0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *base_c2040_device::device_rom_region() const
{
	switch (m_variant)
	{
	default:
	case TYPE_2040:
	case TYPE_3040:
		return ROM_NAME( c2040 );

	case TYPE_4040:
		return ROM_NAME( c4040 );

	case TYPE_8050:
	case TYPE_8250:
		return ROM_NAME( c8050 );

	case TYPE_8250LP:
		return ROM_NAME( c8250lp );

	case TYPE_SFD1001:
		return ROM_NAME( sfd1001 );
	}
}


//-------------------------------------------------
//  ADDRESS_MAP( c2040_main_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c2040_main_mem, AS_PROGRAM, 8, base_c2040_device )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0100) AM_RAM // 6532 #1
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x0100) AM_RAM // 6532 #2
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0x0d60) AM_DEVREADWRITE_LEGACY(M6532_0_TAG, riot6532_r, riot6532_w)
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0x0d60) AM_DEVREADWRITE_LEGACY(M6532_1_TAG, riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x5000, 0x7fff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( c2040_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c2040_fdc_mem, AS_PROGRAM, 8, base_c2040_device )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x003f) AM_MIRROR(0x0300) AM_RAM // 6530
	AM_RANGE(0x0040, 0x004f) AM_MIRROR(0x0330) AM_DEVREADWRITE(M6522_TAG, via6522_device, read, write)
	AM_RANGE(0x0080, 0x008f) AM_MIRROR(0x0330) AM_DEVREADWRITE_LEGACY(M6530_TAG, mos6530_r, mos6530_w)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1c00, 0x1fff) AM_ROM AM_REGION(M6504_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( c8050_main_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c8050_main_mem, AS_PROGRAM, 8, base_c2040_device )
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0100) AM_RAM // 6532 #1
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x0100) AM_RAM // 6532 #2
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0x0d60) AM_DEVREADWRITE_LEGACY(M6532_0_TAG, riot6532_r, riot6532_w)
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0x0d60) AM_DEVREADWRITE_LEGACY(M6532_1_TAG, riot6532_r, riot6532_w)
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share4")
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( c8050_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c8050_fdc_mem, AS_PROGRAM, 8, base_c2040_device )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x003f) AM_MIRROR(0x0300) AM_RAM // 6530
	AM_RANGE(0x0040, 0x004f) AM_MIRROR(0x0330) AM_DEVREADWRITE(M6522_TAG, via6522_device, read, write)
	AM_RANGE(0x0080, 0x008f) AM_MIRROR(0x0330) AM_DEVREADWRITE_LEGACY(M6530_TAG, mos6530_r, mos6530_w)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1c00, 0x1fff) AM_ROM AM_REGION(M6504_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( c8250lp_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c8250lp_fdc_mem, AS_PROGRAM, 8, base_c2040_device )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x003f) AM_MIRROR(0x0300) AM_RAM // 6530
	AM_RANGE(0x0040, 0x004f) AM_MIRROR(0x0330) AM_DEVREADWRITE(M6522_TAG, via6522_device, read, write)
	AM_RANGE(0x0080, 0x008f) AM_MIRROR(0x0330) AM_DEVREADWRITE_LEGACY(M6530_TAG, mos6530_r, mos6530_w)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1800, 0x1fff) AM_ROM AM_REGION(M6504_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( sfd1001_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( sfd1001_fdc_mem, AS_PROGRAM, 8, base_c2040_device )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x003f) AM_MIRROR(0x0300) AM_RAM // 6530
	AM_RANGE(0x0040, 0x004f) AM_MIRROR(0x0330) AM_DEVREADWRITE(M6522_TAG, via6522_device, read, write)
	AM_RANGE(0x0080, 0x008f) AM_MIRROR(0x0330) AM_DEVREADWRITE_LEGACY(M6530_TAG, mos6530_r, mos6530_w)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1800, 0x1fff) AM_ROM AM_REGION(M6504_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  riot6532_interface riot0_intf uc1
//-------------------------------------------------

READ8_MEMBER( base_c2040_device::dio_r )
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


WRITE8_MEMBER( base_c2040_device::dio_w )
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


static const riot6532_interface riot0_intf =
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, dio_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, dio_w),
	DEVCB_NULL
};


//-------------------------------------------------
//  riot6532_interface riot1_intf ue1
//-------------------------------------------------

READ8_MEMBER( base_c2040_device::riot1_pa_r )
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


WRITE8_MEMBER( base_c2040_device::riot1_pa_w )
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


READ8_MEMBER( base_c2040_device::riot1_pb_r )
{
	/*

        bit     description

        PB0     DEVICE NUMBER SELECTION
        PB1     DEVICE NUMBER SELECTION
        PB2     DEVICE NUMBER SELECTION
        PB3     ACT LED 1
        PB4     ACT LED 0
        PB5     ERR LED
        PB6     DACI
        PB7     RFDI

    */

	UINT8 data = 0;

	// device number selection
	data |= m_address - 8;

	// data accepted in
	data |= m_bus->ndac_r() << 6;

	// ready for data in
	data |= m_bus->nrfd_r() << 7;

	return data;
}


WRITE8_MEMBER( base_c2040_device::riot1_pb_w )
{
	/*

        bit     description

        PB0     DEVICE NUMBER SELECTION
        PB1     DEVICE NUMBER SELECTION
        PB2     DEVICE NUMBER SELECTION
        PB3     ACT LED 1
        PB4     ACT LED 0
        PB5     ERR LED
        PB6     DACI
        PB7     RFDI

    */

	// activity led 1
	output_set_led_value(LED_ACT1, BIT(data, 3));

	// activity led 0
	output_set_led_value(LED_ACT0, BIT(data, 4));

	// error led
	output_set_led_value(LED_ERR, BIT(data, 5));
}


static const riot6532_interface riot1_intf =
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, riot1_pa_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, riot1_pb_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, riot1_pa_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, riot1_pb_w),
	DEVCB_CPU_INPUT_LINE(M6502_TAG, INPUT_LINE_IRQ0)
};


//-------------------------------------------------
//  via6522_interface via_intf um3
//-------------------------------------------------

READ8_MEMBER( base_c2040_device::via_pa_r )
{
	/*

        bit     description

        PA0     E0
        PA1     E1
        PA2     I2
        PA3     E2
        PA4     E4
        PA5     E5
        PA6     I7
        PA7     E6

    */

	UINT16 i = m_i;
	UINT8 e = m_e;
	UINT8 data = (BIT(e, 6) << 7) | (BIT(i, 7) << 6) | (e & 0x33) | (BIT(e, 2) << 3) | (i & 0x04);

	return data;
}


WRITE8_MEMBER( base_c2040_device::via_pb_w )
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
	int mtr1 = BIT(data, 4);
	spindle_motor(1, mtr1);

	// spindle motor 0
	int mtr0 = BIT(data, 5);
	spindle_motor(0, mtr0);

	// stepper motor 1
	int s1 = data & 0x03;
	micropolis_step_motor(1, s1);

	// stepper motor 0
	int s0 = (data >> 2) & 0x03;
	micropolis_step_motor(0, s0);

	m_bit_timer->enable(!mtr1 || !mtr0);
}


READ_LINE_MEMBER( base_c2040_device::ready_r )
{
	return m_ready;
}


READ_LINE_MEMBER( base_c2040_device::err_r )
{
	return ERROR;
}


WRITE_LINE_MEMBER( base_c2040_device::mode_sel_w )
{
	// mode select
	m_mode = state;

	update_gcr_data();
	m_via->write_cb1(ERROR);
}

WRITE_LINE_MEMBER( base_c2040_device::rw_sel_w )
{
	// read/write select
	m_rw = state;

	update_gcr_data();
	m_via->write_cb1(ERROR);
}


static const via6522_interface via_intf =
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, via_pa_r),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, ready_r),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, err_r),
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, via_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, mode_sel_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, rw_sel_w),

	DEVCB_NULL
};


//-------------------------------------------------
//  via6522_interface c8050_via_intf um3
//-------------------------------------------------

READ8_MEMBER( base_c2040_device::c8050_via_pb_r )
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
        PB7     SYNC

    */

	UINT8 data = 0;

	// SYNC detected
	data |= SYNC << 7;

	return data;
}


WRITE8_MEMBER( base_c2040_device::c8050_via_pb_w )
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
        PB7     SYNC

    */

	// spindle motor 1
	int mtr1 = BIT(data, 4);
	spindle_motor(1, mtr1);

	// spindle motor 0
	int mtr0 = BIT(data, 5);
	spindle_motor(0, mtr0);

	// stepper motor 1
	int s1 = data & 0x03;
	mpi_step_motor(1, s1);

	// stepper motor 0
	int s0 = (data >> 2) & 0x03;
	mpi_step_motor(0, s0);

	m_bit_timer->enable(!mtr1 || !mtr0);
}


static const via6522_interface c8050_via_intf =
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, via_pa_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, c8050_via_pb_r),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, ready_r),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, err_r),
	DEVCB_NULL,
	DEVCB_NULL,

	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, c8050_via_pb_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, mode_sel_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, rw_sel_w),

	DEVCB_NULL
};


//-------------------------------------------------
//  mos6530_interface miot_intf uk3
//-------------------------------------------------

READ8_MEMBER( base_c2040_device::pi_r )
{
	/*

        bit     description

        PA0     PI0
        PA1     PI1
        PA2     PI2
        PA3     PI3
        PA4     PI4
        PA5     PI5
        PA6     PI6
        PA7     PI7

    */

	return m_pi;
}

WRITE8_MEMBER( base_c2040_device::pi_w )
{
	/*

        bit     description

        PA0     PI0
        PA1     PI1
        PA2     PI2
        PA3     PI3
        PA4     PI4
        PA5     PI5
        PA6     PI6
        PA7     PI7

    */

	m_pi = data;
}

READ8_MEMBER( base_c2040_device::miot_pb_r )
{
	/*

        bit     description

        PB0     DRV SEL
        PB1     DS0
        PB2     DS1
        PB3     WPS
        PB4
        PB5
        PB6     SYNC
        PB7     M6504 IRQ

    */

	UINT8 data = 0;

	// write protect sense
	data |= floppy_wpt_r(m_unit[m_drive].m_image) << 3;

	// SYNC detected
	data |= SYNC << 6;

	return data;
}

WRITE8_MEMBER( base_c2040_device::miot_pb_w )
{
	/*

        bit     description

        PB0     DRV SEL
        PB1     DS0
        PB2     DS1
        PB3     WPS
        PB4
        PB5
        PB6     SYNC
        PB7     M6504 IRQ

    */

	// drive select
	m_drive = BIT(data, 0);

	// density select
	int ds = (data >> 1) & 0x03;

	if (m_ds != ds)
	{
		m_bit_timer->adjust(attotime::zero, 0, attotime::from_hz(C2040_BITRATE[ds]/4));
		m_ds = ds;
	}

	// interrupt
	if (m_miot_irq != BIT(data, 7))
	{
		m_fdccpu->set_input_line(M6502_IRQ_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
		m_miot_irq = BIT(data, 7);
	}
}

static MOS6530_INTERFACE( miot_intf )
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, pi_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, pi_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, miot_pb_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, miot_pb_w)
};


//-------------------------------------------------
//  mos6530_interface c8050_miot_intf uk3
//-------------------------------------------------

READ8_MEMBER( base_c2040_device::c8050_miot_pb_r )
{
	/*

        bit     description

        PB0     DRV SEL
        PB1     DS0
        PB2     DS1
        PB3     WPS
        PB4     DRIVE TYPE (0=2A, 1=2C)
        PB5
        PB6     (0=DS, 1=SS)
        PB7     M6504 IRQ

    */

	UINT8 data = 0;

	// write protect sense
	data |= floppy_wpt_r(m_unit[m_drive].m_image) << 3;

	// drive type
	data |= 0x10;

	// single/dual sided
	if (m_variant == base_c2040_device::TYPE_8050)
	{
		data |= 0x40;
	}

	return data;
}

WRITE8_MEMBER( base_c2040_device::c8050_miot_pb_w )
{
	/*

        bit     description

        PB0     DRV SEL
        PB1     DS0
        PB2     DS1
        PB3     WPS
        PB4     ODD HD (0=78-154, 1=1-77)
        PB5
        PB6     (0=DS, 1=SS)
        PB7     M6504 IRQ

    */

	// drive select
	if ((m_variant == base_c2040_device::TYPE_8050) ||
		(m_variant == base_c2040_device::TYPE_8250))
	{
		m_drive = BIT(data, 0);
	}

	// density select
	int ds = (data >> 1) & 0x03;

	if (m_ds != ds)
	{
		m_bit_timer->adjust(attotime::zero, 0, attotime::from_hz(C8050_BITRATE[ds]));
		m_ds = ds;
	}

	// side select
	if ((m_variant == base_c2040_device::TYPE_8250) ||
		(m_variant == base_c2040_device::TYPE_SFD1001))
	{
		m_side = !BIT(data, 4);
	}

	// interrupt
	if (m_miot_irq != BIT(data, 7))
	{
		m_fdccpu->set_input_line(M6502_IRQ_LINE, BIT(data, 7) ? CLEAR_LINE : ASSERT_LINE);
		m_miot_irq = BIT(data, 7);
	}
}

static MOS6530_INTERFACE( c8050_miot_intf )
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, pi_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, pi_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, c8050_miot_pb_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c2040_device, c8050_miot_pb_w)
};


//-------------------------------------------------
//  LEGACY_FLOPPY_OPTIONS( c2040 )
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( c2040 )
	LEGACY_FLOPPY_OPTION( c2040, "d67", "Commodore 2040/3040 Disk Image", d67_dsk_identify, d64_dsk_construct, NULL, NULL )
	LEGACY_FLOPPY_OPTION( c2040, "g64", "Commodore 2040/3040 GCR Disk Image", g64_dsk_identify, g64_dsk_construct, NULL, NULL )
LEGACY_FLOPPY_OPTIONS_END


//-------------------------------------------------
//  LEGACY_FLOPPY_OPTIONS( c4040 )
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( c4040 )
	LEGACY_FLOPPY_OPTION( c4040, "d64", "Commodore 4040 Disk Image", d64_dsk_identify, d64_dsk_construct, NULL, NULL )
	LEGACY_FLOPPY_OPTION( c4040, "g64", "Commodore 4040 GCR Disk Image", g64_dsk_identify, g64_dsk_construct, NULL, NULL )
LEGACY_FLOPPY_OPTIONS_END


//-------------------------------------------------
//  LEGACY_FLOPPY_OPTIONS( c8050 )
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( c8050 )
	LEGACY_FLOPPY_OPTION( c8050, "d80", "Commodore 8050 Disk Image", d80_dsk_identify, d64_dsk_construct, NULL, NULL )
LEGACY_FLOPPY_OPTIONS_END


//-------------------------------------------------
//  LEGACY_FLOPPY_OPTIONS( c8250 )
//-------------------------------------------------

static LEGACY_FLOPPY_OPTIONS_START( c8250 )
	LEGACY_FLOPPY_OPTION( c8250, "d80", "Commodore 8050 Disk Image", d80_dsk_identify, d64_dsk_construct, NULL, NULL )
	LEGACY_FLOPPY_OPTION( c8250, "d82", "Commodore 8250/SFD1001 Disk Image", d82_dsk_identify, d64_dsk_construct, NULL, NULL )
LEGACY_FLOPPY_OPTIONS_END


//-------------------------------------------------
//  floppy_interface c2040_floppy_interface
//-------------------------------------------------

static const floppy_interface c2040_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(c2040),
	"floppy_5_25",
	NULL
};


//-------------------------------------------------
//  floppy_interface c4040_floppy_interface
//-------------------------------------------------

static const floppy_interface c4040_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(c4040),
	"floppy_5_25",
	NULL
};


//-------------------------------------------------
//  floppy_interface c8050_floppy_interface
//-------------------------------------------------

static const floppy_interface c8050_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_SSDD,
	LEGACY_FLOPPY_OPTIONS_NAME(c8050),
	"floppy_5_25",
	NULL
};


//-------------------------------------------------
//  floppy_interface c8250_floppy_interface
//-------------------------------------------------

static const floppy_interface c8250_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSQD,
	LEGACY_FLOPPY_OPTIONS_NAME(c8250),
	"floppy_5_25",
	NULL
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c2040 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c2040 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_main_mem)

	MCFG_RIOT6532_ADD(M6532_0_TAG, XTAL_16MHz/16, riot0_intf)
	MCFG_RIOT6532_ADD(M6532_1_TAG, XTAL_16MHz/16, riot1_intf)

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_fdc_mem)

	MCFG_VIA6522_ADD(M6522_TAG, XTAL_16MHz/16, via_intf)
	MCFG_MOS6530_ADD(M6530_TAG, XTAL_16MHz/16, miot_intf)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(c2040_floppy_interface)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c4040 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c4040 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_main_mem)

	MCFG_RIOT6532_ADD(M6532_0_TAG, XTAL_16MHz/16, riot0_intf)
	MCFG_RIOT6532_ADD(M6532_1_TAG, XTAL_16MHz/16, riot1_intf)

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_fdc_mem)

	MCFG_VIA6522_ADD(M6522_TAG, XTAL_16MHz/16, via_intf)
	MCFG_MOS6530_ADD(M6530_TAG, XTAL_16MHz/16, miot_intf)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(c4040_floppy_interface)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c8050 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c8050 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_main_mem)

	MCFG_RIOT6532_ADD(M6532_0_TAG, XTAL_12MHz/12, riot0_intf)
	MCFG_RIOT6532_ADD(M6532_1_TAG, XTAL_12MHz/12, riot1_intf)

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_fdc_mem)

	MCFG_VIA6522_ADD(M6522_TAG, XTAL_12MHz/12, c8050_via_intf)
	MCFG_MOS6530_ADD(M6530_TAG, XTAL_12MHz/12, c8050_miot_intf)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(c8050_floppy_interface)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c8250 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c8250 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_main_mem)

	MCFG_RIOT6532_ADD(M6532_0_TAG, XTAL_12MHz/12, riot0_intf)
	MCFG_RIOT6532_ADD(M6532_1_TAG, XTAL_12MHz/12, riot1_intf)

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_fdc_mem)

	MCFG_VIA6522_ADD(M6522_TAG, XTAL_12MHz/12, c8050_via_intf)
	MCFG_MOS6530_ADD(M6530_TAG, XTAL_12MHz/12, c8050_miot_intf)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(c8250_floppy_interface)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c8250lp )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c8250lp )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_main_mem)

	MCFG_RIOT6532_ADD(M6532_0_TAG, XTAL_12MHz/12, riot0_intf)
	MCFG_RIOT6532_ADD(M6532_1_TAG, XTAL_12MHz/12, riot1_intf)

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8250lp_fdc_mem)

	MCFG_VIA6522_ADD(M6522_TAG, XTAL_12MHz/12, c8050_via_intf)
	MCFG_MOS6530_ADD(M6530_TAG, XTAL_12MHz/12, c8050_miot_intf)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(c8250_floppy_interface)
MACHINE_CONFIG_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( sfd1001 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( sfd1001 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(c8050_main_mem)

	MCFG_RIOT6532_ADD(M6532_0_TAG, XTAL_12MHz/12, riot0_intf)
	MCFG_RIOT6532_ADD(M6532_1_TAG, XTAL_12MHz/12, riot1_intf)

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_12MHz/12)
	MCFG_CPU_PROGRAM_MAP(sfd1001_fdc_mem)

	MCFG_VIA6522_ADD(M6522_TAG, XTAL_12MHz/12, c8050_via_intf)
	MCFG_MOS6530_ADD(M6530_TAG, XTAL_12MHz/12, c8050_miot_intf)

	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, c8250_floppy_interface)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor base_c2040_device::device_mconfig_additions() const
{
	switch (m_variant)
	{
	default:
	case TYPE_2040:
	case TYPE_3040:
		return MACHINE_CONFIG_NAME( c2040 );

	case TYPE_4040:
		return MACHINE_CONFIG_NAME( c4040 );

	case TYPE_8050:
		return MACHINE_CONFIG_NAME( c8050 );

	case TYPE_8250:
		return MACHINE_CONFIG_NAME( c8250 );

	case TYPE_8250LP:
		return MACHINE_CONFIG_NAME( c8250lp );

	case TYPE_SFD1001:
		return MACHINE_CONFIG_NAME( sfd1001 );
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void base_c2040_device::update_ieee_signals()
{
	int atn = m_bus->atn_r();
	int nrfd = !(!(!(atn && m_atna) && m_rfdo) || !(atn || m_atna));
	int ndac = !(m_daco || !(atn || m_atna));

	m_bus->nrfd_w(this, nrfd);
	m_bus->ndac_w(this, ndac);
}


//-------------------------------------------------
//  update_gcr_data -
//-------------------------------------------------

inline void base_c2040_device::update_gcr_data()
{
	if (m_rw)
	{
		/*

            bit     description

            I0      SR0
            I1      SR1
            I2      SR2
            I3      SR3
            I4      SR4
            I5      SR5
            I6      SR6
            I7      SR7
            I8      SR8
            I9      SR9
            I10     R/_W SEL

        */

		m_i = (m_rw << 10) | (m_sr & 0x3ff);
	}
	else
	{
		/*

            bit     description

            I0      PI0
            I1      PI1
            I2      PI2
            I3      PI3
            I4      MODE SEL
            I5      PI4
            I6      PI5
            I7      PI6
            I8      PI7
            I9      0
            I10     R/_W SEL

        */

		m_i = (m_rw << 10) | ((m_pi & 0xf0) << 1) | (m_mode << 4) | (m_pi & 0x0f);
	}

	m_e = m_gcr[m_i];
}


//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void base_c2040_device::read_current_track(int unit)
{
	m_unit[unit].m_track_len = G64_BUFFER_SIZE;
	m_unit[unit].m_buffer_pos = 0;
	m_unit[unit].m_bit_pos = 7;
	m_bit_count = 0;

	// read track data
	floppy_drive_read_track_data_info_buffer(m_unit[unit].m_image, m_side, m_unit[unit].m_track_buffer, &m_unit[unit].m_track_len);

	// extract track length
	m_unit[unit].m_track_len = floppy_drive_get_current_track_size(m_unit[unit].m_image, m_side);
}


//-------------------------------------------------
//  spindle_motor -
//-------------------------------------------------

inline void base_c2040_device::spindle_motor(int unit, int mtr)
{
	if (m_unit[unit].m_mtr != mtr)
	{
		if (!mtr)
		{
			// read track data
			read_current_track(unit);
		}

		floppy_mon_w(m_unit[unit].m_image, mtr);

		m_unit[unit].m_mtr = mtr;
	}
}


//-------------------------------------------------
//  micropolis_step_motor -
//-------------------------------------------------

inline void base_c2040_device::micropolis_step_motor(int unit, int stp)
{
	if (!m_unit[unit].m_mtr && (m_unit[unit].m_stp != stp))
	{
		int tracks = 0;

		switch (m_unit[unit].m_stp)
		{
		case 0:	if (stp == 1) tracks++; else if (stp == 3) tracks--; break;
		case 1:	if (stp == 2) tracks++; else if (stp == 0) tracks--; break;
		case 2: if (stp == 3) tracks++; else if (stp == 1) tracks--; break;
		case 3: if (stp == 0) tracks++; else if (stp == 2) tracks--; break;
		}

		if (tracks != 0)
		{
			// step read/write head
			floppy_drive_seek(m_unit[unit].m_image, tracks);

			// read new track data
			read_current_track(unit);
		}

		m_unit[unit].m_stp = stp;
	}
}


//-------------------------------------------------
//  mpi_step_motor -
//-------------------------------------------------

inline void base_c2040_device::mpi_step_motor(int unit, int stp)
{
	if (!m_unit[unit].m_mtr && (m_unit[unit].m_stp != stp))
	{
		int tracks = 0;

		switch (m_unit[unit].m_stp)
		{
		case 0:	if (stp == 1) tracks++; else if (stp == 2) tracks--; break;
		case 1:	if (stp == 3) tracks++; else if (stp == 0) tracks--; break;
		case 2: if (stp == 0) tracks++; else if (stp == 3) tracks--; break;
		case 3: if (stp == 2) tracks++; else if (stp == 1) tracks--; break;
		}

		if (tracks != 0)
		{
			// step read/write head
			floppy_drive_seek(m_unit[unit].m_image, tracks);

			// read new track data
			read_current_track(unit);
		}

		m_unit[unit].m_stp = stp;
	}
}


//-------------------------------------------------
//  initialize -
//-------------------------------------------------

inline void base_c2040_device::initialize(int drives)
{
	// find GCR ROM
	m_gcr = memregion("gcr")->base();

	// install image callbacks
	m_unit[0].m_image = m_image0;

	floppy_install_unload_proc(m_image0, base_c2040_device::on_disk0_change);
	floppy_install_load_proc(m_image0, base_c2040_device::on_disk0_change);

	if (drives == 2)
	{
		m_unit[1].m_image = m_image1;

		floppy_install_unload_proc(m_image1, base_c2040_device::on_disk1_change);
		floppy_install_load_proc(m_image1, base_c2040_device::on_disk1_change);
	}

	// register for state saving
	save_item(NAME(m_drive));
	save_item(NAME(m_side));
	save_item(NAME(m_rfdo));
	save_item(NAME(m_daco));
	save_item(NAME(m_atna));
	save_item(NAME(m_ds));
	save_item(NAME(m_bit_count));
	save_item(NAME(m_sr));
	save_item(NAME(m_pi));
	save_item(NAME(m_i));
	save_item(NAME(m_e));
	save_item(NAME(m_ready));
	save_item(NAME(m_mode));
	save_item(NAME(m_rw));
	save_item(NAME(m_miot_irq));
	save_item(NAME(m_unit[0].m_stp));
	save_item(NAME(m_unit[0].m_mtr));
	save_item(NAME(m_unit[0].m_track_len));
	save_item(NAME(m_unit[0].m_buffer_pos));
	save_item(NAME(m_unit[0].m_bit_pos));

	if (drives == 2)
	{
		save_item(NAME(m_unit[1].m_stp));
		save_item(NAME(m_unit[1].m_mtr));
		save_item(NAME(m_unit[1].m_track_len));
		save_item(NAME(m_unit[1].m_buffer_pos));
		save_item(NAME(m_unit[1].m_bit_pos));
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  base_c2040_device - constructor
//-------------------------------------------------

base_c2040_device::base_c2040_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant)
    : device_t(mconfig, type, name, tag, owner, clock),
	  device_ieee488_interface(mconfig, *this),
	  m_maincpu(*this, M6502_TAG),
	  m_fdccpu(*this, M6504_TAG),
	  m_riot0(*this, M6532_0_TAG),
	  m_riot1(*this, M6532_1_TAG),
	  m_miot(*this, M6530_TAG),
	  m_via(*this, M6522_TAG),
	  m_image0(*this, FLOPPY_0),
	  m_image1(*this, FLOPPY_1),
	  m_drive(0),
	  m_side(0),
	  m_rfdo(1),
	  m_daco(1),
	  m_atna(1),
	  m_ds(-1),
	  m_bit_count(0),
	  m_ready(0),
	  m_mode(0),
	  m_rw(0),
	  m_miot_irq(CLEAR_LINE),
	  m_variant(variant)
{
	for (int i = 0; i < 2; i++)
	{
		m_unit[i].m_stp = 0;
		m_unit[i].m_mtr = 1;
		m_unit[i].m_track_len = 0;
		m_unit[i].m_buffer_pos = 0;
		m_unit[i].m_bit_pos = 0;
	}
}


//-------------------------------------------------
//  base_c2040_device - constructor
//-------------------------------------------------

c2040_device::c2040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c2040_device(mconfig, C2040, "C2040", tag, owner, clock, TYPE_2040) { }


//-------------------------------------------------
//  c3040_device - constructor
//-------------------------------------------------

c3040_device::c3040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c2040_device(mconfig, C3040, "C3040", tag, owner, clock, TYPE_3040) { }


//-------------------------------------------------
//  c4040_device - constructor
//-------------------------------------------------

c4040_device::c4040_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c2040_device(mconfig, C4040, "C4040", tag, owner, clock, TYPE_4040) { }


//-------------------------------------------------
//  c8050_device - constructor
//-------------------------------------------------

c8050_device::c8050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c2040_device(mconfig, C8050, "C8050", tag, owner, clock, TYPE_8050) { }


//-------------------------------------------------
//  c8250_device - constructor
//-------------------------------------------------

c8250_device::c8250_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c2040_device(mconfig, C8250, "C8250", tag, owner, clock, TYPE_8250) { }


//-------------------------------------------------
//  c8250lp_device - constructor
//-------------------------------------------------

c8250lp_device::c8250lp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c2040_device(mconfig, C8250LP, "C8250LP", tag, owner, clock, TYPE_8250LP) { }


//-------------------------------------------------
//  sfd1001_device - constructor
//-------------------------------------------------

sfd1001_device::sfd1001_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c2040_device(mconfig, SFD1001, "SFD1001", tag, owner, clock, TYPE_SFD1001) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void base_c2040_device::device_start()
{
	m_bit_timer = timer_alloc();

	switch (m_variant)
	{
	default:
		initialize(2);
		break;

	case base_c2040_device::TYPE_SFD1001:
		initialize(1);
		break;
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void base_c2040_device::device_reset()
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

	// turn off spindle motors
	m_unit[0].m_mtr = m_unit[1].m_mtr = 1;
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void base_c2040_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int ready = 1;

	// shift in data from the read head
	m_sr <<= 1;
	m_sr |= BIT(m_unit[m_drive].m_track_buffer[m_unit[m_drive].m_buffer_pos], m_unit[m_drive].m_bit_pos);

	// update GCR data
	update_gcr_data();

	// update bit counters
	m_unit[m_drive].m_bit_pos--;
	m_bit_count++;

	if (m_unit[m_drive].m_bit_pos < 0)
	{
		m_unit[m_drive].m_bit_pos = 7;
		m_unit[m_drive].m_buffer_pos++;

		if (m_unit[m_drive].m_buffer_pos >= m_unit[m_drive].m_track_len)
		{
			// loop to the start of the track
			m_unit[m_drive].m_buffer_pos = 0;
		}
	}

	if (!SYNC)
	{
		// SYNC detected
		m_bit_count = 0;
	}

	if (m_bit_count == 10)
	{
		// byte ready
		m_bit_count = 0;
		ready = 0;
	}

	if (m_ready != ready)
	{
		// set byte ready flag
		m_ready = ready;

		m_via->write_ca1(ready);
		m_via->write_cb1(ERROR);

		if ((m_variant == base_c2040_device::TYPE_8050) ||
			(m_variant == base_c2040_device::TYPE_8250) ||
			(m_variant == base_c2040_device::TYPE_SFD1001))
		{
			m_fdccpu->set_input_line(M6502_SET_OVERFLOW, ready ? CLEAR_LINE : ASSERT_LINE);
		}
	}
}


//-------------------------------------------------
//  ieee488_atn -
//-------------------------------------------------

void base_c2040_device::ieee488_atn(int state)
{
	update_ieee_signals();

	// set RIOT PA7
	riot6532_porta_in_set(m_riot1, !state << 7, 0x80);
}


//-------------------------------------------------
//  ieee488_ifc -
//-------------------------------------------------

void base_c2040_device::ieee488_ifc(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  on_disk0_change -
//-------------------------------------------------

void base_c2040_device::on_disk0_change(device_image_interface &image)
{
    base_c2040_device *c2040 = static_cast<base_c2040_device *>(image.device().owner());

	c2040->read_current_track(0);
}


//-------------------------------------------------
//  on_disk_change -
//-------------------------------------------------

void base_c2040_device::on_disk1_change(device_image_interface &image)
{
    base_c2040_device *c2040 = static_cast<base_c2040_device *>(image.device().owner());

	c2040->read_current_track(1);
}
