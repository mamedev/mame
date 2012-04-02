/* Peno Cup?

no idea if this is the real title, I just see a large Peno Cup logo in the 2/3 roms

looks like some kind of ping pong / tennis game?

 ___________________________________________________
|        __     _________ __________   __________  |
|__      |_|   |UM62256D| |UM62256D|   |UM62256D|  |
  |    LN358N  |________| |________|   |________|  |
 _|            _________ _________     __________  |
|_  _1_______  |TI     | |TI     |     |UM62256D|  |
|_  |27C020 |  |52C1HXW| |52ALROW|     |________|  |
|_  |_______|  |       | |       |     __________  |
|__            |       | |       |     |UM62256D|  |
 _| __         |_______| |_______|     |________|  |
|_  |_| 74HC74                                     |
|_  OKI        74LS244   _3_______  _5_______      |
|_  M6295      74LS244   |27C040 |  |27C040 |      |
|_    57C55LK            |_______|  |_______|      |
|_             ________  _2_______  _4_______      |
|_    57C55LK  |GM76C28| |27C040 |  |27C040 |      |
|_             |_______| |_______|  |_______|      |
|_    74LS244  ________                            |
|_             |GM76C28| PAL16L8B PAL16L8B 74LS245 |
|_             |_______|                           |
|_                       74LS244  74LS373N 74LS373 |
|_    74LS244  PIC16C84                            |
|_                       74HC273N 74LS373N 74LS245 |
|_             74HC74B1              _____________ |
  |                            OSC   |NEC V30    | |
 _|   74LS244  74LS14N 74HC74  16MHz |D70116C-10 | |
|__________________________________________________|

The PCB is Spanish and manufacured by Gamart.


--- Need to work out how the program rom is banked
--- hw is similar to hotblock and twins


*/

#include "emu.h"
#include "cpu/nec/nec.h"


class ttchamp_state : public driver_device
{
public:
	ttchamp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_peno_vram;
	UINT16 m_paloff;
};


static VIDEO_START(ttchamp)
{
}

static SCREEN_UPDATE_IND16(ttchamp)
{
	ttchamp_state *state = screen.machine().driver_data<ttchamp_state>();
	int y,x,count;
//  int i;
	static const int xxx=320,yyy=204;

	bitmap.fill(get_black_pen(screen.machine()));

//  for (i=0;i<256;i++)
//  {
//      int dat,r,g,b;
//      dat=(hotblock_pal[i*2+1]<<8)|hotblock_pal[i*2];
//
//      b = (dat>>10)&0x1f;
//      g = (dat>>5)&0x1f;
//      r = (dat>>0)&0x1f;
//      palette_set_color_rgb(machine,i,pal5bit(r),pal5bit(g),pal5bit(b));
//  }

	count=0;
	for (y=0;y<yyy;y++)
	{
		for(x=0;x<xxx;x++)
		{
			/*if(hotblock_port0&0x40)*/bitmap.pix16(y, x) = ((UINT8 *)state->m_peno_vram)[BYTE_XOR_LE(count)]+0x300;
            count++;
        }
    }
    return 0;
}


static WRITE16_HANDLER( paloff_w )
{
	ttchamp_state *state = space->machine().driver_data<ttchamp_state>();
    COMBINE_DATA(&state->m_paloff);
}

#ifdef UNUSED_FUNCTION
static WRITE16_HANDLER( pcup_prgbank_w )
{
    int bank;
    UINT8 *ROM1 = space->machine().region("user1")->base();

    if (ACCESSING_BITS_0_7)
    {
        bank = (data>>4) &0x07;
        memory_set_bankptr(space->machine(), "bank2",&ROM1[0x80000*(bank)]);
    }
}
#endif

static WRITE16_HANDLER( paldat_w )
{
	ttchamp_state *state = space->machine().driver_data<ttchamp_state>();
    palette_set_color_rgb(space->machine(),state->m_paloff & 0x7fff,pal5bit(data>>0),pal5bit(data>>5),pal5bit(data>>10));
}

static READ16_HANDLER( peno_rand )
{
    return 0xffff;// space->machine().rand();
}

#ifdef UNUSED_FUNCTION
static READ16_HANDLER( peno_rand2 )
{
    return space->machine().rand();
}
#endif

static ADDRESS_MAP_START( ttchamp_map, AS_PROGRAM, 16, ttchamp_state )
    AM_RANGE(0x00000, 0x0ffff) AM_RAM
    AM_RANGE(0x10000, 0x1ffff) AM_RAM AM_BASE(m_peno_vram)
    AM_RANGE(0x20000, 0x7ffff) AM_ROMBANK("bank1") // ?
    AM_RANGE(0x80000, 0xfffff) AM_ROMBANK("bank2") // ?
