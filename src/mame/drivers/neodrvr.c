/***************************************************************************

    Neo-Geo hardware

    This file contains all game specific overrides

****************************************************************************/


/* Game specific input definitions */

static INPUT_PORTS_START( svcpcb )
	STANDARD_IN0

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	/* the rom banking is tied directly to the dipswitch?, or is there a bank write somewhere? */
	PORT_START("HARDDIP")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Region ) ) PORT_DIPLOCATION("HARDDIP:3")
	PORT_DIPSETTING(	0x00, DEF_STR( Asia ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Japan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( ms5pcb )
	STANDARD_IN0

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	/* the rom banking is tied directly to the dipswitch?, or is there a bank write somewhere? */
	PORT_START("HARDDIP")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Region ) ) PORT_DIPLOCATION("HARDDIP:3")
	PORT_DIPSETTING(	0x00, DEF_STR( Asia ) )
	PORT_DIPSETTING(	0x01, DEF_STR( Japan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( kog )
	STANDARD_IN0

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	/* a jumper on the pcb overlays a ROM address, very strange but that's how it works. */
	PORT_START("JUMPER")
	PORT_DIPNAME( 0x0001, 0x0001, "Title Language" ) PORT_DIPLOCATION("CART-JUMPER:1")
	PORT_DIPSETTING(	  0x0001, DEF_STR( English ) )
	PORT_DIPSETTING(	  0x0000, "Non-English" )
	PORT_BIT( 0x00fe, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( mjneogeo )
	PORT_START("IN0")
	PORT_DIPNAME( 0x0001, 0x0001, "Test Switch" ) PORT_DIPLOCATION("SW:1")
	PORT_DIPSETTING(	  0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Coin Chutes?" ) PORT_DIPLOCATION("SW:2")
	PORT_DIPSETTING(	  0x0000, "1?" )
	PORT_DIPSETTING(	  0x0002, "2?" )
	PORT_DIPNAME( 0x0004, 0x0000, "Mahjong Control Panel" ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(	  0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0018, "COMM Setting (Cabinet No.)" ) PORT_DIPLOCATION("SW:4,5")
	PORT_DIPSETTING(	  0x0018, "1" )
	PORT_DIPSETTING(	  0x0008, "2" )
	PORT_DIPSETTING(	  0x0010, "3" )
	PORT_DIPSETTING(	  0x0000, "4" )
	PORT_DIPNAME( 0x0020, 0x0020, "COMM Setting (Link Enable)" ) PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(	  0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(	  0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Freeze" ) PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(	  0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(	  0x0000, DEF_STR( On ) )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(mahjong_controller_r, NULL)

	STANDARD_IN1

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	PORT_START("MAHJONG1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MAHJONG2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MAHJONG3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("MAHJONG4")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( irrmaze )
	PORT_START("IN0")
	STANDARD_DIPS
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(multiplexed_controller_r, (void *)0)

	PORT_START("IN1")
	PORT_BIT( 0x0fff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_memcard_status, NULL)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	STANDARD_IN3

	PORT_START("IN4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )  /* this bit is used.. */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Enter BIOS") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN0-0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_REVERSE

	PORT_START("IN0-1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(10) PORT_KEYDELTA(20) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( popbounc )
	PORT_START("IN0")
	STANDARD_DIPS
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(multiplexed_controller_r, (void *)0)

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(multiplexed_controller_r, (void *)1)

	STANDARD_IN2

	STANDARD_IN3

	STANDARD_IN4

	/* Fake inputs read by CUSTOM_INPUT handlers */
	PORT_START("IN0-0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("IN0-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x90, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* note it needs it from 0x80 when using paddle */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1-0")
	PORT_BIT( 0xff, 0x00, IPT_DIAL  ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20) PORT_PLAYER(2)

	PORT_START("IN1-1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x90, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) /* note it needs it from 0x80 when using paddle */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( vliner )
	PORT_START("IN0")
	STANDARD_DIPS
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("View Payout Table/Big")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet/Small")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Stop/Double Up")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Start/Collect")

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* this bit is used.. */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* this bit is used.. */
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_memcard_status, NULL)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_calendar_status, NULL)
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_audio_result, NULL)

	STANDARD_IN4

	PORT_START("IN5")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Operator Menu") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Clear Credit")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Hopper Out")
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* not sure what any of these bits are */
	PORT_START("IN6")
	PORT_BIT( 0x0003, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( jockeygp )
	STANDARD_IN0

	STANDARD_IN1

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* game freezes with this bit enabled */
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Next Game") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* game freezes with this bit enabled */
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Previous Game") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x7000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(get_memcard_status, NULL)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	STANDARD_IN3

	STANDARD_IN4
INPUT_PORTS_END


/* Neo-Geo bios */

/******************************************************************************

 These are the known Bios Roms, Set options.bios to the one you want

 The Universe bios roms are supported because they're now used on enough PCBs
 to be considered 'in active arcade use' rather than just homebrew hacks.
  -- some may be missing, there have been multiple CRCs reported for the same
     revision in some cases (the Universe bios has an option for entering / displaying
     a serial number; these should be noted as such if they're added.

 The 'japan-hotel' BIOS is a dump of an MVS which could be found in some japanese
 hotels. it is a custom MVS mobo which uses MVS carts but it hasn't jamma
 connector and it's similar to a console with a coin mechanism, so it's a sort
 of little coin op console installed in hotels.

 *****************************************************************************/

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios+1)) /* Note '+1' */

#define NEOGEO_BIOS \
	ROM_REGION16_BE( 0x80000, "mainbios", 0 )													\
	ROM_SYSTEM_BIOS( 0, "euro",       "Europe MVS (Ver. 2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "sp-s2.sp1",    0x00000, 0x020000, CRC(9036d879) SHA1(4f5ed7105b7128794654ce82b51723e16e389543) ) /* Europe, 1 Slot, has also been found on 2 Slot and 4 Slot (the old hacks were designed for this one) */ \
	ROM_SYSTEM_BIOS( 1, "euro-s1",    "Europe MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "sp-s.sp1",     0x00000, 0x020000, CRC(c7f2fa45) SHA1(09576ff20b4d6b365e78e6a5698ea450262697cd) ) /* Europe, 4 Slot */ \
	ROM_SYSTEM_BIOS( 2, "us",         "US MVS (Ver. 2?)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "usa_2slt.bin", 0x00000, 0x020000, CRC(e72943de) SHA1(5c6bba07d2ec8ac95776aa3511109f5e1e2e92eb) ) /* US, 2 Slot */ \
	ROM_SYSTEM_BIOS( 3, "us-e",       "US MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "sp-e.sp1",     0x00000, 0x020000, CRC(2723a5b5) SHA1(5dbff7531cf04886cde3ef022fb5ca687573dcb8) ) /* US, 6 Slot (V5?) */ \
	ROM_SYSTEM_BIOS( 4, "asia",       "Asia MVS (Ver. 3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 4, "asia-s3.rom",  0x00000, 0x020000, CRC(91b64be3) SHA1(720a3e20d26818632aedf2c2fd16c54f213543e1) ) /* Asia */ \
	ROM_SYSTEM_BIOS( 5, "japan",      "Japan MVS (Ver. 3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 5, "vs-bios.rom",  0x00000, 0x020000, CRC(f0e8f27d) SHA1(ecf01eda815909f1facec62abf3594eaa8d11075) ) /* Japan, Ver 6 VS Bios */ \
	ROM_SYSTEM_BIOS( 6, "japan-s2",   "Japan MVS (Ver. 2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 6, "sp-j2.sp1",    0x00000, 0x020000, CRC(acede59c) SHA1(b6f97acd282fd7e94d9426078a90f059b5e9dd91) ) /* Japan, Older */ \
	ROM_SYSTEM_BIOS( 7, "japan-s1",   "Japan MVS (Ver. 1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 7, "sp1.jipan.1024",0x00000, 0x020000, CRC(9fb0abe4) SHA1(18a987ce2229df79a8cf6a84f968f0e42ce4e59d) ) /* Japan, Older */ \
	ROM_SYSTEM_BIOS( 8, "mv1c",   "NEO-MVH MV1C" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 8, "sp-45.sp1",0x00000, 0x080000, CRC(03cc9f6a) SHA1(cdf1f49e3ff2bac528c21ed28449cf35b7957dc1) ) /* MV1C; redump required */ \
	ROM_SYSTEM_BIOS( 9, "japan-hotel", "Custom Japanese Hotel" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 9, "sp-1v1_3db8c.bin",0x00000, 0x020000, CRC(162f0ebe) SHA1(fe1c6dd3dfcf97d960065b1bb46c1e11cb7bf271)  ) /* 'rare MVS found in japanese hotels' shows v1.3 in test mode */ \
	ROM_SYSTEM_BIOS( 10, "uni-bios_2_3","Universe Bios (Hack, Ver. 2.3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 10, "uni-bios_2_3.rom",  0x00000, 0x020000, CRC(27664eb5) SHA1(5b02900a3ccf3df168bdcfc98458136fd2b92ac0) ) /* Universe Bios v2.3 (hack) */ \
	ROM_SYSTEM_BIOS( 11, "uni-bios_2_3o","Universe Bios (Hack, Ver. 2.3, older?)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 11, "uni-bios_2_3o.rom",  0x00000, 0x020000, CRC(601720ae) SHA1(1b8a72c720cdb5ee3f1d735bbcf447b09204b8d9) ) /* Universe Bios v2.3 (hack) alt version, withdrawn? */ \
	ROM_SYSTEM_BIOS( 12, "uni-bios_2_2","Universe Bios (Hack, Ver. 2.2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 12, "uni-bios_2_2.rom",  0x00000, 0x020000, CRC(2d50996a) SHA1(5241a4fb0c63b1a23fd1da8efa9c9a9bd3b4279c) ) /* Universe Bios v2.2 (hack) */ \
	ROM_SYSTEM_BIOS( 13, "uni-bios_2_1","Universe Bios (Hack, Ver. 2.1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 13, "uni-bios_2_1.rom",  0x00000, 0x020000, CRC(8dabf76b) SHA1(c23732c4491d966cf0373c65c83c7a4e88f0082c) ) /* Universe Bios v2.1 (hack) */ \
	ROM_SYSTEM_BIOS( 14, "uni-bios_2_0","Universe Bios (Hack, Ver. 2.0)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 14, "uni-bios_2_0.rom",  0x00000, 0x020000, CRC(0c12c2ad) SHA1(37bcd4d30f3892078b46841d895a6eff16dc921e) ) /* Universe Bios v2.0 (hack) */ \
	ROM_SYSTEM_BIOS( 15, "uni-bios_1_3","Universe Bios (Hack, Ver. 1.3)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 15, "uni-bios_1_3.rom",  0x00000, 0x020000, CRC(b24b44a0) SHA1(eca8851d30557b97c309a0d9f4a9d20e5b14af4e) ) /* Universe Bios v1.3 (hack) */ \
	ROM_SYSTEM_BIOS( 16, "uni-bios_1_2","Universe Bios (Hack, Ver. 1.2)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 16, "uni-bios_1_2.rom",  0x00000, 0x020000, CRC(4fa698e9) SHA1(682e13ec1c42beaa2d04473967840c88fd52c75a) ) /* Universe Bios v1.2 (hack) */ \
	ROM_SYSTEM_BIOS( 17, "uni-bios_1_2o","Universe Bios (Hack, Ver. 1.2, older)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 17, "uni-bios_1_2o.rom", 0x00000, 0x020000, CRC(e19d3ce9) SHA1(af88ef837f44a3af2d7144bb46a37c8512b67770) ) /* Universe Bios v1.2 (hack) alt version */ \
	ROM_SYSTEM_BIOS( 18, "uni-bios_1_1","Universe Bios (Hack, Ver. 1.1)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 18, "uni-bios_1_1.rom",  0x00000, 0x020000, CRC(5dda0d84) SHA1(4153d533c02926a2577e49c32657214781ff29b7) ) /* Universe Bios v1.1 (hack) */ \
	ROM_SYSTEM_BIOS( 19, "uni-bios_1_0","Universe Bios (Hack, Ver. 1.0)" ) \
	ROM_LOAD16_WORD_SWAP_BIOS( 19, "uni-bios_1_0.rom",  0x00000, 0x020000, CRC(0ce453a0) SHA1(3b4c0cd26c176fc6b26c3a2f95143dd478f6abf9) ) /* Universe Bios v1.0 (hack) */ \
//  ROM_SYSTEM_BIOS( 20, "debug",      "Debug MVS (Hack?)" )
//  ROM_LOAD16_WORD_SWAP_BIOS( 20, "neodebug.rom", 0x00000, 0x020000, CRC(698ebb7d) SHA1(081c49aa8cc7dad5939833dc1b18338321ea0a07) ) /* Debug (Development) Bios */
//  ROM_SYSTEM_BIOS(21, "asia-aes",   "Asia AES" )
//  ROM_LOAD16_WORD_SWAP_BIOS(21, "neo-epo.bin", 0x00000, 0x020000, CRC(d27a71f1) SHA1(1b3b22092f30c4d1b2c15f04d1670eb1e9fbea07) ) /* AES Console (Asia?) Bios */
//  ROM_SYSTEM_BIOS(22, "jap-aes",   "Japan AES" )
//  ROM_LOAD16_WORD_SWAP_BIOS(22, "neo-po.bin", 0x00000, 0x020000, CRC(16d0c132) SHA1(4e4a440cae46f3889d20234aebd7f8d5f522e22c) ) /* AES Console (Japan) Bios */

/* note you'll have to modify the last for lines of each block to use the extra bios roms,
   they're hacks / homebrew / console bios roms so MAME doesn't list them by default */


/******************************************************************************

    The current Neo-Geo MVS system set (SFIX/SM1/000-LO) used is from a NEO-MVH MV1FS board.
    Other boards (MV1xx / MV2x / MV4 /MV6) other system sets?

    Zoom ROM (LO)    128K   TC531000CP      1x 128Kx8   Zoom look-up table ROM
    Fix ROM (SFIX)   128K   27C1000         1x 128Kx8   Text layer graphics ROM
    Sound ROM (SM1)  128K   27C1000/23C1000 1x 128Kx8   Z80 program ROM

******************************************************************************/

#define NEO_BIOS_AUDIO_64K(name, hash)																			\
	NEOGEO_BIOS 																								\
																												\
	ROM_REGION( 0x20000, "audiobios", 0 )														\
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )		\
																												\
	ROM_REGION( 0x20000, "audiocpu", 0 )													\
	ROM_LOAD( name, 	 0x00000, 0x10000, hash )																\
	ROM_RELOAD(          0x10000, 0x10000 )


#define NEO_BIOS_AUDIO_128K(name, hash) 																		\
	NEOGEO_BIOS 																								\
																												\
	ROM_REGION( 0x20000, "audiobios", 0 )														\
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )		\
																												\
	ROM_REGION( 0x30000, "audiocpu", 0 )													\
	ROM_LOAD( name, 	 0x00000, 0x20000, hash )																\
	ROM_RELOAD(          0x10000, 0x20000 )


#define NEO_BIOS_AUDIO_256K(name, hash) 																		\
	NEOGEO_BIOS 																								\
																												\
	ROM_REGION( 0x20000, "audiobios", 0 )														\
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )		\
																												\
	ROM_REGION( 0x50000, "audiocpu", 0 )													\
	ROM_LOAD( name, 	 0x00000, 0x40000, hash )																\
	ROM_RELOAD(          0x10000, 0x40000 )


#define NEO_BIOS_AUDIO_512K(name, hash) 																		\
	NEOGEO_BIOS 																								\
																												\
	ROM_REGION( 0x20000, "audiobios", 0 )														\
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )		\
																												\
	ROM_REGION( 0x90000, "audiocpu", 0 )													\
	ROM_LOAD( name, 	 0x00000, 0x80000, hash )																\
	ROM_RELOAD(          0x10000, 0x80000 )

#define NEO_BIOS_AUDIO_ENCRYPTED_512K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name, 	 0x00000, 0x80000, hash ) \

#define NEO_BIOS_AUDIO_ENCRYPTED_256K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name, 	 0x00000, 0x40000, hash ) \

#define NEO_BIOS_AUDIO_ENCRYPTED_128K(name, hash) \
	NEOGEO_BIOS \
	ROM_REGION( 0x20000, "audiobios", 0 ) \
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) \
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF ) \
	ROM_REGION( 0x80000, "audiocrypt", 0 ) \
	ROM_LOAD( name, 	 0x00000, 0x20000, hash ) \


#define NO_DELTAT_REGION


#define NEO_SFIX_32K(name, hash)																				\
	ROM_REGION( 0x20000, "fixed", 0 )												\
	ROM_LOAD( name,       0x000000, 0x08000, hash ) 															\
																												\
	ROM_REGION( 0x20000, "fixedbios", 0 )													\
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )		\
																												\
	ROM_Y_ZOOM


#define NEO_SFIX_64K(name, hash)																				\
	ROM_REGION( 0x20000, "fixed", 0 )												\
	ROM_LOAD( name,       0x000000, 0x10000, hash ) 															\
																												\
	ROM_REGION( 0x20000, "fixedbios", 0 )													\
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )		\
																												\
	ROM_Y_ZOOM


#define NEO_SFIX_128K(name, hash)																				\
	ROM_REGION( 0x20000, "fixed", 0 )												\
	ROM_LOAD( name,       0x000000, 0x20000, hash ) 															\
																												\
	ROM_REGION( 0x20000, "fixedbios", 0 )													\
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )		\
																												\
	ROM_Y_ZOOM


#define ROM_Y_ZOOM																								\
	ROM_REGION( 0x20000, "zoomy", 0 )														\
	ROM_LOAD( "000-lo.lo", 0x00000, 0x20000, CRC(5a86cff2) SHA1(5992277debadeb64d1c1c64b0a92d9293eaf7e4a) )


/* AES and MVS cartridge information */

/***************************************************************************

    Neo-Geo game pcb infos:
    =======================

    The Neo-Geo games for AES (home) and MVS (arcade) systems are cartridge based.

    Each cartridge consists of two pcb's: CHA and PROG.

    CHA pcb contains gfx data ('C' - rom), text layer data ('S' - rom) and sound driver ('M' - rom).
    PROG pcb contains sample data ('V' - rom) and program code ('P' - rom).

    On most pcb's various custom/protection chips can also be found:

    CHA:
    . NEO-273
    . NEO-CMC
    . NEO-ZMC
    . NEO-ZMC2
    . PRO-CT0
    . SNK-9201

    PROG:
    . 0103 (QFP144)
    . ALTERA (EPM7128SQC100-15)
    . NEO-COMA
    . NEO-PCM2 (SNK 1999)
    . NEO-PCM2 (PLAYMORE 2002)
    . NEO-PVC
    . NEO-SMA
    . PCM
    . PRO-CT0
    . SNK-9201


    Known pcb's:
    ============

    MVS CHA:
    -- SNK --
    . NEO-MVS CHA-32
    . NEO-MVS CHA-8M
    . NEO-MVS CHA42G
    . NEO-MVS CHA42G-1
    . NEO-MVS CHA 42G-2
    . NEO-MVS CHA 42G-3
    . NEO-MVS CHA42G-3B
    . NEO-MVS CHA256
    . NEO-MVS CHA256B
    . NEO-MVS PSTM CHA136
    . NEO-MVS CHA512Y
    . NEO-MVS CHAFIO (1999.6.14)
    . MVS CHAFIO REV1.0 (KOF-2001)
    . NEO-MVS CHAFIO (SNK 2002) - MADE IN KOREA
    -- SNKPLAYMORE --
    . NEO-MVS CHAFIO (2003.7.24)

    MVS PROG:
    -- SNK --
    . NEO-MVS PROG-NAM
    . NEO-MVS PROG-HERO
    . NEO-MVS PROG-EP
    . NEO-MVS PROG-8MB
    . NEO-MVS PROG8M42
    . NEO-MVS PROG16
    . NEO-MVS PROG42G
    . NEO-MVS PROG42G-COM
    . NEO-MVS PROG42G-1
    . NEO-MVS PROG-G2
    . NEO-MVS PROG 4096
    . NEO-MVS PROG 4096 B
    . NEO-MVS PROGGSC
    . NEO-MVS PROGSM
    . NEO-MVS PROGSS3
    . NEO-MVS PROGTOP
    . NEO-MVS PROGSF1 (1998.6.17)
    . NEO-MVS PROGEOP (1999.2.2)
    . NEO-MVS PROGLBA (1999.4.12) - LBA-SUB (2000.2.24)
    . NEO-MVS PROGBK1
    . NEO-MVS PROGBK2 (2000.3.21)
    . MVS PROGBK2 REV1.0 (KOF-2001)
    . NEO-MVS PROGBK2 (SNK 2002) - MADE IN KOREA
    -- SNKPLAYMORE --
    . NEO-MVS PROGBK2R (2003.8.26) - NEO-HYCS (2003.9.29)
    . NEO-MVS PROGBK3R (2003.9.2) - NEO-HYCS (2003.9.29)
    . NEO-MVS PROGBK3S (2003.10.1)
    . NEO-MVS PROGBK2S (2003.10.18)


    AES CHA:
    -- SNK --
    . NEO-AEG CHA-32
    . NEO-AEG CHA-8M
    . NEO-AEG CHA42G
    . NEO-AEG CHA42G-1
    . NEO-AEG CHA42G-2B
    . NEO-AEG CHA42G-3
    . NEO-AEG CHA42G-4
    . NEO-AEG CHA256
    . NEO-AEG CHA256[B]
    . NEO-AEG CHA256RY
    . NEO-AEG CHA512Y
    . NEO-AEG CHAFIO (1999.8.10)
    -- SNKPLAYMORE --
    . NEO-AEG CHAFIO (2003.7.24)

    AES PROG:
    -- SNK --
    . NEO-AEG PROG-NAM
    . NEO-AEG PROG-HERO
    . NEO-AEG PROG-4A
    . NEO-AEG PROG-4B
    . NEO-AEG PROG 8M42
    . NEO-AEG PROG B
    . NEO-AEG PROG16
    . NEO-AEG PROG42G
    . NEO-AEG PROG42G-COM
    . NEO-AEG PROG42G-1
    . NEO-AEG PROG-G2
    . NEO-AEG PROG4096 B
    . NEO-AEG PROGGS
    . NEO-AEG PROGTOP2
    . NEO-AEG PROGLBA (1999.7.6)
    . NEO-AEG PROGRK
    . NEO-AEG PROGRKB
    . NEO-AEG PROGBK1Y
    . NEO-AEG PROGBK1F
    -- PLAYMORE --
    . NEO-AEG PROGBK2 (2002.4.1)
    -- SNKPLAYMORE --
    . NEO-AEG PROGBK3R (2003.8.29) - NEO-HYCS (2003.9.29)
    . NEO-AEG PROGBK2S (2003.10.16)


    Cartridge colours:
    ==================

    MVS cartridges were produced in different colours.

    Known cartridge colours:
    . Black
    . Blue
    . Green
    . Grey
    . Red
    . Transparent
    . Transparent Blue
    . Transparent Green
    . White
    . Yellow

    The above listed only covers SNK / PLAYMORE / SNKPLAYMORE pcb's. There also exists a
    wide range of 'bootleg' pcb's.


    Neo-Geo game pcb infos by Johnboy

****************************************************************************/

/****************************************************************************

    General set notes:
    ==================

    NOTES BELOW APPLY TO OEM SETS ONLY!

    Many 'M1' roms contain mirrored data (64k mirrored or 128k mirrored).
    Found on several early sets (ID 0001 ~ 0045) and on the last sets (ID 0267 ~ 0272).
    This caused some confusion and incorrect rom sizes.
    Minimum 'M1' size is 1mbit, maximum size 4mbit.
    The remaining 64k 'M1' are marked BAD_DUMP.

    All 'S1' roms found on prom are 1mbit.
    The remainig 64k 'S1' are marked BAD_DUMP.

****************************************************************************/


	/* OEM sets */


/*
    Missing (undumped) sets:
    . Rage of the Dragons (AES)
    . Last Blade 2 (early revisions) ?
    . Power Spikes II (early revisions) ?
    . Metal Slug 3 (rev. Mar/17/2000 1:36)
    . Metal Slug 4 (AES)
    . Metal Slug 4 (MVS, 1st revision)
    . Metal Slug 5 (PCB)
    . Sengoku 3 (alternate revision)

    All rom labels need to be reverified

    SMA protected sets:
    On some SMA protected sets there is a small sub pcb found, LBA-SUB (2000.2.24), which replaces the
    regular used 2x32mbit or 4x16mbit proms. LBA-SUB (2000.2.24) contains 4x16mbit mask proms.
    MVS sets confirmed are garou, mslug3, mslug3a and kof2000.
*/

/*

  About supported sets:

  MVS carts (arcade) were released before the AES carts (home)
  The actual codepath taken depends entirely on the BIOS rom, not the roms in the cartridge, which (with
  a few exceptions) support both codepaths.

  The initial AES releases are therefore later revisions of the game, often with bug fixes over the
  initial MVS releases.  It isn't uncommon for later production runs and bootlegs to use these newer sets,
  so all of them are supported in MAME.

  Likewise, because the MVS carts were released first (and were produced in higher numbers and generally
  have a lower cost) it's not uncommon for AES units to operate with converted MVS carts, so, with the
  exception of the sets that specifically lock out the AES mode* these sets are all equally suitable
  for MESS.

  * nitd, kof2001 (initial release has no AES code), and a number of the hacked bootlegs.

*/

/* dummy entry for the dummy bios driver */
ROM_START( neogeo )
	NEOGEO_BIOS

	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_Y_ZOOM

	ROM_REGION( 0x20000, "fixed", ROMREGION_ERASEFF )

	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION( 0x10000, "ymsnd", ROMREGION_ERASEFF )

	NO_DELTAT_REGION

	ROM_REGION( 0x100000, "sprites", ROMREGION_ERASEFF )
ROM_END

ROM_START( nam1975 ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "001-p1.bin", 0x000000, 0x080000, CRC(cc9fc951) SHA1(92f4e6ddeeb825077d92dbb70b50afea985f15c0) )

	NEO_SFIX_128K( "001-s1.bin", CRC(7988ba51) SHA1(bc2f661f381b06b34ac2fa215dd5689d3bf84832) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "001-m1.bin", CRC(ba874463) SHA1(a83514f4b20301f84a98699900e2593f1c1b8846) )

	ROM_REGION( 0x080000, "ymsnd", 0 )
	ROM_LOAD( "001-v1.bin", 0x000000, 0x080000, CRC(a7c3d5e5) SHA1(e3efc86940f91c53b7724c4566cfc21ea1a7a465) )

	ROM_REGION( 0x180000, "ymsnd.deltat", 0 )
	ROM_LOAD( "001-v2.bin", 0x000000, 0x080000, CRC(55e670b3) SHA1(a047049646a90b6db2d1882264df9256aa5a85e5) )
	ROM_LOAD( "001-v22.bin", 0x080000, 0x080000, CRC(ab0d8368) SHA1(404114db9f3295929080b87a5d0106b40da6223a) )
	ROM_LOAD( "001-v23.bin", 0x100000, 0x080000, CRC(df468e28) SHA1(4e5d4a709a4737a87bba4083aeb788f657862f1a) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "001-c1.bin", 0x000000, 0x80000, CRC(32ea98e1) SHA1(c2fb3fb7dd14523a4b4b7fbdb81f44cb4cc48239) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "001-c2.bin", 0x000001, 0x80000, CRC(cbc4064c) SHA1(224c970fd060d841fd430c946ef609bb57b6d78c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "001-c3.bin", 0x100000, 0x80000, CRC(0151054c) SHA1(f24fb501a7845f64833f4e5a461bcf9dc3262557) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "001-c4.bin", 0x100001, 0x80000, CRC(0a32570d) SHA1(f108446ec7844fde25f7a4ab454f76d384bf5e52) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "001-c5.bin", 0x200000, 0x80000, CRC(90b74cc2) SHA1(89898da36db259180e5261ed45eafc99ca13e504) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "001-c6.bin", 0x200001, 0x80000, CRC(e62bed58) SHA1(d05b2903b212a51ee131e52c761b714cb787683e) ) /* Plane 2,3 */
ROM_END

ROM_START( bstars ) /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "002-pg.p1", 0x000000, 0x080000, CRC(c100b5f5) SHA1(4cea9f29ad67288c3eccfa4cf961ee9782e49165) )

	NEO_SFIX_128K( "002-s1.bin", CRC(1a7fd0c6) SHA1(3fc701b7afddab369ddf9dedfbc5e1aaf80b8af3) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "002-m1.bin", CRC(4ecaa4ee) SHA1(50abfb8eed6cb4887393089f9ccc76f306ef69b5) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "002-v11.bin", 0x000000, 0x080000, CRC(b7b925bd) SHA1(644c92fa90e74998e73714f74b1e0680ee372a07) )
	ROM_LOAD( "002-v12.bin", 0x080000, 0x080000, CRC(329f26fc) SHA1(2c8009edc88c6b26f7be5beb2b8d260aac394ee1) )
	ROM_LOAD( "002-v13.bin", 0x100000, 0x080000, CRC(0c39f3c8) SHA1(db8f8670639601215707d918d4fb93221460446a) )
	ROM_LOAD( "002-v14.bin", 0x180000, 0x080000, CRC(c7e11c38) SHA1(5abf2a7877e0162c758a4dcf09f183930fa7ef24) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "002-v21.bin", 0x000000, 0x080000, CRC(04a733d1) SHA1(84159368c0f6de2c3b8121227201cd3422455cf6) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "002-c1.bin", 0x000000, 0x080000, CRC(aaff2a45) SHA1(c91ee72d1d74514df8ec44fca703409d92158ae3) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "002-c2.bin", 0x000001, 0x080000, CRC(3ba0f7e4) SHA1(f023b134b9c7994f477867307d2732026033501d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "002-c3.bin", 0x100000, 0x080000, CRC(96f0fdfa) SHA1(9f779a1ae46aeda54d69382b074392ade687f62f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "002-c4.bin", 0x100001, 0x080000, CRC(5fd87f2f) SHA1(a5dd6f26f9485f216c2428ae1792c182beb10dbc) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "002-c5.bin", 0x200000, 0x080000, CRC(807ed83b) SHA1(3268e7d4602c3f55f1e0da2c80653d5ae461ef67) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "002-c6.bin", 0x200001, 0x080000, CRC(5a3cad41) SHA1(c620d18f4ff32ed5489c941dfc641030a54f1c14) ) /* Plane 2,3 */
ROM_END

ROM_START( bstarsh ) /* AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "002-p1.bin", 0x000000, 0x080000, CRC(3bc7790e) SHA1(50b2fffb1278151bb4849fbe1f8cb23916019815) )

	NEO_SFIX_128K( "002-s1.bin", CRC(1a7fd0c6) SHA1(3fc701b7afddab369ddf9dedfbc5e1aaf80b8af3) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "002-m1.bin", CRC(4ecaa4ee) SHA1(50abfb8eed6cb4887393089f9ccc76f306ef69b5) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "002-v11.bin", 0x000000, 0x080000, CRC(b7b925bd) SHA1(644c92fa90e74998e73714f74b1e0680ee372a07) )
	ROM_LOAD( "002-v12.bin", 0x080000, 0x080000, CRC(329f26fc) SHA1(2c8009edc88c6b26f7be5beb2b8d260aac394ee1) )
	ROM_LOAD( "002-v13.bin", 0x100000, 0x080000, CRC(0c39f3c8) SHA1(db8f8670639601215707d918d4fb93221460446a) )
	ROM_LOAD( "002-v14.bin", 0x180000, 0x080000, CRC(c7e11c38) SHA1(5abf2a7877e0162c758a4dcf09f183930fa7ef24) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "002-v21.bin", 0x000000, 0x080000, CRC(04a733d1) SHA1(84159368c0f6de2c3b8121227201cd3422455cf6) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "002-c1.bin", 0x000000, 0x080000, CRC(aaff2a45) SHA1(c91ee72d1d74514df8ec44fca703409d92158ae3) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "002-c2.bin", 0x000001, 0x080000, CRC(3ba0f7e4) SHA1(f023b134b9c7994f477867307d2732026033501d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "002-c3.bin", 0x100000, 0x080000, CRC(96f0fdfa) SHA1(9f779a1ae46aeda54d69382b074392ade687f62f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "002-c4.bin", 0x100001, 0x080000, CRC(5fd87f2f) SHA1(a5dd6f26f9485f216c2428ae1792c182beb10dbc) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "002-c5.bin", 0x200000, 0x080000, CRC(807ed83b) SHA1(3268e7d4602c3f55f1e0da2c80653d5ae461ef67) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "002-c6.bin", 0x200001, 0x080000, CRC(5a3cad41) SHA1(c620d18f4ff32ed5489c941dfc641030a54f1c14) ) /* Plane 2,3 */
ROM_END

ROM_START( tpgolf ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "003-p1.bin", 0x000000, 0x080000, CRC(f75549ba) SHA1(3f7bdf5e2964e921fe1dd87c51a79a1a501fc73f) )
	ROM_LOAD16_WORD_SWAP( "003-p2.bin", 0x080000, 0x080000, CRC(b7809a8f) SHA1(1604c889592c9610668bff296de48a0d6906156d) )

	NEO_SFIX_128K( "003-s1.bin", CRC(7b3eb9b1) SHA1(39cd8bad9f8bfdeb8ac681b5b79ae5aa81c8dd5f) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "003-m1.bin", CRC(4cc545e6) SHA1(8e014b8cab3e5b3995756a4ea52ce49c36866377) )

	ROM_REGION( 0x080000, "ymsnd", 0 )
	ROM_LOAD( "003-v1.bin", 0x000000, 0x080000, CRC(ff97f1cb) SHA1(defa249d46ae220f7bfa70746f5202bbbcc3e5fe) )

	ROM_REGION( 0x200000, "ymsnd.deltat", 0 )
	ROM_LOAD( "003-v2.bin", 0x000000, 0x080000, CRC(d34960c6) SHA1(36d5877d5e42aab943f4d693159f4f3ad8b0addc) )
	ROM_LOAD( "003-v22.bin", 0x080000, 0x080000, CRC(9a5f58d4) SHA1(2b580595e1820430a36f06fd3e0e0b8f7d686889) )
	ROM_LOAD( "003-v23.bin", 0x100000, 0x080000, CRC(30f53e54) SHA1(22461f88a56d272b78dbc23204c0c6816200532b) )
	ROM_LOAD( "003-v24.bin", 0x180000, 0x080000, CRC(5ba0f501) SHA1(ca02937a611a2c50c9e4b54f8fd4eaea09259894) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "003-c1.bin", 0x000000, 0x80000, CRC(0315fbaf) SHA1(583c9253219c1026d81ee5e0cf5568683adc2633) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "003-c2.bin", 0x000001, 0x80000, CRC(b4c15d59) SHA1(b0d8ec967f9b8e5216301c10b2d36912abce6515) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "003-c3.bin", 0x100000, 0x80000, CRC(8ce3e8da) SHA1(bc6c49b27d498f75a0d1a8c4d0cca75e140b9efc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "003-c4.bin", 0x100001, 0x80000, CRC(29725969) SHA1(f1407da84919c2b3fe0e8f1fca65934b147c86c7) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "003-c5.bin", 0x200000, 0x80000, CRC(9a7146da) SHA1(2fc83d13e3e9565919aab01bf2a1b028f433b547) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "003-c6.bin", 0x200001, 0x80000, CRC(1e63411a) SHA1(ee397e2f679042e87b37d95837af62bb95a72af9) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "003-c7.bin", 0x300000, 0x80000, CRC(2886710c) SHA1(1533dd935f0a8f92a0a3c47d1d2bc6d035454244) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "003-c8.bin", 0x300001, 0x80000, CRC(422af22d) SHA1(f67c844c34545de6ea187f5bfdf440dec8518532) ) /* Plane 2,3 */
ROM_END

ROM_START( mahretsu )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "004-p1.bin", 0x000000, 0x080000, CRC(fc6f53db) SHA1(64a62ca4c8fb68954e06121399c9402278bd0467) )

	NEO_SFIX_128K( "004-s1.bin", CRC(4e310702) SHA1(edb0a27fe51d5e7f5739715b7c8872b6d4459b06) )

	NEO_BIOS_AUDIO_128K( "004-m1.bin", CRC(c71fbb3b) SHA1(59c58665b53da61352359d191a0569de5dd1f4b3) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "004-v11.bin", 0x000000, 0x080000, CRC(b2fb2153) SHA1(36e0cc8927b11105de40188af46f6cf532794c10) )
	ROM_LOAD( "004-v12.bin", 0x080000, 0x080000, CRC(8503317b) SHA1(ab22f1aba1e977ab234a4f1d73dc6ed789dbeb85) )

	ROM_REGION( 0x180000, "ymsnd.deltat", 0 )
	ROM_LOAD( "004-v21.bin", 0x000000, 0x080000, CRC(4999fb27) SHA1(2d4926a220ea21bdd5e816bb16f985fff089500a) )
	ROM_LOAD( "004-v22.bin", 0x080000, 0x080000, CRC(776fa2a2) SHA1(e7d5a362ab7806b7b009700a435c815a20e8ec68) )
	ROM_LOAD( "004-v23.bin", 0x100000, 0x080000, CRC(b3e7eeea) SHA1(4d1e97f380702a3a06e7f954b4caddd9c4119d8f) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "004-c1.bin", 0x000000, 0x80000, CRC(f1ae16bc) SHA1(df68feed4dcba1e1566032b01ebb7b478a1792bf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "004-c2.bin", 0x000001, 0x80000, CRC(bdc13520) SHA1(2bc4c996d019a4c539f6c3188ef18089e54b7efa) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "004-c3.bin", 0x100000, 0x80000, CRC(9c571a37) SHA1(21388aeb92bb8e15a55a063701ca9df79e292127) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "004-c4.bin", 0x100001, 0x80000, CRC(7e81cb29) SHA1(5036f04df30cf6903bd1a8cc06ff6f015c24a74b) ) /* Plane 2,3 */
ROM_END

ROM_START( maglord )  /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "005-pg1.bin", 0x000000, 0x080000, CRC(599043c5) SHA1(43f234b0f89b72b4c6050c40d9daa5c4e96b94ce) )

	NEO_SFIX_128K( "005-s1.bin", CRC(1c5369a2) SHA1(db0dba0a7dced6c9ca929c5abda491b05d84199c) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "005-m1.bin", CRC(26259f0f) SHA1(4f3e500093d61585048767dbd9fa09b3911a05d6) )

	ROM_REGION( 0x080000, "ymsnd", 0 )
	ROM_LOAD( "005-v11.bin", 0x000000, 0x080000, CRC(cc0455fd) SHA1(a8ff4270e7705e263d25ff0b301f503bccea7e59) )

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 )
	ROM_LOAD( "005-v21.bin", 0x000000, 0x080000, CRC(f94ab5b7) SHA1(2c16985102e3585e08622d8c54ac5c60425b9ff8) )
	ROM_LOAD( "005-v22.bin", 0x080000, 0x080000, CRC(232cfd04) SHA1(61b66a9decbbd1f500a8c186615e7fd077c6861e) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "005-c1.bin", 0x000000, 0x80000, CRC(806aee34) SHA1(3c32a0edbbddb694495b510c13979c44b83de8bc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "005-c2.bin", 0x000001, 0x80000, CRC(34aa9a86) SHA1(cec97e1ff7f91158040c629ba75742db82c4ae5e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "005-c3.bin", 0x100000, 0x80000, CRC(c4c2b926) SHA1(478bfafca21f5a1338808251a06ab405e6a9e65f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "005-c4.bin", 0x100001, 0x80000, CRC(9c46dcf4) SHA1(4c05f3dc25777a87578ce09a6cefb3a4cebf3266) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "005-c5.bin", 0x200000, 0x80000, CRC(69086dec) SHA1(7fa47f4a765948813ebf366168275dcc3c42e951) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "005-c6.bin", 0x200001, 0x80000, CRC(ab7ac142) SHA1(e6ad2843947d35d8e913d2666f87946c1ba7944f) ) /* Plane 2,3 */
ROM_END

ROM_START( maglordh ) /* AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "005-p1.bin", 0x000000, 0x080000, CRC(bd0a492d) SHA1(d043d3710cf2b0d2b3798008e65e4c7c3ead1af3) )

	NEO_SFIX_128K( "005-s1.bin", CRC(1c5369a2) SHA1(db0dba0a7dced6c9ca929c5abda491b05d84199c) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "005-m1.bin", CRC(26259f0f) SHA1(4f3e500093d61585048767dbd9fa09b3911a05d6) )

	ROM_REGION( 0x080000, "ymsnd", 0 )
	ROM_LOAD( "005-v11.bin", 0x000000, 0x080000, CRC(cc0455fd) SHA1(a8ff4270e7705e263d25ff0b301f503bccea7e59) )

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 )
	ROM_LOAD( "005-v21.bin", 0x000000, 0x080000, CRC(f94ab5b7) SHA1(2c16985102e3585e08622d8c54ac5c60425b9ff8) )
	ROM_LOAD( "005-v22.bin", 0x080000, 0x080000, CRC(232cfd04) SHA1(61b66a9decbbd1f500a8c186615e7fd077c6861e) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "005-c1.bin", 0x000000, 0x80000, CRC(806aee34) SHA1(3c32a0edbbddb694495b510c13979c44b83de8bc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "005-c2.bin", 0x000001, 0x80000, CRC(34aa9a86) SHA1(cec97e1ff7f91158040c629ba75742db82c4ae5e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "005-c3.bin", 0x100000, 0x80000, CRC(c4c2b926) SHA1(478bfafca21f5a1338808251a06ab405e6a9e65f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "005-c4.bin", 0x100001, 0x80000, CRC(9c46dcf4) SHA1(4c05f3dc25777a87578ce09a6cefb3a4cebf3266) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "005-c5.bin", 0x200000, 0x80000, CRC(69086dec) SHA1(7fa47f4a765948813ebf366168275dcc3c42e951) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "005-c6.bin", 0x200001, 0x80000, CRC(ab7ac142) SHA1(e6ad2843947d35d8e913d2666f87946c1ba7944f) ) /* Plane 2,3 */
ROM_END

ROM_START( ridhero )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "006-p1.bin", 0x000000, 0x080000, CRC(d4aaf597) SHA1(34d35b71adb5bd06f4f1b50ffd9c58ab9c440a84) )

	NEO_SFIX_128K( "006-s1.bin", CRC(eb5189f0) SHA1(0239c342ea62e73140a2306052f226226461a478) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "006-m1.bin", CRC(92e7b4fe) SHA1(d240056cd632f92bdfaa5e0492c09aa9bd7b0471) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "006-v11.bin", 0x000000, 0x080000, CRC(cdf74a42) SHA1(a17106cc3f9e5c5d52b4def861c0545a98151da2) )
	ROM_LOAD( "006-v12.bin", 0x080000, 0x080000, CRC(e2fd2371) SHA1(cc95297bee7ffbdcb24ac4daeb5307cb39a52067) )

	ROM_REGION( 0x200000, "ymsnd.deltat", 0 )
	ROM_LOAD( "006-v21.bin", 0x000000, 0x080000, CRC(94092bce) SHA1(1a2906271fe6bc396898a756153629a5862930eb) )
	ROM_LOAD( "006-v22.bin", 0x080000, 0x080000, CRC(4e2cd7c3) SHA1(72fb215a4f208a22a764e801186d1643d3d840ca) )
	ROM_LOAD( "006-v23.bin", 0x100000, 0x080000, CRC(069c71ed) SHA1(f450e9f60cd6ef846dbc77993159ec6157fb64e7) )
	ROM_LOAD( "006-v24.bin", 0x180000, 0x080000, CRC(89fbb825) SHA1(656a97c6a8832dab3a5e1577d9cd257b561cc356) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "006-c1.bin", 0x000000, 0x080000, CRC(4a5c7f78) SHA1(f8f1e6b7841c74368210d52a84307bb28f722a2d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "006-c2.bin", 0x000001, 0x080000, CRC(e0b70ece) SHA1(e2b750e43cdddcea29d1c9c943a3628117a16a1b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "006-c3.bin", 0x100000, 0x080000, CRC(8acff765) SHA1(11fe89b9d112d0658c9ddf40d928584de6ea9202) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "006-c4.bin", 0x100001, 0x080000, CRC(205e3208) SHA1(aa2acf2c6f48ffffdcc0c94ddc031acc9e4a2e68) ) /* Plane 2,3 */
ROM_END

ROM_START( ridheroh )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "006-p1h.bin", 0x000000, 0x080000, CRC(52445646) SHA1(647bb31f2f68453c1366cb6e2e867e37d1df7a54) )

	NEO_SFIX_128K( "006-s1.bin", CRC(eb5189f0) SHA1(0239c342ea62e73140a2306052f226226461a478) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "006-m1.bin", CRC(92e7b4fe) SHA1(d240056cd632f92bdfaa5e0492c09aa9bd7b0471) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "006-v11.bin", 0x000000, 0x080000, CRC(cdf74a42) SHA1(a17106cc3f9e5c5d52b4def861c0545a98151da2) )
	ROM_LOAD( "006-v12.bin", 0x080000, 0x080000, CRC(e2fd2371) SHA1(cc95297bee7ffbdcb24ac4daeb5307cb39a52067) )

	ROM_REGION( 0x200000, "ymsnd.deltat", 0 )
	ROM_LOAD( "006-v21.bin", 0x000000, 0x080000, CRC(94092bce) SHA1(1a2906271fe6bc396898a756153629a5862930eb) )
	ROM_LOAD( "006-v22.bin", 0x080000, 0x080000, CRC(4e2cd7c3) SHA1(72fb215a4f208a22a764e801186d1643d3d840ca) )
	ROM_LOAD( "006-v23.bin", 0x100000, 0x080000, CRC(069c71ed) SHA1(f450e9f60cd6ef846dbc77993159ec6157fb64e7) )
	ROM_LOAD( "006-v24.bin", 0x180000, 0x080000, CRC(89fbb825) SHA1(656a97c6a8832dab3a5e1577d9cd257b561cc356) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "006-c1.bin", 0x000000, 0x080000, CRC(4a5c7f78) SHA1(f8f1e6b7841c74368210d52a84307bb28f722a2d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "006-c2.bin", 0x000001, 0x080000, CRC(e0b70ece) SHA1(e2b750e43cdddcea29d1c9c943a3628117a16a1b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "006-c3.bin", 0x100000, 0x080000, CRC(8acff765) SHA1(11fe89b9d112d0658c9ddf40d928584de6ea9202) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "006-c4.bin", 0x100001, 0x080000, CRC(205e3208) SHA1(aa2acf2c6f48ffffdcc0c94ddc031acc9e4a2e68) ) /* Plane 2,3 */
ROM_END

ROM_START( alpham2 ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "007-p1.bin", 0x000000, 0x080000, CRC(5b266f47) SHA1(8afbf995989f47ad93fea1f31a884afc7228b53a) )
	ROM_LOAD16_WORD_SWAP( "007-p2.bin", 0x080000, 0x020000, CRC(eb9c1044) SHA1(65d3416dcd96663bc4e7cefe90ecb7c1eafb2dda) )

	NEO_SFIX_128K( "007-s1.bin", CRC(85ec9acf) SHA1(39a11974438ad36a2cc84307151b31474c3c5518) )

	NEO_BIOS_AUDIO_128K( "007-m1.bin", CRC(28dfe2cd) SHA1(1a1a99fb917c6c8db591e3be695ce03f843ee1df) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "007-v1.bin", 0x000000, 0x100000, CRC(cd5db931) SHA1(b59f9f2df29f49470312a6cd20f5669b6aaf51ff) )
	ROM_LOAD( "007-v2.bin", 0x100000, 0x100000, CRC(63e9b574) SHA1(1ade4cd0b15c84dd4a0fb7f7abf0885eef3a3f71) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "007-c1.bin", 0x000000, 0x100000, CRC(8fba8ff3) SHA1(1a682292e99eb91b0edb9771c44bc5e762867e98) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "007-c2.bin", 0x000001, 0x100000, CRC(4dad2945) SHA1(ac85a146276537fed124bda892bb93ff549f1d93) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "007-c3.bin", 0x200000, 0x080000, CRC(68c2994e) SHA1(4f8dfc6e5188942e03b853a2c9f0ea6138dec791) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "007-c4.bin", 0x200001, 0x080000, CRC(7d588349) SHA1(a5ed789d7bbc25be5c5b2d99883b64d379c103a2) ) /* Plane 2,3 */
ROM_END

ROM_START( ncombat )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "009-p1.bin", 0x000000, 0x080000, CRC(b45fcfbf) SHA1(3872147dda2d1ba905d35f4571065d87b1958b4a) )

	NEO_SFIX_128K( "009-s1.bin", CRC(d49afee8) SHA1(77615f12edf08ae8f1353f7a056a8f3a50d3ebdc) )

	NEO_BIOS_AUDIO_128K( "009-m1.bin", CRC(b5819863) SHA1(6f2309d51531052dbf7d712993c9e35649db0d84) )

	ROM_REGION( 0x180000, "ymsnd", 0 )
	ROM_LOAD( "009-v11.bin", 0x000000, 0x080000, CRC(cf32a59c) SHA1(af5b7fcd8a4aff1307c0a1d937e5f0460c32de79) )
	ROM_LOAD( "009-v12.bin", 0x080000, 0x080000, CRC(7b3588b7) SHA1(a4e6d9d4113ff4ce48b371f65e9187d551821d3b) )
	ROM_LOAD( "009-v13.bin", 0x100000, 0x080000, CRC(505a01b5) SHA1(9426a4f5b31e16f74e72e61951c189a878f211c5) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "009-v21.bin", 0x000000, 0x080000, CRC(365f9011) SHA1(aebd292214ab280b05ee9e759b7e9a681a099c4a) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "009-c1.bin", 0x000000, 0x80000, CRC(33cc838e) SHA1(c445c891c0ba4190aa0b472786150620e76df5b4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "009-c2.bin", 0x000001, 0x80000, CRC(26877feb) SHA1(8f48097fb8e4757f50b6d86219122fbf4b6f87ef) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "009-c3.bin", 0x100000, 0x80000, CRC(3b60a05d) SHA1(0a165a17af4834876fcd634599cd2208adc9248f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "009-c4.bin", 0x100001, 0x80000, CRC(39c2d039) SHA1(8ca6c3f977c43c7abba2a00a0e70f02e2a49f5f2) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "009-c5.bin", 0x200000, 0x80000, CRC(67a4344e) SHA1(b325f152c7b2388fc92c5826e1dc99094b9ea749) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "009-c6.bin", 0x200001, 0x80000, CRC(2eca8b19) SHA1(16764ef10e404325ba0a1a2ad3a4c0af287be21f) ) /* Plane 2,3 */
ROM_END

ROM_START( ncombath ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "009-pg1.bin", 0x000000, 0x080000, CRC(8e9f0add) SHA1(d0b908a86a58f2537eea73a431038f1cd74a5a2f) )

	NEO_SFIX_128K( "009-s1.bin", CRC(d49afee8) SHA1(77615f12edf08ae8f1353f7a056a8f3a50d3ebdc) )

	NEO_BIOS_AUDIO_128K( "009-m1.bin", CRC(b5819863) SHA1(6f2309d51531052dbf7d712993c9e35649db0d84) )

	ROM_REGION( 0x180000, "ymsnd", 0 )
	ROM_LOAD( "009-v11.bin", 0x000000, 0x080000, CRC(cf32a59c) SHA1(af5b7fcd8a4aff1307c0a1d937e5f0460c32de79) )
	ROM_LOAD( "009-v12.bin", 0x080000, 0x080000, CRC(7b3588b7) SHA1(a4e6d9d4113ff4ce48b371f65e9187d551821d3b) )
	ROM_LOAD( "009-v13.bin", 0x100000, 0x080000, CRC(505a01b5) SHA1(9426a4f5b31e16f74e72e61951c189a878f211c5) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "009-v21.bin", 0x000000, 0x080000, CRC(365f9011) SHA1(aebd292214ab280b05ee9e759b7e9a681a099c4a) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "009-c1.bin", 0x000000, 0x80000, CRC(33cc838e) SHA1(c445c891c0ba4190aa0b472786150620e76df5b4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "009-c2.bin", 0x000001, 0x80000, CRC(26877feb) SHA1(8f48097fb8e4757f50b6d86219122fbf4b6f87ef) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "009-c3.bin", 0x100000, 0x80000, CRC(3b60a05d) SHA1(0a165a17af4834876fcd634599cd2208adc9248f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "009-c4.bin", 0x100001, 0x80000, CRC(39c2d039) SHA1(8ca6c3f977c43c7abba2a00a0e70f02e2a49f5f2) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "009-c5.bin", 0x200000, 0x80000, CRC(67a4344e) SHA1(b325f152c7b2388fc92c5826e1dc99094b9ea749) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "009-c6.bin", 0x200001, 0x80000, CRC(2eca8b19) SHA1(16764ef10e404325ba0a1a2ad3a4c0af287be21f) ) /* Plane 2,3 */
ROM_END

ROM_START( cyberlip )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "010-p1.bin", 0x000000, 0x080000, CRC(69a6b42d) SHA1(6e7cb089de83f1d22cc4a87db5b1a94bf76fb1e8) )

	NEO_SFIX_128K( "010-s1.bin", CRC(79a35264) SHA1(c2819a82adbe1f5e489496e0e03477863a5b7665) )

	NEO_BIOS_AUDIO_128K( "010-m1.bin", CRC(8be3a078) SHA1(054ec6a061fcc88df1ecbb0a01611a31f37a7709) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "010-v11.bin", 0x000000, 0x080000, CRC(90224d22) SHA1(5443ee6f90d80d43194cb4b4f0e08851a59e7784) )
	ROM_LOAD( "010-v12.bin", 0x080000, 0x080000, CRC(a0cf1834) SHA1(8df57a7941bdae7e446a6056039adb012cdde246) )
	ROM_LOAD( "010-v13.bin", 0x100000, 0x080000, CRC(ae38bc84) SHA1(c0937b4f89b8b26c8a0e747b234f44ad6a3bf2ba) )
	ROM_LOAD( "010-v14.bin", 0x180000, 0x080000, CRC(70899bd2) SHA1(8cf01144f0bcf59f09777175ae6b71846b09f3a1) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "010-v21.bin", 0x000000, 0x080000, CRC(586f4cb2) SHA1(588460031d84c308e3353ecf714db9986425c21c) )

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "010-c1.bin", 0x000000, 0x80000, CRC(8bba5113) SHA1(70f0926409ab265da4b8632500d1d32d63cf77cf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "010-c2.bin", 0x000001, 0x80000, CRC(cbf66432) SHA1(cc529640c475d08330e116ea9c5e5a28b7cd13db) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "010-c3.bin", 0x100000, 0x80000, CRC(e4f86efc) SHA1(fa60863d8a7ed4f21d30f91eb1936d0b8329db7a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "010-c4.bin", 0x100001, 0x80000, CRC(f7be4674) SHA1(b4ad0432d4bb6d5a98e27015910343c964b73ed4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "010-c5.bin", 0x200000, 0x80000, CRC(e8076da0) SHA1(3ec5cc19809dea688041a42b32c13d257576f3da) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "010-c6.bin", 0x200001, 0x80000, CRC(c495c567) SHA1(2f58475fbb5f1adafce027d396fb05dd71e8fb55) ) /* Plane 2,3 */
ROM_END

ROM_START( superspy ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "011-p1.bin",  0x000000, 0x080000, CRC(c7f944b5) SHA1(da7560e09187c68f1d9f7656218497b4464c56c9) )
	ROM_LOAD16_WORD_SWAP( "011-sp2.bin", 0x080000, 0x020000, CRC(811a4faf) SHA1(8169dfaf79f52d80ecec402ce1b1ab9cafb7ebdd) )

	NEO_SFIX_128K( "011-s1.bin", CRC(ec5fdb96) SHA1(8003028025ac7bf531e568add6ba66c02d0b7e84) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "011-m1.bin", CRC(ca661f1b) SHA1(4e3cb57db716ec48487c1b070c3a55a5faf40856) )

	ROM_REGION( 0x180000, "ymsnd", 0 )
	ROM_LOAD( "011-v11.bin", 0x000000, 0x100000, CRC(5c674d5c) SHA1(d7b9beddeb247b584cea9ca6c43ec6869809b673) )
	ROM_LOAD( "011-v12.bin", 0x100000, 0x080000, CRC(9f513d5a) SHA1(37b04962f0b8e2a74abd35c407337a6151dc4e95) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "011-v21.bin", 0x000000, 0x080000, CRC(426cd040) SHA1(b2b45189837c8287223c2b8bd4df9525b72a3f16) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "011-c1.bin", 0x000000, 0x100000, CRC(cae7be57) SHA1(43b35b349594535689c358d9f324adda55e5281a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "011-c2.bin", 0x000001, 0x100000, CRC(9e29d986) SHA1(b417763bad1acf76116cd56f4203c2d2677e22e5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "011-c3.bin", 0x200000, 0x100000, CRC(14832ff2) SHA1(1179792d773d97d5e45e7d8f009051d362d72e24) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "011-c4.bin", 0x200001, 0x100000, CRC(b7f63162) SHA1(077a81b2bb0a8f17c9df6945078608f74432877a) ) /* Plane 2,3 */
ROM_END

ROM_START( mutnat ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "014-p1.bin", 0x000000, 0x080000, CRC(6f1699c8) SHA1(87206f67a619dede7959230f9ff3701b8b78957a) )

	NEO_SFIX_128K( "014-s1.bin", CRC(99419733) SHA1(b2524af8704941acc72282aa1d62fd4c93e3e822) )

	/* CXK381003A */
	NEO_BIOS_AUDIO_128K( "014-m1.bin", CRC(b6683092) SHA1(623ec7ec2915fb077bf65b4a16c815e071c25259) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "014-v1.bin", 0x000000, 0x100000, CRC(25419296) SHA1(c9fc04987c4e0875d276e1a0fb671740b6f548ad) )
	ROM_LOAD( "014-v2.bin", 0x100000, 0x100000, CRC(0de53d5e) SHA1(467f6040da3dfb1974785e95e14c3f608a93720a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "014-c1.bin", 0x000000, 0x100000, CRC(5e4381bf) SHA1(d429a5e09dafd2fb99495658b3652eecbf58f91b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "014-c2.bin", 0x000001, 0x100000, CRC(69ba4e18) SHA1(b3369190c47771a790c7adffa958ff55d90e758b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "014-c3.bin", 0x200000, 0x100000, CRC(890327d5) SHA1(47f97bf120a8480758e1f3bb8982be4c5325c036) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "014-c4.bin", 0x200001, 0x100000, CRC(e4002651) SHA1(17e53a5f4708866a120415bf24f3b89621ad0bcc) ) /* Plane 2,3 */
ROM_END

ROM_START( kotm )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "016-p1.bin", 0x000000, 0x080000, CRC(1b818731) SHA1(b98b1b33c0301fd79aac908f6b635dd00d1cb08d) )
	ROM_LOAD16_WORD_SWAP( "016-p2.bin", 0x080000, 0x020000, CRC(12afdc2b) SHA1(3a95f5910cbb9f17e63ddece995c6e120fa2f622) )

	NEO_SFIX_128K( "016-s1.bin", CRC(1a2eeeb3) SHA1(8d2b96d395020197bc59294b6b0c8d62b1d8d4dd) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "016-m1.bin", CRC(9da9ca10) SHA1(88b915827d529f39c365d3e41197d5461e07a085) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "016-v1.bin", 0x000000, 0x100000, CRC(86c0a502) SHA1(7fe2db0c64aefdd14d6c36f7fcd6442591e9a014) )
	ROM_LOAD( "016-v2.bin", 0x100000, 0x100000, CRC(5bc23ec5) SHA1(f4ff5d20587469daa026d5c812739335ce53cfdf) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "016-c1.bin", 0x000000, 0x100000, CRC(71471c25) SHA1(bc8e3fee56b33ef2bac5b4b852339d2fbcd09b7c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "016-c2.bin", 0x000001, 0x100000, CRC(320db048) SHA1(d6b43834de6f5442e23ca8fb26b3a36e96790d8d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "016-c3.bin", 0x200000, 0x100000, CRC(98de7995) SHA1(e33edf4d36c82196d2b474e37be180a05976f558) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "016-c4.bin", 0x200001, 0x100000, CRC(070506e2) SHA1(3a2ec365e1d87a9c5ce1ee9bea88402a8eef4ed7) ) /* Plane 2,3 */
ROM_END

ROM_START( kotmh ) /* AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "016-hp1.bin",0x000000, 0x080000, CRC(b774621e) SHA1(7684b2e07163aec68cd083ef1d8900f855f6cb42) )
	ROM_LOAD16_WORD_SWAP( "016-p2.bin", 0x080000, 0x020000, CRC(12afdc2b) SHA1(3a95f5910cbb9f17e63ddece995c6e120fa2f622) )

	NEO_SFIX_128K( "016-s1.bin", CRC(1a2eeeb3) SHA1(8d2b96d395020197bc59294b6b0c8d62b1d8d4dd) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "016-m1.bin", CRC(9da9ca10) SHA1(88b915827d529f39c365d3e41197d5461e07a085) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "016-v1.bin", 0x000000, 0x100000, CRC(86c0a502) SHA1(7fe2db0c64aefdd14d6c36f7fcd6442591e9a014) )
	ROM_LOAD( "016-v2.bin", 0x100000, 0x100000, CRC(5bc23ec5) SHA1(f4ff5d20587469daa026d5c812739335ce53cfdf) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "016-c1.bin", 0x000000, 0x100000, CRC(71471c25) SHA1(bc8e3fee56b33ef2bac5b4b852339d2fbcd09b7c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "016-c2.bin", 0x000001, 0x100000, CRC(320db048) SHA1(d6b43834de6f5442e23ca8fb26b3a36e96790d8d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "016-c3.bin", 0x200000, 0x100000, CRC(98de7995) SHA1(e33edf4d36c82196d2b474e37be180a05976f558) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "016-c4.bin", 0x200001, 0x100000, CRC(070506e2) SHA1(3a2ec365e1d87a9c5ce1ee9bea88402a8eef4ed7) ) /* Plane 2,3 */
ROM_END

ROM_START( sengoku ) /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "017-p1.bin", 0x000000, 0x080000, CRC(f8a63983) SHA1(7a10ecb2f0fd8315641374c065d2602107b09e72) )
	ROM_LOAD16_WORD_SWAP( "017-p2.bin", 0x080000, 0x020000, CRC(3024bbb3) SHA1(88892e1292dd60f35a76f9a22e623d4f0f9693cc) )

	NEO_SFIX_128K( "017-s1.bin", CRC(b246204d) SHA1(73dce64c61fb5bb7e836a8e60f081bb77d80d281) )

	NEO_BIOS_AUDIO_128K( "017-m1.bin", CRC(9b4f34c6) SHA1(7f3a51f47fcbaa598f5c76bc66e2c53c8dfd852d) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "017-v1.bin", 0x000000, 0x100000, CRC(23663295) SHA1(9374a5d9f3de8e6a97c11f07d8b4485ac9d55edb) )
	ROM_LOAD( "017-v2.bin", 0x100000, 0x100000, CRC(f61e6765) SHA1(1c9b287996947319eb3d288c3d82932cf01039db) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "017-c1.bin", 0x000000, 0x100000, CRC(b4eb82a1) SHA1(79879e2ea78c07d04c88dc9a1ad59604b7a078be) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "017-c2.bin", 0x000001, 0x100000, CRC(d55c550d) SHA1(6110f693aa23710939c04153cf5af26493e4a03f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "017-c3.bin", 0x200000, 0x100000, CRC(ed51ef65) SHA1(e8a8d86e24454948e51a75c883bc6e4091cbf820) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "017-c4.bin", 0x200001, 0x100000, CRC(f4f3c9cb) SHA1(8faafa89dbd0345218f71f891419d2e4e7578200) ) /* Plane 2,3 */
ROM_END

ROM_START( sengokh ) /* AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "017-hp1.bin", 0x000000, 0x080000, CRC(33eccae0) SHA1(000ccf9a9c73df75eeba3f2c367c3a1a9e0a3a6b) )
	ROM_LOAD16_WORD_SWAP( "017-p2.bin",  0x080000, 0x020000, CRC(3024bbb3) SHA1(88892e1292dd60f35a76f9a22e623d4f0f9693cc) )

	NEO_SFIX_128K( "017-s1.bin", CRC(b246204d) SHA1(73dce64c61fb5bb7e836a8e60f081bb77d80d281) )

	NEO_BIOS_AUDIO_128K( "017-m1.bin", CRC(9b4f34c6) SHA1(7f3a51f47fcbaa598f5c76bc66e2c53c8dfd852d) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "017-v1.bin", 0x000000, 0x100000, CRC(23663295) SHA1(9374a5d9f3de8e6a97c11f07d8b4485ac9d55edb) )
	ROM_LOAD( "017-v2.bin", 0x100000, 0x100000, CRC(f61e6765) SHA1(1c9b287996947319eb3d288c3d82932cf01039db) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "017-c1.bin", 0x000000, 0x100000, CRC(b4eb82a1) SHA1(79879e2ea78c07d04c88dc9a1ad59604b7a078be) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "017-c2.bin", 0x000001, 0x100000, CRC(d55c550d) SHA1(6110f693aa23710939c04153cf5af26493e4a03f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "017-c3.bin", 0x200000, 0x100000, CRC(ed51ef65) SHA1(e8a8d86e24454948e51a75c883bc6e4091cbf820) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "017-c4.bin", 0x200001, 0x100000, CRC(f4f3c9cb) SHA1(8faafa89dbd0345218f71f891419d2e4e7578200) ) /* Plane 2,3 */
ROM_END

ROM_START( burningf ) /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "018-p1.bin", 0x000000, 0x080000, CRC(4092c8db) SHA1(df194a4ad2c35e0e18bc053ff9284183444a4666) )

	NEO_SFIX_128K( "018-s1.bin", CRC(6799ea0d) SHA1(ec75ef9dfdcb0b123574fc6d81ebaaadfba32fb5) )

	/* HN62321A */
	NEO_BIOS_AUDIO_128K( "018-m1.bin", CRC(0c939ee2) SHA1(57d580d3279e66b9fe66bbcc68529d3384a926ff) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "018-v1.bin", 0x000000, 0x100000, CRC(508c9ffc) SHA1(cd3a97a233a4585f8507116aba85884623cccdc4) )
	ROM_LOAD( "018-v2.bin", 0x100000, 0x100000, CRC(854ef277) SHA1(4b3083b9c80620064cb44e812a787a700e32a6f3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "018-c1.bin", 0x000000, 0x100000, CRC(25a25e9b) SHA1(3cf02d0662e190678d0530d7b7d3f425209adf83) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "018-c2.bin", 0x000001, 0x100000, CRC(d4378876) SHA1(45659aa1755d96b992c977042186e47fff68bba9) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "018-c3.bin", 0x200000, 0x100000, CRC(862b60da) SHA1(e2303eb1609f1050f0b4f46693a15e37deb176fb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "018-c4.bin", 0x200001, 0x100000, CRC(e2e0aff7) SHA1(1c691c092a6e2787de4f433b0eb9252bfdaa7e16) ) /* Plane 2,3 */
ROM_END

ROM_START( burningfh )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "018-p1h.bin", 0x000000, 0x080000, CRC(ddffcbf4) SHA1(c646c4bbdb4e9b32df76c90f582ccd69fcc9f8e7) )

	NEO_SFIX_128K( "018-s1.bin", CRC(6799ea0d) SHA1(ec75ef9dfdcb0b123574fc6d81ebaaadfba32fb5) )

	/* HN62321A */
	NEO_BIOS_AUDIO_128K( "018-m1.bin", CRC(0c939ee2) SHA1(57d580d3279e66b9fe66bbcc68529d3384a926ff) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "018-v1.bin", 0x000000, 0x100000, CRC(508c9ffc) SHA1(cd3a97a233a4585f8507116aba85884623cccdc4) )
	ROM_LOAD( "018-v2.bin", 0x100000, 0x100000, CRC(854ef277) SHA1(4b3083b9c80620064cb44e812a787a700e32a6f3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "018-c1.bin", 0x000000, 0x100000, CRC(25a25e9b) SHA1(3cf02d0662e190678d0530d7b7d3f425209adf83) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "018-c2.bin", 0x000001, 0x100000, CRC(d4378876) SHA1(45659aa1755d96b992c977042186e47fff68bba9) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "018-c3.bin", 0x200000, 0x100000, CRC(862b60da) SHA1(e2303eb1609f1050f0b4f46693a15e37deb176fb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "018-c4.bin", 0x200001, 0x100000, CRC(e2e0aff7) SHA1(1c691c092a6e2787de4f433b0eb9252bfdaa7e16) ) /* Plane 2,3 */
ROM_END

ROM_START( lbowling ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "019-p1.bin", 0x000000, 0x080000, CRC(a2de8445) SHA1(893d7ae72b4644123469de143fa35fac1cbcd61e) )

	NEO_SFIX_128K( "019-s1.bin", CRC(5fcdc0ed) SHA1(86415077e7adc3ba6153eeb4fb0c62cf36e903fa) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "019-m1.bin", CRC(d568c17d) SHA1(a2e318ed6ad1809c79f3f0853d75e0dd1a2f275c) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "019-v11.bin", 0x000000, 0x080000, CRC(0fb74872) SHA1(38c555926c77576d63472bc075210c42e9ce13a3) )
	ROM_LOAD( "019-v12.bin", 0x080000, 0x080000, CRC(029faa57) SHA1(7bbaa87e38929ab1e32df5f6a2ec0fd5001e7cdb) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "019-v21.bin", 0x000000, 0x080000, CRC(2efd5ada) SHA1(8ba70f5f665d566824333075227d9bce1253b8d8) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "019-c1.bin", 0x000000, 0x080000, CRC(4ccdef18) SHA1(5011e30ec235d0b0a5a513a11d4275777e61acdb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "019-c2.bin", 0x000001, 0x080000, CRC(d4dd0802) SHA1(82069752028c118d42384a95befde45844f0f247) ) /* Plane 2,3 */
ROM_END

ROM_START( gpilots ) /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "020-p1.bin", 0x000000, 0x080000, CRC(e6f2fe64) SHA1(50ab82517e077727d97668a4df2b9b96d2e78ab6) )
	ROM_LOAD16_WORD_SWAP( "020-p2.bin", 0x080000, 0x020000, CRC(edcb22ac) SHA1(505d2db38ae999b7d436e8f2ff56b81796d62b54) )

	NEO_SFIX_128K( "020-s1.bin", CRC(a6d83d53) SHA1(9a8c092f89521cc0b27a385aa72e29cbaca926c5) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "020-m1.bin", CRC(48409377) SHA1(0e212d2c76856a90b2c2fdff675239525972ac43) )

	ROM_REGION( 0x180000, "ymsnd", 0 )
	ROM_LOAD( "020-v11.bin", 0x000000, 0x100000, CRC(1b526c8b) SHA1(2801868d2badcf8aaf5d490e010e4049d81d7bc1) )
	ROM_LOAD( "020-v12.bin", 0x100000, 0x080000, CRC(4a9e6f03) SHA1(d3ac11f333b03d8a318921bdaefb14598e289a14) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "020-v21.bin", 0x000000, 0x080000, CRC(7abf113d) SHA1(5b2a0e70f2eaf4638b44702dacd4cb17838fb1d5) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "020-c1.bin", 0x000000, 0x100000, CRC(bd6fe78e) SHA1(50b704862cd79d64fa488e621b079f6e413c33bc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "020-c2.bin", 0x000001, 0x100000, CRC(5f4a925c) SHA1(71c5ef8141234daaa7025427a6c65e79766973a5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "020-c3.bin", 0x200000, 0x100000, CRC(d1e42fd0) SHA1(f0d476aebbdc2ce008f5f0783be86d295b24aa44) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "020-c4.bin", 0x200001, 0x100000, CRC(edde439b) SHA1(79be7b10ecdab54c2f77062b8f5fda0e299fa982) ) /* Plane 2,3 */
ROM_END

ROM_START( gpilotsh ) /* AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "020-hp1.bin", 0x000000, 0x080000, CRC(7cdb01ce) SHA1(32cae2ddf5e26fb7e8a09132e600220db82df3b8) )
	ROM_LOAD16_WORD_SWAP( "020-p2.bin",  0x080000, 0x020000, CRC(edcb22ac) SHA1(505d2db38ae999b7d436e8f2ff56b81796d62b54) )

	NEO_SFIX_128K( "020-s1.bin", CRC(a6d83d53) SHA1(9a8c092f89521cc0b27a385aa72e29cbaca926c5) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "020-m1.bin", CRC(48409377) SHA1(0e212d2c76856a90b2c2fdff675239525972ac43) )

	ROM_REGION( 0x180000, "ymsnd", 0 )
	ROM_LOAD( "020-v11.bin", 0x000000, 0x100000, CRC(1b526c8b) SHA1(2801868d2badcf8aaf5d490e010e4049d81d7bc1) )
	ROM_LOAD( "020-v12.bin", 0x100000, 0x080000, CRC(4a9e6f03) SHA1(d3ac11f333b03d8a318921bdaefb14598e289a14) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "020-v21.bin", 0x000000, 0x080000, CRC(7abf113d) SHA1(5b2a0e70f2eaf4638b44702dacd4cb17838fb1d5) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "020-c1.bin", 0x000000, 0x100000, CRC(bd6fe78e) SHA1(50b704862cd79d64fa488e621b079f6e413c33bc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "020-c2.bin", 0x000001, 0x100000, CRC(5f4a925c) SHA1(71c5ef8141234daaa7025427a6c65e79766973a5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "020-c3.bin", 0x200000, 0x100000, CRC(d1e42fd0) SHA1(f0d476aebbdc2ce008f5f0783be86d295b24aa44) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "020-c4.bin", 0x200001, 0x100000, CRC(edde439b) SHA1(79be7b10ecdab54c2f77062b8f5fda0e299fa982) ) /* Plane 2,3 */
ROM_END

ROM_START( joyjoy ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "021-p1.bin", 0x000000, 0x080000, CRC(39c3478f) SHA1(06ebe54c9c4e14c5c31e770013d58b7162359ecc) )

	NEO_SFIX_128K( "021-s1.bin", CRC(6956d778) SHA1(e3757776d60dc07d8e07c9ca61b223b14732f860) )

	/* MB832000 */
	NEO_BIOS_AUDIO_256K( "021-m1.bin", CRC(5a4be5e8) SHA1(552f025ce0d51c25f42e1a81cf0d08376ca5475d) )

	ROM_REGION( 0x080000, "ymsnd", 0 )
	ROM_LOAD( "021-v11.bin", 0x000000, 0x080000, CRC(66c1e5c4) SHA1(7e85420021d4c39c36ed75a1cec567c5610ffce0) )

	ROM_REGION( 0x080000, "ymsnd.deltat", 0 )
	ROM_LOAD( "021-v21.bin", 0x000000, 0x080000, CRC(8ed20a86) SHA1(d15cba5eac19ea56fdd4877541f1bb3eb755ebba) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "021-c1.bin", 0x000000, 0x080000, CRC(509250ec) SHA1(d6ddb16d8222088f153a85a905bcb99541a5f2cf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "021-c2.bin", 0x000001, 0x080000, CRC(09ed5258) SHA1(6bf50cd10236e29146b49e714a0e0ebcfe30a682) ) /* Plane 2,3 */
ROM_END

ROM_START( bjourney ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "022-p1.bin", 0x000000, 0x100000, CRC(6a2f6d4a) SHA1(b8ca548e56f1c7abcdce415ba7329e0cf698ee13) )

	NEO_SFIX_128K( "022-s1.bin", CRC(843c3624) SHA1(dbdf86c193b7c1d795f8c21f2c103c1d3e18abbe) )

	NEO_BIOS_AUDIO_128K( "022-m1.bin", CRC(8e1d4ab6) SHA1(deabc11ab81e7e68a3e041c03a127ae28d0d7264) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "022-v11.bin", 0x000000, 0x100000, CRC(2cb4ad91) SHA1(169ec7303c4275155a66a88cc08270c24132bb36) )
	ROM_LOAD( "022-v22.bin", 0x100000, 0x100000, CRC(65a54d13) SHA1(a591fbcedca8f679dacbebcd554e3aa3fd163e92) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "022-c1.bin", 0x000000, 0x100000, CRC(4d47a48c) SHA1(6e282285be72583d828e7765b1c1695ecdc44777) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "022-c2.bin", 0x000001, 0x100000, CRC(e8c1491a) SHA1(c468d2556b3de095aaa05edd1bc16d71303e9478) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "022-c3.bin", 0x200000, 0x080000, CRC(66e69753) SHA1(974b823fc62236fbc23e727f25b61a805a707a9e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "022-c4.bin", 0x200001, 0x080000, CRC(71bfd48a) SHA1(47288be69e6992d09ebef108b4de9ffab6293dc8) ) /* Plane 2,3 */
ROM_END

ROM_START( quizdais )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "023-p1.bin", 0x000000, 0x100000, CRC(c488fda3) SHA1(4cdf2f1837fffd720efef42f81f933bdf2ef1402) )

	NEO_SFIX_128K( "023-s1.bin", CRC(ac31818a) SHA1(93c8d67a93606a2e02f12ca4cab849dc3f3de286) )

	NEO_BIOS_AUDIO_128K( "023-m1.bin", CRC(2a2105e0) SHA1(26fc13556fda2dbeb7b5b035abd994e302dc7662) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "023-v1.bin", 0x000000, 0x100000, CRC(a53e5bd3) SHA1(cf115c6478ce155d889e6a5acb962339e08e024b) )

	NO_DELTAT_REGION

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "023-c1.bin", 0x000000, 0x100000, CRC(2999535a) SHA1(0deabf771039987b559edc2444eea741bd7ba861) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "023-c2.bin", 0x000001, 0x100000, CRC(876a99e6) SHA1(8d1dcfc0927d7523f8be8203573192406ec654b4) ) /* Plane 2,3 */
ROM_END

ROM_START( lresort )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "024-p1.bin", 0x000000, 0x080000, CRC(89c4ab97) SHA1(3a1817c427185ea1b44fe52f009c00b0a9007c85) )

	NEO_SFIX_128K( "024-s1.bin", CRC(5cef5cc6) SHA1(9ec305007bdb356e9f8f279beae5e2bcb3f2cf7b) )

	NEO_BIOS_AUDIO_128K( "024-m1.bin", CRC(cec19742) SHA1(ab6c6ba7737e68d2420a0617719c6d4c89039c45) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "024-v1.bin", 0x000000, 0x100000, CRC(efdfa063) SHA1(e4609ecbcc1c820758f229da5145f51285b50555) )
	ROM_LOAD( "024-v2.bin", 0x100000, 0x100000, CRC(3c7997c0) SHA1(8cb7e8e69892b19d318978370dbc510d51b06a69) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "024-c1.bin", 0x000000, 0x100000, CRC(3617c2dc) SHA1(8de2643a618272f8aa1c705363edb007f4a5f5b7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "024-c2.bin", 0x000001, 0x100000, CRC(3f0a7fd8) SHA1(d0c9c7a9dde9ce175fb243d33ec11fa719d0158c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "024-c3.bin", 0x200000, 0x080000, CRC(e9f745f8) SHA1(bbe6141da28b0db7bf5cf321d69b7e613e2414d7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "024-c4.bin", 0x200001, 0x080000, CRC(7382fefb) SHA1(e916dec5bb5462eb9ae9711f08c7388937abb980) ) /* Plane 2,3 */
ROM_END

ROM_START( eightman )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "025-p1.bin", 0x000000, 0x080000, CRC(43344cb0) SHA1(29dfd699f35b0a74e20fedd6c9174c289f0ef6e0) )

	NEO_SFIX_128K( "025-s1.bin", CRC(a402202b) SHA1(75c44e1af459af155f5b892fd18706268dd5e602) )

	NEO_BIOS_AUDIO_128K( "025-m1.bin", CRC(9927034c) SHA1(205665361c5b2ab4f01ec480dd3c9b69db858d09) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "025-v1.bin", 0x000000, 0x100000, CRC(4558558a) SHA1(a4b277703ed67225c652be0d618daeca65a27b88) )
	ROM_LOAD( "025-v2.bin", 0x100000, 0x100000, CRC(c5e052e9) SHA1(fa1119c90ce4c706a6aa0c17d7bc06aa3068d9b2) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "025-c1.bin", 0x000000, 0x100000, CRC(555e16a4) SHA1(1c96f3d2fd0991680fbf627a6cdd26ad2cd60319) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "025-c2.bin", 0x000001, 0x100000, CRC(e1ee51c3) SHA1(da8d074bb4e923ed7b8a154fd31b42f2d65b8e96) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "025-c3.bin", 0x200000, 0x080000, CRC(0923d5b0) SHA1(ab72ba1e3ebf56dd356f9ad181f986b1360a1089) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "025-c4.bin", 0x200001, 0x080000, CRC(e3eca67b) SHA1(88154cbc1a261c2f425430119ebc08a30adc9675) ) /* Plane 2,3 */
ROM_END

ROM_START( minasan )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "027-p1.bin", 0x000000, 0x080000, CRC(c8381327) SHA1(c8f8be0ba276c6d12ef13d05af3cf83a2b924894) )

	NEO_SFIX_128K( "027-s1.bin", CRC(e5824baa) SHA1(8230ff7fe3cabeacecc762d90a084e893db84906) )

	NEO_BIOS_AUDIO_128K( "027-m1.bin", CRC(6a915482) SHA1(ce39c41a0b13798b0401ba1dde489619dfbc30ef) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "027-v11.bin", 0x000000, 0x100000, CRC(59ad4459) SHA1(bbb8ba8a8e337dd2946eefda4757e80d0547d54a) )

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 )
	ROM_LOAD( "027-v21.bin", 0x000000, 0x100000, CRC(df5b4eeb) SHA1(134f3bcc3bb82e2a5711496af1019f343f9c0f7e) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "027-c1.bin", 0x000000, 0x100000, CRC(d0086f94) SHA1(7d6579530ccb5188f653be69b1df17e47e40e7a6) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "027-c2.bin", 0x000001, 0x100000, CRC(da61f5a6) SHA1(82c5b4e5c5c5e30a3fd1c2e11c6157f39d033c42) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "027-c3.bin", 0x200000, 0x100000, CRC(08df1228) SHA1(288b7ad328c2249f28d17df4dad3584995dca7bf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "027-c4.bin", 0x200001, 0x100000, CRC(54e87696) SHA1(90816dc86be3983dc57f56ededf7738475c0c61e) ) /* Plane 2,3 */
ROM_END

ROM_START( legendos )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "029-p1.bin", 0x000000, 0x080000, CRC(9d563f19) SHA1(9bff7bf9fdcf81a0a6c4ce3e196097d4f05e67b6) )

	NEO_SFIX_128K( "029-s1.bin", CRC(bcd502f0) SHA1(a3400f52c037aa6a42e59e602cc24fa45fcbc951) )

	NEO_BIOS_AUDIO_128K( "029-m1.bin", CRC(8ed64977) SHA1(c2f77213f05b4b2e1962a45e718a9e36cc398cbb) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "029-v1.bin", 0x000000, 0x100000, CRC(85065452) SHA1(7154b7c59b16c32753ac6b5790fb50b51ce30a20) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "029-c1.bin", 0x000000, 0x100000, CRC(2f5ab875) SHA1(3e060973bba41a6c22ff7054104bdc5eee1fa13a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "029-c2.bin", 0x000001, 0x100000, CRC(318b2711) SHA1(7014110cee98280317e1189f306ca40652b61f6f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "029-c3.bin", 0x200000, 0x100000, CRC(6bc52cb2) SHA1(14323a4664b7dcbcde82e594168e535d7a921e44) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "029-c4.bin", 0x200001, 0x100000, CRC(37ef298c) SHA1(7a0c4c896dc3e730e06dcadbf00cf354f08a4466) ) /* Plane 2,3 */
ROM_END

ROM_START( 2020bb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "030-p1.bin", 0x000000, 0x080000, CRC(d396c9cb) SHA1(47ba421d14d05b965a8d44e7475b227a208e5a07) )

	NEO_SFIX_128K( "030-s1.bin", CRC(7015b8fc) SHA1(8c09bc3e6c62e0f7c9557c1e10c901be325bae7f) )

	NEO_BIOS_AUDIO_128K( "030-m1.bin", CRC(4cf466ec) SHA1(6a003b53c7a4af9d7529e2c10f27ffc4e58dcda5) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "030-v1.bin", 0x000000, 0x100000, CRC(d4ca364e) SHA1(b0573744b0ea2ef1e2167a225f0d254883f5af04) )
	ROM_LOAD( "030-v2.bin", 0x100000, 0x100000, CRC(54994455) SHA1(76eb62b86e8ed51a77f44313d5cc8091b3f58d57) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "030-c1.bin", 0x000000, 0x100000, CRC(4f5e19bd) SHA1(ef7975c4b33a7aea4a25a385f604799f054d3200) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "030-c2.bin", 0x000001, 0x100000, CRC(d6314bf0) SHA1(0920cc580d7997fcb0170dd619af2f305d635577) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "030-c3.bin", 0x200000, 0x100000, CRC(47fddfee) SHA1(297c505a63448c999a2510c27bf4549102134db8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "030-c4.bin", 0x200001, 0x100000, CRC(780d1c4e) SHA1(2e2cf9de828e3b48642dd2203637103438c62142) ) /* Plane 2,3 */
ROM_END

ROM_START( 2020bba ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "030-epr.p1", 0x000000, 0x080000, CRC(c59be3dd) SHA1(4fbd462c1c18e85a252c58b04b54fd3b82b46cb0) )
	/* P1 on eprom, correct chip label unknown */

	NEO_SFIX_128K( "030-s1.bin", CRC(7015b8fc) SHA1(8c09bc3e6c62e0f7c9557c1e10c901be325bae7f) )

	NEO_BIOS_AUDIO_128K( "030-m1.bin", CRC(4cf466ec) SHA1(6a003b53c7a4af9d7529e2c10f27ffc4e58dcda5) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "030-v1.bin", 0x000000, 0x100000, CRC(d4ca364e) SHA1(b0573744b0ea2ef1e2167a225f0d254883f5af04) )
	ROM_LOAD( "030-v2.bin", 0x100000, 0x100000, CRC(54994455) SHA1(76eb62b86e8ed51a77f44313d5cc8091b3f58d57) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "030-c1.bin", 0x000000, 0x100000, CRC(4f5e19bd) SHA1(ef7975c4b33a7aea4a25a385f604799f054d3200) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "030-c2.bin", 0x000001, 0x100000, CRC(d6314bf0) SHA1(0920cc580d7997fcb0170dd619af2f305d635577) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "030-c3.bin", 0x200000, 0x100000, CRC(47fddfee) SHA1(297c505a63448c999a2510c27bf4549102134db8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "030-c4.bin", 0x200001, 0x100000, CRC(780d1c4e) SHA1(2e2cf9de828e3b48642dd2203637103438c62142) ) /* Plane 2,3 */
ROM_END

ROM_START( 2020bbh )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "030-p1h.bin", 0x000000, 0x080000, CRC(12d048d7) SHA1(ee0d03a565b11ca3bee2d24f62ff46a85ef18d90) )

	NEO_SFIX_128K( "030-s1.bin", CRC(7015b8fc) SHA1(8c09bc3e6c62e0f7c9557c1e10c901be325bae7f) )

	NEO_BIOS_AUDIO_128K( "030-m1.bin", CRC(4cf466ec) SHA1(6a003b53c7a4af9d7529e2c10f27ffc4e58dcda5) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "030-v1.bin", 0x000000, 0x100000, CRC(d4ca364e) SHA1(b0573744b0ea2ef1e2167a225f0d254883f5af04) )
	ROM_LOAD( "030-v2.bin", 0x100000, 0x100000, CRC(54994455) SHA1(76eb62b86e8ed51a77f44313d5cc8091b3f58d57) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "030-c1.bin", 0x000000, 0x100000, CRC(4f5e19bd) SHA1(ef7975c4b33a7aea4a25a385f604799f054d3200) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "030-c2.bin", 0x000001, 0x100000, CRC(d6314bf0) SHA1(0920cc580d7997fcb0170dd619af2f305d635577) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "030-c3.bin", 0x200000, 0x100000, CRC(47fddfee) SHA1(297c505a63448c999a2510c27bf4549102134db8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "030-c4.bin", 0x200001, 0x100000, CRC(780d1c4e) SHA1(2e2cf9de828e3b48642dd2203637103438c62142) ) /* Plane 2,3 */
ROM_END

ROM_START( socbrawl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "031-p1.bin", 0x000000, 0x080000, CRC(a2801c24) SHA1(627d76ff0740ca29586f37b268f47fb469822529) )

	NEO_SFIX_128K( "031-s1.bin", CRC(4c117174) SHA1(26e52c4f628338a9aa1c159517cdf873f738fb98) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "031-m1.bin", CRC(cb37427c) SHA1(99efe9600ebeda48331f396e3203c7588bdb7d24) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "031-v1.bin", 0x000000, 0x100000, CRC(cc78497e) SHA1(895bd647150fae9b2259ef043ed681f4c4de66ea) )
	ROM_LOAD( "031-v2.bin", 0x100000, 0x100000, CRC(dda043c6) SHA1(08165a59700ab6b1e523079dd2a3549e520cc594) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "031-c1.bin", 0x000000, 0x100000, CRC(bd0a4eb8) SHA1(b67988cb3e550d083e81c9bd436da55b242785ed) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "031-c2.bin", 0x000001, 0x100000, CRC(efde5382) SHA1(e42789c8d87ee3d4549d0a903e990c03338cbbd8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "031-c3.bin", 0x200000, 0x080000, CRC(580f7f33) SHA1(f4f95a7c8de00e1366a723fc4cd0e8c1905af636) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "031-c4.bin", 0x200001, 0x080000, CRC(ed297de8) SHA1(616f8fa4c86231f3e79faf9f69f8bb909cbc35f0) ) /* Plane 2,3 */
ROM_END

ROM_START( socbrawla ) /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "031-pg1.bin", 0x000000, 0x080000, CRC(17f034a7) SHA1(2e66c7bd93a08efe63c4894494db50bbf58f60e4) )

	NEO_SFIX_128K( "031-s1.bin", CRC(4c117174) SHA1(26e52c4f628338a9aa1c159517cdf873f738fb98) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "031-m1.bin", CRC(cb37427c) SHA1(99efe9600ebeda48331f396e3203c7588bdb7d24) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "031-v1.bin", 0x000000, 0x100000, CRC(cc78497e) SHA1(895bd647150fae9b2259ef043ed681f4c4de66ea) )
	ROM_LOAD( "031-v2.bin", 0x100000, 0x100000, CRC(dda043c6) SHA1(08165a59700ab6b1e523079dd2a3549e520cc594) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "031-c1.bin", 0x000000, 0x100000, CRC(bd0a4eb8) SHA1(b67988cb3e550d083e81c9bd436da55b242785ed) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "031-c2.bin", 0x000001, 0x100000, CRC(efde5382) SHA1(e42789c8d87ee3d4549d0a903e990c03338cbbd8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "031-c3.bin", 0x200000, 0x080000, CRC(580f7f33) SHA1(f4f95a7c8de00e1366a723fc4cd0e8c1905af636) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "031-c4.bin", 0x200001, 0x080000, CRC(ed297de8) SHA1(616f8fa4c86231f3e79faf9f69f8bb909cbc35f0) ) /* Plane 2,3 */
ROM_END

ROM_START( roboarmy )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "032-p1.bin", 0x000000, 0x080000, CRC(cd11cbd4) SHA1(23163e3da2f07e830a7f4a02aea1cb01a54ccbf3) )

	NEO_SFIX_128K( "032-s1.bin", CRC(ac0daa1b) SHA1(93bae4697dc403fce19422752a514326ccf66a91) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "032-m1.bin", CRC(35ec952d) SHA1(8aed30e26d7e2c70dbce5de752df416091066f7b) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "032-v1.bin", 0x000000, 0x100000, CRC(63791533) SHA1(4479e9308cdc906b9e03b985303f4ebedd00512f) )
	ROM_LOAD( "032-v2.bin", 0x100000, 0x100000, CRC(eb95de70) SHA1(b34885201116d2b3bbdee15ec7b5961cf5c069e1) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "032-c1.bin", 0x000000, 0x100000, CRC(97984c6c) SHA1(deea59c0892f05dc7db98cb57b3eb83688dc57f0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "032-c2.bin", 0x000001, 0x100000, CRC(65773122) SHA1(2c0162a8e971e5e57933e4ae16040bf824ffdefe) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "032-c3.bin", 0x200000, 0x080000, CRC(40adfccd) SHA1(b11f866dd70ba0ed9123424508355cb948b19bdc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "032-c4.bin", 0x200001, 0x080000, CRC(462571de) SHA1(5c3d610d492f91564423873b3b434dcda700373f) ) /* Plane 2,3 */
ROM_END

	/* Disabled for now, this looks like an emulation hack. Can someone confirm this set is real? */
#if 0
ROM_START( roboarma )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "032-p1h.bin", 0x000000, 0x080000, CRC(27c773cb) SHA1(597ca73f142b1129cc7780540bb9cfacd47bc6ce) )

	NEO_SFIX_128K( "032-s1.bin", CRC(ac0daa1b) SHA1(93bae4697dc403fce19422752a514326ccf66a91) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "032-m1.bin", CRC(35ec952d) SHA1(8aed30e26d7e2c70dbce5de752df416091066f7b) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "032-v1.bin", 0x000000, 0x100000, CRC(63791533) SHA1(4479e9308cdc906b9e03b985303f4ebedd00512f) )
	ROM_LOAD( "032-v2.bin", 0x100000, 0x100000, CRC(eb95de70) SHA1(b34885201116d2b3bbdee15ec7b5961cf5c069e1) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "032-c1.bin", 0x000000, 0x100000, CRC(97984c6c) SHA1(deea59c0892f05dc7db98cb57b3eb83688dc57f0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "032-c2.bin", 0x000001, 0x100000, CRC(65773122) SHA1(2c0162a8e971e5e57933e4ae16040bf824ffdefe) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "032-c3.bin", 0x200000, 0x080000, CRC(40adfccd) SHA1(b11f866dd70ba0ed9123424508355cb948b19bdc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "032-c4.bin", 0x200001, 0x080000, CRC(462571de) SHA1(5c3d610d492f91564423873b3b434dcda700373f) ) /* Plane 2,3 */
ROM_END
#endif

ROM_START( fatfury1 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "033-p1.bin", 0x000000, 0x080000, CRC(47ebdc2f) SHA1(d46786502920fb510f1999db00c5e09fb641c0bd) )
	ROM_LOAD16_WORD_SWAP( "033-p2.bin", 0x080000, 0x020000, CRC(c473af1c) SHA1(4919eeca20abe807493872ca7c79a5d1f496fe68) )

	NEO_SFIX_128K( "033-s1.bin", CRC(3c3bdf8c) SHA1(2f3e5feed6c27850b2a0f6fae0b97041690e944c) )

	NEO_BIOS_AUDIO_128K( "033-m1.bin", CRC(5be10ffd) SHA1(90a5e6cbbc58a7883cd2a3a597180d631a466882) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "033-v1.bin", 0x000000, 0x100000, CRC(212fd20d) SHA1(120c040db8c01a6f140eea03725448bfa9ca98c2) )
	ROM_LOAD( "033-v2.bin", 0x100000, 0x100000, CRC(fa2ae47f) SHA1(80d0ba4cd30aab59b6f0db8fa341387bd7388afc) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "033-c1.bin", 0x000000, 0x100000, CRC(74317e54) SHA1(67b9c2814a12603b959612456f59de55f9bf6f57) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "033-c2.bin", 0x000001, 0x100000, CRC(5bb952f3) SHA1(ea964bbcc0408b6ae07cbb5043d003281b1aca15) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "033-c3.bin", 0x200000, 0x100000, CRC(9b714a7c) SHA1(b62bdcede3207d062a89e0a4a9adf706101bb681) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "033-c4.bin", 0x200001, 0x100000, CRC(9397476a) SHA1(a12dbb74020aeb6ebf24ec2abbfba5129cabcb7d) ) /* Plane 2,3 */
ROM_END

ROM_START( fbfrenzy ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "034-p1.bin", 0x000000, 0x080000, CRC(cdef6b19) SHA1(97482db0dffc6d625fb41fa38449c0a74d741a72) )

	NEO_SFIX_128K( "034-s1.bin", CRC(8472ed44) SHA1(42e1a9671dddd090d2a634cff986f6c73ba08b70) )

	NEO_BIOS_AUDIO_128K( "034-m1.bin", CRC(f41b16b8) SHA1(f3e1cfc4cd2c5baece176f169906aa796367d303) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "034-v1.bin", 0x000000, 0x100000, CRC(50c9d0dd) SHA1(2b3f2875b00e5f307d274128bd73c1521a7d901b) )
	ROM_LOAD( "034-v2.bin", 0x100000, 0x100000, CRC(5aa15686) SHA1(efe47954827a98d539ba719347c5f8aa60e6338b) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "034-c1.bin", 0x000000, 0x100000, CRC(91c56e78) SHA1(2944d49ebfc71239d345209ca7f25993c2cc5a77) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "034-c2.bin", 0x000001, 0x100000, CRC(9743ea2f) SHA1(cf4fccdf10d521d555e92bc24123142393c2b3bb) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "034-c3.bin", 0x200000, 0x080000, CRC(e5aa65f5) SHA1(714356a2cee976ec0f515b1034ce971018e5c02e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "034-c4.bin", 0x200001, 0x080000, CRC(0eb138cc) SHA1(21d31e1f136c674caa6dd44073281cd07b72ea9b) ) /* Plane 2,3 */
ROM_END

ROM_START( bakatono )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "036-p1.bin", 0x000000, 0x080000, CRC(1c66b6fa) SHA1(6c50cc452971c46c763ae0b2def95792671a1798) )

	NEO_SFIX_128K( "036-s1.bin", CRC(f3ef4485) SHA1(c30bfceed7e669e4c97b0b3ec2e9f4271e5b6662) )

	NEO_BIOS_AUDIO_128K( "036-m1.bin", BAD_DUMP CRC(f1385b96) SHA1(e7e3d1484188a115e262511116aaf466b8b1f428) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "036-v1.bin", 0x000000, 0x100000, CRC(1c335dce) SHA1(493c273fa71bf81861a20af4c4eaae159e169f39) )
	ROM_LOAD( "036-v2.bin", 0x100000, 0x100000, CRC(bbf79342) SHA1(45a4f40e415cdf35c3073851506648c8f7d53958) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "036-c1.bin", 0x000000, 0x100000, CRC(fe7f1010) SHA1(5b6f5053821f4da8dc3768371e2cd51bb29da963) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "036-c2.bin", 0x000001, 0x100000, CRC(bbf003f5) SHA1(054b2a3327e038836eece652055bb84c115cf8ed) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "036-c3.bin", 0x200000, 0x100000, CRC(9ac0708e) SHA1(8decfe06d73a3dd3c3cf280719978fcf6d559d29) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "036-c4.bin", 0x200001, 0x100000, CRC(f2577d22) SHA1(a37db8055ca4680e244c556dc6df8bdba16c2083) ) /* Plane 2,3 */
ROM_END

ROM_START( crsword )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "037-p1.bin", 0x000000, 0x080000, CRC(e7f2553c) SHA1(8469ecb900477feed05ae3311fe9515019bbec2a) )

	NEO_SFIX_128K( "037-s1.bin", CRC(74651f27) SHA1(bff7ff2429d2be82c1647abac2ee45b339b3b310) )

	NEO_BIOS_AUDIO_128K( "037-m1.bin", CRC(9c384263) SHA1(74b86284048669e316c5d241b4aaeb02d59d4dfa) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "037-v1.bin", 0x000000, 0x100000, CRC(61fedf65) SHA1(98f31d1e23bf7c1f7844e67f14707a704134042e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "037-c1.bin", 0x000000, 0x100000, CRC(09df6892) SHA1(df2579dcf9c9dc88d461212cb74de106be2983c1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "037-c2.bin", 0x000001, 0x100000, CRC(ac122a78) SHA1(7bfa4d29b7d7d9443f64d81caeafa74fe05c606e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "037-c3.bin", 0x200000, 0x100000, CRC(9d7ed1ca) SHA1(2bbd25dc3a3f825d0af79a418f06a23a1bf03cc0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "037-c4.bin", 0x200001, 0x100000, CRC(4a24395d) SHA1(943f911f40985db901eaef4c28dfcda299fca73e) ) /* Plane 2,3 */
ROM_END

ROM_START( trally ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "038-p1.bin", 0x000000, 0x080000, CRC(1e52a576) SHA1(a1cb56354c3378e955b0cd482c3c41ae15add952) )
	ROM_LOAD16_WORD_SWAP( "038-p2.bin", 0x080000, 0x080000, CRC(a5193e2f) SHA1(96803480439e90da23cdca70d59ff519ee85beeb) )

	NEO_SFIX_128K( "038-s1.bin", CRC(fff62ae3) SHA1(6510a762ea41557a8938cbfc0557cd5921306061) )

	NEO_BIOS_AUDIO_128K( "038-m1.bin", CRC(0908707e) SHA1(df7489ea6abf84d7f137ba7a8f52a4fd1b088fd7) )

	ROM_REGION( 0x180000, "ymsnd", 0 )
	ROM_LOAD( "038-v1.bin", 0x000000, 0x100000, CRC(5ccd9fd5) SHA1(c3c8c758a320c39e4ceb0b6d9f188ed6d122eec4) )
	ROM_LOAD( "038-v2.bin", 0x100000, 0x080000, CRC(ddd8d1e6) SHA1(65c819fa2392f264f5a1a0a4967c96775732500b) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "038-c1.bin", 0x000000, 0x100000, CRC(c58323d4) SHA1(a6bd277471a4b612d165f8b804f3cb662f499b70) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "038-c2.bin", 0x000001, 0x100000, CRC(bba9c29e) SHA1(b70bbfdfa8c4f9ea76406530e86b16e42498d284) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "038-c3.bin", 0x200000, 0x080000, CRC(3bb7b9d6) SHA1(bc1eae6181ad5abf79736afc8db4ca34113d43f8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "038-c4.bin", 0x200001, 0x080000, CRC(a4513ecf) SHA1(934aa103c226eac55157b44d7b4dfa35515322c3) ) /* Plane 2,3 */
ROM_END

ROM_START( kotm2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "039-p1.bin", 0x000000, 0x080000, CRC(b372d54c) SHA1(b70fc6f72e16a66b6e144cc01370548e3398b8b8) )
	ROM_LOAD16_WORD_SWAP( "039-p2.bin", 0x080000, 0x080000, CRC(28661afe) SHA1(6c85ff6ab334b1ca744f726f42dac211537e7315) )

	NEO_SFIX_128K( "039-s1.bin", CRC(63ee053a) SHA1(7d4b92bd022708975b1470e8f24d1f5a712e1b94) )

	NEO_BIOS_AUDIO_128K( "039-m1.bin", CRC(0c5b2ad5) SHA1(15eb5ea10fecdbdbcfd06225ae6d88bb239592e7) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "039-v2.bin", 0x000000, 0x200000, CRC(86d34b25) SHA1(89bdb614b0c63d678962da52e2f596750d20828c) )
	ROM_LOAD( "039-v4.bin", 0x200000, 0x100000, CRC(8fa62a0b) SHA1(58ac2fdd73c542eb8178cfc4adfa0e5940183283) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "039-c1.bin", 0x000000, 0x100000, CRC(6d1c4aa9) SHA1(4fbc9d7cb37522ec298eefbe38c75a2d050fbb4a) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "039-c2.bin", 0x000001, 0x100000, CRC(f7b75337) SHA1(4d85f85948c3e6ed38b0b0ccda79de3ce026e2d9) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
	ROM_LOAD16_BYTE( "039-c3.bin", 0x200000, 0x080000, CRC(bfc4f0b2) SHA1(f4abe2b52882b966412f3b503b8f2c8f49b57968) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "039-c4.bin", 0x200001, 0x080000, CRC(81c9c250) SHA1(e3a34ff69081a8681b5ca895915892dcdccfa7aa) ) /* Plane 2,3 */
ROM_END

ROM_START( sengoku2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "040-p1.bin", 0x000000, 0x100000, CRC(6dde02c2) SHA1(e432e63feb88c71629ec96aa84650dcfe356a551) )

	NEO_SFIX_128K( "040-s1.bin", CRC(cd9802a3) SHA1(f685d4638f4f68e7e3f101c0c39128454536721b) )

	NEO_BIOS_AUDIO_128K( "040-m1.bin", CRC(d4de4bca) SHA1(ecf604d06f01d40b04e285facef66a6ae2d35661) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "040-v1.bin", 0x000000, 0x200000, CRC(71cb4b5d) SHA1(56d9aca1d476c19c7d0f707176a8fed53e0189b7) )
	ROM_LOAD( "040-v2.bin", 0x200000, 0x100000, CRC(c5cece01) SHA1(923a3377dac1919e8c3d9ab316902250caa4785f) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "040-c1.bin", 0x000000, 0x100000, CRC(faa8ea99) SHA1(714575e57ea1990612f960ec42b38d2e157ad400) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "040-c2.bin", 0x000001, 0x100000, CRC(87d0ec65) SHA1(23645e0cf859fb4cec3745b3846ca0ef64c689fb) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
	ROM_LOAD16_BYTE( "040-c3.bin", 0x200000, 0x080000, CRC(24b5ba80) SHA1(29d58a6b56bd24ee2046a8d45e023b4d7ab7685b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "040-c4.bin", 0x200001, 0x080000, CRC(1c9e9930) SHA1(d017474873750a7602b7708c663d29b25ef7bb63) ) /* Plane 2,3 */
ROM_END

ROM_START( bstars2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "041-p1.bin", 0x000000, 0x080000, CRC(523567fd) SHA1(f1e81eb4678f586b214ea102cde6effea1b0f768) )

	NEO_SFIX_128K( "041-s1.bin", CRC(015c5c94) SHA1(f1c60cd3dc54986b39f630ef3bf48f68c68695dc) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "041-m1.bin", CRC(15c177a6) SHA1(3f4abed635102f9bc8b44809750828c82e79b44f) )

	ROM_REGION( 0x280000, "ymsnd", 0 )
	ROM_LOAD( "041-v1.bin", 0x000000, 0x100000, CRC(cb1da093) SHA1(4f4d1d5fefa9dda372083c045bf0d268a57ce8f1) )
	ROM_LOAD( "041-v2.bin", 0x100000, 0x100000, CRC(1c954a9d) SHA1(159bc6efdd531615461f6e16f83f6d4c4e67c237) )
	ROM_LOAD( "041-v3.bin", 0x200000, 0x080000, CRC(afaa0180) SHA1(c4a047e21f093830498a163598ed7bd48a8cf9d1) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "041-c1.bin", 0x000000, 0x100000, CRC(b39a12e1) SHA1(bafe383bd7c5a6aac4cb92dabbc56e3672fe174d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "041-c2.bin", 0x000001, 0x100000, CRC(766cfc2f) SHA1(79e1063925d54a57df943019a88bea56c9152df3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "041-c3.bin", 0x200000, 0x100000, CRC(fb31339d) SHA1(f4e821299680970b2e979acc4a170029b968c807) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "041-c4.bin", 0x200001, 0x100000, CRC(70457a0c) SHA1(a1e307f11ddab85d2e9c09d0428fac2e6da774b1) ) /* Plane 2,3 */
ROM_END

ROM_START( quizdai2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "042-p1.bin", 0x000000, 0x100000, CRC(ed719dcf) SHA1(12baf2601e86c0e4358517b9fa1c55f2f5835f1d) )

	NEO_SFIX_128K( "042-s1.bin", CRC(164fd6e6) SHA1(dad35bedc33d502a5ae745a45a972af8d901b160) )

	NEO_BIOS_AUDIO_128K( "042-m1.bin", CRC(bb19995d) SHA1(ed458fad5a23c6bd0d099927d98c31e1e6562d1b) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "042-v1.bin", 0x000000, 0x100000, CRC(af7f8247) SHA1(99a47014017c20e4e22010c60612b6b7f6efc9e5) )
	ROM_LOAD( "042-v2.bin", 0x100000, 0x100000, CRC(c6474b59) SHA1(a6c5054032b698116247b2f09a8b94a1b588c4f1) )

	NO_DELTAT_REGION

	ROM_REGION( 0x300000, "sprites", 0 )
	ROM_LOAD16_BYTE( "042-c1.bin", 0x000000, 0x100000, CRC(cb5809a1) SHA1(b53d06685246dd51b82b5c1d54d639d10e2ec26d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "042-c2.bin", 0x000001, 0x100000, CRC(1436dfeb) SHA1(27d136fb1be793bd345a741f5e55a977275fff86) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "042-c3.bin", 0x200000, 0x080000, CRC(bcd4a518) SHA1(f355298fe0f2cf50ddcc0d613db56a5c04d7230f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "042-c4.bin", 0x200001, 0x080000, CRC(d602219b) SHA1(34cf0f16db1e224396464ac838f4cd2e6d1c640e) ) /* Plane 2,3 */
ROM_END

ROM_START( 3countb ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "043-p1.bin", 0x000000, 0x100000, CRC(ffbdd928) SHA1(05b24655ca32723661adc5509b450824deb0c176) )
	/* The original p1 is 8mbit; also found sets with p1 / p2 4mbit on eprom. */

	NEO_SFIX_128K( "043-s1.bin", CRC(c362d484) SHA1(a3c029292572842feabe9aa8c3372628fb63978d) )

	NEO_BIOS_AUDIO_128K( "043-m1.bin", CRC(7eab59cb) SHA1(5ae4107457e091f73960bfba39b589ae36d51ca3) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "043-v1.bin", 0x000000, 0x200000, CRC(63688ce8) SHA1(5c6ac29a0cc0655a87cfe3ada8706838b86b86e4) )
	ROM_LOAD( "043-v2.bin", 0x200000, 0x200000, CRC(c69a827b) SHA1(f5197ea87bb6573fa6aef3a1713c3679c58c1e74) )

	NO_DELTAT_REGION

	ROM_REGION( 0x0800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "043-c1.bin", 0x000000, 0x100000, CRC(bad2d67f) SHA1(04928e50ca75b7fbc52b64e816ec5701901f5893) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "043-c2.bin", 0x000001, 0x100000, CRC(a7fbda95) SHA1(9da3c5faf22592a7eaf8df9fa6454f48c2a927ae) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
	ROM_LOAD16_BYTE( "043-c3.bin", 0x200000, 0x100000, CRC(f00be011) SHA1(2721cdba37a511a966a2a53b9bd6240f181d920c) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x600000, 0x100000 )
	ROM_LOAD16_BYTE( "043-c4.bin", 0x200001, 0x100000, CRC(1887e5c0) SHA1(9b915359add7c10c78d8b281b4084eceea8f0499) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x600001, 0x100000 )
ROM_END

ROM_START( aof ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "044-p1.bin", 0x000000, 0x080000, CRC(ca9f7a6d) SHA1(4d28ef86696f7e832510a66d3e8eb6c93b5b91a1) )

	NEO_SFIX_128K( "044-s1.bin", CRC(89903f39) SHA1(a04a0c244a5d5c7a595fcf649107969635a6a8b6) )

	/* TC531001 */
	NEO_BIOS_AUDIO_128K( "044-m1.bin", CRC(0987e4bb) SHA1(8fae4b7fac09d46d4727928e609ed9d3711dbded) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "044-v2.bin", 0x000000, 0x200000, CRC(3ec632ea) SHA1(e3f413f580b57f70d2dae16dbdacb797884d3fce) )
	ROM_LOAD( "044-v4.bin", 0x200000, 0x200000, CRC(4b0f8e23) SHA1(105da0cc5ba19869c7147fba8b177500758c232b) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "044-c1.bin", 0x000000, 0x100000, CRC(ddab98a7) SHA1(f20eb81ec431268798c142c482146c1545af1c24) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "044-c2.bin", 0x000001, 0x100000, CRC(d8ccd575) SHA1(f697263fe92164e274bf34c55327b3d4a158b332) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
	ROM_LOAD16_BYTE( "044-c3.bin", 0x200000, 0x100000, CRC(403e898a) SHA1(dd5888f8b24a33b2c1f483316fe80c17849ccfc4) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x600000, 0x100000 )
	ROM_LOAD16_BYTE( "044-c4.bin", 0x200001, 0x100000, CRC(6235fbaa) SHA1(9090e337d7beed25ba81ae0708d0aeb57e6cf405) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x600001, 0x100000 )
ROM_END

ROM_START( samsho ) /* MVS VERSION */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "045-p1.bin",  0x000000, 0x100000, CRC(dfe51bf0) SHA1(2243af3770a516ae698b69bcd9daf53632d9128d) )
	ROM_LOAD16_WORD_SWAP( "045-pg2.bin", 0x100000, 0x100000, CRC(46745b94) SHA1(d9e959fd1f88c9402915c1d0dcdb4a9e3d49cdcb) )

	NEO_SFIX_128K( "045-s1.bin", CRC(9142a4d3) SHA1(54088e99fcfd75fd0f94852890a56350066a05a3) )

	NEO_BIOS_AUDIO_128K( "045-m1.bin", CRC(95170640) SHA1(125c502db0693e8d11cef619b090081c14a9a300) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "045-v1.bin", 0x000000, 0x200000, CRC(37f78a9b) SHA1(6279b497d12fa90b49ab5ac3aae20fb302ec8b81) )
	ROM_LOAD( "045-v2.bin", 0x200000, 0x200000, CRC(568b20cf) SHA1(61af858685472a1fad608e230cccc2b108509ddb) )

	NO_DELTAT_REGION

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "045-c1.bin", 0x000000, 0x200000, CRC(2e5873a4) SHA1(65c74c1e2d34390666bbb630df7d1f4c9570c3db) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "045-c2.bin", 0x000001, 0x200000, CRC(04febb10) SHA1(16a8cbf0fd9468e81bf9eab6dbe7a8e3623a843e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "045-c3.bin", 0x400000, 0x200000, CRC(f3dabd1e) SHA1(c80e52df42be9f8b2e89b467b11ab140a480cee8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "045-c4.bin", 0x400001, 0x200000, CRC(935c62f0) SHA1(0053d40085fac14096b683f4341f65e543b71dc1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "045-c51.bin", 0x800000, 0x100000, CRC(81932894) SHA1(550f15dc5892c4602422c51869f0d59f70f01e9e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "045-c61.bin", 0x800001, 0x100000, CRC(be30612e) SHA1(5e8b785f917c176d6796eba0caed37b13ddb3e63) ) /* Plane 2,3 */
ROM_END

ROM_START( samshoh ) /* AES VERSION */
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "045-p1.bin", 0x000000, 0x100000, CRC(dfe51bf0) SHA1(2243af3770a516ae698b69bcd9daf53632d9128d) )
	ROM_LOAD16_WORD_SWAP( "045-p2.bin", 0x100000, 0x080000, CRC(38ee9ba9) SHA1(48190699a6be83cb6257365ae81f93fdd23abe09) )

	NEO_SFIX_128K( "045-s1.bin", CRC(9142a4d3) SHA1(54088e99fcfd75fd0f94852890a56350066a05a3) )

	NEO_BIOS_AUDIO_128K( "045-m1.bin", CRC(95170640) SHA1(125c502db0693e8d11cef619b090081c14a9a300) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "045-v1.bin", 0x000000, 0x200000, CRC(37f78a9b) SHA1(6279b497d12fa90b49ab5ac3aae20fb302ec8b81) )
	ROM_LOAD( "045-v2.bin", 0x200000, 0x200000, CRC(568b20cf) SHA1(61af858685472a1fad608e230cccc2b108509ddb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x900000, "sprites", 0 )
	ROM_LOAD16_BYTE( "045-c1.bin", 0x000000, 0x200000, CRC(2e5873a4) SHA1(65c74c1e2d34390666bbb630df7d1f4c9570c3db) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "045-c2.bin", 0x000001, 0x200000, CRC(04febb10) SHA1(16a8cbf0fd9468e81bf9eab6dbe7a8e3623a843e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "045-c3.bin", 0x400000, 0x200000, CRC(f3dabd1e) SHA1(c80e52df42be9f8b2e89b467b11ab140a480cee8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "045-c4.bin", 0x400001, 0x200000, CRC(935c62f0) SHA1(0053d40085fac14096b683f4341f65e543b71dc1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "045-c5.bin", 0x800000, 0x080000, CRC(a2bb8284) SHA1(aa118e3b8c062daa219b36758b9a3814c08c69dc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "045-c6.bin", 0x800001, 0x080000, CRC(4fa71252) SHA1(afe374a9d1f2d955a59efe7b6196b89e021b164c) ) /* Plane 2,3 */
ROM_END

	/* Old version looked like this; p's on eprom?
    ROM_START( samsho )
    ROM_REGION( 0x180000, "maincpu", 0 )
    ROM_LOAD16_WORD_SWAP( "045-p1.bin", 0x000000, 0x080000, CRC(80aa6c97) SHA1(6e07a4aa7b4719ae487a10cee9389cb55a370a7a) )
    ROM_LOAD16_WORD_SWAP( "045-p2.bin", 0x080000, 0x080000, CRC(71768728) SHA1(9ff0e01d3fb73ad04279d4fdf4c53c3160888179) )
    ROM_LOAD16_WORD_SWAP( "045-p3.bin", 0x100000, 0x080000, CRC(38ee9ba9) SHA1(48190699a6be83cb6257365ae81f93fdd23abe09) )
    */

ROM_START( tophuntr ) /* MVS VERSION */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "046-p1.bin", 0x000000, 0x100000, CRC(69fa9e29) SHA1(9a40a16163193bb506a32bd34f6323b25ec69622) )
	ROM_LOAD16_WORD_SWAP( "046-p2.bin", 0x100000, 0x100000, CRC(f182cb3e) SHA1(6b4e0af5d4e623f0682f37ff5c69e5b705e20028) )

	NEO_SFIX_128K( "046-s1.bin", CRC(14b01d7b) SHA1(618ce75c25d6cc86a3b46bd64a0aa34ab82f75ae) )

	NEO_BIOS_AUDIO_128K( "046-m1.bin", CRC(3f84bb9f) SHA1(07446040871d11da3c2217ee9d1faf8c3cae7420) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "046-v1.bin", 0x000000, 0x100000, CRC(c1f9c2db) SHA1(bed95a76afefa46503a12e0f0a9787c4c967ac50) )
	ROM_LOAD( "046-v2.bin", 0x100000, 0x100000, CRC(56254a64) SHA1(1cf049cb4c414419859d2c8ee714317a35a85251) )
	ROM_LOAD( "046-v3.bin", 0x200000, 0x100000, CRC(58113fb1) SHA1(40972982a63c7adecef840f9882f4165da723ab6) )
	ROM_LOAD( "046-v4.bin", 0x300000, 0x100000, CRC(4f54c187) SHA1(63a76949301b83bdd44aa1a4462f642ab9ca3c0b) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "046-c1.bin", 0x000000, 0x100000, CRC(fa720a4a) SHA1(364913b9fa40d46e4e39ae3cdae914cfd0de137d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "046-c2.bin", 0x000001, 0x100000, CRC(c900c205) SHA1(50274e79aa26f334eb806288688b30720bade883) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "046-c3.bin", 0x200000, 0x100000, CRC(880e3c25) SHA1(b6974af0c833b766866919b6f15b6f8cef82530d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "046-c4.bin", 0x200001, 0x100000, CRC(7a2248aa) SHA1(8af0b26025a54e3b91604dd24a3c1c518fbd8536) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "046-c5.bin", 0x400000, 0x100000, CRC(4b735e45) SHA1(2f8b46388c4696aee6a97e1e21cdadf6b142b01a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "046-c6.bin", 0x400001, 0x100000, CRC(273171df) SHA1(9c35832221e016c12ef1ed71da167f565daaf86c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "046-c7.bin", 0x600000, 0x100000, CRC(12829c4c) SHA1(ac5f3d848d7116fc35c97f53a72c85e049dd3a2f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "046-c8.bin", 0x600001, 0x100000, CRC(c944e03d) SHA1(be23999b8ce09ee15ba500ce4d5e2a82a4f58d9b) ) /* Plane 2,3 */
ROM_END

ROM_START( tophuntrh ) /* AES VERSION */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "046-pg1.bin",0x000000, 0x100000, CRC(771e39bc) SHA1(c0e05fd1ca81926438bb75e2fa6894e40ab6521e) )
	ROM_LOAD16_WORD_SWAP( "046-p2.bin", 0x100000, 0x100000, CRC(f182cb3e) SHA1(6b4e0af5d4e623f0682f37ff5c69e5b705e20028) )

	NEO_SFIX_128K( "046-sg1.bin", CRC(92e9ac8c) SHA1(cab5c77c091e8d12d9c3a2cc8d741b74e4386efb) )

	NEO_BIOS_AUDIO_128K( "046-m1.bin", CRC(3f84bb9f) SHA1(07446040871d11da3c2217ee9d1faf8c3cae7420) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "046-v1.bin", 0x000000, 0x100000, CRC(c1f9c2db) SHA1(bed95a76afefa46503a12e0f0a9787c4c967ac50) )
	ROM_LOAD( "046-v2.bin", 0x100000, 0x100000, CRC(56254a64) SHA1(1cf049cb4c414419859d2c8ee714317a35a85251) )
	ROM_LOAD( "046-v3.bin", 0x200000, 0x100000, CRC(58113fb1) SHA1(40972982a63c7adecef840f9882f4165da723ab6) )
	ROM_LOAD( "046-v4.bin", 0x300000, 0x100000, CRC(4f54c187) SHA1(63a76949301b83bdd44aa1a4462f642ab9ca3c0b) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "046-c1.bin", 0x000000, 0x100000, CRC(fa720a4a) SHA1(364913b9fa40d46e4e39ae3cdae914cfd0de137d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "046-c2.bin", 0x000001, 0x100000, CRC(c900c205) SHA1(50274e79aa26f334eb806288688b30720bade883) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "046-c3.bin", 0x200000, 0x100000, CRC(880e3c25) SHA1(b6974af0c833b766866919b6f15b6f8cef82530d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "046-c4.bin", 0x200001, 0x100000, CRC(7a2248aa) SHA1(8af0b26025a54e3b91604dd24a3c1c518fbd8536) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "046-c5.bin", 0x400000, 0x100000, CRC(4b735e45) SHA1(2f8b46388c4696aee6a97e1e21cdadf6b142b01a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "046-c6.bin", 0x400001, 0x100000, CRC(273171df) SHA1(9c35832221e016c12ef1ed71da167f565daaf86c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "046-c7.bin", 0x600000, 0x100000, CRC(12829c4c) SHA1(ac5f3d848d7116fc35c97f53a72c85e049dd3a2f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "046-c8.bin", 0x600001, 0x100000, CRC(c944e03d) SHA1(be23999b8ce09ee15ba500ce4d5e2a82a4f58d9b) ) /* Plane 2,3 */
ROM_END

ROM_START( fatfury2 ) /* MVS AND AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "047-p1.bin", 0x000000, 0x080000, CRC(be40ea92) SHA1(958b891bb8beb6af122d5467257ab20cbc6cf574) )
	ROM_LOAD16_WORD_SWAP( "047-p2.bin", 0x080000, 0x080000, CRC(2a9beac5) SHA1(61378f89d64fef4c172825694f83229c2ab1a7af) )

	NEO_SFIX_128K( "047-s1.bin", CRC(d7dbbf39) SHA1(29253e596f475ebd41a6e3bb53952e3a0ccd2eed) )

	NEO_BIOS_AUDIO_128K( "047-m1.bin", CRC(820b0ba7) SHA1(5708248d89446e49184eaadb52f7c61b2b6c13c5) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "047-v1.bin", 0x000000, 0x200000, CRC(d9d00784) SHA1(f6a91eada8c23aa4518c4b82eeebca69f79d845c) )
	ROM_LOAD( "047-v2.bin", 0x200000, 0x200000, CRC(2c9a4b33) SHA1(d4a1c0951c02c8919b3ec32ed96933634ff9e54c) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "047-c1.bin", 0x000000, 0x100000, CRC(f72a939e) SHA1(67fc398ec28061adca0d3be82bbe7297015800da) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "047-c2.bin", 0x000001, 0x100000, CRC(05119a0d) SHA1(c2f100b73eb04f65b6ba6089d49aceb51b470ec6) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
	ROM_LOAD16_BYTE( "047-c3.bin", 0x200000, 0x100000, CRC(01e00738) SHA1(79654f24d777dd5eb68bafc3b8cb9db71d5822e2) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x600000, 0x100000 )
	ROM_LOAD16_BYTE( "047-c4.bin", 0x200001, 0x100000, CRC(9fe27432) SHA1(89d22d77ba8bc6d1f6c974195c34ad61b9010de7) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x600001, 0x100000 )
ROM_END

ROM_START( janshin )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "048-p1.bin", 0x000000, 0x100000, CRC(7514cb7a) SHA1(da512c0a8e8160a9db7f956e351245327c38eaf1) )

	NEO_SFIX_128K( "048-s1.bin", CRC(8285b25a) SHA1(d983640cda3e346e38469b4d3ec8048b116a7bb7) )

	NEO_BIOS_AUDIO_128K( "048-m1.bin", CRC(310467c7) SHA1(c529961195c9bdf5d1ce70a38ad129002d1f3b5f) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "048-v1.bin", 0x000000, 0x200000, CRC(f1947d2b) SHA1(955ff91ab24eb2a7ec51ff46c9f9f2ec060456b2) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "048-c1.bin", 0x000000, 0x200000, CRC(3fa890e9) SHA1(e73d2802bacfbc2b2b16fbbedddde17488e4bbde) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "048-c2.bin", 0x000001, 0x200000, CRC(59c48ad8) SHA1(2630817e735a6d197377558f4324c1442803fe15) ) /* Plane 2,3 */
ROM_END

ROM_START( androdun )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "049-p1.bin", 0x000000, 0x080000, CRC(3b857da2) SHA1(4dd86c739944696c16c3cdd85935d6dfa9fdc276) )
	ROM_LOAD16_WORD_SWAP( "049-p2.bin", 0x080000, 0x080000, CRC(2f062209) SHA1(991cf3e3677929b2cc0b2787b0c7b6ad3700f618) )

	NEO_SFIX_128K( "049-s1.bin", CRC(6349de5d) SHA1(bcc44b9576d7bedd9a39294530bb66f707690c72) )

	NEO_BIOS_AUDIO_128K( "049-m1.bin", CRC(edd2acf4) SHA1(c4ee6ba834d54b9fc5a854dbc41a05877e090371) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "049-v1.bin", 0x000000, 0x100000, CRC(ce43cb89) SHA1(47f82e077abb6efc6b1b0490412ae147d5d2acef) )

	NO_DELTAT_REGION

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "049-c1.bin", 0x000000, 0x100000, CRC(7ace6db3) SHA1(c41cc9de8c0788dcc49ca494fd3bb3124062d9dd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "049-c2.bin", 0x000001, 0x100000, CRC(b17024f7) SHA1(fcf7efae48fcdccaf5255c145de414fb246128f0) ) /* Plane 2,3 */
ROM_END

ROM_START( ncommand )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "050-p1.bin", 0x000000, 0x100000, CRC(4e097c40) SHA1(43311a7ca14a14dcd4a99d8576a12e897b078643) )

	NEO_SFIX_128K( "050-s1.bin", CRC(db8f9c8e) SHA1(11cb82cf3c4d0fc2da5df0c26410a64808093610) )

	NEO_BIOS_AUDIO_128K( "050-m1.bin", CRC(6fcf07d3) SHA1(e9ecff4bfec1f5964bf06645f75d80d611b6231c) )

	ROM_REGION( 0x180000, "ymsnd", 0 )
	ROM_LOAD( "050-v1.bin", 0x000000, 0x100000, CRC(23c3ab42) SHA1(b6c59bb180f1aa34c95f3ec923f3aafb689d57b0) )
	ROM_LOAD( "050-v2.bin", 0x100000, 0x080000, CRC(80b8a984) SHA1(950cf0e78ceffa4037663f1086fbbc88588f49f2) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "050-c1.bin", 0x000000, 0x100000, CRC(87421a0a) SHA1(1d8faaf03778f7c5b062554d7333bbd3f0ca12ad) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "050-c2.bin", 0x000001, 0x100000, CRC(c4cf5548) SHA1(ef9eca5aeff9dda2209a050c2af00ed8979ae2bc) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "050-c3.bin", 0x200000, 0x100000, CRC(03422c1e) SHA1(920e5015aebe2ffc5ce43a52365c7f0a705f3b9e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "050-c4.bin", 0x200001, 0x100000, CRC(0845eadb) SHA1(3c71a02bf0e07a5381846bb6d75bbe7dd546adea) ) /* Plane 2,3 */
ROM_END

ROM_START( viewpoin )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "051-p1.bin", 0x000000, 0x100000, CRC(17aa899d) SHA1(674cd8ace7acdf4f407de741e3d0071bcb49c902) )

	NEO_SFIX_128K( "051-s1.bin", CRC(9fea5758) SHA1(5c6f01da89f2639cf741ee7c39e27023b8083052) )

	NEO_BIOS_AUDIO_128K( "051-m1.bin", CRC(8e69f29a) SHA1(7a25f4997996434ea1b7d0d1ca9e7aaf966cbd03) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "051-v1.bin", 0x000000, 0x200000, CRC(019978b6) SHA1(7896a551115fc6ed38b5944e0c8dcb2b2c1c077d) )
	ROM_LOAD( "051-v2.bin", 0x200000, 0x200000, CRC(5758f38c) SHA1(da10f4b7d22d9139bbf068bd940be82168a74ca1) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "051-c1.bin", 0x000000, 0x100000, CRC(d624c132) SHA1(49c7e9f020cba45d7083b45252bcc03397f8c286) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "051-c2.bin", 0x000001, 0x100000, CRC(40d69f1e) SHA1(ec4a13582772594957f927622d50f54b0dfcd8d8) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
ROM_END

ROM_START( ssideki )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "052-p1.bin", 0x000000, 0x080000, CRC(9cd97256) SHA1(1c780b711137fd79cc81b01941e84f3d59e0071f) )

	NEO_SFIX_128K( "052-s1.bin", CRC(97689804) SHA1(fa8dab3b3353d7115a0368f3fc749950c0186fbc) )

	NEO_BIOS_AUDIO_128K( "052-m1.bin", CRC(49f17d2d) SHA1(70971fcf71ae3a6b2e26e7ade8063941fb178ae5) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "052-v1.bin", 0x000000, 0x200000, CRC(22c097a5) SHA1(328c4e6db0a026f54a633cff1443a3f964a8daea) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "052-c1.bin", 0x000000, 0x100000, CRC(53e1c002) SHA1(2125b1be379ea7933893ffb1cd65d6c4bf8b03bd) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "052-c2.bin", 0x000001, 0x100000, CRC(776a2d1f) SHA1(bca0bac87443e9e78c623d284f6cc96cc9c9098f) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
ROM_END

ROM_START( wh1 ) /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "053-epr.p1", 0x000000, 0x080000, CRC(d42e1e9a) SHA1(9d1911fe4bf6202466ec45557dd008cbf01ca9c6) )
	ROM_LOAD16_WORD_SWAP( "053-epr.p2", 0x080000, 0x080000, CRC(0e33e8a3) SHA1(4b7086edb504f3c30529d51ba8f453d48eba5164) )
	/* P's on eprom, correct chip label unknown */

	NEO_SFIX_128K( "053-s1.bin", CRC(8c2c2d6b) SHA1(87fa79611c6f8886dcc8766814829c669c65b40f) )

	NEO_BIOS_AUDIO_128K( "053-m1.bin", CRC(1bd9d04b) SHA1(65cd7b002123ed1a3111e3d942608d0082799ff3) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "053-v2.bin", 0x000000, 0x200000, CRC(a68df485) SHA1(007fa715423fba72c899cd3db3f4bec13281cf7a) )
	ROM_LOAD( "053-v4.bin", 0x200000, 0x100000, CRC(7bea8f66) SHA1(428e8721bd87f7faa756adb1e12672219be46c1d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "053-c1.bin", 0x000000, 0x100000, CRC(85eb5bce) SHA1(3d03d29296ca6e6b5106aac4aaeec9d4b4ed1313) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "053-c2.bin", 0x000001, 0x100000, CRC(ec93b048) SHA1(d4159210df94e259f874a4671d271ec27be13451) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
	ROM_LOAD16_BYTE( "053-c3.bin", 0x200000, 0x100000, CRC(0dd64965) SHA1(e97b3b8a461da5e8861b3dfdacb25e007ced37a1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "053-c4.bin", 0x200001, 0x100000, CRC(9270d954) SHA1(a2ef909868f6b06cdcc22a63ddf6c96be12b999c) ) /* Plane 2,3 */
ROM_END

ROM_START( wh1h ) /* AES VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "053-p1.bin", 0x000000, 0x080000, CRC(95b574cb) SHA1(b7b7af6a04c3d902e7f8852897741ecaf0b1062c) )
	ROM_LOAD16_WORD_SWAP( "053-p2.bin", 0x080000, 0x080000, CRC(f198ed45) SHA1(24ccc091e97f63796562bb5b30df51f39bd504ef) )

	NEO_SFIX_128K( "053-s1.bin", CRC(8c2c2d6b) SHA1(87fa79611c6f8886dcc8766814829c669c65b40f) )

	NEO_BIOS_AUDIO_128K( "053-m1.bin", CRC(1bd9d04b) SHA1(65cd7b002123ed1a3111e3d942608d0082799ff3) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "053-v2.bin", 0x000000, 0x200000, CRC(a68df485) SHA1(007fa715423fba72c899cd3db3f4bec13281cf7a) )
	ROM_LOAD( "053-v4.bin", 0x200000, 0x100000, CRC(7bea8f66) SHA1(428e8721bd87f7faa756adb1e12672219be46c1d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "053-c1.bin", 0x000000, 0x100000, CRC(85eb5bce) SHA1(3d03d29296ca6e6b5106aac4aaeec9d4b4ed1313) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "053-c2.bin", 0x000001, 0x100000, CRC(ec93b048) SHA1(d4159210df94e259f874a4671d271ec27be13451) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
	ROM_LOAD16_BYTE( "053-c3.bin", 0x200000, 0x100000, CRC(0dd64965) SHA1(e97b3b8a461da5e8861b3dfdacb25e007ced37a1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "053-c4.bin", 0x200001, 0x100000, CRC(9270d954) SHA1(a2ef909868f6b06cdcc22a63ddf6c96be12b999c) ) /* Plane 2,3 */
ROM_END

ROM_START( wh1ha )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "053-p1a.bin", 0x000000, 0x080000, CRC(ed29fde2) SHA1(52b8ca5b804f786f95e1dfb348d8c7b82f1d4ddf) )
	ROM_LOAD16_WORD_SWAP( "053-p2a.bin", 0x080000, 0x080000, CRC(98f2b158) SHA1(a64e1425970eb53cc910891db39973dee3d54ccc) )

	NEO_SFIX_128K( "053-s1.bin", CRC(8c2c2d6b) SHA1(87fa79611c6f8886dcc8766814829c669c65b40f) )

	NEO_BIOS_AUDIO_128K( "053-m1.bin", CRC(1bd9d04b) SHA1(65cd7b002123ed1a3111e3d942608d0082799ff3) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "053-v2.bin", 0x000000, 0x200000, CRC(a68df485) SHA1(007fa715423fba72c899cd3db3f4bec13281cf7a) )
	ROM_LOAD( "053-v4.bin", 0x200000, 0x100000, CRC(7bea8f66) SHA1(428e8721bd87f7faa756adb1e12672219be46c1d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "053-c1.bin", 0x000000, 0x100000, CRC(85eb5bce) SHA1(3d03d29296ca6e6b5106aac4aaeec9d4b4ed1313) ) /* Plane 0,1 */
	ROM_CONTINUE( 0x400000, 0x100000 )
	ROM_LOAD16_BYTE( "053-c2.bin", 0x000001, 0x100000, CRC(ec93b048) SHA1(d4159210df94e259f874a4671d271ec27be13451) ) /* Plane 2,3 */
	ROM_CONTINUE( 0x400001, 0x100000 )
	ROM_LOAD16_BYTE( "053-c3.bin", 0x200000, 0x100000, CRC(0dd64965) SHA1(e97b3b8a461da5e8861b3dfdacb25e007ced37a1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "053-c4.bin", 0x200001, 0x100000, CRC(9270d954) SHA1(a2ef909868f6b06cdcc22a63ddf6c96be12b999c) ) /* Plane 2,3 */
ROM_END

ROM_START( kof94 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "055-p1.bin", 0x100000, 0x100000, CRC(f10a2042) SHA1(d08a3f3c28be4b1793de7d362456281329fe1828) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "055-s1.bin", CRC(825976c1) SHA1(cb6a70bdd95d449d25196ca269b621c362db6743) )

	NEO_BIOS_AUDIO_128K( "055-m1.bin", CRC(f6e77cf5) SHA1(292a3e3a4918ffe72bd1c41acb927b91844e035e) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "055-v1.bin", 0x000000, 0x200000, CRC(8889596d) SHA1(c9ce713b720511438dbd3fe3bcc7c246f475c6a2) )
	ROM_LOAD( "055-v2.bin", 0x200000, 0x200000, CRC(25022b27) SHA1(2b040a831c3c92ac6e4719de38009a0d55b64f6b) )
	ROM_LOAD( "055-v3.bin", 0x400000, 0x200000, CRC(83cf32c0) SHA1(34a31a37eb10945b5169e96321bcea06eec33a00) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "055-c1.bin", 0x000000, 0x200000, CRC(b96ef460) SHA1(e52f5303c17b50ce165c008be2837336369c110b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "055-c2.bin", 0x000001, 0x200000, CRC(15e096a7) SHA1(237c2a3d059de00bfca66e0016ed325d7a32bfec) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "055-c3.bin", 0x400000, 0x200000, CRC(54f66254) SHA1(c594384bcd8b03beb8c595591505fecc44b185ac) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "055-c4.bin", 0x400001, 0x200000, CRC(0b01765f) SHA1(ec1fdcc944611408367bf5023d4ebe7edd9dfa88) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "055-c5.bin", 0x800000, 0x200000, CRC(ee759363) SHA1(8a5621c1b1f8267b9b9b6a14ab4944de542e1945) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "055-c6.bin", 0x800001, 0x200000, CRC(498da52c) SHA1(1e6e6202ee053a5261db889177ce3a087e078bda) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "055-c7.bin", 0xc00000, 0x200000, CRC(62f66888) SHA1(ac91a0eab0753bee175ad40213a4ae5d38ed5b87) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "055-c8.bin", 0xc00001, 0x200000, CRC(fe0a235d) SHA1(a45c66836e4e3c77dfef9d4c6cc422cb59169149) ) /* Plane 2,3 */
ROM_END

ROM_START( aof2 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "056-p1.bin", 0x000000, 0x100000, CRC(a3b1d021) SHA1(ee42f3ca4516226b0088d0303ed28e3ecdabcd71) )

	NEO_SFIX_128K( "056-s1.bin", CRC(8b02638e) SHA1(aa4d28804ca602da776948b5f223ea89e427906b) )

	NEO_BIOS_AUDIO_128K( "056-m1.bin", CRC(f27e9d52) SHA1(dddae733d87ce7c88ad2580a8f64cb6ff9572e67) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "056-v1.bin", 0x000000, 0x200000, CRC(4628fde0) SHA1(ece2a50f5270d844d58401b1447d1d856d78ea45) )
	ROM_LOAD( "056-v2.bin", 0x200000, 0x200000, CRC(b710e2f2) SHA1(df4da585203eea7554d3ce718eb107e9cb6a0254) )
	ROM_LOAD( "056-v3.bin", 0x400000, 0x100000, CRC(d168c301) SHA1(969273d1d11943e81560959359a2c4e69522af0e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "056-c1.bin", 0x000000, 0x200000, CRC(17b9cbd2) SHA1(1eee81e02763d384bd1c10a6012473ca931e4093) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "056-c2.bin", 0x000001, 0x200000, CRC(5fd76b67) SHA1(11925a41a53b53c6df4a5ebd28f98300950f743b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "056-c3.bin", 0x400000, 0x200000, CRC(d2c88768) SHA1(22e2d84aa0c095944190e249ce87ef50d3f7b8ce) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "056-c4.bin", 0x400001, 0x200000, CRC(db39b883) SHA1(59de86c513dc4e230ae25d9e3b7e84621b657b54) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "056-c5.bin", 0x800000, 0x200000, CRC(c3074137) SHA1(9a75e3d63cb98d54f900dcfb3a03e21f3148d32f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "056-c6.bin", 0x800001, 0x200000, CRC(31de68d3) SHA1(13ba7046cdd6863125f8284e60f102d4720af5a4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "056-c7.bin", 0xc00000, 0x200000, CRC(3f36df57) SHA1(79ee97e9ae811a51141b535633f90e1491209d54) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "056-c8.bin", 0xc00001, 0x200000, CRC(e546d7a8) SHA1(74a2fca994a5a93a5784a46c0f68193122456a09) ) /* Plane 2,3 */
ROM_END

ROM_START( aof2a ) /* make parent? */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "056-p1.bin",  0x000000, 0x100000, CRC(a3b1d021) SHA1(ee42f3ca4516226b0088d0303ed28e3ecdabcd71) )
	/* the rom below acts as a patch to the program rom in the cart, replacing the first 512kb */
	ROM_LOAD16_WORD_SWAP( "056-ep1.bin", 0x000000, 0x80000, CRC(75d6301c) SHA1(e72d15fba55f96be7b4fa29e705a7b78f56edf7d) )

	NEO_SFIX_128K( "056-s1.bin", CRC(8b02638e) SHA1(aa4d28804ca602da776948b5f223ea89e427906b) )

	NEO_BIOS_AUDIO_128K( "056-m1.bin", CRC(f27e9d52) SHA1(dddae733d87ce7c88ad2580a8f64cb6ff9572e67) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "056-v1.bin", 0x000000, 0x200000, CRC(4628fde0) SHA1(ece2a50f5270d844d58401b1447d1d856d78ea45) )
	ROM_LOAD( "056-v2.bin", 0x200000, 0x200000, CRC(b710e2f2) SHA1(df4da585203eea7554d3ce718eb107e9cb6a0254) )
	ROM_LOAD( "056-v3.bin", 0x400000, 0x100000, CRC(d168c301) SHA1(969273d1d11943e81560959359a2c4e69522af0e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "056-c1.bin", 0x000000, 0x200000, CRC(17b9cbd2) SHA1(1eee81e02763d384bd1c10a6012473ca931e4093) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "056-c2.bin", 0x000001, 0x200000, CRC(5fd76b67) SHA1(11925a41a53b53c6df4a5ebd28f98300950f743b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "056-c3.bin", 0x400000, 0x200000, CRC(d2c88768) SHA1(22e2d84aa0c095944190e249ce87ef50d3f7b8ce) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "056-c4.bin", 0x400001, 0x200000, CRC(db39b883) SHA1(59de86c513dc4e230ae25d9e3b7e84621b657b54) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "056-c5.bin", 0x800000, 0x200000, CRC(c3074137) SHA1(9a75e3d63cb98d54f900dcfb3a03e21f3148d32f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "056-c6.bin", 0x800001, 0x200000, CRC(31de68d3) SHA1(13ba7046cdd6863125f8284e60f102d4720af5a4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "056-c7.bin", 0xc00000, 0x200000, CRC(3f36df57) SHA1(79ee97e9ae811a51141b535633f90e1491209d54) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "056-c8.bin", 0xc00001, 0x200000, CRC(e546d7a8) SHA1(74a2fca994a5a93a5784a46c0f68193122456a09) ) /* Plane 2,3 */
ROM_END

ROM_START( wh2 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "057-p1.bin", 0x100000, 0x100000, CRC(65a891d9) SHA1(ff8d5ccb0dd22c523902bb3db3c645583a335056) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "057-s1.bin", CRC(fcaeb3a4) SHA1(1f3f85e38b8552333261c04ae5af0d6e3b310622) )

	NEO_BIOS_AUDIO_128K( "057-m1.bin", CRC(8fa3bc77) SHA1(982f92978671e4ee66630948e6bb7565b37b5dc0) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "057-v1.bin", 0x000000, 0x200000, CRC(8877e301) SHA1(1bab988d74ea8fd12db201c257ec844622cf5f4e) )
	ROM_LOAD( "057-v2.bin", 0x200000, 0x200000, CRC(c1317ff4) SHA1(4c28b2b5998abaeaa5143f2f3a9ba52c6041f4f3) )

	NO_DELTAT_REGION

	ROM_REGION( 0xc00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "057-c1.bin", 0x000000, 0x200000, CRC(21c6bb91) SHA1(a2c17d0c91dd59528d8fa7fe110af8b20b25ff99) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "057-c2.bin", 0x000001, 0x200000, CRC(a3999925) SHA1(0ee861a77850d378d03c1bf00b9692abd860c759) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "057-c3.bin", 0x400000, 0x200000, CRC(b725a219) SHA1(4857687d156a9150a69b97d2729245a51c144a0c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "057-c4.bin", 0x400001, 0x200000, CRC(8d96425e) SHA1(0f79c868a6a33ad25e38d842f30ec4440d809033) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "057-c5.bin", 0x800000, 0x200000, CRC(b20354af) SHA1(da7609fd467f2f4d71d92970f438a04d11ab1cc1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "057-c6.bin", 0x800001, 0x200000, CRC(b13d1de3) SHA1(7d749c23a33d90fe50279e884540d71cf1aaaa6b) ) /* Plane 2,3 */
ROM_END

ROM_START( fatfursp )
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "058-p1.bin", 0x000000, 0x100000, CRC(2f585ba2) SHA1(429b4bf43fb9b1082c15d645ca328f9d175b976b) )
	ROM_LOAD16_WORD_SWAP( "058-p2.bin", 0x100000, 0x080000, CRC(d7c71a6b) SHA1(b3428063031a2e5857da40a5d2ffa87fb550c1bb) )

	NEO_SFIX_128K( "058-s1.bin", CRC(2df03197) SHA1(24083cfc97e720ac9e131c9fe37df57e27c49294) )

	NEO_BIOS_AUDIO_128K( "058-m1.bin", CRC(ccc5186e) SHA1(cf9091c523c182aebfb928c91640b2d72fd70123) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "058-v1.bin", 0x000000, 0x200000, CRC(55d7ce84) SHA1(05ac6a395d9bf9166925acca176a8d6129f533c8) )
	ROM_LOAD( "058-v2.bin", 0x200000, 0x200000, CRC(ee080b10) SHA1(29814fc21bbe30d37745c8918fab00c83a309be4) )
	ROM_LOAD( "058-v3.bin", 0x400000, 0x100000, CRC(f9eb3d4a) SHA1(d1747f9460b965f6daf4f881ed4ecd04c5253434) )

	NO_DELTAT_REGION

	ROM_REGION( 0xc00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "058-c1.bin", 0x000000, 0x200000, CRC(044ab13c) SHA1(569d283638a132bc163faac2a9055497017ee0d2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "058-c2.bin", 0x000001, 0x200000, CRC(11e6bf96) SHA1(c093a4f93f13e07b276e28b30c2a14dda9135d8f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "058-c3.bin", 0x400000, 0x200000, CRC(6f7938d5) SHA1(be057b0a3faeb76d5fff161d3e6fea8a26e11d2c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "058-c4.bin", 0x400001, 0x200000, CRC(4ad066ff) SHA1(4e304646d954d5f7bbabc5d068e85de31d38830f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "058-c5.bin", 0x800000, 0x200000, CRC(49c5e0bf) SHA1(f3784178f90751990ea47a082a6aa869ee3566c9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "058-c6.bin", 0x800001, 0x200000, CRC(8ff1f43d) SHA1(6180ceb5412a3e2e34e9513a3283b9f63087f747) ) /* Plane 2,3 */
ROM_END

ROM_START( fatfursa )
	ROM_REGION( 0x180000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "058-p1.bin", 0x000000, 0x100000, CRC(2f585ba2) SHA1(429b4bf43fb9b1082c15d645ca328f9d175b976b) )
	ROM_LOAD16_WORD_SWAP( "058-p2.bin", 0x100000, 0x080000, CRC(d7c71a6b) SHA1(b3428063031a2e5857da40a5d2ffa87fb550c1bb) )
	/* the rom below acts as a patch to the program rom in the cart, replacing the first 512kb */
	ROM_LOAD16_WORD_SWAP( "058-ep1.bin", 0x000000, 0x080000, CRC(9f0c1e1a) SHA1(02861b0f230541becccc3df6a2c85dbe8733e7ce) )

	NEO_SFIX_128K( "058-s1.bin", CRC(2df03197) SHA1(24083cfc97e720ac9e131c9fe37df57e27c49294) )

	NEO_BIOS_AUDIO_128K( "058-m1.bin", CRC(ccc5186e) SHA1(cf9091c523c182aebfb928c91640b2d72fd70123) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "058-v1.bin", 0x000000, 0x200000, CRC(55d7ce84) SHA1(05ac6a395d9bf9166925acca176a8d6129f533c8) )
	ROM_LOAD( "058-v2.bin", 0x200000, 0x200000, CRC(ee080b10) SHA1(29814fc21bbe30d37745c8918fab00c83a309be4) )
	ROM_LOAD( "058-v3.bin", 0x400000, 0x100000, CRC(f9eb3d4a) SHA1(d1747f9460b965f6daf4f881ed4ecd04c5253434) )

	NO_DELTAT_REGION

	ROM_REGION( 0xc00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "058-c1.bin", 0x000000, 0x200000, CRC(044ab13c) SHA1(569d283638a132bc163faac2a9055497017ee0d2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "058-c2.bin", 0x000001, 0x200000, CRC(11e6bf96) SHA1(c093a4f93f13e07b276e28b30c2a14dda9135d8f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "058-c3.bin", 0x400000, 0x200000, CRC(6f7938d5) SHA1(be057b0a3faeb76d5fff161d3e6fea8a26e11d2c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "058-c4.bin", 0x400001, 0x200000, CRC(4ad066ff) SHA1(4e304646d954d5f7bbabc5d068e85de31d38830f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "058-c5.bin", 0x800000, 0x200000, CRC(49c5e0bf) SHA1(f3784178f90751990ea47a082a6aa869ee3566c9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "058-c6.bin", 0x800001, 0x200000, CRC(8ff1f43d) SHA1(6180ceb5412a3e2e34e9513a3283b9f63087f747) ) /* Plane 2,3 */
ROM_END

ROM_START( savagere )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "059-p1.bin", 0x100000, 0x100000, CRC(01d4e9c0) SHA1(3179d2be59bf2de6918d506117cff50acf7e09f3) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "059-s1.bin", CRC(e08978ca) SHA1(55152cb9bd0403ae8656b93a6b1522dba5db6d1a) )

	NEO_BIOS_AUDIO_128K( "059-m1.bin", CRC(29992eba) SHA1(187be624abe8670503edb235ff21ae8fdc3866e0) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "059-v1.bin", 0x000000, 0x200000, CRC(530c50fd) SHA1(29401cee7f7d2c199c7cb58092e86b28205e81ad) )
	ROM_LOAD( "059-v2.bin", 0x200000, 0x200000, CRC(eb6f1cdb) SHA1(7a311388315ea543babf872f62219fdc4d39d013) )
	ROM_LOAD( "059-v3.bin", 0x400000, 0x200000, CRC(7038c2f9) SHA1(c1d6f86b24feba03fe009b58199d2eeabe572f4e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "059-c1.bin", 0x000000, 0x200000, CRC(763ba611) SHA1(d3262e0332c894ee149c5963f882cc5e5562ee57) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "059-c2.bin", 0x000001, 0x200000, CRC(e05e8ca6) SHA1(986a9b16ff92bc101ab567d2d01348e093abea9a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "059-c3.bin", 0x400000, 0x200000, CRC(3e4eba4b) SHA1(770adec719e63a30ebe9522cc7576caaca44f3b2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "059-c4.bin", 0x400001, 0x200000, CRC(3c2a3808) SHA1(698adcec0715c9e78b6286be38debf0ce28fd644) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "059-c5.bin", 0x800000, 0x200000, CRC(59013f9e) SHA1(5bf48fcc450da72a8c4685f6e3887e67eae49988) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "059-c6.bin", 0x800001, 0x200000, CRC(1c8d5def) SHA1(475d89a5c4922a9f6bd756d23c2624d57b6e9d62) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "059-c7.bin", 0xc00000, 0x200000, CRC(c88f7035) SHA1(c29a428b741f4fe7b71a3bc23c87925b6bc1ca8f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "059-c8.bin", 0xc00001, 0x200000, CRC(484ce3ba) SHA1(4f21ed20ce6e2b67e2b079404599310c94f591ff) ) /* Plane 2,3 */
ROM_END

ROM_START( fightfev )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "060-p1.bin", 0x000000, 0x080000, CRC(3032041b) SHA1(4b8ed2e6f74579ea35a53e06ccac42d6905b0f51) )
	ROM_LOAD16_WORD_SWAP( "060-p2.bin", 0x080000, 0x080000, CRC(b0801d5f) SHA1(085746d8f5d271d5f84ccbb7f577193c391f88d4) )

	NEO_SFIX_128K( "060-s1.bin", CRC(7f012104) SHA1(f366dcc3923655dff16ec08a40d5fce22a84257d) )

	NEO_BIOS_AUDIO_128K( "060-m1.bin", CRC(0b7c4e65) SHA1(999a1e784de18db3f1332b30bc425836ea6970be) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "060-v1.bin", 0x000000, 0x200000, CRC(f417c215) SHA1(0f53b8dd056f43b5d880628e8b74c2b27881ffac) )
	ROM_LOAD( "060-v2.bin", 0x200000, 0x100000, CRC(efcff7cf) SHA1(e8372303724284a750b706dc6bf7641e4c52bb95) )

	NO_DELTAT_REGION

	ROM_REGION( 0x0800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "060-c1.bin", 0x0000000, 0x200000, CRC(8908fff9) SHA1(f8c16ab0248b60f3a62e0d4d65c456e2f8e4da49) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "060-c2.bin", 0x0000001, 0x200000, CRC(c6649492) SHA1(5d39b077387ed6897ac075ede4a2aa94bb64545e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "060-c3.bin", 0x0400000, 0x200000, CRC(0956b437) SHA1(c70be8b5cebf321afe4c3f5e9a12413c3077694a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "060-c4.bin", 0x0400001, 0x200000, CRC(026f3b62) SHA1(d608483b70d60e7aa0e41f25a8b3fed508129eb7) ) /* Plane 2,3 */
ROM_END

ROM_START( fightfeva ) /* MVS VERSION */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "060-p1.rom", 0x0000000, 0x100000, CRC(2a104b50) SHA1(3eb663d3df7074e1cdf4c0e450a35c9cf55d8979) )
	/* There was also a copy of the 060-p1.bin with the name 060-p2.bin maybe it should be loaded over the top or this
    larger rom is an older revision... */

	NEO_SFIX_128K( "060-s1.bin", CRC(7f012104) SHA1(f366dcc3923655dff16ec08a40d5fce22a84257d) )

	NEO_BIOS_AUDIO_128K( "060-m1.bin", CRC(0b7c4e65) SHA1(999a1e784de18db3f1332b30bc425836ea6970be) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "060-v1.bin", 0x000000, 0x200000, CRC(f417c215) SHA1(0f53b8dd056f43b5d880628e8b74c2b27881ffac) )
	ROM_LOAD( "060-v2.bin", 0x200000, 0x100000, CRC(efcff7cf) SHA1(e8372303724284a750b706dc6bf7641e4c52bb95) )

	NO_DELTAT_REGION

	ROM_REGION( 0x0800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "060-c1.bin", 0x0000000, 0x200000, CRC(8908fff9) SHA1(f8c16ab0248b60f3a62e0d4d65c456e2f8e4da49) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "060-c2.bin", 0x0000001, 0x200000, CRC(c6649492) SHA1(5d39b077387ed6897ac075ede4a2aa94bb64545e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "060-c3.bin", 0x0400000, 0x200000, CRC(0956b437) SHA1(c70be8b5cebf321afe4c3f5e9a12413c3077694a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "060-c4.bin", 0x0400001, 0x200000, CRC(026f3b62) SHA1(d608483b70d60e7aa0e41f25a8b3fed508129eb7) ) /* Plane 2,3 */
ROM_END

ROM_START( ssideki2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "061-p1.bin", 0x000000, 0x100000, CRC(5969e0dc) SHA1(78abea880c125ec5a85bef6404478512a34b5513) )

	NEO_SFIX_128K( "061-s1.bin", CRC(226d1b68) SHA1(de010f6fda3ddadb181fe37daa6105f22e78b970) )

	NEO_BIOS_AUDIO_128K( "061-m1.bin", CRC(156f6951) SHA1(49686f615f109a02b4f23931f1c84fee13872ffd) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "061-v1.bin", 0x000000, 0x200000, CRC(f081c8d3) SHA1(fc9da0ddc1ddd1f9ae1443a726815c25e9dc38ae) )
	ROM_LOAD( "061-v2.bin", 0x200000, 0x200000, CRC(7cd63302) SHA1(c39984c0ae0a8e76f1fc036344bbb83635c18937) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "061-c1-16.bin", 0x000000, 0x200000, CRC(a626474f) SHA1(d695f0dcb9480088b3a7c1488bd541b4c159528a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "061-c2-16.bin", 0x000001, 0x200000, CRC(c3be42ae) SHA1(7fa65538bd0a0a162e4d3e9f49913da59d915e02) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "061-c3-16.bin", 0x400000, 0x200000, CRC(2a7b98b9) SHA1(75e1019dca8a8583afcc53651ac856cba3a96315) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "061-c4-16.bin", 0x400001, 0x200000, CRC(c0be9a1f) SHA1(228f41eaefdf3e147761f8ef849e3b5f321877d4) ) /* Plane 2,3 */
ROM_END

ROM_START( spinmast )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "062-p1.bin", 0x000000, 0x100000, CRC(37aba1aa) SHA1(1a2ab9593371cc2f665121d554eec3f6bb4d09ff) )
	ROM_LOAD16_WORD_SWAP( "062-p2.bin", 0x100000, 0x100000, CRC(f025ab77) SHA1(4944be04648296d0b93cfe4c5ca7b9cede072cff) )

	NEO_SFIX_128K( "062-s1.bin", CRC(289e2bbe) SHA1(f52c7f2bffc89df3130b3cabd200408509a28cdc) )

	NEO_BIOS_AUDIO_128K( "062-m1.bin", CRC(76108b2f) SHA1(08c89a8b746dbb10ff885b41cde344173c2e3699) )

	ROM_REGION( 0x100000, "ymsnd", 0 )
	ROM_LOAD( "062-v1.bin", 0x000000, 0x100000, CRC(cc281aef) SHA1(68be154b3e25f837afb4a477600dbe0ee69bec44) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "062-c1.bin", 0x000000, 0x100000, CRC(a9375aa2) SHA1(69218d8f1361e9ea709da11e3f15fe46b1db7181) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "062-c2.bin", 0x000001, 0x100000, CRC(0e73b758) SHA1(a247f736fbca0b609818dca4844ebb8442753bc1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "062-c3.bin", 0x200000, 0x100000, CRC(df51e465) SHA1(171953c7a870f3ab96e0f875117ee7343931fd38) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "062-c4.bin", 0x200001, 0x100000, CRC(38517e90) SHA1(f7c64b94ac20f5146f9bb48b53cb2b30fe5b8f8c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "062-c5.bin", 0x400000, 0x100000, CRC(7babd692) SHA1(0d4cd5006baa8d951cd2b6194ace566fa2845b8a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "062-c6.bin", 0x400001, 0x100000, CRC(cde5ade5) SHA1(5899ef5dfcdbb8cf8c6aba748dbb52f3c5fed5fe) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "062-c7.bin", 0x600000, 0x100000, CRC(bb2fd7c0) SHA1(cce11c4cf39ac60143235ff89261806df339dae5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "062-c8.bin", 0x600001, 0x100000, CRC(8d7be933) SHA1(e7097cfa26a959f90721e2e8368ceb47ea9db661) ) /* Plane 2,3 */
ROM_END

ROM_START( samsho2 ) /* MVS AND AES VERSION */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "063-p1.bin", 0x100000, 0x100000, CRC(22368892) SHA1(0997f8284aa0f57a333be8a0fdea777d0d01afd6) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "063-s1.bin", CRC(64a5cd66) SHA1(12cdfb27bf9ccd5a8df6ddd4628ef7cf2c6d4964) )

	NEO_BIOS_AUDIO_128K( "063-m1.bin", CRC(56675098) SHA1(90429fc40d056d480d0e2bbefbc691d9fa260fc4) )

	ROM_REGION( 0x700000, "ymsnd", 0 )
	ROM_LOAD( "063-v1.bin", 0x000000, 0x200000, CRC(37703f91) SHA1(a373ebef4c33ba1d8340e826981a58769aada238) )
	ROM_LOAD( "063-v2.bin", 0x200000, 0x200000, CRC(0142bde8) SHA1(0be6c53acac44802bf70b6925452f70289a139d9) )
	ROM_LOAD( "063-v3.bin", 0x400000, 0x200000, CRC(d07fa5ca) SHA1(1da7f081f8b8fc86a91feacf900f573218d82676) )
	ROM_LOAD( "063-v4.bin", 0x600000, 0x100000, CRC(24aab4bb) SHA1(10ee4c5b3579865b93dcc1e4079963276aa700a6) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "063-c1.bin", 0x000000, 0x200000, CRC(86cd307c) SHA1(0d04336f7c436d74638d8c1cd8651faf436a6bec) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "063-c2.bin", 0x000001, 0x200000, CRC(cdfcc4ca) SHA1(179dc81432424d68cefedd20cc1c4b2a95deb891) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "063-c3.bin", 0x400000, 0x200000, CRC(7a63ccc7) SHA1(49d97c543bc2860d493a353ab0d059088c6fbd21) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "063-c4.bin", 0x400001, 0x200000, CRC(751025ce) SHA1(e1bbaa7cd67fd04e4aab7f7ea77f63ae1cbc90d0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "063-c5.bin", 0x800000, 0x200000, CRC(20d3a475) SHA1(28da44a136bd14c73c62c147c3f6e6bcfa1066de) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "063-c6.bin", 0x800001, 0x200000, CRC(ae4c0a88) SHA1(cc8a7d11daa3821f83a6fd0942534706f939e576) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "063-c7.bin", 0xc00000, 0x200000, CRC(2df3cbcf) SHA1(e54f9022359963711451c2025825b862d36c6975) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "063-c8.bin", 0xc00001, 0x200000, CRC(1ffc6dfa) SHA1(acea18aca76c072e0bac2a364fc96d49cfc86e77) ) /* Plane 2,3 */
ROM_END

// This has corrupt text if used with the Japan bios due to the replacement of the s1 rom to contain the new logo
ROM_START( samsho2k )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "063-p1-kan.p1", 0x100000, 0x100000, CRC(147cc6d7) SHA1(8e22305f41a0688786ff55437c25948e6c8fda58) )
	ROM_CONTINUE( 0x000000, 0x100000 )
	// the roms below apply as patch over the main program (I haven't checked what they change, the game boots as the Korean version even with just the above program)
	ROM_LOAD16_WORD_SWAP( "063-ep1-kan.ep1", 0x000000, 0x080000, CRC(fa32e2d8) SHA1(94f56759ec04ab3a1e557bc2dc51b92176b3c147) )
	ROM_LOAD16_WORD_SWAP( "063-ep2-kan.ep2", 0x080000, 0x080000, CRC(70b1a4d9) SHA1(387737e87a68d0ea4fd13693f1f30d3227a17c82) ) // this is exactly the same data anyway!

	NEO_SFIX_128K( "063-s1-kan.s1", CRC(ff08f80b) SHA1(240c6a1c52edebb49cc99ea08484c6a2d61ebf84) )

	NEO_BIOS_AUDIO_128K( "063-m1.bin", CRC(56675098) SHA1(90429fc40d056d480d0e2bbefbc691d9fa260fc4) )

	ROM_REGION( 0x700000, "ymsnd", 0 )
	ROM_LOAD( "063-v1.bin", 0x000000, 0x200000, CRC(37703f91) SHA1(a373ebef4c33ba1d8340e826981a58769aada238) )
	ROM_LOAD( "063-v2.bin", 0x200000, 0x200000, CRC(0142bde8) SHA1(0be6c53acac44802bf70b6925452f70289a139d9) )
	ROM_LOAD( "063-v3.bin", 0x400000, 0x200000, CRC(d07fa5ca) SHA1(1da7f081f8b8fc86a91feacf900f573218d82676) )
	ROM_LOAD( "063-v4.bin", 0x600000, 0x100000, CRC(24aab4bb) SHA1(10ee4c5b3579865b93dcc1e4079963276aa700a6) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "063-c1.bin", 0x000000, 0x200000, CRC(86cd307c) SHA1(0d04336f7c436d74638d8c1cd8651faf436a6bec) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "063-c2.bin", 0x000001, 0x200000, CRC(cdfcc4ca) SHA1(179dc81432424d68cefedd20cc1c4b2a95deb891) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "063-c3.bin", 0x400000, 0x200000, CRC(7a63ccc7) SHA1(49d97c543bc2860d493a353ab0d059088c6fbd21) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "063-c4.bin", 0x400001, 0x200000, CRC(751025ce) SHA1(e1bbaa7cd67fd04e4aab7f7ea77f63ae1cbc90d0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "063-c5.bin", 0x800000, 0x200000, CRC(20d3a475) SHA1(28da44a136bd14c73c62c147c3f6e6bcfa1066de) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "063-c6.bin", 0x800001, 0x200000, CRC(ae4c0a88) SHA1(cc8a7d11daa3821f83a6fd0942534706f939e576) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "063-c7.bin", 0xc00000, 0x200000, CRC(2df3cbcf) SHA1(e54f9022359963711451c2025825b862d36c6975) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "063-c8.bin", 0xc00001, 0x200000, CRC(1ffc6dfa) SHA1(acea18aca76c072e0bac2a364fc96d49cfc86e77) ) /* Plane 2,3 */
ROM_END

ROM_START( wh2j ) /* MVS VERSION */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "064-p1.bin", 0x100000, 0x100000, CRC(385a2e86) SHA1(cfde4a1aeae038a3d6ca9946065624f097682d3d) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "064-s1.bin", CRC(2a03998a) SHA1(5e33f469982f12d4622a06d323a345f192bf88e6) )

	NEO_BIOS_AUDIO_128K( "064-m1.bin", CRC(d2eec9d3) SHA1(09478787045f1448d19d064dd3d540d1741fd619) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "064-v1.bin", 0x000000, 0x200000, CRC(aa277109) SHA1(35c22b15bb0a4d0ab118cb22a2d450d03995a17c) )
	ROM_LOAD( "064-v2.bin", 0x200000, 0x200000, CRC(b6527edd) SHA1(2bcf5bfa6e117cf4a3728a5e5f5771313c93f22a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "064-c13.bin", 0x000000, 0x400000, CRC(771a6365) SHA1(646bdf3a79819fbca98f513c172b9e9500c073be) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c24.bin", 0x000001, 0x400000, CRC(e7863a05) SHA1(39d34119137216f914b8b8b7b3e622a6c210f130) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "064-c57.bin", 0x800000, 0x400000, CRC(64594ed4) SHA1(982dbb932c0cb2eb9292bf19c463fd618c8180f3) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c68.bin", 0x800001, 0x400000, CRC(6e385398) SHA1(e74a0e72876bff8766152b76e5d94f2958b03cf2) ) /* Plane 2,3 */
ROM_END

ROM_START( wh2jh ) /* AES VERSION */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "064-p1.bin", 0x100000, 0x100000, CRC(385a2e86) SHA1(cfde4a1aeae038a3d6ca9946065624f097682d3d) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "064-s1.bin", CRC(2a03998a) SHA1(5e33f469982f12d4622a06d323a345f192bf88e6) )

	NEO_BIOS_AUDIO_128K( "064-m1.bin", CRC(d2eec9d3) SHA1(09478787045f1448d19d064dd3d540d1741fd619) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "064-v1.bin", 0x000000, 0x200000, CRC(aa277109) SHA1(35c22b15bb0a4d0ab118cb22a2d450d03995a17c) )
	ROM_LOAD( "064-v2.bin", 0x200000, 0x200000, CRC(b6527edd) SHA1(2bcf5bfa6e117cf4a3728a5e5f5771313c93f22a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "064-c1.bin", 0x000000, 0x200000, CRC(2ec87cea) SHA1(e713ec7839a7665edee6ee3f82a6e530b3b4bd7c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c2.bin", 0x000001, 0x200000, CRC(526b81ab) SHA1(b5f0a2f04489539ed6b9d0810b12787356c64b23) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "064-c3.bin", 0x400000, 0x200000, CRC(436d1b31) SHA1(059776d77b91377ed0bcfc278802d659c917fc0f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c4.bin", 0x400001, 0x200000, CRC(f9c8dd26) SHA1(25a9eea1d49b21b4a988beb32c25bf2f7796f227) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "064-c5.bin", 0x800000, 0x200000, CRC(8e34a9f4) SHA1(67b839b426ef3fad0a85d951fdd44c0a45c55226) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c6.bin", 0x800001, 0x200000, CRC(a43e4766) SHA1(54f282f2b1ff2934cca7acbb4386a2b99a29df3a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "064-c7.bin", 0xc00000, 0x200000, CRC(59d97215) SHA1(85a960dc7f364df13ee0c2f99a4c53aefb081486) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c8.bin", 0xc00001, 0x200000, CRC(fc092367) SHA1(69ff4ae909dd857de3ca8645d63f8b4bde117448) ) /* Plane 2,3 */
ROM_END

ROM_START( wjammers )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "065-p1.bin", 0x000000, 0x100000, CRC(6692c140) SHA1(5da574e906974fac92bb2f49bdeea257c014a897) )

	NEO_SFIX_128K( "065-s1.bin", CRC(074b5723) SHA1(86d3b3bb5414f43e4d3b7a2af0db23cc71ce8412) )

	NEO_BIOS_AUDIO_128K( "065-m1.bin", CRC(52c23cfc) SHA1(809a7e072ad9acbffc25e9bd27cdb97638d09d07) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "065-v1.bin", 0x000000, 0x100000, CRC(ce8b3698) SHA1(7d75e2a5cf8c90be422f8b425447e81597fe145a) )
	ROM_LOAD( "065-v2.bin", 0x100000, 0x100000, CRC(659f9b96) SHA1(62f40365212153bc3b92a1187fa44f6cdc7f7b83) )
	ROM_LOAD( "065-v3.bin", 0x200000, 0x100000, CRC(39f73061) SHA1(ec57cd58e7f8569cff925d11e2320d588ce4fe49) )
	ROM_LOAD( "065-v4.bin", 0x300000, 0x100000, CRC(5dee7963) SHA1(f8e6de73d65dd80b29c711f00835a574a770cb4e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "065-c1.bin", 0x000000, 0x100000, CRC(c7650204) SHA1(42918d700d59864f8ab15caf968a062a563c9b09) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "065-c2.bin", 0x000001, 0x100000, CRC(d9f3e71d) SHA1(fad1f64061eac1bf85bf6d75d2eae974a8c94069) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "065-c3.bin", 0x200000, 0x100000, CRC(40986386) SHA1(65795a50197049681265946713d416c9cdb68f08) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "065-c4.bin", 0x200001, 0x100000, CRC(715e15ff) SHA1(ac8b8b01f5c7384b883afbe0cf977430378e3fef) ) /* Plane 2,3 */
ROM_END

ROM_START( karnovr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "066-p1.bin", 0x000000, 0x100000, CRC(8c86fd22) SHA1(8cf97c6fb9c5717167ccc54bf5856248ccaf32c6) )

	NEO_SFIX_128K( "066-s1.bin", CRC(bae5d5e5) SHA1(aa69d9b235b781ec51f72a528fada9cb12e72cbc) )

	NEO_BIOS_AUDIO_128K( "066-m1.bin", CRC(030beae4) SHA1(ceb6ee6c09514504efacdbca7b280901e4c97084) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "066-v1.bin", 0x000000, 0x200000, CRC(0b7ea37a) SHA1(34e7d4f6db053674a7e8c8b2e3e398777d5b02e6) )

	NO_DELTAT_REGION

	ROM_REGION( 0xc00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "066-c1.bin", 0x000000, 0x200000, CRC(09dfe061) SHA1(ca4c0f0ce80967b4be2f18b72435c468bbfbac4c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "066-c2.bin", 0x000001, 0x200000, CRC(e0f6682a) SHA1(addb4fbc30da2b8ffc86819d92a874eb232f67dd) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "066-c3.bin", 0x400000, 0x200000, CRC(a673b4f7) SHA1(d138f5b38fd65c61549ce36f5c4983f7c8a3e7f6) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "066-c4.bin", 0x400001, 0x200000, CRC(cb3dc5f4) SHA1(865d9ccfc3df517c341d6aac16120f6b6aa759fe) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "066-c5.bin", 0x800000, 0x200000, CRC(9a28785d) SHA1(19723e1f7ff429e8a038d89488b279f830dfaf6e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "066-c6.bin", 0x800001, 0x200000, CRC(c15c01ed) SHA1(7cf5583e6610bcdc3b332896cefc71df84fb3f19) ) /* Plane 2,3 */
ROM_END

ROM_START( gururin )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "067-p1.bin", 0x000000, 0x80000, CRC(4cea8a49) SHA1(cea4a35db8de898e30eb40dd339b3cbe77ac0856) )

	NEO_SFIX_128K( "067-s1.bin", CRC(b119e1eb) SHA1(f63a68a71aea220d3d4475847652e2a1f68b2b6f) )

	NEO_BIOS_AUDIO_128K( "067-m1.bin", CRC(e777a234) SHA1(9f3974ac07859337bc0203f903c40ae3f60dc1fb) )

	ROM_REGION( 0x80000, "ymsnd", 0 )
	ROM_LOAD( "067-v1.bin", 0x000000, 0x80000, CRC(cf23afd0) SHA1(10f87014ee10613f92b04f482f449721a6379db7) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "067-c1.bin", 0x000000, 0x200000, CRC(35866126) SHA1(e4b24670ccc7901af5f66b11b15fae4e67f843ab) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "067-c2.bin", 0x000001, 0x200000, CRC(9db64084) SHA1(68a43c12f63f5e98d68ad0902a6551c5d30f8543) ) /* Plane 2,3 */
ROM_END

ROM_START( pspikes2 ) /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "068-pg1.bin", 0x000000, 0x100000, CRC(105a408f) SHA1(2ee51defa1c24c66c63a6498ee542ac26de3cfbb) )

	NEO_SFIX_128K( "068-sg1.bin", CRC(18082299) SHA1(efe93fabe6a76a5dc8cf12f255e571480afb40a0) )

	NEO_BIOS_AUDIO_128K( "068-mg1.bin", CRC(b1c7911e) SHA1(27b298e7d50981331e17aa642e2e363ffac4333a) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "068-v1.bin", 0x000000, 0x100000, CRC(2ced86df) SHA1(d6b73d1f31efbd74fb745200d4dade5f80b71541) )
	ROM_LOAD( "068-v2.bin", 0x100000, 0x100000, CRC(970851ab) SHA1(6c9b04e9cc6b92133f1154e5bdd9d38d8ef050a7) )
	ROM_LOAD( "068-v3.bin", 0x200000, 0x100000, CRC(81ff05aa) SHA1(d74302f38c59055bfc83b39dff798a585314fecd) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "068-c1.bin", 0x000000, 0x100000, CRC(7f250f76) SHA1(5109a41adcb7859e24dc43d88842d4cc18cd3305) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "068-c2.bin", 0x000001, 0x100000, CRC(20912873) SHA1(2df8766b531e47ffc30457e41c63b83557b4f468) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "068-c3.bin", 0x200000, 0x100000, CRC(4b641ba1) SHA1(7a9c42a30163eda455f7bde2302402b1a5de7178) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "068-c4.bin", 0x200001, 0x100000, CRC(35072596) SHA1(4150a21041f06514c97592bd8af686504b06e187) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "068-c5.bin", 0x400000, 0x100000, CRC(151dd624) SHA1(f2690a3fe9c64f70f283df785a5217d5b92a289f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "068-c6.bin", 0x400001, 0x100000, CRC(a6722604) SHA1(b40c57fb4be93ac0b918829f88393ced3d4f8bde) ) /* Plane 2,3 */
ROM_END

ROM_START( fatfury3 ) /* MVS AND AES VERSION */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "069-p1.bin", 0x000000, 0x100000, CRC(a8bcfbbc) SHA1(519c4861151797e5f4d4f33432b83dfabed8e7c4) )
	ROM_LOAD16_WORD_SWAP( "069-sp2.bin", 0x100000, 0x200000, CRC(dbe963ed) SHA1(8ece7f663cfe8e563576a397e41161d392cee67e) )

	NEO_SFIX_128K( "069-s1.bin", CRC(0b33a800) SHA1(b7d2cc97da4f30ddebc7b801f5e1d17d2306b2db) )

	NEO_BIOS_AUDIO_128K( "069-m1.bin", CRC(fce72926) SHA1(a40c74f793900b8542f0b8383ce4bf46fca112d4) )

	ROM_REGION( 0xa00000, "ymsnd", 0 )
	ROM_LOAD( "069-v1.bin", 0x000000, 0x400000, CRC(2bdbd4db) SHA1(5f4fecf69c2329d699cbd45829c19303b1e2a80e) )
	ROM_LOAD( "069-v2.bin", 0x400000, 0x400000, CRC(a698a487) SHA1(11b8bc53bc26a51f4a408e900e3769958625c4ed) )
	ROM_LOAD( "069-v3.bin", 0x800000, 0x200000, CRC(581c5304) SHA1(e9550ec547b4f605afed996b22d711f49b48fa92) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "069-c1.bin", 0x0000000, 0x400000, CRC(e302f93c) SHA1(d8610b14900b2b8fe691b67ca9b1abb335dbff74) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "069-c2.bin", 0x0000001, 0x400000, CRC(1053a455) SHA1(69501bfac68739e63d798045b812badd251d57b8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "069-c3.bin", 0x0800000, 0x400000, CRC(1c0fde2f) SHA1(cf6c2ef56c03a861de3b0b6dc0d7c9204d947f9d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "069-c4.bin", 0x0800001, 0x400000, CRC(a25fc3d0) SHA1(83cb349e2f1032652060b233e741fb893be5af16) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "069-c5.bin", 0x1000000, 0x200000, CRC(b3ec6fa6) SHA1(7e4c8ee9dd8d9a25ff183d9d8b05f38769348bc7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "069-c6.bin", 0x1000001, 0x200000, CRC(69210441) SHA1(6d496c549dba65caabeaffe5b762e86f9d648a26) ) /* Plane 2,3 */
ROM_END

ROM_START( zupapa ) /* Original Version - Encrypted GFX */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "070-p1.bin", 0x000000, 0x100000, CRC(5a96203e) SHA1(49cddec9ca6cc51e5ecf8a34e447a23e1f8a15a1) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do _not_ have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "070-m1.bin", CRC(5a3b3191) SHA1(fa9a9930e18c64e598841fb344c4471d3d2c1964) )

	ROM_REGION( 0x0200000, "ymsnd", 0 )
	ROM_LOAD( "070-v1.bin", 0x000000, 0x200000, CRC(d3a7e1ff) SHA1(4a4a227e10f4af58168f6c26011ea1d414253f92) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "070-c1.bin", 0x0000000, 0x800000, CRC(f8ad02d8) SHA1(9be54532332a8e963ec35ff1e518947bb11ebade) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "070-c2.bin", 0x0000001, 0x800000, CRC(70156dde) SHA1(06286bf043d50199b47df9a76ca91f39cb28cb90) ) /* Plane 2,3 */
ROM_END

ROM_START( panicbom )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "073-p1.bin", 0x000000, 0x080000, CRC(adc356ad) SHA1(801e0a54b65d7a3500e6cef2d6bba40c6356dc1f) )

	NEO_SFIX_128K( "073-s1.bin", CRC(b876de7e) SHA1(910347d7657470da914fb0a6b0ea02891e13c081) )

	NEO_BIOS_AUDIO_128K( "073-m1.bin", CRC(3cdf5d88) SHA1(6d8365a946fbd0b7c7b896536322638d80f6a764) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "073-v1.bin", 0x000000, 0x200000, CRC(7fc86d2f) SHA1(aa4234d22157060e0ba97a09c4e85c5276b74099) )
	ROM_LOAD( "073-v2.bin", 0x200000, 0x100000, CRC(082adfc7) SHA1(19c168e9a6cadcbed79033c320bcf3a45f846daf) )

	NO_DELTAT_REGION

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "073-c1.bin", 0x000000, 0x100000, CRC(8582e1b5) SHA1(e17d8f57b8ebee14b8e705374b34abe928937258) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "073-c2.bin", 0x000001, 0x100000, CRC(e15a093b) SHA1(548a418c81af79cd7ab6ad165b8d6daee30abb49) ) /* Plane 2,3 */
ROM_END

ROM_START( aodk ) /* MVS AND AES VERSION */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "074-p1.bin", 0x100000, 0x100000, CRC(62369553) SHA1(ca4d561ee08d16fe6804249d1ba49188eb3bd606) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "074-s1.bin", CRC(96148d2b) SHA1(47725a8059346ebe5639bbdbf62a2ac8028756a9) )

	NEO_BIOS_AUDIO_128K( "074-m1.bin", CRC(5a52a9d1) SHA1(ef913a9a55d29d5dd3beab1ce6039d64ce9b1a5b) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "074-v1.bin", 0x000000, 0x200000, CRC(7675b8fa) SHA1(29f4facf89d551237b31bf779693cbbbc94e1ede) )
	ROM_LOAD( "074-v2.bin", 0x200000, 0x200000, CRC(a9da86e9) SHA1(ff65af61e42b79a75060a352b24077d1fa28c83f) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "074-c1.bin", 0x000000, 0x200000, CRC(a0b39344) SHA1(adfff7b8836347abf030611563e6068a91164d0a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "074-c2.bin", 0x000001, 0x200000, CRC(203f6074) SHA1(737f2d707d504df1da1ca5c5cf61cf489a33eb56) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "074-c3.bin", 0x400000, 0x200000, CRC(7fff4d41) SHA1(bebd18a75adeb34c3bbd49cfc8fd3d8c2bf9e475) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "074-c4.bin", 0x400001, 0x200000, CRC(48db3e0a) SHA1(a88505e001e01bb45fb26beda5af24943d02552a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "074-c5.bin", 0x800000, 0x200000, CRC(c74c5e51) SHA1(0399c53e2a3d721901dddc073fda6ec22e02dfd4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "074-c6.bin", 0x800001, 0x200000, CRC(73e8e7e0) SHA1(dd6580227743e6a3db4950456ebe870008e022b2) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "074-c7.bin", 0xc00000, 0x200000, CRC(ac7daa01) SHA1(78407a464f67d949933ce2ccaa23fbed80dff1ea) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "074-c8.bin", 0xc00001, 0x200000, CRC(14e7ad71) SHA1(d4583fbce361fd1a11ac6c1a27b0b669e8a5c718) ) /* Plane 2,3 */
ROM_END

ROM_START( sonicwi2 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "075-p1.bin", 0x100000, 0x100000, CRC(92871738) SHA1(fed040a7c1ff9e495109813a702d09fb1d2ecf3a) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "075-s1.bin", CRC(c9eec367) SHA1(574e1afe7e0d54610c145131106e59ba2894eeb7) )

	NEO_BIOS_AUDIO_128K( "075-m1.bin", CRC(bb828df1) SHA1(eab8e2868173bdaac7c7ed97305a9aa1033fd303) )

	ROM_REGION( 0x300000, "ymsnd", 0 )
	ROM_LOAD( "075-v1.bin", 0x000000, 0x200000, CRC(7577e949) SHA1(3ba9f11094dd0cf519f33a16016cfae0d2c6629c) )
	ROM_LOAD( "075-v2.bin", 0x200000, 0x100000, CRC(021760cd) SHA1(8a24e38f1d4982c4dcd82718995571ac94cbb390) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "075-c1.bin", 0x000000, 0x200000, CRC(3278e73e) SHA1(d9e6c8a3a5213690a1b8747d27806d8ac5aac405) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "075-c2.bin", 0x000001, 0x200000, CRC(fe6355d6) SHA1(ca72fff7a908b6d9325761079ff2a0e28f34cf89) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "075-c3.bin", 0x400000, 0x200000, CRC(c1b438f1) SHA1(b3751c5b426bca0fcc3a58bdb86712c22ef908ab) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "075-c4.bin", 0x400001, 0x200000, CRC(1f777206) SHA1(e29c5ae65ebdcc1167a894306d2446ce909639da) ) /* Plane 2,3 */
ROM_END

ROM_START( zedblade )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "076-p1.bin", 0x000000, 0x080000, CRC(d7c1effd) SHA1(485c2308a40baecd122be9ab4996044622bdcc7e) )

	NEO_SFIX_128K( "076-s1.bin", CRC(f4c25dd5) SHA1(8ec9026219f393930634f9170edbaaee479f875e) )

	NEO_BIOS_AUDIO_128K( "076-m1.bin", CRC(7b5f3d0a) SHA1(4a301781a57ff236f49492b576ff4858b0ffbdf8) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "076-v1.bin", 0x000000, 0x200000, CRC(1a21d90c) SHA1(4793ab06421228ad122e359653ed0f1339b90c7a) )
	ROM_LOAD( "076-v2.bin", 0x200000, 0x200000, CRC(b61686c3) SHA1(5a3405e833ce36abb7421190438b5cccc8537919) )
	ROM_LOAD( "076-v3.bin", 0x400000, 0x100000, CRC(b90658fa) SHA1(b9a4b34565ce3688495c47e35c9b888ef686ae9f) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "076-c1.bin", 0x000000, 0x200000, CRC(4d9cb038) SHA1(c0b52b32e1fa719b99ae242d61d5dbea1437331c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "076-c2.bin", 0x000001, 0x200000, CRC(09233884) SHA1(1895cd0d126a022bce1cc4c7a569032d89f35e3f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "076-c3.bin", 0x400000, 0x200000, CRC(d06431e3) SHA1(643bd1ad74af272795b02143ba80a76e375036ab) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "076-c4.bin", 0x400001, 0x200000, CRC(4b1c089b) SHA1(cd63961d88c5be84673cce83c683a86b222a064d) ) /* Plane 2,3 */
ROM_END

ROM_START( galaxyfg )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "078-p1.bin", 0x100000, 0x100000, CRC(45906309) SHA1(cdcd96a564acf42e959193e139e149b29c103e25) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "078-s1.bin", CRC(72f8923e) SHA1(da908bffc2b5d8baa2002dbb5bfb3aa17d2472b7) )

	NEO_BIOS_AUDIO_128K( "078-m1.bin", CRC(8e9e3b10) SHA1(7c44d1dbd4f8d337c99e90361d1dab837df85e31) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "078-v1.bin", 0x000000, 0x200000, CRC(e3b735ac) SHA1(e16dfac09aef8115a20bae0bef8c86d4e7d0dc4a) )
	ROM_LOAD( "078-v2.bin", 0x200000, 0x200000, CRC(6a8e78c2) SHA1(f60b1f8a3a945f279a582745e82f37278ce5d83b) )
	ROM_LOAD( "078-v3.bin", 0x400000, 0x100000, CRC(70bca656) SHA1(218b7079c90898e7faa382b386e77f81f415e7ac) )

	NO_DELTAT_REGION

	ROM_REGION( 0xe00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "078-c1.bin", 0x000000, 0x200000, CRC(c890c7c0) SHA1(b96c18a41c34070a4f24ca77cb7516fae8b0fd0c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "078-c2.bin", 0x000001, 0x200000, CRC(b6d25419) SHA1(e089df9c9a9645f706e501108d634f4d222622a2) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "078-c3.bin", 0x400000, 0x200000, CRC(9d87e761) SHA1(ea1b6d7c9d5ef3a9b48968bde5a52d5699d591cc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "078-c4.bin", 0x400001, 0x200000, CRC(765d7cb8) SHA1(7b9c86714d688602064d928c9d2b49d70bb7541e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "078-c5.bin", 0x800000, 0x200000, CRC(e6b77e6a) SHA1(db3b8fc62a6f21c6653621c0665450d5d9a9913d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "078-c6.bin", 0x800001, 0x200000, CRC(d779a181) SHA1(2761026abd9698a7b56114b76631563abd41fd12) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "078-c7.bin", 0xc00000, 0x100000, CRC(4f27d580) SHA1(c0f12496b45b2fe6e94aa8ac52b0157063127e0a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "078-c8.bin", 0xc00001, 0x100000, CRC(0a7cc0d8) SHA1(68aaee6341c87e56ce11acc1c4ec8047839fe70d) ) /* Plane 2,3 */
ROM_END

ROM_START( strhoop )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "079-p1.bin", 0x000000, 0x100000, CRC(5e78328e) SHA1(7a00b096ed6dd77afc3008c5a4c83686e475f323) )

	NEO_SFIX_128K( "079-s1.bin", CRC(3ac06665) SHA1(ba9ab51eb95c3568304377ef6d7b5f32e8fbcde1) )

	NEO_BIOS_AUDIO_128K( "079-m1.bin", CRC(bee3455a) SHA1(fd5345d9847982085a9b364fff542580889bf02f) )

	ROM_REGION( 0x280000, "ymsnd", 0 )
	ROM_LOAD( "079-v1.bin", 0x000000, 0x200000, CRC(718a2400) SHA1(cefc5d0b302bd4a87ab1fa244ade4482c23c6806) )
	ROM_LOAD( "079-v2.bin", 0x200000, 0x080000, BAD_DUMP CRC(b19884f8) SHA1(5fe910f2029da19ddab4dc95c2292d7fbb086741) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "079-c1.bin", 0x000000, 0x200000, CRC(0581c72a) SHA1(453f7a8474195a1120da5fa24337d79674563d9e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "079-c2.bin", 0x000001, 0x200000, CRC(5b9b8fb6) SHA1(362aa0de0d2cf9aa03758363ffb1e15e046a3930) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "079-c3.bin", 0x400000, 0x200000, CRC(cd65bb62) SHA1(6f47d77d61d4289bcee82df7c4efa5346a6e4c80) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "079-c4.bin", 0x400001, 0x200000, CRC(a4c90213) SHA1(1b9f7b5f31acd6df2bdab81b849f32c13aa1b884) ) /* Plane 2,3 */
ROM_END

ROM_START( quizkof )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "080-p1.bin", 0x000000, 0x100000, CRC(4440315e) SHA1(f4adba8e341d64a1f6280dfd98ebf6918c00608d) )

	NEO_SFIX_128K( "080-s1.bin", CRC(d7b86102) SHA1(09e1ca6451f3035ce476e3b045541646f860aad5) )

	NEO_BIOS_AUDIO_128K( "080-m1.bin", CRC(f5f44172) SHA1(eaaba1781622901b91bce9257be4e05f84df053b) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "080-v1.bin", 0x000000, 0x200000, CRC(0be18f60) SHA1(05c8b7d9f5a8583015f31902ad16d9c621f47d4e) )
	ROM_LOAD( "080-v2.bin", 0x200000, 0x200000, CRC(4abde3ff) SHA1(0188bfcafa9a1aac302705736a2bcb26b9d684c2) )
	ROM_LOAD( "080-v3.bin", 0x400000, 0x200000, CRC(f02844e2) SHA1(8c65ebe146f4ddb6c904f8125cb32767f74c24d5) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "080-c1.bin", 0x000000, 0x200000, CRC(ea1d764a) SHA1(78cc1735624c37f90607baa92e110a3c5cc54c6f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "080-c2.bin", 0x000001, 0x200000, CRC(d331d4a4) SHA1(94228d13fb1e30973eb54058e697f17456ee16ea) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "080-c3.bin", 0x400000, 0x200000, CRC(b4851bfe) SHA1(b8286c601de5755c1681ea46e177fc89006fc066) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "080-c4.bin", 0x400001, 0x200000, CRC(ca6f5460) SHA1(ed36e244c9335f4c0a97c57b7b7f1b849dd3a90d) ) /* Plane 2,3 */
ROM_END

ROM_START( ssideki3 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "081-p1.bin", 0x100000, 0x100000, CRC(6bc27a3d) SHA1(94692abe7343f9204a557acae4ab74d0af511ca3) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "081-s1.bin", CRC(7626da34) SHA1(30bad65633d0035fd578323c22cbddb8c9d549a6) )

	NEO_BIOS_AUDIO_128K( "081-m1.bin", CRC(82fcd863) SHA1(b219a5685450f9c24cc195f1c914bc3b292d72c0) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "081-v1.bin", 0x000000, 0x200000, CRC(201fa1e1) SHA1(9c27cc1b1d075223ed4a90dd02571d09a2f0d076) )
	ROM_LOAD( "081-v2.bin", 0x200000, 0x200000, CRC(acf29d96) SHA1(5426985c33aea2efc8ff774b59d34d8b03bd9a85) )
	ROM_LOAD( "081-v3.bin", 0x400000, 0x200000, CRC(e524e415) SHA1(8733e1b63471381b16c2b7c64b909745d99c8925) )

	NO_DELTAT_REGION

	ROM_REGION( 0xc00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "081-c1.bin", 0x000000, 0x200000, CRC(1fb68ebe) SHA1(abd9dbe7b7cbe0b6cd1d87e53c6bdc6edeccf83c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "081-c2.bin", 0x000001, 0x200000, CRC(b28d928f) SHA1(9f05148e3e1e94339752658c066f47f133db8fbf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "081-c3.bin", 0x400000, 0x200000, CRC(3b2572e8) SHA1(41aba1554bf59d4e5d5814249eaa0d531449e1de) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "081-c4.bin", 0x400001, 0x200000, CRC(47d26a7c) SHA1(591ef24a3d381163c5da80fa64e6883b8ea9abfb) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "081-c5.bin", 0x800000, 0x200000, CRC(17d42f0d) SHA1(7de7765bf43d390c50b2f59c2288502a7121d086) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "081-c6.bin", 0x800001, 0x200000, CRC(6b53fb75) SHA1(fadf7a12661d83ae35d9258aa4947969d51c08b8) ) /* Plane 2,3 */
ROM_END

ROM_START( doubledr )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "082-p1.bin", 0x100000, 0x100000, CRC(34ab832a) SHA1(fbb1bd195f5653f7b9c89648649f838eaf83cbe4) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "082-s1.bin", CRC(bef995c5) SHA1(9c89adbdaa5c1f827632c701688563dac2e482a4) )

	NEO_BIOS_AUDIO_128K( "082-m1.bin", CRC(10b144de) SHA1(cf1ed0a447da68240c62bcfd76b1569803f6bf76) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "082-v1.bin", 0x000000, 0x200000, CRC(cc1128e4) SHA1(bfcfff24bc7fbde0b02b1bc0dffebd5270a0eb04) )
	ROM_LOAD( "082-v2.bin", 0x200000, 0x200000, CRC(c3ff5554) SHA1(c685887ad64998e5572607a916b023f8b9efac49) )

	NO_DELTAT_REGION

	ROM_REGION( 0xe00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "082-c1.bin", 0x000000, 0x200000, CRC(b478c725) SHA1(3a777c5906220f246a6dc06cb084e6ad650d67bb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "082-c2.bin", 0x000001, 0x200000, CRC(2857da32) SHA1(9f13245965d23db86d46d7e73dfb6cc63e6f25a1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "082-c3.bin", 0x400000, 0x200000, CRC(8b0d378e) SHA1(3a347215e414b738164f1fe4144102f07d4ffb80) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "082-c4.bin", 0x400001, 0x200000, CRC(c7d2f596) SHA1(e2d09d4d1b1fef9c0c53ecf3629e974b75e559f5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "082-c5.bin", 0x800000, 0x200000, CRC(ec87bff6) SHA1(3fa86da93881158c2c23443855922a7b32e55135) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "082-c6.bin", 0x800001, 0x200000, CRC(844a8a11) SHA1(b2acbd4cacce66fb32c052b2fba9984904679bda) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "082-c7.bin", 0xc00000, 0x100000, CRC(727c4d02) SHA1(8204c7f037d46e0c58f269f9c7a535bc2589f526) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "082-c8.bin", 0xc00001, 0x100000, CRC(69a5fa37) SHA1(020e70e0e8b3c5d00a40fe97e418115a3187e50a) ) /* Plane 2,3 */
ROM_END

ROM_START( pbobblen ) /* MVS VERSION */
	/* This set uses CHA and PROG board from Power Spikes II. Six Power Spikes II prom's are replaced with
    Puzzle Bobble prom's. Confirmed on several original carts. Do other layouts also exist? */

	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "d96-07.ep1", 0x000000, 0x080000, CRC(6102ca14) SHA1(328429d11de5b327a0654ae0548da4d0025a2ae6) )

	NEO_SFIX_128K( "d96-04.s1", CRC(9caae538) SHA1 (cf2d90a7c1a42107c0bb8b9a61397634286dbe0a) )

	NEO_BIOS_AUDIO_128K( "d96-06.m1", CRC(f424368a) SHA1 (5e5bbcaeb82bed2ee17df08f005ca20ad1030723) )

	ROM_REGION( 0x380000, "ymsnd", 0 )
	ROM_LOAD( "068-v1.bin", 0x000000, 0x100000, CRC(2ced86df) SHA1(d6b73d1f31efbd74fb745200d4dade5f80b71541) ) /* unused */
	ROM_LOAD( "068-v2.bin", 0x100000, 0x100000, CRC(970851ab) SHA1(6c9b04e9cc6b92133f1154e5bdd9d38d8ef050a7) ) /* unused */
	ROM_LOAD( "d96-01.v3", 0x200000, 0x100000, CRC(0840cbc4) SHA1 (1adbd7aef44fa80832f63dfb8efdf69fd7256a57) )
	ROM_LOAD( "d96-05.v4", 0x300000, 0x080000, CRC(0a548948) SHA1 (e1e4afd17811cb60401c14fbcf0465035165f4fb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x500000, "sprites", 0 )
	ROM_LOAD16_BYTE( "068-c1.bin", 0x000000, 0x100000, CRC(7f250f76) SHA1(5109a41adcb7859e24dc43d88842d4cc18cd3305) ) /* unused */
	ROM_LOAD16_BYTE( "068-c2.bin", 0x000001, 0x100000, CRC(20912873) SHA1(2df8766b531e47ffc30457e41c63b83557b4f468) ) /* unused */
	ROM_LOAD16_BYTE( "068-c3.bin", 0x200000, 0x100000, CRC(4b641ba1) SHA1(7a9c42a30163eda455f7bde2302402b1a5de7178) ) /* unused */
	ROM_LOAD16_BYTE( "068-c4.bin", 0x200001, 0x100000, CRC(35072596) SHA1(4150a21041f06514c97592bd8af686504b06e187) ) /* unused */
	ROM_LOAD16_BYTE( "d96-02.c5", 0x400000, 0x80000, CRC(e89ad494) SHA1 (69c9ea415773af94ac44c48af05d55ada222b138) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "d96-03.c6", 0x400001, 0x80000, CRC(4b42d7eb) SHA1 (042ae50a528cea21cf07771d3915c57aa16fd5af) ) /* Plane 2,3 */
ROM_END

ROM_START( kof95 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "084-p1.bin",0x100000, 0x100000, CRC(2cba2716) SHA1(f6c2d0537c9c3e0938065c65b1797c47198fcff8) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "084-s1.bin", CRC(de716f8a) SHA1(f7386454a943ed5caf625f67ee1d0197b1c6fa13) )

	NEO_BIOS_AUDIO_128K( "084-m1.bin", CRC(6f2d7429) SHA1(6f8462e4f07af82a5ca3197895d5dcbb67bdaa61) )

	ROM_REGION( 0x900000, "ymsnd", 0 )
	ROM_LOAD( "084-v1.bin", 0x000000, 0x400000, CRC(84861b56) SHA1(1b6c91ddaed01f45eb9b7e49d9c2b9b479d50da6) )
	ROM_LOAD( "084-v2.bin", 0x400000, 0x200000, CRC(b38a2803) SHA1(dbc2c8606ca09ed7ff20906b022da3cf053b2f09) )
	/* 600000-7fffff empty */
	ROM_LOAD( "084-v3.bin", 0x800000, 0x100000, CRC(d683a338) SHA1(eb9866b4b286edc09963cb96c43ce0a8fb09adbb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1a00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "084-c1.bin", 0x0000000, 0x400000, CRC(fe087e32) SHA1(e8e89faa616027e4fb9b8a865c1a67f409c93bdf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "084-c2.bin", 0x0000001, 0x400000, CRC(07864e09) SHA1(0817fcfd75d0735fd8ff27561eaec371e4ff5829) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "084-c3.bin", 0x0800000, 0x400000, CRC(a4e65d1b) SHA1(740a405b40b3a4b324697d2652cae29ffe0ac0bd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "084-c4.bin", 0x0800001, 0x400000, CRC(c1ace468) SHA1(74ea2a3cfd7b744f0988a05baaff10016ca8f625) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "084-c5.bin", 0x1000000, 0x200000, CRC(8a2c1edc) SHA1(67866651bc0ce27122285a66b0aab108acf3d065) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "084-c6.bin", 0x1000001, 0x200000, CRC(f593ac35) SHA1(302c92c63f092a8d49429c3331e5e5678f0ea48d) ) /* Plane 2,3 */
	/* 1400000-17fffff empty */
	ROM_LOAD16_BYTE( "084-c7.bin", 0x1800000, 0x100000, CRC(9904025f) SHA1(eec770746a0ad073f7d353ab16a2cc3a5278d307) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "084-c8.bin", 0x1800001, 0x100000, CRC(78eb0f9b) SHA1(2925ea21ed2ce167f08a25589e94f28643379034) ) /* Plane 2,3 */
ROM_END

ROM_START( kof95h ) /* MVS AND AES VERSION */
	/* There also exists a MVS version with 4x eprom (EP1~EP4); board used is NEO-MVS PROGSM */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "084-pg1.bin", 0x100000, 0x100000, CRC(5e54cf95) SHA1(41abe2042fdbb1526e92a0789976a9b1ac5e60f0) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "084-s1.bin", CRC(de716f8a) SHA1(f7386454a943ed5caf625f67ee1d0197b1c6fa13) )

	NEO_BIOS_AUDIO_128K( "084-m1.bin", CRC(6f2d7429) SHA1(6f8462e4f07af82a5ca3197895d5dcbb67bdaa61) )

	ROM_REGION( 0x900000, "ymsnd", 0 )
	ROM_LOAD( "084-v1.bin", 0x000000, 0x400000, CRC(84861b56) SHA1(1b6c91ddaed01f45eb9b7e49d9c2b9b479d50da6) )
	ROM_LOAD( "084-v2.bin", 0x400000, 0x200000, CRC(b38a2803) SHA1(dbc2c8606ca09ed7ff20906b022da3cf053b2f09) )
	/* 600000-7fffff empty */
	ROM_LOAD( "084-v3.bin", 0x800000, 0x100000, CRC(d683a338) SHA1(eb9866b4b286edc09963cb96c43ce0a8fb09adbb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1a00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "084-c1.bin", 0x0000000, 0x400000, CRC(fe087e32) SHA1(e8e89faa616027e4fb9b8a865c1a67f409c93bdf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "084-c2.bin", 0x0000001, 0x400000, CRC(07864e09) SHA1(0817fcfd75d0735fd8ff27561eaec371e4ff5829) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "084-c3.bin", 0x0800000, 0x400000, CRC(a4e65d1b) SHA1(740a405b40b3a4b324697d2652cae29ffe0ac0bd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "084-c4.bin", 0x0800001, 0x400000, CRC(c1ace468) SHA1(74ea2a3cfd7b744f0988a05baaff10016ca8f625) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "084-c5.bin", 0x1000000, 0x200000, CRC(8a2c1edc) SHA1(67866651bc0ce27122285a66b0aab108acf3d065) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "084-c6.bin", 0x1000001, 0x200000, CRC(f593ac35) SHA1(302c92c63f092a8d49429c3331e5e5678f0ea48d) ) /* Plane 2,3 */
	/* 1400000-17fffff empty */
	ROM_LOAD16_BYTE( "084-c7.bin", 0x1800000, 0x100000, CRC(9904025f) SHA1(eec770746a0ad073f7d353ab16a2cc3a5278d307) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "084-c8.bin", 0x1800001, 0x100000, CRC(78eb0f9b) SHA1(2925ea21ed2ce167f08a25589e94f28643379034) ) /* Plane 2,3 */
ROM_END

ROM_START( tws96 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "086-p1.bin", 0x000000, 0x100000, CRC(03e20ab6) SHA1(3a0a5a54649178ce7a6158980cb4445084b40fb5) )

	NEO_SFIX_128K( "086-s1.bin", CRC(6f5e2b3a) SHA1(273341489f6625d35a4a920042a60e2b86373847) )

	NEO_BIOS_AUDIO_128K( "086-m1.bin", CRC(cb82bc5d) SHA1(8e3ecabec25d89adb6e0eed0ef5f94d34a4d5fc0) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "086-v1.bin", 0x000000, 0x200000, CRC(97bf1986) SHA1(b80d3a37e18d0a52f1e0092dc300989c9647efd1) )
	ROM_LOAD( "086-v2.bin", 0x200000, 0x200000, CRC(b7eb05df) SHA1(ff2b55c7021c248cfdcfc9cd3658f2896bcbca38) )

	NO_DELTAT_REGION

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "086-c1.bin", 0x000000, 0x400000, CRC(2611bc2a) SHA1(bb5a96acd4a90fcb41c49cc8e9f760c4a06d6b84) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "086-c2.bin", 0x000001, 0x400000, CRC(6b0d6827) SHA1(3cb2bbab381a26ec69f97c3d6116ce47254286b4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "086-c3.bin", 0x800000, 0x100000, CRC(750ddc0c) SHA1(9304a83d81afd544d88be0cd3ee47ae401d2da0e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "086-c4.bin", 0x800001, 0x100000, CRC(7a6e7d82) SHA1(b1bb82cec3d68367d5e01e63c44c11b67e577411) ) /* Plane 2,3 */
ROM_END

ROM_START( samsho3 ) /* MVS VERSION */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "087-ep1.bin", 0x000000, 0x080000, CRC(23e09bb8) SHA1(79da99fa50a639fce9d1266699b5e53c9ac55642) )
	ROM_LOAD16_WORD_SWAP( "087-ep2.bin", 0x080000, 0x080000, CRC(256f5302) SHA1(e2d21b413a6059194a994b7902b2a7df98a15151) )
	ROM_LOAD16_WORD_SWAP( "087-ep3.bin", 0x100000, 0x080000, CRC(bf2db5dd) SHA1(b4fa1dc1eccc9eb1ce74f0a06992ef89b1cbc732) )
	ROM_LOAD16_WORD_SWAP( "087-ep4.bin", 0x180000, 0x080000, CRC(53e60c58) SHA1(f975e81cab6322d3260348402721c673023259fa) )
	ROM_LOAD16_WORD_SWAP( "087-p5.bin",  0x200000, 0x100000, CRC(e86ca4af) SHA1(5246acbab77ac2f232b88b8522187764ff0872f0) )

	NEO_SFIX_128K( "087-s1.bin", CRC(74ec7d9f) SHA1(d79c479838a7ca51735a44f91f1968ec5b3c6b91) )

	NEO_BIOS_AUDIO_128K( "087-m1.bin", CRC(8e6440eb) SHA1(e3f72150af4e326543b29df71cda27d73ec087c1) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "087-v1.bin", 0x000000, 0x400000, CRC(84bdd9a0) SHA1(adceceb00569eca13fcc2e0f0d9f0d9b06a06851) )
	ROM_LOAD( "087-v2.bin", 0x400000, 0x200000, CRC(ac0f261a) SHA1(5411bdff24cba7fdbc3397d45a70fb468d7a44b3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1a00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "087-c1.bin", 0x0000000, 0x400000, CRC(07a233bc) SHA1(654cb56cfd6eeebe6745c0b8b730317fb8ccd3d9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c2.bin", 0x0000001, 0x400000, CRC(7a413592) SHA1(b8c7a2d0d7a8b14d6cab94d7a5f347e73c6ab7a4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c3.bin", 0x0800000, 0x400000, CRC(8b793796) SHA1(053acc129ea56691607a5d255845703e61fd3ada) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c4.bin", 0x0800001, 0x400000, CRC(728fbf11) SHA1(daa319d455f759bfc08a37b43218bdb48dc1c9e5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c5.bin", 0x1000000, 0x400000, CRC(172ab180) SHA1(a6122f683bdb78d0079e1e360c1b96ba28def7b7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c6.bin", 0x1000001, 0x400000, CRC(002ff8f3) SHA1(3a378708697d727796c4f702dd5bbf1c9eb4daec) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c7.bin", 0x1800000, 0x100000, CRC(ae450e3d) SHA1(ec482632cc347ec3f9e68df0ebcaa16ebe41b9f9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c8.bin", 0x1800001, 0x100000, CRC(a9e82717) SHA1(e39ee15d5140dbe7f06eea945cce9984a5e8b06a) ) /* Plane 2,3 */
ROM_END

ROM_START( samsho3h ) /* AES VERSION */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "087-pg1.bin", 0x000000, 0x100000, CRC(282a336e) SHA1(e062f1939d36a45f185b5dbd726cdd833dc7c28c) )
	ROM_LOAD16_WORD_SWAP( "087-p2.bin",  0x100000, 0x200000, CRC(9bbe27e0) SHA1(b18117102159903c8e8f4e4226e1cc91a400e816) )

	NEO_SFIX_128K( "087-s1.bin", CRC(74ec7d9f) SHA1(d79c479838a7ca51735a44f91f1968ec5b3c6b91) )

	NEO_BIOS_AUDIO_128K( "087-m1.bin", CRC(8e6440eb) SHA1(e3f72150af4e326543b29df71cda27d73ec087c1) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "087-v1.bin", 0x000000, 0x400000, CRC(84bdd9a0) SHA1(adceceb00569eca13fcc2e0f0d9f0d9b06a06851) )
	ROM_LOAD( "087-v2.bin", 0x400000, 0x200000, CRC(ac0f261a) SHA1(5411bdff24cba7fdbc3397d45a70fb468d7a44b3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1a00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "087-c1.bin", 0x0000000, 0x400000, CRC(07a233bc) SHA1(654cb56cfd6eeebe6745c0b8b730317fb8ccd3d9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c2.bin", 0x0000001, 0x400000, CRC(7a413592) SHA1(b8c7a2d0d7a8b14d6cab94d7a5f347e73c6ab7a4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c3.bin", 0x0800000, 0x400000, CRC(8b793796) SHA1(053acc129ea56691607a5d255845703e61fd3ada) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c4.bin", 0x0800001, 0x400000, CRC(728fbf11) SHA1(daa319d455f759bfc08a37b43218bdb48dc1c9e5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c5.bin", 0x1000000, 0x400000, CRC(172ab180) SHA1(a6122f683bdb78d0079e1e360c1b96ba28def7b7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c6.bin", 0x1000001, 0x400000, CRC(002ff8f3) SHA1(3a378708697d727796c4f702dd5bbf1c9eb4daec) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c7.bin", 0x1800000, 0x100000, CRC(ae450e3d) SHA1(ec482632cc347ec3f9e68df0ebcaa16ebe41b9f9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c8.bin", 0x1800001, 0x100000, CRC(a9e82717) SHA1(e39ee15d5140dbe7f06eea945cce9984a5e8b06a) ) /* Plane 2,3 */
ROM_END

ROM_START( fswords ) /* KOREAN VERSION */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "187-p1k.bin", 0x000000, 0x100000, CRC(c8e7c075) SHA1(7b74f2917114460d79d8f46ee24829a4c08cbf2a) )
	ROM_LOAD16_WORD_SWAP( "087-p2.bin",  0x100000, 0x200000, CRC(9bbe27e0) SHA1(b18117102159903c8e8f4e4226e1cc91a400e816) )

	NEO_SFIX_128K( "087-s1.bin", CRC(74ec7d9f) SHA1(d79c479838a7ca51735a44f91f1968ec5b3c6b91) )

	NEO_BIOS_AUDIO_128K( "087-m1.bin", CRC(8e6440eb) SHA1(e3f72150af4e326543b29df71cda27d73ec087c1) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "087-v1.bin", 0x000000, 0x400000, CRC(84bdd9a0) SHA1(adceceb00569eca13fcc2e0f0d9f0d9b06a06851) )
	ROM_LOAD( "087-v2.bin", 0x400000, 0x200000, CRC(ac0f261a) SHA1(5411bdff24cba7fdbc3397d45a70fb468d7a44b3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1a00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "087-c1.bin", 0x0000000, 0x400000, CRC(07a233bc) SHA1(654cb56cfd6eeebe6745c0b8b730317fb8ccd3d9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c2.bin", 0x0000001, 0x400000, CRC(7a413592) SHA1(b8c7a2d0d7a8b14d6cab94d7a5f347e73c6ab7a4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c3.bin", 0x0800000, 0x400000, CRC(8b793796) SHA1(053acc129ea56691607a5d255845703e61fd3ada) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c4.bin", 0x0800001, 0x400000, CRC(728fbf11) SHA1(daa319d455f759bfc08a37b43218bdb48dc1c9e5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c5.bin", 0x1000000, 0x400000, CRC(172ab180) SHA1(a6122f683bdb78d0079e1e360c1b96ba28def7b7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c6.bin", 0x1000001, 0x400000, CRC(002ff8f3) SHA1(3a378708697d727796c4f702dd5bbf1c9eb4daec) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "087-c7.bin", 0x1800000, 0x100000, CRC(ae450e3d) SHA1(ec482632cc347ec3f9e68df0ebcaa16ebe41b9f9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "087-c8.bin", 0x1800001, 0x100000, CRC(a9e82717) SHA1(e39ee15d5140dbe7f06eea945cce9984a5e8b06a) ) /* Plane 2,3 */
ROM_END

ROM_START( stakwin )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "088-p1.bin", 0x100000, 0x100000, CRC(bd5814f6) SHA1(95179a4dee61ae88bb5d9fd74af0c56c8c29f5ea) )
	ROM_CONTINUE( 0x000000, 0x100000)

	NEO_SFIX_128K( "088-s1.bin", CRC(073cb208) SHA1(c5b4697d767575884dd49ae416c1fe4a4a92d3f6) )

	NEO_BIOS_AUDIO_128K( "088-m1.bin", CRC(2fe1f499) SHA1(5b747eeef65be04423d2db05e086df9132758a47) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "088-v1.bin", 0x000000, 0x200000, CRC(b7785023) SHA1(d11df1e623434669cd3f97f0feda747b24dac05d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "088-c1.bin", 0x000000, 0x200000, CRC(6e733421) SHA1(b67c5d2654a62cc4e44bd54d28e62c7da5eea424) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "088-c2.bin", 0x000001, 0x200000, CRC(4d865347) SHA1(ad448cf96f3dce44c83412ed6878c495eb4a8a1e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "088-c3.bin", 0x400000, 0x200000, CRC(8fa5a9eb) SHA1(7bee19d8a2bccedd8e2cf0c0e9138902b9dafc23) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "088-c4.bin", 0x400001, 0x200000, CRC(4604f0dc) SHA1(ddf5dbb5e07313998a8f695ad19354ea54585dd6) ) /* Plane 2,3 */
ROM_END

ROM_START( pulstar )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "089-p1.bin", 0x000000, 0x100000, CRC(5e5847a2) SHA1(b864d0ec4184b785569ddbf67c2115b5ab86ee3e) )
	ROM_LOAD16_WORD_SWAP( "089-p2.bin", 0x100000, 0x200000, CRC(028b774c) SHA1(fc5da2821a5072f2b78245fc59b6e3eeef116d16) )

	NEO_SFIX_128K( "089-s1.bin", CRC(c79fc2c8) SHA1(914c224fb3c461a68d7425cae724cf22bd5f985d) )

	NEO_BIOS_AUDIO_128K( "089-m1.bin", CRC(ff3df7c7) SHA1(59d2ef64f734f6026073b365300221909057a512) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "089-v1.bin", 0x000000, 0x400000, CRC(b458ded2) SHA1(75d659ecb1fd6002188f469fcaafc9907440e624) )
	ROM_LOAD( "089-v2.bin", 0x400000, 0x400000, CRC(9d2db551) SHA1(83f7e5db7fb1502ceadcd334df90b11b1bba78e5) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1c00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "089-c1.bin", 0x0000000, 0x400000, CRC(f4e97332) SHA1(54693827a99836e7d61c45d495dd78bf3fcf1544) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "089-c2.bin", 0x0000001, 0x400000, CRC(836d14da) SHA1(99cc4f9b764503eff7849ff2977d90bb47c5564a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "089-c3.bin", 0x0800000, 0x400000, CRC(913611c4) SHA1(9664eb1fe1e6f8c3ddeeff872d38ea920ed38a82) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "089-c4.bin", 0x0800001, 0x400000, CRC(44cef0e3) SHA1(34f6f348ba86a2a06cb9c43a16b97cf6ee6158ac) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "089-c5.bin", 0x1000000, 0x400000, CRC(89baa1d7) SHA1(976c745c44967de61e2a23227835be580b1d283a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "089-c6.bin", 0x1000001, 0x400000, CRC(b2594d56) SHA1(685c0bf8ff76c76e41c2ceaebb96349634cfdb2e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "089-c7.bin", 0x1800000, 0x200000, CRC(6a5618ca) SHA1(9a1d5f998b0dfabacf9dad45c94bef2bb43e5e0c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "089-c8.bin", 0x1800001, 0x200000, CRC(a223572d) SHA1(2791b1212f57937b2b2a95bc9e420c06d0c37669) ) /* Plane 2,3 */
ROM_END

ROM_START( whp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "090-p1.bin", 0x100000, 0x100000, CRC(afaa4702) SHA1(83d122fddf17d4774353abf4a0655f3939f7b752) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "090-s1.bin", CRC(174a880f) SHA1(c35d315d728d119a6e9aa42e0593937c90897449) )

	NEO_BIOS_AUDIO_128K( "090-m1.bin", CRC(28065668) SHA1(0c60d4afa1dccad0135e733104f056be73b54e4e) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "090-v1.bin", 0x000000, 0x200000, CRC(30cf2709) SHA1(d1845033f16de2470afd3858ee0efb45176d9ed7) )
	ROM_LOAD( "064-v2.bin", 0x200000, 0x200000, CRC(b6527edd) SHA1(2bcf5bfa6e117cf4a3728a5e5f5771313c93f22a) )
	ROM_LOAD( "090-v3.bin", 0x400000, 0x200000, CRC(1908a7ce) SHA1(78f31bcfea33eb94752bbf5226c481baec1af5ac) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1c00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "090-c1.bin", 0x0000000, 0x400000, CRC(cd30ed9b) SHA1(839c20f7ff31251acc875ae402b5d267e55510c7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "090-c2.bin", 0x0000001, 0x400000, CRC(10eed5ee) SHA1(12131b1c8c017ea77a98c044b392a5db6aad0143) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "064-c3.bin", 0x0800000, 0x200000, CRC(436d1b31) SHA1(059776d77b91377ed0bcfc278802d659c917fc0f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c4.bin", 0x0800001, 0x200000, CRC(f9c8dd26) SHA1(25a9eea1d49b21b4a988beb32c25bf2f7796f227) ) /* Plane 2,3 */
	/* 0c00000-0ffffff empty */
	ROM_LOAD16_BYTE( "064-c5.bin", 0x1000000, 0x200000, CRC(8e34a9f4) SHA1(67b839b426ef3fad0a85d951fdd44c0a45c55226) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c6.bin", 0x1000001, 0x200000, CRC(a43e4766) SHA1(54f282f2b1ff2934cca7acbb4386a2b99a29df3a) ) /* Plane 2,3 */
	/* 1400000-17fffff empty */
	ROM_LOAD16_BYTE( "064-c7.bin", 0x1800000, 0x200000, CRC(59d97215) SHA1(85a960dc7f364df13ee0c2f99a4c53aefb081486) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "064-c8.bin", 0x1800001, 0x200000, CRC(fc092367) SHA1(69ff4ae909dd857de3ca8645d63f8b4bde117448) ) /* Plane 2,3 */
ROM_END

ROM_START( kabukikl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "092-p1.bin", 0x100000, 0x100000, CRC(28ec9b77) SHA1(7cdc789a99f8127f437d68cbc41278c926be9efd) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "092-s1.bin", CRC(a3d68ee2) SHA1(386f6110a16967a72fbf788f9d968fddcdcd2889) )

	NEO_BIOS_AUDIO_128K( "092-m1.bin", CRC(91957ef6) SHA1(7b6907532a0e02ceb643623cbd689cf228776ed1) )

	ROM_REGION( 0x700000, "ymsnd", 0 )
	ROM_LOAD( "092-v1.bin", 0x000000, 0x200000, CRC(69e90596) SHA1(1a2007d7784b3ce90d115980c3353862f1664d45) )
	ROM_LOAD( "092-v2.bin", 0x200000, 0x200000, CRC(7abdb75d) SHA1(0bff764889fe02f37877514c7fc450250839f632) )
	ROM_LOAD( "092-v3.bin", 0x400000, 0x200000, CRC(eccc98d3) SHA1(b0dfbdb1ea045cb961323ac6906ab342256c3dc7) )
	ROM_LOAD( "092-v4.bin", 0x600000, 0x100000, CRC(a7c9c949) SHA1(574bc55b45e81ce357b14f5992426115de25cd35) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "092-c1.bin", 0x000000, 0x400000, CRC(2a9fab01) SHA1(aa9f037df33ae0575b328734c76c0918ae1917e9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "092-c2.bin", 0x000001, 0x400000, CRC(6d2bac02) SHA1(dfe96b62883333872be432e8af1ae617c9e62698) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "092-c3.bin", 0x800000, 0x400000, CRC(5da735d6) SHA1(f1c05a73794ece15576a0a30c81f4a44faac475a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "092-c4.bin", 0x800001, 0x400000, CRC(de07f997) SHA1(c27a4d4bef868eed38dc152ff37d4135b16cc991) ) /* Plane 2,3 */
ROM_END

ROM_START( neobombe )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "093-p1.bin", 0x000000, 0x100000, CRC(a1a71d0d) SHA1(059284c84f61a825923d86d2f29c91baa2c439cd) )

	NEO_SFIX_128K( "093-s1.bin", CRC(4b3fa119) SHA1(41cb0909bfb017eb6f2c530cb92a423319ed7ab1) )

	NEO_BIOS_AUDIO_128K( "093-m1.bin", CRC(e81e780b) SHA1(c56c53984e0f92e180e850c60a75f550ee84917c) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "093-v1.bin", 0x000000, 0x400000, CRC(02abd4b0) SHA1(6bf33ebc9b01cd4a029f6a555694a9835e30ca1b) )
	ROM_LOAD( "093-v2.bin", 0x400000, 0x200000, CRC(a92b8b3d) SHA1(b672c97b85d2f52eba3cb26025008ebc7a18312a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x900000, "sprites", 0 )
	ROM_LOAD16_BYTE( "093-c1.bin", 0x000000, 0x400000, CRC(d1f328f8) SHA1(ddf71280c2ce85225f15fe9e973f330609281878) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "093-c2.bin", 0x000001, 0x400000, CRC(82c49540) SHA1(5f37c1bc0d63c98a13967b44da3d2c85e6dbbe50) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "093-c3.bin", 0x800000, 0x080000, CRC(e37578c5) SHA1(20024caa0f09ee887a6418dd02d02a0df93786fd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "093-c4.bin", 0x800001, 0x080000, CRC(59826783) SHA1(0110a2b6186cca95f75225d4d0269d61c2ad25b1) ) /* Plane 2,3 */
ROM_END

ROM_START( gowcaizr )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "094-p1.bin", 0x100000, 0x100000, CRC(33019545) SHA1(213db6c0b7d24b74b809854f9c606dbea1d9ba00) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "094-s1.bin", CRC(2f8748a2) SHA1(5cc723c4284120473d63d8b0c1a3b3be74bdc324) )

	NEO_BIOS_AUDIO_128K( "094-m1.bin", CRC(78c851cb) SHA1(a9923c002e4e2171a564af45cff0958c5d57b275) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "094-v1.bin", 0x000000, 0x200000, CRC(6c31223c) SHA1(ede3a2806d7d872a0f737626a23ecce200b534e6) )
	ROM_LOAD( "094-v2.bin", 0x200000, 0x200000, CRC(8edb776c) SHA1(a9eac5e24f83ccdcf303d63261747b1bad876a24) )
	ROM_LOAD( "094-v3.bin", 0x400000, 0x100000, CRC(c63b9285) SHA1(6bbbacfe899e204e74657d6c3f3d05ce75e432f1) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "094-c1.bin", 0x000000, 0x200000, CRC(042f6af5) SHA1(1c50df6a1a53ffb3079ea0a19c746f5c9536a3ed) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "094-c2.bin", 0x000001, 0x200000, CRC(0fbcd046) SHA1(9a6dc920a877f27424477c3478907b23afbaa5ea) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "094-c3.bin", 0x400000, 0x200000, CRC(58bfbaa1) SHA1(4c6f9cf138c5e6dfe89a45e2a690a986c75f5bfc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "094-c4.bin", 0x400001, 0x200000, CRC(9451ee73) SHA1(7befee4a886b1d7493c06cefb7abf4ec01c14a8b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "094-c5.bin", 0x800000, 0x200000, CRC(ff9cf48c) SHA1(5f46fb5d0812275b0006919d8540f22be7c16492) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "094-c6.bin", 0x800001, 0x200000, CRC(31bbd918) SHA1(7ff8c5e3f17d40e7a8a189ad8f8026de55368810) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "094-c7.bin", 0xc00000, 0x200000, CRC(2091ec04) SHA1(a81d4bdbef1ac6ea49845dc30e31bf9745694100) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "094-c8.bin", 0xc00001, 0x200000, CRC(0d31dee6) SHA1(4979aa3daa7e490fbe39e7b3c70cbb2ef7551c5f) ) /* Plane 2,3 */
ROM_END

ROM_START( rbff1 ) /* MVS AND AES VERSION */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "095-p1.bin", 0x000000, 0x100000, CRC(63b4d8ae) SHA1(03aa9f6bab6aee685d1b57a52823797704eea845) )
	ROM_LOAD16_WORD_SWAP( "095-p2.bin", 0x100000, 0x200000, CRC(cc15826e) SHA1(44d6ac6c0ca697a6f367dcfd809b1e1771cb0635) )

	NEO_SFIX_128K( "095-s1.bin", CRC(b6bf5e08) SHA1(b527355c35ea097f3448676f2ffa65b8e56ae30c) )

	NEO_BIOS_AUDIO_128K( "095-m1.bin", CRC(653492a7) SHA1(39e511fb9ed5d2135dc8428a31d0baafb2ab36e0) )

	ROM_REGION( 0xc00000, "ymsnd", 0 )
	ROM_LOAD( "069-v1.bin", 0x000000, 0x400000, CRC(2bdbd4db) SHA1(5f4fecf69c2329d699cbd45829c19303b1e2a80e) )
	ROM_LOAD( "069-v2.bin", 0x400000, 0x400000, CRC(a698a487) SHA1(11b8bc53bc26a51f4a408e900e3769958625c4ed) )
	ROM_LOAD( "095-v3.bin", 0x800000, 0x400000, CRC(189d1c6c) SHA1(f0b8cd1ee40ea3feeb2800f0723b451ec8240203) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1c00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "069-c1.bin", 0x0000000, 0x400000, CRC(e302f93c) SHA1(d8610b14900b2b8fe691b67ca9b1abb335dbff74) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "069-c2.bin", 0x0000001, 0x400000, CRC(1053a455) SHA1(69501bfac68739e63d798045b812badd251d57b8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "069-c3.bin", 0x0800000, 0x400000, CRC(1c0fde2f) SHA1(cf6c2ef56c03a861de3b0b6dc0d7c9204d947f9d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "069-c4.bin", 0x0800001, 0x400000, CRC(a25fc3d0) SHA1(83cb349e2f1032652060b233e741fb893be5af16) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "095-c5.bin", 0x1000000, 0x400000, CRC(8b9b65df) SHA1(e2a7e20855501f240bcd22f5cc92fcb4a9806abe) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "095-c6.bin", 0x1000001, 0x400000, CRC(3e164718) SHA1(53217f938c8964c1ca68a6fd5249c4169a5ac8e6) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "095-c7.bin", 0x1800000, 0x200000, CRC(ca605e12) SHA1(5150b835247fd705bc1dece97d423d9c20a51416) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "095-c8.bin", 0x1800001, 0x200000, CRC(4e6beb6c) SHA1(c0ac7cfc832ace6ad52c58f5da3a8101baead749) ) /* Plane 2,3 */
ROM_END

ROM_START( rbff1a ) /* MVS VERSION */
	ROM_REGION( 0x300000, "maincpu", 0 )

	/* This is a bug fixed revision applied over the original cart. The original P1 and P2 stayed in the cart and this */
	/* 512k ROM was added to replace the first 512k of P1. */

	ROM_LOAD16_WORD_SWAP( "095-p1.bin",  0x000000, 0x100000, CRC(63b4d8ae) SHA1(03aa9f6bab6aee685d1b57a52823797704eea845) )
	ROM_LOAD16_WORD_SWAP( "095-p2.bin",  0x100000, 0x200000, CRC(cc15826e) SHA1(44d6ac6c0ca697a6f367dcfd809b1e1771cb0635) )
	/* the rom below acts as a patch to the program rom in the cart, replacing the first 512kb */
	ROM_LOAD16_WORD_SWAP( "095-ep1.bin", 0x000000, 0x080000, CRC(be0060a3) SHA1(fa741d34898ad5004a23e280139d1446f1a082c7) )

	NEO_SFIX_128K( "095-s1.bin", CRC(b6bf5e08) SHA1(b527355c35ea097f3448676f2ffa65b8e56ae30c) )

	NEO_BIOS_AUDIO_128K( "095-m1.bin", CRC(653492a7) SHA1(39e511fb9ed5d2135dc8428a31d0baafb2ab36e0) )

	ROM_REGION( 0xc00000, "ymsnd", 0 )
	ROM_LOAD( "069-v1.bin", 0x000000, 0x400000, CRC(2bdbd4db) SHA1(5f4fecf69c2329d699cbd45829c19303b1e2a80e) )
	ROM_LOAD( "069-v2.bin", 0x400000, 0x400000, CRC(a698a487) SHA1(11b8bc53bc26a51f4a408e900e3769958625c4ed) )
	ROM_LOAD( "095-v3.bin", 0x800000, 0x400000, CRC(189d1c6c) SHA1(f0b8cd1ee40ea3feeb2800f0723b451ec8240203) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1c00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "069-c1.bin", 0x0000000, 0x400000, CRC(e302f93c) SHA1(d8610b14900b2b8fe691b67ca9b1abb335dbff74) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "069-c2.bin", 0x0000001, 0x400000, CRC(1053a455) SHA1(69501bfac68739e63d798045b812badd251d57b8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "069-c3.bin", 0x0800000, 0x400000, CRC(1c0fde2f) SHA1(cf6c2ef56c03a861de3b0b6dc0d7c9204d947f9d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "069-c4.bin", 0x0800001, 0x400000, CRC(a25fc3d0) SHA1(83cb349e2f1032652060b233e741fb893be5af16) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "095-c5.bin", 0x1000000, 0x400000, CRC(8b9b65df) SHA1(e2a7e20855501f240bcd22f5cc92fcb4a9806abe) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "095-c6.bin", 0x1000001, 0x400000, CRC(3e164718) SHA1(53217f938c8964c1ca68a6fd5249c4169a5ac8e6) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "095-c7.bin", 0x1800000, 0x200000, CRC(ca605e12) SHA1(5150b835247fd705bc1dece97d423d9c20a51416) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "095-c8.bin", 0x1800001, 0x200000, CRC(4e6beb6c) SHA1(c0ac7cfc832ace6ad52c58f5da3a8101baead749) ) /* Plane 2,3 */
ROM_END

ROM_START( aof3 )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "096-p1.bin", 0x000000, 0x100000, CRC(9edb420d) SHA1(150d80707325ece351c72c21c6186cfb5996adba) )
	ROM_LOAD16_WORD_SWAP( "096-p2.bin", 0x100000, 0x200000, CRC(4d5a2602) SHA1(4c26d6135d2877d9c38169662033e9d0cc24d943) )

	NEO_SFIX_128K( "096-s1.bin", CRC(cc7fd344) SHA1(2c6846cf8ea61fb192ba181dbccb63594d572c0e) )

	NEO_BIOS_AUDIO_128K( "096-m1.bin", CRC(cb07b659) SHA1(940b379957c2987d7ab0443cb80c3ff58f6ba559) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "096-v1.bin", 0x000000, 0x200000, CRC(e2c32074) SHA1(69426e7e63fc31a73d1cd056cc9ae6a2c4499407) )
	ROM_LOAD( "096-v2.bin", 0x200000, 0x200000, CRC(a290eee7) SHA1(e66a98cd9740188bf999992b417f8feef941cede) )
	ROM_LOAD( "096-v3.bin", 0x400000, 0x200000, CRC(199d12ea) SHA1(a883bf34e685487705a8dafdd0b8db15eb360e80) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1c00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "096-c1.bin", 0x0000000, 0x400000, CRC(f17b8d89) SHA1(7180df23f7c7a964b0835fda76970b12f0aa9ea8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "096-c2.bin", 0x0000001, 0x400000, CRC(3840c508) SHA1(55adc7cd26fec3e4dbd779df6701bc6eaba41b84) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "096-c3.bin", 0x0800000, 0x400000, CRC(55f9ee1e) SHA1(fbe1b7891beae66c5fcbc7e36168dc1b460ede91) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "096-c4.bin", 0x0800001, 0x400000, CRC(585b7e47) SHA1(d50ea91397fc53d86470ff5b493a44d57c010306) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "096-c5.bin", 0x1000000, 0x400000, CRC(c75a753c) SHA1(fc977f8710816a369a5d0d49ee84059380e93fb7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "096-c6.bin", 0x1000001, 0x400000, CRC(9a9d2f7a) SHA1(a89a713bfcd93974c9acb21ce699d365b08e7e39) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "096-c7.bin", 0x1800000, 0x200000, CRC(51bd8ab2) SHA1(c8def9c64de64571492b5b7e14b794e3c18f1393) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "096-c8.bin", 0x1800001, 0x200000, CRC(9a34f99c) SHA1(fca72d95ec42790a7f1e771a1e25dbc5bec5fc19) ) /* Plane 2,3 */
ROM_END

ROM_START( aof3k ) /* KOREAN VERSION */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "196-p1k.bin", 0x000000, 0x100000, CRC(a0780789) SHA1(83657922a9a3502653ef8cda45b15d9f935aa96a) )
	ROM_LOAD16_WORD_SWAP( "096-p2.bin",  0x100000, 0x200000, CRC(4d5a2602) SHA1(4c26d6135d2877d9c38169662033e9d0cc24d943) )

	NEO_SFIX_128K( "096-s1.bin", CRC(cc7fd344) SHA1(2c6846cf8ea61fb192ba181dbccb63594d572c0e) )

	NEO_BIOS_AUDIO_128K( "096-m1.bin", CRC(cb07b659) SHA1(940b379957c2987d7ab0443cb80c3ff58f6ba559) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "096-v1.bin", 0x000000, 0x200000, CRC(e2c32074) SHA1(69426e7e63fc31a73d1cd056cc9ae6a2c4499407) )
	ROM_LOAD( "096-v2.bin", 0x200000, 0x200000, CRC(a290eee7) SHA1(e66a98cd9740188bf999992b417f8feef941cede) )
	ROM_LOAD( "096-v3.bin", 0x400000, 0x200000, CRC(199d12ea) SHA1(a883bf34e685487705a8dafdd0b8db15eb360e80) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1c00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "096-c1.bin", 0x0000000, 0x400000, CRC(f17b8d89) SHA1(7180df23f7c7a964b0835fda76970b12f0aa9ea8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "096-c2.bin", 0x0000001, 0x400000, CRC(3840c508) SHA1(55adc7cd26fec3e4dbd779df6701bc6eaba41b84) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "096-c3.bin", 0x0800000, 0x400000, CRC(55f9ee1e) SHA1(fbe1b7891beae66c5fcbc7e36168dc1b460ede91) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "096-c4.bin", 0x0800001, 0x400000, CRC(585b7e47) SHA1(d50ea91397fc53d86470ff5b493a44d57c010306) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "096-c5.bin", 0x1000000, 0x400000, CRC(c75a753c) SHA1(fc977f8710816a369a5d0d49ee84059380e93fb7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "096-c6.bin", 0x1000001, 0x400000, CRC(9a9d2f7a) SHA1(a89a713bfcd93974c9acb21ce699d365b08e7e39) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "096-c7.bin", 0x1800000, 0x200000, CRC(51bd8ab2) SHA1(c8def9c64de64571492b5b7e14b794e3c18f1393) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "096-c8.bin", 0x1800001, 0x200000, CRC(9a34f99c) SHA1(fca72d95ec42790a7f1e771a1e25dbc5bec5fc19) ) /* Plane 2,3 */
ROM_END

ROM_START( sonicwi3 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "097-p1.bin", 0x100000, 0x100000, CRC(0547121d) SHA1(e0bb6c614f572b74ba9a9f0d3d5b69fbc91ebc52) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "097-s1.bin", CRC(8dd66743) SHA1(39214bb25a1d5b44a8524010be05bf5a0211981f) )

	NEO_BIOS_AUDIO_128K( "097-m1.bin", CRC(b20e4291) SHA1(0e891ab53f9fded510295dfc7818bc59b4a9dd97) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "097-v1.bin", 0x000000, 0x400000, CRC(6f885152) SHA1(8175804d5c1420c5d37b733d4a8fa2aa81e59f1b) )
	ROM_LOAD( "097-v2.bin", 0x400000, 0x200000, CRC(3359e868) SHA1(b7efd9f1a6dab33271fe8356bcc863aeae1d3ed8) )

	NO_DELTAT_REGION

	ROM_REGION( 0xc00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "097-c1.bin", 0x000000, 0x400000, CRC(33d0d589) SHA1(fe4aa95555e478ceb2d28fd27d83ee06cd09520c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "097-c2.bin", 0x000001, 0x400000, CRC(186f8b43) SHA1(f5cced93e21dc841b00ebeaa30786cb0e047bd9a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "097-c3.bin", 0x800000, 0x200000, CRC(c339fff5) SHA1(58dfd1e30dc0ad3f816a5dbd1cc7e7ccbb792c53) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "097-c4.bin", 0x800001, 0x200000, CRC(84a40c6e) SHA1(061a13fba5fed883e5ee9566cedc208df2511bcf) ) /* Plane 2,3 */
ROM_END

ROM_START( turfmast )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "200-p1.bin", 0x100000, 0x100000, CRC(28c83048) SHA1(e7ef87e1de21d2bb17ef17bb08657e92363f0e9a) )
	ROM_CONTINUE( 0x000000, 0x100000)

	NEO_SFIX_128K( "200-s1.bin", CRC(9a5402b2) SHA1(ae1a0b5450869d61b2bb23671c744d3dda8769c4) )

	NEO_BIOS_AUDIO_128K( "200-m1.bin", CRC(9994ac00) SHA1(7bded797f3b80fd00bcbe451ac0abe6646b19a14) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "200-v1.bin", 0x000000, 0x200000, CRC(00fd48d2) SHA1(ddfee09328632e598fd51537b3ae8593219b2111) )
	ROM_LOAD( "200-v2.bin", 0x200000, 0x200000, CRC(082acb31) SHA1(2f1c053040e9d50a6d45fd7bea1b96742bae694f) )
	ROM_LOAD( "200-v3.bin", 0x400000, 0x200000, CRC(7abca053) SHA1(e229bc0ea82a371d6ee8fd9fe442b0fd141d0a71) )
	ROM_LOAD( "200-v4.bin", 0x600000, 0x200000, CRC(6c7b4902) SHA1(d55e0f542d928a9a851133ff26763c8236cbbd4d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "200-c1.bin", 0x000000, 0x400000, CRC(8e7bf41a) SHA1(148eb747f2f4d8e921eb0411c88a636022ceab80) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "200-c2.bin", 0x000001, 0x400000, CRC(5a65a8ce) SHA1(d6c7afe035411f3eacdf6868d36f91572dd593e0) ) /* Plane 2,3 */
ROM_END

ROM_START( mslug )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "201-p1.bin", 0x100000, 0x100000, CRC(08d8daa5) SHA1(b888993dbb7e9f0a28a01d7d2e1da00ef9cf6f38) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "201-s1.bin", CRC(2f55958d) SHA1(550b53628daec9f1e1e11a398854092d90f9505a) )

	NEO_BIOS_AUDIO_128K( "201-m1.bin", CRC(c28b3253) SHA1(fd75bd15aed30266a8b3775f276f997af57d1c06) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "201-v1.bin", 0x000000, 0x400000, CRC(23d22ed1) SHA1(cd076928468ad6bcc5f19f88cb843ecb5e660681) )
	ROM_LOAD( "201-v2.bin", 0x400000, 0x400000, CRC(472cf9db) SHA1(5f79ea9286d22ed208128f9c31ca75552ce08b57) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "201-c1.bin", 0x000000, 0x400000, CRC(72813676) SHA1(7b045d1a48980cb1a140699011cb1a3d4acdc4d1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "201-c2.bin", 0x000001, 0x400000, CRC(96f62574) SHA1(cb7254b885989223bba597b8ff0972dfa5957816) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "201-c3.bin", 0x800000, 0x400000, CRC(5121456a) SHA1(0a7a27d603d1bb2520b5570ebf5b34a106e255a6) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "201-c4.bin", 0x800001, 0x400000, CRC(f4ad59a3) SHA1(4e94fda8ee63abf0f92afe08060a488546e5c280) ) /* Plane 2,3 */
ROM_END

ROM_START( puzzledp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "202-p1.bin", 0x000000, 0x080000, CRC(2b61415b) SHA1(0e3e4faf2fd6e63407425e1ac788003e75aeeb4f) )

	NEO_SFIX_128K( "202-s1.bin", CRC(cd19264f) SHA1(531be2305cd56d332fb7a53ab924214ade34a9e8) )

	NEO_BIOS_AUDIO_128K( "202-m1.bin", CRC(9c0291ea) SHA1(3fa67c62acba79be6b3a98cc1601e45569fa11ae) )

	ROM_REGION( 0x080000, "ymsnd", 0 )
	ROM_LOAD( "202-v1.bin", 0x000000, 0x080000, CRC(debeb8fb) SHA1(49a3d3578c087f1a0050168571ef8d1b08c5dc05) )

	NO_DELTAT_REGION

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "202-c1.bin", 0x000000, 0x100000, CRC(cc0095ef) SHA1(3d86f455e6db10a2449b775dc386f1826ba3b62e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "202-c2.bin", 0x000001, 0x100000, CRC(42371307) SHA1(df794f989e2883634bf7ffeea48d6bc3854529af) ) /* Plane 2,3 */
ROM_END

ROM_START( mosyougi )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "203-p1.bin", 0x000000, 0x100000, CRC(7ba70e2d) SHA1(945f472cc3e7706f613c52df18de35c986d166e7) )

	NEO_SFIX_128K( "203-s1.bin", CRC(bfdc8309) SHA1(781337eab932a130b396a6c1080611d6f9c24c6e) )

	NEO_BIOS_AUDIO_128K( "203-m1.bin", CRC(a602c2c2) SHA1(19fd5d0379244c528b58343f6cbf78b4766fb23d) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "203-v1.bin", 0x000000, 0x200000, CRC(baa2b9a5) SHA1(914782b6c81d9a76ce02251575592b0648434ba3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "203-c1.bin", 0x000000, 0x200000, CRC(bba9e8c0) SHA1(db89b7275a59ae6104a8308025c7e142a67b947b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "203-c2.bin", 0x000001, 0x200000, CRC(2574be03) SHA1(198cfd697c623022919ae4118928a7fe30cd6c46) ) /* Plane 2,3 */
ROM_END

ROM_START( marukodq )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "206-p1.bin", 0x000000, 0x100000, CRC(c33ed21e) SHA1(bffff0d17e587e67672227e60c0ebd3f3a7193e6) )

	NEO_SFIX_128K( "206-s1.bin", CRC(f0b68780) SHA1(3f60950b14d121a5af3e6a8155ae9832ddc6ec46) )

	NEO_BIOS_AUDIO_128K( "206-m1.bin", CRC(0e22902e) SHA1(fb8466c342d4abd8bb4cad01c6ceab03f96cdad8) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "206-v1.bin", 0x000000, 0x200000, CRC(5385eca8) SHA1(1ca171ce74a5885ae8841d0924de21dc0af2214e) )
	ROM_LOAD( "206-v2.bin", 0x200000, 0x200000, CRC(f8c55404) SHA1(cecc41e9e08a7ff05b6f62e713fc86a816bf55a2) )

	NO_DELTAT_REGION

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "206-c1.bin", 0x000000, 0x400000, CRC(846e4e8e) SHA1(ba9b96340aca7fadaff0e6d484391ddb5c5e7bd4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "206-c2.bin", 0x000001, 0x400000, CRC(1cba876d) SHA1(3254ceb5a2f76c172930d9889d5d81e093e87628) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "206-c3.bin", 0x800000, 0x100000, CRC(79aa2b48) SHA1(31f94217cd35f48845c74a55256314c16fd26ed7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "206-c4.bin", 0x800001, 0x100000, CRC(55e1314d) SHA1(fffbc9eb9000ff5b1063af1817de7ea4a267fedd) ) /* Plane 2,3 */
ROM_END

ROM_START( neomrdo )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "207-p1.bin", 0x000000, 0x80000, CRC(39efdb82) SHA1(75fe68921f871872e5fc92594e43b4cd712e819b) )

	NEO_SFIX_128K( "207-s1.bin", CRC(6aebafce) SHA1(5db03715fbed62f2ff3cef7f93606f30261c0362) )

	NEO_BIOS_AUDIO_128K( "207-m1.bin", CRC(81eade02) SHA1(d6d135bc525f3ed14a8c255f0b83d83a52b0659c) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "207-v1.bin", 0x000000, 0x200000, CRC(4143c052) SHA1(561b19bc8811b80f2f42ffc0b5df27132696470a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "207-c1.bin", 0x000000, 0x200000, CRC(c7541b9d) SHA1(25ca1a2b14cc2648d8dbe432cbd1396017af822c) )
	ROM_LOAD16_BYTE( "207-c2.bin", 0x000001, 0x200000, CRC(f57166d2) SHA1(bf3aa47d17156485c2177fb63cba093f050abb98) )
ROM_END

ROM_START( sdodgeb )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "208-p1.bin", 0x100000, 0x100000, CRC(127f3d32) SHA1(18e77b79b1197a89371533ef9b1e4d682c44d875) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "208-s1.bin", CRC(64abd6b3) SHA1(0315d724e4d83a44ce84c531ff9b8c398363c039) )

	NEO_BIOS_AUDIO_128K( "208-m1.bin", CRC(0a5f3325) SHA1(04e0236df478a5452654c823dcb42fea65b6a718) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "208-v1.bin", 0x000000, 0x200000, CRC(8b53e945) SHA1(beb7d63f6101f8435f35321fddb8479d312505c4) )
	ROM_LOAD( "208-v2.bin", 0x200000, 0x200000, CRC(af37ebf8) SHA1(f5c511479483533480f2b9ecf8edd4b7ae64c2d9) )

	NO_DELTAT_REGION

	ROM_REGION( 0x0c00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "208-c1.bin", 0x0000000, 0x400000, CRC(93d8619b) SHA1(6588cb67e38722d5843fb29943d92e3905101aff) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "208-c2.bin", 0x0000001, 0x400000, CRC(1c737bb6) SHA1(8e341989981a713e61dfed8bde9a6459583ef46d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "208-c3.bin", 0x0800000, 0x200000, CRC(14cb1703) SHA1(a46acec03c1b2351fe36810628f02b7c848d13db) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "208-c4.bin", 0x0800001, 0x200000, CRC(c7165f19) SHA1(221f03de893dca0e5305fa17aa94f96c67713818) ) /* Plane 2,3 */
ROM_END

ROM_START( goalx3 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "209-p1.bin", 0x100000, 0x100000, CRC(2a019a79) SHA1(422a639e74284fef2e53e1b49cf8803b0a7e80c6) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "209-s1.bin", CRC(c0eaad86) SHA1(99412093c9707d51817893971e73fb8469cdc9d0) )

	NEO_BIOS_AUDIO_128K( "209-m1.bin", CRC(cd758325) SHA1(b51eac634fc646c07210dff993018ad9ebabd3f9) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "209-v1.bin", 0x000000, 0x200000, CRC(ef214212) SHA1(3e05ccaa2d06decb18b379b96f900c0e6b39ce70) )

	NO_DELTAT_REGION

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "209-c1.bin", 0x000000, 0x400000, CRC(b49d980e) SHA1(722d10074f16fa7f14c71270f43fdab427b85e2b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "209-c2.bin", 0x000001, 0x400000, CRC(5649b015) SHA1(9c9674f3841e6becd3b8e63bae9b9df45ac9f11e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "209-c3.bin", 0x800000, 0x100000, CRC(5f91bace) SHA1(3864be27dce6d8f8828d3bf09bfc8116116a2b56) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "209-c4.bin", 0x800001, 0x100000, CRC(1e9f76f2) SHA1(b57fdc226bfe328b8848127fb4292295f1287bf6) ) /* Plane 2,3 */
ROM_END

ROM_START( overtop )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "212-p1.bin", 0x100000, 0x100000, CRC(16c063a9) SHA1(5432869f830eed816ee5ed71c7fd39f749d15619) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "212-s1.bin", CRC(481d3ddc) SHA1(7b0df3fc5b19f282abfd0eb5a4c6ed836a536ece) )

	NEO_BIOS_AUDIO_128K( "212-m1.bin", CRC(fcab6191) SHA1(488b8310b0957f0012fe50f73641b606f6ac4a57) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "212-v1.bin", 0x000000, 0x400000, CRC(013d4ef9) SHA1(438a697c44525bdf78b54432c4f7217ab5667047) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "212-c1.bin", 0x0000000, 0x400000, CRC(50f43087) SHA1(e5a8c914ef8e77c7a29bffdeb18f1877b5c2fc7d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "212-c2.bin", 0x0000001, 0x400000, CRC(a5b39807) SHA1(e98e82cf99576cb48cc5e8dc655b7e9a428c2843) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "212-c3.bin", 0x0800000, 0x400000, CRC(9252ea02) SHA1(269066e0f893d3e8e7c308528026a486c2b023a2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "212-c4.bin", 0x0800001, 0x400000, CRC(5f41a699) SHA1(abbb162658e06a37db8475b659ece7e1270ebb49) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "212-c5.bin", 0x1000000, 0x200000, CRC(fc858bef) SHA1(0031def13e7cf4a465a1eca7aa0d13d1b21427e2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "212-c6.bin", 0x1000001, 0x200000, CRC(0589c15e) SHA1(b1167caf7cb61f3e05a5d342290bfe00e02e9d38) ) /* Plane 2,3 */
ROM_END

ROM_START( neodrift )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "213-p1.bin", 0x100000, 0x100000, CRC(e397d798) SHA1(10f459111db4bab7aaa63ca47e83304a84300812) )
	ROM_CONTINUE( 0x000000, 0x100000)

	NEO_SFIX_128K( "213-s1.bin", CRC(b76b61bc) SHA1(5fdb407d16ab9e33c4f26ee09ff70891ae1d2bd0) )

	NEO_BIOS_AUDIO_128K( "213-m1.bin", CRC(200045f1) SHA1(7a6cd1c8d4447ea260d7ff4520c676b8d685f2e4) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "213-v1.bin", 0x000000, 0x200000, CRC(a421c076) SHA1(129f05c1a28a6493442f47a79c2d3577a1a43ef5) )
	ROM_LOAD( "213-v2.bin", 0x200000, 0x200000, CRC(233c7dd9) SHA1(be7f980aa83831b6605aaaf4ec904180bb96c935) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "213-c1.bin", 0x000000, 0x400000, CRC(3edc8bd3) SHA1(71dcba9afd3b08ebfa13294644dcb365c2740780) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "213-c2.bin", 0x000001, 0x400000, CRC(46ae5f16) SHA1(a01310632734e776e889af6a531063cb1661c33a) ) /* Plane 2,3 */
ROM_END

ROM_START( kof96 ) /* MVS VERSION */
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "214-p1.bin", 0x000000, 0x100000, CRC(52755d74) SHA1(4232d627f1d2e6ea9fc8cf01571d77d4d5b8a1bb) )
	ROM_LOAD16_WORD_SWAP( "214-p2.bin", 0x100000, 0x200000, CRC(002ccb73) SHA1(3ae8df682c75027ca82db25491021eeba00a267e) )

	NEO_SFIX_128K( "214-s1.bin", CRC(1254cbdb) SHA1(fce5cf42588298711a3633e9c9c1d4dcb723ac76) )

	NEO_BIOS_AUDIO_128K( "214-m1.bin", CRC(dabc427c) SHA1(b76722ed142ee7addceb4757424870dbd003e8b3) )

	ROM_REGION( 0xa00000, "ymsnd", 0 )
	ROM_LOAD( "214-v1.bin", 0x000000, 0x400000, CRC(63f7b045) SHA1(1353715f1a8476dca6f8031d9e7a401eacab8159) )
	ROM_LOAD( "214-v2.bin", 0x400000, 0x400000, CRC(25929059) SHA1(6a721c4cb8f8dc772774023877d4a9f50d5a9e31) )
	ROM_LOAD( "214-v3.bin", 0x800000, 0x200000, CRC(92a2257d) SHA1(5064aec78fa0d104e5dd5869b95382aa170214ee) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "214-c1.bin", 0x0000000, 0x400000, CRC(7ecf4aa2) SHA1(f773c4c1f05d58dd37e7bb2ac1d1e0ec43998a71) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "214-c2.bin", 0x0000001, 0x400000, CRC(05b54f37) SHA1(cc31653fe4cb05201fba234e080cb9c7a7592b1b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "214-c3.bin", 0x0800000, 0x400000, CRC(64989a65) SHA1(e6f3749d43be0afa9dad7b085cb782ba694252ca) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "214-c4.bin", 0x0800001, 0x400000, CRC(afbea515) SHA1(ae875052728de33174827705646bd14cf3937b5c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "214-c5.bin", 0x1000000, 0x400000, CRC(2a3bbd26) SHA1(7c1a7e50a10a1b082e0d0d515c34135ee9f995ac) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "214-c6.bin", 0x1000001, 0x400000, CRC(44d30dc7) SHA1(c8ae001e37224b55d9e4a4d99f6578b4f6eb055f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "214-c7.bin", 0x1800000, 0x400000, CRC(3687331b) SHA1(2be95caab76d7af51674f93884330ba73a6053e4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "214-c8.bin", 0x1800001, 0x400000, CRC(fa1461ad) SHA1(6c71a7f08e4044214223a6bf80984582ab5e0328) ) /* Plane 2,3 */
ROM_END

ROM_START( kof96h )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "214-pg1.bin",0x000000, 0x100000, CRC(bd3757c9) SHA1(35392a044117e46c088ff0fdd07d69a3faa4f96e) )
	ROM_LOAD16_WORD_SWAP( "214-p2.bin", 0x100000, 0x200000, CRC(002ccb73) SHA1(3ae8df682c75027ca82db25491021eeba00a267e) )

	NEO_SFIX_128K( "214-s1.bin", CRC(1254cbdb) SHA1(fce5cf42588298711a3633e9c9c1d4dcb723ac76) )

	NEO_BIOS_AUDIO_128K( "214-m1.bin", CRC(dabc427c) SHA1(b76722ed142ee7addceb4757424870dbd003e8b3) )

	ROM_REGION( 0xa00000, "ymsnd", 0 )
	ROM_LOAD( "214-v1.bin", 0x000000, 0x400000, CRC(63f7b045) SHA1(1353715f1a8476dca6f8031d9e7a401eacab8159) )
	ROM_LOAD( "214-v2.bin", 0x400000, 0x400000, CRC(25929059) SHA1(6a721c4cb8f8dc772774023877d4a9f50d5a9e31) )
	ROM_LOAD( "214-v3.bin", 0x800000, 0x200000, CRC(92a2257d) SHA1(5064aec78fa0d104e5dd5869b95382aa170214ee) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "214-c1.bin", 0x0000000, 0x400000, CRC(7ecf4aa2) SHA1(f773c4c1f05d58dd37e7bb2ac1d1e0ec43998a71) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "214-c2.bin", 0x0000001, 0x400000, CRC(05b54f37) SHA1(cc31653fe4cb05201fba234e080cb9c7a7592b1b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "214-c3.bin", 0x0800000, 0x400000, CRC(64989a65) SHA1(e6f3749d43be0afa9dad7b085cb782ba694252ca) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "214-c4.bin", 0x0800001, 0x400000, CRC(afbea515) SHA1(ae875052728de33174827705646bd14cf3937b5c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "214-c5.bin", 0x1000000, 0x400000, CRC(2a3bbd26) SHA1(7c1a7e50a10a1b082e0d0d515c34135ee9f995ac) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "214-c6.bin", 0x1000001, 0x400000, CRC(44d30dc7) SHA1(c8ae001e37224b55d9e4a4d99f6578b4f6eb055f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "214-c7.bin", 0x1800000, 0x400000, CRC(3687331b) SHA1(2be95caab76d7af51674f93884330ba73a6053e4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "214-c8.bin", 0x1800001, 0x400000, CRC(fa1461ad) SHA1(6c71a7f08e4044214223a6bf80984582ab5e0328) ) /* Plane 2,3 */
ROM_END

ROM_START( ssideki4 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "215-p1.bin", 0x100000, 0x100000, CRC(519b4ba3) SHA1(5aa59514b23aa663f2c4014ee94a31e9f59151de) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "215-s1.bin", CRC(f0fe5c36) SHA1(b7badd6d2ac3788ce5cace1fcf5cdad14734e4e6) )

	NEO_BIOS_AUDIO_128K( "215-m1.bin", CRC(a932081d) SHA1(376a45e19edb780ac8798c41ae2260c8a8a4bba8) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "215-v1.bin", 0x000000, 0x400000, CRC(877d1409) SHA1(77c58ebffe677ea6369c964ec7975b11df512fa1) )
	ROM_LOAD( "215-v2.bin", 0x400000, 0x200000, CRC(1bfa218b) SHA1(344836a578bde3c0ab59b58c8734f868e7403c26) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "215-c1.bin", 0x0000000, 0x400000, CRC(8ff444f5) SHA1(e2dc52d09512cb378df96ddf45435f9bcbbe9947) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "215-c2.bin", 0x0000001, 0x400000, CRC(5b155037) SHA1(68900c0fdcd35c9f38e0effdf27e1dbd3c53daf8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "215-c3.bin", 0x0800000, 0x400000, CRC(456a073a) SHA1(3488013f371012eab4e788e1525c81260e0b7080) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "215-c4.bin", 0x0800001, 0x400000, CRC(43c182e1) SHA1(343f034c65ca498b437e22e06a866a5daf3b9602) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "215-c5.bin", 0x1000000, 0x200000, CRC(0c6f97ec) SHA1(b8d297f0ba2b04404eb0f7c6673ecc206fadae0c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "215-c6.bin", 0x1000001, 0x200000, CRC(329c5e1b) SHA1(015c36b8d3efab9b4647f110ecb5c118a9c80f43) ) /* Plane 2,3 */
ROM_END

ROM_START( kizuna )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "216-p1.bin", 0x100000, 0x100000, CRC(75d2b3de) SHA1(ee778656c26828935ee2a2bfd0ce5a22aa681c10) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "216-s1.bin", CRC(efdc72d7) SHA1(be37cbf1852e2e4c907cc799b754b538544b6703) )

	NEO_BIOS_AUDIO_128K( "216-m1.bin", CRC(1b096820) SHA1(72852e78c620038f8dafde5e54e02e418c31be9c) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "059-v1.bin", 0x000000, 0x200000, CRC(530c50fd) SHA1(29401cee7f7d2c199c7cb58092e86b28205e81ad) )
	ROM_LOAD( "216-v2.bin", 0x200000, 0x200000, CRC(03667a8d) SHA1(3b0475e553a49f8788f32b0c84f82645cc6b4273) )
	ROM_LOAD( "059-v3.bin", 0x400000, 0x200000, CRC(7038c2f9) SHA1(c1d6f86b24feba03fe009b58199d2eeabe572f4e) )
	ROM_LOAD( "216-v4.bin", 0x600000, 0x200000, CRC(31b99bd6) SHA1(5871751f8e9e6b98337472c22b5e1c7ede0a9311) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1c00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "059-c1.bin", 0x0000000, 0x200000, CRC(763ba611) SHA1(d3262e0332c894ee149c5963f882cc5e5562ee57) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "059-c2.bin", 0x0000001, 0x200000, CRC(e05e8ca6) SHA1(986a9b16ff92bc101ab567d2d01348e093abea9a) ) /* Plane 2,3 */
	/* 400000-7fffff empty */
	ROM_LOAD16_BYTE( "216-c3.bin", 0x0800000, 0x400000, CRC(665c9f16) SHA1(7ec781a49a462f395b450460b29493f55134eac2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "216-c4.bin", 0x0800001, 0x400000, CRC(7f5d03db) SHA1(365ed266c121f4df0bb76898955a8ae0e668a216) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "059-c5.bin", 0x1000000, 0x200000, CRC(59013f9e) SHA1(5bf48fcc450da72a8c4685f6e3887e67eae49988) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "059-c6.bin", 0x1000001, 0x200000, CRC(1c8d5def) SHA1(475d89a5c4922a9f6bd756d23c2624d57b6e9d62) ) /* Plane 2,3 */
	/* 1400000-17fffff empty */
	ROM_LOAD16_BYTE( "059-c7.bin", 0x1800000, 0x200000, CRC(c88f7035) SHA1(c29a428b741f4fe7b71a3bc23c87925b6bc1ca8f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "059-c8.bin", 0x1800001, 0x200000, CRC(484ce3ba) SHA1(4f21ed20ce6e2b67e2b079404599310c94f591ff) ) /* Plane 2,3 */
ROM_END

ROM_START( ninjamas )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "217-p1.bin", 0x000000, 0x100000, CRC(3e97ed69) SHA1(336bcae375a5109945d11356503bf0d9f4a9a50a) )
	ROM_LOAD16_WORD_SWAP( "217-p2.bin", 0x100000, 0x200000, CRC(191fca88) SHA1(e318e5931704779bbe461719a5eeeba89bd83a5d) )

	NEO_SFIX_128K( "217-s1.bin", CRC(8ff782f0) SHA1(90099c154357042ba658d4ef6abe4d9335bb7172) )

	NEO_BIOS_AUDIO_128K( "217-m1.bin", CRC(d00fb2af) SHA1(6bcaa52e1641cc24288e1f22f4dc98e8d8921b90) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "217-v1.bin", 0x000000, 0x400000, CRC(1c34e013) SHA1(5368e413d2188c4fd063b6bb7d5f498ff83ea812) )
	ROM_LOAD( "217-v2.bin", 0x400000, 0x200000, CRC(22f1c681) SHA1(09da03b2e63d180e55173ff25e8735c4162f027b) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "217-c1.bin", 0x0000000, 0x400000, CRC(5fe97bc4) SHA1(d76c955d83baa2b9fd24222a9b2852947b7b92f0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "217-c2.bin", 0x0000001, 0x400000, CRC(886e0d66) SHA1(d407e1525e4ebe996e14f6e5c0396a10f736a50d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "217-c3.bin", 0x0800000, 0x400000, CRC(59e8525f) SHA1(19f602c71545d6c021dc72e112d3a8b8efe7a9b7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "217-c4.bin", 0x0800001, 0x400000, CRC(8521add2) SHA1(0d1a6f2979302c4c282e31ff334d2d887aec74f7) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "217-c5.bin", 0x1000000, 0x400000, CRC(fb1896e5) SHA1(777a8caa9ebdbddf89e3d5ab650c94a55228ce54) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "217-c6.bin", 0x1000001, 0x400000, CRC(1c98c54b) SHA1(cb1cad161d9b9f2f5a7cf8ae4d6d35b51acf90f5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "217-c7.bin", 0x1800000, 0x400000, CRC(8b0ede2e) SHA1(ea632ac98291ddac95441b7fe2349974b2da8a42) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "217-c8.bin", 0x1800001, 0x400000, CRC(a085bb61) SHA1(6a3e9e6ba96072b8849b407f2b24103dc0852259) ) /* Plane 2,3 */
ROM_END

ROM_START( ragnagrd )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "218-p1.bin", 0x100000, 0x100000, CRC(ca372303) SHA1(67991e4fef9b36bc7d909810eebb857ac2f906f1) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "218-s1.bin", CRC(7d402f9a) SHA1(59ec29d03e62e7a8bef689a124a9164f43b2ace1) )

	NEO_BIOS_AUDIO_128K( "218-m1.bin", CRC(17028bcf) SHA1(7a4e8f33ce9b41beac2152b8f6003f247699e2e1) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "218-v1.bin", 0x000000, 0x400000, CRC(61eee7f4) SHA1(5b11b1a0b1b74dfbc2998cbda9f8f7a5e9059957) )
	ROM_LOAD( "218-v2.bin", 0x400000, 0x400000, CRC(6104e20b) SHA1(18e8aae3e51e141977d523a10e737ff68fe81910) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "218-c1.bin", 0x0000000, 0x400000, CRC(c31500a4) SHA1(cc82100038988872721028044ed2e9764bcc2fb0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "218-c2.bin", 0x0000001, 0x400000, CRC(98aba1f9) SHA1(121276c569967e501d8e1b83747f1bdebff612ea) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "218-c3.bin", 0x0800000, 0x400000, CRC(833c163a) SHA1(b7e5356bbd9efab67fedb5bc671ba8bbd661fe0f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "218-c4.bin", 0x0800001, 0x400000, CRC(c1a30f69) SHA1(f87ddda4695abcd14f5c2d4b7d41f72ad5b064cc) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "218-c5.bin", 0x1000000, 0x400000, CRC(6b6de0ff) SHA1(1abb24cb407258235f4a572cf101d0774823040b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "218-c6.bin", 0x1000001, 0x400000, CRC(94beefcf) SHA1(d2ff0bac325c9c823dba68bd4f281b3b9f8f68e7) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "218-c7.bin", 0x1800000, 0x400000, CRC(de6f9b28) SHA1(455adb6bb986af8a00d7f32b7f4f3715fc3007f6) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "218-c8.bin", 0x1800001, 0x400000, CRC(d9b311f6) SHA1(ba61a7ab3f08bb7348ad6cd01e5d29ca5ee75074) ) /* Plane 2,3 */
ROM_END

ROM_START( pgoal )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "219-p1.bin", 0x100000, 0x100000, CRC(6af0e574) SHA1(c3f0fed0d942e48c99c80b1713f271c033ce0f4f) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "219-s1.bin", CRC(002f3c88) SHA1(a8a5bbc5397c8ae9858e38997ebdc713b7b4f50a) )

	NEO_BIOS_AUDIO_128K( "219-m1.bin", CRC(958efdc8) SHA1(aacc6056b1ff48cde8f241a11a27473cfb4b4aa3) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "219-v1.bin", 0x000000, 0x200000, CRC(2cc1bd05) SHA1(e204b6d359e5b37661dddc2c1f14d64fd9089f75) )
	ROM_LOAD( "219-v2.bin", 0x200000, 0x200000, CRC(06ac1d3f) SHA1(c7f2287e4e8366b78bc26ede7802e2e92d44d7c9) )

	NO_DELTAT_REGION

	ROM_REGION( 0xc00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "219-c1.bin", 0x0000000, 0x200000, CRC(2dc69faf) SHA1(658327fdea658ed40f0074fe8800423cc9d553d5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "219-c2.bin", 0x0000001, 0x200000, CRC(5db81811) SHA1(362c4df6ae62e125c0942ff8661a771ff32afbd7) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "219-c3.bin", 0x0400000, 0x200000, CRC(9dbfece5) SHA1(f52e2a34faab0416ea152559ff9089fb988b1f0b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "219-c4.bin", 0x0400001, 0x200000, CRC(c9f4324c) SHA1(9d0dfcd39e72cbd63252b71b3ed879971b6f3443) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "219-c5.bin", 0x0800000, 0x200000, CRC(5fdad0a5) SHA1(56f6d2a7224aa4e82a1858079f918e85cadbd6c2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "219-c6.bin", 0x0800001, 0x200000, CRC(f57b4a1c) SHA1(875ca69afbc5304ec23f4bc9186abe92f477f6c8) ) /* Plane 2,3 */
ROM_END

ROM_START( magdrop2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "221-p1.bin", 0x000000, 0x80000, CRC(7be82353) SHA1(08ab39f52b893591c13a7d7aa26b20ce86e9ddf5) )

	NEO_SFIX_128K( "221-s1.bin", CRC(2a4063a3) SHA1(0e09a7d88d85b1a2100888f4211960ea56ef978b) )

	NEO_BIOS_AUDIO_128K( "221-m1.bin", CRC(bddae628) SHA1(02c77e6aaaed43e39778bf83a3184e7c21db63d4) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "221-v1.bin", 0x000000, 0x200000, CRC(7e5e53e4) SHA1(72b063b2d4acaaf72a20d14ad5bfc90cb64d3fed) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "221-c1.bin", 0x000000, 0x400000, CRC(1f862a14) SHA1(1253e8b65d863d552d00dbdbfc5c168f5fc7edd1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "221-c2.bin", 0x000001, 0x400000, CRC(14b90536) SHA1(e0d41f6b84d8261729f154b44ddd95c9b9c0714a) ) /* Plane 2,3 */
ROM_END

ROM_START( samsho4 )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "222-p1.bin", 0x000000, 0x100000, CRC(1a5cb56d) SHA1(9a0a5a1c7c5d428829f22d3d17f7033d43a51b5b) )
	ROM_LOAD16_WORD_SWAP( "222-p2.bin", 0x100000, 0x400000, CRC(b023cd8b) SHA1(35b4cec9858225f90acdfa16ed8a3017d0d08327) )

	NEO_SFIX_128K( "222-s1.bin", CRC(8d3d3bf9) SHA1(9975ed9b458bdd14e23451d2534153f68a5e4e6c) )

	NEO_BIOS_AUDIO_128K( "222-m1.bin", CRC(7615bc1b) SHA1(b936f7b341f6fe0921b4c41049734684583e3596) )

	ROM_REGION( 0xa00000, "ymsnd", 0 )
	ROM_LOAD( "222-v1.bin", 0x000000, 0x400000, CRC(7d6ba95f) SHA1(03cb4e0d770e0b332b07b64cacef624460b84c78) )
	ROM_LOAD( "222-v2.bin", 0x400000, 0x400000, CRC(6c33bb5d) SHA1(fd5d4e08a962dd0d22c52c91bad5ec7f23cfb901) )
	ROM_LOAD( "222-v3.bin", 0x800000, 0x200000, CRC(831ea8c0) SHA1(f2987b7d09bdc4311e972ce8a9ab7ca9802db4db) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "222-c1.bin", 0x0000000, 0x400000, CRC(68f2ed95) SHA1(c0a02df012cd25bcfe341770ea861a80294148cb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "222-c2.bin", 0x0000001, 0x400000, CRC(a6e9aff0) SHA1(15addca49951ed53fa3c000c8d7cd327d012a620) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "222-c3.bin", 0x0800000, 0x400000, CRC(c91b40f4) SHA1(dcda45e0336204e3e024de08edfd0a3217bc1fdd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "222-c4.bin", 0x0800001, 0x400000, CRC(359510a4) SHA1(b6642677ebdff7788263266402080272b8a66b15) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "222-c5.bin", 0x1000000, 0x400000, CRC(9cfbb22d) SHA1(789c32f917d0c6e38601cd390a7bf9d803131a4a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "222-c6.bin", 0x1000001, 0x400000, CRC(685efc32) SHA1(db21ba1c7e3631ce0f1cb6f503ae7e0e043ff71b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "222-c7.bin", 0x1800000, 0x400000, CRC(d0f86f0d) SHA1(32502d71c2ab1469c492b6b382bf2bb3f85981d9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "222-c8.bin", 0x1800001, 0x400000, CRC(adfc50e3) SHA1(7d7ee874355b5aa75ad9c9a5c9c3df98d098d85e) ) /* Plane 2,3 */
ROM_END

ROM_START( samsho4k ) /* Uses PCBs "NEO-MVS PROGBK1" and "NEO-MVS CHA256 B" */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "222-p1k.p1", 0x000000, 0x100000, CRC(06e0a25d) SHA1(81e6727e0acb67ae41383518c0fc07f28d232979) )
	ROM_LOAD16_WORD_SWAP( "222-p2.bin", 0x100000, 0x400000, CRC(b023cd8b) SHA1(35b4cec9858225f90acdfa16ed8a3017d0d08327) )

	NEO_SFIX_128K( "222-s1k.s1", CRC(d313687d) SHA1(9ba468a9d582ef76c863f57ad9a0f811f3f08bd9) )

	NEO_BIOS_AUDIO_128K( "222-m1.bin", CRC(7615bc1b) SHA1(b936f7b341f6fe0921b4c41049734684583e3596) )

	ROM_REGION( 0xa00000, "ymsnd", 0 )
	ROM_LOAD( "222-v1.bin", 0x000000, 0x400000, CRC(7d6ba95f) SHA1(03cb4e0d770e0b332b07b64cacef624460b84c78) )
	ROM_LOAD( "222-v2.bin", 0x400000, 0x400000, CRC(6c33bb5d) SHA1(fd5d4e08a962dd0d22c52c91bad5ec7f23cfb901) )
	ROM_LOAD( "222-v3.bin", 0x800000, 0x200000, CRC(831ea8c0) SHA1(f2987b7d09bdc4311e972ce8a9ab7ca9802db4db) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "222-c1.bin", 0x0000000, 0x400000, CRC(68f2ed95) SHA1(c0a02df012cd25bcfe341770ea861a80294148cb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "222-c2.bin", 0x0000001, 0x400000, CRC(a6e9aff0) SHA1(15addca49951ed53fa3c000c8d7cd327d012a620) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "222-c3.bin", 0x0800000, 0x400000, CRC(c91b40f4) SHA1(dcda45e0336204e3e024de08edfd0a3217bc1fdd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "222-c4.bin", 0x0800001, 0x400000, CRC(359510a4) SHA1(b6642677ebdff7788263266402080272b8a66b15) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "222-c5.bin", 0x1000000, 0x400000, CRC(9cfbb22d) SHA1(789c32f917d0c6e38601cd390a7bf9d803131a4a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "222-c6.bin", 0x1000001, 0x400000, CRC(685efc32) SHA1(db21ba1c7e3631ce0f1cb6f503ae7e0e043ff71b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "222-c7.bin", 0x1800000, 0x400000, CRC(d0f86f0d) SHA1(32502d71c2ab1469c492b6b382bf2bb3f85981d9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "222-c8.bin", 0x1800001, 0x400000, CRC(adfc50e3) SHA1(7d7ee874355b5aa75ad9c9a5c9c3df98d098d85e) ) /* Plane 2,3 */
ROM_END

ROM_START( rbffspec )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "223-p1.bin", 0x000000, 0x100000, CRC(f84a2d1d) SHA1(fc19225d9dbdb6bd0808023ee32c7829f6ffdef6) )
	ROM_LOAD16_WORD_SWAP( "223-p2.bin", 0x100000, 0x400000, CRC(addd8f08) SHA1(abaf5b86c8ec915c07ef2d83fce9ad03acaa4817) )

	NEO_SFIX_128K( "223-s1.bin", CRC(7ecd6e8c) SHA1(465455afc4d83cbb118142be4671b2539ffafd79) )

	NEO_BIOS_AUDIO_128K( "223-m1.bin", CRC(3fee46bf) SHA1(e750f85233953853618fcdff980a4721af1710a3) )

	ROM_REGION( 0xc00000, "ymsnd", 0 )
	ROM_LOAD( "223-v1.bin", 0x000000, 0x400000, CRC(76673869) SHA1(78a26afa29f73de552ffabdbf3fc4bf26be8ae9e) )
	ROM_LOAD( "223-v2.bin", 0x400000, 0x400000, CRC(7a275acd) SHA1(8afe87ce822614262b72a90b371fc79155ac0d0c) )
	ROM_LOAD( "223-v3.bin", 0x800000, 0x400000, CRC(5a797fd2) SHA1(94958e334f86d4d71059af8138f255b8d97a3b01) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "223-c1.bin", 0x0000000, 0x400000, CRC(ebab05e2) SHA1(0d60a8b631e3a3dcfbfdd7779dee081c9548ec39) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "223-c2.bin", 0x0000001, 0x400000, CRC(641868c3) SHA1(aa1aeb661842276b3326bfa4f1456f75bfecd52e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "223-c3.bin", 0x0800000, 0x400000, CRC(ca00191f) SHA1(96977febfcc513e1848d7029ff169cdf51104038) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "223-c4.bin", 0x0800001, 0x400000, CRC(1f23d860) SHA1(e18df52f898a51074e07a0b8c6e75873e7cde35e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "223-c5.bin", 0x1000000, 0x400000, CRC(321e362c) SHA1(39bd189334278f266124c97c6f70995f6f171cea) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "223-c6.bin", 0x1000001, 0x400000, CRC(d8fcef90) SHA1(bbccacb27f1e587bc144fe7ce68bd7b327ceaaee) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "223-c7.bin", 0x1800000, 0x400000, CRC(bc80dd2d) SHA1(086f372015eede88c6c578595fe915e28a589d2f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "223-c8.bin", 0x1800001, 0x400000, CRC(5ad62102) SHA1(e28cc9840caed2a1a8bd65a03bef05231071040c) ) /* Plane 2,3 */
ROM_END

ROM_START( twinspri )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "224-p1.bin", 0x100000, 0x100000, CRC(7697e445) SHA1(5b55ca120f77a931d40719b14e0bfc8cac1d628c) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "224-s1.bin", CRC(eeed5758) SHA1(24e48f396716e145b692468762cf595fb7267873) )

	NEO_BIOS_AUDIO_128K( "224-m1.bin", CRC(364d6f96) SHA1(779b95a6476089b71f48c8368d9043ee1dba9032) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "224-v1.bin", 0x000000, 0x400000, CRC(ff57f088) SHA1(1641989b8aac899dbd68aa2332bcdf9b90b33564) )
	ROM_LOAD( "224-v2.bin", 0x400000, 0x200000, CRC(7ad26599) SHA1(822030037b7664795bf3d64e1452d0aecc22497e) )

	NO_DELTAT_REGION

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "224-c1.bin", 0x000000, 0x400000, CRC(f7da64ab) SHA1(587a10ed9235c9046a3523fe80feba07764fac9b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "224-c2.bin", 0x000001, 0x400000, CRC(4c09bbfb) SHA1(e781aafba3bdefb7ed152826f4c3eb441735331c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "224-c3.bin", 0x800000, 0x100000, CRC(c59e4129) SHA1(93f02d1b4fbb152a9d336494fbff0d7642921de5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "224-c4.bin", 0x800001, 0x100000, CRC(b5532e53) SHA1(7d896c25ba97f6e5d43c13d4df4ba72964a976ed) ) /* Plane 2,3 */
ROM_END

ROM_START( wakuwak7 )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "225-p1.bin", 0x000000, 0x100000, CRC(b14da766) SHA1(bdffd72ff705fc6b085a4026217bac1c4bc93163) )
	ROM_LOAD16_WORD_SWAP( "225-p2.bin", 0x100000, 0x200000, CRC(fe190665) SHA1(739d9a8fc2da34381654d9e291141eacc210ae5c) )

	NEO_SFIX_128K( "225-s1.bin", CRC(71c4b4b5) SHA1(9410f13807f01082dc86f2d84051be4bed8e9f7c) )

	NEO_BIOS_AUDIO_128K( "225-m1.bin", CRC(0634bba6) SHA1(153aaf016440500df7a4454f3f2f2911219cb7d8) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "225-v1.bin", 0x000000, 0x400000, CRC(6195c6b4) SHA1(66c06b5904aedb256e3997bbec60f8ab50c6ff0c) )
	ROM_LOAD( "225-v2.bin", 0x400000, 0x400000, CRC(6159c5fe) SHA1(9015e93416497f1ef877c717afed40f7ecfa42e4) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "225-c1.bin", 0x0000000, 0x400000, CRC(ee4fea54) SHA1(314b513a52b2cc88cbf2409d1934c357269a8bb2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "225-c2.bin", 0x0000001, 0x400000, CRC(0c549e2d) SHA1(d8c4626231c92e43d9bf183202553ee2b5c532e6) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "225-c3.bin", 0x0800000, 0x400000, CRC(af0897c0) SHA1(2b8ec19b9dd0bd1f1171fb01b915e9d25ec8c421) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "225-c4.bin", 0x0800001, 0x400000, CRC(4c66527a) SHA1(6c8c9342fad70b456e282b0d52e7ad890e4673d3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "225-c5.bin", 0x1000000, 0x400000, CRC(8ecea2b5) SHA1(cad51e6e76d8258a78becb6f4096dd061f537494) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "225-c6.bin", 0x1000001, 0x400000, CRC(0eb11a6d) SHA1(c6d4f978ff3ca190a3060ac52bd7347189194f76) ) /* Plane 2,3 */
ROM_END

ROM_START( stakwin2 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "227-p1.bin", 0x100000, 0x100000, CRC(daf101d2) SHA1(96b90f884bae2969ebd8c04aba509928464e2433) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "227-s1.bin", CRC(2a8c4462) SHA1(9155fbb5fee6d46a68d17ea780a7a92565f9aa47) )

	NEO_BIOS_AUDIO_128K( "227-m1.bin", CRC(c8e5e0f9) SHA1(09bb05ae6f09b59b9e4871fae1fc7c3bafd07394) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "227-v1.bin", 0x000000, 0x400000, CRC(b8f24181) SHA1(0a3af88d20ff65b82c58325d32c20b99fc07f7f3) )
	ROM_LOAD( "227-v2.bin", 0x400000, 0x400000, CRC(ee39e260) SHA1(4ed6802564ce262ebe92c7276424056b70998758) )

	NO_DELTAT_REGION

	ROM_REGION( 0xc00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "227-c1.bin", 0x0000000, 0x400000, CRC(7d6c2af4) SHA1(e54f0ab15c95d7a6f965b5d8ab28b5445100650b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "227-c2.bin", 0x0000001, 0x400000, CRC(7e402d39) SHA1(9d3a44f98ddd0b606c8b3efa0c6b9d5a46c0bfeb) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "227-c3.bin", 0x0800000, 0x200000, CRC(93dfd660) SHA1(5b473c556ef919cd7a872351dbb20a636aae32b6) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "227-c4.bin", 0x0800001, 0x200000, CRC(7efea43a) SHA1(3f2b1718fe7be06b6d75ec34badc2de2a3554d3e) ) /* Plane 2,3 */
ROM_END

ROM_START( ghostlop )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "gl-p1.bin", 0x000000, 0x100000, CRC(6033172e) SHA1(f57fb706aa8dd9e5f9e992a5d35c1799578b59f8) )

	NEO_SFIX_128K( "gl-s1.bin", CRC(83c24e81) SHA1(585ef209d8bfc23bdccc1f37d8b764eeedfedc1c) )

	NEO_BIOS_AUDIO_128K( "gl-m1.bin", CRC(fd833b33) SHA1(ab6c218c42cba821654cbdae154efecb69f844f6) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "gl-v1.bin", 0x000000, 0x200000, CRC(c603fce6) SHA1(5a866471d35895b2ae13cbd5d1cb41bf2e72e1b8) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "gl-c1.bin", 0x000000, 0x400000, CRC(bfc99efe) SHA1(5cd2545310142080b8286e787cf5b859f627b3db) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "gl-c2.bin", 0x000001, 0x400000, CRC(69788082) SHA1(c3ecb42ddcbd9e16d0018a0c3adb56a911d813ca) ) /* Plane 2,3 */
ROM_END

ROM_START( breakers )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "230-p1.bin", 0x100000, 0x100000, CRC(ed24a6e6) SHA1(3fb77ae696d92d2f9a5d589e08b708545c7cda0a) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "230-s1.bin", CRC(076fb64c) SHA1(c166038128d7004f69932141f83b320a35c2b4ca) )

	NEO_BIOS_AUDIO_128K( "230-m1.bin", CRC(3951a1c1) SHA1(1e6442a7ea82ada9503d71045dd93e12bd05254f) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "230-v1.bin", 0x000000, 0x400000, CRC(7f9ed279) SHA1(acd0558960ec29bfc3e3ee99d00e503bebff8513) )
	ROM_LOAD( "230-v2.bin", 0x400000, 0x400000, CRC(1d43e420) SHA1(26d09b8b18b4b802dbda4d6f06626c24d0b7c512) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "230-c1.bin", 0x000000, 0x400000, CRC(68d4ae76) SHA1(2e820067f6963669f104bebf19e865fe4127b4dd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "230-c2.bin", 0x000001, 0x400000, CRC(fdee05cd) SHA1(efc4ffd790953ac7c25d5f045c64a9b49d24b096) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "230-c3.bin", 0x800000, 0x400000, CRC(645077f3) SHA1(0ae74f3b4b3b88f128c6d8c0f35ffa53f5d67ef2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "230-c4.bin", 0x800001, 0x400000, CRC(63aeb74c) SHA1(9ff6930c0c3d79b46b86356e8565ce4fcd69ac38) ) /* Plane 2,3 */
ROM_END

ROM_START( miexchng )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "231-p1.bin", 0x000000, 0x80000, CRC(61be1810) SHA1(1ab0e11352ca05329c6e3f5657b60e4a227fcbfb) )

	NEO_SFIX_128K( "231-s1.bin", CRC(fe0c0c53) SHA1(54d56d4463db193e504658f4f6f4997a62ae3d95) )

	NEO_BIOS_AUDIO_128K( "231-m1.bin", CRC(de41301b) SHA1(59ce3836ac8f064d56a446c9374f05bcb40fcfd8) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "231-v1.bin", 0x000000, 0x400000, CRC(113fb898) SHA1(9168ba90c4aa969f69eb11ba3f4d76592d81e05a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x500000, "sprites", 0 )
	ROM_LOAD16_BYTE( "231-c1.bin", 0x000000, 0x200000, CRC(6c403ba3) SHA1(3830446fbd07d5a6564f9ac68a4bec5ff5b7d5c9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "231-c2.bin", 0x000001, 0x200000, CRC(554bcd9b) SHA1(e658161618bd41a66f1040be409efdea28020cf6) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "231-c3.bin", 0x400000, 0x080000, CRC(14524eb5) SHA1(e090b99a1ee2cca4a7f7d5262e764d1cab6bada2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "231-c4.bin", 0x400001, 0x080000, CRC(1694f171) SHA1(a6af5d3f1e0b3f73da275e04d4434c6c9c7b9208) ) /* Plane 2,3 */
ROM_END

ROM_START( kof97 ) /* MVS VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "232-p1.bin", 0x000000, 0x100000, CRC(7db81ad9) SHA1(8bc42be872fd497eb198ca13bf004852b88eb1dc) )
	ROM_LOAD16_WORD_SWAP( "232-p2.bin", 0x100000, 0x400000, CRC(158b23f6) SHA1(9744620a70513490aaf9c5eda33e5ec31222be19) )

	NEO_SFIX_128K( "232-s1.bin", CRC(8514ecf5) SHA1(18d8e7feb51ea88816f1c786932a53655b0de6a0) )

	NEO_BIOS_AUDIO_128K( "232-m1.bin", CRC(45348747) SHA1(ed77cbae2b208d1177a9f5f6e8cd57070e90b65b) )

	ROM_REGION( 0xc00000, "ymsnd", 0 )
	ROM_LOAD( "232-v1.bin", 0x000000, 0x400000, CRC(22a2b5b5) SHA1(ebdbc977332e6d93e266755000b43857e0082965) )
	ROM_LOAD( "232-v2.bin", 0x400000, 0x400000, CRC(2304e744) SHA1(98d283e2bcc9291a53f52afd35ef76dfb0828432) )
	ROM_LOAD( "232-v3.bin", 0x800000, 0x400000, CRC(759eb954) SHA1(54e77c4e9e6b89458e59824e478ddc33a9c72655) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "232-c1.bin", 0x0000000, 0x800000, CRC(5f8bf0a1) SHA1(e8b63bbc814de171fd18c5864a7fc639970c1ecf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c2.bin", 0x0000001, 0x800000, CRC(e4d45c81) SHA1(fdb2b9326362e27b1c7a5beb977e0bc537488186) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "232-c3.bin", 0x1000000, 0x800000, CRC(581d6618) SHA1(14d3124a08ded59f86932c6b28e1a4e48c564ccd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c4.bin", 0x1000001, 0x800000, CRC(49bb1e68) SHA1(f769c1bd1b019521111ff3f0d22c63cb1f2640ef) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "232-c5.bin", 0x2000000, 0x400000, CRC(34fc4e51) SHA1(b39c65f27873f71a6f5a5d1d04e5435f874472ee) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c6.bin", 0x2000001, 0x400000, CRC(4ff4d47b) SHA1(4d5689ede24a5fe4330bd85d4d3f4eb2795308bb) ) /* Plane 2,3 */
ROM_END

ROM_START( kof97h ) /* AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "232-pg1.bin",0x000000, 0x100000, CRC(5c2400b7) SHA1(49e23f80c012c62146a1bb8f254a7597823de430) )
	ROM_LOAD16_WORD_SWAP( "232-p2.bin", 0x100000, 0x400000, CRC(158b23f6) SHA1(9744620a70513490aaf9c5eda33e5ec31222be19) )

	NEO_SFIX_128K( "232-s1.bin", CRC(8514ecf5) SHA1(18d8e7feb51ea88816f1c786932a53655b0de6a0) )

	NEO_BIOS_AUDIO_128K( "232-m1.bin", CRC(45348747) SHA1(ed77cbae2b208d1177a9f5f6e8cd57070e90b65b) )

	ROM_REGION( 0xc00000, "ymsnd", 0 )
	ROM_LOAD( "232-v1.bin", 0x000000, 0x400000, CRC(22a2b5b5) SHA1(ebdbc977332e6d93e266755000b43857e0082965) )
	ROM_LOAD( "232-v2.bin", 0x400000, 0x400000, CRC(2304e744) SHA1(98d283e2bcc9291a53f52afd35ef76dfb0828432) )
	ROM_LOAD( "232-v3.bin", 0x800000, 0x400000, CRC(759eb954) SHA1(54e77c4e9e6b89458e59824e478ddc33a9c72655) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "232-c1.bin", 0x0000000, 0x800000, CRC(5f8bf0a1) SHA1(e8b63bbc814de171fd18c5864a7fc639970c1ecf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c2.bin", 0x0000001, 0x800000, CRC(e4d45c81) SHA1(fdb2b9326362e27b1c7a5beb977e0bc537488186) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "232-c3.bin", 0x1000000, 0x800000, CRC(581d6618) SHA1(14d3124a08ded59f86932c6b28e1a4e48c564ccd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c4.bin", 0x1000001, 0x800000, CRC(49bb1e68) SHA1(f769c1bd1b019521111ff3f0d22c63cb1f2640ef) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "232-c5.bin", 0x2000000, 0x400000, CRC(34fc4e51) SHA1(b39c65f27873f71a6f5a5d1d04e5435f874472ee) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c6.bin", 0x2000001, 0x400000, CRC(4ff4d47b) SHA1(4d5689ede24a5fe4330bd85d4d3f4eb2795308bb) ) /* Plane 2,3 */
ROM_END

ROM_START( magdrop3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "233-p1.bin", 0x000000, 0x100000, CRC(931e17fa) SHA1(4a95c4b79d0878485ce272e9f4c4f647bec0e070) )

	NEO_SFIX_128K( "233-s1.bin", CRC(7399e68a) SHA1(b535ee56a0f0995f04674e676f6aa636ffad26aa) )

	NEO_BIOS_AUDIO_128K( "233-m1.bin", CRC(5beaf34e) SHA1(2905d26945932cddc2dd3a1dc5abba8aa3baee14) )

	ROM_REGION( 0x480000, "ymsnd", 0 )
	ROM_LOAD( "233-v1.bin", 0x000000, 0x400000, CRC(58839298) SHA1(18cae7bba997c52780761cbf119c4e4b34397a61) )
	ROM_LOAD( "233-v2.bin", 0x400000, 0x080000, CRC(d5e30df4) SHA1(bbbc0ff5b975471bd682f85976ac4a93f6d44f2e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "233-c1.bin", 0x000000, 0x400000, CRC(65e3f4c4) SHA1(a6deb75d802225327f8f1c2733a7f2b47e722e59) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "233-c2.bin", 0x000001, 0x400000, CRC(35dea6c9) SHA1(ea133bf947f950236f49d0ae0d1a9af3bc1a9a50) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "233-c3.bin", 0x800000, 0x400000, CRC(0ba2c502) SHA1(8e0f1e553aef04758aaaa14d5115f0ecace4391e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "233-c4.bin", 0x800001, 0x400000, CRC(70dbbd6d) SHA1(32dd6a04c6329e89f4878e7a56f0d172a6388eea) ) /* Plane 2,3 */
ROM_END

ROM_START( lastblad ) /* MVS VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "234-p1.bin", 0x000000, 0x100000, CRC(e123a5a3) SHA1(a3ddabc00feeb54272b145246612ad4632b0e413) )
	ROM_LOAD16_WORD_SWAP( "234-p2.bin", 0x100000, 0x400000, CRC(0fdc289e) SHA1(1ff31c0b0f4f9ddbedaf4bcf927faaae81892ec7) )

	NEO_SFIX_128K( "234-s1.bin", CRC(95561412) SHA1(995de272f572fd08d909d3d0af4251b9957b3640) )

	NEO_BIOS_AUDIO_128K( "234-m1.bin", CRC(087628ea) SHA1(48dcf739bb16699af4ab8ed632b7dcb25e470e06) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "234-v1.bin", 0x000000, 0x400000, CRC(ed66b76f) SHA1(8a05ff06d9b6f01c6c16b3026282eaabb0e25b44) )
	ROM_LOAD( "234-v2.bin", 0x400000, 0x400000, CRC(a0e7f6e2) SHA1(753ff74fa9294f695aae511ae01ead119b114a57) )
	ROM_LOAD( "234-v3.bin", 0x800000, 0x400000, CRC(a506e1e2) SHA1(b3e04ba1a5cb50b77c6fbe9fe353b9b64b6f3f74) )
	ROM_LOAD( "234-v4.bin", 0xc00000, 0x400000, CRC(0e34157f) SHA1(20A1F4833E5E29BA0073C1712D7A17AB7A2A035C) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "234-c1.bin", 0x0000000, 0x800000, CRC(9f7e2bd3) SHA1(2828aca0c0f5802110f10453c1cf640f69736554) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c2.bin", 0x0000001, 0x800000, CRC(80623d3c) SHA1(ad460615115ec8fb25206f012da59ecfc8059b64) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "234-c3.bin", 0x1000000, 0x800000, CRC(91ab1a30) SHA1(e3cf9133784bef2c8f1bfe45f277ccf82cc6f6a1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c4.bin", 0x1000001, 0x800000, CRC(3d60b037) SHA1(78a50233bcd19e92c7b6f7ee1a53417d9db21f6a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "234-c5.bin", 0x2000000, 0x400000, CRC(1ba80cee) SHA1(0c59057183b5279b747e73213b4cd3c6d7ad9eb1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c6.bin", 0x2000001, 0x400000, CRC(beafd091) SHA1(55df9cc128eb0f00856de3996c946e3efe8f09a5) ) /* Plane 2,3 */
ROM_END

ROM_START( lastbladh ) /* AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "234-pg1.bin", 0x000000, 0x100000, CRC(cd01c06d) SHA1(d66142571afe07c6191b52f319f1bc8bc8541c14) )
	ROM_LOAD16_WORD_SWAP( "234-p2.bin", 0x100000, 0x400000, CRC(0fdc289e) SHA1(1ff31c0b0f4f9ddbedaf4bcf927faaae81892ec7) )

	NEO_SFIX_128K( "234-s1.bin", CRC(95561412) SHA1(995de272f572fd08d909d3d0af4251b9957b3640) )

	NEO_BIOS_AUDIO_128K( "234-m1.bin", CRC(087628ea) SHA1(48dcf739bb16699af4ab8ed632b7dcb25e470e06) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "234-v1.bin", 0x000000, 0x400000, CRC(ed66b76f) SHA1(8a05ff06d9b6f01c6c16b3026282eaabb0e25b44) )
	ROM_LOAD( "234-v2.bin", 0x400000, 0x400000, CRC(a0e7f6e2) SHA1(753ff74fa9294f695aae511ae01ead119b114a57) )
	ROM_LOAD( "234-v3.bin", 0x800000, 0x400000, CRC(a506e1e2) SHA1(b3e04ba1a5cb50b77c6fbe9fe353b9b64b6f3f74) )
	ROM_LOAD( "234-v4.bin", 0xc00000, 0x400000, CRC(0e34157f) SHA1(20A1F4833E5E29BA0073C1712D7A17AB7A2A035C) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "234-c1.bin", 0x0000000, 0x800000, CRC(9f7e2bd3) SHA1(2828aca0c0f5802110f10453c1cf640f69736554) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c2.bin", 0x0000001, 0x800000, CRC(80623d3c) SHA1(ad460615115ec8fb25206f012da59ecfc8059b64) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "234-c3.bin", 0x1000000, 0x800000, CRC(91ab1a30) SHA1(e3cf9133784bef2c8f1bfe45f277ccf82cc6f6a1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c4.bin", 0x1000001, 0x800000, CRC(3d60b037) SHA1(78a50233bcd19e92c7b6f7ee1a53417d9db21f6a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "234-c5.bin", 0x2000000, 0x400000, CRC(1ba80cee) SHA1(0c59057183b5279b747e73213b4cd3c6d7ad9eb1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c6.bin", 0x2000001, 0x400000, CRC(beafd091) SHA1(55df9cc128eb0f00856de3996c946e3efe8f09a5) ) /* Plane 2,3 */
ROM_END

ROM_START( lastsold ) /* KOREAN VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "134-p1k.bin", 0x000000, 0x100000, CRC(906f3065) SHA1(25167665f1b8e82e13f7fcf4d0e3c54a925c2a58) )
	ROM_LOAD16_WORD_SWAP( "234-p2.bin",  0x100000, 0x400000, CRC(0fdc289e) SHA1(1ff31c0b0f4f9ddbedaf4bcf927faaae81892ec7) )

	NEO_SFIX_128K( "234-s1.bin", CRC(95561412) SHA1(995de272f572fd08d909d3d0af4251b9957b3640) )

	NEO_BIOS_AUDIO_128K( "234-m1.bin", CRC(087628ea) SHA1(48dcf739bb16699af4ab8ed632b7dcb25e470e06) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "234-v1.bin", 0x000000, 0x400000, CRC(ed66b76f) SHA1(8a05ff06d9b6f01c6c16b3026282eaabb0e25b44) )
	ROM_LOAD( "234-v2.bin", 0x400000, 0x400000, CRC(a0e7f6e2) SHA1(753ff74fa9294f695aae511ae01ead119b114a57) )
	ROM_LOAD( "234-v3.bin", 0x800000, 0x400000, CRC(a506e1e2) SHA1(b3e04ba1a5cb50b77c6fbe9fe353b9b64b6f3f74) )
	ROM_LOAD( "234-v4.bin", 0xc00000, 0x400000, CRC(0e34157f) SHA1(20A1F4833E5E29BA0073C1712D7A17AB7A2A035C) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "234-c1.bin", 0x0000000, 0x800000, CRC(9f7e2bd3) SHA1(2828aca0c0f5802110f10453c1cf640f69736554) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c2.bin", 0x0000001, 0x800000, CRC(80623d3c) SHA1(ad460615115ec8fb25206f012da59ecfc8059b64) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "234-c3.bin", 0x1000000, 0x800000, CRC(91ab1a30) SHA1(e3cf9133784bef2c8f1bfe45f277ccf82cc6f6a1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c4.bin", 0x1000001, 0x800000, CRC(3d60b037) SHA1(78a50233bcd19e92c7b6f7ee1a53417d9db21f6a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "234-c5.bin", 0x2000000, 0x400000, CRC(1ba80cee) SHA1(0c59057183b5279b747e73213b4cd3c6d7ad9eb1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "234-c6.bin", 0x2000001, 0x400000, CRC(beafd091) SHA1(55df9cc128eb0f00856de3996c946e3efe8f09a5) ) /* Plane 2,3 */
ROM_END

ROM_START( puzzldpr )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "235-p1.bin", 0x000000, 0x080000, CRC(afed5de2) SHA1(a5d82c6dbe687505e8c8d7339908da45cd379a0b) )

	NEO_SFIX_64K( "235-s1.bin", BAD_DUMP CRC(5a68d91e) SHA1(a8c58eb68fd7e6e2d9d1153a9da514430437f342) )

	NEO_BIOS_AUDIO_128K( "202-m1.bin", CRC(9c0291ea) SHA1(3fa67c62acba79be6b3a98cc1601e45569fa11ae) )

	ROM_REGION( 0x080000, "ymsnd", 0 )
	ROM_LOAD( "202-v1.bin", 0x000000, 0x080000, CRC(debeb8fb) SHA1(49a3d3578c087f1a0050168571ef8d1b08c5dc05) )

	NO_DELTAT_REGION

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD16_BYTE( "202-c1.bin", 0x000000, 0x100000, CRC(cc0095ef) SHA1(3d86f455e6db10a2449b775dc386f1826ba3b62e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "202-c2.bin", 0x000001, 0x100000, CRC(42371307) SHA1(df794f989e2883634bf7ffeea48d6bc3854529af) ) /* Plane 2,3 */
ROM_END

ROM_START( irrmaze )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "236-p1.bin", 0x000000, 0x200000, CRC(6d536c6e) SHA1(87d66683304a6617da8af7dfdfcbf4a3ab63056a) )

	NEO_SFIX_128K( "236-s1.bin", CRC(5d1ca640) SHA1(40a9668a1742a44597a07ce72273d17119815637) )

	ROM_REGION16_BE( 0x20000, "mainbios", 0 )
	/* special BIOS with trackball support, we only have one Irritating Maze bios and thats asia */
	ROM_LOAD16_WORD_SWAP("236-bios.bin", 0x00000, 0x020000, CRC(853e6b96) SHA1(de369cb4a7df147b55168fa7aaf0b98c753b735e) )

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "236-m1.bin", 0x00000, 0x20000, CRC(880a1abd) SHA1(905afa157aba700e798243b842792e50729b19a0) ) /* so overwrite it with the real thing */
	ROM_RELOAD( 0x10000, 0x20000 )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "236-v1.bin", 0x000000, 0x200000, CRC(5f89c3b4) SHA1(dc8fd561cf8dfdd41696dcf14ea8d2d0ac4eec4b) )

	ROM_REGION( 0x100000, "ymsnd.deltat", 0 )
	ROM_LOAD( "236-v2.bin", 0x000000, 0x100000, CRC(1e843567) SHA1(30d63887b4900571025b3077b9e41099a59c3ad9) )

	ROM_REGION( 0x0800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "236-c1.bin", 0x000000, 0x400000, CRC(c1d47902) SHA1(727001c34f979226fc8f581113ce2aaac4fc0d42) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "236-c2.bin", 0x000001, 0x400000, CRC(e15f972e) SHA1(6a329559c57a67be73a6733513b59e9e6c8d61cc) ) /* Plane 2,3 */
ROM_END

ROM_START( popbounc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "237-p1.bin", 0x000000, 0x100000, CRC(be96e44f) SHA1(43679da8664fbb491103a1108040ddf94d59fc2b) )

	NEO_SFIX_128K( "237-s1.bin", CRC(b61cf595) SHA1(b14f8b78af7c634d41cf34d36b11b116e61f7342) )

	NEO_BIOS_AUDIO_128K( "237-m1.bin", CRC(d4c946dd) SHA1(6ca09040b5db8d89511d627954c783154d58ab01) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "237-v1.bin", 0x000000, 0x200000, CRC(edcb1beb) SHA1(62f086b9968b366b59276ee4ae3c32c4d76fc6ce) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "237-c1.bin", 0x000000, 0x200000, CRC(eda42d66) SHA1(2735538fcb9dc0c16e043a8728c8b642650189f4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "237-c2.bin", 0x000001, 0x200000, CRC(5e633c65) SHA1(9a82107caf027317c173c1c1ef676f0fdeea79b2) ) /* Plane 2,3 */
ROM_END

ROM_START( shocktro )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "238-p1.bin", 0x000000, 0x100000, CRC(5677456f) SHA1(f76169fa5c90871d65e2a16fd1bb036c90533ac8) )
	ROM_LOAD16_WORD_SWAP( "238-p2.bin", 0x100000, 0x400000, CRC(5b4a09c5) SHA1(de04036cba2da4bb2da73d902d1822b82b4f67a9) )

	NEO_SFIX_128K( "238-s1.bin", CRC(1f95cedb) SHA1(adfa74868147fd260481e4c387d254d3b6de83f4) )

	NEO_BIOS_AUDIO_128K( "238-m1.bin", CRC(075b9518) SHA1(ac21b88a860b9572bf24432b4cadcc96d108055d) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "238-v1.bin", 0x000000, 0x400000, CRC(260c0bef) SHA1(9c4f80ce4bb205afed11bb8b8926d20748eb5512) )
	ROM_LOAD( "238-v2.bin", 0x400000, 0x200000, CRC(4ad7d59e) SHA1(bfdf2684f7f38af4e75ad0068ff9463dc2601598) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "238-c1.bin", 0x0000000, 0x400000, CRC(90c6a181) SHA1(a381bc8449718814ff12b3a4f7fc4d1bb7ea1631) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "238-c2.bin", 0x0000001, 0x400000, CRC(888720f0) SHA1(cd4d65df8d3ef0dbcca2b7f3f803f45c457f5beb) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "238-c3.bin", 0x0800000, 0x400000, CRC(2c393aa3) SHA1(1cd7cebe5861a2d65f1d6615dd7752162e573a02) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "238-c4.bin", 0x0800001, 0x400000, CRC(b9e909eb) SHA1(33cc9b2d13e4ed2ab6040ff582a53dc9bca402e0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "238-c5.bin", 0x1000000, 0x400000, CRC(c22c68eb) SHA1(a4b04118b1b1909d3b76be8d9ee5d97db6120600) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "238-c6.bin", 0x1000001, 0x400000, CRC(119323cd) SHA1(05a9d4b1fb4cc963b25452ff6f81e296e0c0b2a1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "238-c7.bin", 0x1800000, 0x400000, CRC(a72ce7ed) SHA1(05b151554bd7af09ccf554a17bc3c75a0512faaf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "238-c8.bin", 0x1800001, 0x400000, CRC(1c7c2efb) SHA1(b055ee43cbdaf9a3cb19e4e1f9dd2c40bde69d70) ) /* Plane 2,3 */
ROM_END

ROM_START( shocktra )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "238-pg1.bin", 0x000000, 0x100000, CRC(efedf8dc) SHA1(f638df9bf7aa7d514ee2bccfc7f2adbf39ca83fc) )
	ROM_LOAD16_WORD_SWAP( "238-p2.bin", 0x100000, 0x400000, CRC(5b4a09c5) SHA1(de04036cba2da4bb2da73d902d1822b82b4f67a9) )

	NEO_SFIX_128K( "238-s1.bin", CRC(1f95cedb) SHA1(adfa74868147fd260481e4c387d254d3b6de83f4) )

	NEO_BIOS_AUDIO_128K( "238-m1.bin", CRC(075b9518) SHA1(ac21b88a860b9572bf24432b4cadcc96d108055d) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "238-v1.bin", 0x000000, 0x400000, CRC(260c0bef) SHA1(9c4f80ce4bb205afed11bb8b8926d20748eb5512) )
	ROM_LOAD( "238-v2.bin", 0x400000, 0x200000, CRC(4ad7d59e) SHA1(bfdf2684f7f38af4e75ad0068ff9463dc2601598) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "238-c1.bin", 0x0000000, 0x400000, CRC(90c6a181) SHA1(a381bc8449718814ff12b3a4f7fc4d1bb7ea1631) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "238-c2.bin", 0x0000001, 0x400000, CRC(888720f0) SHA1(cd4d65df8d3ef0dbcca2b7f3f803f45c457f5beb) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "238-c3.bin", 0x0800000, 0x400000, CRC(2c393aa3) SHA1(1cd7cebe5861a2d65f1d6615dd7752162e573a02) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "238-c4.bin", 0x0800001, 0x400000, CRC(b9e909eb) SHA1(33cc9b2d13e4ed2ab6040ff582a53dc9bca402e0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "238-c5.bin", 0x1000000, 0x400000, CRC(c22c68eb) SHA1(a4b04118b1b1909d3b76be8d9ee5d97db6120600) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "238-c6.bin", 0x1000001, 0x400000, CRC(119323cd) SHA1(05a9d4b1fb4cc963b25452ff6f81e296e0c0b2a1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "238-c7.bin", 0x1800000, 0x400000, CRC(a72ce7ed) SHA1(05b151554bd7af09ccf554a17bc3c75a0512faaf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "238-c8.bin", 0x1800001, 0x400000, CRC(1c7c2efb) SHA1(b055ee43cbdaf9a3cb19e4e1f9dd2c40bde69d70) ) /* Plane 2,3 */
ROM_END

ROM_START( blazstar )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "239-p1.bin", 0x000000, 0x100000, CRC(183682f8) SHA1(dcee1c2cf4a991ca1f9f2b40c4a738f21682807b) )
	ROM_LOAD16_WORD_SWAP( "239-p2.bin", 0x100000, 0x200000, CRC(9a9f4154) SHA1(f8805453d0995c8fa16cd9accfb7a990071ca630) )

	NEO_SFIX_128K( "239-s1.bin", CRC(d56cb498) SHA1(420ce56431dc7f3f7de84fcbc8c0a17b5eab205e) )

	NEO_BIOS_AUDIO_128K( "239-m1.bin", CRC(d31a3aea) SHA1(e23abfeb23052f0358edcf2c83401025fe632511) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "239-v1.bin", 0x000000, 0x400000, CRC(1b8d5bf7) SHA1(67fc1f7e36e92a89cd1d415eb31a2892f57b0d04) )
	ROM_LOAD( "239-v2.bin", 0x400000, 0x400000, CRC(74cf0a70) SHA1(b00451a2a30de2517ae3eca35eb1fe985b950eb8) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "239-c1.bin", 0x0000000, 0x400000, CRC(84f6d584) SHA1(ff36db8504611b0d8d942d1e24823ff71e4aeb37) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "239-c2.bin", 0x0000001, 0x400000, CRC(05a0cb22) SHA1(4abe03e7f3a86f277131d413a3151c7b9c3646c8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "239-c3.bin", 0x0800000, 0x400000, CRC(5fb69c9e) SHA1(77b96518d8ad8ad120537e0f8ba65d69d1c33566) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "239-c4.bin", 0x0800001, 0x400000, CRC(0be028c4) SHA1(d3f8b37786ca7838c3525895a7f2b49afc8530d4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "239-c5.bin", 0x1000000, 0x400000, CRC(74bae5f8) SHA1(812c9a31f0721c2971a316b084ce69337dbe3747) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "239-c6.bin", 0x1000001, 0x400000, CRC(4e0700d2) SHA1(cd059fb713c403208923c17e1e8ef02fcfd2fe8d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "239-c7.bin", 0x1800000, 0x400000, CRC(010ff4fd) SHA1(2571d406442f007a7458d8ccb0939a9201c9c9bf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "239-c8.bin", 0x1800001, 0x400000, CRC(db60460e) SHA1(a5cb27c0983c8b400d96fd0828ef0639a66d4dba) ) /* Plane 2,3 */
ROM_END

ROM_START( rbff2 ) /* MVS VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "240-p1.bin", 0x000000, 0x100000, CRC(80e41205) SHA1(8f83eb8ff54be4ec40f8a0dd2cbe56c54908d00a) )
	ROM_LOAD16_WORD_SWAP( "240-p2.bin", 0x100000, 0x400000, CRC(960aa88d) SHA1(3d9e785891871af90313f178dca2724633406674) )

	NEO_SFIX_128K( "240-s1.bin", CRC(da3b40de) SHA1(e6bf74e057ac6fe1f249a7547f13ba7fbc694561) )

	/* TC532000DP */
	NEO_BIOS_AUDIO_256K( "240-m1.bin", CRC(ed482791) SHA1(1f54a45967cb7842c33aa24be322c9f33ff75ac3) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "240-v1.bin", 0x000000, 0x400000, CRC(f796265a) SHA1(736dff37eb91fc856b4d189249fb0de9b6c0813a) )
	ROM_LOAD( "240-v2.bin", 0x400000, 0x400000, CRC(2cb3f3bb) SHA1(697e677890892f4b028c9a27c66809ca0a8a9b18) )
	//ROM_LOAD( "240-v3.bin", 0x800000, 0x400000, CRC(df77b7fa) SHA1(4df971ce20bdb8c1ce8cc1692a32ac69505ffa9a) ) // bad?
	ROM_LOAD( "240-v3.bin", 0x800000, 0x400000, CRC(8fe1367a) SHA1(093d7a4ac2b54ad7ffb2dc316fe29415f7a99535) )
	ROM_LOAD( "240-v4.bin", 0xc00000, 0x200000, CRC(996704d8) SHA1(0bf7a1d0660199dedf3c25be757eeab75cc6147e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "240-c1.bin", 0x0000000, 0x800000, CRC(effac504) SHA1(e36a96e7369b02c7e839b5abf3c6799453ba1927) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c2.bin", 0x0000001, 0x800000, CRC(ed182d44) SHA1(a9fc0a3a786bf067c129ec7220df65953dff804f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "240-c3.bin", 0x1000000, 0x800000, CRC(22e0330a) SHA1(0fe7f6a8aeba7f17dbb278e85003969ff10d3cd2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c4.bin", 0x1000001, 0x800000, CRC(c19a07eb) SHA1(139eac8b51cadf328dd42d8109f4e2463f57230c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "240-c5.bin", 0x2000000, 0x800000, CRC(244dff5a) SHA1(156548156d3ceaa808d0053d0749af2526a3943e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c6.bin", 0x2000001, 0x800000, CRC(4609e507) SHA1(bb17f50a377dddb77c1eeda5944a7bcbf0cca5f7) ) /* Plane 2,3 */
ROM_END

ROM_START( rbff2h ) /* AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "240-pg1.bin", 0x000000, 0x100000, CRC(b6969780) SHA1(e3373d18e0f0724d69efb8024a27cca121f1b5b2) )
	ROM_LOAD16_WORD_SWAP( "240-p2.bin",  0x100000, 0x400000, CRC(960aa88d) SHA1(3d9e785891871af90313f178dca2724633406674) )

	NEO_SFIX_128K( "240-s1.bin", CRC(da3b40de) SHA1(e6bf74e057ac6fe1f249a7547f13ba7fbc694561) )

	/* TC532000DP */
	NEO_BIOS_AUDIO_256K( "240-m1.bin", CRC(ed482791) SHA1(1f54a45967cb7842c33aa24be322c9f33ff75ac3) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "240-v1.bin", 0x000000, 0x400000, CRC(f796265a) SHA1(736dff37eb91fc856b4d189249fb0de9b6c0813a) )
	ROM_LOAD( "240-v2.bin", 0x400000, 0x400000, CRC(2cb3f3bb) SHA1(697e677890892f4b028c9a27c66809ca0a8a9b18) )
	//ROM_LOAD( "240-v3.bin", 0x800000, 0x400000, CRC(df77b7fa) SHA1(4df971ce20bdb8c1ce8cc1692a32ac69505ffa9a) ) // bad?
	ROM_LOAD( "240-v3.bin", 0x800000, 0x400000, CRC(8fe1367a) SHA1(093d7a4ac2b54ad7ffb2dc316fe29415f7a99535) )
	ROM_LOAD( "240-v4.bin", 0xc00000, 0x200000, CRC(996704d8) SHA1(0bf7a1d0660199dedf3c25be757eeab75cc6147e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "240-c1.bin", 0x0000000, 0x800000, CRC(effac504) SHA1(e36a96e7369b02c7e839b5abf3c6799453ba1927) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c2.bin", 0x0000001, 0x800000, CRC(ed182d44) SHA1(a9fc0a3a786bf067c129ec7220df65953dff804f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "240-c3.bin", 0x1000000, 0x800000, CRC(22e0330a) SHA1(0fe7f6a8aeba7f17dbb278e85003969ff10d3cd2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c4.bin", 0x1000001, 0x800000, CRC(c19a07eb) SHA1(139eac8b51cadf328dd42d8109f4e2463f57230c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "240-c5.bin", 0x2000000, 0x800000, CRC(244dff5a) SHA1(156548156d3ceaa808d0053d0749af2526a3943e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c6.bin", 0x2000001, 0x800000, CRC(4609e507) SHA1(bb17f50a377dddb77c1eeda5944a7bcbf0cca5f7) ) /* Plane 2,3 */
ROM_END

ROM_START( rbff2k ) /* KOREAN VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "140-p1k.bin", 0x000000, 0x100000, CRC(965edee1) SHA1(7f4b947b19ccfee32fc73e4fd89645eb313b5c77) )
	ROM_LOAD16_WORD_SWAP( "240-p2.bin",  0x100000, 0x400000, CRC(960aa88d) SHA1(3d9e785891871af90313f178dca2724633406674) )

	NEO_SFIX_128K( "240-s1.bin", CRC(da3b40de) SHA1(e6bf74e057ac6fe1f249a7547f13ba7fbc694561) )

	/* TC532000DP */
	NEO_BIOS_AUDIO_256K( "240-m1.bin", CRC(ed482791) SHA1(1f54a45967cb7842c33aa24be322c9f33ff75ac3) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "240-v1.bin", 0x000000, 0x400000, CRC(f796265a) SHA1(736dff37eb91fc856b4d189249fb0de9b6c0813a) )
	ROM_LOAD( "240-v2.bin", 0x400000, 0x400000, CRC(2cb3f3bb) SHA1(697e677890892f4b028c9a27c66809ca0a8a9b18) )
	//ROM_LOAD( "240-v3.bin", 0x800000, 0x400000, CRC(df77b7fa) SHA1(4df971ce20bdb8c1ce8cc1692a32ac69505ffa9a) ) // bad?
	ROM_LOAD( "240-v3.bin", 0x800000, 0x400000, CRC(8fe1367a) SHA1(093d7a4ac2b54ad7ffb2dc316fe29415f7a99535) )
	ROM_LOAD( "240-v4.bin", 0xc00000, 0x200000, CRC(996704d8) SHA1(0bf7a1d0660199dedf3c25be757eeab75cc6147e) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "240-c1.bin", 0x0000000, 0x800000, CRC(effac504) SHA1(e36a96e7369b02c7e839b5abf3c6799453ba1927) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c2.bin", 0x0000001, 0x800000, CRC(ed182d44) SHA1(a9fc0a3a786bf067c129ec7220df65953dff804f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "240-c3.bin", 0x1000000, 0x800000, CRC(22e0330a) SHA1(0fe7f6a8aeba7f17dbb278e85003969ff10d3cd2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c4.bin", 0x1000001, 0x800000, CRC(c19a07eb) SHA1(139eac8b51cadf328dd42d8109f4e2463f57230c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "240-c5.bin", 0x2000000, 0x800000, CRC(244dff5a) SHA1(156548156d3ceaa808d0053d0749af2526a3943e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "240-c6.bin", 0x2000001, 0x800000, CRC(4609e507) SHA1(bb17f50a377dddb77c1eeda5944a7bcbf0cca5f7) ) /* Plane 2,3 */
ROM_END

ROM_START( mslug2 )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "241-p1.bin", 0x000000, 0x100000, CRC(2a53c5da) SHA1(5a6aba482cac588a6c2c51179c95b487c6e11899) )
	ROM_LOAD16_WORD_SWAP( "241-p2.bin", 0x100000, 0x200000, CRC(38883f44) SHA1(fcf34b8c6e37774741542393b963635412484a27) )

	NEO_SFIX_128K( "241-s1.bin", CRC(f3d32f0f) SHA1(2dc38b7dfd3ff14f64d5c0733c510b6bb8c692d0) )

	NEO_BIOS_AUDIO_128K( "241-m1.bin", CRC(94520ebd) SHA1(f8a1551cebcb91e416f30f50581feed7f72899e9) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "241-v1.bin", 0x000000, 0x400000, CRC(99ec20e8) SHA1(80597707f1fe115eed1941bb0701fc00790ad504) )
	ROM_LOAD( "241-v2.bin", 0x400000, 0x400000, CRC(ecb16799) SHA1(b4b4ddc680836ed55942c66d7dfe756314e02211) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "241-c1.bin", 0x0000000, 0x800000, CRC(394b5e0d) SHA1(4549926f5054ee6aa7689cf920be0327e3908a50) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "241-c2.bin", 0x0000001, 0x800000, CRC(e5806221) SHA1(1e5475cfab129c77acc610f09369ca42ba5aafa5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "241-c3.bin", 0x1000000, 0x800000, CRC(9f6bfa6f) SHA1(a4319b48004e723f81a980887678e3e296049a53) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "241-c4.bin", 0x1000001, 0x800000, CRC(7d3e306f) SHA1(1499316fb381775218d897b81a6a0c3465d1a37c) ) /* Plane 2,3 */
ROM_END

ROM_START( kof98 ) /* encrypted code + protection */
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "242-p1.bin", 0x000000, 0x200000, CRC(8893df89) SHA1(0452828785110601c65f667209fc2d2926cd3751) )
	ROM_LOAD16_WORD_SWAP( "242-p2.bin", 0x200000, 0x400000, CRC(980aba4c) SHA1(5e735929ec6c3ca5b2efae3c7de47bcbb8ade2c5) )

	NEO_SFIX_128K( "242-s1.bin", CRC(7f7b4805) SHA1(80ee6e5d0ece9c34ebca54b043a7cb33f9ff6b92) )

	/* TC532000DP */
	NEO_BIOS_AUDIO_256K( "242-m1.bin", CRC(4ef7016b) SHA1(4182235e963bd70d398a79abeb54ab4d62887c48) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "242-v1.bin", 0x000000, 0x400000, CRC(b9ea8051) SHA1(49606f64eb249263b3341b4f50cc1763c390b2af) )
	ROM_LOAD( "242-v2.bin", 0x400000, 0x400000, CRC(cc11106e) SHA1(d3108bc05c9bf041d4236b2fa0c66b013aa8db1b) )
	ROM_LOAD( "242-v3.bin", 0x800000, 0x400000, CRC(044ea4e1) SHA1(062a2f2e52098d73bc31c9ad66f5db8080395ce8) )
	ROM_LOAD( "242-v4.bin", 0xc00000, 0x400000, CRC(7985ea30) SHA1(54ed5f0324de6164ea81943ebccb3e8d298368ec) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "242-c1.bin", 0x0000000, 0x800000, CRC(e564ecd6) SHA1(78f22787a204f26bae9b2b1c945ddbc27143352f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c2.bin", 0x0000001, 0x800000, CRC(bd959b60) SHA1(2c97c59e77c9a3fe7d664e741d37944f3d56c10b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c3.bin", 0x1000000, 0x800000, CRC(22127b4f) SHA1(bd0d00f889d9da7c6ac48f287d9ed8c605ae22cf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c4.bin", 0x1000001, 0x800000, CRC(0b4fa044) SHA1(fa13c3764fae6b035a626601bc43629f1ebaaffd) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c5.bin", 0x2000000, 0x800000, CRC(9d10bed3) SHA1(4d44addc7c808649bfb03ec45fb9529da413adff) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c6.bin", 0x2000001, 0x800000, CRC(da07b6a2) SHA1(9c3f0da7cde1ffa8feca89efc88f07096e502acf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c7.bin", 0x3000000, 0x800000, CRC(f6d7a38a) SHA1(dd295d974dd4a7e5cb26a3ef3febcd03f28d522b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c8.bin", 0x3000001, 0x800000, CRC(c823e045) SHA1(886fbf64bcb58bc4eabb1fc9262f6ac9901a0f28) ) /* Plane 2,3 */
ROM_END

ROM_START( kof98k ) /* encrypted code + protection, only z80 rom is different to kof98 */ /* KOREAN VERSION */
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "242-p1.bin", 0x000000, 0x200000, CRC(8893df89) SHA1(0452828785110601c65f667209fc2d2926cd3751) )
	ROM_LOAD16_WORD_SWAP( "242-p2.bin", 0x200000, 0x400000, CRC(980aba4c) SHA1(5e735929ec6c3ca5b2efae3c7de47bcbb8ade2c5) )

	NEO_SFIX_128K( "242-s1.bin", CRC(7f7b4805) SHA1(80ee6e5d0ece9c34ebca54b043a7cb33f9ff6b92) )

	/* Correct chip label */
	NEO_BIOS_AUDIO_256K( "242-m1k.bin", CRC(ce12da0c) SHA1(e7c01dae2852d543d1a58d55735239f6a5aa05a5) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "242-v1.bin", 0x000000, 0x400000, CRC(b9ea8051) SHA1(49606f64eb249263b3341b4f50cc1763c390b2af) )
	ROM_LOAD( "242-v2.bin", 0x400000, 0x400000, CRC(cc11106e) SHA1(d3108bc05c9bf041d4236b2fa0c66b013aa8db1b) )
	ROM_LOAD( "242-v3.bin", 0x800000, 0x400000, CRC(044ea4e1) SHA1(062a2f2e52098d73bc31c9ad66f5db8080395ce8) )
	ROM_LOAD( "242-v4.bin", 0xc00000, 0x400000, CRC(7985ea30) SHA1(54ed5f0324de6164ea81943ebccb3e8d298368ec) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "242-c1.bin", 0x0000000, 0x800000, CRC(e564ecd6) SHA1(78f22787a204f26bae9b2b1c945ddbc27143352f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c2.bin", 0x0000001, 0x800000, CRC(bd959b60) SHA1(2c97c59e77c9a3fe7d664e741d37944f3d56c10b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c3.bin", 0x1000000, 0x800000, CRC(22127b4f) SHA1(bd0d00f889d9da7c6ac48f287d9ed8c605ae22cf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c4.bin", 0x1000001, 0x800000, CRC(0b4fa044) SHA1(fa13c3764fae6b035a626601bc43629f1ebaaffd) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c5.bin", 0x2000000, 0x800000, CRC(9d10bed3) SHA1(4d44addc7c808649bfb03ec45fb9529da413adff) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c6.bin", 0x2000001, 0x800000, CRC(da07b6a2) SHA1(9c3f0da7cde1ffa8feca89efc88f07096e502acf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c7.bin", 0x3000000, 0x800000, CRC(f6d7a38a) SHA1(dd295d974dd4a7e5cb26a3ef3febcd03f28d522b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c8.bin", 0x3000001, 0x800000, CRC(c823e045) SHA1(886fbf64bcb58bc4eabb1fc9262f6ac9901a0f28) ) /* Plane 2,3 */
ROM_END

ROM_START( kof98ka ) /* encrypted code + protection, only z80 rom is different to kof98 */ /* KOREAN VERSION */
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "242-p1.bin", 0x000000, 0x200000, CRC(8893df89) SHA1(0452828785110601c65f667209fc2d2926cd3751) )
	ROM_LOAD16_WORD_SWAP( "242-p2.bin", 0x200000, 0x400000, CRC(980aba4c) SHA1(5e735929ec6c3ca5b2efae3c7de47bcbb8ade2c5) )

	NEO_SFIX_128K( "242-s1.bin", CRC(7f7b4805) SHA1(80ee6e5d0ece9c34ebca54b043a7cb33f9ff6b92) )

	/* Correct chip label */
	NEO_BIOS_AUDIO_256K( "242-mg1k.bin", CRC(ce9fb07c) SHA1(631d995f1291dd803fb069f3b25e7b9ed30d8649) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "242-v1.bin", 0x000000, 0x400000, CRC(b9ea8051) SHA1(49606f64eb249263b3341b4f50cc1763c390b2af) )
	ROM_LOAD( "242-v2.bin", 0x400000, 0x400000, CRC(cc11106e) SHA1(d3108bc05c9bf041d4236b2fa0c66b013aa8db1b) )
	ROM_LOAD( "242-v3.bin", 0x800000, 0x400000, CRC(044ea4e1) SHA1(062a2f2e52098d73bc31c9ad66f5db8080395ce8) )
	ROM_LOAD( "242-v4.bin", 0xc00000, 0x400000, CRC(7985ea30) SHA1(54ed5f0324de6164ea81943ebccb3e8d298368ec) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "242-c1.bin", 0x0000000, 0x800000, CRC(e564ecd6) SHA1(78f22787a204f26bae9b2b1c945ddbc27143352f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c2.bin", 0x0000001, 0x800000, CRC(bd959b60) SHA1(2c97c59e77c9a3fe7d664e741d37944f3d56c10b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c3.bin", 0x1000000, 0x800000, CRC(22127b4f) SHA1(bd0d00f889d9da7c6ac48f287d9ed8c605ae22cf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c4.bin", 0x1000001, 0x800000, CRC(0b4fa044) SHA1(fa13c3764fae6b035a626601bc43629f1ebaaffd) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c5.bin", 0x2000000, 0x800000, CRC(9d10bed3) SHA1(4d44addc7c808649bfb03ec45fb9529da413adff) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c6.bin", 0x2000001, 0x800000, CRC(da07b6a2) SHA1(9c3f0da7cde1ffa8feca89efc88f07096e502acf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c7.bin", 0x3000000, 0x800000, CRC(f6d7a38a) SHA1(dd295d974dd4a7e5cb26a3ef3febcd03f28d522b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c8.bin", 0x3000001, 0x800000, CRC(c823e045) SHA1(886fbf64bcb58bc4eabb1fc9262f6ac9901a0f28) ) /* Plane 2,3 */
ROM_END

ROM_START( kof98h ) /* AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "242-pn1.bin", 0x000000, 0x100000, CRC(61ac868a) SHA1(26577264aa72d6af272952a876fcd3775f53e3fa) )
	ROM_LOAD16_WORD_SWAP( "242-p2.bin",  0x100000, 0x400000, CRC(980aba4c) SHA1(5e735929ec6c3ca5b2efae3c7de47bcbb8ade2c5) )

	NEO_SFIX_128K( "242-s1.bin", CRC(7f7b4805) SHA1(80ee6e5d0ece9c34ebca54b043a7cb33f9ff6b92) )

	/* TC532000DP */
	NEO_BIOS_AUDIO_256K( "242-mg1.bin", CRC(4e7a6b1b) SHA1(b54d08f88713ed0271aa06f9f7c9c572ef555b1a) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "242-v1.bin", 0x000000, 0x400000, CRC(b9ea8051) SHA1(49606f64eb249263b3341b4f50cc1763c390b2af) )
	ROM_LOAD( "242-v2.bin", 0x400000, 0x400000, CRC(cc11106e) SHA1(d3108bc05c9bf041d4236b2fa0c66b013aa8db1b) )
	ROM_LOAD( "242-v3.bin", 0x800000, 0x400000, CRC(044ea4e1) SHA1(062a2f2e52098d73bc31c9ad66f5db8080395ce8) )
	ROM_LOAD( "242-v4.bin", 0xc00000, 0x400000, CRC(7985ea30) SHA1(54ed5f0324de6164ea81943ebccb3e8d298368ec) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "242-c1.bin", 0x0000000, 0x800000, CRC(e564ecd6) SHA1(78f22787a204f26bae9b2b1c945ddbc27143352f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c2.bin", 0x0000001, 0x800000, CRC(bd959b60) SHA1(2c97c59e77c9a3fe7d664e741d37944f3d56c10b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c3.bin", 0x1000000, 0x800000, CRC(22127b4f) SHA1(bd0d00f889d9da7c6ac48f287d9ed8c605ae22cf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c4.bin", 0x1000001, 0x800000, CRC(0b4fa044) SHA1(fa13c3764fae6b035a626601bc43629f1ebaaffd) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c5.bin", 0x2000000, 0x800000, CRC(9d10bed3) SHA1(4d44addc7c808649bfb03ec45fb9529da413adff) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c6.bin", 0x2000001, 0x800000, CRC(da07b6a2) SHA1(9c3f0da7cde1ffa8feca89efc88f07096e502acf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "242-c7.bin", 0x3000000, 0x800000, CRC(f6d7a38a) SHA1(dd295d974dd4a7e5cb26a3ef3febcd03f28d522b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "242-c8.bin", 0x3000001, 0x800000, CRC(c823e045) SHA1(886fbf64bcb58bc4eabb1fc9262f6ac9901a0f28) ) /* Plane 2,3 */
ROM_END

ROM_START( lastbld2 ) /* MVS AND AES VERSION */ /* later revision */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "243-pg1.bin", 0x000000, 0x100000, CRC(af1e6554) SHA1(bd8526f60c2472937728a5d933fbd19d899f2cba) )
	ROM_LOAD16_WORD_SWAP( "243-pg2.bin", 0x100000, 0x400000, CRC(add4a30b) SHA1(7db62564db49fe0218cbb35b119d62582a24d658) )

	NEO_SFIX_128K( "243-s1.bin", CRC(c9cd2298) SHA1(a9a18b5347f9dbe29a2ccb63fd4c8fd19537bf8b) )

	NEO_BIOS_AUDIO_128K( "243-m1.bin", CRC(acf12d10) SHA1(6e6b98cc1fa44f24a5168877559b0055e6957b60) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "243-v1.bin", 0x000000, 0x400000, CRC(f7ee6fbb) SHA1(55137bcabeeb590e40a9b8a7c07dd106e4d12a90) )
	ROM_LOAD( "243-v2.bin", 0x400000, 0x400000, CRC(aa9e4df6) SHA1(a0b91f63e2552a8ad9e0d1af00e2c38288637161) )
	ROM_LOAD( "243-v3.bin", 0x800000, 0x400000, CRC(4ac750b2) SHA1(585a154acc67bd84ea5b944686b78ed082b768d9) )
	ROM_LOAD( "243-v4.bin", 0xc00000, 0x400000, CRC(f5c64ba6) SHA1(2eac455def8c27090862cc042f65a3a8aad88283) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "243-c1.bin", 0x0000000, 0x800000, CRC(5839444d) SHA1(0616921c4cce20422563578bd0e806d359508599) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "243-c2.bin", 0x0000001, 0x800000, CRC(dd087428) SHA1(ca27fdb60425664956a18c021ea465f452fb1527) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "243-c3.bin", 0x1000000, 0x800000, CRC(6054cbe0) SHA1(ec2f65e9c930250ee25fd064ee5ae76a7a9c61d9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "243-c4.bin", 0x1000001, 0x800000, CRC(8bd2a9d2) SHA1(0935df65cd2b0891a708bcc0f1c188148058d4b5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "243-c5.bin", 0x2000000, 0x800000, CRC(6a503dcf) SHA1(23241b16d7e20f923d41186b29487ab922c7f530) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "243-c6.bin", 0x2000001, 0x800000, CRC(ec9c36d0) SHA1(e145e9e359000dda6e1dfe95a996bc6d29cfca21) ) /* Plane 2,3 */
ROM_END

ROM_START( neocup98 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "244-p1.bin", 0x100000, 0x100000, CRC(f8fdb7a5) SHA1(f34ee5d1c24e70427d05ef488f46906dbd9f9950) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "244-s1.bin", CRC(9bddb697) SHA1(2f479bcd5a433201168792a578de3057252d649f) )

	NEO_BIOS_AUDIO_128K( "244-m1.bin", CRC(a701b276) SHA1(055550ebc650835bcf8ea4457b2c91bd73e21281) )

	ROM_REGION( 0x600000, "ymsnd", 0 )
	ROM_LOAD( "244-v1.bin", 0x000000, 0x400000, CRC(79def46d) SHA1(63414235de2e177654508f1c840040424f8993e6) )
	ROM_LOAD( "244-v2.bin", 0x400000, 0x200000, CRC(b231902f) SHA1(9209772e947a2c7ac31b49dd613bf2eab0cb3358) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "244-c1.bin", 0x000000, 0x800000, CRC(c7a62b23) SHA1(4534ecc9ade69c543188c66229dcad89dbc48668) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "244-c2.bin", 0x000001, 0x800000, CRC(33aa0f35) SHA1(3443c7765c6aa177003d42bbfcac9f31d1e12575) ) /* Plane 2,3 */
ROM_END

ROM_START( breakrev )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "245-p1.bin", 0x100000, 0x100000, CRC(c828876d) SHA1(1dcba850e5cf8219d0945612cfded6d20ca8682a) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "245-s1.bin", CRC(e7660a5d) SHA1(1cd54964ba60b245ea57d9daf0e27b572b815d21) )

	NEO_BIOS_AUDIO_128K( "245-m1.bin", CRC(00f31c66) SHA1(8488598415c9b74bce00e05b31d96e3d1625c20d) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "245-v1.bin", 0x000000, 0x400000, CRC(e255446c) SHA1(b3933340d49d4ba581f3bf1af7ad69d786205790) )
	ROM_LOAD( "245-v2.bin", 0x400000, 0x400000, CRC(9068198a) SHA1(71819b0475a5e173a2f9a6e4ff19a94655141c3c) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1800000, "sprites", 0 )
	/* The chip labels and sizes are correct */
	ROM_LOAD16_BYTE( "245-c1.bin", 0x0000000, 0x400000, CRC(68d4ae76) SHA1(2e820067f6963669f104bebf19e865fe4127b4dd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "245-c2.bin", 0x0000001, 0x400000, CRC(fdee05cd) SHA1(efc4ffd790953ac7c25d5f045c64a9b49d24b096) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "245-c3.bin", 0x0800000, 0x400000, CRC(645077f3) SHA1(0ae74f3b4b3b88f128c6d8c0f35ffa53f5d67ef2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "245-c4.bin", 0x0800001, 0x400000, CRC(63aeb74c) SHA1(9ff6930c0c3d79b46b86356e8565ce4fcd69ac38) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "245-c5.bin", 0x1000000, 0x400000, CRC(b5f40e7f) SHA1(b332bac64dbb9a9dd66c5315f47ea08634d36f45) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "245-c6.bin", 0x1000001, 0x400000, CRC(d0337328) SHA1(dff86b75dc283bd4512557a5c64f16e6be6c16e4) ) /* Plane 2,3 */
ROM_END

ROM_START( shocktr2 )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "246-p1.bin", 0x000000, 0x100000, CRC(6d4b7781) SHA1(3c9d53d5da9842bfd45037c919064dda3fb2e089) )
	ROM_LOAD16_WORD_SWAP( "246-p2.bin", 0x100000, 0x400000, CRC(72ea04c3) SHA1(4fb1d22c30f5f3db4637dd92a4d2705c88de399d) )

	NEO_SFIX_128K( "246-s1.bin", CRC(2a360637) SHA1(431b43da5377dd189e51bd93d88d8a24d1b5090a) )

	NEO_BIOS_AUDIO_128K( "246-m1.bin", CRC(d0604ad1) SHA1(fae3cd52a177eadd5f5775ace957cc0f8301e65d) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "246-v1.bin", 0x000000, 0x400000, CRC(16986fc6) SHA1(cff3103dadf2f4390460456a5bd3fb5f28e21f6a) )
	ROM_LOAD( "246-v2.bin", 0x400000, 0x400000, CRC(ada41e83) SHA1(78e37ffaaa5679c8775a3a71f6df7a0d15082bdc) )
	ROM_LOAD( "246-v3.bin", 0x800000, 0x200000, CRC(a05ba5db) SHA1(09d739cad323d918f4196f91b654627fcafd8f4d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "246-c1.bin", 0x0000000, 0x800000, CRC(47ac9ec5) SHA1(2d9eea11ba87baa23b18a1a3f607dc137846e807) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "246-c2.bin", 0x0000001, 0x800000, CRC(7bcab64f) SHA1(08d0edddd14b53d606e9a7a46aa4fb4e7398e0d0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "246-c3.bin", 0x1000000, 0x800000, CRC(db2f73e8) SHA1(8d0c3473a8b2a4e28fed1b74beb2e025b7e61867) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "246-c4.bin", 0x1000001, 0x800000, CRC(5503854e) SHA1(a0f2e7c609cbb2aa43493a39d7dcaeca3d511d26) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "246-c5.bin", 0x2000000, 0x800000, CRC(055b3701) SHA1(97f5e92538d1f2e437dcb3f80e56e1230287e8d1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "246-c6.bin", 0x2000001, 0x800000, CRC(7e2caae1) SHA1(d9de14e3e323664a8c5b7f1df1ba9ec7dd0e6a46) ) /* Plane 2,3 */
ROM_END

ROM_START( flipshot )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "247-p1.bin", 0x000000, 0x080000, BAD_DUMP CRC(d2e7a7e3) SHA1(1ff4f070fcc658bbc7dc69e16c87f82d7392d100) )

	NEO_SFIX_128K( "247-s1.bin", CRC(6300185c) SHA1(cb2f1de085fde214f96a962b1c2fa285eb387d44) )

	NEO_BIOS_AUDIO_128K( "247-m1.bin", CRC(a9fe0144) SHA1(4cc076ecce9216a373f3dcd7ba28a03d6050e522) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "247-v1.bin", 0x000000, 0x200000, CRC(42ec743d) SHA1(f45b5167ebcbd59300f4e5b05448cd421654102a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "247-c1.bin", 0x000000, 0x200000, CRC(c9eedcb2) SHA1(7627f2810322c146511525eb70b573a6a5ede926) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "247-c2.bin", 0x000001, 0x200000, CRC(7d6d6e87) SHA1(6475b58b9f91c20d1f465f3e892de0c68e12a92b) ) /* Plane 2,3 */
ROM_END

ROM_START( pbobbl2n )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "248-p1.bin", 0x000000, 0x100000, CRC(9d6c0754) SHA1(95c70c2d51fc4de01e768e03cc800a850aaad5dc) )

	NEO_SFIX_128K( "248-s1.bin", CRC(0a3fee41) SHA1(0ab2120e462086be942efcf6ffb37f58ea966ca3) )

	NEO_BIOS_AUDIO_128K( "248-m1.bin", CRC(883097a9) SHA1(677bf9684c0c7977a9a3f0c1288e430040a53b49) )

	ROM_REGION( 0x800000, "ymsnd", 0 )
	ROM_LOAD( "248-v1.bin", 0x000000, 0x400000, CRC(57fde1fa) SHA1(af39bc141fc35b78dcacfd42b3abb29d7e5c2c89) )
	ROM_LOAD( "248-v2.bin", 0x400000, 0x400000, CRC(4b966ef3) SHA1(083c0e9fd7b8e506087648cdd8ec4206103984cd) )

	NO_DELTAT_REGION

	ROM_REGION( 0xa00000, "sprites", 0 )
	ROM_LOAD16_BYTE( "248-c1.bin", 0x000000, 0x400000, CRC(d9115327) SHA1(a49aa836a902326cfe785428e1699fefcf8566d4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "248-c2.bin", 0x000001, 0x400000, CRC(77f9fdac) SHA1(4642d71d32b6a05dc8bfa0f95c936a77c7cef05e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "248-c3.bin", 0x800000, 0x100000, CRC(8890bf7c) SHA1(a52f6bafd60e72003bfe38c80c1dde24b4983b2a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "248-c4.bin", 0x800001, 0x100000, CRC(8efead3f) SHA1(f577d2f7c6f850b3d100c36947ad15e33dfa0bed) ) /* Plane 2,3 */
ROM_END

ROM_START( ctomaday )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "249-p1.bin", 0x100000, 0x100000, CRC(c9386118) SHA1(5554662c7bc8605889cac4a67fee05bbb4eb786f) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "249-s1.bin", CRC(dc9eb372) SHA1(b8aa142243ba303799554479bfc88eb49260f3b1) )

	NEO_BIOS_AUDIO_128K( "249-m1.bin", CRC(80328a47) SHA1(34b6b1a81eab1cf38834b2eea55454ce1b6100e2) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "249-v1.bin", 0x000000, 0x400000, CRC(de7c8f27) SHA1(3681a68a702ab5da8f509b8301d6cada75959332) )
	ROM_LOAD( "249-v2.bin", 0x400000, 0x100000, CRC(c8e40119) SHA1(738f525c381ed68c0b8a89318a3e4d0089473c45) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "249-c1.bin", 0x000000, 0x400000, CRC(041fb8ee) SHA1(dacc84d713d76818d89a26358374afaa22fa82a2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "249-c2.bin", 0x000001, 0x400000, CRC(74f3cdf4) SHA1(55ddabaf77f4d575f4deb24fe63e4bdc2c6f31e1) ) /* Plane 2,3 */
ROM_END

ROM_START( mslugx )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "250-p1.bin", 0x000000, 0x100000, CRC(81f1f60b) SHA1(4c19f2e9824e606178ac1c9d4b0516fbaa625035) )
	ROM_LOAD16_WORD_SWAP( "250-p2.bin", 0x100000, 0x400000, CRC(1fda2e12) SHA1(18aaa7a3ba8da99f78c430e9be69ccde04bc04d9) )

	NEO_SFIX_128K( "250-s1.bin", CRC(fb6f441d) SHA1(2cc392ecde5d5afb28ddbaa1030552b48571dcfb) )

	NEO_BIOS_AUDIO_128K( "250-m1.bin", CRC(fd42a842) SHA1(55769bad4860f64ef53a333e0da9e073db483d6a) )

	ROM_REGION( 0xa00000, "ymsnd", 0 )
	ROM_LOAD( "250-v1.bin", 0x000000, 0x400000, CRC(c79ede73) SHA1(ebfcc67204ff9677cf7972fd5b6b7faabf07280c) )
	ROM_LOAD( "250-v2.bin", 0x400000, 0x400000, CRC(ea9aabe1) SHA1(526c42ca9a388f7435569400e2f132e2724c71ff) )
	ROM_LOAD( "250-v3.bin", 0x800000, 0x200000, CRC(2ca65102) SHA1(45979d1edb1fc774a415d9386f98d7cb252a2043) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "250-c1.bin", 0x0000000, 0x800000, CRC(09a52c6f) SHA1(c3e8a8ccdac0f8bddc4c3413277626532405fae2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "250-c2.bin", 0x0000001, 0x800000, CRC(31679821) SHA1(554f600a3aa09c16c13c625299b087a79d0d15c5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "250-c3.bin", 0x1000000, 0x800000, CRC(fd602019) SHA1(c56646c62387bc1439d46610258c755beb8d7dd8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "250-c4.bin", 0x1000001, 0x800000, CRC(31354513) SHA1(31be8ea2498001f68ce4b06b8b90acbf2dcab6af) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "250-c5.bin", 0x2000000, 0x800000, CRC(a4b56124) SHA1(d41069856df990a1a99d39fb263c8303389d5475) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "250-c6.bin", 0x2000001, 0x800000, CRC(83e3e69d) SHA1(39be66287696829d243fb71b3fb8b7dc2bc3298f) ) /* Plane 0,1 */
ROM_END

ROM_START( kof99 ) /* Original Version - Encrypted Code & GFX */ /* AES VERSION */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "251-sma.kc",  0x0c0000, 0x040000, CRC(6c9d0647) SHA1(2a0ce62ca6c18007e8fbe1b60475c7874ab79389) )	/* stored in the custom chip */
	ROM_LOAD16_WORD_SWAP( "251-p1.bin",  0x100000, 0x400000, CRC(006e4532) SHA1(47791ab4044ad55988b1d3412d95b65b91a163c8) )
	ROM_LOAD16_WORD_SWAP( "251-pg2.bin", 0x500000, 0x400000, CRC(d9057f51) SHA1(8d365b4dd40351495df99d6c765df1434b0b0548) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "251-m1.bin", CRC(5e74539c) SHA1(6f49a9343cbd026b2c6720ff3fa2e5b1f85e80da) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "251-v1.bin", 0x000000, 0x400000, CRC(ef2eecc8) SHA1(8ed13b9db92dba3124bc5ba66e3e275885ece24a) )
	ROM_LOAD( "251-v2.bin", 0x400000, 0x400000, CRC(73e211ca) SHA1(0e60fa64cab6255d9721e2b4bc22e3de64c874c5) )
	ROM_LOAD( "251-v3.bin", 0x800000, 0x400000, CRC(821901da) SHA1(c6d4975bfaa19a62ed59126cadf2578c0a5c257f) )
	ROM_LOAD( "251-v4.bin", 0xc00000, 0x200000, CRC(b49e6178) SHA1(dde6f76e958841e8c99b693e13ced9aa9ef316dc) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "251-c1.bin", 0x0000000, 0x800000, CRC(0f9e93fe) SHA1(c7d77f0833c6f526f632e4f2dce59e302f6b9a15) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c2.bin", 0x0000001, 0x800000, CRC(e71e2ea3) SHA1(39c7a326fddbcca3b29c68cdc96aad4d62295c0f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c3.bin", 0x1000000, 0x800000, CRC(238755d2) SHA1(01125b5c7a28e350f091280b041954fd1ac7c98f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c4.bin", 0x1000001, 0x800000, CRC(438c8b22) SHA1(ffbc643f5b27dd00f2f95d4ef4c5f29ee814722b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c5.bin", 0x2000000, 0x800000, CRC(0b0abd0a) SHA1(d5ad324fe523bdc6f09209d236cc4932524a48f1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c6.bin", 0x2000001, 0x800000, CRC(65bbf281) SHA1(79ae174667a23dabcfe865b6cd6133c86098452e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c7.bin", 0x3000000, 0x800000, CRC(ff65f62e) SHA1(7cd335fede05b56e15db90ce407c1183a78da4e9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c8.bin", 0x3000001, 0x800000, CRC(8d921c68) SHA1(42acf1d27d52a8e3b6262eb7df50693c0b135565) ) /* Plane 2,3 */
ROM_END

ROM_START( kof99a ) /* Original Version - Encrypted Code & GFX */ /* MVS VERSION */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "251-sma.ka", 0x0c0000, 0x040000, CRC(7766d09e) SHA1(4e0a49d1ad669a62676cb30f527c6590cde80194) )	/* stored in the custom chip */
	ROM_LOAD16_WORD_SWAP( "251-p1.bin", 0x100000, 0x400000, CRC(006e4532) SHA1(47791ab4044ad55988b1d3412d95b65b91a163c8) )
	ROM_LOAD16_WORD_SWAP( "251-p2.bin", 0x500000, 0x400000, CRC(90175f15) SHA1(aa9e75810438a8b45808a8bf32cb04d91b5c0b3a) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "251-m1.bin", CRC(5e74539c) SHA1(6f49a9343cbd026b2c6720ff3fa2e5b1f85e80da) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "251-v1.bin", 0x000000, 0x400000, CRC(ef2eecc8) SHA1(8ed13b9db92dba3124bc5ba66e3e275885ece24a) )
	ROM_LOAD( "251-v2.bin", 0x400000, 0x400000, CRC(73e211ca) SHA1(0e60fa64cab6255d9721e2b4bc22e3de64c874c5) )
	ROM_LOAD( "251-v3.bin", 0x800000, 0x400000, CRC(821901da) SHA1(c6d4975bfaa19a62ed59126cadf2578c0a5c257f) )
	ROM_LOAD( "251-v4.bin", 0xc00000, 0x200000, CRC(b49e6178) SHA1(dde6f76e958841e8c99b693e13ced9aa9ef316dc) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "251-c1.bin", 0x0000000, 0x800000, CRC(0f9e93fe) SHA1(c7d77f0833c6f526f632e4f2dce59e302f6b9a15) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c2.bin", 0x0000001, 0x800000, CRC(e71e2ea3) SHA1(39c7a326fddbcca3b29c68cdc96aad4d62295c0f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c3.bin", 0x1000000, 0x800000, CRC(238755d2) SHA1(01125b5c7a28e350f091280b041954fd1ac7c98f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c4.bin", 0x1000001, 0x800000, CRC(438c8b22) SHA1(ffbc643f5b27dd00f2f95d4ef4c5f29ee814722b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c5.bin", 0x2000000, 0x800000, CRC(0b0abd0a) SHA1(d5ad324fe523bdc6f09209d236cc4932524a48f1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c6.bin", 0x2000001, 0x800000, CRC(65bbf281) SHA1(79ae174667a23dabcfe865b6cd6133c86098452e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c7.bin", 0x3000000, 0x800000, CRC(ff65f62e) SHA1(7cd335fede05b56e15db90ce407c1183a78da4e9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c8.bin", 0x3000001, 0x800000, CRC(8d921c68) SHA1(42acf1d27d52a8e3b6262eb7df50693c0b135565) ) /* Plane 2,3 */
ROM_END

ROM_START( kof99e ) /* Original Version - Encrypted Code & GFX */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "251-sma.ka", 0x0c0000, 0x040000, CRC(7766d09e) SHA1(4e0a49d1ad669a62676cb30f527c6590cde80194) )	/* stored in the custom chip */
	ROM_LOAD16_WORD_SWAP( "251-ep1.p1", 0x100000, 0x200000, CRC(1e8d692d) SHA1(eea1aa8c0a17f089ac14831889c36535e559072c) )
	ROM_LOAD16_WORD_SWAP( "251-ep2.p2", 0x300000, 0x200000, CRC(d6206e5a) SHA1(0e1100d03c40c6d5cfa899d009e319ae73fce6b8) )
	ROM_LOAD16_WORD_SWAP( "251-ep3.p3", 0x500000, 0x200000, CRC(d58c3ef8) SHA1(f927d90d55b49944f448d6286e0cb913cc70ade1) )
	ROM_LOAD16_WORD_SWAP( "251-ep4.p4", 0x700000, 0x200000, CRC(52de02ae) SHA1(f16924ff8eef92da7716236a6a055e22e090a02b) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "251-m1.bin", CRC(5e74539c) SHA1(6f49a9343cbd026b2c6720ff3fa2e5b1f85e80da) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "251-v1.bin", 0x000000, 0x400000, CRC(ef2eecc8) SHA1(8ed13b9db92dba3124bc5ba66e3e275885ece24a) )
	ROM_LOAD( "251-v2.bin", 0x400000, 0x400000, CRC(73e211ca) SHA1(0e60fa64cab6255d9721e2b4bc22e3de64c874c5) )
	ROM_LOAD( "251-v3.bin", 0x800000, 0x400000, CRC(821901da) SHA1(c6d4975bfaa19a62ed59126cadf2578c0a5c257f) )
	ROM_LOAD( "251-v4.bin", 0xc00000, 0x200000, CRC(b49e6178) SHA1(dde6f76e958841e8c99b693e13ced9aa9ef316dc) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "251-c1.bin", 0x0000000, 0x800000, CRC(0f9e93fe) SHA1(c7d77f0833c6f526f632e4f2dce59e302f6b9a15) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c2.bin", 0x0000001, 0x800000, CRC(e71e2ea3) SHA1(39c7a326fddbcca3b29c68cdc96aad4d62295c0f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c3.bin", 0x1000000, 0x800000, CRC(238755d2) SHA1(01125b5c7a28e350f091280b041954fd1ac7c98f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c4.bin", 0x1000001, 0x800000, CRC(438c8b22) SHA1(ffbc643f5b27dd00f2f95d4ef4c5f29ee814722b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c5.bin", 0x2000000, 0x800000, CRC(0b0abd0a) SHA1(d5ad324fe523bdc6f09209d236cc4932524a48f1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c6.bin", 0x2000001, 0x800000, CRC(65bbf281) SHA1(79ae174667a23dabcfe865b6cd6133c86098452e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c7.bin", 0x3000000, 0x800000, CRC(ff65f62e) SHA1(7cd335fede05b56e15db90ce407c1183a78da4e9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c8.bin", 0x3000001, 0x800000, CRC(8d921c68) SHA1(42acf1d27d52a8e3b6262eb7df50693c0b135565) ) /* Plane 2,3 */
ROM_END

ROM_START( kof99n ) /* Original Version - Encrypted GFX */
	/* If chip label / ID correct: Korean Version ? */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "152-p1.bin", 0x000000, 0x100000, CRC(f2c7ddfa) SHA1(d592eecc53d442c55c2f26a6a721fdf2924d2a5b) )
	ROM_LOAD16_WORD_SWAP( "152-p2.bin", 0x100000, 0x400000, CRC(274ef47a) SHA1(98654b68cc85c19d4a90b46f3110f551fa2e5357) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "251-m1.bin", CRC(5e74539c) SHA1(6f49a9343cbd026b2c6720ff3fa2e5b1f85e80da) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "251-v1.bin", 0x000000, 0x400000, CRC(ef2eecc8) SHA1(8ed13b9db92dba3124bc5ba66e3e275885ece24a) )
	ROM_LOAD( "251-v2.bin", 0x400000, 0x400000, CRC(73e211ca) SHA1(0e60fa64cab6255d9721e2b4bc22e3de64c874c5) )
	ROM_LOAD( "251-v3.bin", 0x800000, 0x400000, CRC(821901da) SHA1(c6d4975bfaa19a62ed59126cadf2578c0a5c257f) )
	ROM_LOAD( "251-v4.bin", 0xc00000, 0x200000, CRC(b49e6178) SHA1(dde6f76e958841e8c99b693e13ced9aa9ef316dc) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "251-c1.bin", 0x0000000, 0x800000, CRC(0f9e93fe) SHA1(c7d77f0833c6f526f632e4f2dce59e302f6b9a15) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c2.bin", 0x0000001, 0x800000, CRC(e71e2ea3) SHA1(39c7a326fddbcca3b29c68cdc96aad4d62295c0f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c3.bin", 0x1000000, 0x800000, CRC(238755d2) SHA1(01125b5c7a28e350f091280b041954fd1ac7c98f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c4.bin", 0x1000001, 0x800000, CRC(438c8b22) SHA1(ffbc643f5b27dd00f2f95d4ef4c5f29ee814722b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c5.bin", 0x2000000, 0x800000, CRC(0b0abd0a) SHA1(d5ad324fe523bdc6f09209d236cc4932524a48f1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c6.bin", 0x2000001, 0x800000, CRC(65bbf281) SHA1(79ae174667a23dabcfe865b6cd6133c86098452e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c7.bin", 0x3000000, 0x800000, CRC(ff65f62e) SHA1(7cd335fede05b56e15db90ce407c1183a78da4e9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c8.bin", 0x3000001, 0x800000, CRC(8d921c68) SHA1(42acf1d27d52a8e3b6262eb7df50693c0b135565) ) /* Plane 2,3 */
ROM_END

ROM_START( kof99p ) /* Prototype Version - Possibly Hacked */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "251-p1p.bin", 0x000000, 0x100000, CRC(f37929c4) SHA1(226e7e3d629568399b88275e5bcd4e5b3839be95) )
	ROM_LOAD16_WORD_SWAP( "251-p2p.bin", 0x100000, 0x400000, CRC(739742ad) SHA1(31acaf05a9bf186305888d3db7e4e8a83f7bb0a4) )

	/* This is the S1 from the prototype, the final is different */
	NEO_SFIX_128K( "251-s1p.bin", CRC(fb1498ed) SHA1(d40060b31b6f217a4abdf3b336439fcd7bd7aaef) )

	/* Did the Prototype really use the same sound program / voice roms, sound isn't great .. */
	NEO_BIOS_AUDIO_128K( "251-m1.bin", CRC(5e74539c) SHA1(6f49a9343cbd026b2c6720ff3fa2e5b1f85e80da) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "251-v1.bin", 0x000000, 0x400000, CRC(ef2eecc8) SHA1(8ed13b9db92dba3124bc5ba66e3e275885ece24a) )
	ROM_LOAD( "251-v2.bin", 0x400000, 0x400000, CRC(73e211ca) SHA1(0e60fa64cab6255d9721e2b4bc22e3de64c874c5) )
	ROM_LOAD( "251-v3.bin", 0x800000, 0x400000, CRC(821901da) SHA1(c6d4975bfaa19a62ed59126cadf2578c0a5c257f) )
	ROM_LOAD( "251-v4.bin", 0xc00000, 0x200000, CRC(b49e6178) SHA1(dde6f76e958841e8c99b693e13ced9aa9ef316dc) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* these are probably decrypted versions of the roms found in the final */
	ROM_LOAD16_BYTE( "251-c1p.bin", 0x0000000, 0x800000, CRC(e5d8ffa4) SHA1(65f15f9f02424a7a9dd35916166594f283e8d424) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c2p.bin", 0x0000001, 0x800000, CRC(d822778f) SHA1(b590055e9bf1549bd6e1ecdabd65702202615712) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c3p.bin", 0x1000000, 0x800000, CRC(f20959e8) SHA1(38293043fa77ac51c5e3191118874c58f1ae4d30) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c4p.bin", 0x1000001, 0x800000, CRC(54ffbe9f) SHA1(8e62442923551f07a552621951b1accab2830e3b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c5p.bin", 0x2000000, 0x800000, CRC(d87a3bbc) SHA1(430f6812088712e0eb5714dcc664d8bba75e921a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c6p.bin", 0x2000001, 0x800000, CRC(4d40a691) SHA1(2b580d0678a5e6033ef16130671e860364d35e56) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "251-c7p.bin", 0x3000000, 0x800000, CRC(a4479a58) SHA1(d50e6cc9ccfe1ddbc6d90d46b8ca2cb0304edd8c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "251-c8p.bin", 0x3000001, 0x800000, CRC(ead513ce) SHA1(e9b07a0b01fdeb3004755a479df059c81b4d0ed6) ) /* Plane 2,3 */
ROM_END

ROM_START( ganryu ) /* Original Version - Encrypted GFX */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "252-p1.bin", 0x100000, 0x100000, CRC(4b8ac4fb) SHA1(93d90271bff281862b03beba3809cf95a47a1e44) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "252-m1.bin", CRC(30cc4099) SHA1(46453b7aac41855a92724a785372f8daf931d8d4) )

	ROM_REGION( 0x0400000, "ymsnd", 0 )
	ROM_LOAD( "252-v1.bin", 0x000000, 0x400000, CRC(e5946733) SHA1(d5904a50465af03d6ff33399a98f3259721ca0b2) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "252-c1.bin", 0x0000000, 0x800000, CRC(50ee7882) SHA1(ace0f95407c246d0456341cf2ad8a7668b81df8a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "252-c2.bin", 0x0000001, 0x800000, CRC(62585474) SHA1(b35461598087aa82886af0030c61b26cc064af5f) ) /* Plane 2,3 */
ROM_END

ROM_START( garou ) /* Original Version - Encrypted GFX */ /* later revision */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "253-sma.kf", 0x0c0000, 0x040000, CRC(98bc93dc) SHA1(01fe3d18b50f770e131e8d8eeff4c630ba8c9551) )	/* stored in the custom chip */
	ROM_LOAD16_WORD_SWAP( "253-ep1.p1", 0x100000, 0x200000, CRC(ea3171a4) SHA1(bbda40f652baa0dc5fc6a006c001a1bdb0df43f6) )
	ROM_LOAD16_WORD_SWAP( "253-ep2.p2", 0x300000, 0x200000, CRC(382f704b) SHA1(0ace9c84a8b8a0524fd9a503e7d872de1bf1bd52) )
	ROM_LOAD16_WORD_SWAP( "253-ep3.p3", 0x500000, 0x200000, CRC(e395bfdd) SHA1(6b50f5ac15bf66b7e4e9bff57594fd3d7530c831) )
	ROM_LOAD16_WORD_SWAP( "253-ep4.p4", 0x700000, 0x200000, CRC(da92c08e) SHA1(5556f983ebcebc33160e90a6a6cf589d54c8cedc) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 )	/* larger char set */
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_256K( "253-m1.bin", CRC(36a806be) SHA1(90fb44dc0c3fb57946a0f35716056abb84a0f191) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "253-v1.bin", 0x000000, 0x400000, CRC(263e388c) SHA1(11f05feee170370c4bfc5053af79246a6e3de5dc) )
	ROM_LOAD( "253-v2.bin", 0x400000, 0x400000, CRC(2c6bc7be) SHA1(c9c61054ce1a47bf1bf77a31117726b499df24a4) )
	ROM_LOAD( "253-v3.bin", 0x800000, 0x400000, CRC(0425b27d) SHA1(986863c98fc3445487242dcf2ea75b075e7f33ee) )
	ROM_LOAD( "253-v4.bin", 0xc00000, 0x400000, CRC(a54be8a9) SHA1(d7123e79b43e8adfaa5ecadbfcbeb6be890ec311) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "253-c1.bin", 0x0000000, 0x800000, CRC(0603e046) SHA1(5ef4557ce90ba65d36129de97be1fdc049c4a3d0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c2.bin", 0x0000001, 0x800000, CRC(0917d2a4) SHA1(d4ed3a13ae22f880fb399671c1752f1a0283f316) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c3.bin", 0x1000000, 0x800000, CRC(6737c92d) SHA1(678f0c9cc1267bd131546981b9989bfb7289d8ba) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c4.bin", 0x1000001, 0x800000, CRC(5ba92ec6) SHA1(aae36b050a3a0321026a96eba06dd184c0e2acca) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c5.bin", 0x2000000, 0x800000, CRC(3eab5557) SHA1(47c433015aa81a0b0a1d3ee51382c4948b80c023) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c6.bin", 0x2000001, 0x800000, CRC(308d098b) SHA1(b052f1fa9fbc69606004c250e2505360eaa24949) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c7.bin", 0x3000000, 0x800000, CRC(c0e995ae) SHA1(8675ca787d28246174c313167f82557f021366fc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c8.bin", 0x3000001, 0x800000, CRC(21a11303) SHA1(fd61221ad257c185ef5c1f9694bd6b840b591af3) ) /* Plane 2,3 */
ROM_END

ROM_START( garouo ) /* Original Version - Encrypted GFX */ /* earlier revision */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "253-sma.ke", 0x0c0000, 0x040000, CRC(96c72233) SHA1(29e19effd40fdf7e5144332396857f4ad0eff13e) )	/* stored in the custom chip */
	ROM_LOAD16_WORD_SWAP( "253-p1.bin", 0x100000, 0x400000, CRC(18ae5d7e) SHA1(bdb58ec9137d8653979b47132f2d10e1cc6aaa24) )
	ROM_LOAD16_WORD_SWAP( "253-p2.bin", 0x500000, 0x400000, CRC(afffa779) SHA1(ac017986f02277fbcd656b8c02492a3f4216a90e) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 )	/* larger char set */
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_256K( "253-m1.bin", CRC(36a806be) SHA1(90fb44dc0c3fb57946a0f35716056abb84a0f191) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "253-v1.bin", 0x000000, 0x400000, CRC(263e388c) SHA1(11f05feee170370c4bfc5053af79246a6e3de5dc) )
	ROM_LOAD( "253-v2.bin", 0x400000, 0x400000, CRC(2c6bc7be) SHA1(c9c61054ce1a47bf1bf77a31117726b499df24a4) )
	ROM_LOAD( "253-v3.bin", 0x800000, 0x400000, CRC(0425b27d) SHA1(986863c98fc3445487242dcf2ea75b075e7f33ee) )
	ROM_LOAD( "253-v4.bin", 0xc00000, 0x400000, CRC(a54be8a9) SHA1(d7123e79b43e8adfaa5ecadbfcbeb6be890ec311) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "253-c1.bin", 0x0000000, 0x800000, CRC(0603e046) SHA1(5ef4557ce90ba65d36129de97be1fdc049c4a3d0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c2.bin", 0x0000001, 0x800000, CRC(0917d2a4) SHA1(d4ed3a13ae22f880fb399671c1752f1a0283f316) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c3.bin", 0x1000000, 0x800000, CRC(6737c92d) SHA1(678f0c9cc1267bd131546981b9989bfb7289d8ba) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c4.bin", 0x1000001, 0x800000, CRC(5ba92ec6) SHA1(aae36b050a3a0321026a96eba06dd184c0e2acca) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c5.bin", 0x2000000, 0x800000, CRC(3eab5557) SHA1(47c433015aa81a0b0a1d3ee51382c4948b80c023) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c6.bin", 0x2000001, 0x800000, CRC(308d098b) SHA1(b052f1fa9fbc69606004c250e2505360eaa24949) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c7.bin", 0x3000000, 0x800000, CRC(c0e995ae) SHA1(8675ca787d28246174c313167f82557f021366fc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c8.bin", 0x3000001, 0x800000, CRC(21a11303) SHA1(fd61221ad257c185ef5c1f9694bd6b840b591af3) ) /* Plane 2,3 */
ROM_END

ROM_START( garoup ) /* Prototype Version, seems genuine */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "253-p1p.bin", 0x000000, 0x100000, CRC(c72f0c16) SHA1(1ff6bb651682f93bef9ff02622c3cf63fe594986) )
	ROM_LOAD16_WORD_SWAP( "253-p2p.bin", 0x100000, 0x400000, CRC(bf8de565) SHA1(0e24574168cd38138bed0aa4dca49849f6901ca2) )

	NEO_SFIX_128K( "253-s1p.bin", CRC(779989de) SHA1(8bd550857b60f8a907f6d39a4225ceffdd330307) )

	NEO_BIOS_AUDIO_256K( "253-m1p.bin", CRC(bbe464f7) SHA1(f5f8f3e48f5d453f45107085d6f4023bcd24c053) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "253-v1p.bin", 0x000000, 0x400000, CRC(274f3923) SHA1(4c7a8ad1cd0e3afc1f78de3c2929120ed434f104) )
	ROM_LOAD( "253-v2p.bin", 0x400000, 0x400000, CRC(8f86dabe) SHA1(b3d2d9f5c1d97a6e7aee2c674fb6627f41bbb240) )
	ROM_LOAD( "253-v3p.bin", 0x800000, 0x400000, CRC(05fd06cd) SHA1(6cd699719614bb87547632ea3d61d92d81fdf563) )
	ROM_LOAD( "253-v4p.bin", 0xc00000, 0x400000, CRC(14984063) SHA1(170d5638327ec0eb3590b80dc11590897367250c) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "253-c1p.bin", 0x0000000, 0x800000, CRC(5bb5d137) SHA1(d648febd8e6a0bdd9bdbb6ce1f1f8b08567ec05a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c2p.bin", 0x0000001, 0x800000, CRC(5c8d2960) SHA1(f7503502be0332adf408ee0ea5ee5161c8939fd8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c3p.bin", 0x1000000, 0x800000, CRC(234d16fc) SHA1(7b9221f7ecc438150c8a10be72390329854ed21b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c4p.bin", 0x1000001, 0x800000, CRC(b9b5b993) SHA1(6059793eaf6e58c172235fe64aa9d25a40c38ed6) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c5p.bin", 0x2000000, 0x800000, CRC(722615d2) SHA1(798832c535869f0e247c3db0d8253779b103e213) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c6p.bin", 0x2000001, 0x800000, CRC(0a6fab38) SHA1(eaee6f2f18af91f7959d84d4b991b3fc182d07c4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "253-c7p.bin", 0x3000000, 0x800000, CRC(d68e806f) SHA1(92bfd9839115bd590972ae4ecc45ad35dce22387) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "253-c8p.bin", 0x3000001, 0x800000, CRC(f778fe99) SHA1(c963f6ba90a36d02991728b44ffcf174ca18268a) ) /* Plane 2,3 */
ROM_END

ROM_START( s1945p ) /* Original Version, Encrypted GFX Roms */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "254-p1.bin", 0x000000, 0x100000, CRC(ff8efcff) SHA1(dcaeaca573385c172ecc43ee6bee355359091893) )
	ROM_LOAD16_WORD_SWAP( "254-p2.bin", 0x100000, 0x400000, CRC(efdfd4dd) SHA1(254f3e1b546eed788f7ae919be9d1bf9702148ce) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "254-m1.bin", CRC(994b4487) SHA1(a4e645a3ababa48a8325980ff022e04a8b51b017) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "254-v1.bin", 0x000000, 0x400000, CRC(844f58fb) SHA1(e59544457be9f21481eac8b5a39b9cbb502f252d) )
	ROM_LOAD( "254-v2.bin", 0x400000, 0x400000, CRC(d9a248f0) SHA1(dd3e0974b753e6f94d0943a002de45668a1b072b) )
	ROM_LOAD( "254-v3.bin", 0x800000, 0x400000, CRC(0b0d2d33) SHA1(f8e76af42a997f36a40f66b39de00f68afe6a89c) )
	ROM_LOAD( "254-v4.bin", 0xc00000, 0x400000, CRC(6d13dc91) SHA1(8433513c0b5aea61939068a25ab90efbe3e44116) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "254-c1.bin", 0x0000000, 0x800000, CRC(ae6fc8ef) SHA1(544ccdaee8a4a45cdce9483e30852811d2d5f3cc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "254-c2.bin", 0x0000001, 0x800000, CRC(436fa176) SHA1(d70141a91a360a1b1070753086f976608fec38af) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "254-c3.bin", 0x1000000, 0x800000, CRC(e53ff2dc) SHA1(31f6aaffe28146d574aa72f14f90a9d968f36bc6) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "254-c4.bin", 0x1000001, 0x800000, CRC(818672f0) SHA1(460c6738d0ee5ae440a23fc1434fab53bbb242b5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "254-c5.bin", 0x2000000, 0x800000, CRC(4580eacd) SHA1(feb96eb5e80c9125ddd7836e0939212cd3011c34) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "254-c6.bin", 0x2000001, 0x800000, CRC(e34970fc) SHA1(6e43e15e27bc914357f977116ab1e2d98711bb21) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "254-c7.bin", 0x3000000, 0x800000, CRC(f2323239) SHA1(5b3e8dd77474203be010ec7363858d806344a320) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "254-c8.bin", 0x3000001, 0x800000, CRC(66848c7d) SHA1(24d4ed627940a4cf8129761c1da15556e52e199c) ) /* Plane 2,3 */
ROM_END

ROM_START( preisle2 ) /* Original Version, Encrypted GFX */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "255-p1.bin", 0x000000, 0x100000, CRC(dfa3c0f3) SHA1(793c6a46f3a794536dc0327a3f3fad20e25ab661) )
	ROM_LOAD16_WORD_SWAP( "255-p2.bin", 0x100000, 0x400000, CRC(42050b80) SHA1(0981a8295d43b264c2b95e5d7568bdda4e64c976) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "255-m1.bin", CRC(8efd4014) SHA1(5b73809b6e4e49264d281ef3e5004ac8a9de296d) )

	ROM_REGION( 0x0600000, "ymsnd", 0 )
	ROM_LOAD( "255-v1.bin", 0x000000, 0x400000, CRC(5a14543d) SHA1(7146ac748f846c7e2d5b0bdcf953892e39b648fe) )
	ROM_LOAD( "255-v2.bin", 0x400000, 0x200000, CRC(6610d91a) SHA1(b2c6786920dc1712e88c3cc26d2c6c3ac2615bf4) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "255-c1.bin", 0x0000000, 0x800000, CRC(ea06000b) SHA1(1539b12e461fa48301190eb8171bbffff9d984b7) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "255-c2.bin", 0x0000001, 0x800000, CRC(04e67d79) SHA1(aadb6ee750da2c14c6eededa2218db95e051a32c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "255-c3.bin", 0x1000000, 0x800000, CRC(60e31e08) SHA1(bd5b81ad9d04cdc4e0df31ac40eca305f98277eb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "255-c4.bin", 0x1000001, 0x800000, CRC(40371d69) SHA1(90011ccc5672ff1b90737cf50c963e71b6217ce3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "255-c5.bin", 0x2000000, 0x800000, CRC(0b2e6adf) SHA1(15c7d9aa8b1ad9a071e6fd0ef0de8a057c23b02e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "255-c6.bin", 0x2000001, 0x800000, CRC(b001bdd3) SHA1(394ba8004644844ee97a120cfda48aeac685af8a) ) /* Plane 2,3 */
ROM_END

ROM_START( mslug3 ) /* Original Version - Encrypted Code & GFX */ /* revision 2000.4.1 */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "256-sma.bin", 0x0c0000, 0x040000, CRC(9cd55736) SHA1(d6efb2b313127c2911d47d9324626b3f1e7c6ccb) )	/* stored in the custom chip */
	ROM_LOAD16_WORD_SWAP( "256-p1.bin",  0x100000, 0x400000, CRC(b07edfd5) SHA1(dcbd9e500bfae98d754e55cdbbbbf9401013f8ee) )
	ROM_LOAD16_WORD_SWAP( "256-p2.bin",  0x500000, 0x400000, CRC(6097c26b) SHA1(248ec29d21216f29dc6f5f3f0e1ad1601b3501b6) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_512K( "256-m1.bin", CRC(eaeec116) SHA1(54419dbb21edc8c4b37eaac2e7ad9496d2de037a) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "256-v1.bin", 0x000000, 0x400000, CRC(f2690241) SHA1(fd56babc1934d10e0d27c32f032f9edda7ca8ce9) )
	ROM_LOAD( "256-v2.bin", 0x400000, 0x400000, CRC(7e2a10bd) SHA1(0d587fb9f64cba0315ce2d8a03e2b8fe34936dff) )
	ROM_LOAD( "256-v3.bin", 0x800000, 0x400000, CRC(0eaec17c) SHA1(c3ed613cc6993edd6fc0d62a90bcd85de8e21915) )
	ROM_LOAD( "256-v4.bin", 0xc00000, 0x400000, CRC(9b4b22d4) SHA1(9764fbf8453e52f80aa97a46fb9cf5937ef15a31) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "256-c1.bin", 0x0000000, 0x800000, CRC(5a79c34e) SHA1(b8aa51fa50935cae62ab3d125b723ab888691e60) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c2.bin", 0x0000001, 0x800000, CRC(944c362c) SHA1(3843ab300f956280475469caee70135658f67089) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c3.bin", 0x1000000, 0x800000, CRC(6e69d36f) SHA1(94e8cf42e999114b4bd8b30e0aa2f365578c4c9a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c4.bin", 0x1000001, 0x800000, CRC(b755b4eb) SHA1(804700a0966a48f130c434ede3f970792ea74fa5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c5.bin", 0x2000000, 0x800000, CRC(7aacab47) SHA1(312c1c9846175fe1a3cad51d5ae230cf674fc93d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c6.bin", 0x2000001, 0x800000, CRC(c698fd5d) SHA1(16818883b06849ba2f8d61bdd5e21aaf99bd8408) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c7.bin", 0x3000000, 0x800000, CRC(cfceddd2) SHA1(7def666adf8bd1703f40c61f182fc040b6362dc9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c8.bin", 0x3000001, 0x800000, CRC(4d9be34c) SHA1(a737bdfa2b815aea7067e7af2636e83a9409c414) ) /* Plane 2,3 */
ROM_END

ROM_START( mslug3h ) /* Original Version - Encrypted GFX */ /* MVS AND AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "256-ph1.bin", 0x000000, 0x100000, CRC(9c42ca85) SHA1(7a8f77a89867b889295ae9b9dfd4ba28f02d234d) )
	ROM_LOAD16_WORD_SWAP( "256-ph2.bin", 0x100000, 0x400000, CRC(1f3d8ce8) SHA1(08b05a8abfb86ec09a5e758d6273acf1489961f9) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_512K( "256-m1.bin", CRC(eaeec116) SHA1(54419dbb21edc8c4b37eaac2e7ad9496d2de037a) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "256-v1.bin", 0x000000, 0x400000, CRC(f2690241) SHA1(fd56babc1934d10e0d27c32f032f9edda7ca8ce9) )
	ROM_LOAD( "256-v2.bin", 0x400000, 0x400000, CRC(7e2a10bd) SHA1(0d587fb9f64cba0315ce2d8a03e2b8fe34936dff) )
	ROM_LOAD( "256-v3.bin", 0x800000, 0x400000, CRC(0eaec17c) SHA1(c3ed613cc6993edd6fc0d62a90bcd85de8e21915) )
	ROM_LOAD( "256-v4.bin", 0xc00000, 0x400000, CRC(9b4b22d4) SHA1(9764fbf8453e52f80aa97a46fb9cf5937ef15a31) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "256-c1.bin", 0x0000000, 0x800000, CRC(5a79c34e) SHA1(b8aa51fa50935cae62ab3d125b723ab888691e60) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c2.bin", 0x0000001, 0x800000, CRC(944c362c) SHA1(3843ab300f956280475469caee70135658f67089) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c3.bin", 0x1000000, 0x800000, CRC(6e69d36f) SHA1(94e8cf42e999114b4bd8b30e0aa2f365578c4c9a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c4.bin", 0x1000001, 0x800000, CRC(b755b4eb) SHA1(804700a0966a48f130c434ede3f970792ea74fa5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c5.bin", 0x2000000, 0x800000, CRC(7aacab47) SHA1(312c1c9846175fe1a3cad51d5ae230cf674fc93d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c6.bin", 0x2000001, 0x800000, CRC(c698fd5d) SHA1(16818883b06849ba2f8d61bdd5e21aaf99bd8408) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c7.bin", 0x3000000, 0x800000, CRC(cfceddd2) SHA1(7def666adf8bd1703f40c61f182fc040b6362dc9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c8.bin", 0x3000001, 0x800000, CRC(4d9be34c) SHA1(a737bdfa2b815aea7067e7af2636e83a9409c414) ) /* Plane 2,3 */
ROM_END

ROM_START( kof2000 ) /* Original Version, Encrypted Code + Sound + GFX Roms */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "257-sma.bin", 0x0c0000, 0x040000, CRC(71c6e6bb) SHA1(1bd29ded4c6b29780db8e8b772c452189699ca89) )	/* stored in the custom chip */
	ROM_LOAD16_WORD_SWAP( "257-p1.bin",  0x100000, 0x400000, CRC(60947b4c) SHA1(5faa0a7ac7734d6c8e276589bd12dd574264647d) )
	ROM_LOAD16_WORD_SWAP( "257-p2.bin",  0x500000, 0x400000, CRC(1b7ec415) SHA1(f19fa44e9ee5b5a6eb4a051349d6bc4acc3bbbdb) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 )	/* larger char set */
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_256K( "257-m1.bin", CRC(4b749113) SHA1(2af2361146edd0ce3966614d90165a5c1afb8de4) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "257-v1.bin", 0x000000, 0x400000, CRC(17cde847) SHA1(4bcc0205b70dc6d9216b29025450c9c5b08cb65d) )
	ROM_LOAD( "257-v2.bin", 0x400000, 0x400000, CRC(1afb20ff) SHA1(57dfd2de058139345ff2b744a225790baaecd5a2) )
	ROM_LOAD( "257-v3.bin", 0x800000, 0x400000, CRC(4605036a) SHA1(51b228a0600d38a6ec37aec4822879ec3b0ee106) )
	ROM_LOAD( "257-v4.bin", 0xc00000, 0x400000, CRC(764bbd6b) SHA1(df23c09ca6cf7d0ae5e11ff16e30c159725106b3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "257-c1.bin", 0x0000000, 0x800000, CRC(cef1cdfa) SHA1(6135080f3a6b4712b76cc217edcc58e72b55c2b9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "257-c2.bin", 0x0000001, 0x800000, CRC(f7bf0003) SHA1(9f7b19a2100cf7d12867e742f440dd5277b4f895) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "257-c3.bin", 0x1000000, 0x800000, CRC(101e6560) SHA1(8073ae1139e215d1167f8d32c14079a46ce3ee1c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "257-c4.bin", 0x1000001, 0x800000, CRC(bd2fc1b1) SHA1(da0006761923ad49b404a08d7a151193ee307a69) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "257-c5.bin", 0x2000000, 0x800000, CRC(89775412) SHA1(b221b30224bc4239f1b3c2d2fd1cd4fa84e3523c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "257-c6.bin", 0x2000001, 0x800000, CRC(fa7200d5) SHA1(6f2b0d38af34e280d56a58955400e5c679906871) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "257-c7.bin", 0x3000000, 0x800000, CRC(7da11fe4) SHA1(065336cf166807acb6c8569d59d3bf37a19b0a42) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "257-c8.bin", 0x3000001, 0x800000, CRC(b1afa60b) SHA1(b916184f5cfe4121752270f4f65abf35d8eb0519) ) /* Plane 2,3 */
ROM_END

ROM_START( kof2000n ) /* Original Version, Encrypted Sound + GFX Roms */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "257-pg1.bin", 0x000000, 0x100000, CRC(5f809dbe) SHA1(2bc233dcff5622de86d01e3b74b840c7caf12982) )
	ROM_LOAD16_WORD_SWAP( "257-pg2.bin", 0x100000, 0x400000, CRC(693c2c5e) SHA1(dc9121b7369ef46596343cac055a00aec81704d4) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 )	/* larger char set */
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_256K( "257-m1.bin", CRC(4b749113) SHA1(2af2361146edd0ce3966614d90165a5c1afb8de4) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "257-v1.bin", 0x000000, 0x400000, CRC(17cde847) SHA1(4bcc0205b70dc6d9216b29025450c9c5b08cb65d) )
	ROM_LOAD( "257-v2.bin", 0x400000, 0x400000, CRC(1afb20ff) SHA1(57dfd2de058139345ff2b744a225790baaecd5a2) )
	ROM_LOAD( "257-v3.bin", 0x800000, 0x400000, CRC(4605036a) SHA1(51b228a0600d38a6ec37aec4822879ec3b0ee106) )
	ROM_LOAD( "257-v4.bin", 0xc00000, 0x400000, CRC(764bbd6b) SHA1(df23c09ca6cf7d0ae5e11ff16e30c159725106b3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "257-c1.bin", 0x0000000, 0x800000, CRC(cef1cdfa) SHA1(6135080f3a6b4712b76cc217edcc58e72b55c2b9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "257-c2.bin", 0x0000001, 0x800000, CRC(f7bf0003) SHA1(9f7b19a2100cf7d12867e742f440dd5277b4f895) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "257-c3.bin", 0x1000000, 0x800000, CRC(101e6560) SHA1(8073ae1139e215d1167f8d32c14079a46ce3ee1c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "257-c4.bin", 0x1000001, 0x800000, CRC(bd2fc1b1) SHA1(da0006761923ad49b404a08d7a151193ee307a69) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "257-c5.bin", 0x2000000, 0x800000, CRC(89775412) SHA1(b221b30224bc4239f1b3c2d2fd1cd4fa84e3523c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "257-c6.bin", 0x2000001, 0x800000, CRC(fa7200d5) SHA1(6f2b0d38af34e280d56a58955400e5c679906871) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "257-c7.bin", 0x3000000, 0x800000, CRC(7da11fe4) SHA1(065336cf166807acb6c8569d59d3bf37a19b0a42) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "257-c8.bin", 0x3000001, 0x800000, CRC(b1afa60b) SHA1(b916184f5cfe4121752270f4f65abf35d8eb0519) ) /* Plane 2,3 */
ROM_END

ROM_START( bangbead ) /* Original Version - Encrypted GFX */
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "259-p1.bin", 0x100000, 0x100000, CRC(88a37f8b) SHA1(566db84850fad5e8fe822e8bba910a33e083b550) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "259-m1.bin", CRC(85668ee9) SHA1(7d3f51710cf90c097cd3faaeeef10ceb85cbb3e8) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "259-v1.bin", 0x000000, 0x400000, CRC(088eb8ab) SHA1(608306e35501dd7d382d9f96b28e7550aa896a03) )
	ROM_LOAD( "259-v2.bin", 0x400000, 0x100000, CRC(97528fe9) SHA1(8f5eddbb3a9a225492479d1a44801f3916c8e791) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "259-c1.bin", 0x0000000, 0x800000, CRC(1f537f74) SHA1(b8ef691e92191c20a5ed4f20a75cca3c7383bca6) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "259-c2.bin", 0x0000001, 0x800000, CRC(0efd98ff) SHA1(d350315d3c7f26d638458e5ccf2126069a4c7a5b) ) /* Plane 2,3 */
ROM_END

/* same data, diff. format roms, not encrypted, it could be a bootleg, not a prototype, since its identical
   when decrypted i'm disabling it for now */
#if 0
ROM_START( bangbedp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "259-p1.bin", 0x100000, 0x100000, CRC(88a37f8b) SHA1(566db84850fad5e8fe822e8bba910a33e083b550) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "259-s1p.bin", CRC(bb50fb2d) SHA1(7372939f328fb5e7d09c16985e09ae8c34702b0c) )

	NEO_BIOS_AUDIO_128K( "259-m1.bin", CRC(85668ee9) SHA1(7d3f51710cf90c097cd3faaeeef10ceb85cbb3e8) )

	ROM_REGION( 0x500000, "ymsnd", 0 )
	ROM_LOAD( "259-v1p.bin", 0x000000, 0x200000, CRC(e97b9385) SHA1(d213cae6cf5732b8ab1f8a8cf04afee5dfd9a260) )
	ROM_LOAD( "259-v2p.bin", 0x200000, 0x200000, CRC(b0cbd70a) SHA1(fc7c8183f8ff9800e8aae7c8d44d962300058cfb) )
	ROM_LOAD( "259-v3p.bin", 0x400000, 0x100000, CRC(97528fe9) SHA1(8f5eddbb3a9a225492479d1a44801f3916c8e791) )

	NO_DELTAT_REGION

	ROM_REGION( 0x600000, "sprites", 0 )
	ROM_LOAD16_BYTE( "259-c1p.bin", 0x000000, 0x200000, CRC(e3919e44) SHA1(54c722414b5a7ad311dc8ddf6fdda88535e829d1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "259-c2p.bin", 0x000001, 0x200000, CRC(baf5a320) SHA1(ead3d81d9b4aeb45af4f9cb5c38157f2236b506c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "259-c3p.bin", 0x400000, 0x100000, CRC(c8e52157) SHA1(f10f58e905c4cbaf182b20e63abe5364462133c5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "259-c4p.bin", 0x400001, 0x100000, CRC(69fa8e60) SHA1(29c1fbdb79dedf1470683202e2cb3435732d9275) ) /* Plane 2,3 */
ROM_END
#endif

ROM_START( nitd ) /* Original Version - Encrypted GFX */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "260-p1.bin", 0x000000, 0x080000, CRC(61361082) SHA1(441f3f41c1aa752c0e0a9a0b1d92711d9e636b85) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_512K( "260-m1.bin", CRC(6407c5e5) SHA1(d273e154cc905b63205a17a1a6d419cac3485a92) )

	ROM_REGION( 0x0400000, "ymsnd", 0 )
	ROM_LOAD( "260-v1.bin", 0x000000, 0x400000, CRC(24b0480c) SHA1(d769e621be52a5cd2e2568891b5f95a48268e1e0) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "260-c1.bin", 0x0000000, 0x800000, CRC(147b0c7f) SHA1(a647c3a2f6d146ff47521c1d39f58830601f5781) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "260-c2.bin", 0x0000001, 0x800000, CRC(d2b04b0d) SHA1(ce4322e6cfacb627fe997efe81018861e21d3c27) ) /* Plane 2,3 */
ROM_END

ROM_START( sengoku3 ) /* Original Version - Encrypted GFX */
	ROM_REGION( 0x200000, "maincpu", 0 )
	//ROM_LOAD16_WORD_SWAP( "261-p1.bin", 0x100000, 0x100000, CRC(5b557201) SHA1(d01421d1dc80fe7d2a46b9f79c0f344b3c81c1e7) ) this one is almost certainly bad
	ROM_LOAD16_WORD_SWAP( "261-ph1.bin", 0x100000, 0x100000, CRC(e0d4bc0a) SHA1(8df366097f224771ca6d1aa5c1691cd46776cd12) )
	ROM_CONTINUE( 0x000000, 0x100000 )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "261-m1.bin", CRC(36ed9cdd) SHA1(78a7d755e9e9f52255ac6228d9d402fd6a02c126) )

	ROM_REGION( 0x0e00000, "ymsnd", 0 )
	ROM_LOAD( "261-v1.bin", 0x000000, 0x400000, CRC(64c30081) SHA1(f9ebd20cf59b72e864b7274c1bdb6d99ecaf4595) )
	ROM_LOAD( "261-v2.bin", 0x400000, 0x400000, CRC(392a9c47) SHA1(7ab90a54089236ca6c3ef1af8e566a8025d38159) )
	ROM_LOAD( "261-v3.bin", 0x800000, 0x400000, CRC(c1a7ebe3) SHA1(1d7bb481451f5ee0457e954bb5210300182c3c9c) )
	ROM_LOAD( "261-v4.bin", 0xc00000, 0x200000, CRC(9000d085) SHA1(11157b355ab4eb6627e9f322ed875332d3d77349) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "261-c1.bin", 0x0000000, 0x800000, CRC(ded84d9c) SHA1(d960523b813d4fae06d716298d4e431a5c77a0c5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "261-c2.bin", 0x0000001, 0x800000, CRC(b8eb4348) SHA1(619d24312549932959481fa58f43f11c048e1ca5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "261-c3.bin", 0x1000000, 0x800000, CRC(84e2034a) SHA1(38ec4ae4b86933a25c9a03799b8cade4b1346401) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "261-c4.bin", 0x1000001, 0x800000, CRC(0b45ae53) SHA1(a19fb21408ab633aee8bbf38bf43b5e26766b355) ) /* Plane 2,3 */
ROM_END

ROM_START( kof2001 ) /* MVS VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "262-p1-08-e0.bin", 0x000000, 0x100000, CRC(9381750d) SHA1(dcfecd69e563ff52fe07d23c5372d0f748b07819) )
	ROM_LOAD16_WORD_SWAP( "262-p2-08-e0.bin", 0x100000, 0x400000, CRC(8e0d8329) SHA1(10dcc1baf0aaf1fc84c4d856bca6bcff85aed2bc) )
	/* The first/early production run sets have proms with above labels. Some later? sets found have eproms instead of proms */

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_256K( "265-262-m1.bin", CRC(a7f8119f) SHA1(71805b39b8b09c32425cf39f9de59b2f755976c2) ) /* yes it really does have a strange name */

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "262-v1-08-e0.bin", 0x000000, 0x400000, CRC(83d49ecf) SHA1(2f2c116e45397652e77fcf5d951fa5f71b639572) )
	ROM_LOAD( "262-v2-08-e0.bin", 0x400000, 0x400000, CRC(003f1843) SHA1(bdd58837ad542548bd4053c262f558af88e3b989) )
	ROM_LOAD( "262-v3-08-e0.bin", 0x800000, 0x400000, CRC(2ae38dbe) SHA1(4e82b7dd3b899d61907620517a5a27bdaba0725d) )
	ROM_LOAD( "262-v4-08-e0.bin", 0xc00000, 0x400000, CRC(26ec4dd9) SHA1(8bd68d95a2d913be41a51f51e48dbe3bff5924fb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "262-c1-08-e0.bin", 0x0000000, 0x800000, CRC(99cc785a) SHA1(374f0674871d0196fa274aa6c5956d7b3848d5da) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "262-c2-08-e0.bin", 0x0000001, 0x800000, CRC(50368cbf) SHA1(5d9e206e98e0b0c7735b72ea46b45058fdec2352) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "262-c3-08-e0.bin", 0x1000000, 0x800000, CRC(fb14ff87) SHA1(445a8db2fc69eff54a252700f2d3a89244c58e75) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "262-c4-08-e0.bin", 0x1000001, 0x800000, CRC(4397faf8) SHA1(6752b394f6647502a649a3e62bd3442f936b733e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "262-c5-08-e0.bin", 0x2000000, 0x800000, CRC(91f24be4) SHA1(88190c41f7d4a0f4b1982149fc9acfc640af498d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "262-c6-08-e0.bin", 0x2000001, 0x800000, CRC(a31e4403) SHA1(5cd1a14703aa58810e2377dfb7353c61e9dc9c1f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "262-c7-08-e0.bin", 0x3000000, 0x800000, CRC(54d9d1ec) SHA1(80c3a8ec39130dd5d3da561f287709da6b8abcf4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "262-c8-08-e0.bin", 0x3000001, 0x800000, CRC(59289a6b) SHA1(ddfce7c85b2a144975db5bb14b4b51aaf881880e) ) /* Plane 2,3 */
ROM_END

ROM_START( kof2001h ) /* AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "262-pg1.bin", 0x000000, 0x100000, CRC(2af7e741) SHA1(e41282d73ed6d521da056f1a16573bb61bfa3826) )
	ROM_LOAD16_WORD_SWAP( "262-pg2.bin", 0x100000, 0x400000, CRC(91eea062) SHA1(82bae42bbeedb9f3aa0c7c0b0a7a69be499cf98f) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_256K( "265-262-m1.bin", CRC(a7f8119f) SHA1(71805b39b8b09c32425cf39f9de59b2f755976c2) ) /* yes it really does have a strange name */

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "262-v1-08-e0.bin", 0x000000, 0x400000, CRC(83d49ecf) SHA1(2f2c116e45397652e77fcf5d951fa5f71b639572) )
	ROM_LOAD( "262-v2-08-e0.bin", 0x400000, 0x400000, CRC(003f1843) SHA1(bdd58837ad542548bd4053c262f558af88e3b989) )
	ROM_LOAD( "262-v3-08-e0.bin", 0x800000, 0x400000, CRC(2ae38dbe) SHA1(4e82b7dd3b899d61907620517a5a27bdaba0725d) )
	ROM_LOAD( "262-v4-08-e0.bin", 0xc00000, 0x400000, CRC(26ec4dd9) SHA1(8bd68d95a2d913be41a51f51e48dbe3bff5924fb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "262-c1-08-e0.bin", 0x0000000, 0x800000, CRC(99cc785a) SHA1(374f0674871d0196fa274aa6c5956d7b3848d5da) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "262-c2-08-e0.bin", 0x0000001, 0x800000, CRC(50368cbf) SHA1(5d9e206e98e0b0c7735b72ea46b45058fdec2352) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "262-c3-08-e0.bin", 0x1000000, 0x800000, CRC(fb14ff87) SHA1(445a8db2fc69eff54a252700f2d3a89244c58e75) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "262-c4-08-e0.bin", 0x1000001, 0x800000, CRC(4397faf8) SHA1(6752b394f6647502a649a3e62bd3442f936b733e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "262-c5-08-e0.bin", 0x2000000, 0x800000, CRC(91f24be4) SHA1(88190c41f7d4a0f4b1982149fc9acfc640af498d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "262-c6-08-e0.bin", 0x2000001, 0x800000, CRC(a31e4403) SHA1(5cd1a14703aa58810e2377dfb7353c61e9dc9c1f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "262-c7-08-e0.bin", 0x3000000, 0x800000, CRC(54d9d1ec) SHA1(80c3a8ec39130dd5d3da561f287709da6b8abcf4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "262-c8-08-e0.bin", 0x3000001, 0x800000, CRC(59289a6b) SHA1(ddfce7c85b2a144975db5bb14b4b51aaf881880e) ) /* Plane 2,3 */
ROM_END

ROM_START( mslug4 ) /* Original Version - Encrypted GFX */ /* MVS VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	/* There also exist carts where p1 label is pg1;
    The PG1 revision has a Japanese cart label, SN 02Jxxxxx
    The P1 revision has a US/EUR cart label, SN 02Txxxxx
    Rom data on both is identical.
    These carts were manufactured by Mega Enterprise, not SNK. */
	ROM_LOAD16_WORD_SWAP( "263-p1.bin", 0x000000, 0x100000, CRC(27e4def3) SHA1(a08785e8145981bb6b5332a3b2df7eb321253cca) )
	ROM_LOAD16_WORD_SWAP( "263-p2.bin", 0x100000, 0x400000, CRC(fdb7aed8) SHA1(dbeaec38f44e58ffedba99e70fa1439c2bf0dfa3) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 )	/* larger char set */
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "263-m1.bin", CRC(46ac8228) SHA1(5aeea221050c98e4bb0f16489ce772bf1c80f787) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "263-v1.bin", 0x000000, 0x800000, CRC(01e9b9cd) SHA1(0b045c2999449f7dab5ae8a42e957d5b6650431e) )
	ROM_LOAD( "263-v2.bin", 0x800000, 0x800000, CRC(4ab2bf81) SHA1(77ccfa48f7e3daddef5fe5229a0093eb2f803742) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "263-c1.bin", 0x0000000, 0x800000, CRC(84865f8a) SHA1(34467ada896eb7c7ca58658bf2a932936d8b632c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c2.bin", 0x0000001, 0x800000, CRC(81df97f2) SHA1(2b74493b8ec8fd49216a627aeb3db493f76124e3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "263-c3.bin", 0x1000000, 0x800000, CRC(1a343323) SHA1(bbbb5232bba538c277ce2ee02e2956ca2243b787) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c4.bin", 0x1000001, 0x800000, CRC(942cfb44) SHA1(d9b46c71726383c4581fb042e63897e5a3c92d1b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "263-c5.bin", 0x2000000, 0x800000, CRC(a748854f) SHA1(2611bbedf9b5d8e82c6b2c99b88f842c46434d41) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c6.bin", 0x2000001, 0x800000, CRC(5c8ba116) SHA1(6034db09c8706d4ddbcefc053efbc47a0953eb92) ) /* Plane 2,3 */
ROM_END

ROM_START( mslug4h ) /* Original Version - Encrypted GFX */ /* AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "263-ph1.bin", 0x000000, 0x100000, CRC(c67f5c8d) SHA1(12af74964843f103520d9f0825069ea2f67eeb2f) )
	ROM_LOAD16_WORD_SWAP( "263-ph2.bin", 0x100000, 0x400000, CRC(bc3ec89e) SHA1(2cb0626bc4fa57e1d25f208e04532b570d87b3fb) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 )	/* larger char set */
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "263-m1.bin", CRC(46ac8228) SHA1(5aeea221050c98e4bb0f16489ce772bf1c80f787) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "263-v1.bin", 0x000000, 0x800000, CRC(01e9b9cd) SHA1(0b045c2999449f7dab5ae8a42e957d5b6650431e) )
	ROM_LOAD( "263-v2.bin", 0x800000, 0x800000, CRC(4ab2bf81) SHA1(77ccfa48f7e3daddef5fe5229a0093eb2f803742) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "263-c1.bin", 0x0000000, 0x800000, CRC(84865f8a) SHA1(34467ada896eb7c7ca58658bf2a932936d8b632c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c2.bin", 0x0000001, 0x800000, CRC(81df97f2) SHA1(2b74493b8ec8fd49216a627aeb3db493f76124e3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "263-c3.bin", 0x1000000, 0x800000, CRC(1a343323) SHA1(bbbb5232bba538c277ce2ee02e2956ca2243b787) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c4.bin", 0x1000001, 0x800000, CRC(942cfb44) SHA1(d9b46c71726383c4581fb042e63897e5a3c92d1b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "263-c5.bin", 0x2000000, 0x800000, CRC(a748854f) SHA1(2611bbedf9b5d8e82c6b2c99b88f842c46434d41) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c6.bin", 0x2000001, 0x800000, CRC(5c8ba116) SHA1(6034db09c8706d4ddbcefc053efbc47a0953eb92) ) /* Plane 2,3 */
ROM_END


ROM_START( rotd ) /* Encrypted Set */ /* MVS VERSION */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "264-p1.bin", 0x000000, 0x800000, CRC(b8cc969d) SHA1(4f2205b4bdd32dd1522106ef4df10ac0eb1b852d) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "264-m1.bin", CRC(4dbd7b43) SHA1(6b63756b0d2d30bbf13fbd219833c81fd060ef96) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "264-v1.bin", 0x000000, 0x800000, CRC(fa005812) SHA1(73723126dab5a640ac11955ed6da1bf7a91394f5) )
	ROM_LOAD( "264-v2.bin", 0x800000, 0x800000, CRC(c3dc8bf0) SHA1(a105e37262d9500a30fb8a5dac05aa4fab2562a3) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "264-c1.bin", 0x0000000, 0x800000, CRC(4f148fee) SHA1(0821463765fad8fbd0dfbbabb7807337d0333719) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "264-c2.bin", 0x0000001, 0x800000, CRC(7cf5ff72) SHA1(ccb2f94bce943576d224cb326806942426d25584) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "264-c3.bin", 0x1000000, 0x800000, CRC(64d84c98) SHA1(8faf153f465ce6fb7770b27a7ce63caf11dd4086) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "264-c4.bin", 0x1000001, 0x800000, CRC(2f394a95) SHA1(82347e8f2b48b0522d7d91fd3f372d5768934ab2) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "264-c5.bin", 0x2000000, 0x800000, CRC(6b99b978) SHA1(8fd0a60029b41668f9e1e3056edd3c90f62efa83) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "264-c6.bin", 0x2000001, 0x800000, CRC(847d5c7d) SHA1(a2ce03f6302edf81f2645de9ec61df1a281ddd78) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "264-c7.bin", 0x3000000, 0x800000, CRC(231d681e) SHA1(87836e64dc816f8bf1c834641535ea96baacc024) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "264-c8.bin", 0x3000001, 0x800000, CRC(c5edb5c4) SHA1(253378c8739daa5da4edb15eff7050820b2b3755) ) /* Plane 2,3 */
ROM_END

ROM_START( kof2002 ) /* Encrypted Set */ /* MVS AND AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "265-p1.bin", 0x000000, 0x100000, CRC(9ede7323) SHA1(ad9d45498777fda9fa58e75781f48e09aee705a6) )
	ROM_LOAD16_WORD_SWAP( "265-p2.bin", 0x100000, 0x400000, CRC(327266b8) SHA1(98f445cc0a94f8744d74bca71cb420277622b034) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "265-m1.bin", CRC(85aaa632) SHA1(744fba4ca3bc3a5873838af886efb97a8a316104) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "265-v1.bin", 0x000000, 0x800000, CRC(15e8f3f5) SHA1(7c9e6426b9fa6db0158baa17a6485ffce057d889) )
	ROM_LOAD( "265-v2.bin", 0x800000, 0x800000, CRC(da41d6f9) SHA1(a43021f1e58947dcbe3c8ca5283b20b649f0409d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "265-c1.bin", 0x0000000, 0x800000, CRC(2b65a656) SHA1(9c46d8cf5b1ef322db442ac6a9b9406ab49206c5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c2.bin", 0x0000001, 0x800000, CRC(adf18983) SHA1(150cd4a5e51e9df88688469d2ea7675c2cf3658a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c3.bin", 0x1000000, 0x800000, CRC(875e9fd7) SHA1(28f52d56192d48bbc5dc3c97abf456bd34a58cbd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c4.bin", 0x1000001, 0x800000, CRC(2da13947) SHA1(f8d79ec2c236aa3d3648a4f715676899602122c1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c5.bin", 0x2000000, 0x800000, CRC(61bd165d) SHA1(b3424db84bc683d858fb635bc42728f9cdd89caf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c6.bin", 0x2000001, 0x800000, CRC(03fdd1eb) SHA1(6155c7e802062f4eafa27e414c4e73ee59b868bf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c7.bin", 0x3000000, 0x800000, CRC(1a2749d8) SHA1(af7d9ec1d576209826fa568f676bbff92f6d6ddd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c8.bin", 0x3000001, 0x800000, CRC(ab0bb549) SHA1(d23afb60b7f831f7d4a98ad3c4a00ee19877a1ce) ) /* Plane 2,3 */
ROM_END

ROM_START( matrim ) /* Encrypted Set */ /* MVS AND AES VERSION */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "266-p1.bin", 0x000000, 0x100000, CRC(5d4c2dc7) SHA1(8d723b0d28ec344eef26009b361a2b97d300dd51) )
	ROM_LOAD16_WORD_SWAP( "266-p2.bin", 0x100000, 0x400000, CRC(a14b1906) SHA1(1daa14d73512f760ef569b06f9facb279437d1db) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "266-m1.bin", CRC(456c3e6c) SHA1(5a07d0186198a18d2dda1331093cf29b0b9b2984) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "266-v1.bin", 0x000000, 0x800000, CRC(a4f83690) SHA1(200d683d8c30ebc6d0ed331aa5bbba39b4e07160) )
	ROM_LOAD( "266-v2.bin", 0x800000, 0x800000, CRC(d0f69eda) SHA1(9d7e98976ad433ed8a35d7afffa38130444ba7db) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "266-c1.bin", 0x0000000, 0x800000, CRC(505f4e30) SHA1(f22b6f76fc0cad963555dc89d072967c8dc8b79a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "266-c2.bin", 0x0000001, 0x800000, CRC(3cb57482) SHA1(dab15bc24391f9a5173de76af48b612fb9636ccf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "266-c3.bin", 0x1000000, 0x800000, CRC(f1cc6ad0) SHA1(66c1cccc0332ffd2d3064f06330c41f95ca09ced) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "266-c4.bin", 0x1000001, 0x800000, CRC(45b806b7) SHA1(c2bb866fded53d62fad0fc88d89d5e7d4cb1894f) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "266-c5.bin", 0x2000000, 0x800000, CRC(9a15dd6b) SHA1(194a6973a7a9e3847efe1bdbaeaeb16e74aff2dd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "266-c6.bin", 0x2000001, 0x800000, CRC(281cb939) SHA1(bdb7766cfde581ccfaee2be7fe48445f360a2301) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "266-c7.bin", 0x3000000, 0x800000, CRC(4b71f780) SHA1(d5611a6f6b730db58613b48f2b0174661ccfb7bb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "266-c8.bin", 0x3000001, 0x800000, CRC(29873d33) SHA1(dc77f129ed49b8d40d0d4241feef3f6c2f19a987) ) /* Plane 2,3 */
ROM_END

ROM_START( pnyaa ) /* Encrypted Set */ /* MVS VERSION */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "267-p1.bin", 0x000000, 0x100000, CRC(112fe2c0) SHA1(01420e051f0bdbd4f68ce306a3738161b96f8ba8) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "267-m1.bin", CRC(c7853ccd) SHA1(1b7a4c5093cf0fe3861ce44fd1d3b30c71ad0abe) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "267-v1.bin", 0x000000, 0x400000, CRC(e2e8e917) SHA1(7f412d55aebff3d38a225a88c632916295ab0584) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "267-c1.bin", 0x0000000, 0x800000, CRC(5eebee65) SHA1(7eb3eefdeb24e19831d0f51d4ea07a0292c25ab6) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "267-c2.bin", 0x0000001, 0x800000, CRC(2b67187b) SHA1(149c3efd3c444fd0d35a97fa2268102bf76be3ed) ) /* Plane 2,3 */
ROM_END

ROM_START( mslug5 ) /* Encrypted Set */ /* MVS VERSION */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "268-p1cr.bin", 0x000000, 0x400000, CRC(d0466792) SHA1(880819933d997fab398f91061e9dbccb959ae8a1) )
	ROM_LOAD32_WORD_SWAP( "268-p2cr.bin", 0x000002, 0x400000, CRC(fbf6b61e) SHA1(9ec743d5988b5e3183f37f8edf45c72a8c0c893e) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "268-m1.bin", CRC(4a5a6e0e) SHA1(df0f660f2465e1db7be5adfcaf5e88ad61a74a42) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "268-v1c.bin", 0x000000, 0x800000, CRC(ae31d60c) SHA1(c42285cf4e52fea74247860813e826df5aa7600a) )
	ROM_LOAD( "268-v2c.bin", 0x800000, 0x800000, CRC(c40613ed) SHA1(af889570304e2867d7dfea1e94e388c06249fb67) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "268-c1c.bin", 0x0000000, 0x800000, CRC(ab7c389a) SHA1(025a188de589500bf7637fa8e7a37ab24bf4312e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c2c.bin", 0x0000001, 0x800000, CRC(3560881b) SHA1(493d218c92290b4770024d6ee2917c4022753b07) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c3c.bin", 0x1000000, 0x800000, CRC(3af955ea) SHA1(cf36b6ae9b0d12744b17cb7a928399214de894be) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c4c.bin", 0x1000001, 0x800000, CRC(c329c373) SHA1(5073d4079958a0ef5426885af2c9e3178f37d5e0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c5c.bin", 0x2000000, 0x800000, CRC(959c8177) SHA1(889bda7c65d71172e7d89194d1269561888fe789) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c6c.bin", 0x2000001, 0x800000, CRC(010a831b) SHA1(aec140661e3ae35d264df416478ba15188544d91) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c7c.bin", 0x3000000, 0x800000, CRC(6d72a969) SHA1(968dd9a4d1209b770b9b85ea6532fa24d262a262) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c8c.bin", 0x3000001, 0x800000, CRC(551d720e) SHA1(ebf69e334fcaba0fda6fd432fd0970283a365d12) ) /* Plane 2,3 */
ROM_END

ROM_START( mslug5h ) /* Encrypted Set */ /* AES release of the game but is also found in later MVS carts */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "268-p1cr2.bin", 0x000000, 0x400000, CRC(3636690a) SHA1(e0da714b4bdc6efffe1250ded02ebddb3ab6d7b3) )
	ROM_LOAD32_WORD_SWAP( "268-p2cr2.bin", 0x000002, 0x400000, CRC(8dfc47a2) SHA1(27d618cfbd0107a4d2a836797e967b39d2eb4851) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "268-m1h.bin", CRC(4a5a6e0e) SHA1(df0f660f2465e1db7be5adfcaf5e88ad61a74a42) ) /* Same as MVS above, but different label */

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "268-v1c.bin", 0x000000, 0x800000, CRC(ae31d60c) SHA1(c42285cf4e52fea74247860813e826df5aa7600a) )
	ROM_LOAD( "268-v2c.bin", 0x800000, 0x800000, CRC(c40613ed) SHA1(af889570304e2867d7dfea1e94e388c06249fb67) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "268-c1c.bin", 0x0000000, 0x800000, CRC(ab7c389a) SHA1(025a188de589500bf7637fa8e7a37ab24bf4312e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c2c.bin", 0x0000001, 0x800000, CRC(3560881b) SHA1(493d218c92290b4770024d6ee2917c4022753b07) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c3c.bin", 0x1000000, 0x800000, CRC(3af955ea) SHA1(cf36b6ae9b0d12744b17cb7a928399214de894be) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c4c.bin", 0x1000001, 0x800000, CRC(c329c373) SHA1(5073d4079958a0ef5426885af2c9e3178f37d5e0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c5c.bin", 0x2000000, 0x800000, CRC(959c8177) SHA1(889bda7c65d71172e7d89194d1269561888fe789) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c6c.bin", 0x2000001, 0x800000, CRC(010a831b) SHA1(aec140661e3ae35d264df416478ba15188544d91) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c7c.bin", 0x3000000, 0x800000, CRC(6d72a969) SHA1(968dd9a4d1209b770b9b85ea6532fa24d262a262) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c8c.bin", 0x3000001, 0x800000, CRC(551d720e) SHA1(ebf69e334fcaba0fda6fd432fd0970283a365d12) ) /* Plane 2,3 */
ROM_END

ROM_START( svc ) /* Encrypted Set */ /* MVS AND AES VERSION */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "269-p1.bin", 0x000000, 0x400000, CRC(38e2005e) SHA1(1b902905916a30969282f1399a756e32ff069097) )
	ROM_LOAD32_WORD_SWAP( "269-p2.bin", 0x000002, 0x400000, CRC(6d13797c) SHA1(3cb71a95cea6b006b44cac0f547df88aec0007b7) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "269-m1.bin", CRC(f6819d00) SHA1(d3bbe09df502464f104e53501708ac6e2c1832c6) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "269-v1.bin", 0x000000, 0x800000, CRC(c659b34c) SHA1(1931e8111ef43946f68699f8707334c96f753a1e) )
	ROM_LOAD( "269-v2.bin", 0x800000, 0x800000, CRC(dd903835) SHA1(e58d38950a7a8697bb22a1cc7a371ae6664ae8f9) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "269-c1r.bin", 0x0000000, 0x800000, CRC(887b4068) SHA1(227cdcf7a10a415f1e3afe7ae97acc9afc2cc8e1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "269-c2r.bin", 0x0000001, 0x800000, CRC(4e8903e4) SHA1(31daaa4fd6c23e8f0a8428931c513d97d2eee1bd) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "269-c3r.bin", 0x1000000, 0x800000, CRC(7d9c55b0) SHA1(1f94a948b3e3c31b3ff05518ef525031a3cb2c62) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "269-c4r.bin", 0x1000001, 0x800000, CRC(8acb5bb6) SHA1(2c27d6e309646d7b84da85f78c06e4aaa74e844b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "269-c5r.bin", 0x2000000, 0x800000, CRC(097a4157) SHA1(54d839f55d27f68c704a94ea3c63c644ffc22ca4) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "269-c6r.bin", 0x2000001, 0x800000, CRC(e19df344) SHA1(20448add53ab25dd3a8f0b681131ad3b9c68acc9) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "269-c7r.bin", 0x3000000, 0x800000, CRC(d8f0340b) SHA1(43114af7557361a8903bb8cf8553f602946a9220) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "269-c8r.bin", 0x3000001, 0x800000, CRC(2570b71b) SHA1(99266e1c2ffcf324793fb5c55325fbc7e6265ac0) ) /* Plane 2,3 */
ROM_END

ROM_START( samsho5 ) /* Encrypted Set */ /* MVS VERSION */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "270-p1.bin", 0x000000, 0x400000, CRC(4a2a09e6) SHA1(2644de02cdab8ccc605488a7c76b8c9cd1d5bcb9) )
	ROM_LOAD16_WORD_SWAP( "270-p2.bin", 0x400000, 0x400000, CRC(e0c74c85) SHA1(df24a4ee76438e40c2f04a714175a7f85cacdfe0) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "270-m1.bin", CRC(49c9901a) SHA1(2623e9765a0eba58fee2de72851e9dc502344a3d) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "270-v1.bin", 0x000000, 0x800000, CRC(62e434eb) SHA1(1985f5e88f8e866f9683b6cea901aa28c04b80bf) )
	ROM_LOAD( "270-v2.bin", 0x800000, 0x800000, CRC(180f3c9a) SHA1(6d7dc2605ead6e78704efa127e7e0dfe621e2c54) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "270-c1.bin", 0x0000000, 0x800000, CRC(14ffffac) SHA1(2ccebfdd0c7907679ae95bf6eca85b8d322441e2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "270-c2.bin", 0x0000001, 0x800000, CRC(401f7299) SHA1(94e48cdf1682b1250f53c59f3f71d995e928d17b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "270-c3.bin", 0x1000000, 0x800000, CRC(838f0260) SHA1(d5c8d3c6e7221d04e0b20882a847752e5ba95635) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "270-c4.bin", 0x1000001, 0x800000, CRC(041560a5) SHA1(d165e533699f15b1e079c82f97db3542b3a7dd66) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "270-c5.bin", 0x2000000, 0x800000, CRC(bd30b52d) SHA1(9f8282e684415b4045218cf764ef7d75a70e3240) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "270-c6.bin", 0x2000001, 0x800000, CRC(86a69c70) SHA1(526732cdb408cf680af9da39057bce6a4dfb5e13) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "270-c7.bin", 0x3000000, 0x800000, CRC(d28fbc3c) SHA1(a82a6ba6760fad14d9309f9147cb7d80bd6f70fc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "270-c8.bin", 0x3000001, 0x800000, CRC(02c530a6) SHA1(7a3fafa6075506c6ef78cc4ec2cb72118ec83cb9) ) /* Plane 2,3 */
ROM_END

ROM_START( samsho5h ) /* Encrypted Set, Alternate Set */ /* AES VERSION */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "270-p1c.bin", 0x000000, 0x400000, CRC(bf956089) SHA1(c538289069bf338b9fa7ecc5c9143763dbb776a8) )
	ROM_LOAD16_WORD_SWAP( "270-p2c.bin", 0x400000, 0x400000, CRC(943a6b1d) SHA1(12bd02fc197456da6ee86f066086094cef0f4bf9) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "270-m1.bin", CRC(49c9901a) SHA1(2623e9765a0eba58fee2de72851e9dc502344a3d) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "270-v1.bin", 0x000000, 0x800000, CRC(62e434eb) SHA1(1985f5e88f8e866f9683b6cea901aa28c04b80bf) )
	ROM_LOAD( "270-v2.bin", 0x800000, 0x800000, CRC(180f3c9a) SHA1(6d7dc2605ead6e78704efa127e7e0dfe621e2c54) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "270-c1.bin", 0x0000000, 0x800000, CRC(14ffffac) SHA1(2ccebfdd0c7907679ae95bf6eca85b8d322441e2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "270-c2.bin", 0x0000001, 0x800000, CRC(401f7299) SHA1(94e48cdf1682b1250f53c59f3f71d995e928d17b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "270-c3.bin", 0x1000000, 0x800000, CRC(838f0260) SHA1(d5c8d3c6e7221d04e0b20882a847752e5ba95635) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "270-c4.bin", 0x1000001, 0x800000, CRC(041560a5) SHA1(d165e533699f15b1e079c82f97db3542b3a7dd66) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "270-c5.bin", 0x2000000, 0x800000, CRC(bd30b52d) SHA1(9f8282e684415b4045218cf764ef7d75a70e3240) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "270-c6.bin", 0x2000001, 0x800000, CRC(86a69c70) SHA1(526732cdb408cf680af9da39057bce6a4dfb5e13) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "270-c7.bin", 0x3000000, 0x800000, CRC(d28fbc3c) SHA1(a82a6ba6760fad14d9309f9147cb7d80bd6f70fc) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "270-c8.bin", 0x3000001, 0x800000, CRC(02c530a6) SHA1(7a3fafa6075506c6ef78cc4ec2cb72118ec83cb9) ) /* Plane 2,3 */
ROM_END

ROM_START( kof2003 ) /* Encrypted Code + Sound + GFX Roms */ /* MVS VERSION */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "271-p1c.bin", 0x000000, 0x400000, CRC(530ecc14) SHA1(812cf7e9902af3f5e9e330b7c05c2171b139ad2b) )
	ROM_LOAD32_WORD_SWAP( "271-p2c.bin", 0x000002, 0x400000, CRC(fd568da9) SHA1(46364906a1e81dc251117e91a1a7b43af1373ada) )
	ROM_LOAD16_WORD_SWAP( "271-p3c.bin", 0x800000, 0x100000, CRC(aec5b4a9) SHA1(74087f785590eda5898ce146029818f86ced42b6) ) // Encrypted

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "271-m1c.bin", CRC(f5515629) SHA1(7516bf1b0207a3c8d41dc30c478f8d8b1f71304b) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1c.bin", 0x000000, 0x800000, CRC(ffa3f8c7) SHA1(7cf4a933973ca23b7f87c81151d8659e6ec4bd20) )
	ROM_LOAD( "271-v2c.bin", 0x800000, 0x800000, CRC(5382c7d1) SHA1(1bf999705eda80ba1e7b0d6bdd010d9bfb18bd76) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "271-c1c.bin", 0x0000000, 0x800000, CRC(b1dc25d0) SHA1(50adc3c60d5b4b3abd10a49db2267306c6dbd772) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c2c.bin", 0x0000001, 0x800000, CRC(d5362437) SHA1(66db36522dc09106388c707252df9fe1c88b4856) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c3c.bin", 0x1000000, 0x800000, CRC(0a1fbeab) SHA1(9fe30d36ba98d00fda010832ff2f27783dd577c1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c4c.bin", 0x1000001, 0x800000, CRC(87b19a0c) SHA1(b72a8e7d9124ce859b5149bb4381ba481c161ea5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c5c.bin", 0x2000000, 0x800000, CRC(704ea371) SHA1(e75b80422f0d72eac826f8ffadf79efeccaab124) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c6c.bin", 0x2000001, 0x800000, CRC(20a1164c) SHA1(c9843b37612a16fc95f6851793b1cfb5d49d811d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c7c.bin", 0x3000000, 0x800000, CRC(189aba7f) SHA1(7152195a57ad36b28290810fe87ed8c206262ba9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c8c.bin", 0x3000001, 0x800000, CRC(20ec4fdc) SHA1(deb5f7ec5a090e419b9d1a6a74877bee081198e2) ) /* Plane 2,3 */
ROM_END

ROM_START( kof2003h ) /* Encrypted Code + Sound + GFX Roms */ /* AES VERSION */
	/* All chip labels for this set are correct */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "271-p1k.bin", 0x000000, 0x400000, CRC(d0d0ae3e) SHA1(538d054ac50c91694fbbfefcce548b063713e14e) )
	ROM_LOAD32_WORD_SWAP( "271-p2k.bin", 0x000002, 0x400000, CRC(fb3f79d9) SHA1(f253d10e732d6e23ae82d74ac9269d21f69ddb4d) )
	ROM_LOAD16_WORD_SWAP( "271-p3k.bin", 0x800000, 0x100000, CRC(232702ad) SHA1(6045046027dac1cbd4cbd14b5c1ece522bc6197f) ) // Encrypted

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "271-m1k.bin", CRC(48d9affe) SHA1(68f01560b91bbada39001ce01bdeeed5c9bb29f2) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1c.bin", 0x000000, 0x800000, CRC(ffa3f8c7) SHA1(7cf4a933973ca23b7f87c81151d8659e6ec4bd20) )
	ROM_LOAD( "271-v2c.bin", 0x800000, 0x800000, CRC(5382c7d1) SHA1(1bf999705eda80ba1e7b0d6bdd010d9bfb18bd76) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "271-c1k.bin", 0x0000000, 0x800000, CRC(efb9dd24) SHA1(1c6fe10fdbfc3306c3b7321c731f28ffdbfb15b8) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c2k.bin", 0x0000001, 0x800000, CRC(3fb90447) SHA1(04d196de7c54c77bc75eba56d3060d46efc2d406) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c3k.bin", 0x1000000, 0x800000, CRC(27950f28) SHA1(924f4de61c86b9efde6f1104b986886f1117055d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c4k.bin", 0x1000001, 0x800000, CRC(735177f8) SHA1(c95da1bc256995a7f44c9cc3312879ab6cbc15d6) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c5k.bin", 0x2000000, 0x800000, CRC(a51b7c0f) SHA1(53dcf692b35b8d32abe5962ac799b8d641f04710) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c6k.bin", 0x2000001, 0x800000, CRC(d5cae4e0) SHA1(248cd9eaac7a04d6b5d80c7534de90b057d566d7) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c7k.bin", 0x3000000, 0x800000, CRC(e65ae2d0) SHA1(39744e10697d7ac539ecfcfa597e75597f321955) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c8k.bin", 0x3000001, 0x800000, CRC(312f528c) SHA1(b4ad75f54f730ada6cb00112b74022250f055725) ) /* Plane 2,3 */
ROM_END

ROM_START( samsh5sp ) /* Encrypted Set */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "272-p1.bin", 0x000000, 0x400000, CRC(fb7a6bba) SHA1(f68c527208d8a55ca44b0caaa8ab66b3a0ffdfe5) )
	ROM_LOAD16_WORD_SWAP( "272-p2.bin", 0x400000, 0x400000, CRC(63492ea6) SHA1(6ba946acb62c63ed61a42fe72b7fff3828883bcc) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_ENCRYPTED_512K( "272-m1.bin", CRC(adeebf40) SHA1(8cbd63dda3fff4de38060405bf70cd9308c9e66e) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "272-v1.bin", 0x000000, 0x800000, CRC(76a94127) SHA1(c3affd7ff1eb02345cfb755962ec173a8ec34acd) )
	ROM_LOAD( "272-v2.bin", 0x800000, 0x800000, CRC(4ba507f1) SHA1(728d139da3fe8a391fd8be4d24bb7fdd4bf9548a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "272-c1.bin", 0x0000000, 0x800000, CRC(4f97661a) SHA1(87f1721bae5ef16bc23c06b05e64686c396413df) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c2.bin", 0x0000001, 0x800000, CRC(a3afda4f) SHA1(86b475fce0bc0aa04d34e31324e8c7c7c847df19) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c3.bin", 0x1000000, 0x800000, CRC(8c3c7502) SHA1(6639020a8860d2400308e110d7277cbaf6eccc2a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c4.bin", 0x1000001, 0x800000, CRC(32d5e2e2) SHA1(2b5612017152afd7433aaf99951a084ef5ad6bf0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c5.bin", 0x2000000, 0x800000, CRC(6ce085bc) SHA1(0432b04a2265c649bba1bbd934dfb425c5d80fb1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c6.bin", 0x2000001, 0x800000, CRC(05c8dc8e) SHA1(da45c222893f25495a66bdb302f9b0b1de3c8ae0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c7.bin", 0x3000000, 0x800000, CRC(1417b742) SHA1(dfe35eb4bcd022d2f2dc544ccbbb77078f08c0aa) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c8.bin", 0x3000001, 0x800000, CRC(d49773cd) SHA1(cd8cf3b762d381c1f8f12919579c84a7ef7efb3f) ) /* Plane 2,3 */
ROM_END

ROM_START( samsh5sph ) /* Encrypted Set */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "272-p1c.bin", 0x000000, 0x400000, CRC(9291794d) SHA1(66588ff9b00ffad6508b03423548984e28a3209d) )
	ROM_LOAD16_WORD_SWAP( "272-p2c.bin", 0x400000, 0x400000, CRC(fa1a7dd8) SHA1(62443dad76d6c1e18f515d7d4ef8e1295a4b7f1d) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "272-m1.bin", CRC(adeebf40) SHA1(8cbd63dda3fff4de38060405bf70cd9308c9e66e) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "272-v1.bin", 0x000000, 0x800000, CRC(76a94127) SHA1(c3affd7ff1eb02345cfb755962ec173a8ec34acd) )
	ROM_LOAD( "272-v2.bin", 0x800000, 0x800000, CRC(4ba507f1) SHA1(728d139da3fe8a391fd8be4d24bb7fdd4bf9548a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "272-c1.bin", 0x0000000, 0x800000, CRC(4f97661a) SHA1(87f1721bae5ef16bc23c06b05e64686c396413df) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c2.bin", 0x0000001, 0x800000, CRC(a3afda4f) SHA1(86b475fce0bc0aa04d34e31324e8c7c7c847df19) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c3.bin", 0x1000000, 0x800000, CRC(8c3c7502) SHA1(6639020a8860d2400308e110d7277cbaf6eccc2a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c4.bin", 0x1000001, 0x800000, CRC(32d5e2e2) SHA1(2b5612017152afd7433aaf99951a084ef5ad6bf0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c5.bin", 0x2000000, 0x800000, CRC(6ce085bc) SHA1(0432b04a2265c649bba1bbd934dfb425c5d80fb1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c6.bin", 0x2000001, 0x800000, CRC(05c8dc8e) SHA1(da45c222893f25495a66bdb302f9b0b1de3c8ae0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c7.bin", 0x3000000, 0x800000, CRC(1417b742) SHA1(dfe35eb4bcd022d2f2dc544ccbbb77078f08c0aa) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c8.bin", 0x3000001, 0x800000, CRC(d49773cd) SHA1(cd8cf3b762d381c1f8f12919579c84a7ef7efb3f) ) /* Plane 2,3 */
ROM_END

ROM_START( samsh5spn ) /* Encrypted Set */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "272-p1ca.bin", 0x000000, 0x400000, CRC(c30a08dd) SHA1(66864954017c841d7ca8490112c3aa7a71a4da70) )
	ROM_LOAD16_WORD_SWAP( "272-p2ca.bin", 0x400000, 0x400000, CRC(bd64a518) SHA1(aa259a168930f106377d680db444535411b3bce0) )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "272-m1.bin", CRC(adeebf40) SHA1(8cbd63dda3fff4de38060405bf70cd9308c9e66e) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "272-v1.bin", 0x000000, 0x800000, CRC(76a94127) SHA1(c3affd7ff1eb02345cfb755962ec173a8ec34acd) )
	ROM_LOAD( "272-v2.bin", 0x800000, 0x800000, CRC(4ba507f1) SHA1(728d139da3fe8a391fd8be4d24bb7fdd4bf9548a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "272-c1.bin", 0x0000000, 0x800000, CRC(4f97661a) SHA1(87f1721bae5ef16bc23c06b05e64686c396413df) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c2.bin", 0x0000001, 0x800000, CRC(a3afda4f) SHA1(86b475fce0bc0aa04d34e31324e8c7c7c847df19) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c3.bin", 0x1000000, 0x800000, CRC(8c3c7502) SHA1(6639020a8860d2400308e110d7277cbaf6eccc2a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c4.bin", 0x1000001, 0x800000, CRC(32d5e2e2) SHA1(2b5612017152afd7433aaf99951a084ef5ad6bf0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c5.bin", 0x2000000, 0x800000, CRC(6ce085bc) SHA1(0432b04a2265c649bba1bbd934dfb425c5d80fb1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c6.bin", 0x2000001, 0x800000, CRC(05c8dc8e) SHA1(da45c222893f25495a66bdb302f9b0b1de3c8ae0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "272-c7.bin", 0x3000000, 0x800000, CRC(1417b742) SHA1(dfe35eb4bcd022d2f2dc544ccbbb77078f08c0aa) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "272-c8.bin", 0x3000001, 0x800000, CRC(d49773cd) SHA1(cd8cf3b762d381c1f8f12919579c84a7ef7efb3f) ) /* Plane 2,3 */
ROM_END

ROM_START( jockeygp )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "jgp-p1.bin", 0x000000, 0x100000, CRC(2fb7f388) SHA1(e3c9b03944b4c10cf5081caaf9c8be1f08c06493) )
	ROM_FILL( 0x100000, 0x100000, 0xff )

	ROM_Y_ZOOM

	/* The Encrypted Boards do not have an s1 rom, data for it comes from the Cx ROMs */
	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "jgp-m1.bin", CRC(d163c690) SHA1(1dfd04d20c5985037f07cd01000d0b04f3a8f4f4) )

	ROM_REGION( 0x0200000, "ymsnd", 0 )
	ROM_LOAD( "jgp-v1.bin", 0x000000, 0x200000, CRC(443eadba) SHA1(3def3c22f0e276bc4c2fc7ff70ce473c08b0d2df) )

	NO_DELTAT_REGION

	ROM_REGION( 0x1000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "jgp-c1.bin", 0x0000000, 0x800000, CRC(a9acbf18) SHA1(d55122c70cbe78c2679598dc07863e1d1d1a31df) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "jgp-c2.bin", 0x0000001, 0x800000, CRC(6289eef9) SHA1(a2ede77bb2468a2e1486d74745a22a5451026039) ) /* Plane 2,3 */
ROM_END

ROM_START( vliner )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vl-p1.bin", 0x000000, 0x080000, CRC(72a2c043) SHA1(b34bcc10ff33e4465126a6865fe8bf6b6a3d6cee) )

	NEO_SFIX_128K( "vl-s1.bin", CRC(972d8c31) SHA1(41f09ef28a3791668ea304c74b8b06c117a50e9a) )

	NEO_BIOS_AUDIO_64K( "vl-m1.bin", CRC(9b92b7d1) SHA1(2c9b777feb9a8e43fa1bd942aba5afe3b5427d94) )

	ROM_REGION( 0x200000, "ymsnd", ROMREGION_ERASE00 )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "vl-c1.bin", 0x000000, 0x80000, CRC(5118f7c0) SHA1(b6fb6e9cbb660580d98e00780ebf248c0995145a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "vl-c2.bin", 0x000001, 0x80000, CRC(efe9b33e) SHA1(910c651aadce9bf59e51c338ceef62287756d2e8) ) /* Plane 2,3 */
ROM_END

ROM_START( vlinero )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vl-p1_54.bin", 0x000000, 0x080000, CRC(172efc18) SHA1(8ca739f8780a9e6fa19ac2c3e931d75871603f58) )

	NEO_SFIX_128K( "vl-s1.bin", CRC(972d8c31) SHA1(41f09ef28a3791668ea304c74b8b06c117a50e9a) )

	NEO_BIOS_AUDIO_64K( "vl-m1.bin", CRC(9b92b7d1) SHA1(2c9b777feb9a8e43fa1bd942aba5afe3b5427d94) )

	ROM_REGION( 0x200000, "ymsnd", ROMREGION_ERASE00 )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "vl-c1.bin", 0x000000, 0x80000, CRC(5118f7c0) SHA1(b6fb6e9cbb660580d98e00780ebf248c0995145a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "vl-c2.bin", 0x000001, 0x80000, CRC(efe9b33e) SHA1(910c651aadce9bf59e51c338ceef62287756d2e8) ) /* Plane 2,3 */
ROM_END


	/* Jamma PCB sets */


/******************************************************************************
    The following Jamma PCB boards are known:

    SNK vs. CAPCOM SVC CHAOS (svcpcb)
    NEO-MVH MVO 2003.6.5

    SNK vs. CAPCOM SVC CHAOS (svcpcba)
    NEO-MVH MVOB 2003.7.9

    Metal Slug 5
    NEO-MVH MVOBR 2003.8.4

    The King of Fighters 2003
    NEO-MVH MVOC 2003.11.3
******************************************************************************/


/* complete redump required */
ROM_START( ms5pcb ) /* Encrypted Set */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "268-p1r.bin", 0x000000, 0x400000, NO_DUMP )
	ROM_LOAD32_WORD_SWAP( "268-p2r.bin", 0x000002, 0x400000, NO_DUMP )

	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x20000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "268-m1.bin", 0x00000, 0x80000, CRC(4a5a6e0e) SHA1(df0f660f2465e1db7be5adfcaf5e88ad61a74a42) )
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	/* Encrypted */

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "268-v1.bin", 0x000000, 0x1000000, CRC(8458afe5) SHA1(62b4c6e7db763e9ff2697bbcdb43dc5a56b48c68) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "268-c1.bin", 0x0000000, 0x1000000, CRC(802042e8) SHA1(ff028b65f60f0b51b255a380cc47ec19fdc0c0cf) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "268-c2.bin", 0x0000002, 0x1000000, CRC(3b89fb9f) SHA1(cbc0729aae961f683b105ec3e1cda58b3f985abc) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "268-c3.bin", 0x2000000, 0x1000000, CRC(0f3f59e3) SHA1(8cc751dc7d4e94864a9ce3346f23b8f011082fcc) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "268-c4.bin", 0x2000002, 0x1000000, CRC(3ad8435a) SHA1(b333c8993c9b4c4ea59450ad0a3560e0b28056bc) ) /* Plane 2,3 */
ROM_END

ROM_START( svcpcb ) /* Encrypted Set, JAMMA PCB */
	ROM_REGION( 0x2000000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "269-p1.bin", 0x000000, 0x2000000, CRC(432cfdfc) SHA1(19b40d32188a8bace6d2d570c6cf3d2f1e31e379) )

	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "269-m1.bin", 0x00000, 0x80000, CRC(f6819d00) SHA1(d3bbe09df502464f104e53501708ac6e2c1832c6) )
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "269-v1.bin", 0x000000, 0x800000, CRC(c659b34c) SHA1(1931e8111ef43946f68699f8707334c96f753a1e) )
	ROM_LOAD( "269-v2.bin", 0x800000, 0x800000, CRC(dd903835) SHA1(e58d38950a7a8697bb22a1cc7a371ae6664ae8f9) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD( "269-c1.bin", 0x0000000, 0x2000000, CRC(1b608f9c) SHA1(4e70ad182da2ca18815bd3936efb04a06ebce01e) ) /* Plane 0,1 */
	ROM_LOAD( "269-c2.bin", 0x2000000, 0x2000000, CRC(5a95f294) SHA1(6123cc7b20b494076185d27c2ffea910e124b195) ) /* Plane 0,1 */
ROM_END

/* alt PCB version, this one has the same program roms as the MVS set, and different GFX / Sound rom arrangements */
ROM_START( svcpcba ) /* Encrypted Set, JAMMA PCB */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "269-p1a.bin", 0x000000, 0x400000, CRC(38e2005e) SHA1(1b902905916a30969282f1399a756e32ff069097)  )
	ROM_LOAD32_WORD_SWAP( "269-p2a.bin", 0x000002, 0x400000, CRC(6d13797c) SHA1(3cb71a95cea6b006b44cac0f547df88aec0007b7)  )

	ROM_REGION( 0x80000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	/* this contains both an ASIA and JAPAN bios, HARDDIP3 on the PCB selects which to use */
	ROM_LOAD16_WORD_SWAP( "sp-4x.sp1", 0x00000, 0x80000, CRC(b4590283) SHA1(47047ed5b6062babc0a0bebcc30e4b3f021e115a) )

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "269-m1.bin", 0x00000, 0x80000, CRC(f6819d00) SHA1(d3bbe09df502464f104e53501708ac6e2c1832c6) )
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "269-v1a.bin", 0x000000, 0x1000000, CRC(a6af4753) SHA1(ec4f61a526b707a7faec4653b773beb3bf3a17ba) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "269-c1a.bin", 0x0000000, 0x1000000, CRC(e64d2b0c) SHA1(0714198c400e5c273181e4c6f906b49e35fef75d) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "269-c2a.bin", 0x0000002, 0x1000000, CRC(249089c2) SHA1(1c0ca19e330efe1a74b2d35a1a9a8d61481e16a9) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "269-c3a.bin", 0x2000000, 0x1000000, CRC(d32f2fab) SHA1(273d58cb3c9075075b1ca39a3b247a2cd545fbe7) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "269-c4a.bin", 0x2000002, 0x1000000, CRC(bf77e878) SHA1(e6e76f8eed0d04ee9ad39bf38ce305930b10e2c1) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k3pcb ) /* Encrypted Set, JAMMA PCB */
	ROM_REGION( 0x900000, "maincpu", 0 )
	ROM_LOAD32_WORD_SWAP( "271-p1.bin", 0x000000, 0x400000, CRC(b9da070c) SHA1(1a26325af142a4dd221c336061761468598c4634) )
	ROM_LOAD32_WORD_SWAP( "271-p2.bin", 0x000002, 0x400000, CRC(da3118c4) SHA1(582e4f44f03276adecb7b2848d3b96bf6da57f1e) )
	ROM_LOAD16_WORD_SWAP( "271-p3.bin", 0x800000, 0x100000, CRC(5cefd0d2) SHA1(cddc3164629fed4b6f715e12b109ad35d1009355) )

	ROM_REGION( 0x100000, "fixed", 0 ) /* larger char set */
	ROM_FILL( 0x000000, 0x100000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_LOAD16_WORD_SWAP( "271-bios.bin", 0x00000, 0x080000, CRC(148dd727) SHA1(2cf592a16c7157de02a989675d47965f2b3a44dd) ) // encrypted

	/* Encrypted */
	ROM_REGION( 0x80000, "audiocrypt", 0 )
	ROM_LOAD( "271-m1.bin", 0x00000, 0x80000, CRC(d6bcf2bc) SHA1(df78bc95990eb8e8f3638dde6e1876354df7fe84) )
	ROM_REGION( 0x90000, "audiocpu", ROMREGION_ERASEFF )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1.bin", 0x000000, 0x1000000, CRC(1d96154b) SHA1(1d4e262b0d30cee79a4edc83bb9706023c736668) )

	NO_DELTAT_REGION

	ROM_REGION( 0x6000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD32_WORD( "271-c1.bin", 0x0000000, 0x1000000, CRC(f5ebb327) SHA1(e4f799a54b09adcca13b1b0cf95971a1f4291b61) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c2.bin", 0x0000002, 0x1000000, CRC(2be21620) SHA1(872c658f53bbc558e90f18d5db9cbaa82e748a6a) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "271-c3.bin", 0x2000000, 0x1000000, CRC(ddded4ff) SHA1(ff7b356125bc9e6637b164f5e81b13eabeb8d804) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c4.bin", 0x2000002, 0x1000000, CRC(d85521e6) SHA1(62278fa8690972ed32aca07a4f7f97e7203d9f3a) ) /* Plane 2,3 */
	ROM_LOAD32_WORD( "271-c5.bin", 0x4000000, 0x1000000, CRC(18aa3540) SHA1(15e0a8c4e0927b1f7eb9bee8f532acea6818d5eb) ) /* Plane 0,1 */
	ROM_LOAD32_WORD( "271-c6.bin", 0x4000002, 0x1000000, CRC(1c40de87) SHA1(8d6425aed43ff6a96c88194e203df6a783286373) ) /* Plane 2,3 */
ROM_END


	/* Bootleg sets */


/*
    Most bootleg sets are incomplete (missing 'C', 'V' and 'M' roms).
    Verification required.

*/


/* Zintrick bootleg */

/* This Zintrick set appears to be a bootleg made from the CD version, not a genuine
   prototype the code is based on that of the NeoCD version with some minor patches,
   for example the ADK SAMPLE TEST text that appears on the screen is actually a hacked
   PROG LOAD ERROR message. The set is supported in order to distinguish the hacks from
   a real prototype should one turn up. */

ROM_START( zintrckb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "zin-p1.bin", 0x000000, 0x100000, CRC(06c8fca7) SHA1(b7bf38965c3d0db4d7a9684d14cac94a45b4a45b))

	NEO_SFIX_128K( "zin-s1.bin", CRC(a7ab0e81) SHA1(f0649819b96cea79b05411e0b15c8edc677d79ba) )

	NEO_BIOS_AUDIO_128K( "zin-m1.bin", CRC(fd9627ca) SHA1(b640c1f1ff466f734bb1cb5d7b589cb7e8a55346) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "zin-v1.bin", 0x000000, 0x200000, CRC(c09f74f1) SHA1(d0b56a780a6eba85ff092240b1f1cc6718f17c21) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "zin-c1.bin", 0x000000, 0x200000, CRC(76aee189) SHA1(ad6929804c5b9a59aa609e6baebc6aa37e858a47) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "zin-c2.bin", 0x000001, 0x200000, CRC(844ed4b3) SHA1(fb7cd057bdc6cbe8b78097dd124118bae7402256) ) /* Plane 2,3 */
ROM_END

/* The King of Fighters '97 bootlegs */

ROM_START( kof97pls )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "kf97-p1p.bin", 0x000000, 0x100000, CRC(c01fda46) SHA1(bc6402f5082efc80a8936364c657165f19b49415) )
	ROM_LOAD16_WORD_SWAP( "kf97-p2p.bin", 0x100000, 0x400000, CRC(5502b020) SHA1(37c48198d8b3798910a44075782cd1a20b687b4a) )

	NEO_SFIX_128K( "kf97-s1p.bin", CRC(73254270) SHA1(8d06305f9d8890da1327356272b88bdd0dc089f5) )

	NEO_BIOS_AUDIO_128K( "232-m1.bin", CRC(45348747) SHA1(ed77cbae2b208d1177a9f5f6e8cd57070e90b65b) )

	ROM_REGION( 0xc00000, "ymsnd", 0 )
	ROM_LOAD( "232-v1.bin", 0x000000, 0x400000, CRC(22a2b5b5) SHA1(ebdbc977332e6d93e266755000b43857e0082965) )
	ROM_LOAD( "232-v2.bin", 0x400000, 0x400000, CRC(2304e744) SHA1(98d283e2bcc9291a53f52afd35ef76dfb0828432) )
	ROM_LOAD( "232-v3.bin", 0x800000, 0x400000, CRC(759eb954) SHA1(54e77c4e9e6b89458e59824e478ddc33a9c72655) )

	NO_DELTAT_REGION

	ROM_REGION( 0x2800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "232-c1.bin", 0x0000000, 0x800000, CRC(5f8bf0a1) SHA1(e8b63bbc814de171fd18c5864a7fc639970c1ecf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c2.bin", 0x0000001, 0x800000, CRC(e4d45c81) SHA1(fdb2b9326362e27b1c7a5beb977e0bc537488186) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "232-c3.bin", 0x1000000, 0x800000, CRC(581d6618) SHA1(14d3124a08ded59f86932c6b28e1a4e48c564ccd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c4.bin", 0x1000001, 0x800000, CRC(49bb1e68) SHA1(f769c1bd1b019521111ff3f0d22c63cb1f2640ef) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "232-c5.bin", 0x2000000, 0x400000, CRC(34fc4e51) SHA1(b39c65f27873f71a6f5a5d1d04e5435f874472ee) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "232-c6.bin", 0x2000001, 0x400000, CRC(4ff4d47b) SHA1(4d5689ede24a5fe4330bd85d4d3f4eb2795308bb) ) /* Plane 2,3 */
ROM_END

ROM_START( kog )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "5232-p1.bin", 0x000000, 0x200000, CRC(d2413ec6) SHA1(c0bf409d1e714cba5fdc6f79e4c2aec805316634) )
	ROM_LOAD16_WORD_SWAP( "232-p2.bin",  0x200000, 0x400000, CRC(158b23f6) SHA1(9744620a70513490aaf9c5eda33e5ec31222be19) )

	NEO_SFIX_128K( "5232-s1.bin", CRC(0bef69da) SHA1(80918586e694dce35c4dba796eb18abf6a070ebb) )

	NEO_BIOS_AUDIO_128K( "232-m1.bin", CRC(45348747) SHA1(ed77cbae2b208d1177a9f5f6e8cd57070e90b65b) )

	ROM_REGION( 0xc00000, "ymsnd", 0 )
	ROM_LOAD( "232-v1.bin", 0x000000, 0x400000, CRC(22a2b5b5) SHA1(ebdbc977332e6d93e266755000b43857e0082965) )
	ROM_LOAD( "232-v2.bin", 0x400000, 0x400000, CRC(2304e744) SHA1(98d283e2bcc9291a53f52afd35ef76dfb0828432) )
	ROM_LOAD( "232-v3.bin", 0x800000, 0x400000, CRC(759eb954) SHA1(54e77c4e9e6b89458e59824e478ddc33a9c72655) )

	ROM_REGION( 0x2800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "5232-c1a.bin", 0x0000000, 0x800000, CRC(4eab9b0a) SHA1(a6f6b755215a3f41474e0a76b5463303a522c2d3) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5232-c2a.bin", 0x0000001, 0x800000, CRC(697f8fd0) SHA1(5784464c2357ccef8e6e79b6298843fc3d13b39c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5232-c1b.bin", 0x1000000, 0x800000, CRC(1143fdf3) SHA1(9dc5fe9a3b7599380db62095880e2d6f237a41bd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5232-c2b.bin", 0x1000001, 0x800000, CRC(ea82cf8f) SHA1(3d9ab64b69cecd6b3950839ac2c6d151ad66dcf8) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5232-c3.bin",  0x2000000, 0x400000, CRC(abd1be07) SHA1(857eb68bbee4538770bbfa77aaa540d61ab0abcd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5232-c4.bin",  0x2000001, 0x400000, CRC(d2bd967b) SHA1(c494e0a98e127d37ca360a28accc167fa50fb626) ) /* Plane 2,3 */
ROM_END

/* Shock Troopers - 2nd Squad bootleg */

ROM_START( lans2004 )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "lnsq-p1.bin",  0x000000, 0x200000, CRC(b40a879a) SHA1(379f3d44b0bae430b32157fd5f4018d77b243c58) )
	ROM_LOAD16_WORD_SWAP( "lnsq-p21.bin", 0x200000, 0x200000, CRC(ecdb2d42) SHA1(0d930cd369dfbcab7778b144355e5f70874aa324) )
	ROM_LOAD16_WORD_SWAP( "lnsq-p22.bin", 0x400000, 0x200000, CRC(fac5e2e7) SHA1(5cce7226c137da80c969df00e1cda41ef9c5082c) )

	NEO_SFIX_128K( "lnsq-s1.bin", CRC(39e82897) SHA1(24a8c94dd7e70ecde8f90ea17f75b6b5d065704f) )

	NEO_BIOS_AUDIO_128K( "246-m1.bin", CRC(d0604ad1) SHA1(fae3cd52a177eadd5f5775ace957cc0f8301e65d) )

	ROM_REGION( 0xA00000, "ymsnd", 0 )
	ROM_LOAD( "lnsq-v1.bin", 0x000000, 0x400000, CRC(4408ebc3) SHA1(e3f4d8a7e243a8cf48e97d91bbfec7829c0d9404) )
	ROM_LOAD( "lnsq-v2.bin", 0x400000, 0x400000, CRC(3d953975) SHA1(6a4ab02ab3d4416a65343cf16815007cb273f19b) )
	ROM_LOAD( "lnsq-v3.bin", 0x800000, 0x200000, CRC(437d1d8e) SHA1(95e015c21707b53ed7223eaa19f6cdcfb4d94f0c) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "lnsq-c1.bin", 0x0000000, 0x800000, CRC(b83de59f) SHA1(8cc060f9a57ab7d4238543b0bce5f5cd1d271d4f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "lnsq-c2.bin", 0x0000001, 0x800000, CRC(e08969fd) SHA1(c192639d023cdad64a8f53dbcda02aa8cfb4168e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "lnsq-c3.bin", 0x1000000, 0x800000, CRC(013f2cda) SHA1(6261111ce69dc23fbf97241131e5a6a49355d18c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "lnsq-c4.bin", 0x1000001, 0x800000, CRC(d8c3a758) SHA1(d19ca3be06f9fb0cb1933b1eb3da318524c3145d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "lnsq-c5.bin", 0x2000000, 0x800000, CRC(75500b82) SHA1(06d2afe94ea3eb3c4e523f593b8e709dd7c284a3) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "lnsq-c6.bin", 0x2000001, 0x800000, CRC(670ac13b) SHA1(f448a144caae51b69ea19e1f43940db135d1164a) ) /* Plane 2,3 */
ROM_END

/* Garou - Mark of the Wolves bootleg */

ROM_START( garoubl ) /* bootleg of garoup */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "garou-p1.bin", 0x000000, 0x100000, CRC(fd446d59) SHA1(e37484673a935b2af76c84cd26977c751c0f8cff) )
	ROM_LOAD16_WORD_SWAP( "garou-p2.bin", 0x100000, 0x400000, CRC(3fb10a84) SHA1(4e4a4f4cd7f0ad2520c938c64c8910e6f8805eaf) )

	NEO_SFIX_128K( "garou-s1.bin", CRC(df720e33) SHA1(58d05002d4851682bd626241fa7b70f78f6f3bc8) )

	NEO_BIOS_AUDIO_512K( "garou-m1.bin", CRC(7c51d002) SHA1(01ffba6cbc8da07804f7b21d8c71c39d64a1a4e2) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "garou-v1.bin", 0x000000, 0x400000, CRC(98d736e4) SHA1(3edda9a1d45b0e38a85156d24fc8ff2f0011239b) )
	ROM_LOAD( "garou-v2.bin", 0x400000, 0x400000, CRC(eb43c03f) SHA1(83c9c168b154e60a64f1033004b2d33e218bbb8b) )
	ROM_LOAD( "garou-v3.bin", 0x800000, 0x400000, CRC(10a0f814) SHA1(e86def80d6fb2a38ebc9f3338d22f28c15ce85da) )
	ROM_LOAD( "garou-v4.bin", 0xc00000, 0x400000, CRC(8918fdd3) SHA1(60ea2104a0f993341124728d8fde0e8e937c55ef) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "garou-c1.bin", 0x0000000, 0x1000000, CRC(e2ac83fa) SHA1(186f88a85d80efbb0371bd42cca152b6b59817fb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "garou-c2.bin", 0x0000001, 0x1000000, CRC(7c344b24) SHA1(f8af62a917e0ce2bf8ae4f17736fdd84d55d0788) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "garou-c3.bin", 0x2000000, 0x1000000, CRC(d3aec5a6) SHA1(c1a584909a8a1519f676aa49351742b87c18276d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "garou-c4.bin", 0x2000001, 0x1000000, CRC(e02a242d) SHA1(8a73826d14880303a7bea2a903e842c733178aca) ) /* Plane 2,3 */
ROM_END

/* Metal Slug 3 bootleg */

ROM_START( mslug3b6 ) /* This "Metal Slug 6" is a hack/bootleg of Metal Slug 3, the real Metal Slug 6 is on Atomiswave Hardware */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "299-p1.bin", 0x000000, 0x200000, CRC(5f2fe228) SHA1(747775a2dfc0da87ad2ddd4f57ce5b2522f23fa5) )
	ROM_LOAD16_WORD_SWAP( "299-p2.bin", 0x100000, 0x400000, CRC(193fa835) SHA1(fb1f26db7998b0bb6b1c8b92500c1596ec5dfc71) )

	NEO_SFIX_128K( "299-s1.bin", CRC(6f8b9635) SHA1(86b0c8c0ccac913c6192ed6a96c35d4e1a5e8061) )

	NEO_BIOS_AUDIO_512K( "256-m1.bin", CRC(eaeec116) SHA1(54419dbb21edc8c4b37eaac2e7ad9496d2de037a) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "256-v1.bin", 0x000000, 0x400000, CRC(f2690241) SHA1(fd56babc1934d10e0d27c32f032f9edda7ca8ce9) )
	ROM_LOAD( "256-v2.bin", 0x400000, 0x400000, CRC(7e2a10bd) SHA1(0d587fb9f64cba0315ce2d8a03e2b8fe34936dff) )
	ROM_LOAD( "256-v3.bin", 0x800000, 0x400000, CRC(0eaec17c) SHA1(c3ed613cc6993edd6fc0d62a90bcd85de8e21915) )
	ROM_LOAD( "256-v4.bin", 0xc00000, 0x400000, CRC(9b4b22d4) SHA1(9764fbf8453e52f80aa97a46fb9cf5937ef15a31) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "256-c1.bin", 0x0000000, 0x800000, CRC(5a79c34e) SHA1(b8aa51fa50935cae62ab3d125b723ab888691e60) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c2.bin", 0x0000001, 0x800000, CRC(944c362c) SHA1(3843ab300f956280475469caee70135658f67089) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c3.bin", 0x1000000, 0x800000, CRC(6e69d36f) SHA1(94e8cf42e999114b4bd8b30e0aa2f365578c4c9a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c4.bin", 0x1000001, 0x800000, CRC(b755b4eb) SHA1(804700a0966a48f130c434ede3f970792ea74fa5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c5.bin", 0x2000000, 0x800000, CRC(7aacab47) SHA1(312c1c9846175fe1a3cad51d5ae230cf674fc93d) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c6.bin", 0x2000001, 0x800000, CRC(c698fd5d) SHA1(16818883b06849ba2f8d61bdd5e21aaf99bd8408) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "256-c7.bin", 0x3000000, 0x800000, CRC(cfceddd2) SHA1(7def666adf8bd1703f40c61f182fc040b6362dc9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "256-c8.bin", 0x3000001, 0x800000, CRC(4d9be34c) SHA1(a737bdfa2b815aea7067e7af2636e83a9409c414) ) /* Plane 2,3 */
ROM_END

/* Nightmare in the Dark bootleg */

ROM_START( nitdbl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "nitd-p1.bin", 0x000000, 0x080000, CRC(1a05bd1b) SHA1(7bbddef842d50b0778711063af695b168a76ff61) )

	NEO_SFIX_128K( "nitd-s1.bin", CRC(dd3bf47c) SHA1(881271caee6508b8be51bf1b59c8f1e58e08e551) )

	/* Bootleg m1 is 128k, data is identical */
	NEO_BIOS_AUDIO_512K( "260-m1.bin", CRC(6407c5e5) SHA1(d273e154cc905b63205a17a1a6d419cac3485a92) )

	ROM_REGION( 0x400000, "ymsnd", 0 )
	ROM_LOAD( "nitd-v1.bin", 0x000000, 0x200000, CRC(79008868) SHA1(90bd6aaefd37341297ab1f4ae7246e52facd87d0) )
	ROM_LOAD( "nitd-v2.bin", 0x200000, 0x200000, CRC(728558F9) SHA1(309aa7c933c199b2e540a601b363e7af8744fe00) )

	NO_DELTAT_REGION

	ROM_REGION( 0x800000, "sprites", 0 )
	ROM_LOAD16_BYTE( "nitd-c1.bin", 0x000000, 0x200000, CRC(b4353190) SHA1(90d5352e243a05f5c2be4fa7475667bb56e78016) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "nitd-c2.bin", 0x000001, 0x200000, CRC(6e27511f) SHA1(1fc5cf7786ad0f0bc7b1623acabe605ad04af3c1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "nitd-c3.bin", 0x400000, 0x200000, CRC(472cf075) SHA1(7cdd25019e37a3d127e68a4179c051881df19afa) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "nitd-c4.bin", 0x400001, 0x200000, CRC(4c3926e6) SHA1(7fc54a9886dbef911f7b226e3cd20081c535e989) ) /* Plane 2,3 */
ROM_END

/* The King of Fighters 2001 bootlegs */

ROM_START( cthd2003 ) /* Protected hack/bootleg of kof2001 Phenixsoft */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "5003-p1.bin", 0x000000, 0x100000, CRC(bb7602c1) SHA1(abf329a40f34c88f7325b255e3bc090db1edaca4) )
	ROM_LOAD16_WORD_SWAP( "5003-p2.bin", 0x100000, 0x400000, CRC(adc1c22b) SHA1(271e0629989257a0d21d280c05df53df259414b1) )

	NEO_SFIX_128K( "5003-s1.bin", CRC(5ba29aab) SHA1(e7ea67268a10243693bff722e6fd2276ca540acf) )

	NEO_BIOS_AUDIO_128K( "5003-m1.bin", CRC(1a8c274b) SHA1(5f6f9c533f4a296a18c741ce59a69cf6f5c836b9) )

	/* sound roms are identical to kof2001 */
	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "262-v1-08-e0.bin", 0x000000, 0x400000, CRC(83d49ecf) SHA1(2f2c116e45397652e77fcf5d951fa5f71b639572) )
	ROM_LOAD( "262-v2-08-e0.bin", 0x400000, 0x400000, CRC(003f1843) SHA1(bdd58837ad542548bd4053c262f558af88e3b989) )
	ROM_LOAD( "262-v3-08-e0.bin", 0x800000, 0x400000, CRC(2ae38dbe) SHA1(4e82b7dd3b899d61907620517a5a27bdaba0725d) )
	ROM_LOAD( "262-v4-08-e0.bin", 0xc00000, 0x400000, CRC(26ec4dd9) SHA1(8bd68d95a2d913be41a51f51e48dbe3bff5924fb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "5003-c1.bin", 0x0000000, 0x800000, CRC(68f54b67) SHA1(e2869709b11ea2846799fe431211c83e928e103e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c2.bin", 0x0000001, 0x800000, CRC(2f8849d5) SHA1(7ef74981aa056f5acab4ddabffd3e98b4cb970be) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c3.bin", 0x1000000, 0x800000, CRC(ac4aff71) SHA1(c983f642e68deaa40fee3e208f2dd55f3bacbdc1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c4.bin", 0x1000001, 0x800000, CRC(afef5d66) SHA1(39fe785563fbea54bba88de60dcc62e2458bd74a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c5.bin", 0x2000000, 0x800000, CRC(c7c1ae50) SHA1(f54f5be7513a5ce2f01ab107a2b26f6a9ee1f2a9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c6.bin", 0x2000001, 0x800000, CRC(613197f9) SHA1(6d1fefa1be81b79e251e55a1352544c0298e4674) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c7.bin", 0x3000000, 0x800000, CRC(64ddfe0f) SHA1(361f3f4618009bf6419961266eb9ab5002bef53c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c8.bin", 0x3000001, 0x800000, CRC(917a1439) SHA1(6f28d1d7c6edee1283f25e632c69204dbebe40af) ) /* Plane 2,3 */
ROM_END

ROM_START( ct2k3sp ) /* Protected hack/bootleg of kof2001 Phenixsoft */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "5003-p1sp.bin", 0x000000, 0x100000, CRC(ab5c4de0) SHA1(ca9a6bfd4c32d791ecabb34ccbf2cbf0e84f97d5) )
	ROM_LOAD16_WORD_SWAP( "5003-p2.bin",   0x100000, 0x400000, CRC(adc1c22b) SHA1(271e0629989257a0d21d280c05df53df259414b1) )

	ROM_Y_ZOOM

	ROM_REGION( 0x40000, "fixed", 0 )
	ROM_LOAD( "5003-s1sp.bin", 0x00000, 0x40000, CRC(6c355ab4) SHA1(71ac2bcd3dbda8402baecc56dabc2297b148a900) )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "5003-m1.bin", CRC(1a8c274b) SHA1(5f6f9c533f4a296a18c741ce59a69cf6f5c836b9) )

	/* sound roms are identical to kof2001 */
	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "262-v1-08-e0.bin", 0x000000, 0x400000, CRC(83d49ecf) SHA1(2f2c116e45397652e77fcf5d951fa5f71b639572) )
	ROM_LOAD( "262-v2-08-e0.bin", 0x400000, 0x400000, CRC(003f1843) SHA1(bdd58837ad542548bd4053c262f558af88e3b989) )
	ROM_LOAD( "262-v3-08-e0.bin", 0x800000, 0x400000, CRC(2ae38dbe) SHA1(4e82b7dd3b899d61907620517a5a27bdaba0725d) )
	ROM_LOAD( "262-v4-08-e0.bin", 0xc00000, 0x400000, CRC(26ec4dd9) SHA1(8bd68d95a2d913be41a51f51e48dbe3bff5924fb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "5003-c1.bin", 0x0000000, 0x800000, CRC(68f54b67) SHA1(e2869709b11ea2846799fe431211c83e928e103e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c2.bin", 0x0000001, 0x800000, CRC(2f8849d5) SHA1(7ef74981aa056f5acab4ddabffd3e98b4cb970be) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c3.bin", 0x1000000, 0x800000, CRC(ac4aff71) SHA1(c983f642e68deaa40fee3e208f2dd55f3bacbdc1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c4.bin", 0x1000001, 0x800000, CRC(afef5d66) SHA1(39fe785563fbea54bba88de60dcc62e2458bd74a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c5.bin", 0x2000000, 0x800000, CRC(c7c1ae50) SHA1(f54f5be7513a5ce2f01ab107a2b26f6a9ee1f2a9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c6.bin", 0x2000001, 0x800000, CRC(613197f9) SHA1(6d1fefa1be81b79e251e55a1352544c0298e4674) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c7.bin", 0x3000000, 0x800000, CRC(64ddfe0f) SHA1(361f3f4618009bf6419961266eb9ab5002bef53c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c8.bin", 0x3000001, 0x800000, CRC(917a1439) SHA1(6f28d1d7c6edee1283f25e632c69204dbebe40af) ) /* Plane 2,3 */
ROM_END

ROM_START( ct2k3sa ) /* Protected hack/bootleg of kof2001 Phenixsoft, alternate version */
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "5003-p1sa.bin", 0x000000, 0x100000, CRC(013a509d) SHA1(c61c9b777e6e062b5f4ad87cdb78e9ca05e9bfb9) )
	ROM_LOAD16_WORD_SWAP( "5003-p2.bin",   0x100000, 0x400000, CRC(adc1c22b) SHA1(271e0629989257a0d21d280c05df53df259414b1) )

	ROM_Y_ZOOM

	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_LOAD( "5003-s1sa.bin", 0x00000, 0x20000, CRC(4e1f7eae) SHA1(3302ad290804272447ccd2e8edd3ce968f043db1) )
	/* S1 needs redump, correct? */
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "5003-m1.bin", CRC(1a8c274b) SHA1(5f6f9c533f4a296a18c741ce59a69cf6f5c836b9) )

	/* Original set has 2x64 mbit sound roms */
	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "262-v1-08-e0.bin", 0x000000, 0x400000, CRC(83d49ecf) SHA1(2f2c116e45397652e77fcf5d951fa5f71b639572) )
	ROM_LOAD( "262-v2-08-e0.bin", 0x400000, 0x400000, CRC(003f1843) SHA1(bdd58837ad542548bd4053c262f558af88e3b989) )
	ROM_LOAD( "262-v3-08-e0.bin", 0x800000, 0x400000, CRC(2ae38dbe) SHA1(4e82b7dd3b899d61907620517a5a27bdaba0725d) )
	ROM_LOAD( "262-v4-08-e0.bin", 0xc00000, 0x400000, CRC(26ec4dd9) SHA1(8bd68d95a2d913be41a51f51e48dbe3bff5924fb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "5003-c1.bin", 0x0000000, 0x800000, CRC(68f54b67) SHA1(e2869709b11ea2846799fe431211c83e928e103e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c2.bin", 0x0000001, 0x800000, CRC(2f8849d5) SHA1(7ef74981aa056f5acab4ddabffd3e98b4cb970be) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c3.bin", 0x1000000, 0x800000, CRC(ac4aff71) SHA1(c983f642e68deaa40fee3e208f2dd55f3bacbdc1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c4.bin", 0x1000001, 0x800000, CRC(afef5d66) SHA1(39fe785563fbea54bba88de60dcc62e2458bd74a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c5.bin", 0x2000000, 0x800000, CRC(c7c1ae50) SHA1(f54f5be7513a5ce2f01ab107a2b26f6a9ee1f2a9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c6.bin", 0x2000001, 0x800000, CRC(613197f9) SHA1(6d1fefa1be81b79e251e55a1352544c0298e4674) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5003-c7.bin", 0x3000000, 0x800000, CRC(64ddfe0f) SHA1(361f3f4618009bf6419961266eb9ab5002bef53c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5003-c8.bin", 0x3000001, 0x800000, CRC(917a1439) SHA1(6f28d1d7c6edee1283f25e632c69204dbebe40af) ) /* Plane 2,3 */
ROM_END

/* Metal Slug 4 bootleg */

ROM_START( ms4plus )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ms4-p1p.bin", 0x000000, 0x100000, CRC(806a6e04) SHA1(df503772d607271ea51285154c9fd68e18b143ce) )
	ROM_LOAD16_WORD_SWAP( "263-p2.bin",  0x100000, 0x400000, CRC(fdb7aed8) SHA1(dbeaec38f44e58ffedba99e70fa1439c2bf0dfa3) )

	NEO_SFIX_128K( "ms4-s1p.bin", CRC(07ff87ce) SHA1(96ddb439de2a26bf9869015d7fb19129d40f3fd9) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "263-m1.bin", CRC(46ac8228) SHA1(5aeea221050c98e4bb0f16489ce772bf1c80f787) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "263-v1.bin", 0x000000, 0x800000, CRC(01e9b9cd) SHA1(0b045c2999449f7dab5ae8a42e957d5b6650431e) )
	ROM_LOAD( "263-v2.bin", 0x800000, 0x800000, CRC(4ab2bf81) SHA1(77ccfa48f7e3daddef5fe5229a0093eb2f803742) )

	NO_DELTAT_REGION

	ROM_REGION( 0x3000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "263-c1.bin", 0x0000000, 0x800000, CRC(84865f8a) SHA1(34467ada896eb7c7ca58658bf2a932936d8b632c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c2.bin", 0x0000001, 0x800000, CRC(81df97f2) SHA1(2b74493b8ec8fd49216a627aeb3db493f76124e3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "263-c3.bin", 0x1000000, 0x800000, CRC(1a343323) SHA1(bbbb5232bba538c277ce2ee02e2956ca2243b787) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c4.bin", 0x1000001, 0x800000, CRC(942cfb44) SHA1(d9b46c71726383c4581fb042e63897e5a3c92d1b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "263-c5.bin", 0x2000000, 0x800000, CRC(a748854f) SHA1(2611bbedf9b5d8e82c6b2c99b88f842c46434d41) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "263-c6.bin", 0x2000001, 0x800000, CRC(5c8ba116) SHA1(6034db09c8706d4ddbcefc053efbc47a0953eb92) ) /* Plane 2,3 */
ROM_END

/* The King of Fighters 2002 bootlegs */

ROM_START( kof2002b )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "265-p1.bin", 0x000000, 0x100000, CRC(9ede7323) SHA1(ad9d45498777fda9fa58e75781f48e09aee705a6) )
	ROM_LOAD16_WORD_SWAP( "2k2-p2.bin", 0x100000, 0x400000, CRC(6dbee4df) SHA1(9a9646c81b233b44213c624b898c19f83e9a07f8) )

	NEO_SFIX_128K( "2k2-s1.bin", CRC(2255f5bf) SHA1(8a82b3e9717df30b580b9d0bac0b403f8102a002) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "265-m1.bin", CRC(85aaa632) SHA1(744fba4ca3bc3a5873838af886efb97a8a316104) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "265-v1.bin", 0x000000, 0x800000, CRC(15e8f3f5) SHA1(7c9e6426b9fa6db0158baa17a6485ffce057d889) )
	ROM_LOAD( "265-v2.bin", 0x800000, 0x800000, CRC(da41d6f9) SHA1(a43021f1e58947dcbe3c8ca5283b20b649f0409d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "2k2-c1.bin", 0x0000000, 0x800000, CRC(f25d3d66) SHA1(eb1da3e171c126d91e851ce141840709a2f62f8a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "2k2-c2.bin", 0x0000001, 0x800000, CRC(e3e66f1d) SHA1(af93e9e134816353d6187a53959c6e418b83ad8d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "2k2-c3.bin", 0x1000000, 0x800000, CRC(8732fa30) SHA1(81c482b375c04bcfbbc69e3e2a2e9ab567c9bb78) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "2k2-c4.bin", 0x1000001, 0x800000, CRC(0989fd40) SHA1(355d6b2c528319e41ce89952c5cf5bcc47cd6de0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "2k2-c5.bin", 0x2000000, 0x800000, CRC(60635cd2) SHA1(0cf2c54e003edfcdbed64e0570e6b800e7ed3c1b) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "2k2-c6.bin", 0x2000001, 0x800000, CRC(bd736824) SHA1(d897fc8248ace145fef57d8aa393eaebc4a1ccc4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "2k2-c7.bin", 0x3000000, 0x800000, CRC(2da8d8cf) SHA1(ab8aa88b8e1baba88e5fc01d0f3cb55503b6c81a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "2k2-c8.bin", 0x3000001, 0x800000, CRC(2048404a) SHA1(d6d0f049ffc196334825328e0472b04e04bf6695) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k2pls )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "2k2-p1p.bin", 0x000000, 0x100000, CRC(3ab03781) SHA1(86946c19f1c4d9ab5cde86688d698bf63118a39d) )
	ROM_LOAD16_WORD_SWAP( "265-p2.bin",  0x100000, 0x400000, CRC(327266b8) SHA1(98f445cc0a94f8744d74bca71cb420277622b034) )

	NEO_SFIX_128K( "2k2-s1p.bin", CRC(595e0006) SHA1(ff086bdaa6f40e9ad963e1100a27f44618d684ed) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "265-m1.bin", CRC(85aaa632) SHA1(744fba4ca3bc3a5873838af886efb97a8a316104) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "265-v1.bin", 0x000000, 0x800000, CRC(15e8f3f5) SHA1(7c9e6426b9fa6db0158baa17a6485ffce057d889) )
	ROM_LOAD( "265-v2.bin", 0x800000, 0x800000, CRC(da41d6f9) SHA1(a43021f1e58947dcbe3c8ca5283b20b649f0409d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "265-c1.bin", 0x0000000, 0x800000, CRC(2b65a656) SHA1(9c46d8cf5b1ef322db442ac6a9b9406ab49206c5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c2.bin", 0x0000001, 0x800000, CRC(adf18983) SHA1(150cd4a5e51e9df88688469d2ea7675c2cf3658a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c3.bin", 0x1000000, 0x800000, CRC(875e9fd7) SHA1(28f52d56192d48bbc5dc3c97abf456bd34a58cbd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c4.bin", 0x1000001, 0x800000, CRC(2da13947) SHA1(f8d79ec2c236aa3d3648a4f715676899602122c1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c5.bin", 0x2000000, 0x800000, CRC(61bd165d) SHA1(b3424db84bc683d858fb635bc42728f9cdd89caf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c6.bin", 0x2000001, 0x800000, CRC(03fdd1eb) SHA1(6155c7e802062f4eafa27e414c4e73ee59b868bf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c7.bin", 0x3000000, 0x800000, CRC(1a2749d8) SHA1(af7d9ec1d576209826fa568f676bbff92f6d6ddd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c8.bin", 0x3000001, 0x800000, CRC(ab0bb549) SHA1(d23afb60b7f831f7d4a98ad3c4a00ee19877a1ce) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k2pla )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "2k2-p1pa.bin", 0x000000, 0x100000, CRC(6a3a02f3) SHA1(c9973b64e9a87fa38dde233ee3e9a73ba085b013) )
	ROM_LOAD16_WORD_SWAP( "265-p2.bin",   0x100000, 0x400000, CRC(327266b8) SHA1(98f445cc0a94f8744d74bca71cb420277622b034) )

	NEO_SFIX_128K( "2k2-s1pa.bin", CRC(1a3ed064) SHA1(9749bb55c750e6b65d651998c2649c5fb68db68e))

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "265-m1.bin", CRC(85aaa632) SHA1(744fba4ca3bc3a5873838af886efb97a8a316104) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "265-v1.bin", 0x000000, 0x800000, CRC(15e8f3f5) SHA1(7c9e6426b9fa6db0158baa17a6485ffce057d889) )
	ROM_LOAD( "265-v2.bin", 0x800000, 0x800000, CRC(da41d6f9) SHA1(a43021f1e58947dcbe3c8ca5283b20b649f0409d) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "265-c1.bin", 0x0000000, 0x800000, CRC(2b65a656) SHA1(9c46d8cf5b1ef322db442ac6a9b9406ab49206c5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c2.bin", 0x0000001, 0x800000, CRC(adf18983) SHA1(150cd4a5e51e9df88688469d2ea7675c2cf3658a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c3.bin", 0x1000000, 0x800000, CRC(875e9fd7) SHA1(28f52d56192d48bbc5dc3c97abf456bd34a58cbd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c4.bin", 0x1000001, 0x800000, CRC(2da13947) SHA1(f8d79ec2c236aa3d3648a4f715676899602122c1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c5.bin", 0x2000000, 0x800000, CRC(61bd165d) SHA1(b3424db84bc683d858fb635bc42728f9cdd89caf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c6.bin", 0x2000001, 0x800000, CRC(03fdd1eb) SHA1(6155c7e802062f4eafa27e414c4e73ee59b868bf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c7.bin", 0x3000000, 0x800000, CRC(1a2749d8) SHA1(af7d9ec1d576209826fa568f676bbff92f6d6ddd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c8.bin", 0x3000001, 0x800000, CRC(ab0bb549) SHA1(d23afb60b7f831f7d4a98ad3c4a00ee19877a1ce) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k2mp )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "kf02m-p1.bin", 0x000000, 0x400000, CRC(ff7c6ec0) SHA1(704c14d671dcb4cfed44d9f978a289cb7dd9d065) )
	ROM_LOAD16_WORD_SWAP( "kf02m-p2.bin", 0x400000, 0x400000, CRC(91584716) SHA1(90da863037cf775957fa154cd42536e221df5740) )

	NEO_SFIX_128K( "kf02m-s1.bin", CRC(348d6f2c) SHA1(586da8a936ebbb71af324339a4b60ec91dfa0990) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "265-m1.bin", CRC(85aaa632) SHA1(744fba4ca3bc3a5873838af886efb97a8a316104) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "265-v1.bin", 0x000000, 0x800000, CRC(15e8f3f5) SHA1(7c9e6426b9fa6db0158baa17a6485ffce057d889) )
	ROM_LOAD( "265-v2.bin", 0x800000, 0x800000, CRC(da41d6f9) SHA1(a43021f1e58947dcbe3c8ca5283b20b649f0409d) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "265-c1.bin", 0x0000000, 0x800000, CRC(2b65a656) SHA1(9c46d8cf5b1ef322db442ac6a9b9406ab49206c5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c2.bin", 0x0000001, 0x800000, CRC(adf18983) SHA1(150cd4a5e51e9df88688469d2ea7675c2cf3658a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c3.bin", 0x1000000, 0x800000, CRC(875e9fd7) SHA1(28f52d56192d48bbc5dc3c97abf456bd34a58cbd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c4.bin", 0x1000001, 0x800000, CRC(2da13947) SHA1(f8d79ec2c236aa3d3648a4f715676899602122c1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c5.bin", 0x2000000, 0x800000, CRC(61bd165d) SHA1(b3424db84bc683d858fb635bc42728f9cdd89caf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c6.bin", 0x2000001, 0x800000, CRC(03fdd1eb) SHA1(6155c7e802062f4eafa27e414c4e73ee59b868bf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c7.bin", 0x3000000, 0x800000, CRC(1a2749d8) SHA1(af7d9ec1d576209826fa568f676bbff92f6d6ddd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c8.bin", 0x3000001, 0x800000, CRC(ab0bb549) SHA1(d23afb60b7f831f7d4a98ad3c4a00ee19877a1ce) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k2mp2 )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "k2k2m2p1.bin", 0x000000, 0x200000, CRC(1016806c) SHA1(a583b45e9c0d6f67b95c52e44444aabe88f68d97) )
	ROM_LOAD16_WORD_SWAP( "k2k2m2p2.bin", 0x200000, 0x400000, CRC(432fdf53) SHA1(d7e542cd84d948162c60768e40ee4ed33d8e7913) )

	NEO_SFIX_128K( "k2k2m2s1.bin", CRC(446e74c5) SHA1(efc2afb26578bad9eb21659c70eb0f827d6d1ef6) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_128K( "265-m1.bin", CRC(85aaa632) SHA1(744fba4ca3bc3a5873838af886efb97a8a316104) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "265-v1.bin", 0x000000, 0x800000, CRC(15e8f3f5) SHA1(7c9e6426b9fa6db0158baa17a6485ffce057d889) )
	ROM_LOAD( "265-v2.bin", 0x800000, 0x800000, CRC(da41d6f9) SHA1(a43021f1e58947dcbe3c8ca5283b20b649f0409d) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "265-c1.bin", 0x0000000, 0x800000, CRC(2b65a656) SHA1(9c46d8cf5b1ef322db442ac6a9b9406ab49206c5) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c2.bin", 0x0000001, 0x800000, CRC(adf18983) SHA1(150cd4a5e51e9df88688469d2ea7675c2cf3658a) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c3.bin", 0x1000000, 0x800000, CRC(875e9fd7) SHA1(28f52d56192d48bbc5dc3c97abf456bd34a58cbd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c4.bin", 0x1000001, 0x800000, CRC(2da13947) SHA1(f8d79ec2c236aa3d3648a4f715676899602122c1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c5.bin", 0x2000000, 0x800000, CRC(61bd165d) SHA1(b3424db84bc683d858fb635bc42728f9cdd89caf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c6.bin", 0x2000001, 0x800000, CRC(03fdd1eb) SHA1(6155c7e802062f4eafa27e414c4e73ee59b868bf) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "265-c7.bin", 0x3000000, 0x800000, CRC(1a2749d8) SHA1(af7d9ec1d576209826fa568f676bbff92f6d6ddd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "265-c8.bin", 0x3000001, 0x800000, CRC(ab0bb549) SHA1(d23afb60b7f831f7d4a98ad3c4a00ee19877a1ce) ) /* Plane 2,3 */
ROM_END

ROM_START( kof10th )
	ROM_REGION( 0x900000, "maincpu", 0 ) // Modified
	ROM_LOAD16_WORD_SWAP( "kf10-p1.bin", 0x000000, 0x800000, CRC(b1fd0c43) SHA1(5f842a8a27be2d957fd4140d6431ae47154997bb) )

	ROM_Y_ZOOM

	ROM_REGION( 0x40000, "fixed", 0 ) // modified
	ROM_FILL( 0x000000, 0x40000, 0 ) // modified
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "kf10-m1.bin", CRC(f6fab859) SHA1(0184aa1394b9f9946d610278b53b846020dd88dc) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "kf10-v1.bin", 0x000000, 0x800000, CRC(0fc9a58d) SHA1(9d79ef00e2c2abd9f29af5521c2fbe5798bf336f) )
	ROM_LOAD( "kf10-v2.bin", 0x800000, 0x800000, CRC(b8c475a4) SHA1(10caf9c69927a223445d2c4b147864c02ce520a8) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "kf10-c1a.bin", 0x0000000, 0x400000, CRC(3bbc0364) SHA1(e8aa7ff82f151ce1db56f259377b64cceef85af0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c2a.bin", 0x0000001, 0x400000, CRC(91230075) SHA1(d9098e05a7ba6008661147b6bf8bc2f494b8b72b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c1b.bin", 0x0800000, 0x400000, CRC(b5abfc28) SHA1(eabf60992bb3485c95330065294071ec155bfe7c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c2b.bin", 0x0800001, 0x400000, CRC(6cc4c6e1) SHA1(be824a944e745ee18efdc45c81fd496a4d624b9c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c3a.bin", 0x1000000, 0x400000, CRC(5b3d4a16) SHA1(93ac1cd7739100f8c32732644f81f2a19837b131) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c4a.bin", 0x1000001, 0x400000, CRC(c6f3419b) SHA1(340c17a73aeb7bf8a6209f8459e6f00000075b50) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c3b.bin", 0x1800000, 0x400000, CRC(9d2bba19) SHA1(5ebbd0af3f83a60e33c8ccb743e3d5f5a96f1273) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c4b.bin", 0x1800001, 0x400000, CRC(5a4050cb) SHA1(8fd2291f349efa1ed5cd37ad4e273b60fe831a77) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c5a.bin", 0x2000000, 0x400000, CRC(a289d1e1) SHA1(50c7d7ebde6e118a01036cc3e40827fcd9f0d3fd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c6a.bin", 0x2000001, 0x400000, CRC(e6494b5d) SHA1(18e064b9867ae0b0794065f8dbefd486620419db) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c5b.bin", 0x2800000, 0x400000, CRC(404fff02) SHA1(56d1b32c87ea4885e49264e8b21846e465a20e1f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c6b.bin", 0x2800001, 0x400000, CRC(f2ccfc9e) SHA1(69db7fac7023785ab94ea711a72dbc2826cfe1a3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c7a.bin", 0x3000000, 0x400000, CRC(be79c5a8) SHA1(ded3c5eb3571647f50533eb682c2675372ace3fb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c8a.bin", 0x3000001, 0x400000, CRC(a5952ca4) SHA1(76dbb3cb45ce5a4beffa1ed29491204fc6617e42) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c7b.bin", 0x3800000, 0x400000, CRC(3fdb3542) SHA1(7d2050752a2064cd6729f483a0da93808e2c6033) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c8b.bin", 0x3800001, 0x400000, CRC(661b7a52) SHA1(0ae2ad2389134892f156337332b77adade3ddad1) ) /* Plane 2,3 */
ROM_END

ROM_START( kf10thep ) /* this is a hack of kof2002 much like the various korean hacks / bootlegs of games */
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "5008-p2.bin", 0x100000, 0x400000, CRC(a649ec38) SHA1(5c63ed5e5c848940f587c966da4908d04cf1293c) )
	ROM_LOAD16_WORD_SWAP( "5008-p3.bin", 0x500000, 0x200000, CRC(e629e13c) SHA1(6ebe080ce01c51064cb2f4d89315ba98a45ae727) )

	ROM_REGION( 0x200000, "audiocrypt", 0 )
	ROM_LOAD( "5008-p1.bin", 0x000000, 0x200000, CRC(bf5469ba) SHA1(f05236d8fffab5836c0d27becdeeb80def32ee49) )

	NEO_SFIX_128K( "5008-s1.bin", CRC(92410064) SHA1(1fb800b46341858207d3b6961a760289fbec7faa) )

	NEO_BIOS_AUDIO_128K( "5008-m1.bin", CRC(5a47d9ad) SHA1(0197737934653acc6c97221660d789e9914f3578) )
	//NEO_BIOS_AUDIO_128K( "5004-m1.bin", CRC(f6fab859) SHA1(0184aa1394b9f9946d610278b53b846020dd88dc) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "kf10-v1.bin", 0x000000, 0x800000, CRC(0fc9a58d) SHA1(9d79ef00e2c2abd9f29af5521c2fbe5798bf336f) )
	ROM_LOAD( "kf10-v2.bin", 0x800000, 0x800000, CRC(b8c475a4) SHA1(10caf9c69927a223445d2c4b147864c02ce520a8) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "kf10-c1a.bin", 0x0000000, 0x400000, CRC(3bbc0364) SHA1(e8aa7ff82f151ce1db56f259377b64cceef85af0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c2a.bin", 0x0000001, 0x400000, CRC(91230075) SHA1(d9098e05a7ba6008661147b6bf8bc2f494b8b72b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c1b.bin", 0x0800000, 0x400000, CRC(b5abfc28) SHA1(eabf60992bb3485c95330065294071ec155bfe7c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c2b.bin", 0x0800001, 0x400000, CRC(6cc4c6e1) SHA1(be824a944e745ee18efdc45c81fd496a4d624b9c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c3a.bin", 0x1000000, 0x400000, CRC(5b3d4a16) SHA1(93ac1cd7739100f8c32732644f81f2a19837b131) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c4a.bin", 0x1000001, 0x400000, CRC(c6f3419b) SHA1(340c17a73aeb7bf8a6209f8459e6f00000075b50) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c3b.bin", 0x1800000, 0x400000, CRC(9d2bba19) SHA1(5ebbd0af3f83a60e33c8ccb743e3d5f5a96f1273) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c4b.bin", 0x1800001, 0x400000, CRC(5a4050cb) SHA1(8fd2291f349efa1ed5cd37ad4e273b60fe831a77) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c5a.bin", 0x2000000, 0x400000, CRC(a289d1e1) SHA1(50c7d7ebde6e118a01036cc3e40827fcd9f0d3fd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c6a.bin", 0x2000001, 0x400000, CRC(e6494b5d) SHA1(18e064b9867ae0b0794065f8dbefd486620419db) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c5b.bin", 0x2800000, 0x400000, CRC(404fff02) SHA1(56d1b32c87ea4885e49264e8b21846e465a20e1f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c6b.bin", 0x2800001, 0x400000, CRC(f2ccfc9e) SHA1(69db7fac7023785ab94ea711a72dbc2826cfe1a3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c7a.bin", 0x3000000, 0x400000, CRC(be79c5a8) SHA1(ded3c5eb3571647f50533eb682c2675372ace3fb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c8a.bin", 0x3000001, 0x400000, CRC(a5952ca4) SHA1(76dbb3cb45ce5a4beffa1ed29491204fc6617e42) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "5008-c7b.bin", 0x3800000, 0x400000, CRC(33604ef0) SHA1(57deec23c81d5d673ce5992cef1f2567f1a2148e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "5008-c8b.bin", 0x3800001, 0x400000, CRC(51f6a8f8) SHA1(9ef1cdbdd125a2b430346c22b59f36902312905f) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k5uni )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "5006-p2a.bin", 0x000000, 0x400000, CRC(ced883a2) SHA1(f93db4d74ce0a73a3e9631966fee37be22470c89) )
	ROM_LOAD16_WORD_SWAP( "5006-p1.bin",  0x400000, 0x400000, CRC(72c39c46) SHA1(4ba0657de20319c0bc30c7c3bba7d7331d0ce9a7) )

	NEO_SFIX_128K( "5006-s1.bin", CRC(91f8c544) SHA1(9d16cafb9ca4bc54f31f7fd82b1be06ec8b11c79) )

	NEO_BIOS_AUDIO_128K( "5006-m1.bin", CRC(9050bfe7) SHA1(765bf3d954f775231b7ef2504bb844cd0b29e3f7) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "kf10-v1.bin", 0x000000, 0x800000, CRC(0fc9a58d) SHA1(9d79ef00e2c2abd9f29af5521c2fbe5798bf336f) )
	ROM_LOAD( "kf10-v2.bin", 0x800000, 0x800000, CRC(b8c475a4) SHA1(10caf9c69927a223445d2c4b147864c02ce520a8) )

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "kf10-c1a.bin", 0x0000000, 0x400000, CRC(3bbc0364) SHA1(e8aa7ff82f151ce1db56f259377b64cceef85af0) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c2a.bin", 0x0000001, 0x400000, CRC(91230075) SHA1(d9098e05a7ba6008661147b6bf8bc2f494b8b72b) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c1b.bin", 0x0800000, 0x400000, CRC(b5abfc28) SHA1(eabf60992bb3485c95330065294071ec155bfe7c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c2b.bin", 0x0800001, 0x400000, CRC(6cc4c6e1) SHA1(be824a944e745ee18efdc45c81fd496a4d624b9c) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c3a.bin", 0x1000000, 0x400000, CRC(5b3d4a16) SHA1(93ac1cd7739100f8c32732644f81f2a19837b131) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c4a.bin", 0x1000001, 0x400000, CRC(c6f3419b) SHA1(340c17a73aeb7bf8a6209f8459e6f00000075b50) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c3b.bin", 0x1800000, 0x400000, CRC(9d2bba19) SHA1(5ebbd0af3f83a60e33c8ccb743e3d5f5a96f1273) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c4b.bin", 0x1800001, 0x400000, CRC(5a4050cb) SHA1(8fd2291f349efa1ed5cd37ad4e273b60fe831a77) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c5a.bin", 0x2000000, 0x400000, CRC(a289d1e1) SHA1(50c7d7ebde6e118a01036cc3e40827fcd9f0d3fd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c6a.bin", 0x2000001, 0x400000, CRC(e6494b5d) SHA1(18e064b9867ae0b0794065f8dbefd486620419db) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c5b.bin", 0x2800000, 0x400000, CRC(404fff02) SHA1(56d1b32c87ea4885e49264e8b21846e465a20e1f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c6b.bin", 0x2800001, 0x400000, CRC(f2ccfc9e) SHA1(69db7fac7023785ab94ea711a72dbc2826cfe1a3) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c7a.bin", 0x3000000, 0x400000, CRC(be79c5a8) SHA1(ded3c5eb3571647f50533eb682c2675372ace3fb) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c8a.bin", 0x3000001, 0x400000, CRC(a5952ca4) SHA1(76dbb3cb45ce5a4beffa1ed29491204fc6617e42) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "kf10-c7b.bin", 0x3800000, 0x400000, CRC(3fdb3542) SHA1(7d2050752a2064cd6729f483a0da93808e2c6033) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "kf10-c8b.bin", 0x3800001, 0x400000, CRC(661b7a52) SHA1(0ae2ad2389134892f156337332b77adade3ddad1) ) /* Plane 2,3 */
ROM_END

ROM_START( kof2k4se )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "k2k4s-p2.bin", 0x000000, 0x080000, CRC(21a84084) SHA1(973e8a0bffa0e1f055803f663f81a8e03701802d) )
	ROM_LOAD16_WORD_SWAP( "k2k4s-p3.bin", 0x080000, 0x080000, CRC(febb484e) SHA1(4b1838795b84f22d578ad043641df0a7bf7d9774) )
	ROM_LOAD16_WORD_SWAP( "k2k4s-p1.bin", 0x100000, 0x400000, CRC(e6c50566) SHA1(cc6a3489a3bfeb4dcc65b6ddae0030f7e66fbabe) )

	NEO_SFIX_128K( "k2k4s-s1.bin", CRC(a3c9b2d8) SHA1(1472d2cbd7bb73e84824ecf773924007e6117e77) )

	NEO_BIOS_AUDIO_128K( "k2k4s-m1.bin", CRC(5a47d9ad) SHA1(0197737934653acc6c97221660d789e9914f3578) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "k2k4s-v2.bin", 0x000000, 0x800000, CRC(e4ddfb3f) SHA1(eb8220ab01c16cf9244b7f3f9912bec0db561b85) )
	ROM_LOAD( "k2k4s-v1.bin", 0x800000, 0x800000, CRC(b887d287) SHA1(f593a5722df6f6fac023d189a739a117e976bb2f) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "k2k4s-c4.bin", 0x0000000, 0x800000, CRC(7a050288) SHA1(55a20c5b01e11a859f096af3f8e09986025d288f) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "k2k4s-c8.bin", 0x0000001, 0x800000, CRC(e924afcf) SHA1(651e974f7339d2cdcfa58c5398013197a0525b77) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "k2k4s-c3.bin", 0x1000000, 0x800000, CRC(959fad0b) SHA1(63ab83ddc5f688dc8165a7ff8d262df3fcd942a2) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "k2k4s-c7.bin", 0x1000001, 0x800000, CRC(efe6a468) SHA1(2a414285e48aa948b5b0d4a9333bab083b5fb853) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "k2k4s-c2.bin", 0x2000000, 0x800000, CRC(74bba7c6) SHA1(e01adc7a4633bc0951b9b4f09abc07d728e9a2d9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "k2k4s-c6.bin", 0x2000001, 0x800000, CRC(e20d2216) SHA1(5d28eea7b581e780b78f391a8179f1678ee0d9a5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "k2k4s-c1.bin", 0x3000000, 0x800000, CRC(fa705b2b) SHA1(f314c66876589601806352484dd8e45bc41be692) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "k2k4s-c5.bin", 0x3000001, 0x800000, CRC(2c912ff9) SHA1(b624a625ea3e221808b7ea43fb0b1a51d8c1853e) ) /* Plane 2,3 */
ROM_END

/* Matrimelee bootleg */

ROM_START( matrimbl )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "266-p1.bin", 0x000000, 0x100000, CRC(5d4c2dc7) SHA1(8d723b0d28ec344eef26009b361a2b97d300dd51) )
	ROM_LOAD16_WORD_SWAP( "266-p2.bin", 0x100000, 0x400000, CRC(a14b1906) SHA1(1daa14d73512f760ef569b06f9facb279437d1db) )

	ROM_Y_ZOOM

	ROM_REGION( 0x80000, "fixed", 0 )
	ROM_FILL( 0x000000, 0x80000, 0 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEO_BIOS_AUDIO_128K( "mart-m1.bin", CRC(3ea96ab1) SHA1(e5053c4312f658faed2a34e38325a22ef792d384) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "mart-v1.bin", 0x000000, 0x400000, CRC(352b0a07) SHA1(19f7cc12f3f6d0fda9c7449816c4c32367447897) )
	ROM_LOAD16_WORD_SWAP( "mart-v2.bin", 0x400000, 0x400000, CRC(1e9bd59e) SHA1(0f754e780d0ebb815a92a45ad55f85f6d0181b70) )
	ROM_LOAD( "mart-v3.bin", 0x800000, 0x400000, CRC(e8362fcc) SHA1(42d558fd80cabe22a1c09a1fa75741afbcf46b7c) )
	ROM_LOAD16_WORD_SWAP( "mart-v4.bin", 0xc00000, 0x400000, CRC(c8c79b19) SHA1(9c7a5e694d68f37a27209e1400b60b6241a04cc7) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "mart-c1.bin", 0x0000000, 0x800000, CRC(a5595656) SHA1(d86281607f22e4f2001047eaeeda99cd673c508c) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "mart-c2.bin", 0x0000001, 0x800000, CRC(c5f7c300) SHA1(9ff5ffb750bd2e925667d84389192f92183e8677) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "mart-c3.bin", 0x1000000, 0x800000, CRC(574efd7d) SHA1(6cac303db705fe2800701ee51de9e9fca04e6e66) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "mart-c4.bin", 0x1000001, 0x800000, CRC(109d54d9) SHA1(22cb748b3b14317b90d9d9951297ada2bfc3a3f1) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "mart-c5.bin", 0x2000000, 0x800000, CRC(15c9e882) SHA1(1c9f1ccaed4fdd9d8f5cc9b6fcaca3c4e328e59e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "mart-c6.bin", 0x2000001, 0x800000, CRC(77497b97) SHA1(c6481bea5a36f8210971fdcb4bfbe7ed93c769de) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "mart-c7.bin", 0x3000000, 0x800000, CRC(ab481bb6) SHA1(6b2d97c5505eeb28e300b075f37f0d69ef44463a) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "mart-c8.bin", 0x3000001, 0x800000, CRC(906cf267) SHA1(b0f2cf8887794d715f208751ddd1ed26b2c3ffdf) ) /* Plane 2,3 */
ROM_END

/* Metal Slug 5 bootleg */

ROM_START( ms5plus )
	ROM_REGION( 0x500000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ms5-p1p.bin", 0x000000, 0x100000, CRC(106b276f) SHA1(0e840df95f3813145e5043573483c7610d2d3e68) )
	ROM_LOAD16_WORD_SWAP( "ms5-p2p.bin", 0x100000, 0x200000, CRC(d6a458e8) SHA1(c0a8bdae06d62859fb6734766ccc190eb2a809a4) )
	ROM_LOAD16_WORD_SWAP( "ms5-p3p.bin", 0x300000, 0x200000, CRC(439ec031) SHA1(f0ad8f9be7d26bc504593c1321bd23c286a221f0) )

	ROM_Y_ZOOM

	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_LOAD("ms5-s1p.bin", 0x000000, 0x20000, CRC(21e04432) SHA1(10057a2aa487087f7143d1d69fdad978a6bef0f7) )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	/* Encrypted */
	NEO_BIOS_AUDIO_ENCRYPTED_512K( "268-m1.bin", CRC(4a5a6e0e) SHA1(df0f660f2465e1db7be5adfcaf5e88ad61a74a42) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "268-v1c.bin", 0x000000, 0x800000, CRC(ae31d60c) SHA1(c42285cf4e52fea74247860813e826df5aa7600a) )
	ROM_LOAD( "268-v2c.bin", 0x800000, 0x800000, CRC(c40613ed) SHA1(af889570304e2867d7dfea1e94e388c06249fb67) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "268-c1c.bin", 0x0000000, 0x800000, CRC(ab7c389a) SHA1(025a188de589500bf7637fa8e7a37ab24bf4312e) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c2c.bin", 0x0000001, 0x800000, CRC(3560881b) SHA1(493d218c92290b4770024d6ee2917c4022753b07) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c3c.bin", 0x1000000, 0x800000, CRC(3af955ea) SHA1(cf36b6ae9b0d12744b17cb7a928399214de894be) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c4c.bin", 0x1000001, 0x800000, CRC(c329c373) SHA1(5073d4079958a0ef5426885af2c9e3178f37d5e0) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c5c.bin", 0x2000000, 0x800000, CRC(959c8177) SHA1(889bda7c65d71172e7d89194d1269561888fe789) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c6c.bin", 0x2000001, 0x800000, CRC(010a831b) SHA1(aec140661e3ae35d264df416478ba15188544d91) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "268-c7c.bin", 0x3000000, 0x800000, CRC(6d72a969) SHA1(968dd9a4d1209b770b9b85ea6532fa24d262a262) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "268-c8c.bin", 0x3000001, 0x800000, CRC(551d720e) SHA1(ebf69e334fcaba0fda6fd432fd0970283a365d12) ) /* Plane 2,3 */
ROM_END

/* Puzzle Bobble / Bust-A-Move (Neo-Geo) bootleg */

ROM_START( pbobblenb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u7", 0x000000, 0x080000, CRC(ac1e9ef3) SHA1(a2b125ee70869667431ab125bc29e768500802ad) )

	NEO_SFIX_128K( "us1", CRC(9caae538) SHA1 (cf2d90a7c1a42107c0bb8b9a61397634286dbe0a) )

	NEO_BIOS_AUDIO_128K( "um1", CRC(f424368a) SHA1 (5e5bbcaeb82bed2ee17df08f005ca20ad1030723) )

	ROM_REGION( 0x380000, "ymsnd", 0 )
	/* 0x000000-0x1fffff empty */
	ROM_LOAD( "u8", 0x200000, 0x100000, CRC(0840cbc4) SHA1 (1adbd7aef44fa80832f63dfb8efdf69fd7256a57) )
	ROM_LOAD( "u9", 0x300000, 0x080000, CRC(0a548948) SHA1 (e1e4afd17811cb60401c14fbcf0465035165f4fb) )

	NO_DELTAT_REGION

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD16_BYTE( "uc5", 0x000000, 0x80000, CRC(e89ad494) SHA1 (69c9ea415773af94ac44c48af05d55ada222b138) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "uc6", 0x000001, 0x80000, CRC(4b42d7eb) SHA1 (042ae50a528cea21cf07771d3915c57aa16fd5af) ) /* Plane 2,3 */
ROM_END

/* SNK vs. CAPCOM SVC CHAOS bootlegs */

ROM_START( svcboot )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "svc-p1.bin", 0x000000, 0x800000, CRC(0348f162) SHA1(c313351d68effd92aeb80ed320e4f8c26a3bb53e) )

	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_LOAD( "svc-s1.bin", 0x10000, 0x10000, CRC(70b44df1) SHA1(52ae3f264d7b33e94e770e6b2d0cf35a64e7dda4) )
	ROM_CONTINUE( 0x00000, 0x10000 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEOGEO_BIOS

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "svc-m1.bin", 0x20000, 0x10000, CRC(804328c3) SHA1(f931636c563b0789d4812033a77b47bf663db43f) )
	ROM_CONTINUE( 0x00000, 0x10000 )
	ROM_COPY( "audiocpu", 0x00000, 0x10000, 0x10000 )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD16_WORD_SWAP( "svc-v2.bin", 0x000000, 0x400000, CRC(b5097287) SHA1(3ba3a9b5624879616382ed40337a3d9c50a0f314) )
	ROM_LOAD16_WORD_SWAP( "svc-v1.bin", 0x400000, 0x400000, CRC(bd3a391f) SHA1(972bf09b75e99a683ee965bec93b0da8f15d72d9) )
	ROM_LOAD16_WORD_SWAP( "svc-v4.bin", 0x800000, 0x400000, CRC(33fc0b37) SHA1(d61017d829f44c7df8795ba10c55c727d9972662) )
	ROM_LOAD16_WORD_SWAP( "svc-v3.bin", 0xc00000, 0x400000, CRC(aa9849a0) SHA1(9539b3356a070a066a89f27c287f316e7367ce2a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "svc-c1.bin", 0x0000000, 0x800000, CRC(a7826b89) SHA1(3bbe348ce54b80b56ef032ea532a18ef3cafeb11) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c2.bin", 0x0000001, 0x800000, CRC(ed3c2089) SHA1(b5d17692f15f5a678c273589fab2e3918711135e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c3.bin", 0x1000000, 0x800000, CRC(71ed8063) SHA1(ea1df9e2e382a8560a06d447421844cc588f43dd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c4.bin", 0x1000001, 0x800000, CRC(250bde2d) SHA1(8c72dcfceef6d022ab4b73ab37cf3ac0c3940c17) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c5.bin", 0x2000000, 0x800000, CRC(9817c082) SHA1(1bea9c7220c2b1524896c86841d6d8fd55f5d366) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c6.bin", 0x2000001, 0x800000, CRC(2bc0307f) SHA1(8090fa82c46eb503832359093c8cc3cee3141c90) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c7.bin", 0x3000000, 0x800000, CRC(4358d7b9) SHA1(9270b58c2abc072a046bedda72f1395df26d0714) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c8.bin", 0x3000001, 0x800000, CRC(366deee5) SHA1(d477ad7a5987fd6c7ef2c1680fbb7c884654590e) ) /* Plane 2,3 */
ROM_END

ROM_START( svcplus )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "svc-p1p.bin", 0x000000, 0x200000, CRC(a194d842) SHA1(72b7bfa34a97632b1aa003488e074d766a6c2f08) )
	ROM_LOAD16_WORD_SWAP( "svc-p2p.bin", 0x200000, 0x200000, CRC(50c0e2b7) SHA1(97b396415ab0e692e43ddf371091e5a456712f0a) )
	ROM_LOAD16_WORD_SWAP( "svc-p3p.bin", 0x400000, 0x200000, CRC(58cdc293) SHA1(3c4f2418ec513bcc13ed33a727de11dfb98f7525) )

	NEO_SFIX_128K( "svc-s1p.bin", CRC(73344711) SHA1(04d84c4fe241b9135cd210f8ed8c725f595d11d2) )

	NEOGEO_BIOS

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "svc-m1.bin", 0x20000, 0x10000, CRC(804328c3) SHA1(f931636c563b0789d4812033a77b47bf663db43f) )
	ROM_CONTINUE( 0x00000, 0x10000 )
	ROM_COPY( "audiocpu", 0x00000, 0x10000, 0x10000 )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD16_WORD_SWAP( "svc-v2.bin", 0x000000, 0x400000, CRC(b5097287) SHA1(3ba3a9b5624879616382ed40337a3d9c50a0f314) )
	ROM_LOAD16_WORD_SWAP( "svc-v1.bin", 0x400000, 0x400000, CRC(bd3a391f) SHA1(972bf09b75e99a683ee965bec93b0da8f15d72d9) )
	ROM_LOAD16_WORD_SWAP( "svc-v4.bin", 0x800000, 0x400000, CRC(33fc0b37) SHA1(d61017d829f44c7df8795ba10c55c727d9972662) )
	ROM_LOAD16_WORD_SWAP( "svc-v3.bin", 0xc00000, 0x400000, CRC(aa9849a0) SHA1(9539b3356a070a066a89f27c287f316e7367ce2a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "svc-c1.bin", 0x0000000, 0x800000, CRC(a7826b89) SHA1(3bbe348ce54b80b56ef032ea532a18ef3cafeb11) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c2.bin", 0x0000001, 0x800000, CRC(ed3c2089) SHA1(b5d17692f15f5a678c273589fab2e3918711135e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c3.bin", 0x1000000, 0x800000, CRC(71ed8063) SHA1(ea1df9e2e382a8560a06d447421844cc588f43dd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c4.bin", 0x1000001, 0x800000, CRC(250bde2d) SHA1(8c72dcfceef6d022ab4b73ab37cf3ac0c3940c17) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c5.bin", 0x2000000, 0x800000, CRC(9817c082) SHA1(1bea9c7220c2b1524896c86841d6d8fd55f5d366) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c6.bin", 0x2000001, 0x800000, CRC(2bc0307f) SHA1(8090fa82c46eb503832359093c8cc3cee3141c90) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c7.bin", 0x3000000, 0x800000, CRC(4358d7b9) SHA1(9270b58c2abc072a046bedda72f1395df26d0714) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c8.bin", 0x3000001, 0x800000, CRC(366deee5) SHA1(d477ad7a5987fd6c7ef2c1680fbb7c884654590e) ) /* Plane 2,3 */
ROM_END

ROM_START( svcplusa )
	ROM_REGION( 0x600000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "svc-p1pl.bin", 0x000000, 0x200000, CRC(16b44144) SHA1(5eab530274b1b6f480a39a86c199da524cddfccc) )
	ROM_LOAD16_WORD_SWAP( "svc-p2pl.bin", 0x200000, 0x400000, CRC(7231ace2) SHA1(d2f13ddd5d3ee29b4b9824e8663f7ee0241f30cf) )

	ROM_REGION( 0x20000, "fixed", 0 )
	ROM_LOAD( "svc-s1pl.bin", 0x10000, 0x10000, CRC(ca3c735e) SHA1(aebd15253c90432a2e0a4c40f37110c1e2176ee4) )
	ROM_CONTINUE( 0x00000, 0x10000 )
	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) )

	NEOGEO_BIOS

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "svc-m1.bin", 0x20000, 0x10000, CRC(804328c3) SHA1(f931636c563b0789d4812033a77b47bf663db43f) )
	ROM_CONTINUE( 0x00000, 0x10000 )
	ROM_COPY( "audiocpu", 0x00000, 0x10000, 0x10000 )

	ROM_Y_ZOOM

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD16_WORD_SWAP( "svc-v2.bin", 0x000000, 0x400000, CRC(b5097287) SHA1(3ba3a9b5624879616382ed40337a3d9c50a0f314) )
	ROM_LOAD16_WORD_SWAP( "svc-v1.bin", 0x400000, 0x400000, CRC(bd3a391f) SHA1(972bf09b75e99a683ee965bec93b0da8f15d72d9) )
	ROM_LOAD16_WORD_SWAP( "svc-v4.bin", 0x800000, 0x400000, CRC(33fc0b37) SHA1(d61017d829f44c7df8795ba10c55c727d9972662) )
	ROM_LOAD16_WORD_SWAP( "svc-v3.bin", 0xc00000, 0x400000, CRC(aa9849a0) SHA1(9539b3356a070a066a89f27c287f316e7367ce2a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "svc-c1.bin", 0x0000000, 0x800000, CRC(a7826b89) SHA1(3bbe348ce54b80b56ef032ea532a18ef3cafeb11) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c2.bin", 0x0000001, 0x800000, CRC(ed3c2089) SHA1(b5d17692f15f5a678c273589fab2e3918711135e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c3.bin", 0x1000000, 0x800000, CRC(71ed8063) SHA1(ea1df9e2e382a8560a06d447421844cc588f43dd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c4.bin", 0x1000001, 0x800000, CRC(250bde2d) SHA1(8c72dcfceef6d022ab4b73ab37cf3ac0c3940c17) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c5.bin", 0x2000000, 0x800000, CRC(9817c082) SHA1(1bea9c7220c2b1524896c86841d6d8fd55f5d366) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c6.bin", 0x2000001, 0x800000, CRC(2bc0307f) SHA1(8090fa82c46eb503832359093c8cc3cee3141c90) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c7.bin", 0x3000000, 0x800000, CRC(4358d7b9) SHA1(9270b58c2abc072a046bedda72f1395df26d0714) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c8.bin", 0x3000001, 0x800000, CRC(366deee5) SHA1(d477ad7a5987fd6c7ef2c1680fbb7c884654590e) ) /* Plane 2,3 */
ROM_END

ROM_START( svcsplus )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "svc-p1sp.bin", 0x000000, 0x400000, CRC(2601902f) SHA1(202348a13c6480f7de37a3ee983823838822fc98) )
	ROM_LOAD16_WORD_SWAP( "svc-p2sp.bin", 0x400000, 0x400000, CRC(0ca13305) SHA1(ac8fbca71b754acbcdd11802161a62ae1cf32d88) )

	NEO_SFIX_128K( "svc-s1sp.bin", CRC(233d6439) SHA1(369024c7a2405c3144c14ac016c07c3dc0f44187) )

	NEOGEO_BIOS

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) )

	ROM_REGION( 0x50000, "audiocpu", 0 )
	ROM_LOAD( "svc-m1.bin", 0x20000, 0x10000, CRC(804328c3) SHA1(f931636c563b0789d4812033a77b47bf663db43f) )
	ROM_CONTINUE( 0x00000, 0x10000 )
	ROM_COPY( "audiocpu", 0x00000, 0x10000, 0x10000 )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD16_WORD_SWAP( "svc-v2.bin", 0x000000, 0x400000, CRC(b5097287) SHA1(3ba3a9b5624879616382ed40337a3d9c50a0f314) )
	ROM_LOAD16_WORD_SWAP( "svc-v1.bin", 0x400000, 0x400000, CRC(bd3a391f) SHA1(972bf09b75e99a683ee965bec93b0da8f15d72d9) )
	ROM_LOAD16_WORD_SWAP( "svc-v4.bin", 0x800000, 0x400000, CRC(33fc0b37) SHA1(d61017d829f44c7df8795ba10c55c727d9972662) )
	ROM_LOAD16_WORD_SWAP( "svc-v3.bin", 0xc00000, 0x400000, CRC(aa9849a0) SHA1(9539b3356a070a066a89f27c287f316e7367ce2a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "svc-c1.bin", 0x0000000, 0x800000, CRC(a7826b89) SHA1(3bbe348ce54b80b56ef032ea532a18ef3cafeb11) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c2.bin", 0x0000001, 0x800000, CRC(ed3c2089) SHA1(b5d17692f15f5a678c273589fab2e3918711135e) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c3.bin", 0x1000000, 0x800000, CRC(71ed8063) SHA1(ea1df9e2e382a8560a06d447421844cc588f43dd) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c4.bin", 0x1000001, 0x800000, CRC(250bde2d) SHA1(8c72dcfceef6d022ab4b73ab37cf3ac0c3940c17) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c5.bin", 0x2000000, 0x800000, CRC(9817c082) SHA1(1bea9c7220c2b1524896c86841d6d8fd55f5d366) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c6.bin", 0x2000001, 0x800000, CRC(2bc0307f) SHA1(8090fa82c46eb503832359093c8cc3cee3141c90) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "svc-c7.bin", 0x3000000, 0x800000, CRC(4358d7b9) SHA1(9270b58c2abc072a046bedda72f1395df26d0714) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "svc-c8.bin", 0x3000001, 0x800000, CRC(366deee5) SHA1(d477ad7a5987fd6c7ef2c1680fbb7c884654590e) ) /* Plane 2,3 */
ROM_END

/* Samurai Shodown 5 bootleg */

ROM_START( samsho5b )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "ssv-p2.bin", 0x000000, 0x400000, CRC(5023067f) SHA1(b1d682fa7d158f19664356a919da6572e8cfeee0) )
	ROM_LOAD16_WORD_SWAP( "ssv-p1.bin", 0x400000, 0x400000, CRC(b6cbe386) SHA1(99c2407361116c2b2c5fe72df53e05c5f99163c1) )

	NEO_SFIX_128K( "ssv-s1.bin", CRC(70f667d0) SHA1(6d7ce62bb77eb215cc22d6c3c677accfd740aa83) )

	NEO_BIOS_AUDIO_128K( "ssv-m1.bin", CRC(18114fb1) SHA1(016dc2f328340f3637a9bff373a20973df29f6b8) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	ROM_LOAD( "ssv-v1.bin", 0x000000, 0x400000, CRC(a3609761) SHA1(6dce1dbfd228c739b3716ae1cf08fd7f925d8650) )
	ROM_LOAD( "ssv-v2.bin", 0x400000, 0x400000, CRC(cbd6ebd5) SHA1(00211be3fa32035b0947ac65920ea8acae7bfae2) )
	ROM_LOAD( "ssv-v3.bin", 0x800000, 0x400000, CRC(6f1c2703) SHA1(8015df3d788cb7926ebbcda64a96964fe102ba27) )
	ROM_LOAD( "ssv-v4.bin", 0xc00000, 0x400000, CRC(5020c055) SHA1(bd1e68d1b0a47b0e2b365159e210048f8b22823a) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	ROM_LOAD16_BYTE( "ssv-c1.bin", 0x0000000, 0x1000000, CRC(9c564a01) SHA1(99dc8900fd8f56ae04fff72b34ddcaa8abe4c1be) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "ssv-c2.bin", 0x0000001, 0x1000000, CRC(4b73b8da) SHA1(a8b626de74cf57bbd8c222e8e24c953c9e8680f4) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "ssv-c3.bin", 0x2000000, 0x1000000, CRC(029f9bb5) SHA1(6296c879aa0bbd22383ceeeac0326805cbc8b4ec) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "ssv-c4.bin", 0x2000001, 0x1000000, CRC(75722430) SHA1(30594c30a167e75463670249df7744755e39e75b) ) /* Plane 2,3 */
ROM_END

/* The King of Fighters 2003 bootlegs */

ROM_START( kf2k3bl )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "2k3-p1.bin", 0x100000, 0x400000, CRC(92ed6ee3) SHA1(5e7e21eb40dfcc453ba73808760d5ddedd49c58a) )
	ROM_LOAD16_WORD_SWAP( "2k3-p2.bin", 0x500000, 0x200000, CRC(5d3d8bb3) SHA1(7f2341f14ca12ff5721eb038b3496228a1f34b60) )
	ROM_CONTINUE( 0x000000, 0x100000 )
	ROM_CONTINUE( 0x000000, 0x100000 )

	NEO_SFIX_128K( "2k3-s1.bin", CRC(482c48a5) SHA1(27e2f5295a9a838e112be28dafc111893a388a16) )

	NEO_BIOS_AUDIO_128K( "2k3-m1.bin", CRC(3a4969ff) SHA1(2fc107a023a82053a8df63025829bcf12cee9610) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1c.bin", 0x000000, 0x800000, CRC(ffa3f8c7) SHA1(7cf4a933973ca23b7f87c81151d8659e6ec4bd20) )
	ROM_LOAD( "271-v2c.bin", 0x800000, 0x800000, CRC(5382c7d1) SHA1(1bf999705eda80ba1e7b0d6bdd010d9bfb18bd76) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "271-c1c.bin", 0x0000000, 0x800000, CRC(b1dc25d0) SHA1(50adc3c60d5b4b3abd10a49db2267306c6dbd772) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c2c.bin", 0x0000001, 0x800000, CRC(d5362437) SHA1(66db36522dc09106388c707252df9fe1c88b4856) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c3c.bin", 0x1000000, 0x800000, CRC(0a1fbeab) SHA1(9fe30d36ba98d00fda010832ff2f27783dd577c1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c4c.bin", 0x1000001, 0x800000, CRC(87b19a0c) SHA1(b72a8e7d9124ce859b5149bb4381ba481c161ea5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c5c.bin", 0x2000000, 0x800000, CRC(704ea371) SHA1(e75b80422f0d72eac826f8ffadf79efeccaab124) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c6c.bin", 0x2000001, 0x800000, CRC(20a1164c) SHA1(c9843b37612a16fc95f6851793b1cfb5d49d811d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c7c.bin", 0x3000000, 0x800000, CRC(189aba7f) SHA1(7152195a57ad36b28290810fe87ed8c206262ba9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c8c.bin", 0x3000001, 0x800000, CRC(20ec4fdc) SHA1(deb5f7ec5a090e419b9d1a6a74877bee081198e2) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k3bla )
	ROM_REGION( 0x700000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "2k3-p1bl.bin", 0x000000, 0x100000, CRC(4ea414dd) SHA1(c242c9709c20a8cde3ad562adbe640a5dd5abcf1) )
	ROM_LOAD16_WORD_SWAP( "2k3-p3bl.bin", 0x100000, 0x400000, CRC(370acbff) SHA1(e72544de1c5e2e4f7478fc003caba9e33a306c19) )
	ROM_LOAD16_WORD_SWAP( "2k3-p2bl.bin", 0x500000, 0x200000, CRC(9c04fc52) SHA1(f41b53c79e4209373ec68276fa5941c91424bb15) )

	NEO_SFIX_128K( "2k3-s1.bin", CRC(482c48a5) SHA1(27e2f5295a9a838e112be28dafc111893a388a16) )

	NEO_BIOS_AUDIO_128K( "2k3-m1.bin", CRC(3a4969ff) SHA1(2fc107a023a82053a8df63025829bcf12cee9610) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1c.bin", 0x000000, 0x800000, CRC(ffa3f8c7) SHA1(7cf4a933973ca23b7f87c81151d8659e6ec4bd20) )
	ROM_LOAD( "271-v2c.bin", 0x800000, 0x800000, CRC(5382c7d1) SHA1(1bf999705eda80ba1e7b0d6bdd010d9bfb18bd76) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "271-c1c.bin", 0x0000000, 0x800000, CRC(b1dc25d0) SHA1(50adc3c60d5b4b3abd10a49db2267306c6dbd772) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c2c.bin", 0x0000001, 0x800000, CRC(d5362437) SHA1(66db36522dc09106388c707252df9fe1c88b4856) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c3c.bin", 0x1000000, 0x800000, CRC(0a1fbeab) SHA1(9fe30d36ba98d00fda010832ff2f27783dd577c1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c4c.bin", 0x1000001, 0x800000, CRC(87b19a0c) SHA1(b72a8e7d9124ce859b5149bb4381ba481c161ea5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c5c.bin", 0x2000000, 0x800000, CRC(704ea371) SHA1(e75b80422f0d72eac826f8ffadf79efeccaab124) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c6c.bin", 0x2000001, 0x800000, CRC(20a1164c) SHA1(c9843b37612a16fc95f6851793b1cfb5d49d811d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c7c.bin", 0x3000000, 0x800000, CRC(189aba7f) SHA1(7152195a57ad36b28290810fe87ed8c206262ba9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c8c.bin", 0x3000001, 0x800000, CRC(20ec4fdc) SHA1(deb5f7ec5a090e419b9d1a6a74877bee081198e2) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k3pl )
	ROM_REGION( 0x700000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "2k3-p1pl.bin", 0x000000, 0x100000, CRC(07b84112) SHA1(0b085a928a39ff9c0745a58bfa4ce6106b5f474a) )
	ROM_LOAD16_WORD_SWAP( "2k3-p3bl.bin", 0x100000, 0x400000, CRC(370acbff) SHA1(e72544de1c5e2e4f7478fc003caba9e33a306c19) )
	ROM_LOAD16_WORD_SWAP( "2k3-p2bl.bin", 0x500000, 0x200000, CRC(9c04fc52) SHA1(f41b53c79e4209373ec68276fa5941c91424bb15) )

	NEO_SFIX_128K( "2k3-s1pl.bin", CRC(ad548a36) SHA1(7483dbe2d74a1bd1b4dc501e99e48a683416d08e) )

	NEO_BIOS_AUDIO_128K( "2k3-m1.bin", CRC(3a4969ff) SHA1(2fc107a023a82053a8df63025829bcf12cee9610) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1c.bin", 0x000000, 0x800000, CRC(ffa3f8c7) SHA1(7cf4a933973ca23b7f87c81151d8659e6ec4bd20) )
	ROM_LOAD( "271-v2c.bin", 0x800000, 0x800000, CRC(5382c7d1) SHA1(1bf999705eda80ba1e7b0d6bdd010d9bfb18bd76) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "271-c1c.bin", 0x0000000, 0x800000, CRC(b1dc25d0) SHA1(50adc3c60d5b4b3abd10a49db2267306c6dbd772) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c2c.bin", 0x0000001, 0x800000, CRC(d5362437) SHA1(66db36522dc09106388c707252df9fe1c88b4856) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c3c.bin", 0x1000000, 0x800000, CRC(0a1fbeab) SHA1(9fe30d36ba98d00fda010832ff2f27783dd577c1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c4c.bin", 0x1000001, 0x800000, CRC(87b19a0c) SHA1(b72a8e7d9124ce859b5149bb4381ba481c161ea5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c5c.bin", 0x2000000, 0x800000, CRC(704ea371) SHA1(e75b80422f0d72eac826f8ffadf79efeccaab124) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c6c.bin", 0x2000001, 0x800000, CRC(20a1164c) SHA1(c9843b37612a16fc95f6851793b1cfb5d49d811d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c7c.bin", 0x3000000, 0x800000, CRC(189aba7f) SHA1(7152195a57ad36b28290810fe87ed8c206262ba9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c8c.bin", 0x3000001, 0x800000, CRC(20ec4fdc) SHA1(deb5f7ec5a090e419b9d1a6a74877bee081198e2) ) /* Plane 2,3 */
ROM_END

ROM_START( kf2k3upl )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "2k3-p1up.bin", 0x000000, 0x800000, CRC(87294c01) SHA1(21420415a6b2ba1b43ecc1934270dc085d6bd7d9) )

	NEO_SFIX_128K( "2k3-s1up.bin", CRC(e5708c0c) SHA1(5649446d3b0b1bd138b5a8b40b96a6d0f892f4d8) )

	NEO_BIOS_AUDIO_128K( "2k3-m1.bin", CRC(3a4969ff) SHA1(2fc107a023a82053a8df63025829bcf12cee9610) )

	ROM_REGION( 0x1000000, "ymsnd", 0 )
	/* Encrypted */
	ROM_LOAD( "271-v1c.bin", 0x000000, 0x800000, CRC(ffa3f8c7) SHA1(7cf4a933973ca23b7f87c81151d8659e6ec4bd20) )
	ROM_LOAD( "271-v2c.bin", 0x800000, 0x800000, CRC(5382c7d1) SHA1(1bf999705eda80ba1e7b0d6bdd010d9bfb18bd76) )

	NO_DELTAT_REGION

	ROM_REGION( 0x4000000, "sprites", 0 )
	/* Encrypted */
	ROM_LOAD16_BYTE( "271-c1c.bin", 0x0000000, 0x800000, CRC(b1dc25d0) SHA1(50adc3c60d5b4b3abd10a49db2267306c6dbd772) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c2c.bin", 0x0000001, 0x800000, CRC(d5362437) SHA1(66db36522dc09106388c707252df9fe1c88b4856) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c3c.bin", 0x1000000, 0x800000, CRC(0a1fbeab) SHA1(9fe30d36ba98d00fda010832ff2f27783dd577c1) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c4c.bin", 0x1000001, 0x800000, CRC(87b19a0c) SHA1(b72a8e7d9124ce859b5149bb4381ba481c161ea5) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c5c.bin", 0x2000000, 0x800000, CRC(704ea371) SHA1(e75b80422f0d72eac826f8ffadf79efeccaab124) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c6c.bin", 0x2000001, 0x800000, CRC(20a1164c) SHA1(c9843b37612a16fc95f6851793b1cfb5d49d811d) ) /* Plane 2,3 */
	ROM_LOAD16_BYTE( "271-c7c.bin", 0x3000000, 0x800000, CRC(189aba7f) SHA1(7152195a57ad36b28290810fe87ed8c206262ba9) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "271-c8c.bin", 0x3000001, 0x800000, CRC(20ec4fdc) SHA1(deb5f7ec5a090e419b9d1a6a74877bee081198e2) ) /* Plane 2,3 */
ROM_END


	/* Unlicensed Prototype */


ROM_START( diggerma ) /* Unlicensed Prototype, no official game ID # */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "dig-p1.bin", 0x000000, 0x080000, CRC(eda433d7) SHA1(abb14c66777ab0fe4ac76a402e253a49df7178d8) )

	NEO_SFIX_64K( "dig-s1.bin", CRC(75a88c1f) SHA1(295dd9225f1e3d2fc64a65b3c287c7f1765417b1) )

	NEO_BIOS_AUDIO_64K( "dig-m1.bin", CRC(833cdf1b) SHA1(3a92c79adbe0d37956ea46a4746d6f1cbf7d2c14) )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "dig-v1.bin", 0x000000, 0x080000, CRC(ee15bda4) SHA1(fe2206728e6efd02d6302869a98b196eb19a17df) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "dig-c1.bin", 0x000000, 0x080000, CRC(3db0a4ed) SHA1(6214faa883d97ea05809b6af7e0c85a236a18a28) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "dig-c2.bin", 0x000001, 0x080000, CRC(3e632161) SHA1(83711c4286fb1d9f3f91414ac6e5fed36618033e) ) /* Plane 2,3 */
ROM_END

/* NeoPrint

Very little is known about the NeoPrint system at this time.

The cartridges fit in a normal NeoGeo unit but they do not work.  They appear to be designed to work with a
different motherboard (and different bios rom)

98best44 is the only NeoPrint cart we've seen with a program rom.
Many of the others only contain graphic roms.  I'm speculating that the NeoPrint was a multi-slot system and
had a concept of Master and Slave cartridges, and the slave ones simply added new graphic themes.

If anybody has any actual factual information on the NeoPrint system, please contribute it.

*/

/*

logo: Neo Print

small text: Cassette supporting Neo Print and Neo Print Multi

(cassette=cartridge)

title: '98 NeoPri Best 44 version

*/

ROM_START( 98best44 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p060-ep1", 0x000000, 0x080000, CRC(d42e505d) SHA1(0ad6b0288f36c339832730a03e53cbc07dab4f82))

	ROM_REGION16_BE( 0x80000, "mainbios", 0 )
	ROM_LOAD16_WORD_SWAP( "neoprint.bios",    0x00000, 0x020000, NO_DUMP ) // should be different
	ROM_COPY( "maincpu", 0x00000, 0x00000, 0x80000 ) // just copy the program rom to stop it crashing

	ROM_REGION( 0x20000, "fixedbios", 0 )
	ROM_LOAD( "sfix.sfix", 0x000000, 0x20000, CRC(c2ea0cfd) SHA1(fd4a618cdcdbf849374f0a50dd8efe9dbab706c3) ) // maybe

	ROM_Y_ZOOM // maybe

	ROM_REGION( 0x20000, "audiobios", 0 )
	ROM_LOAD( "sm1.sm1", 0x00000, 0x20000, CRC(94416d67) SHA1(42f9d7ddd6c0931fd64226a60dc73602b2819dcf) ) // maybe

	ROM_REGION( 0x20000, "fixed", ROMREGION_ERASE00 )
	/* no S1 rom */

	ROM_REGION( 0x30000, "audiocpu", 0 )
	ROM_LOAD( "pt004-m1",	 0x00000, 0x20000, CRC(6d77cdaa) SHA1(f88a93b3085b18b6663b4e51fccaa41958aafae1) )
	ROM_RELOAD(          0x10000, 0x20000 )

	ROM_REGION( 0x200000, "ymsnd", 0 )
	ROM_LOAD( "pt004-v1", 0x000000, 0x200000, CRC(118a84fd) SHA1(9059297a42a329eca47a82327c301853219013bd) )

	NO_DELTAT_REGION

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD16_BYTE( "pt060-c1", 0x000000, 0x100000, CRC(22a23090) SHA1(0e219fcfea6ca2ddf4b7b4197aac8bc55a29d5cf) ) /* Plane 0,1 */
	ROM_LOAD16_BYTE( "pt060-c2", 0x000001, 0x100000, CRC(66a8e56a) SHA1(adfd1e52d52806a785f1e9b1ae2ac969b6ed60af) ) /* Plane 2,3 */
ROM_END


/* Game specific inits */

static DRIVER_INIT( fatfury2 )
{
	DRIVER_INIT_CALL(neogeo);
	fatfury2_install_protection(machine);
}

static DRIVER_INIT ( kof98 )
{
	DRIVER_INIT_CALL(neogeo);
	kof98_decrypt_68k(machine);
	install_kof98_protection(machine);
}

static DRIVER_INIT( mslugx )
{
	DRIVER_INIT_CALL(neogeo);
	mslugx_install_protection(machine);
}

static DRIVER_INIT( kof99 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	kof99_decrypt_68k(machine);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x00);
	kof99_install_protection(machine);
}

static DRIVER_INIT( kof99n )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x00);
}

static DRIVER_INIT( garou )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	garou_decrypt_68k(machine);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x06);
	garou_install_protection(machine);
}

static DRIVER_INIT( garouo )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	garouo_decrypt_68k(machine);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x06);
	garouo_install_protection(machine);
}

static DRIVER_INIT( garoubl )
{
	DRIVER_INIT_CALL(neogeo);
	neogeo_bootleg_sx_decrypt(machine, 2);
	neogeo_bootleg_cx_decrypt(machine);
}

static DRIVER_INIT( mslug3 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	mslug3_decrypt_68k(machine);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xad);
	mslug3_install_protection(machine);
}

static DRIVER_INIT( mslug3h )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xad);
}

static DRIVER_INIT( mslug3b6 )
{
	DRIVER_INIT_CALL(neogeo);
	neogeo_bootleg_sx_decrypt(machine, 2);
	cmc42_neogeo_gfx_decrypt(machine, 0xad);
}

static DRIVER_INIT( kof2000 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	kof2000_decrypt_68k(machine);
	state->fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x00);
	kof2000_install_protection(machine);
}

static DRIVER_INIT( kof2000n )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x00);
}

static DRIVER_INIT( kof2001 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof2000_neogeo_gfx_decrypt(machine, 0x1e);
	neogeo_cmc50_m1_decrypt(machine);
}

static DRIVER_INIT( cthd2003 )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_cthd2003(machine);
	patch_cthd2003(machine);
}

static DRIVER_INIT ( ct2k3sp )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_ct2k3sp(machine);
	patch_cthd2003(machine);
}

static DRIVER_INIT ( ct2k3sa )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_ct2k3sa(machine);
	patch_ct2k3sa(machine);
}

static DRIVER_INIT( mslug4 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1; /* USA violent content screen is wrong -- not a bug, confirmed on real hardware! */
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x31);
	neo_pcm2_snk_1999(machine, 8);
}

static DRIVER_INIT( ms4plus )
{
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x31);
	neo_pcm2_snk_1999(machine, 8);
	neogeo_cmc50_m1_decrypt(machine);
}

static DRIVER_INIT( ganryu )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x07);
}

static DRIVER_INIT( s1945p )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x05);
}

static DRIVER_INIT( preisle2 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0x9f);
}

static DRIVER_INIT( bangbead )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xf8);
}

static DRIVER_INIT( nitd )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xff);
}

static DRIVER_INIT( zupapa )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xbd);
}

static DRIVER_INIT( sengoku3 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	kof99_neogeo_gfx_decrypt(machine, 0xfe);
}

static DRIVER_INIT( rotd )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	neo_pcm2_snk_1999(machine, 16);
	state->fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x3f);
}

static DRIVER_INIT( kof2002 )
{
	DRIVER_INIT_CALL(neogeo);
	kof2002_decrypt_68k(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0xec);
}

static DRIVER_INIT( kof2002b )
{
	DRIVER_INIT_CALL(neogeo);
	kof2002_decrypt_68k(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	kof2002b_gfx_decrypt(machine, memory_region(machine, "sprites"),0x4000000);
	kof2002b_gfx_decrypt(machine, memory_region(machine, "fixed"),0x20000);
}

static DRIVER_INIT( kf2k2pls )
{
	DRIVER_INIT_CALL(neogeo);
	kof2002_decrypt_68k(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	cmc50_neogeo_gfx_decrypt(machine, 0xec);
}

static DRIVER_INIT( kf2k2mp )
{
	DRIVER_INIT_CALL(neogeo);
	kf2k2mp_decrypt(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 2);
	cmc50_neogeo_gfx_decrypt(machine, 0xec);
}

static DRIVER_INIT( kf2k2mp2 )
{
	DRIVER_INIT_CALL(neogeo);
	kf2k2mp2_px_decrypt(machine);
	neo_pcm2_swap(machine, 0);
	neogeo_cmc50_m1_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	cmc50_neogeo_gfx_decrypt(machine, 0xec);
}

static DRIVER_INIT( kof10th )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_kof10th(machine);
	install_kof10th_protection(machine);
}

static DRIVER_INIT( kf10thep )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_kf10thep(machine);
}

static DRIVER_INIT( kf2k5uni )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_kf2k5uni(machine);
}

static DRIVER_INIT( kof2k4se )
{
	DRIVER_INIT_CALL(neogeo);
	decrypt_kof2k4se_68k(machine);
}

static DRIVER_INIT( matrim )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	matrim_decrypt_68k(machine);
	neo_pcm2_swap(machine, 1);
	state->fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x6a);
}

static DRIVER_INIT( matrimbl )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	matrim_decrypt_68k(machine);
	state->fixed_layer_bank_type = 2;
	matrimbl_decrypt(machine);
	neogeo_sfix_decrypt(machine); /* required for text layer */
}

static DRIVER_INIT( pnyaa )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	neo_pcm2_snk_1999(machine, 4);
	state->fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x2e);
}

static DRIVER_INIT( mslug5 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	mslug5_decrypt_68k(machine);
	neo_pcm2_swap(machine, 2);
	state->fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x19);
	install_pvc_protection(machine);
}

static TIMER_CALLBACK( ms5pcb_bios_timer_callback )
{
	int harddip3 = input_port_read(machine, "HARDDIP") & 1;
	memory_set_bankptr(machine, NEOGEO_BANK_BIOS, memory_region(machine, "mainbios") + 0x20000 + harddip3 * 0x20000);
}

static DRIVER_INIT( ms5pcb )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);

	/* start a timer that will check the BIOS select DIP every second */
	timer_set(machine, attotime_zero, NULL, 0, ms5pcb_bios_timer_callback);
	timer_pulse(machine, ATTOTIME_IN_MSEC(1000), NULL, 0, ms5pcb_bios_timer_callback);

	mslug5_decrypt_68k(machine);
	svcpcb_gfx_decrypt(machine);
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x19);
	state->fixed_layer_bank_type = 2;
	svcpcb_s1data_decrypt(machine);
	neo_pcm2_swap(machine, 2);
	install_pvc_protection(machine);
}

static DRIVER_INIT( ms5plus )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x19);
	neo_pcm2_swap(machine, 2);
	neogeo_bootleg_sx_decrypt(machine, 1);
	state->fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	install_ms5plus_protection(machine);
}

static TIMER_CALLBACK( svcpcb_bios_timer_callback )
{
	int harddip3 = input_port_read(machine, "HARDDIP") & 1;
	memory_set_bankptr(machine, NEOGEO_BANK_BIOS, memory_region(machine, "mainbios") + 0x20000 + harddip3 * 0x20000);
}

static DRIVER_INIT( svcpcb )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);

	/* start a timer that will check the BIOS select DIP every second */
	timer_set(machine, attotime_zero, NULL, 0, svcpcb_bios_timer_callback);
	timer_pulse(machine, ATTOTIME_IN_MSEC(1000), NULL, 0, svcpcb_bios_timer_callback);

	svc_px_decrypt(machine);
	svcpcb_gfx_decrypt(machine);
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x57);
	svcpcb_s1data_decrypt(machine);
	neo_pcm2_swap(machine, 3);
	state->fixed_layer_bank_type = 2;
	install_pvc_protection(machine);
}

static DRIVER_INIT( svc )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	svc_px_decrypt(machine);
	neo_pcm2_swap(machine, 3);
	state->fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x57);
	install_pvc_protection(machine);
}

static DRIVER_INIT( svcboot )
{
	DRIVER_INIT_CALL(neogeo);
	svcboot_px_decrypt(machine);
	svcboot_cx_decrypt(machine);
	install_pvc_protection(machine);
}

static DRIVER_INIT( svcplus )
{
	DRIVER_INIT_CALL(neogeo);
	svcplus_px_decrypt(machine);
	svcboot_cx_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	svcplus_px_hack(machine);
}

static DRIVER_INIT( svcplusa )
{
	DRIVER_INIT_CALL(neogeo);
	svcplusa_px_decrypt(machine);
	svcboot_cx_decrypt(machine);
	svcplus_px_hack(machine);
}

static DRIVER_INIT( svcsplus )
{
	DRIVER_INIT_CALL(neogeo);
	svcsplus_px_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 2);
	svcboot_cx_decrypt(machine);
	svcsplus_px_hack(machine);
	install_pvc_protection(machine);
}

static DRIVER_INIT( samsho5 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	samsho5_decrypt_68k(machine);
	neo_pcm2_swap(machine, 4);
	state->fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x0f);
}

static DRIVER_INIT( samsho5b )
{
	DRIVER_INIT_CALL(neogeo);
	samsho5b_px_decrypt(machine);
	samsho5b_vx_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	neogeo_bootleg_cx_decrypt(machine);
}

static DRIVER_INIT( kf2k3pcb )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	kf2k3pcb_decrypt_68k(machine);
	kf2k3pcb_gfx_decrypt(machine);
	kof2003biosdecode(machine);
	neogeo_cmc50_m1_decrypt(machine);

	/* extra little swap on the m1 - this must be performed AFTER the m1 decrypt
       or the m1 checksum (used to generate the key) for decrypting the m1 is
       incorrect */
	{
		int i;
		UINT8* rom = memory_region(machine, "audiocpu");
		for (i = 0; i < 0x90000; i++)
		{
			rom[i] = BITSWAP8(rom[i], 5, 6, 1, 4, 3, 0, 7, 2);
		}

	}

	kof2000_neogeo_gfx_decrypt(machine, 0x9d);
	kf2k3pcb_decrypt_s1data(machine);
	neo_pcm2_swap(machine, 5);
	state->fixed_layer_bank_type = 2;
	install_pvc_protection(machine);
	memory_install_read_bank(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xc00000, 0xc7ffff, 0, 0, "bios" );  // 512k bios
}

static DRIVER_INIT( kof2003 )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	kof2003_decrypt_68k(machine);
	neo_pcm2_swap(machine, 5);
	state->fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x9d);
	install_pvc_protection(machine);
}

static DRIVER_INIT( kof2003h )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	kof2003h_decrypt_68k(machine);
	neo_pcm2_swap(machine, 5);
	state->fixed_layer_bank_type = 2;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x9d);
	install_pvc_protection(machine);
}

static DRIVER_INIT( kf2k3bl )
{
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x9d);
	neo_pcm2_swap(machine, 5);
	neogeo_bootleg_sx_decrypt(machine, 1);
	kf2k3bl_install_protection(machine);
}

static DRIVER_INIT( kf2k3pl )
{
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x9d);
	neo_pcm2_swap(machine, 5);
	kf2k3pl_px_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	kf2k3pl_install_protection(machine);
}

static DRIVER_INIT( kf2k3upl )
{
	DRIVER_INIT_CALL(neogeo);
	cmc50_neogeo_gfx_decrypt(machine, 0x9d);
	neo_pcm2_swap(machine, 5);
	kf2k3upl_px_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 2);
	kf2k3upl_install_protection(machine);
}

static DRIVER_INIT( samsh5sp )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	samsh5sp_decrypt_68k(machine);
	neo_pcm2_swap(machine, 6);
	state->fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0x0d);
}

static DRIVER_INIT( jockeygp )
{
	neogeo_state *state = (neogeo_state *)machine->driver_data;
	DRIVER_INIT_CALL(neogeo);
	state->fixed_layer_bank_type = 1;
	neogeo_cmc50_m1_decrypt(machine);
	kof2000_neogeo_gfx_decrypt(machine, 0xac);

	/* install some extra RAM */
	memory_install_ram(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x201fff, 0, 0, NULL);

//  memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x280000, 0x280001, 0, 0, "IN5");
//  memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x2c0000, 0x2c0001, 0, 0, "IN6");
}

static DRIVER_INIT( vliner )
{
	memory_install_ram(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x200000, 0x201fff, 0, 0, NULL);

	memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x280000, 0x280001, 0, 0, "IN5");
	memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x2c0000, 0x2c0001, 0, 0, "IN6");

	DRIVER_INIT_CALL(neogeo);
}

static DRIVER_INIT( kog )
{
	/* overlay cartridge ROM */
	memory_install_read_port(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x0ffffe, 0x0fffff, 0, 0, "JUMPER");

	kog_px_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	neogeo_bootleg_cx_decrypt(machine);
	DRIVER_INIT_CALL(neogeo);
}

static DRIVER_INIT( lans2004 )
{
	lans2004_decrypt_68k(machine);
	lans2004_vx_decrypt(machine);
	neogeo_bootleg_sx_decrypt(machine, 1);
	neogeo_bootleg_cx_decrypt(machine);
	DRIVER_INIT_CALL(neogeo);
}

/***********************************************************************/


/*    YEAR  NAME       PARENT    MACHINE   INPUT     INIT      MONITOR  */
GAME( 1990, neogeo,    0,        neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Neo-Geo", GAME_IS_BIOS_ROOT | GAME_SUPPORTS_SAVE )

/* SNK */
GAME( 1990, nam1975,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "NAM-1975", GAME_SUPPORTS_SAVE )
GAME( 1990, bstars,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Baseball Stars Professional (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1990, bstarsh,   bstars,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Baseball Stars Professional (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1990, tpgolf,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Top Player's Golf", GAME_SUPPORTS_SAVE )
GAME( 1990, mahretsu,  neogeo,   neogeo,   mjneogeo, neogeo,   ROT0, "SNK", "Mahjong Kyoretsuden", GAME_SUPPORTS_SAVE )
GAME( 1990, ridhero,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Riding Hero (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1990, ridheroh,  ridhero,  neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Riding Hero (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1991, alpham2,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Alpha Mission II / ASO II - Last Guardian", GAME_SUPPORTS_SAVE )
GAME( 1990, cyberlip,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Cyber-Lip", GAME_SUPPORTS_SAVE )
GAME( 1990, superspy,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The Super Spy", GAME_SUPPORTS_SAVE )
GAME( 1992, mutnat,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Mutation Nation", GAME_SUPPORTS_SAVE )
GAME( 1991, kotm,      neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "King of the Monsters (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1991, kotmh,     kotm,     neogeo,   neogeo,   neogeo,   ROT0, "SNK", "King of the Monsters (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1991, sengoku,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Sengoku / Sengoku Denshou (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1991, sengokh,   sengoku,  neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Sengoku / Sengoku Denshou (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1991, burningf,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Burning Fight (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1991, burningfh, burningf, neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Burning Fight (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1990, lbowling,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "League Bowling", GAME_SUPPORTS_SAVE )
GAME( 1991, gpilots,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Ghost Pilots (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1991, gpilotsh,  gpilots,  neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Ghost Pilots (AES cart)", GAME_SUPPORTS_SAVE )
GAME( 1990, joyjoy,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Puzzled / Joy Joy Kid", GAME_SUPPORTS_SAVE )
GAME( 1991, quizdais,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Quiz Daisousa Sen - The Last Count Down", GAME_SUPPORTS_SAVE )
GAME( 1992, lresort,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Last Resort", GAME_SUPPORTS_SAVE )
GAME( 1991, eightman,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK / Pallas", "Eight Man", GAME_SUPPORTS_SAVE )
GAME( 1991, legendos,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Legend of Success Joe / Ashitano Joe Densetsu", GAME_SUPPORTS_SAVE )
GAME( 1991, 2020bb,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK / Pallas", "2020 Super Baseball (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1991, 2020bba,   2020bb,   neogeo,   neogeo,   neogeo,   ROT0, "SNK / Pallas", "2020 Super Baseball (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1991, 2020bbh,   2020bb,   neogeo,   neogeo,   neogeo,   ROT0, "SNK / Pallas", "2020 Super Baseball (set 3)", GAME_SUPPORTS_SAVE )
GAME( 1991, socbrawl,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Soccer Brawl (AES cart)", GAME_SUPPORTS_SAVE )
GAME( 1991, socbrawla, socbrawl, neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Soccer Brawl (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1991, fatfury1,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Fatal Fury - King of Fighters / Garou Densetsu - shukumei no tatakai", GAME_SUPPORTS_SAVE )
GAME( 1991, roboarmy,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Robo Army", GAME_SUPPORTS_SAVE )
//GAME( 1991, roboarma,  roboarmy, neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Robo Army (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1992, fbfrenzy,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Football Frenzy", GAME_SUPPORTS_SAVE )
GAME( 1992, kotm2,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "King of the Monsters 2 - The Next Thing", GAME_SUPPORTS_SAVE )
GAME( 1993, sengoku2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Sengoku 2 / Sengoku Denshou 2", GAME_SUPPORTS_SAVE )
GAME( 1992, bstars2,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Baseball Stars 2", GAME_SUPPORTS_SAVE )
GAME( 1992, quizdai2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Quiz Meitantei Neo & Geo - Quiz Daisousa Sen part 2", GAME_SUPPORTS_SAVE )
GAME( 1993, 3countb,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "3 Count Bout / Fire Suplex", GAME_SUPPORTS_SAVE )
GAME( 1992, aof,       neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Art of Fighting / Ryuuko no Ken", GAME_SUPPORTS_SAVE )
GAME( 1993, samsho,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Samurai Shodown / Samurai Spirits (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1993, samshoh,   samsho,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Samurai Shodown / Samurai Spirits (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1994, tophuntr,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Top Hunter - Roddy & Cathy (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1994, tophuntrh, tophuntr, neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Top Hunter - Roddy & Cathy (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1992, fatfury2,  neogeo,   neogeo,   neogeo,   fatfury2, ROT0, "SNK", "Fatal Fury 2 / Garou Densetsu 2 - arata-naru tatakai", GAME_SUPPORTS_SAVE )
GAME( 1992, ssideki,   neogeo,   neogeo,   neogeo,   fatfury2, ROT0, "SNK", "Super Sidekicks / Tokuten Ou", GAME_SUPPORTS_SAVE )
GAME( 1994, kof94,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '94", GAME_SUPPORTS_SAVE )
GAME( 1994, aof2,      neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Art of Fighting 2 / Ryuuko no Ken 2 (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1994, aof2a,     aof2,     neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Art of Fighting 2 / Ryuuko no Ken 2 (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1993, fatfursp,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Fatal Fury Special / Garou Densetsu Special (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1993, fatfursa,  fatfursp, neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Fatal Fury Special / Garou Densetsu Special (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1995, savagere,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Savage Reign / Fu'un Mokushiroku - kakutou sousei", GAME_SUPPORTS_SAVE )
GAME( 1994, ssideki2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Super Sidekicks 2 - The World Championship / Tokuten Ou 2 - real fight football", GAME_SUPPORTS_SAVE )
GAME( 1994, samsho2,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Samurai Shodown II / Shin Samurai Spirits - Haohmaru jigokuhen", GAME_SUPPORTS_SAVE )
GAME( 1994, samsho2k,  samsho2,  neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Saulabi Spirits / Jin Saulabi Tu Hon (Korean release of Samurai Shodown II)", GAME_SUPPORTS_SAVE ) // official or hack?
GAME( 1995, fatfury3,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Fatal Fury 3 - Road to the Final Victory / Garou Densetsu 3 - haruka-naru tatakai", GAME_SUPPORTS_SAVE )
GAME( 1995, ssideki3,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Super Sidekicks 3 - The Next Glory / Tokuten Ou 3 - eikoue no michi", GAME_SUPPORTS_SAVE )
GAME( 1995, kof95,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '95 (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1995, kof95h,    kof95,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '95 (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1995, samsho3,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Samurai Shodown III / Samurai Spirits - Zankurou Musouken (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1995, samsho3h,  samsho3,  neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Samurai Shodown III / Samurai Spirits - Zankurou Musouken (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1995, fswords,   samsho3,  neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Fighters Swords (Korean release of Samurai Shodown III)", GAME_SUPPORTS_SAVE )
GAME( 1995, rbff1,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Real Bout Fatal Fury / Real Bout Garou Densetsu", GAME_SUPPORTS_SAVE )
GAME( 1995, rbff1a,    rbff1,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Real Bout Fatal Fury / Real Bout Garou Densetsu (bug fix revision)", GAME_SUPPORTS_SAVE )
GAME( 1996, aof3,      neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Art of Fighting 3 - The Path of the Warrior / Art of Fighting - Ryuuko no Ken Gaiden", GAME_SUPPORTS_SAVE )
GAME( 1996, aof3k,     aof3,     neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Art of Fighting 3 - The Path of the Warrior (Korean release)", GAME_SUPPORTS_SAVE ) // no Japanese title / mode
GAME( 1996, kof96,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '96 (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1996, kof96h,    kof96,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '96 (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1996, ssideki4,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The Ultimate 11 - The SNK Football Championship / Tokuten Ou - Honoo no Libero", GAME_SUPPORTS_SAVE )
GAME( 1996, kizuna,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Kizuna Encounter - Super Tag Battle / Fu'un Super Tag Battle", GAME_SUPPORTS_SAVE )
GAME( 1996, samsho4,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Samurai Shodown IV - Amakusa's Revenge / Samurai Spirits - Amakusa Kourin", GAME_SUPPORTS_SAVE )
GAME( 1996, samsho4k,  samsho4,  neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Pae Wang Jeon Seol / Legend of a Warrior (Korean censored Samurai Shodown IV)", GAME_SUPPORTS_SAVE )
GAME( 1996, rbffspec,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Real Bout Fatal Fury Special / Real Bout Garou Densetsu Special", GAME_SUPPORTS_SAVE )
GAME( 1997, kof97,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '97 (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1997, kof97h,    kof97,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '97 (AES cart)", GAME_SUPPORTS_SAVE )
GAME( 1997, kof97pls,  kof97,    neogeo,   neogeo,   neogeo,   ROT0, "bootleg", "The King of Fighters '97 Plus (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1997, kog,       kof97,    neogeo,   kog,      kog,      ROT0, "bootleg", "King of Gladiator (The King of Fighters '97 bootleg)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE ) // protected bootleg
GAME( 1997, lastblad,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The Last Blade / Bakumatsu Roman - Gekka no Kenshi (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1997, lastbladh, lastblad, neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The Last Blade / Bakumatsu Roman - Gekka no Kenshi (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1997, lastsold,  lastblad, neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The Last Soldier (Korean release of The Last Blade)", GAME_SUPPORTS_SAVE )
GAME( 1997, irrmaze,   neogeo,   neogeo,   irrmaze,  neogeo,   ROT0, "SNK / Saurus", "The Irritating Maze / Ultra Denryu Iraira Bou", GAME_SUPPORTS_SAVE )
GAME( 1998, rbff2,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Real Bout Fatal Fury 2 - The Newcomers / Real Bout Garou Densetsu 2 - the newcomers (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1998, rbff2h,    rbff2,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Real Bout Fatal Fury 2 - The Newcomers / Real Bout Garou Densetsu 2 - the newcomers (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1998, rbff2k,    rbff2,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Real Bout Fatal Fury 2 - The Newcomers (Korean release)", GAME_SUPPORTS_SAVE ) // no Japanese title / mode
GAME( 1998, mslug2,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Metal Slug 2 - Super Vehicle-001/II", GAME_SUPPORTS_SAVE )
GAME( 1998, kof98,     neogeo,   neogeo,   neogeo,   kof98,    ROT0, "SNK", "The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends", GAME_SUPPORTS_SAVE )
GAME( 1998, kof98k,    kof98,    neogeo,   neogeo,   kof98,    ROT0, "SNK", "The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (Korean board)", GAME_SUPPORTS_SAVE )
GAME( 1998, kof98ka,   kof98,    neogeo,   neogeo,   kof98,    ROT0, "SNK", "The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (Korean board 2)", GAME_SUPPORTS_SAVE )
GAME( 1998, kof98h,    kof98,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '98 - The Slugfest / King of Fighters '98 - dream match never ends (AES cart)", GAME_SUPPORTS_SAVE )
GAME( 1998, lastbld2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The Last Blade 2 / Bakumatsu Roman - Dai Ni Maku Gekka no Kenshi", GAME_SUPPORTS_SAVE )
GAME( 1998, neocup98,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Neo-Geo Cup '98 - The Road to the Victory", GAME_SUPPORTS_SAVE )
GAME( 1999, mslugx,    neogeo,   neogeo,   neogeo,   mslugx,   ROT0, "SNK", "Metal Slug X - Super Vehicle-001", GAME_SUPPORTS_SAVE )
GAME( 1999, kof99,     neogeo,   neogeo,   neogeo,   kof99,    ROT0, "SNK", "The King of Fighters '99 - Millennium Battle (set 1)" , GAME_SUPPORTS_SAVE ) /* Encrypted Code & GFX */
GAME( 1999, kof99a,    kof99,    neogeo,   neogeo,   kof99,    ROT0, "SNK", "The King of Fighters '99 - Millennium Battle (set 2)" , GAME_SUPPORTS_SAVE ) /* Encrypted Code & GFX, crashes going into attract demo */
GAME( 1999, kof99e,    kof99,    neogeo,   neogeo,   kof99,    ROT0, "SNK", "The King of Fighters '99 - Millennium Battle (earlier)" , GAME_SUPPORTS_SAVE ) /* Encrypted Code & GFX */
GAME( 1999, kof99n,    kof99,    neogeo,   neogeo,   kof99n,   ROT0, "SNK", "The King of Fighters '99 - Millennium Battle (not encrypted)" , GAME_SUPPORTS_SAVE )	/* Encrypted GFX */
GAME( 1999, kof99p,    kof99,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "The King of Fighters '99 - Millennium Battle (prototype)", GAME_SUPPORTS_SAVE )
GAME( 1999, garou,     neogeo,   neogeo,   neogeo,   garou,    ROT0, "SNK", "Garou - Mark of the Wolves (set 1)" , GAME_SUPPORTS_SAVE ) /* Encrypted Code & GFX */
GAME( 1999, garouo,    garou,    neogeo,   neogeo,   garouo,   ROT0, "SNK", "Garou - Mark of the Wolves (set 2)" , GAME_SUPPORTS_SAVE ) /* Encrypted Code & GFX */
GAME( 1999, garoup,    garou,    neogeo,   neogeo,   neogeo,   ROT0, "SNK", "Garou - Mark of the Wolves (prototype)", GAME_SUPPORTS_SAVE )
GAME( 1999, garoubl,   garou,    neogeo,   neogeo,   garoubl,  ROT0, "bootleg", "Garou - Mark of the Wolves (bootleg)", GAME_SUPPORTS_SAVE ) /* Bootleg of garoup */
GAME( 2000, mslug3,    neogeo,   neogeo,   neogeo,   mslug3,   ROT0, "SNK", "Metal Slug 3" , GAME_SUPPORTS_SAVE ) /* Encrypted Code & GFX */
GAME( 2000, mslug3h,   mslug3,   neogeo,   neogeo,   mslug3h,  ROT0, "SNK", "Metal Slug 3 (not encrypted)" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2000, mslug3b6,  mslug3,   neogeo,   neogeo,   mslug3b6, ROT0, "bootleg", "Metal Slug 6 (Metal Slug 3 bootleg)", GAME_SUPPORTS_SAVE ) /* real Metal Slug 6 is an Atomiswave HW game, see atomswav.c ;-) */
GAME( 2000, kof2000,   neogeo,   neogeo,   neogeo,   kof2000,  ROT0, "SNK", "The King of Fighters 2000" , GAME_SUPPORTS_SAVE ) /* Encrypted Code & GFX */
GAME( 2000, kof2000n,  kof2000,  neogeo,   neogeo,   kof2000n, ROT0, "SNK", "The King of Fighters 2000 (not encrypted)" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2001, zupapa,    neogeo,   neogeo,   neogeo,   zupapa,   ROT0, "SNK", "Zupapa!" , GAME_SUPPORTS_SAVE )	/* Encrypted GFX */
GAME( 2001, sengoku3,  neogeo,   neogeo,   neogeo,   sengoku3, ROT0, "Noise Factory / SNK", "Sengoku 3 / Sengoku Densho 2001" , GAME_SUPPORTS_SAVE )	/* Encrypted GFX */
GAME( 2001, kof2001,   neogeo,   neogeo,   neogeo,   kof2001,  ROT0, "Eolith / SNK", "The King of Fighters 2001 (set 1)" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2001, kof2001h,  kof2001,  neogeo,   neogeo,   kof2001,  ROT0, "Eolith / SNK", "The King of Fighters 2001 (set 2)" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2003, cthd2003,  kof2001,  neogeo,   neogeo,   cthd2003, ROT0, "bootleg", "Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg)", GAME_SUPPORTS_SAVE ) /* Protected Hack / Bootleg of kof2001 */
GAME( 2003, ct2k3sp,   kof2001,  neogeo,   neogeo,   ct2k3sp,  ROT0, "bootleg", "Crouching Tiger Hidden Dragon 2003 Super Plus (The King of Fighters 2001 bootleg)", GAME_SUPPORTS_SAVE ) /* Protected Hack / Bootleg of kof2001 */
GAME( 2003, ct2k3sa,   kof2001,  neogeo,   neogeo,   ct2k3sa,  ROT0, "bootleg", "Crouching Tiger Hidden Dragon 2003 Super Plus alternate (The King of Fighters 2001 bootleg)", GAME_SUPPORTS_SAVE ) /* Hack / Bootleg of kof2001 */
GAME( 2002, kof2002,   neogeo,   neogeo,   neogeo,   kof2002,  ROT0, "Eolith / Playmore", "The King of Fighters 2002" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2002, kof2002b,  kof2002,  neogeo,   neogeo,   kof2002b, ROT0, "bootleg", "The King of Fighters 2002 (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 2002, kf2k2pls,  kof2002,  neogeo,   neogeo,   kf2k2pls, ROT0, "bootleg", "The King of Fighters 2002 Plus (bootleg set 1)" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2002, kf2k2pla,  kof2002,  neogeo,   neogeo,   kf2k2pls, ROT0, "bootleg", "The King of Fighters 2002 Plus (bootleg set 2)" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2002, kf2k2mp,   kof2002,  neogeo,   neogeo,   kf2k2mp,  ROT0, "bootleg", "The King of Fighters 2002 Magic Plus (bootleg)" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2002, kf2k2mp2,  kof2002,  neogeo,   neogeo,   kf2k2mp2, ROT0, "bootleg", "The King of Fighters 2002 Magic Plus II (bootleg)" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2002, kof10th,   kof2002,  neogeo,   neogeo,   kof10th,  ROT0, "bootleg", "The King of Fighters 10th Anniversary (The King of Fighters 2002 bootleg)", GAME_SUPPORTS_SAVE ) // fake SNK copyright
GAME( 2005, kf10thep,  kof2002,  neogeo,   neogeo,   kf10thep, ROT0, "bootleg", "The King of Fighters 10th Anniversary Extra Plus (The King of Fighters 2002 bootleg)", GAME_SUPPORTS_SAVE ) // fake SNK copyright
GAME( 2004, kf2k5uni,  kof2002,  neogeo,   neogeo,   kf2k5uni, ROT0, "bootleg", "The King of Fighters 10th Anniversary 2005 Unique (The King of Fighters 2002 bootleg)", GAME_SUPPORTS_SAVE ) // fake SNK copyright
GAME( 2004, kof2k4se,  kof2002,  neogeo,   neogeo,   kof2k4se, ROT0, "bootleg", "The King of Fighters Special Edition 2004 (The King of Fighters 2002 bootleg)", GAME_SUPPORTS_SAVE ) /* Hack / Bootleg of kof2002 */
GAME( 2003, mslug5,    neogeo,   neogeo,   neogeo,   mslug5,   ROT0, "SNK Playmore", "Metal Slug 5", GAME_SUPPORTS_SAVE )
GAME( 2003, mslug5h,   mslug5,   neogeo,   neogeo,   mslug5,   ROT0, "SNK Playmore", "Metal Slug 5 (AES Cart)", GAME_SUPPORTS_SAVE ) /* Also found in later MVS carts */
GAME( 2003, ms5pcb,    0,        neogeo,   ms5pcb,   ms5pcb,   ROT0, "SNK Playmore", "Metal Slug 5 (JAMMA PCB)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
GAME( 2003, ms5plus,   mslug5,   neogeo,   neogeo,   ms5plus,  ROT0, "bootleg", "Metal Slug 5 Plus (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 2003, svcpcb,    0,        neogeo,   svcpcb,   svcpcb,   ROT0, "SNK Playmore", "SNK vs. Capcom - SVC Chaos (JAMMA PCB, set 1)", GAME_SUPPORTS_SAVE ) // not a clone of neogeo because it's NOT a neogeo cart.
GAME( 2003, svcpcba,   svcpcb,   neogeo,   svcpcb,   svcpcb,   ROT0, "SNK Playmore", "SNK vs. Capcom - SVC Chaos (JAMMA PCB, set 2)" , GAME_SUPPORTS_SAVE ) /* Encrypted Code */
GAME( 2003, svc,       neogeo,   neogeo,   neogeo,   svc,      ROT0, "SNK Playmore", "SNK vs. Capcom - SVC Chaos", GAME_SUPPORTS_SAVE )
GAME( 2003, svcboot,   svc,      neogeo,   neogeo,   svcboot,  ROT0, "bootleg", "SNK vs. Capcom - SVC Chaos (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 2003, svcplus,   svc,      neogeo,   neogeo,   svcplus,  ROT0, "bootleg", "SNK vs. Capcom - SVC Chaos Plus (bootleg set 1)", GAME_SUPPORTS_SAVE )
GAME( 2003, svcplusa,  svc,      neogeo,   neogeo,   svcplusa, ROT0, "bootleg", "SNK vs. Capcom - SVC Chaos Plus (bootleg set 2)", GAME_SUPPORTS_SAVE )
GAME( 2003, svcsplus,  svc,      neogeo,   neogeo,   svcsplus, ROT0, "bootleg", "SNK vs. Capcom - SVC Chaos Super Plus (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 2003, samsho5,   neogeo,   neogeo,   neogeo,   samsho5,  ROT0, "Yuki Enterprise / SNK Playmore", "Samurai Shodown V / Samurai Spirits Zero (set 1)", GAME_SUPPORTS_SAVE )
GAME( 2003, samsho5h,  samsho5,  neogeo,   neogeo,   samsho5,  ROT0, "Yuki Enterprise / SNK Playmore", "Samurai Shodown V / Samurai Spirits Zero (set 2)", GAME_SUPPORTS_SAVE )
GAME( 2003, samsho5b,  samsho5,  neogeo,   neogeo,   samsho5b, ROT0, "bootleg", "Samurai Shodown V / Samurai Spirits Zero (bootleg)", GAME_SUPPORTS_SAVE ) // different program scrambling
GAME( 2003, kf2k3pcb,  0,        neogeo,   neogeo,   kf2k3pcb, ROT0, "SNK Playmore", "The King of Fighters 2003 (Japan, JAMMA PCB)", GAME_SUPPORTS_SAVE ) // not a clone of neogeo because it's NOT a neogeo cart.
GAME( 2003, kof2003,   neogeo,   neogeo,   neogeo,   kof2003,  ROT0, "SNK Playmore", "The King of Fighters 2003 (set 1)", GAME_SUPPORTS_SAVE )
GAME( 2003, kof2003h,  kof2003,  neogeo,   neogeo,   kof2003h, ROT0, "SNK Playmore", "The King of Fighters 2003 (AES cart)", GAME_SUPPORTS_SAVE )
GAME( 2003, kf2k3bl,   kof2003,  neogeo,   neogeo,   kf2k3bl , ROT0, "bootleg", "The King of Fighters 2003 (bootleg set 1)", GAME_SUPPORTS_SAVE ) // zooming is wrong because its a bootleg of the pcb version on a cart (unless it was a bootleg pcb with the new bios?)
GAME( 2003, kf2k3bla,  kof2003,  neogeo,   neogeo,   kf2k3pl,  ROT0, "bootleg", "The King of Fighters 2003 (bootleg set 2)", GAME_SUPPORTS_SAVE ) // zooming is wrong because its a bootleg of the pcb version on a cart
GAME( 2003, kf2k3pl,   kof2003,  neogeo,   neogeo,   kf2k3pl,  ROT0, "bootleg", "The King of Fighters 2004 Plus / Hero (The King of Fighters 2003 bootleg)", GAME_SUPPORTS_SAVE ) // zooming is wrong because its a bootleg of the pcb version on a cart
GAME( 2003, kf2k3upl,  kof2003,  neogeo,   neogeo,   kf2k3upl, ROT0, "bootleg", "The King of Fighters 2004 Ultra Plus (The King of Fighters 2003 bootleg)", GAME_SUPPORTS_SAVE ) // zooming is wrong because its a bootleg of the pcb version on a cart
GAME( 2003, samsh5sp,  neogeo,   neogeo,   neogeo,   samsh5sp, ROT0, "Yuki Enterprise / SNK Playmore", "Samurai Shodown V Special / Samurai Spirits Zero Special (set 1, uncensored)", GAME_SUPPORTS_SAVE )
GAME( 2003, samsh5sph, samsh5sp, neogeo,   neogeo,   samsh5sp, ROT0, "Yuki Enterprise / SNK Playmore", "Samurai Shodown V Special / Samurai Spirits Zero Special (set 2, censored)", GAME_SUPPORTS_SAVE )
GAME( 2003, samsh5spn, samsh5sp, neogeo,   neogeo,   samsh5sp, ROT0, "Yuki Enterprise / SNK Playmore", "Samurai Shodown V Special / Samurai Spirits Zero Special (set 3, less censored)", GAME_SUPPORTS_SAVE )

/* there are other bootlegs kof96ep, kf2k1pls etc.? -- work out which should be supported */

/* Alpha Denshi Co. / ADK (changed name in 1993) */
GAME( 1990, maglord,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "Magician Lord (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1990, maglordh,  maglord,  neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "Magician Lord (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1990, ncombat,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "Ninja Combat (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1990, ncombath,  ncombat,  neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "Ninja Combat (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1990, bjourney,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "Blue's Journey / Raguy", GAME_SUPPORTS_SAVE )
GAME( 1991, crsword,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "Crossed Swords", GAME_SUPPORTS_SAVE )
GAME( 1991, trally,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "Thrash Rally", GAME_SUPPORTS_SAVE )
GAME( 1992, ncommand,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "Ninja Commando", GAME_SUPPORTS_SAVE )
GAME( 1992, wh1,       neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "World Heroes (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1992, wh1h,      wh1,      neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "World Heroes (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1992, wh1ha,     wh1,      neogeo,   neogeo,   neogeo,   ROT0, "Alpha Denshi Co.", "World Heroes (set 3)", GAME_SUPPORTS_SAVE )
GAME( 1993, wh2,       neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "ADK",              "World Heroes 2", GAME_SUPPORTS_SAVE )
GAME( 1994, wh2j,      neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "ADK / SNK",        "World Heroes 2 Jet (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1994, wh2jh,     wh2j,     neogeo,   neogeo,   neogeo,   ROT0, "ADK / SNK",        "World Heroes 2 Jet (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1994, aodk,      neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "ADK / SNK",        "Aggressors of Dark Kombat / Tsuukai GANGAN Koushinkyoku", GAME_SUPPORTS_SAVE )
GAME( 1995, whp,       neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "ADK / SNK",        "World Heroes Perfect", GAME_SUPPORTS_SAVE )
GAME( 1995, mosyougi,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "ADK / SNK",        "Syougi No Tatsujin - Master of Syougi", GAME_SUPPORTS_SAVE )
GAME( 1996, overtop,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "ADK",              "Over Top", GAME_SUPPORTS_SAVE )
GAME( 1996, ninjamas,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "ADK / SNK",        "Ninja Master's - haoh-ninpo-cho", GAME_SUPPORTS_SAVE )
GAME( 1996, twinspri,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "ADK / SNK",        "Twinkle Star Sprites", GAME_SUPPORTS_SAVE )
GAME( 1996, zintrckb,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "hack",             "Zintrick / Oshidashi Zentrix (hack)", GAME_SUPPORTS_SAVE )

/* Aicom (was a part of Sammy) / Yumekobo (changed name in 1996) */
GAME( 1992, viewpoin,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Sammy / Aicom", "Viewpoint", GAME_SUPPORTS_SAVE )
GAME( 1994, janshin,   neogeo,   neogeo,   mjneogeo, neogeo,   ROT0, "Aicom", "Jyanshin Densetsu - Quest of Jongmaster", GAME_SUPPORTS_SAVE )
GAME( 1995, pulstar,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Aicom", "Pulstar", GAME_SUPPORTS_SAVE )
GAME( 1998, blazstar,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Yumekobo", "Blazing Star", GAME_SUPPORTS_SAVE )
GAME( 1999, preisle2,  neogeo,   neogeo,   neogeo,   preisle2, ROT0, "Yumekobo", "Prehistoric Isle 2" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */

/* Data East Corporation */
GAME( 1993, spinmast,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Data East Corporation", "Spin Master / Miracle Adventure", GAME_SUPPORTS_SAVE )
GAME( 1994, wjammers,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Data East Corporation", "Windjammers / Flying Power Disc", GAME_SUPPORTS_SAVE )
GAME( 1994, karnovr,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Data East Corporation", "Karnov's Revenge / Fighter's History Dynamite", GAME_SUPPORTS_SAVE )
GAME( 1994, strhoop,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Data East Corporation", "Street Hoop / Street Slam / Dunk Dream", GAME_SUPPORTS_SAVE )
GAME( 1996, ghostlop,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Data East Corporation", "Ghostlop (prototype)", GAME_SUPPORTS_SAVE )
GAME( 1996, magdrop2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Data East Corporation", "Magical Drop II", GAME_SUPPORTS_SAVE )
GAME( 1997, magdrop3,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Data East Corporation", "Magical Drop III", GAME_SUPPORTS_SAVE )

/* Eleven */
GAME( 2000, nitd,      neogeo,   neogeo,   neogeo,   nitd,     ROT0, "Eleven / Gavaking", "Nightmare in the Dark" , GAME_SUPPORTS_SAVE ) /* Encrypted GFX */
GAME( 2001, nitdbl,    nitd,     neogeo,   neogeo,   neogeo,   ROT0, "bootleg", "Nightmare in the Dark (bootleg)" , GAME_SUPPORTS_SAVE )

/* Face */
GAME( 1994, gururin,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Face", "Gururin", GAME_SUPPORTS_SAVE )
GAME( 1997, miexchng,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Face", "Money Puzzle Exchanger / Money Idol Exchanger", GAME_SUPPORTS_SAVE )

/* Hudson Soft */
GAME( 1994, panicbom,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Eighting / Hudson", "Panic Bomber", GAME_SUPPORTS_SAVE )
GAME( 1995, kabukikl,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Hudson", "Far East of Eden - Kabuki Klash / Tengai Makyou - Shin Den", GAME_SUPPORTS_SAVE )
GAME( 1997, neobombe,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Hudson", "Neo Bomberman", GAME_SUPPORTS_SAVE )

/* Monolith Corp. */
GAME( 1990, minasan,   neogeo,   neogeo,   mjneogeo, neogeo,   ROT0, "Monolith Corp.", "Minnasanno Okagesamadesu", GAME_SUPPORTS_SAVE )
GAME( 1991, bakatono,  neogeo,   neogeo,   mjneogeo, neogeo,   ROT0, "Monolith Corp.", "Bakatonosama Mahjong Manyuki", GAME_SUPPORTS_SAVE )

/* Nazca (later acquired by SNK) */
GAME( 1996, turfmast,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Nazca", "Neo Turf Masters / Big Tournament Golf", GAME_SUPPORTS_SAVE )
GAME( 1996, mslug,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Nazca", "Metal Slug - Super Vehicle-001", GAME_SUPPORTS_SAVE )

/* NMK */
GAME( 1994, zedblade,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "NMK", "Zed Blade / Operation Ragnarok", GAME_SUPPORTS_SAVE )

/* Psikyo */
GAME( 1999, s1945p,    neogeo,   neogeo,   neogeo,   s1945p,   ROT0, "Psikyo", "Strikers 1945 Plus" , GAME_SUPPORTS_SAVE )	/* Encrypted GFX */

/* Saurus */
GAME( 1995, quizkof,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Saurus", "Quiz King of Fighters", GAME_SUPPORTS_SAVE )
GAME( 1995, stakwin,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Saurus", "Stakes Winner / Stakes Winner - GI kinzen seihae no michi", GAME_SUPPORTS_SAVE )
GAME( 1996, ragnagrd,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Saurus", "Ragnagard / Shin-Oh-Ken", GAME_SUPPORTS_SAVE )
GAME( 1996, pgoal,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Saurus", "Pleasure Goal / Futsal - 5 on 5 Mini Soccer", GAME_SUPPORTS_SAVE )
GAME( 1996, stakwin2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Saurus", "Stakes Winner 2", GAME_SUPPORTS_SAVE )
GAME( 1997, shocktro,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Saurus", "Shock Troopers (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1997, shocktra,  shocktro, neogeo,   neogeo,   neogeo,   ROT0, "Saurus", "Shock Troopers (set 2)", GAME_SUPPORTS_SAVE )
GAME( 1998, shocktr2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Saurus", "Shock Troopers - 2nd Squad", GAME_SUPPORTS_SAVE )
GAME( 1998, lans2004,  shocktr2, neogeo,   neogeo,   lans2004, ROT0, "bootleg", "Lansquenet 2004 (Shock Troopers - 2nd Squad bootleg)", GAME_SUPPORTS_SAVE )

/* Sunsoft */
GAME( 1995, galaxyfg,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Sunsoft", "Galaxy Fight - Universal Warriors", GAME_SUPPORTS_SAVE )
GAME( 1996, wakuwak7,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Sunsoft", "Waku Waku 7", GAME_SUPPORTS_SAVE )

/* Taito */
GAME( 1994, pbobblen,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Taito", "Puzzle Bobble / Bust-A-Move (Neo-Geo) (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1994, pbobblenb, pbobblen, neogeo,   neogeo,   neogeo,   ROT0, "bootleg", "Puzzle Bobble / Bust-A-Move (Neo-Geo) (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1999, pbobbl2n,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Taito (SNK license)", "Puzzle Bobble 2 / Bust-A-Move Again (Neo-Geo)", GAME_SUPPORTS_SAVE )
GAME( 2003, pnyaa,     neogeo,   neogeo,   neogeo,   pnyaa,    ROT0, "Aiky / Taito", "Pochi and Nyaa", GAME_SUPPORTS_SAVE )

/* Takara */
GAME( 1995, marukodq,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Takara", "Chibi Marukochan Deluxe Quiz", GAME_SUPPORTS_SAVE )

/* Technos Japan */
GAME( 1995, doubledr,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Technos Japan", "Double Dragon (Neo-Geo)", GAME_SUPPORTS_SAVE )
GAME( 1995, gowcaizr,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Technos Japan", "Voltage Fighter - Gowcaizer / Choujin Gakuen Gowcaizer", GAME_SUPPORTS_SAVE )
GAME( 1996, sdodgeb,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Technos Japan", "Super Dodge Ball / Kunio no Nekketsu Toukyuu Densetsu", GAME_SUPPORTS_SAVE )

/* Tecmo */
GAME( 1996, tws96,     neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Tecmo", "Tecmo World Soccer '96", GAME_SUPPORTS_SAVE )

/* Viccom */
GAME( 1994, fightfev,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Viccom", "Fight Fever (set 1)", GAME_SUPPORTS_SAVE )
GAME( 1994, fightfeva, fightfev, neogeo,   neogeo,   neogeo,   ROT0, "Viccom", "Fight Fever (set 2)", GAME_SUPPORTS_SAVE )

/* Video System Co. */
GAME( 1994, pspikes2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Video System Co.", "Power Spikes II", GAME_SUPPORTS_SAVE )
GAME( 1994, sonicwi2,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Video System Co.", "Aero Fighters 2 / Sonic Wings 2", GAME_SUPPORTS_SAVE )
GAME( 1995, sonicwi3,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Video System Co.", "Aero Fighters 3 / Sonic Wings 3", GAME_SUPPORTS_SAVE )
GAME( 1997, popbounc,  neogeo,   neogeo,   popbounc, neogeo,   ROT0, "Video System Co.", "Pop 'n Bounce / Gapporin", GAME_SUPPORTS_SAVE )

/* Visco */
GAME( 1992, androdun,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Andro Dunos", GAME_SUPPORTS_SAVE )
GAME( 1995, puzzledp,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Taito (Visco license)", "Puzzle De Pon!", GAME_SUPPORTS_SAVE )
GAME( 1996, neomrdo,   neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Neo Mr. Do!", GAME_SUPPORTS_SAVE )
GAME( 1995, goalx3,    neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Goal! Goal! Goal!", GAME_SUPPORTS_SAVE )
GAME( 1996, neodrift,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Neo Drift Out - New Technology", GAME_SUPPORTS_SAVE )
GAME( 1996, breakers,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Breakers", GAME_SUPPORTS_SAVE )
GAME( 1997, puzzldpr,  puzzledp, neogeo,   neogeo,   neogeo,   ROT0, "Taito (Visco license)", "Puzzle De Pon! R!", GAME_SUPPORTS_SAVE )
GAME( 1998, breakrev,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Breakers Revenge", GAME_SUPPORTS_SAVE )
GAME( 1998, flipshot,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Battle Flip Shot", GAME_SUPPORTS_SAVE )
GAME( 1999, ctomaday,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Captain Tomaday", GAME_SUPPORTS_SAVE )
GAME( 1999, ganryu,    neogeo,   neogeo,   neogeo,   ganryu,   ROT0, "Visco", "Ganryu / Musashi Ganryuki" , GAME_SUPPORTS_SAVE )	/* Encrypted GFX */
GAME( 2000, bangbead,  neogeo,   neogeo,   neogeo,   bangbead, ROT0, "Visco", "Bang Bead", GAME_SUPPORTS_SAVE )
//GAME( 2000, bangbedp,  bangbead, neogeo,   neogeo,   neogeo,   ROT0, "Visco", "Bang Bead (prototype)", GAME_SUPPORTS_SAVE )

/* Mega Enterprise */
GAME( 2002, mslug4,    neogeo,   neogeo,   neogeo,   mslug4,   ROT0, "Mega / Playmore", "Metal Slug 4 (set 1)", GAME_SUPPORTS_SAVE )
GAME( 2002, mslug4h,   mslug4,   neogeo,   neogeo,   mslug4,   ROT0, "Mega / Playmore", "Metal Slug 4 (set 2)", GAME_SUPPORTS_SAVE )
GAME( 2002, ms4plus,   mslug4,   neogeo,   neogeo,   ms4plus,  ROT0, "bootleg", "Metal Slug 4 Plus (bootleg)", GAME_SUPPORTS_SAVE )

/* Evoga */
GAME( 2002, rotd,      neogeo,   neogeo,   neogeo,   rotd,     ROT0, "Evoga / Playmore", "Rage of the Dragons", GAME_SUPPORTS_SAVE )

/* Atlus */
GAME( 2002, matrim,    neogeo,   neogeo,   neogeo,   matrim,   ROT0, "Noise Factory / Atlus", "Matrimelee / Shin Gouketsuji Ichizoku Toukon", GAME_SUPPORTS_SAVE )
GAME( 2002, matrimbl,  matrim,   neogeo,   neogeo,   matrimbl, ROT0, "bootleg", "Matrimelee / Shin Gouketsuji Ichizoku Toukon (bootleg)", GAME_SUPPORTS_SAVE )

/* BrezzaSoft */
GAME( 2001, jockeygp,  neogeo,   neogeo,   jockeygp, jockeygp, ROT0, "Sun Amusement / BrezzaSoft", "Jockey Grand Prix", GAME_SUPPORTS_SAVE )
GAME( 2001, vliner,    neogeo,   neogeo,   vliner,   vliner,   ROT0, "Dyna / BrezzaSoft", "V-Liner (set 1)", GAME_SUPPORTS_SAVE )
GAME( 2001, vlinero,   vliner,   neogeo,   vliner,   vliner,   ROT0, "Dyna / BrezzaSoft", "V-Liner (set 2)", GAME_SUPPORTS_SAVE )

/* Kyle Hodgetts */
GAME( 2000, diggerma,  neogeo,   neogeo,   neogeo,   neogeo,   ROT0, "Kyle Hodgetts", "Digger Man (prototype)", GAME_SUPPORTS_SAVE )

/* Neo Print */
GAME( 1998, 98best44,  0,        neogeo,   neogeo,   neogeo,   ROT0, "SNK", "'98 NeoPri Best 44 (Neo Print)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )
