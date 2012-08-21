/***************************************************************************

    drivers/amstr_pc.c

    Driver file for Amstrad PC1512 and related machines.

    PC-XT memory map

    00000-9FFFF   RAM
    A0000-AFFFF   NOP       or videoram EGA/VGA
    B0000-B7FFF   videoram  MDA, page #0
    B8000-BFFFF   videoram  CGA and/or MDA page #1, T1T mapped RAM
    C0000-C7FFF   NOP       or ROM EGA/VGA
    C8000-C9FFF   ROM       XT HDC #1
    CA000-CBFFF   ROM       XT HDC #2
    D0000-EFFFF   NOP       or 'adapter RAM'
    F0000-FDFFF   NOP       or ROM Basic + other Extensions
    FE000-FFFFF   ROM

Amstrad PC1640
==============

More information can be found at http://www.seasip.info/AmstradXT/1640tech/index.html

***************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "cpu/i86/i86.h"
#include "sound/speaker.h"

#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"

#include "machine/pit8253.h"
#include "video/pc_vga.h"
#include "video/pc_cga.h"
#include "video/pc_aga.h"
#include "video/pc_t1t.h"

#include "machine/pc_fdc.h"
#include "machine/pc_joy.h"
#include "machine/pckeybrd.h"
#include "machine/pc_lpt.h"

#include "includes/amstr_pc.h"

#include "machine/pcshare.h"
#include "includes/pc.h"

#include "imagedev/flopdrv.h"
#include "imagedev/harddriv.h"
#include "imagedev/cassette.h"
#include "imagedev/cartslot.h"
#include "formats/pc_dsk.h"

#include "machine/8237dma.h"
#include "sound/sn76496.h"

#include "machine/ram.h"

static ADDRESS_MAP_START( ppc512_map, AS_PROGRAM, 16, pc_state )
	AM_RANGE(0x00000, 0x7ffff) AM_RAMBANK("bank10")
	AM_RANGE(0x80000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( ppc640_map, AS_PROGRAM, 16, pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ppc512_io, AS_IO, 16, pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8_LEGACY("dma8237", i8237_r, i8237_w, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0065) AM_READWRITE8_LEGACY(pc1640_port60_r, pc1640_port60_w, 0xffff)
	AM_RANGE(0x0070, 0x0071) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffff)
	AM_RANGE(0x0078, 0x0079) AM_READWRITE8_LEGACY(pc1640_mouse_x_r,	pc1640_mouse_x_w, 0xffff)
	AM_RANGE(0x007a, 0x007b) AM_READWRITE8_LEGACY(pc1640_mouse_y_r,	pc1640_mouse_y_w, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r, pc_page_w, 0xffff)
	AM_RANGE(0x0200, 0x0207) AM_READWRITE8_LEGACY(pc_JOY_r,	pc_JOY_w, 0xffff)
	AM_RANGE(0x0278, 0x027b) AM_READ8_LEGACY(pc200_port278_r, 0xffff) AM_DEVWRITE8_LEGACY("lpt_2", pc_lpt_w, 0x00ff)
	AM_RANGE(0x02e8, 0x02ef) AM_DEVREADWRITE8("ins8250_3", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0378, 0x037b) AM_READ8_LEGACY(pc200_port378_r, 0xffff) AM_DEVWRITE8_LEGACY("lpt_1", pc_lpt_w, 0x00ff)
	AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE8_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w, 0x00ff)
	AM_RANGE(0x03e8, 0x03ef) AM_DEVREADWRITE8("ins8250_2", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_READWRITE8_LEGACY(pc_fdc_r,					pc_fdc_w, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc200_map, AS_PROGRAM, 16, pc_state )
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc200_io, AS_IO, 16, pc_state )
	AM_RANGE(0x0000, 0x000f) AM_DEVREADWRITE8_LEGACY("dma8237", i8237_r, i8237_w, 0xffff)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE8_LEGACY("pic8259", pic8259_r, pic8259_w, 0xffff)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE8_LEGACY("pit8253", pit8253_r, pit8253_w, 0xffff)
	AM_RANGE(0x0060, 0x0065) AM_READWRITE8_LEGACY(pc1640_port60_r, pc1640_port60_w, 0xffff)
	AM_RANGE(0x0078, 0x0079) AM_READWRITE8_LEGACY(pc1640_mouse_x_r, pc1640_mouse_x_w, 0xffff)
	AM_RANGE(0x007a, 0x007b) AM_READWRITE8_LEGACY(pc1640_mouse_y_r, pc1640_mouse_y_w, 0xffff)
	AM_RANGE(0x0080, 0x0087) AM_READWRITE8(pc_page_r, pc_page_w, 0xffff)
	AM_RANGE(0x0200, 0x0207) AM_READWRITE8_LEGACY(pc_JOY_r, pc_JOY_w, 0xffff)
	AM_RANGE(0x0278, 0x027b) AM_READ8_LEGACY(pc200_port278_r, 0xffff) AM_DEVWRITE8_LEGACY("lpt_2", pc_lpt_w, 0x00ff)
	AM_RANGE(0x02e8, 0x02ef) AM_DEVREADWRITE8("ins8250_3", ins8250_device, ins8250_r,  ins8250_w, 0xffff)
	AM_RANGE(0x02f8, 0x02ff) AM_DEVREADWRITE8("ins8250_1", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x0378, 0x037b) AM_READ8_LEGACY(pc200_port378_r, 0xffff) AM_DEVWRITE8_LEGACY("lpt_1", pc_lpt_w, 0x00ff)
	AM_RANGE(0x03bc, 0x03bf) AM_DEVREADWRITE8_LEGACY("lpt_0", pc_lpt_r, pc_lpt_w, 0x00ff)
	AM_RANGE(0x03e8, 0x03ef) AM_DEVREADWRITE8("ins8250_2", ins8250_device, ins8250_r, ins8250_w, 0xffff)
	AM_RANGE(0x03f0, 0x03f7) AM_READWRITE8_LEGACY(pc_fdc_r,					pc_fdc_w, 0xffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ins8250_0", ins8250_device, ins8250_r, ins8250_w, 0xffff)
ADDRESS_MAP_END


static INPUT_PORTS_START( pc200 )
	PORT_START("IN0") /* IN0 */
	PORT_BIT ( 0xf0, 0xf0,	 IPT_UNUSED )
	PORT_BIT ( 0x08, 0x08,	 IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT ( 0x07, 0x07,	 IPT_UNUSED )

	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0x07, 0x07, "Name/Language")
	PORT_DIPSETTING(	0x00, "English/less checks" )
	PORT_DIPSETTING(	0x01, DEF_STR( Italian ) ) //prego attendere
	PORT_DIPSETTING(	0x02, "V.g. v\xC3\xA4nta" )
	PORT_DIPSETTING(	0x03, "Vent et cjeblik" ) // seldom c
	PORT_DIPSETTING(	0x04, DEF_STR( Spanish ) ) //Por favor
	PORT_DIPSETTING(	0x05, DEF_STR( French ) ) //patientez
	PORT_DIPSETTING(	0x06, DEF_STR( German ) ) // bitte warten
	PORT_DIPSETTING(	0x07, DEF_STR( English ) ) // please wait
	PORT_DIPNAME( 0x08, 0x00, "37a 0x40")
	PORT_DIPSETTING(	0x00, "0x00" )
	PORT_DIPSETTING(	0x08, "0x08" )
