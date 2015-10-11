// license:GPL-2.0+
// copyright-holders:Norbert Kehrer
/***************************************************************************

    Mad Alien (c) 1980 Data East Corporation

    Original driver by Norbert Kehrer (February 2004)

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "includes/madalien.h"


#define SOUND_CLOCK XTAL_4MHz


INPUT_CHANGED_MEMBER(madalien_state::coin_inserted)
{
	/* coin insertion causes an NMI */
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


inline UINT8 madalien_state::shift_common(UINT8 hi, UINT8 lo)
{
	const UINT8 *table = memregion("user2")->base();

	return table[((hi & 0x07) << 8) | lo];
}

READ8_MEMBER(madalien_state::shift_r)
{
	return shift_common(*m_shift_hi, *m_shift_lo);
}

READ8_MEMBER(madalien_state::shift_rev_r)
{
	UINT8 hi = *m_shift_hi ^ 0x07;
	UINT8 lo = BITSWAP8(*m_shift_lo,0,1,2,3,4,5,6,7);

	UINT8 ret = shift_common(hi, lo);

	return BITSWAP8(ret,7,0,1,2,3,4,5,6) & 0x7f;
}


WRITE8_MEMBER(madalien_state::madalien_output_w)
{
	/* output latch, eight output bits, none connected */
}


WRITE8_MEMBER(madalien_state::madalien_sound_command_w)
{
	m_audiocpu->set_input_line(0, ASSERT_LINE);
	soundlatch_byte_w(space, offset, data);
}


READ8_MEMBER(madalien_state::madalien_sound_command_r)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return soundlatch_byte_r(space, offset);
}


WRITE8_MEMBER(madalien_state::madalien_portA_w)
{
	m_discrete->write(space, MADALIEN_8910_PORTA, data);
}
WRITE8_MEMBER(madalien_state::madalien_portB_w)
{
	m_discrete->write(space, MADALIEN_8910_PORTB, data);
}


static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, madalien_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM

	AM_RANGE(0x6000, 0x63ff) AM_RAM_WRITE(madalien_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x6400, 0x67ff) AM_RAM
	AM_RANGE(0x6800, 0x7fff) AM_RAM_WRITE(madalien_charram_w) AM_SHARE("charram")

	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x0ff0) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x8001, 0x8001) AM_MIRROR(0x0ff0) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x8004, 0x8004) AM_MIRROR(0x0ff0) AM_WRITEONLY AM_SHARE("video_control")
	AM_RANGE(0x8005, 0x8005) AM_MIRROR(0x0ff0) AM_WRITE(madalien_output_w)
	AM_RANGE(0x8006, 0x8006) AM_MIRROR(0x0ff0) AM_READ(soundlatch2_byte_r) AM_WRITE(madalien_sound_command_w)
	AM_RANGE(0x8008, 0x8008) AM_MIRROR(0x07f0) AM_RAM_READ(shift_r) AM_SHARE("shift_hi")
	AM_RANGE(0x8009, 0x8009) AM_MIRROR(0x07f0) AM_RAM_READ(shift_rev_r) AM_SHARE("shift_lo")
	AM_RANGE(0x800b, 0x800b) AM_MIRROR(0x07f0) AM_WRITEONLY AM_SHARE("video_flags")
	AM_RANGE(0x800c, 0x800c) AM_MIRROR(0x07f0) AM_WRITEONLY AM_SHARE("headlight_pos")
	AM_RANGE(0x800d, 0x800d) AM_MIRROR(0x07f0) AM_WRITEONLY AM_SHARE("edge1_pos")
	AM_RANGE(0x800e, 0x800e) AM_MIRROR(0x07f0) AM_WRITEONLY AM_SHARE("edge2_pos")
	AM_RANGE(0x800f, 0x800f) AM_MIRROR(0x07f0) AM_WRITEONLY AM_SHARE("scroll")

	AM_RANGE(0x9000, 0x9000) AM_MIRROR(0x0ff0) AM_READ_PORT("PLAYER1")
	AM_RANGE(0x9001, 0x9001) AM_MIRROR(0x0ff0) AM_READ_PORT("DSW")
	AM_RANGE(0x9002, 0x9002) AM_MIRROR(0x0ff0) AM_READ_PORT("PLAYER2")

	AM_RANGE(0xa000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( audio_map, AS_PROGRAM, 8, madalien_state )
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x1c00) AM_RAM
	AM_RANGE(0x6000, 0x6003) AM_MIRROR(0x1ffc) AM_RAM /* unknown device in an epoxy block, might be tilt detection */
	AM_RANGE(0x8000, 0x8000) AM_MIRROR(0x1ffc) AM_READ(madalien_sound_command_r)
	AM_RANGE(0x8000, 0x8001) AM_MIRROR(0x1ffc) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x8002, 0x8002) AM_MIRROR(0x1ffc) AM_WRITE(soundlatch2_byte_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( madalien )
	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives )) PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage )) PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life )) PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPSETTING(    0x10, "5000" )
	PORT_DIPSETTING(    0x20, "7000" )
	PORT_DIPSETTING(    0x30, "never" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet )) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ))
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, madalien_state,coin_inserted, 0)
INPUT_PORTS_END


