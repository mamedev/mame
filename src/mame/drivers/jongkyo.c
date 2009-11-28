/* Jongkyo */
#include "driver.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/segacrpt.h"

#define JONGKYO_CLOCK 18432000

/*

Jongkyo
(c)1985 Kiwako

834-5558 JONGKYO
C2-00173

CPU: SEGA Custom 315-5084 (Z80)
Sound: AY-3-8910
OSC: 18.432MHz

ROMs:
EPR-6258 (2764)
EPR-6259 (2764)
EPR-6260 (2764)
EPR-6261 (2764)
EPR-6262 (2732)

PR-6263.6J (82S123N)
PR-6264.0H (82S123N)
PR-6265.0M (82S129N)
PR-6266.0B (82S129N)

*/


//static UINT8 *videoram;

static int rom_bank;
static UINT8* videoram2;

static VIDEO_START(jongkyo)
{

}

static VIDEO_UPDATE(jongkyo)
{
	int y;

	for (y = 0; y < 256; ++y)
	{
		int x;

		for (x = 0; x < 256; x += 4)
		{
			int b;
			UINT8 data1;
			UINT8 data2;
			UINT8 data3;

	//      data3 = videoram2[x/4 + y*64]; // wrong

	// good mahjong tiles
	      data3 = 0x0f; // we're missing 2 bits.. there must be another piece of video ram somewhere or we can't use all the colours (6bpp).. banked somehow?
	// good girl tiles
	//  data3 = 0x00; // we're missing 2 bits.. there must be another piece of video ram somewhere or we can't use all the colours (6bpp).. banked somehow?



			data1 = screen->machine->generic.videoram.u8[0x4000 + x/4 + y*64];
			data2 = screen->machine->generic.videoram.u8[x/4 + y*64];

			for (b = 0; b < 4; ++b)
			{
				*BITMAP_ADDR16(bitmap, 255-y, 255-(x+b)) = ((data2 & 0x01)) +
					                                       ((data2 & 0x10) >> 3) +
                                                           ((data1 & 0x01) << 2) +
					                                       ((data1 & 0x10) >> 1) +
                                                           ((data3 & 0x01) << 4) +
					                                       ((data3 & 0x10) << 1);
				data1 >>= 1;
				data2 >>= 1;
				data3 >>= 1;
			}
		}
	}

	return 0;
}


static WRITE8_HANDLER( bank_select_w )
{

	int mask = 1 << (offset >> 1);

	rom_bank &= ~mask;

	if (offset & 1)
		rom_bank |= mask;

	memory_set_bank(space->machine, 1, rom_bank);
}

static UINT8 mux_data;

static WRITE8_HANDLER( mux_w )
{
	mux_data = ~data;
//  printf("%02x\n",mux_data);
}

static WRITE8_HANDLER( jongkyo_coin_counter_w )
{
	/* bit 1 = coin counter */
	coin_counter_w(space->machine, 0,data & 2);

	/* bit 2 always set? */
}

static READ8_DEVICE_HANDLER( input_1p_r )
{
	static UINT8 cr_clear;

	cr_clear = input_port_read(device->machine, "CR_CLEAR");

	switch(mux_data)
	{
		case 0x01: return input_port_read(device->machine, "PL1_1") | cr_clear;
		case 0x02: return input_port_read(device->machine, "PL1_2") | cr_clear;
		case 0x04: return input_port_read(device->machine, "PL1_3") | cr_clear;
		case 0x08: return input_port_read(device->machine, "PL1_4") | cr_clear;
		case 0x10: return input_port_read(device->machine, "PL1_5") | cr_clear;
		case 0x20: return input_port_read(device->machine, "PL1_6") | cr_clear;
	}
//  printf("%04x\n",mux_data);

	return (input_port_read(device->machine, "PL1_1") & input_port_read(device->machine, "PL1_2") & input_port_read(device->machine, "PL1_3") &
	       input_port_read(device->machine, "PL1_4") & input_port_read(device->machine, "PL1_5") & input_port_read(device->machine, "PL1_6")) | cr_clear;//input_port_read(device->machine, "PL1_0") && ;
}