/* 2008-05 FP: This Dip Switch overlaps the next one.
Since pc200 is anyway NOT_WORKING, I comment out this one */
/*  PORT_DIPNAME( 0x10, 0x00, "37a 0x80")
    PORT_DIPSETTING(    0x00, "0x00" )
    PORT_DIPSETTING(    0x10, "0x10" ) */
	PORT_DIPNAME( 0x30, 0x00, "Integrated Graphics Adapter")
	PORT_DIPSETTING(	0x00, "CGA 1" )
	PORT_DIPSETTING(	0x10, "CGA 2" )
	PORT_DIPSETTING(	0x20, "external" )
	PORT_DIPSETTING(	0x30, "MDA" )
	PORT_DIPNAME( 0xc0, 0x80, "Startup Mode")
	PORT_DIPSETTING(	0x00, "external Color 80 Columns" )
	PORT_DIPSETTING(	0x40, "Color 40 Columns" )
	PORT_DIPSETTING(	0x80, "Color 80 Columns" )
	PORT_DIPSETTING(	0xc0, DEF_STR( Mono ) )

	PORT_START("DSW1") /* IN2 */
	PORT_BIT ( 0x80, 0x80,	 IPT_UNUSED ) // com 1 on motherboard
	PORT_DIPNAME( 0x40, 0x40, "COM2: enable")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x00, "COM3: enable")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "COM4: enable")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "LPT1: enable")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Yes ) )
	PORT_BIT ( 0x04, 0x04,	 IPT_UNUSED ) // lpt 1 on motherboard
	PORT_DIPNAME( 0x02, 0x00, "LPT3: enable")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x00, "Game port enable")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )

	PORT_START("DSW2") /* IN3 */
	PORT_DIPNAME( 0x08, 0x08, "HDC1 (C800:0 port 320-323)")
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "HDC2 (CA00:0 port 324-327)")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Yes ) )
	PORT_BIT( 0x02, 0x02,	IPT_UNUSED ) /* no turbo switch */
	PORT_BIT( 0x01, 0x01,	IPT_UNUSED )

	PORT_INCLUDE( at_keyboard )		/* IN4 - IN11 */
	PORT_INCLUDE( pc_joystick )			/* IN15 - IN19 */

	PORT_START("VIDEO") /* IN20 */
	PORT_CONFNAME( 0x03, 0x03, "IDA character set")
	PORT_CONFSETTING(0x00, "Greek")
	PORT_CONFSETTING(0x01, "Norwegian (Codepage 860)")
	PORT_CONFSETTING(0x02, "Portugese (Codepage 865)")
	PORT_CONFSETTING(0x03, "Default (Codepage 437)")
	PORT_CONFNAME( 0x1C, 0x00, "CGA monitor type")
	PORT_CONFSETTING(0x00, "Colour RGB")
	PORT_CONFSETTING(0x04, "Mono RGB")
	PORT_CONFSETTING(0x0C, "Television")
	PORT_BIT ( 0xE0, 0x40, IPT_UNUSED )	/* Chipset is always PPC512 */

