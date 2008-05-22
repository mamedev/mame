/***************************************************************************

    HAR MadMax hardware

****************************************************************************

    Games supported:
        * Double Cheese
        * Lotto Fun 2
        * Fred Flintstones' Memory Match

    Known bugs:
        * Test/tilt buttons seem to be swapped compared to test mode
        * Don't know what the opto switches do

    [dcheese]
    * you can not spin the wheel by using the inc/dec buttons unless you
      switch back and forth between them.  The game seems to check for a
      constant turn rate and constant acceleration/deceleration, then not
      allow the wheel to start spinning.  This is most likely to stop
      people from rigging the game.

    [fredmem]
    * Controls are set up as a 3 x 3 matrix of buttons that match the 9
      positions on the screen.

**************************************************************************/


#include "driver.h"
#include "deprecat.h"
#include "machine/eeprom.h"
#include "machine/ticket.h"
#include "sound/bsmt2000.h"
#include "dcheese.h"


#define MAIN_OSC	14318180
#define SOUND_OSC	24000000



/*************************************
 *
 *  Local variables
 *
 *************************************/

static UINT8 irq_state[5];
static UINT8 soundlatch_full;
static UINT8 sound_control;
static UINT8 sound_msb_latch;



/*************************************
 *
 *  Interrupts
 *
 *************************************/

static void update_irq_state(running_machine *machine)
{
	int i;

	/* loop from high priority to low; if we find a live IRQ, assert it */
	for (i = 4; i >= 0; i--)
		if (irq_state[i])
		{
			cpunum_set_input_line(machine, 0, i, ASSERT_LINE);
			return;
		}

	/* otherwise, clear them all */
	cpunum_set_input_line(machine, 0, 7, CLEAR_LINE);
}


static IRQ_CALLBACK(irq_callback)
{
	/* auto-ack the IRQ */
	irq_state[irqline] = 0;
	update_irq_state(machine);

	/* vector is 0x40 + index */
	return 0x40 + irqline;
}


void dcheese_signal_irq(int which)
{
	irq_state[which] = 1;
	update_irq_state(Machine);
}


