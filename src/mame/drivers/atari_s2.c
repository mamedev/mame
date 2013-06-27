/****************************************************************************************

    Pinball
    Atari Generation/System 2 and 3

    System 2 : Manuals and PinMAME used as references (couldn't find full schematics).
    System 3 : PinMAME used as reference (couldn't find anything else).

    The only difference seems to be an extra bank of inputs (or something) at 2008-200B.


*****************************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "atari_s2.lh"


class atari_s2_state : public genpin_class
{
public:
	atari_s2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	DECLARE_WRITE8_MEMBER(sound0_w) { };
	DECLARE_WRITE8_MEMBER(sound1_w) { };
	DECLARE_WRITE8_MEMBER(lamp_w) { };
	DECLARE_WRITE8_MEMBER(sol0_w);
	DECLARE_WRITE8_MEMBER(sol1_w) { };
	DECLARE_WRITE8_MEMBER(intack_w);
	DECLARE_WRITE8_MEMBER(display_w);
	DECLARE_READ8_MEMBER(switch_r);

	TIMER_DEVICE_CALLBACK_MEMBER(irq);
protected:

	// devices
	required_device<cpu_device> m_maincpu;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_t_c;
	UINT8 m_segment[7];
};


static ADDRESS_MAP_START( atari_s2_map, AS_PROGRAM, 8, atari_s2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("nvram") // battery backed
	AM_RANGE(0x1000, 0x1007) AM_MIRROR(0x07F8) AM_READ(switch_r)
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x071F) AM_WRITE(sound0_w)
	AM_RANGE(0x1820, 0x1820) AM_MIRROR(0x071F) AM_WRITE(sound1_w)
	AM_RANGE(0x1840, 0x1847) AM_MIRROR(0x0718) AM_WRITE(display_w)
	AM_RANGE(0x1860, 0x1867) AM_MIRROR(0x0718) AM_WRITE(lamp_w)
	AM_RANGE(0x1880, 0x1880) AM_MIRROR(0x071F) AM_WRITE(sol0_w)
	AM_RANGE(0x18a0, 0x18a7) AM_MIRROR(0x0718) AM_WRITE(sol1_w)
	AM_RANGE(0x18c0, 0x18c0) AM_MIRROR(0x071F) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x18e0, 0x18e0) AM_MIRROR(0x071F) AM_WRITE(intack_w)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x07FC) AM_READ_PORT("DSW0")
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x07FC) AM_READ_PORT("DSW1")
	AM_RANGE(0x2002, 0x2002) AM_MIRROR(0x07FC) AM_READ_PORT("DSW2")
	AM_RANGE(0x2003, 0x2003) AM_MIRROR(0x07FC) AM_READ_PORT("DSW3")
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( atari_s3_map, AS_PROGRAM, 8, atari_s2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("nvram") // battery backed
	AM_RANGE(0x1000, 0x1007) AM_MIRROR(0x07F8) AM_READ(switch_r)
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x071F) AM_WRITE(sound0_w)
	AM_RANGE(0x1820, 0x1820) AM_MIRROR(0x071F) AM_WRITE(sound1_w)
	AM_RANGE(0x1840, 0x1847) AM_MIRROR(0x0718) AM_WRITE(display_w)
	AM_RANGE(0x1860, 0x1867) AM_MIRROR(0x0718) AM_WRITE(lamp_w)
	AM_RANGE(0x1880, 0x1880) AM_MIRROR(0x071F) AM_WRITE(sol0_w)
	AM_RANGE(0x18a0, 0x18a7) AM_MIRROR(0x0718) AM_WRITE(sol1_w)
	AM_RANGE(0x18c0, 0x18c0) AM_MIRROR(0x071F) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x18e0, 0x18e0) AM_MIRROR(0x071F) AM_WRITE(intack_w)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x07F4) AM_READ_PORT("DSW0")
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x07F4) AM_READ_PORT("DSW1")
	AM_RANGE(0x2002, 0x2002) AM_MIRROR(0x07F4) AM_READ_PORT("DSW2")
	AM_RANGE(0x2003, 0x2003) AM_MIRROR(0x07F4) AM_READ_PORT("DSW3")
	AM_RANGE(0x2008, 0x2008) AM_MIRROR(0x07F4) AM_READ_PORT("DSW4")
	AM_RANGE(0x2009, 0x2009) AM_MIRROR(0x07F4) AM_READ_PORT("DSW5")
	AM_RANGE(0x200a, 0x200a) AM_MIRROR(0x07F4) AM_READ_PORT("DSW6")
	AM_RANGE(0x200b, 0x200b) AM_MIRROR(0x07F4) AM_READ_PORT("DSW7")
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( atari_s2 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x05, "Max Credits" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "15" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x04, "25" )
	PORT_DIPSETTING(    0x05, "30" )
	PORT_DIPSETTING(    0x06, "35" )
	PORT_DIPSETTING(    0x07, "40" )
	PORT_DIPNAME( 0x08, 0x00, "Balls" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x50, 0x00, "Special" )
	PORT_DIPSETTING(    0x10, "Extra Ball" )
	PORT_DIPSETTING(    0x00, "Free Game" )
	PORT_DIPSETTING(    0x40, "50000 points" )
	PORT_DIPSETTING(    0x50, "60000 points" )
	PORT_DIPNAME( 0x20, 0x00, "Free Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Match" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Upper Lanes" )
	PORT_DIPSETTING(    0x10, "Start only" )
	PORT_DIPSETTING(    0x00, "Start and Advance" )
	PORT_DIPNAME( 0x60, 0x00, "Extra Ball reward" )
	PORT_DIPSETTING(    0x00, "Extra Ball" )
	PORT_DIPSETTING(    0x20, "20000 points" )
	PORT_DIPSETTING(    0x60, "30000 points" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x1f, 0x02, "Coinage L Chute" )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x11, "1 coin/10 credits" )
	PORT_DIPSETTING(    0x12, "1 coin/11 credits" )
	PORT_DIPSETTING(    0x13, "1 coin/12 credits" )
	PORT_DIPSETTING(    0x14, "1 coin/13 credits" )
	PORT_DIPSETTING(    0x15, "1 coin/14 credits" )
	PORT_DIPSETTING(    0x16, "1 coin/15 credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x09, "2 coins/9 credits" )
	PORT_DIPSETTING(    0x0b, "2 coins/11 credits" )
	PORT_DIPSETTING(    0x0d, "2 coins/13 credits" )
	PORT_DIPSETTING(    0x0f, "2 coins/15 credits" )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_BIT( 0x40, 0x00, IPT_UNUSED )
	PORT_DIPNAME( 0x80, 0x00, "High Score Display" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x1f, 0x02, "Coinage R Chute" )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x11, "1 coin/10 credits" )
	PORT_DIPSETTING(    0x12, "1 coin/11 credits" )
	PORT_DIPSETTING(    0x13, "1 coin/12 credits" )
	PORT_DIPSETTING(    0x14, "1 coin/13 credits" )
	PORT_DIPSETTING(    0x15, "1 coin/14 credits" )
	PORT_DIPSETTING(    0x16, "1 coin/15 credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x09, "2 coins/9 credits" )
	PORT_DIPSETTING(    0x0b, "2 coins/11 credits" )
	PORT_DIPSETTING(    0x0d, "2 coins/13 credits" )
	PORT_DIPSETTING(    0x0f, "2 coins/15 credits" )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x20, 0x00, "Ladder Memory" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Replays for High Score" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "3" )

	PORT_START("DSW4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X0") // 1000
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Test") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1") // 1001
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2") // 1002
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3") // 1003
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4") // 1004
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5") // 1005
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6") // 1006
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7") // 1007
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* solenoids hercules
   4,5 = bumpers
   8,9 = slings
    15 = outhole
   6,7 = coin counters
    14 = total plays counter

   solenoids superman
   4,5,8,11 = bumpers
   9,10     = slings
        15  = outhole
       6,7  = coin counters
        12  = drop target
        13  = drop hole kicker
        14  = total plays counter
*/