static MACHINE_CONFIG_START( madalien, madalien_state )

	/* main CPU */
	MCFG_CPU_ADD("maincpu", M6502, MADALIEN_MAIN_CLOCK / 8) /* 1324kHz */
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("audiocpu", M6502, SOUND_CLOCK / 8)        /* 512kHz */
	MCFG_CPU_PROGRAM_MAP(audio_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", madalien_state,  nmi_line_pulse)

	/* video hardware */
	MCFG_FRAGMENT_ADD(madalien_video)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_ORIENTATION(ROT270)

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, SOUND_CLOCK / 4)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(madalien_state, madalien_portA_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(madalien_state, madalien_portB_w))
	MCFG_SOUND_ROUTE_EX(0, "discrete", 1.0, 0)
	MCFG_SOUND_ROUTE_EX(1, "discrete", 1.0, 1)
	MCFG_SOUND_ROUTE_EX(2, "discrete", 1.0, 2)

	MCFG_SOUND_ADD("discrete", DISCRETE, 0)
	MCFG_DISCRETE_INTF(madalien)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( madalien )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* main CPU */
	ROM_LOAD( "m7.3f",  0xc000, 0x0800, CRC(4d12f89d) SHA1(e155f9135bc2bea56e211052f2b74d25e76308c8) )
	ROM_LOAD( "m6.3hj", 0xc800, 0x0800, CRC(1bc4a57b) SHA1(02252b868d0c07c0a18240e9d831c303cdcfa9a6) )
	ROM_LOAD( "m5.3k",  0xd000, 0x0800, CRC(8db99572) SHA1(f8cf22f8c134b47756b7f02c5ca0217100466744) )
	ROM_LOAD( "m4.3l",  0xd800, 0x0800, CRC(fba671af) SHA1(dd74bd357c82d525948d836a7f860bbb3182c825) )
	ROM_LOAD( "m3.4f",  0xe000, 0x0800, CRC(1aad640d) SHA1(9ace7d2c5ef9e789c2b8cc65420b19ce72cd95fa) )
	ROM_LOAD( "m2.4hj", 0xe800, 0x0800, CRC(cbd533a0) SHA1(d3be81fb9ba40e30e5ff0171efd656b11dd20f2b) )
	ROM_LOAD( "m1.4k",  0xf000, 0x0800, CRC(ad654b1d) SHA1(f8b365dae3801e97e04a10018a790d3bdb5d9439) )
	ROM_LOAD( "m0.4l",  0xf800, 0x0800, CRC(cf7aa787) SHA1(f852cc806ecc582661582326747974a14f50174a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* audio CPU */
	ROM_LOAD( "m8.4de", 0xf800, 0x0400, CRC(cfd19dab) SHA1(566dc84ffe9bcaeb112250a9e1882bf62f47b579) )
	ROM_LOAD( "m9.3de", 0xfc00, 0x0400, CRC(48f30f24) SHA1(9c0bf6e43b143d6af1ebb9dad2bdc2b53eb2e48e) )

	ROM_REGION( 0x0400, "gfx2", 0 )    /* headlight */
	ROM_LOAD( "ma-.2bc", 0x0000, 0x0400, CRC(aab16446) SHA1(d2342627cc2766004343f27515d8a7989d5fe932) )

	ROM_REGION( 0x0400, "user1", 0 )        /* background tile map */
	ROM_LOAD( "mf.4h", 0x0000, 0x0400, CRC(e9cba773) SHA1(356c7edb1b412a9e04f0747e780c945af8791c55) )

	ROM_REGION( 0x0c00, "gfx1", 0 )         /* background tiles */
	ROM_LOAD( "mc.3k", 0x0000, 0x0400, CRC(2daadfb7) SHA1(8be084a39b256e538fd57111e92d47115cb142cd) )
	ROM_LOAD( "md.3l", 0x0400, 0x0400, CRC(3ee1287a) SHA1(33bc59a8d09d22f3db80f881c2f37aa788718138) )
	ROM_LOAD( "me.3m", 0x0800, 0x0400, CRC(45a5c201) SHA1(ac600afeabf494634c3189d8e96644bd0deb45f3) )

	ROM_REGION( 0x0800, "user2", 0 )                   /* shifting table */
	ROM_LOAD( "mb-.5cd", 0x0000, 0x0800, CRC(cb801e49) SHA1(7444c4af7cf07e5fdc54044d62ea4fcb201b2b8b) )

	ROM_REGION( 0x0020, "proms", 0 )        /* color PROM */
	ROM_LOAD( "mg.7e",  0x0000, 0x0020, CRC(3395b31f) SHA1(26235fb448a4180c58f0887e53a29c17857b3b34) )
ROM_END


/***************************************************************************

Mad Rider / Mad Alien
Data East, 1980

This game runs on 4 boards that plug into a PCB containing slots. The whole
thing is housed in a metal box. All of the PCBs in this box are labelled
'MAD RIDER'. The actual game appears to be Highway Chase on a Mad Alien PCB
and might have been converted from Mad Alien. However, the title screen still
says 'Mad Alien'. The graphics are bad on the edge of the road because one
PROM at 3K is incorrect. If you have access to a Mad Rider PCB, please dump
the PROM at 3K or contact us.


PCB Layouts
-----------

Top board

DE-0048B-1 CPU
|-----------------------------------|
|                                   |
|                   10.595MHz       |
|           DIP8(?)                 |
|                                   |
|                                   |
|     DSW(8)                        |
|                                   |
|     DIP40                         |
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
|                                   |
|-----------------------------------|
Notes:
      This is joined to the PCB below with 4 small flat cables
      All of the part numbers are scratched out on this PCB
      DIP40 is a 6502 @ 1.324375MHz [10.595 / 8]


DE-0044B
 |-------------------------------------|
 |                                     |
|-|                                    |
| |     DIP40                          |-|
| |      M5L8216P  2114 2114             |
| |*                                     |
| |      M5L8216P  2114 2114             |
|-|                                      |
 |   2114 M5L8216P  2114 2114  MG-1.7F   |
 |                                       |
 |   2114 M5L8216P  2114 2114            |
 |                                       |
 |   2114 M5L8216P  2114 2114          |-|
 |                                     |
 |   2114 M5L8216P  2114 2114          |
 |-------------------------------------|
Notes:
      * - Flat cable joined to next PCB down
  DIP40 - probably 6845 video chip (surface scratched)
   MG-1 - 82S123 Bipolar PROM


DE-0045B-1
 |-------------------------------------|
 |            MD-1.3M                  |
|-|           ME-1.3L                  |
| |           MC-1.3K                  |-|
| |                                      |
| |*               MF-1.4H               |
| |                                      |
|-|                                      |
 |                                       |
 |                                       |
 |                                       |
 |                       MB.5C           |
 |                                     |-|
 |      MA.2B                          |
 |                                     |
 |-------------------------------------|
Notes:
      * - Flat cable joined to above PCB
     MA - 2708 EPROM
     MB - 2716 EPROM
MC / MD \
ME / MF - uPB426 or 82S137 Bipolar PROMs


DE-0047B-1
|-------------------------------------|
|                       DIP16         |
|   555                4MHz           |
|                 8910                |-|
|   4066                6502            |
|                                       |
|                                       |
|*                                      |
|   555        9_2708.3D                |
|                      8_2708.4D        |
|                                       |
|          LM348                        |
|                 4066  2114          |-|
|   555                               |
|          LM348  4066  2114          |
|-------------------------------------|
Notes:
      * - 3 pin sound output connector
   6502 - 6502 CPU running at 0.500MHz [4/8]
          NMI on pin 6 measured 50.0Hz
   8910 - AY3-8910 sound chip running at 1.000MHz [4/4]
  DIP16 - socket for small plug-in board containing 3 chips
          which is covered with epoxy resin


Bottom board

DE-0046B-1 ROM
 |-------------------------------------|
 |                                  @  |
 |             2716.3L   2716.4L       |
 |                                     |-|
|-|            2716.3K   2716.4K         |
| |                                      |
| |            2716.3H   2716.4H         |
| |                                  #   |
| |            2716.3F   2716.4F         |
| |                                      |
| |                      2716.4E         |
| |                                      |
|-|                      2716.4C       |-|
 |                                     |
 |    &                             %  |
 |-------------------------------------|
Notes:
      * - 50 pin flat cable connector for controls and video output
      & - 4 wire jumpers
      % - 4 wire jumpers
      # - 6 wire jumpers
      @ - 3 wire jumpers
      All ROMs type 2716
      VSync - 55Hz
      HSync - 15.43kHz

***************************************************************************/

ROM_START( madaliena )
	ROM_REGION(0x10000, "maincpu", 0)                   /* main CPU */
	ROM_LOAD("mk-6.c4",  0xb000, 0x0800, CRC(90be68af) SHA1(472ccfd2e04d6d49be47d919cba0c55d850b2887) )
	ROM_LOAD("mj-6.4e",  0xb800, 0x0800, CRC(aba10cbb) SHA1(6ca213ded8ed7f4f310ab5ae25220cf867dd1d00) )
	ROM_LOAD("m7-6.3f",  0xc000, 0x0800, CRC(c3af484c) SHA1(c3667526d3b5aeee68823f92826053e657512851) )
	ROM_LOAD("m6-6.3hj", 0xc800, 0x0800, CRC(78ca5a87) SHA1(729d69ee63c710241a098471e9769063dfe8ef1e) )
	ROM_LOAD("m5-6.3k",  0xd000, 0x0800, CRC(070e81ea) SHA1(006831f4bf289812e4e87a3ece7885e8b901f2f5) )
	ROM_LOAD("m4-6.3l",  0xd800, 0x0800, CRC(98225cb0) SHA1(ca74f5e33fa9116215b03abadd5d09840c04fb0b) )
	ROM_LOAD("m3-6.4f",  0xe000, 0x0800, CRC(52fea0fc) SHA1(443fd859daf4279d5976256a4b1c970b520661a2) )
	ROM_LOAD("m2-6.4hj", 0xe800, 0x0800, CRC(dba6c4f6) SHA1(51f815fc7eb99a05eee6204de2d4cad1734adc52) )
	ROM_LOAD("m1-6.4k",  0xf000, 0x0800, CRC(06991af6) SHA1(19112306529721222b6e1c07920348c263d8b8aa) )
	ROM_LOAD("m0-6.4l",  0xf800, 0x0800, CRC(57752b47) SHA1(a34d3150ea9082889154042dbea3386f71322a78) )

	ROM_REGION( 0x10000, "audiocpu", 0 )                   /* audio CPU */
	ROM_LOAD( "m8-1.4de", 0xf800, 0x0400, CRC(46162e7e) SHA1(7ed85f4a9ac58d6d9bafba0c843a16c269656563) )
	ROM_LOAD( "m9-1.3de", 0xfc00, 0x0400, CRC(4175f5c4) SHA1(45cae8a1fcfd34b91c63cc7e544a32922da14f16) )

	ROM_REGION( 0x0400, "gfx2", 0 )    /* headlight */
	ROM_LOAD( "ma-.2bc", 0x0000, 0x0400, CRC(aab16446) SHA1(d2342627cc2766004343f27515d8a7989d5fe932) )

	// same gfx and tilemap as 'madalien' set but in double size roms, 2nd half empty
	ROM_REGION( 0x0800, "user1", 0 )                    /* background tile map */
	ROM_LOAD( "mf.4h", 0x0000, 0x0800, CRC(5c1e1602) SHA1(5ea8b3e9381364fed1030221fd5cdda318edaa1a) )

	ROM_REGION( 0x1800, "gfx1", 0 )    /* background tiles */
	ROM_LOAD( "mc.3k", 0x0000, 0x0800, CRC(75072223) SHA1(3c292698683d20d83d512cc4d17c6578da19e29d) )
	ROM_LOAD( "me.3l", 0x0800, 0x0800, CRC(740f606d) SHA1(a430ee1333d2d85381eaf197570799a694301d2d) )
	ROM_LOAD( "md.3m", 0x1000, 0x0800, CRC(7813dd11) SHA1(49bcdbbad13a5b9dc7ea43b8cdc564f6a8697148) )

	ROM_REGION( 0x0800, "user2", 0 )                   /* shifting table */
	ROM_LOAD( "mb-.5cd", 0x0000, 0x0800, CRC(cb801e49) SHA1(7444c4af7cf07e5fdc54044d62ea4fcb201b2b8b) )

	ROM_REGION( 0x0020, "proms", 0 )                   /* color PROM */
	ROM_LOAD( "prom.7e", 0x0000, 0x0020, CRC(e622396a) SHA1(8972704bd25fed462e25c453771cc5ca4fc74034) )
ROM_END

// This set has the same ROMs as 'madaliena' except it uses a different set of tile roms and background tilemap, much closer to
// 'Highway Chase' on the Cassette system with a Desert theme instead of a Vegas one (only the design on the 2nd tile differs slightly)
// unfortuantely one of the ROMs was damaged and due to the single tile being different we can't use GFX extracted from the cassette
// version.
ROM_START( madalienb )
	ROM_REGION( 0x10000, "maincpu", 0 )                   /* main CPU */
	ROM_LOAD("mk-6.c4",  0xb000, 0x0800, CRC(90be68af) SHA1(472ccfd2e04d6d49be47d919cba0c55d850b2887) )
	ROM_LOAD("mj-6.4e",  0xb800, 0x0800, CRC(aba10cbb) SHA1(6ca213ded8ed7f4f310ab5ae25220cf867dd1d00) )
	ROM_LOAD("m7-6.3f",  0xc000, 0x0800, CRC(c3af484c) SHA1(c3667526d3b5aeee68823f92826053e657512851) )
	ROM_LOAD("m6-6.3hj", 0xc800, 0x0800, CRC(78ca5a87) SHA1(729d69ee63c710241a098471e9769063dfe8ef1e) )
	ROM_LOAD("m5-6.3k",  0xd000, 0x0800, CRC(070e81ea) SHA1(006831f4bf289812e4e87a3ece7885e8b901f2f5) )
	ROM_LOAD("m4-6.3l",  0xd800, 0x0800, CRC(98225cb0) SHA1(ca74f5e33fa9116215b03abadd5d09840c04fb0b) )
	ROM_LOAD("m3-6.4f",  0xe000, 0x0800, CRC(52fea0fc) SHA1(443fd859daf4279d5976256a4b1c970b520661a2) )
	ROM_LOAD("m2-6.4hj", 0xe800, 0x0800, CRC(dba6c4f6) SHA1(51f815fc7eb99a05eee6204de2d4cad1734adc52) )
	ROM_LOAD("m1-6.4k",  0xf000, 0x0800, CRC(06991af6) SHA1(19112306529721222b6e1c07920348c263d8b8aa) )
	ROM_LOAD("m0-6.4l",  0xf800, 0x0800, CRC(57752b47) SHA1(a34d3150ea9082889154042dbea3386f71322a78) )

	ROM_REGION( 0x10000, "audiocpu", 0 )                   /* audio CPU */
	ROM_LOAD( "m8-1.4de", 0xf800, 0x0400, CRC(46162e7e) SHA1(7ed85f4a9ac58d6d9bafba0c843a16c269656563) )
	ROM_LOAD( "m9-1.3de", 0xfc00, 0x0400, CRC(4175f5c4) SHA1(45cae8a1fcfd34b91c63cc7e544a32922da14f16) )

	ROM_REGION( 0x0400, "gfx2", 0 )    /* headlight */
	ROM_LOAD( "ma-.2bc", 0x0000, 0x0400, CRC(aab16446) SHA1(d2342627cc2766004343f27515d8a7989d5fe932) )



	ROM_REGION( 0x0400, "user1", 0 )                    // background tile map
	ROM_LOAD( "mf-1.4h", 0x0000, 0x0400, CRC(9b04c446) SHA1(918013f3c0244ab6a670b9d1b6b642298e2c5ab8) )

	ROM_REGION( 0x0c00, "gfx1", 0 )    // background tiles
	ROM_LOAD( "mc-1.3k", 0x0000, 0x0400, BAD_DUMP CRC(2710c47e) SHA1(337e4f160c7db143ec3bfae3e08e8789b9e41cc5) ) // taken from chwy, see below, tile 2 is mismatched with the 2 roms from the actual PCB.
	ROM_LOAD( "me-1.3l", 0x0400, 0x0400, CRC(7328a425) SHA1(327adc8b0e25d93f1ae98a44c26d0aaaac1b1a9c) )
	ROM_LOAD( "md-1.3m", 0x0800, 0x0400, CRC(b5329929) SHA1(86890e1b7cc8cb31fc0dcbc2d3cff02e4cf95619) )

	/*  for reference, this is the data used by Highway Chase on the cassette system when extracted
	ROM_REGION( 0x0400, "user1", 0 )                    // background tile map
	ROM_LOAD( "rom1", 0x0000, 0x0400, CRC(9b04c446) SHA1(918013f3c0244ab6a670b9d1b6b642298e2c5ab8) )

	ROM_REGION( 0x0c00, "gfx1", 0 )    // background tiles
	ROM_LOAD( "rom2", 0x0000, 0x0400, CRC(2710c47e) SHA1(337e4f160c7db143ec3bfae3e08e8789b9e41cc5) )
	ROM_LOAD( "rom3", 0x0400, 0x0400, CRC(4ec78c6a) SHA1(52c7fd0cbd2716edbfae0c73cc59fd3097941e2d) )
	ROM_LOAD( "rom4", 0x0800, 0x0400, CRC(c1f7375e) SHA1(02358f498565bb88b6c323620a5b85a39e8c84db) )
	*/

	ROM_REGION( 0x0800, "user2", 0 )                   /* shifting table */
	ROM_LOAD( "mb-.5cd", 0x0000, 0x0800, CRC(cb801e49) SHA1(7444c4af7cf07e5fdc54044d62ea4fcb201b2b8b) )

	ROM_REGION( 0x0020, "proms", 0 )                   /* color PROM */
	ROM_LOAD( "prom.7e", 0x0000, 0x0020, CRC(e622396a) SHA1(8972704bd25fed462e25c453771cc5ca4fc74034) )
ROM_END

/*          set       parent    machine   inp       init */
GAME( 1980, madalien, 0,        madalien, madalien, driver_device, 0, ROT270, "Data East Corporation", "Mad Alien (set 1)",          MACHINE_SUPPORTS_SAVE )
GAME( 1980, madaliena,madalien, madalien, madalien, driver_device, 0, ROT270, "Data East Corporation", "Mad Alien (set 2)",          MACHINE_SUPPORTS_SAVE )
GAME( 1980, madalienb,madalien, madalien, madalien, driver_device, 0, ROT270, "Data East Corporation", "Mad Alien (set 2, alt gfx)", MACHINE_SUPPORTS_SAVE )