static INTERRUPT_GEN( dcheese_vblank )
{
	logerror("---- VBLANK ----\n");
	dcheese_signal_irq(4);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

static MACHINE_START( dcheese )
{
	cpunum_set_irq_callback(0, irq_callback);

	state_save_register_global_array(irq_state);
	state_save_register_global(soundlatch_full);
	state_save_register_global(sound_control);
	state_save_register_global(sound_msb_latch);
}



/*************************************
 *
 *  I/O ports
 *
 *************************************/

static READ16_HANDLER( port_0_r )
{
	return (input_port_read_indexed(machine, 0) & 0xff7f) | (EEPROM_read_bit() << 7);
}


static READ16_HANDLER( port_2_r )
{
	return (input_port_read_indexed(machine, 2) & 0xff1f) | (!soundlatch_full << 7) | (ticket_dispenser_r(machine, 0) >> 2);
}


static WRITE16_HANDLER( eeprom_control_w )
{
	/* toggles bit $0100 very frequently while waiting for things */
	/* bits $0080-$0010 are probably lamps */
	if (ACCESSING_BITS_0_7)
	{
		EEPROM_set_cs_line(~data & 8);
		EEPROM_write_bit(data & 2);
		EEPROM_set_clock_line(data & 4);
		ticket_dispenser_w(machine, 0, (data & 1) << 7);
	}
}


static WRITE16_HANDLER( sound_command_w )
{
	if (ACCESSING_BITS_0_7)
	{
		/* write the latch and set the IRQ */
		soundlatch_full = 1;
		cpunum_set_input_line(machine, 1, 0, ASSERT_LINE);
		soundlatch_w(machine, 0, data & 0xff);
	}
}



/*************************************
 *
 *  Sound CPU handlers
 *
 *************************************/

static READ8_HANDLER( sound_command_r )
{
	/* read the latch and clear the IRQ */
	soundlatch_full = 0;
	cpunum_set_input_line(machine, 1, 0, CLEAR_LINE);
	return soundlatch_r(machine, 0);
}


static READ8_HANDLER( sound_status_r )
{
	/* seems to be ready signal on BSMT or latching hardware */
	return 0x80;
}


static WRITE8_HANDLER( sound_control_w )
{
	UINT8 diff = data ^ sound_control;
	sound_control = data;

	/* bit 0x20 = LED */
	/* bit 0x40 = BSMT2000 reset */
	if ((diff & 0x40) && (data & 0x40))
		sndti_reset(SOUND_BSMT2000, 0);
	if (data != 0x40 && data != 0x60)
		logerror("%04X:sound_control_w = %02X\n", activecpu_get_pc(), data);
}


static WRITE8_HANDLER( bsmt_data_w )
{
	/* writes come in pairs; even bytes latch, odd bytes write */
	if (offset % 2 == 0)
		sound_msb_latch = data;
	else
		BSMT2000_data_0_w(machine, offset/2, (sound_msb_latch << 8) | data, 0xffff);
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_cpu_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM
	AM_RANGE(0x200000, 0x200001) AM_READWRITE(port_0_r, watchdog_reset16_w)
	AM_RANGE(0x220000, 0x220001) AM_READWRITE(input_port_1_word_r, madmax_blitter_color_w)
	AM_RANGE(0x240000, 0x240001) AM_READWRITE(port_2_r, eeprom_control_w)
	AM_RANGE(0x260000, 0x26001f) AM_WRITE(madmax_blitter_xparam_w)
	AM_RANGE(0x280000, 0x28001f) AM_WRITE(madmax_blitter_yparam_w)
	AM_RANGE(0x2a0000, 0x2a003f) AM_READWRITE(madmax_blitter_vidparam_r, madmax_blitter_vidparam_w)
	AM_RANGE(0x2e0000, 0x2e0001) AM_WRITE(sound_command_w)
	AM_RANGE(0x300000, 0x300001) AM_WRITE(madmax_blitter_unknown_w)
ADDRESS_MAP_END



/*************************************
 *
 *  Sound CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( sound_cpu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_READWRITE(sound_status_r, sound_control_w)
	AM_RANGE(0x0800, 0x0fff) AM_READ(sound_command_r)
	AM_RANGE(0x1000, 0x10ff) AM_MIRROR(0x0700) AM_WRITE(bsmt_data_w)
	AM_RANGE(0x1800, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0xffff) AM_ROM
ADDRESS_MAP_END



/*************************************
 *
 *  Input port definitions
 *
 *************************************/

static INPUT_PORTS_START( dcheese )
	PORT_START	/* 200000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE )		/* says tilt */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT )			/* says test */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )		/* EEPROM data */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 )		/* bump left */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 )		/* bump right */
	PORT_BIT( 0x1800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )		/* brake right */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 )		/* brake left */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* 220000 */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* 240000 */
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNKNOWN )		/* low 5 bits read as a unit */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* ticket status */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* sound->main buffer status (0=empty) */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* main->sound buffer status (1=empty) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* 2a0002 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )	// read as a unit
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON8 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 )	// opto 1
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON6 )	// opto 2
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* 2a000e */
	PORT_BIT( 0x00ff, 0x0000, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(30) PORT_REVERSE
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( lottof2 )
	PORT_START	/* 200000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )		/* EEPROM data */
	PORT_BIT( 0x1f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 )		/* button */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON2 )		/* ticket */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* 220000 */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* 240000 */
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNKNOWN )		/* low 5 bits read as a unit */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* ticket status */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* sound->main buffer status (0=empty) */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* main->sound buffer status (1=empty) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* 2a0002 */
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* 2a000e */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( fredmem )
	PORT_START	/* 200000 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x000c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SPECIAL )		/* EEPROM data */
	PORT_BIT( 0x1f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0xc000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* 220000 */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* 240000 */
	PORT_BIT( 0x001f, IP_ACTIVE_LOW, IPT_UNKNOWN )		/* low 5 bits read as a unit */
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* ticket status */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* sound->main buffer status (0=empty) */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_SPECIAL )		/* main->sound buffer status (1=empty) */
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START	/* 2a0002 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SPECIAL )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* 2a000e */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( dcheese )

	/* basic machine hardware */
	MDRV_CPU_ADD(M68000, MAIN_OSC)
	MDRV_CPU_PROGRAM_MAP(main_cpu_map,0)
	MDRV_CPU_VBLANK_INT("main", dcheese_vblank)

	MDRV_CPU_ADD(M6809, SOUND_OSC/16)
	MDRV_CPU_PROGRAM_MAP(sound_cpu_map,0)
	MDRV_CPU_PERIODIC_INT(irq1_line_hold, 480)	/* accurate for fredmem */

	MDRV_MACHINE_START(dcheese)

	MDRV_NVRAM_HANDLER(93C46)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(360, 262)	/* guess, need to see what the games write to the vid registers */
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 239)

	MDRV_PALETTE_LENGTH(65534)

	MDRV_PALETTE_INIT(dcheese)
	MDRV_VIDEO_START(dcheese)
	MDRV_VIDEO_UPDATE(dcheese)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(BSMT2000, SOUND_OSC)
	MDRV_SOUND_CONFIG(bsmt2000_interface_region_1)
	MDRV_SOUND_ROUTE(0, "left", 1.2)
	MDRV_SOUND_ROUTE(1, "right", 1.2)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( fredmem )
	MDRV_IMPORT_FROM(dcheese)
	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_VISIBLE_AREA(0, 359, 0, 239)
