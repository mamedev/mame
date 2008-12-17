/***************************************************************************

    Cinematronics Cosmic Chasm hardware

    driver by Mathis Rosenhauer

    Games supported:
        * Cosmic Chasm

    Known bugs:
        * none at this time

***************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "video/vector.h"
#include "machine/6840ptm.h"
#include "machine/z80ctc.h"
#include "cchasm.h"
#include "sound/custom.h"
#include "sound/ay8910.h"
#include "cpu/z80/z80daisy.h"

#define CCHASM_68K_CLOCK (XTAL_8MHz)

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( memmap, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x04000f) AM_READWRITE(ptm6840_0_lsb_r,ptm6840_0_lsb_w)
	AM_RANGE(0x050000, 0x050001) AM_WRITE(cchasm_refresh_control_w)
	AM_RANGE(0x060000, 0x060001) AM_READ_PORT("DSW") AM_WRITE(cchasm_led_w)
	AM_RANGE(0x070000, 0x070001) AM_WRITE(watchdog_reset16_w)
	AM_RANGE(0xf80000, 0xf800ff) AM_READWRITE(cchasm_io_r,cchasm_io_w)
	AM_RANGE(0xffb000, 0xffffff) AM_RAM AM_BASE(&cchasm_ram)
ADDRESS_MAP_END

/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x5000, 0x53ff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_READWRITE(cchasm_snd_io_r,cchasm_snd_io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE(Z80CTC, "ctc", z80ctc_r, z80ctc_w)
ADDRESS_MAP_END

static void cchasm_6840_irq(running_machine *machine, int state)
{
	cpu_set_input_line(machine->cpu[0], 4, state?ASSERT_LINE:CLEAR_LINE);
}
static const ptm6840_interface cchasm_6840_intf =
{
	CCHASM_68K_CLOCK/10,
	{ 0,CCHASM_68K_CLOCK/10,0 },
	{ NULL, NULL, NULL },
	cchasm_6840_irq
};

/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cchasm )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x06, "40000" )
	PORT_DIPSETTING(    0x04, "60000" )
	PORT_DIPSETTING(    0x02, "80000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "Once" )
	PORT_DIPSETTING(    0x10, "Every" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_1C ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN1")
	PORT_BIT( 0xff, 0, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Test 1") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) /* Test 2, not used in cchasm */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) /* Test 3, not used in cchasm */
INPUT_PORTS_END

static MACHINE_START( cchasm )
{
	ptm6840_config(machine, 0, &cchasm_6840_intf );
}



/*************************************
 *
 *  CPU config
 *
 *************************************/

static const z80_daisy_chain daisy_chain[] =
{
	{ Z80CTC, "ctc" },
	{ NULL }
};



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

extern z80ctc_interface cchasm_ctc_intf;