ADDRESS_MAP_END

static ADDRESS_MAP_START( ttchamp_io, AS_IO, 16, ttchamp_state )
    AM_RANGE(0x0000, 0x0001) AM_WRITENOP

    AM_RANGE(0x0002, 0x0003) AM_READ_PORT("SYSTEM")
    AM_RANGE(0x0004, 0x0005) AM_READ_PORT("P1_P2")

//  AM_RANGE(0x0018, 0x0019) AM_READ_LEGACY(peno_rand2)
//  AM_RANGE(0x001e, 0x001f) AM_READ_LEGACY(peno_rand2)

    AM_RANGE(0x0008, 0x0009) AM_WRITE_LEGACY(paldat_w)
    AM_RANGE(0x000a, 0x000b) AM_WRITE_LEGACY(paloff_w)

//  AM_RANGE(0x0010, 0x0010) AM_WRITE_LEGACY(pcup_prgbank_w)
    AM_RANGE(0x0010, 0x0011) AM_WRITENOP

    AM_RANGE(0x0020, 0x0021) AM_WRITENOP

    AM_RANGE(0x0034, 0x0035) AM_READ_LEGACY(peno_rand) AM_WRITENOP
ADDRESS_MAP_END



static INPUT_PORTS_START(ttchamp)
    PORT_START("SYSTEM")
    PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
    PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
    PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0100, 0x0100, "0x000003" )
    PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
    PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
    PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
    PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

    PORT_START("P1_P2")
    PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
    PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
    PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
    PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
    PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
    PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
    PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
    PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
    PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
    PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
    PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
    PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
    PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
    PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
    PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
    PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START1 )

INPUT_PORTS_END


static INTERRUPT_GEN( ttchamp_irq ) /* right? */
{
	device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( ttchamp, ttchamp_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", V30, 8000000)
	MCFG_CPU_PROGRAM_MAP(ttchamp_map)
	MCFG_CPU_IO_MAP(ttchamp_io)
	MCFG_CPU_VBLANK_INT("screen", ttchamp_irq)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(1024,1024)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)
	MCFG_SCREEN_UPDATE_STATIC(ttchamp)

	MCFG_PALETTE_LENGTH(0x8000)

	MCFG_VIDEO_START(ttchamp)
MACHINE_CONFIG_END

ROM_START( ttchamp )

	/* hopefully this is a good dump */

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "2.bin", 0x000000, 0x080000,  CRC(6a6c6d75) SHA1(3742b82462176d77732a69e142db9e6f61f25dc5) )
	ROM_LOAD16_BYTE( "3.bin", 0x000001, 0x080000,  CRC(6062c0b2) SHA1(c5f0ac58c847ce2588c805f40180f2586a6477b7) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x080000,  CRC(4388dead) SHA1(1965e4b84452b244e32c8d218aace8d287c67ec2) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x080000,  CRC(fdbf9b28) SHA1(2d260555586097c8a396f65111f55ace801c7a5d) )

	/* dumps below are bad dumps */

	/* dump a */
//  ROM_REGION( 0x200000, "user1", 0 )
//  ROM_LOAD16_BYTE( "27c040_dump_a.2", 0x000000, 0x080000, BAD_DUMP CRC(791d68c8) SHA1(641c989d50e95ac3ff7c87d148cfab44abbdc774) )
//  ROM_LOAD16_BYTE( "27c040_dump_a.3", 0x000001, 0x080000, BAD_DUMP CRC(00c81241) SHA1(899d4d1566f5f5d2967b6a8ec7dca60833846bbe) )
//  ROM_LOAD16_BYTE( "27c040_dump_a.4", 0x100000, 0x080000, BAD_DUMP CRC(11af50f6) SHA1(1e5b6cc5c5a6c1ec302b2de7ce40c9ebfb349b46) )
//  ROM_LOAD16_BYTE( "27c040_dump_a.5", 0x100001, 0x080000, BAD_DUMP CRC(f6b87231) SHA1(3db461c0858c207e8a3dfd822c99d28e3a26b4ee) )

	/* dump b */