MACHINE_DRIVER_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

/*
    Double Cheese (c) 1993 HAR

    CPU: 68000
    Sound: 6809, BSMt2000 D15505N
    RAM: 84256 (x2), 5116
    Other: TRW9312HH (x2), LSI L1A6017 (MAX1 EXIT)

    Notes: PCB labeled "Exit Entertainment MADMAX version 5". Title screen reports
    (c)1993 Midway Manufacturing. ROM labels (c) 1993 HAR
*/
ROM_START( dcheese )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68k */
	ROM_LOAD16_BYTE( "dchez.104", 0x00000, 0x20000, CRC(5b6233d8) SHA1(7fdb606b5780dd8f45db07d3ee50e14a27f39533) )
	ROM_LOAD16_BYTE( "dchez.103", 0x00001, 0x20000, CRC(599c73ff) SHA1(f33e617ab7e9489c52b2434cfc61a5e1696e9400) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* M6809 */
	ROM_LOAD( "dchez.102", 0x8000, 0x8000, CRC(5d110061) SHA1(10d852a408a75979b8e8843afc7b39737ca2c6c8) )

	ROM_REGION( 0x100000, REGION_GFX1, 0 )
	ROM_LOAD( "dchez.123", 0x00000, 0x40000, CRC(2293dd9a) SHA1(3f0550c2a6f59a233c5b1010cecdb19404170dc0) )
	ROM_LOAD( "dchez.127", 0x40000, 0x40000, CRC(372f9d67) SHA1(74f73f0344bfb890b5e457fcde3d82c9106e7edd) )
	ROM_LOAD( "dchez.125", 0x80000, 0x40000, CRC(ddf28bab) SHA1(0f3bc86d0db7afebf8c6094b8337e5f343a82f29) )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 )
	ROM_LOAD( "dchez.ar0", 0x000000, 0x40000, CRC(6a9e2b12) SHA1(f7cb4d6b4a459682a68f734b2b2e27e3639b9ed5) )
	ROM_RELOAD(            0x040000, 0x40000 )
	ROM_RELOAD(            0x080000, 0x40000 )
	ROM_RELOAD(            0x0c0000, 0x40000 )
	ROM_LOAD( "dchez.ar1", 0x100000, 0x40000, CRC(5f3a5f41) SHA1(30e0c7b2ab43a3224432204a9388d509a6a06a11) )
	ROM_RELOAD(            0x140000, 0x40000 )
	ROM_RELOAD(            0x180000, 0x40000 )
	ROM_RELOAD(            0x1c0000, 0x40000 )
	ROM_LOAD( "dchez.ar2", 0x200000, 0x20000, CRC(d79b0d41) SHA1(cc84ddf6635097ba0aad2f1838ad0606c5bb8166) )
	ROM_RELOAD(            0x220000, 0x20000 )
	ROM_RELOAD(            0x240000, 0x20000 )
	ROM_RELOAD(            0x260000, 0x20000 )
	ROM_RELOAD(            0x280000, 0x20000 )
	ROM_RELOAD(            0x2a0000, 0x20000 )
	ROM_RELOAD(            0x2c0000, 0x20000 )
	ROM_RELOAD(            0x2e0000, 0x20000 )
	ROM_LOAD( "dchez.ar3", 0x300000, 0x20000, CRC(2056c1fd) SHA1(4c44930fb87ea6ad71326cc29313f3b817919d08) )
	ROM_RELOAD(            0x320000, 0x20000 )
	ROM_RELOAD(            0x340000, 0x20000 )
	ROM_RELOAD(            0x360000, 0x20000 )
	ROM_RELOAD(            0x380000, 0x20000 )
	ROM_RELOAD(            0x3a0000, 0x20000 )
	ROM_RELOAD(            0x3c0000, 0x20000 )
	ROM_RELOAD(            0x3e0000, 0x20000 )

	ROM_REGION16_LE( 0x20000, REGION_USER1, 0 )
	ROM_LOAD16_BYTE( "dchez.144", 0x00000, 0x10000, CRC(52c96252) SHA1(46de465c25e4602aa360336315b3c8e1a9a0b5f3) )
	ROM_LOAD16_BYTE( "dchez.145", 0x00001, 0x10000, CRC(a11b92d0) SHA1(265f93cb3657910aabca21ed8afbb55bdc86a964) )