static MACHINE_DRIVER_START( cchasm )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", M68000,CCHASM_68K_CLOCK)	/* 8 MHz (from schematics) */
	MDRV_CPU_PROGRAM_MAP(memmap,0)

	MDRV_CPU_ADD("audio", Z80,3584229)		/* 3.58  MHz (from schematics) */
	MDRV_CPU_CONFIG(daisy_chain)
	MDRV_CPU_PROGRAM_MAP(sound_memmap,0)
	MDRV_CPU_IO_MAP(sound_portmap,0)

	MDRV_Z80CTC_ADD("ctc", cchasm_ctc_intf)

	MDRV_MACHINE_START(cchasm)

	/* video hardware */
	MDRV_SCREEN_ADD("main", VECTOR)
	MDRV_SCREEN_REFRESH_RATE(40)
	MDRV_SCREEN_SIZE(400, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)

	MDRV_VIDEO_START(cchasm)
	MDRV_VIDEO_UPDATE(vector)

	/* sound hardware */
	MDRV_SOUND_START(cchasm)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ay1", AY8910, 1818182)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("ay2", AY8910, 1818182)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( cchasm )
	ROM_REGION( 0x010000, "main", 0 )
    ROM_LOAD16_BYTE( "chasm.u4",  0x000000, 0x001000, CRC(19244f25) SHA1(79deaae82da8d1b16d05bbac43ba900c4b1d9f26) )
    ROM_LOAD16_BYTE( "chasm.u12", 0x000001, 0x001000, CRC(5d702c7d) SHA1(cbdceed45a1112594fbcbeb6976edc932b32d518) )
    ROM_LOAD16_BYTE( "chasm.u8",  0x002000, 0x001000, CRC(56a7ce8a) SHA1(14c790dcddb78d3b81b5a65fe3529e42c9708273) )
    ROM_LOAD16_BYTE( "chasm.u16", 0x002001, 0x001000, CRC(2e192db0) SHA1(1a8ff983295ab52b5099c089b3142cdc56d28aee) )
    ROM_LOAD16_BYTE( "chasm.u3",  0x004000, 0x001000, CRC(9c71c600) SHA1(900526eaff7483fc478ebfb3f14796ff8fd1d01f) )
    ROM_LOAD16_BYTE( "chasm.u11", 0x004001, 0x001000, CRC(a4eb59a5) SHA1(a7bb3ca8f1f000f224def6342ca9d1eabcb210e6) )
    ROM_LOAD16_BYTE( "chasm.u7",  0x006000, 0x001000, CRC(8308dd6e) SHA1(82ad7c27e9a41af5280ecd975d3530ff2ed27ad4) )
    ROM_LOAD16_BYTE( "chasm.u15", 0x006001, 0x001000, CRC(9d3abf97) SHA1(476d684182d92d66263df82e1b5c4ff24b6814e8) )
    ROM_LOAD16_BYTE( "u2",        0x008000, 0x001000, CRC(4e076ae7) SHA1(a72f5425b256785b810ee5f23917b44f778cfcd3) )
    ROM_LOAD16_BYTE( "u10",       0x008001, 0x001000, CRC(cc9e19ca) SHA1(6c46ec265c2cc0683470ed1df978b96b577c5ca1) )
    ROM_LOAD16_BYTE( "chasm.u6",  0x00a000, 0x001000, CRC(a96525d2) SHA1(1c41bc3bf051cf1830182cbde6fba4e56db7e431) )
    ROM_LOAD16_BYTE( "chasm.u14", 0x00a001, 0x001000, CRC(8e426628) SHA1(2d70a7717b18cc892332b9d5d2de3ceba6c1481d) )
    ROM_LOAD16_BYTE( "u1",        0x00c000, 0x001000, CRC(88b71027) SHA1(49fa676d7838c643d642fbc70579ce29e76ba724) )
    ROM_LOAD16_BYTE( "chasm.u9",  0x00c001, 0x001000, CRC(d90c9773) SHA1(4033f0579f0782db2157f6cbece53b0d74e61d4f) )
    ROM_LOAD16_BYTE( "chasm.u5",  0x00e000, 0x001000, CRC(e4a58b7d) SHA1(0e5f948cd110804e6119fafb4e3fa5904dd1390f) )
    ROM_LOAD16_BYTE( "chasm.u13", 0x00e001, 0x001000, CRC(877e849c) SHA1(bdeb97fcb7488e7f0866dd651204c362d2ec9f4f) )

	ROM_REGION( 0x10000, "audio", 0 )
	ROM_LOAD( "2732.bin", 0x0000, 0x1000, CRC(715adc4a) SHA1(426be4f3334ef7f2e8eb4d533e64276c30812aa3) )

	ROM_REGION( 0x0200, "plds", ROMREGION_DISPOSE )
	ROM_LOAD( "pal12l6.u76", 0x0000, 0x0034, CRC(a30e02b7) SHA1(572f6d3f03e559f12e3bd5e087d7680ac69e9182) )
	ROM_LOAD( "pal12l6.u77", 0x0100, 0x0034, CRC(458b9cdb) SHA1(a3bff56d805f6dc494d294f079c3580430acf317) )
ROM_END

