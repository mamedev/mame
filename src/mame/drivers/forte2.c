/* Brazilian bootleg board from 1989. Forte II Games, Industria Brasileira.
MAME driver by Mariusz Wojcieszek & hap, based on information from Alexandre.

Hardware is based on MSX1, excluding i8255 PPI:
 64KB RAM, largely unused
 64KB EPROM (2764-15, contains hacked BIOS and game ROM)
 Z80 @ 3.58MHz
 GI AY-3-8910
 TI TMS9928
 (no dipswitches)

Games:
Pesadelo (means Nightmare in Portuguese), 1989 bootleg of Knightmare (Majou
Densetsu in Japan) (C) 1986 Konami, originally released exclusively on MSX.
This arcade conversion has been made a bit harder, eg. bonus power-ups deplete
three times quicker, and the game starts at a later, more difficult level.
A rough translation of the text after inserting a coin: Coins won't accumulate,
insert a new coin after the game is over if you want another play.

According to Alexandre, there are more games for this board, but not
found/dumped yet. */

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"

static UINT8 forte2_input_mask;

static ADDRESS_MAP_START( program_mem, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_mem, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x98, 0x98) AM_READWRITE( TMS9928A_vram_r, TMS9928A_vram_w )
	AM_RANGE(0x99, 0x99) AM_READWRITE( TMS9928A_register_r, TMS9928A_register_w )
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREAD("aysnd", ay8910_r)

/* Ports a8-ab are originally for communicating with the i8255 PPI on MSX.
Since this arcade board doesn't have one, those ports should be unmapped. */
//  AM_RANGE(0xa8, 0xa8) AM_RAM
//  AM_RANGE(0xa9, 0xab) AM_NOP
ADDRESS_MAP_END

static INPUT_PORTS_START( pesadelo )
	PORT_START("IN0")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_COIN1)
INPUT_PORTS_END


static READ8_DEVICE_HANDLER(forte2_ay8910_read_input)
{
	return input_port_read(device->machine, "IN0") | (forte2_input_mask&0x3f);
}

static WRITE8_DEVICE_HANDLER( forte2_ay8910_set_input_mask )
{
	/* PSG reg 15, writes 0 at coin insert, 0xff at boot and game over */
	forte2_input_mask = data;
}

static const ay8910_interface forte2_ay8910_interface =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(forte2_ay8910_read_input),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(forte2_ay8910_set_input_mask)
};


static void vdp_interrupt(running_machine *machine, int i)
{
	cputag_set_input_line(machine, "maincpu", 0, (i ? HOLD_LINE : CLEAR_LINE));
}

static const TMS9928a_interface tms9928a_interface =
{
	TMS99x8A,
	0x4000,
	0, 0,
	vdp_interrupt
};

static MACHINE_START( forte2 )
{
	TMS9928A_configure(&tms9928a_interface);

	forte2_input_mask = 0xff;

	/* register for save states */
	state_save_register_global(machine, forte2_input_mask);
}

static MACHINE_RESET( forte2 )
{
	TMS9928A_reset();
}

static INTERRUPT_GEN( pesadelo_interrupt )
{
	TMS9928A_interrupt(device->machine);
}


static MACHINE_DRIVER_START( pesadelo )

	MDRV_CPU_ADD("maincpu", Z80, 3579545)		  /* 3.579545 Mhz */
	MDRV_CPU_PROGRAM_MAP(program_mem)
	MDRV_CPU_IO_MAP(io_mem)
	MDRV_CPU_VBLANK_INT("screen",pesadelo_interrupt)

	MDRV_MACHINE_START( forte2 )
	MDRV_MACHINE_RESET( forte2 )

	/* video hardware */
	MDRV_IMPORT_FROM(tms9928a)
	MDRV_SCREEN_MODIFY("screen")
	MDRV_SCREEN_REFRESH_RATE((float)XTAL_10_738635MHz/2/342/262)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(4458)) /* 70 lines */

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, (float)XTAL_3_579545MHz/2)
	MDRV_SOUND_CONFIG(forte2_ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

static DRIVER_INIT(pesadelo)
{
	int i;
	UINT8 *mem = memory_region(machine, "maincpu");
	int memsize = memory_region_length(machine, "maincpu");
	UINT8 *buf;

	// data swap
	for ( i = 0; i < memsize; i++ )
	{
		mem[i] = BITSWAP8(mem[i],3,5,6,7,0,4,2,1);
	}

	// address line swap
	buf = auto_alloc_array(machine, UINT8, memsize);
	memcpy(buf, mem, memsize);
	for ( i = 0; i < memsize; i++ )
	{
		mem[BITSWAP16(i,11,9,8,13,14,15,12,7,6,5,4,3,2,1,0,10)] = buf[i];
	}
	auto_free(machine, buf);

}

ROM_START( pesadelo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr2764.15", 0x00000, 0x10000, CRC(1ae2f724) SHA1(12880dd7ad82acf04861843fb9d4f0f926d18f6b) )
ROM_END

GAME( 1989, pesadelo, 0, pesadelo, pesadelo, pesadelo, ROT0, "bootleg / Konami", "Pesadelo (bootleg of Knightmare on MSX)", GAME_SUPPORTS_SAVE )