ROM_END


ROM_START( lottof2 )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68k */
	ROM_LOAD16_BYTE( "u104.r20", 0x00000, 0x20000, CRC(0dfa710e) SHA1(b28676caf2074822e87bd213d76a892bcce07c1a) )
	ROM_LOAD16_BYTE( "u103.r20", 0x00001, 0x20000, CRC(1bcd7c77) SHA1(891f066cbcf558e7a725154758cf5a7a58a4400a) )

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* M6809 */
	ROM_LOAD( "u102.r10", 0x8000, 0x8000, CRC(fcb34c81) SHA1(f80cef85d0f4218c88c01b238f10eff2c6241d33) )

	ROM_REGION( 0x100000, REGION_GFX1, 0 )
	ROM_LOAD( "u123.r10", 0x00000, 0x40000, CRC(dbcdb5aa) SHA1(7473c5e0fc1a40a39e148277b4094fe1338d988c) )
	ROM_LOAD( "u127.r10", 0x40000, 0x40000, CRC(029ffed9) SHA1(63ba56277745ebea7c2c2b3738790cd2f4ddbe00) )
	ROM_LOAD( "u125.r10", 0x80000, 0x40000, CRC(c70cf1c6) SHA1(eb5f0c5f7485d92ce569ad915b9f5c3c48338172) )
	ROM_LOAD( "u129.r10", 0xc0000, 0x40000, CRC(e9c9e4b0) SHA1(02a3bc279e2489fd53f9a08df5f1023f75fff4d1) )

	ROM_REGION( 0x400000, REGION_SOUND1, 0 )
	ROM_LOAD( "arom0.r10", 0x000000, 0x40000, CRC(05e7581b) SHA1(e12be200abfbef269fc085d6c5efea106487e05f) )
	ROM_RELOAD(            0x040000, 0x40000 )
	ROM_RELOAD(            0x080000, 0x40000 )
	ROM_RELOAD(            0x0c0000, 0x40000 )
	ROM_LOAD( "arom1.r10", 0x100000, 0x20000, CRC(6c4ebbfd) SHA1(2b396d96ce8903e5e8d455ce019422b744f3c4d5) )
	ROM_RELOAD(            0x120000, 0x20000 )
	ROM_RELOAD(            0x140000, 0x20000 )
	ROM_RELOAD(            0x160000, 0x20000 )
	ROM_RELOAD(            0x180000, 0x20000 )
	ROM_RELOAD(            0x1a0000, 0x20000 )
	ROM_RELOAD(            0x1c0000, 0x20000 )
	ROM_RELOAD(            0x1e0000, 0x20000 )
	ROM_LOAD( "arom2.r10", 0x200000, 0x20000, CRC(fbe9fbbb) SHA1(457fc3c0d33cf430e5969f4fa11317f1f930351b) )
	ROM_RELOAD(            0x220000, 0x20000 )
	ROM_RELOAD(            0x240000, 0x20000 )
	ROM_RELOAD(            0x260000, 0x20000 )
	ROM_RELOAD(            0x280000, 0x20000 )
	ROM_RELOAD(            0x2a0000, 0x20000 )
	ROM_RELOAD(            0x2c0000, 0x20000 )
	ROM_RELOAD(            0x2e0000, 0x20000 )
	ROM_LOAD( "arom3.r10", 0x300000, 0x20000, CRC(ffb6e463) SHA1(1349455d2ce8eb141bc0fa5219f5e7c52ee969dc) )
	ROM_RELOAD(            0x320000, 0x20000 )
	ROM_RELOAD(            0x340000, 0x20000 )
	ROM_RELOAD(            0x360000, 0x20000 )
	ROM_RELOAD(            0x380000, 0x20000 )
	ROM_RELOAD(            0x3a0000, 0x20000 )
	ROM_RELOAD(            0x3c0000, 0x20000 )
	ROM_RELOAD(            0x3e0000, 0x20000 )

	ROM_REGION16_LE( 0x20000, REGION_USER1, 0 )
	ROM_LOAD16_BYTE( "u144.r10", 0x00000, 0x10000, CRC(3b9d5d9e) SHA1(b3fbfeb41c62c689a825dfe9487917a927a71f58) )
	ROM_LOAD16_BYTE( "u145.r10", 0x00001, 0x10000, CRC(e5a022a4) SHA1(567a37d24b36ca01a2ac3c40a0392cf97b1eb948) )