static READ8_DEVICE_HANDLER( input_2p_r )
{
	static UINT8 coin_port;

	coin_port = input_port_read(device->machine, "COINS");

	switch(mux_data)
	{
		case 0x01: return input_port_read(device->machine, "PL2_1") | coin_port;
		case 0x02: return input_port_read(device->machine, "PL2_2") | coin_port;
		case 0x04: return input_port_read(device->machine, "PL2_3") | coin_port;
		case 0x08: return input_port_read(device->machine, "PL2_4") | coin_port;
		case 0x10: return input_port_read(device->machine, "PL2_5") | coin_port;
		case 0x20: return input_port_read(device->machine, "PL2_6") | coin_port;
	}
//  printf("%04x\n",mux_data);

	return (input_port_read(device->machine, "PL2_1") & input_port_read(device->machine, "PL2_2") & input_port_read(device->machine, "PL2_3") &
	       input_port_read(device->machine, "PL2_4") & input_port_read(device->machine, "PL2_5") & input_port_read(device->machine, "PL2_6")) | coin_port;//input_port_read(device->machine, "PL1_0") && ;

}
static WRITE8_HANDLER( videoram2_w )
{
	videoram2[offset] = data;
}

static WRITE8_HANDLER( unknown_w )
{
	switch (offset)
	{
		case 0: // different values
			break;
		case 1: // set to 0 at the boot and set to 1 continuesly
			break;
		case 2: // only set to 0 at the boot
			break;
		case 3: // not used
			break;
		case 4: // set to 1 before the girl drawing (probably is the palette selector, not sure how to restore the old palette)
			break;
		case 5: // only set to 0 at the boot
			break;
		case 6: // different values
			break;
		case 7: // 07 and 08 are like a counter: every write in 08 is a incremented value (from 00 to ff)
			break;
		case 8: // when this value is 0xff the next value is 00 and port 07 is incremented (from 00 to ff)
			break;
		case 9: // different values
			break;
	}
}

static ADDRESS_MAP_START( jongkyo_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_READ(SMH_ROM) AM_WRITE(videoram2_w) // wrong, this doesn't seem to be video ram on write..
	AM_RANGE(0x4000, 0x6bff) AM_READ(SMH_ROM) // fixed rom
	AM_RANGE(0x6c00, 0x6fff) AM_READ(SMH_BANK(1))	// banked (8 banks)
	AM_RANGE(0x7000, 0x77ff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_RAM AM_BASE_GENERIC(videoram)
ADDRESS_MAP_END


static ADDRESS_MAP_START( jongkyo_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	// R 01 keyboard
	AM_RANGE(0x01, 0x01) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("aysnd", ay8910_data_address_w)

	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW") AM_WRITE(jongkyo_coin_counter_w)
	AM_RANGE(0x11, 0x11) AM_READ_PORT("IN0") AM_WRITE(mux_w)
	// W 11 select keyboard row (fe fd fb f7)
	AM_RANGE(0x40, 0x45) AM_WRITE(bank_select_w)
	AM_RANGE(0x46, 0x4f) AM_WRITE(unknown_w)
ADDRESS_MAP_END

/*
-------------------------------------------------------------
Jongkyo ?1985 Kiwako
DIPSW         |      |1    2    3    4   |5   |6   |7   |8
-------------------------------------------------------------
Payout rate   |50%   |on   on   on   on  |    |    |    |
              |53%   |off  on   on   on  |    |    |    |
              |56%   |on   off  on   on  |    |    |    |
              |59%   |off  off  on   on  |    |    |    |
              |62%   |on   on   off  on  |    |    |    |
              |65%   |off  on   off  on  |    |    |    |
              |68%   |on   off  off  on  |    |    |    |
              |71%   |off  off  off  on  |    |    |    |
              |75%   |on   on   on   off |    |    |    |
              |78%   |off  on   on   off |    |    |    |
              |81%   |on   off  on   off |    |    |    |
              |84%   |off  off  on   off |    |    |    |
              |87%   |on   on   off  off |    |    |    |
              |90%   |off  on   off  off |    |    |    |
              |93%   |on   off  off  off |    |    |    |
              |96%   |off  off  off  off |    |    |    |
-------------------------------------------------------------
Start chance  |Yes   |                   |on  |    |    |
              |No    |                   |off |    |    |
-------------------------------------------------------------
Bet up        |Yes   |                   |    |on  |    |
              |No    |                   |    |off |    |
-------------------------------------------------------------
Last chance   |5     |                   |    |    |on  |
              |1     |                   |    |    |off |
-------------------------------------------------------------
Bonus credit  |50    |                   |    |    |    |on
              |10    |                   |    |    |    |off
-------------------------------------------------------------
*/


static INPUT_PORTS_START( jongkyo )
	PORT_START("CR_CLEAR")
	PORT_DIPNAME( 0x40, 0x40, "Credit Clear-1" )//button
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Credit Clear-2" )//button
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 ) //player-1 side
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 ) //player-2 side

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_3)//rate button
	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button
	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button
	PORT_START("PL1_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Option 4")
	PORT_START("PL1_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1P Pass") //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_CODE(KEYCODE_4) PORT_PLAYER(2)//rate button
	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) //another D button
	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED ) //another opt 1 button
	PORT_START("PL2_5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 1") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 2") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 3") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Option 4") PORT_PLAYER(2)
	PORT_START("PL2_6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2P Pass") PORT_PLAYER(2) //???
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x00, "Note" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, "Memory Reset" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x00, "Analizer" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_HIGH )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x0f, "Payout Rate" ) PORT_DIPLOCATION("SW:1,2,3,4")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x0b, "84$" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPNAME( 0x10, 0x10, "Start Chance" ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Bet Up" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Last Chance" ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, "1" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Bonus Credit" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x00, "50" )
INPUT_PORTS_END


static PALETTE_INIT(jongkyo)
{
	int i;
	UINT8* proms = memory_region(machine, "proms");
	for (i=0;i<0x40;i++)
	{
		int data = proms[i];

		int r = (data  >> 0) & 0x07;
		int g = (data  >> 3) & 0x07;
		int b = (data  >> 6) & 0x03;

		 palette_set_color_rgb(machine, i, r<<5, g<<5, b<<6 );

	}
}

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_HANDLER(input_1p_r),
	DEVCB_HANDLER(input_2p_r),
	DEVCB_NULL,
	DEVCB_NULL
};