INPUT_PORTS_END

static const unsigned i86_address_mask = 0x000fffff;

static const pc_lpt_interface pc_lpt_config =
{
	DEVCB_CPU_INPUT_LINE("maincpu", 0)
};

static const floppy_interface ibmpc_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(pc),
	NULL,
	NULL
};

SLOT_INTERFACE_START(amstr_com)
	SLOT_INTERFACE("microsoft_mouse", MSFT_SERIAL_MOUSE)
	SLOT_INTERFACE("mouse_systems_mouse", MSYSTEM_SERIAL_MOUSE)
SLOT_INTERFACE_END

#define MCFG_CPU_PC(mem, port, type, clock, vblankfunc)	\
	MCFG_CPU_ADD("maincpu", type, clock)				\
	MCFG_CPU_PROGRAM_MAP(mem##_map)	\
	MCFG_CPU_IO_MAP(port##_io)	\
	MCFG_TIMER_ADD_SCANLINE("scantimer", vblankfunc, "screen", 0, 1) \
	MCFG_CPU_CONFIG(i86_address_mask)


static const gfx_layout pc200_charlayout =
{
	8, 16,					/* 8 x 16 characters */
	2048,					/* 2048 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16					/* every char takes 16 bytes */
};

static GFXDECODE_START( pc200 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc200_charlayout, 3, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( pc200, pc_state )
	/* basic machine hardware */
	MCFG_CPU_PC(pc200, pc200, I8086, 8000000, pc_frame_interrupt)

	MCFG_MACHINE_START(pc)
	MCFG_MACHINE_RESET(pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )	/* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )	/* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )	/* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )	/* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], amstr_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], amstr_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[3], amstr_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[4], amstr_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_pc200 )
	MCFG_GFXDECODE(pc200)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_UPD765A_ADD("upd765", pc_fdc_upd765_not_connected_interface)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(ibmpc_floppy_interface)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END




static const gfx_layout pc1512_charlayout =
{
	8, 8,					/* 8 x 8 characters */
	1024,					/* 1024 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8					/* every char takes 8 bytes */
};

static GFXDECODE_START( pc1512 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc1512_charlayout, 3, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( ppc512, pc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 8000000)
	MCFG_CPU_PROGRAM_MAP(ppc512_map)
	MCFG_CPU_IO_MAP(ppc512_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer", pc_frame_interrupt, "screen", 0, 1)

	MCFG_MACHINE_START(pc)
	MCFG_MACHINE_RESET(pc)

	MCFG_PIT8253_ADD( "pit8253", ibm5150_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, ibm5150_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", ibm5150_pic8259_config )

	MCFG_I8255_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_INS8250_ADD( "ins8250_0", ibm5150_com_interface[0], XTAL_1_8432MHz )	/* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_1", ibm5150_com_interface[1], XTAL_1_8432MHz )	/* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_2", ibm5150_com_interface[2], XTAL_1_8432MHz )	/* TODO: Verify model */
	MCFG_INS8250_ADD( "ins8250_3", ibm5150_com_interface[3], XTAL_1_8432MHz )	/* TODO: Verify model */
	MCFG_RS232_PORT_ADD( "serport0", ibm5150_serport_config[0], amstr_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport1", ibm5150_serport_config[1], amstr_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport2", ibm5150_serport_config[3], amstr_com, NULL, NULL )
	MCFG_RS232_PORT_ADD( "serport3", ibm5150_serport_config[4], amstr_com, NULL, NULL )

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_pc200 )
	MCFG_GFXDECODE(pc200)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	/* printer */
	MCFG_PC_LPT_ADD("lpt_0", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_1", pc_lpt_config)
	MCFG_PC_LPT_ADD("lpt_2", pc_lpt_config)

	MCFG_UPD765A_ADD("upd765", pc_fdc_upd765_not_connected_interface)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(ibmpc_floppy_interface)

	MCFG_MC146818_ADD( "rtc", MC146818_IGNORE_CENTURY )

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ppc640, ppc512 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ppc640_map)

	/* internal ram */
	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")
MACHINE_CONFIG_END

/*
Sinclair PC200 ROMs (from a v1.2 PC200):

40109.ic159     -- Character set, the same as in the 1.5 PC200. Label:

            AMSTRAD
            40109
            8827 B

40184.ic132 -- BIOS v1.2.
40185.ic129    Labels are:

            AMSTRAD         AMSTRAD
            40184           40185
            V1.2:5EA8       V1.2:A058
*/
ROM_START( pc200 )
//  ROM_REGION(0x100000,"maincpu", 0)
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	// special bios at 0xe0000 !?
	ROM_SYSTEM_BIOS(0, "v15", "v1.5")
	ROMX_LOAD("40185-2.ic129", 0xfc001, 0x2000, CRC(41302eb8) SHA1(8b4b2afea543b96b45d6a30365281decc15f2932), ROM_SKIP(1) | ROM_BIOS(1)) // v2
	ROMX_LOAD("40184-2.ic132", 0xfc000, 0x2000, CRC(71b84616) SHA1(4135102a491b25fc659d70b957e07649f3eacf24), ROM_SKIP(1) | ROM_BIOS(1)) // v2
	ROM_SYSTEM_BIOS(1, "v12", "v1.2")
	ROMX_LOAD("40185.ic129", 0xfc001, 0x2000, CRC(c2b4eeac) SHA1(f11015fadf0c16d86ce2c5047be3e6a4782044f7), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("40184.ic132", 0xfc000, 0x2000, CRC(b22704a6) SHA1(dadd573db6cd34f339f2f0ae55b07537924c024a), ROM_SKIP(1) | ROM_BIOS(2))
	// also mapped to f0000, f4000, f8000
	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("40109.ic159",     0x00000, 0x08000, CRC(a8b67639) SHA1(99663bfb61798526e092205575370c2ad34249a1))

	ROM_REGION( 0x800, "keyboard", 0 )
	ROM_LOAD( "40112.ic801", 0x000, 0x800, CRC(842a954c) SHA1(93ca6badf20e0215025fe109959eddead8c52f38) )
ROM_END


ROM_START( pc20 )
//  ROM_REGION(0x100000,"maincpu", 0)
	ROM_REGION16_LE(0x100000,"maincpu", 0)

	// special bios at 0xe0000 !?
	// This is probably referring to a check for the Amstrad RP5-2 diagnostic
	// card, which can be plugged into an Amstrad XT for troubleshooting purposes.
	// - John Elliott
	ROM_LOAD16_BYTE("pc20v2.0", 0xfc001, 0x2000, CRC(41302eb8) SHA1(8b4b2afea543b96b45d6a30365281decc15f2932)) // v2
	ROM_LOAD16_BYTE("pc20v2.1", 0xfc000, 0x2000, CRC(71b84616) SHA1(4135102a491b25fc659d70b957e07649f3eacf24)) // v2
	// also mapped to f0000, f4000, f8000
	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("40109.bin",     0x00000, 0x08000, CRC(a8b67639) SHA1(99663bfb61798526e092205575370c2ad34249a1))
ROM_END


ROM_START( ppc512 )
//  ROM_REGION(0x100000,"maincpu", 0)
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	// special bios at 0xe0000 !?
	ROM_LOAD16_BYTE("40107.v1", 0xfc000, 0x2000, CRC(4e37e769) SHA1(88be3d3375ec3b0a7041dbcea225b197e50d4bfe)) // v1.9
	ROM_LOAD16_BYTE("40108.v1", 0xfc001, 0x2000, CRC(4f0302d9) SHA1(e4d69ca98c3b98f3705a2902b16746360043f039)) // v1.9
	// also mapped to f0000, f4000, f8000
	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("40109.bin",     0x00000, 0x08000, CRC(a8b67639) SHA1(99663bfb61798526e092205575370c2ad34249a1))

	ROM_REGION( 0x800, "keyboard", 0 ) // PPC512 / PPC640 / PC200 102-key keyboard
	ROM_LOAD( "40112.ic801", 0x000, 0x800, CRC(842a954c) SHA1(93ca6badf20e0215025fe109959eddead8c52f38) )
ROM_END


ROM_START( ppc640 )
//  ROM_REGION(0x100000,"maincpu", 0)
	ROM_REGION16_LE(0x100000,"maincpu", 0)
	// special bios at 0xe0000 !?
	ROM_LOAD16_BYTE("40107.v2", 0xfc000, 0x2000, CRC(0785b63e) SHA1(4dbde6b9e9500298bb6241a8daefd85927f1ad28)) // v2.1
	ROM_LOAD16_BYTE("40108.v2", 0xfc001, 0x2000, CRC(5351cf8c) SHA1(b4dbf11b39378ab4afd2107d3fe54a99fffdedeb)) // v2.1
	// also mapped to f0000, f4000, f8000
	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("40109.bin",     0x00000, 0x08000, CRC(a8b67639) SHA1(99663bfb61798526e092205575370c2ad34249a1))

	ROM_REGION( 0x800, "keyboard", 0 ) // PPC512 / PPC640 / PC200 102-key keyboard
	ROM_LOAD( "40112.ic801", 0x000, 0x800, CRC(842a954c) SHA1(93ca6badf20e0215025fe109959eddead8c52f38) )
ROM_END


ROM_START( pc2086 )
	ROM_REGION16_LE( 0x100000, "maincpu", 0 )
	ROM_LOAD(        "40186.ic171", 0xc0000, 0x8000, CRC(959f00ba) SHA1(5df1efe4203cd076292a7713bd7ebd1196dca577) )
	ROM_LOAD16_BYTE( "40179.ic129", 0xfc000, 0x2000, CRC(003605e4) SHA1(b882e97ee81b9ba0e7d969c63da3f2052f23b4b9) )
	ROM_LOAD16_BYTE( "40180.ic132", 0xfc001, 0x2000, CRC(28ee5e58) SHA1(93e045609466fcec74e2bb72578bb7405281cf7b) )

	ROM_REGION(0x08100,"gfx1", ROMREGION_ERASE00)

	ROM_REGION( 0x800, "keyboard", 0 ) // PC2086 / PC3086 102-key keyboard
	ROM_LOAD( "40178.ic801", 0x000, 0x800, CRC(f72f1c2e) SHA1(34897e78b3d10f96b36d81778e97c4a9a1b8618b) )
ROM_END


ROM_START( pc3086 )
	ROM_REGION16_LE( 0x100000, "maincpu", 0 )
	ROM_LOAD( "c000.bin", 0xc0000, 0x8000, CRC(5a6c38e9) SHA1(382d2028e0dc5515a68843829563ce29018edb08) )
	ROM_LOAD( "c800.bin", 0xc8000, 0x2000, CRC(3329c6d5) SHA1(982e852278185d69acde47a4f3942bc09ed76777) )
	ROM_LOAD( "fc00.bin", 0xfc000, 0x4000, CRC(b5630753) SHA1(98c344831cc4dc59ebb39bbb1961964a8d39fe20) )

	ROM_REGION(0x08100,"gfx1", ROMREGION_ERASE00)

	ROM_REGION( 0x800, "keyboard", 0 ) // PC2086 / PC3086 102-key keyboard
	ROM_LOAD( "40178.ic801", 0x000, 0x800, CRC(f72f1c2e) SHA1(34897e78b3d10f96b36d81778e97c4a9a1b8618b) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR     NAME        PARENT      COMPAT      MACHINE     INPUT       INIT        COMPANY     FULLNAME */
COMP(  1987,	ppc512,     ibm5150,	0,	ppc512,		pc200, pc_state,	ppc512,	"Amstrad plc",  "Amstrad PPC512", GAME_NOT_WORKING)
COMP(  1987,	ppc640,     ibm5150,	0,	ppc640,		pc200, pc_state,	ppc512,	"Amstrad plc",  "Amstrad PPC640", GAME_NOT_WORKING)
COMP(  1988,	pc20,       ibm5150,	0,	pc200,		pc200, pc_state,	pc200,	"Amstrad plc",  "Amstrad PC20" , GAME_NOT_WORKING)
COMP(  1988,	pc200,      ibm5150,	0,	pc200,		pc200, pc_state,	pc200,	"Sinclair Research Ltd",  "PC200 Professional Series", GAME_NOT_WORKING)
COMP(  1988,	pc2086,     ibm5150,	0,	pc200,      pc200, pc_state,	pc200,	"Amstrad plc",  "Amstrad PC2086", GAME_NOT_WORKING )
COMP(  1990,	pc3086,     ibm5150,	0,	pc200,      pc200, pc_state,	pc200,	"Amstrad plc",  "Amstrad PC3086", GAME_NOT_WORKING )