//  ROM_REGION( 0x200000, "user2", 0 )
//  ROM_LOAD16_BYTE( "27c040_dump_b.2", 0x000000, 0x080000, BAD_DUMP CRC(df1f2618) SHA1(7c6abb7a6ec55c49b95809f003d217f1ea758729) )
//  ROM_LOAD16_BYTE( "27c040_dump_b.3", 0x000001, 0x080000, BAD_DUMP CRC(0292ca69) SHA1(fe3b0e78d9e946d8f8a86e8246e5a94483f44ce1) )
//  ROM_LOAD16_BYTE( "27c040_dump_b.4", 0x100000, 0x080000, BAD_DUMP CRC(f6bcadc6) SHA1(d8c61c207175d67f4229103696dc2a4447af2ba4) )
//  ROM_LOAD16_BYTE( "27c040_dump_b.5", 0x100001, 0x080000, BAD_DUMP CRC(b872747c) SHA1(24d2aa2603a71cdfd3d45608177bb60ab7cfe8a2) )

	/* dump c */
//  ROM_REGION( 0x200000, "user3", 0 )
//  ROM_LOAD16_BYTE( "27c040_dump_c.2", 0x000000, 0x080000, BAD_DUMP CRC(be70adc7) SHA1(fe439caa54856c75ef310e456a7e61b15321031d) )
//  ROM_LOAD16_BYTE( "27c040_dump_c.3", 0x000001, 0x080000, BAD_DUMP CRC(8e3b3396) SHA1(f47243041b9283712e34ea58fa2456c35785c5ee) )
//  ROM_LOAD16_BYTE( "27c040_dump_c.4", 0x100000, 0x080000, BAD_DUMP CRC(34ab75e9) SHA1(779f03139b336cdc46f4d00bf3fd9e6de79942e2) )
//  ROM_LOAD16_BYTE( "27c040_dump_c.5", 0x100001, 0x080000, BAD_DUMP CRC(3e7b3533) SHA1(433439c2b8a8e54bb20fc3c1690d3f183c6fa6f6) )

	/* these were the same in each dump..*/
	ROM_REGION( 0x10000, "cpu1", 0 ) /* not verified if this is correct yet, seems very empty, maybe protected */
	ROM_LOAD( "pic16c84.rom", 0x000000, 0x4280,  CRC(900f2ef8) SHA1(08f206fe52f413437436e4b0d2b4ec310767446c) )

	ROM_REGION( 0x40000, "samples", 0 )
	ROM_LOAD( "27c020.1", 0x000000, 0x040000,  CRC(e2c4fe95) SHA1(da349035cc348db220a1e12b4c2a6021e2168425) )
ROM_END

/*

Table tennis Championships by Gamart 1995

This game come from Gamart,an obscure spanish software house.
Hardware info:
main cpu: V30
sound chip: oki6295
custom chip: tpc1020bfn x2
osc: 16 mhz
Rom files definition:
ttennis2/3 main program
ttennis1 adpcm data
ttennis4/5 graphics
*there is a pic16c84 that i cannot dump because my programmer doesn't support it.

Dumped by tirino73 >isolani (at) interfree.it<

*/

ROM_START( ttchampa )
	/* this is from a different board */

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "ttennis2.bin", 0x000000, 0x080000,  CRC(b060e72c) SHA1(376e71bb4b1687fec4b719cbc5a7b25b64d159ac) )
	ROM_LOAD16_BYTE( "ttennis3.bin", 0x000001, 0x080000,  CRC(33e085a8) SHA1(ea6af05690b4b0803c303a3c858df10e4d907fb1) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x080000,  CRC(4388dead) SHA1(1965e4b84452b244e32c8d218aace8d287c67ec2) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x080000,  CRC(fdbf9b28) SHA1(2d260555586097c8a396f65111f55ace801c7a5d) )

	ROM_REGION( 0x10000, "cpu1", 0 ) /* not verified if this is correct yet, seems very empty, maybe protected */
	ROM_LOAD( "pic16c84.rom", 0x000000, 0x4280,  CRC(900f2ef8) SHA1(08f206fe52f413437436e4b0d2b4ec310767446c) )

	ROM_REGION( 0x40000, "samples", 0 )
	ROM_LOAD( "27c020.1", 0x000000, 0x040000,  CRC(e2c4fe95) SHA1(da349035cc348db220a1e12b4c2a6021e2168425) )
ROM_END

static DRIVER_INIT (ttchamp)
{
	UINT8 *ROM1 = machine.region("user1")->base();
	memory_set_bankptr(machine, "bank1",&ROM1[0x120000]);
	memory_set_bankptr(machine, "bank2",&ROM1[0x180000]);
}

GAME( 199?, ttchamp, 0,        ttchamp, ttchamp, ttchamp, ROT0,  "Gamart?", "Table Tennis Champions (set 1)", GAME_NOT_WORKING|GAME_NO_SOUND )
GAME( 199?, ttchampa,ttchamp,  ttchamp, ttchamp, ttchamp, ROT0,  "Gamart?", "Table Tennis Champions (set 2)", GAME_NOT_WORKING|GAME_NO_SOUND )