ROM_END


ROM_START( fredmem )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68k */
	ROM_LOAD16_BYTE( "prog0.104", 0x00000, 0x20000, CRC(9e90ebc3) SHA1(ef86e5070ec64772b8e8b9b30910b88bbd46285b) ) /* Program 0 - V2.0 at U104 */
	ROM_LOAD16_BYTE( "prog1.103", 0x00001, 0x20000, CRC(79cadede) SHA1(bfc04edf6dc3beb942ffba442fe4203d1e1a3c0e) ) /* Program 1 - V2.0 at U103 */

	ROM_REGION( 0x10000, REGION_CPU2, 0 ) /* M6809 */
	ROM_LOAD( "prog.102", 0x00000, 0x10000, CRC(b1526a1a) SHA1(456c44a0a908b3cd054b7c6741d7a1033c9b12fb) ) /* Sound Program 6809 code at U102 */

	ROM_REGION( 0x200000, REGION_GFX1, 0 )
	ROM_LOAD( "art-rom.123", 0x000000, 0x80000, CRC(48133505) SHA1(60f69b053e67256928db57e0a5335bbd5a72ddfc) ) /* Graphics / Art at U123 */
	ROM_LOAD( "art-rom.125", 0x080000, 0x80000, CRC(8181e154) SHA1(4d16b84ad52d8e3d3bcad3fdf5f8da23df198d46) ) /* Graphics / Art at U125 */
	ROM_LOAD( "art-rom.127", 0x100000, 0x80000, CRC(93095f3b) SHA1(de746829e04bf153024e94e6ef0ceffb1eae2b14) ) /* Graphics / Art at U127 */
	ROM_LOAD( "art-rom.129", 0x180000, 0x80000, CRC(d5715a02) SHA1(b7d9d29f2fc5d74adff1fefce312e6472c0f7565) ) /* Graphics / Art at U129 */

	ROM_REGION( 0x400000, REGION_SOUND1, 0 )
	ROM_LOAD( "arom0", 0x000000, 0x80000, CRC(3b85ea34) SHA1(0a68e7df20a2c36e230c7935415dd5068c338669) )
	ROM_RELOAD(        0x080000, 0x80000 )
	ROM_LOAD( "arom1", 0x100000, 0x80000, CRC(405df3d4) SHA1(190b928789a879408beadd1647136bd85b018c63) )
	ROM_RELOAD(        0x180000, 0x80000 )
	ROM_LOAD( "arom2", 0x200000, 0x80000, CRC(48ecd5c9) SHA1(6aad371db7b658454c5feed548ffd19b81a8fcf4) )
	ROM_RELOAD(        0x280000, 0x80000 )
	ROM_LOAD( "arom3", 0x300000, 0x80000, CRC(411900b0) SHA1(ddc5b387c89baab0fd5c654f3768c6e27972c06a) )
	ROM_RELOAD(        0x380000, 0x80000 )

	ROM_REGION16_LE( 0x20000, REGION_USER1, 0 )
	ROM_LOAD16_BYTE( "0.144", 0x00000, 0x10000, CRC(793c4bda) SHA1(5a8a2981b48922f4d9e617a9bf9ef6a47ab702b7) ) /* Pallette - 0 at U144 */
	ROM_LOAD16_BYTE( "1.145", 0x00001, 0x10000, CRC(fe2c3521) SHA1(896e53427c7831620ca565be9c0b76aabc36b9f4) ) /* Pallette - 1 at U145 */

	ROM_REGION( 0x100, REGION_USER2, 0 )
	ROM_LOAD16_BYTE( "93c46.u158", 0x00000, 0x0080, CRC(a40a7b87) SHA1(3632b7538b3bf41ee0cbe7541a0f5951f70b4a9b) ) /* EEPROM data at U158 */
ROM_END



/*************************************
 *
 *  Driver configuration
 *
 *************************************/

static DRIVER_INIT( dcheese )
{
	ticket_dispenser_init(200, TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_LOW);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, dcheese, 0, dcheese, dcheese, dcheese, ROT90, "HAR", "Double Cheese", GAME_SUPPORTS_SAVE )
GAME( 1993, lottof2, 0, dcheese, lottof2, dcheese, ROT0,  "HAR", "Lotto Fun 2", GAME_SUPPORTS_SAVE )
GAME( 1994, fredmem, 0, fredmem, fredmem, dcheese, ROT0,  "Coastal Amusements", "Fred Flintstones' Memory Match", GAME_SUPPORTS_SAVE )