WRITE8_MEMBER( atari_s2_state::sol0_w )
{
	switch (data)
	{
		case 15:
			m_samples->start(0, 5);
			break;
		case 4:
		case 5:
			m_samples->start(1, 0);
			break;
		case 8:
		case 9:
			//m_samples->start(1, 7);
			break;
		//default:
			//if (data) printf("%X ",data);
	}
}

WRITE8_MEMBER( atari_s2_state::display_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	if (offset<7)
		m_segment[offset] = patterns[data&15];
	else
	{
		data &= 7;
		for (UINT8 i = 0; i < 7; i++)
			output_set_digit_value(i * 10 + data, m_segment[i]);
	}
}

WRITE8_MEMBER( atari_s2_state::intack_w )
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

READ8_MEMBER( atari_s2_state::switch_r )
{
	char kbdrow[8];
	sprintf(kbdrow,"X%X",offset);
	return ioport(kbdrow)->read();
}

TIMER_DEVICE_CALLBACK_MEMBER( atari_s2_state::irq )
{
	if (m_t_c > 0x40)
		m_maincpu->set_input_line(M6800_IRQ_LINE, HOLD_LINE);
	else
		m_t_c++;
}

void atari_s2_state::machine_reset()
{
}

static MACHINE_CONFIG_START( atari_s2, atari_s2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_4MHz / 4)
	MCFG_CPU_PROGRAM_MAP(atari_s2_map)
	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", atari_s2_state, irq, attotime::from_hz(XTAL_4MHz / 8192))

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_atari_s2)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( atari_s3, atari_s2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(atari_s3_map)
MACHINE_CONFIG_END


/*-------------------------------------------------------------------
/ Superman (03/1979)
/-------------------------------------------------------------------*/
ROM_START(supermap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("supmn_k.rom", 0x2800, 0x0800, CRC(a28091c2) SHA1(9f5e47db408da96a31cb2f3be0fa9fb1e79f8d85))
	ROM_LOAD("atari_m.rom", 0x3000, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_LOAD("atari_j.rom", 0x3800, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))

	ROM_REGION(0x1000, "sound1", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Hercules (05/1979)
/-------------------------------------------------------------------*/
ROM_START(hercules)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("herc_k.rom",  0x2800, 0x0800, CRC(65e099b1) SHA1(83a06bc82e0f8f4c0655886c6a9962bb28d00c5e))
	ROM_LOAD("atari_m.rom", 0x3000, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_LOAD("atari_j.rom", 0x3800, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))

	ROM_REGION(0x1000, "sound1", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Road Runner (??/1979)
/-------------------------------------------------------------------*/
ROM_START(roadrunr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0000.716", 0x2800, 0x0800, CRC(62f5f394) SHA1(ff91066d43d788119e3337788abd86e5c0bf2d92))
	ROM_LOAD("3000.716", 0x3000, 0x0800, CRC(2fc01359) SHA1(d3df20c764bb68a5316367bb18d34a03293e7fa6))
	ROM_LOAD("3800.716", 0x3800, 0x0800, CRC(77262408) SHA1(3045a732c39c96002f495f64ed752279f7d43ee7))
	ROM_REGION(0x1000, "sound1", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END


GAME( 1979, supermap,  0,  atari_s2,  atari_s2, driver_device, 0,  ROT0, "Atari", "Superman (Pinball)", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, hercules,  0,  atari_s2,  atari_s2, driver_device, 0,  ROT0, "Atari", "Hercules", GAME_MECHANICAL | GAME_NO_SOUND)
GAME( 1979, roadrunr,  0,  atari_s3,  atari_s2, driver_device, 0,  ROT0, "Atari", "Road Runner", GAME_MECHANICAL | GAME_NO_SOUND)