ROM_START( cchasm1 )
	ROM_REGION( 0x010000, "main", 0 )
    ROM_LOAD16_BYTE( "chasm.u4",  0x000000, 0x001000, CRC(19244f25) SHA1(79deaae82da8d1b16d05bbac43ba900c4b1d9f26) )
    ROM_LOAD16_BYTE( "chasm.u12", 0x000001, 0x001000, CRC(5d702c7d) SHA1(cbdceed45a1112594fbcbeb6976edc932b32d518) )
    ROM_LOAD16_BYTE( "chasm.u8",  0x002000, 0x001000, CRC(56a7ce8a) SHA1(14c790dcddb78d3b81b5a65fe3529e42c9708273) )
    ROM_LOAD16_BYTE( "chasm.u16", 0x002001, 0x001000, CRC(2e192db0) SHA1(1a8ff983295ab52b5099c089b3142cdc56d28aee) )
    ROM_LOAD16_BYTE( "chasm.u3",  0x004000, 0x001000, CRC(9c71c600) SHA1(900526eaff7483fc478ebfb3f14796ff8fd1d01f) )
    ROM_LOAD16_BYTE( "chasm.u11", 0x004001, 0x001000, CRC(a4eb59a5) SHA1(a7bb3ca8f1f000f224def6342ca9d1eabcb210e6) )
    ROM_LOAD16_BYTE( "chasm.u7",  0x006000, 0x001000, CRC(8308dd6e) SHA1(82ad7c27e9a41af5280ecd975d3530ff2ed27ad4) )
    ROM_LOAD16_BYTE( "chasm.u15", 0x006001, 0x001000, CRC(9d3abf97) SHA1(476d684182d92d66263df82e1b5c4ff24b6814e8) )
    ROM_LOAD16_BYTE( "chasm.u2",  0x008000, 0x001000, CRC(008b26ef) SHA1(6758d77bf48f466b8692bf7c678a597792d8cfdb) )
    ROM_LOAD16_BYTE( "chasm.u10", 0x008001, 0x001000, CRC(c2c532a3) SHA1(d29d40d42a2f69de0b1e2ee6a32633468a94fd85) )
    ROM_LOAD16_BYTE( "chasm.u6",  0x00a000, 0x001000, CRC(a96525d2) SHA1(1c41bc3bf051cf1830182cbde6fba4e56db7e431) )
    ROM_LOAD16_BYTE( "chasm.u14", 0x00a001, 0x001000, CRC(8e426628) SHA1(2d70a7717b18cc892332b9d5d2de3ceba6c1481d) )
    ROM_LOAD16_BYTE( "chasm.u1",  0x00c000, 0x001000, CRC(e02293f8) SHA1(136757b3c9e0ebfde6c13c57ac52f5fdbf5fd65b) )
    ROM_LOAD16_BYTE( "chasm.u9",  0x00c001, 0x001000, CRC(d90c9773) SHA1(4033f0579f0782db2157f6cbece53b0d74e61d4f) )
    ROM_LOAD16_BYTE( "chasm.u5",  0x00e000, 0x001000, CRC(e4a58b7d) SHA1(0e5f948cd110804e6119fafb4e3fa5904dd1390f) )
    ROM_LOAD16_BYTE( "chasm.u13", 0x00e001, 0x001000, CRC(877e849c) SHA1(bdeb97fcb7488e7f0866dd651204c362d2ec9f4f) )

	ROM_REGION( 0x10000, "audio", 0 )
	ROM_LOAD( "2732.bin", 0x0000, 0x1000, CRC(715adc4a) SHA1(426be4f3334ef7f2e8eb4d533e64276c30812aa3) )
ROM_END




/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, cchasm,  0,      cchasm, cchasm, 0, ROT270, "Cinematronics / GCE", "Cosmic Chasm (set 1)", 0 )
GAME( 1983, cchasm1, cchasm, cchasm, cchasm, 0, ROT270, "Cinematronics / GCE", "Cosmic Chasm (set 2)", 0 )
