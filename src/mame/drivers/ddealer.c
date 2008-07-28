/*
Double Dealer (c)NMK 1991

Based on jalmah.c and nmk.c drivers
Skeleton driver by Tomasz Slanina

--

pcb marked  GD91071

68000P10
YM2203C
91071-3 (????) marked as "3" on the pcb
NMK-110 8131 ( Mitsubishi M50747 MCU ?)
NMK 901
NMK 902
NMK 903 x2
82S135N ("5")
82S129N ("6")
xtals 16.000 MHz and  6.000 MHz
DSW x2

--

Few words about protection:

- Work RAM at $fe000 - $fffff is shared with MCU . Maybe whole $f0000-$fffff is shared ...
- After boot, game writes random-looking data to work RAM:

 00052C: 33FC 1234 000F E086        move.w  #$1234, $fe086.l
 000534: 33FC 5678 000F E164        move.w  #$5678, $fe164.l
 00053C: 33FC 9CA3 000F E62E        move.w  #$9ca3, $fe62e.l
 000544: 33FC ABA2 000F E734        move.w  #$aba2, $fe734.l
 00054C: 33FC B891 000F E828        move.w  #$b891, $fe828.l
 000554: 33FC C760 000F E950        move.w  #$c760, $fe950.l
 00055C: 33FC D45F 000F EA7C        move.w  #$d45f, $fea7c.l
 000564: 33FC E32E 000F ED4A        move.w  #$e32e, $fed4a.l

  Some (or maybe all ?) of above enables random generator at $fe010 - $fe017

- There's also MCU response (write/read/test) test just after these writes.
  (probably data used in the check depends on above writes). It's similar to
  jalmah.c tests, but num of responses is different, and  shared ram is
  used to communicate with MCU

- After last check (or maybe durning tests ... no idea)
  MCU writes $4ef900000604 (jmp $604) to $fe000 and game jumps to this address.

- code at $604  writes $20.w to $fe018 and $1.w to $fe01e.
  As result shared ram $fe000 - $fe007 is cleared.

  Also many, many other reads/writes  from/to shared mem.
  Few checks every interrupt:

    interrupt, lvl1

    000796: 007C 0700                  ori     #$700, SR
    00079A: 48E7 FFFE                  movem.l D0-D7/A0-A6, -(A7)
    00079E: 33FC 0001 000F E006        move.w  #$1, $fe006.l ; shared ram W (watchdog ?)
    0007A6: 4A79 000F E000             tst.w   $fe000.l ; shared ram R
    0007AC: 6600 0012                  bne     $7c0
    0007B0: 4A79 000F 02FE             tst.w   $f02fe.l
    0007B6: 6600 0008                  bne     $7c0
    0007BA: 4279 000F C880             clr.w   $fc880.l
+-0007C0: 6100 0236                  bsr     $9f8
| 0007C4: 4EB9 0003 0056             jsr     $30056.l
|   0007CA: 33FC 00FF 000F C880        move.w  #$ff, $fc880.l
|   0007D2: 007C 2000                  ori     #$2000, SR
|   0007D6: 4CDF 7FFF                  movem.l (A7)+, D0-D7/A0-A6
|   0007DA: 4E73                       rte
|
|
+->0009F8: 4A79 000F 02C0             tst.w   $f02c0.l
     0009FE: 6700 0072                  beq     $a72
     000A02: 4A79 000F 02F6             tst.w   $f02f6.l
     000A08: 6600 0068                  bne     $a72
     000A0C: 3439 000F E002             move.w  $fe002.l, D2 ; shared ram R
     000A12: 3602                       move.w  D2, D3
     000A14: 0242 00FF                  andi.w  #$ff, D2
     000A18: 0243 FF00                  andi.w  #$ff00, D3
     000A1C: E04B                       lsr.w   #8, D3
     000A1E: 3039 000F E000             move.w  $fe000.l, D0  ; shared ram R
     000A24: 3239 000F 0010             move.w  $f0010.l, D1
     000A2A: B041                       cmp.w   D1, D0
     000A2C: 6200 002A                  bhi     $a58
     000A30: 6600 002E                  bne     $a60
     000A34: 3039 000F 0012             move.w  $f0012.l, D0
     000A3A: B440                       cmp.w   D0, D2
     000A3C: 6200 001A                  bhi     $a58
     000A40: 6600 001E                  bne     $a60
     000A44: 3039 000F 0014             move.w  $f0014.l, D0
     000A4A: B640                       cmp.w   D0, D3
     000A4C: 6200 000A                  bhi     $a58
     000A50: 6600 000E                  bne     $a60
     000A54: 6000 001C                  bra     $a72
     000A58: 33FC 0007 000F C880        move.w  #$7, $fc880.l ; used later in the code...
     000A60: 33C0 000F 0010             move.w  D0, $f0010.l ;update mem, used in next test
     000A66: 33C2 000F 0012             move.w  D2, $f0012.l
     000A6C: 33C3 000F 0014             move.w  D3, $f0014.l
     000A72: 4E75                       rts

*/