static MACHINE_DRIVER_START( jongkyo )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80,JONGKYO_CLOCK/4)
	MDRV_CPU_PROGRAM_MAP(jongkyo_memmap)
	MDRV_CPU_IO_MAP(jongkyo_portmap)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(256, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 256-1, 8, 256-8-1)

	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(jongkyo)

	MDRV_VIDEO_START(jongkyo)
	MDRV_VIDEO_UPDATE(jongkyo)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd", AY8910, JONGKYO_CLOCK/8)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.33)
MACHINE_DRIVER_END


ROM_START( jongkyo )
	ROM_REGION( 0x9000, "maincpu", 0 )
	ROM_LOAD( "epr-6258", 0x00000, 0x02000, CRC(fb8b7bcc) SHA1(8ece7c2c82c237b4b51829d412b2109b96ccd0e7) )
	ROM_LOAD( "epr-6259", 0x02000, 0x02000, CRC(e46cde5d) SHA1(1cbe1677cfb3fa9f76ad90d5b1446ce9cefee6b7) )
	ROM_LOAD( "epr-6260", 0x04000, 0x02000, CRC(369a5365) SHA1(037a2971a59ab339595b333cbdfd4cbb104de2be) )
	ROM_LOAD( "epr-6262", 0x06000, 0x01000, CRC(ecf50f34) SHA1(ecfa1a9360d8fbcbed457d46e53bae77f6d78c1d) )
	ROM_LOAD( "epr-6261", 0x07000, 0x02000, CRC(9c475ae1) SHA1(b993c2636dafed9f80fa87e71921c3c85c039e45) )	// banked at 6c00-6fff

	ROM_REGION( 0x300, "proms", 0 )
	/* colours */
	ROM_LOAD( "pr-6263.6j", 0x00000, 0x00020, CRC(468134d9) SHA1(bb633929df17e448882ee80613fc1dfac3c35d7a) )
	ROM_LOAD( "pr-6264.0h", 0x00020, 0x00020, CRC(46014727) SHA1(eec451f292ee319fa6bfbbf223aaa12b231692c1) )

	/* unknown purpose */
	ROM_LOAD( "pr-6265.0m", 0x00100, 0x00100, CRC(f09d3c4c) SHA1(a9e752d75e7f3ebd05add4ccf2f9f15d8f9a8d15) )
	ROM_LOAD( "pr-6266.0b", 0x00200, 0x00100, CRC(86aeafd1) SHA1(c4e5c56ce5baf2be3962675ae333e28bd8108a00) )
ROM_END


static DRIVER_INIT( jongkyo )
{
	int i;
	UINT8 *rom = memory_region(machine, "maincpu");

	/* first of all, do a simple bitswap */
	for (i = 0x6000; i < 0x9000; ++i)
	{
		rom[i] = BITSWAP8(rom[i], 7,6,5,3,4,2,1,0);
	}

	/* then do the standard Sega decryption */
	jongkyo_decode(machine, "maincpu");

	videoram2 = auto_alloc_array(machine, UINT8, 0x4000);
	state_save_register_global_pointer(machine, videoram2, 0x4000);

}


GAME( 1985, jongkyo,  0,    jongkyo, jongkyo,  jongkyo, ROT0, "Kiwako", "Jongkyo", GAME_WRONG_COLORS )