#include "driver.h"

static UINT16 *shared_ram;

static int prot=0;
static int respcount;


static VIDEO_START( ddealer )
{
}

static void ddealer_protection(running_machine *machine)
{

	if(prot==0)
	{
		shared_ram[0xe000/2]=0x4ef9;
		shared_ram[0xe002/2]=0x0000;
		shared_ram[0xe004/2]=0x0604;
	}

	shared_ram[0xe008/2]=0x0001;
	shared_ram[0xe00a/2]=0x0001;
	shared_ram[0xe00c/2]=0x0001;
	shared_ram[0xe00e/2]=0x0001;

	shared_ram[0xe010/2]=mame_rand(machine) & 0xffff;
	shared_ram[0xe012/2]=mame_rand(machine) & 0xffff;
	shared_ram[0xe014/2]=mame_rand(machine) & 0xffff;
	shared_ram[0xe016/2]=mame_rand(machine) & 0xffff;

}

static VIDEO_UPDATE( ddealer )
{
	ddealer_protection(screen->machine);
	return 0;
}

static ADDRESS_MAP_START( ddealer, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_RAM
	AM_RANGE(0x080006, 0x080007) AM_RAM // read: low byte = dsw 1
	AM_RANGE(0x084000, 0x084003) AM_RAM // ym ?
	AM_RANGE(0x088000, 0x0887ff) AM_RAM // palette ram
	AM_RANGE(0x08c000, 0x08cfff) AM_RAM // palette ram
	AM_RANGE(0x090000, 0x093fff) AM_RAM // bg tilemap
	AM_RANGE(0x09c000, 0x09ffff) AM_RAM // fg tilemap
	AM_RANGE(0x0f0000, 0x0fffff) AM_RAM AM_BASE(&shared_ram)// at least fe000-ffff shared with mcu
ADDRESS_MAP_END

static INPUT_PORTS_START( ddealer )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( jalmah )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END

static MACHINE_RESET (ddealer)
{
	respcount = 0;
	prot = 0;
}

static INTERRUPT_GEN( ddealer_interrupt )
{
	cpunum_set_input_line(machine, 0, 4, HOLD_LINE);


}

static MACHINE_DRIVER_START( ddealer )
	MDRV_CPU_ADD("main" , M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(ddealer,0)
	MDRV_CPU_VBLANK_INT("main", ddealer_interrupt)
	MDRV_CPU_PERIODIC_INT(irq1_line_hold, 112)

	MDRV_GFXDECODE(jalmah)


	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_PALETTE_LENGTH(0x400)
	MDRV_MACHINE_RESET(ddealer)

	MDRV_VIDEO_START(ddealer)
	MDRV_VIDEO_UPDATE(ddealer)

	MDRV_SPEAKER_STANDARD_MONO("mono")
MACHINE_DRIVER_END



static READ16_HANDLER( ddealer_mcu_r )
{
	static const int resp[] =
	{
		0x93, 0xc7, 0x00, 0x8000,
		0x2d, 0x6d, 0x00, 0x8000,
		0x99, 0xc7, 0x00, 0x8000,
		0x2a, 0x6a, 0x00, 0x8000,
		0x8e, 0xc7, 0x00, 0x8000,
	-1};

	int res;

	res = resp[respcount++];
	if (resp[respcount]<0)
	{
		 respcount = 0;
		 ddealer_protection(machine);

	}
	return res;
}

static WRITE16_HANDLER( ddealer_mcu_w )
{
	if(data==1)
	{
		prot=1;
		shared_ram[0xe000/2]=0;
		shared_ram[0xe002/2]=0;
		shared_ram[0xe004/2]=0;
		shared_ram[0xe006/2]=0;
	}
}

static DRIVER_INIT( ddealer )
{
	memory_install_read16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfe01c, 0xfe01d, 0, 0, ddealer_mcu_r );
	memory_install_write16_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0xfe01e, 0xfe01f, 0, 0, ddealer_mcu_w );
}

ROM_START( ddealer )
	ROM_REGION( 0x40000, "main", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "1.ic6", 0x00001, 0x20000, CRC(ce0dff50) SHA1(2d7a03f6b9609aea7511a4dc49560a901b0b9f19) )
	ROM_LOAD16_BYTE( "2.ic28", 0x00000, 0x20000, CRC(f00c346f) SHA1(bd73efb19d5f9efc88210d92a82a3f4595b41097) )

	ROM_REGION( 0x20000, "gfx1", 0 ) /* BG0 */
	ROM_LOAD( "4.ic65", 0x00000, 0x20000, CRC(4939ff1b) SHA1(af2f2feeef5520d775731a58cbfc8fcc913b7348) )

	ROM_REGION( 0x200, "user1", 0 ) /* Proms */
	ROM_LOAD( "5.ic67", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "6.ic86", 0x100, 0x100, NO_DUMP )
ROM_END

GAME( 1991, ddealer,  0, ddealer, ddealer, ddealer,  ROT0, "NMK", "Double Dealer", GAME_NOT_WORKING | GAME_NO_SOUND)

