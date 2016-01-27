// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
/**************************************************************************

  Waku Waku Doubutsu Land TonTon (c) 199? Success.

  HW based off MSX2

  Driver by Angelo Salese & Roberto Fresca.


***************************************************************************

  WAKUWAKU DOUBUTSU LAND TONTON (ANIMAL VIDEO SLOT)
  (c)SUCCESS / CABINET :TAIYO JIDOKI (SUN AUTO MACHINE)

  CPU   : Z80
  SOUND : YM2149F
  XTAL  : 21477.27KHz

  TONTON.BIN  : MAIN ROM


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/v9938.h"
#include "machine/ticket.h"
#include "machine/nvram.h"

class tonton_state : public driver_device
{
public:
	tonton_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_v9938(*this, "v9938"),
		m_maincpu(*this, "maincpu"),
		m_hopper(*this, "hopper")
	{ }

	required_device<v9938_device> m_v9938;
	DECLARE_WRITE8_MEMBER(tonton_outport_w);
	DECLARE_WRITE8_MEMBER(ay_aout_w);
	DECLARE_WRITE8_MEMBER(ay_bout_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_WRITE_LINE_MEMBER(tonton_vdp0_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<ticket_dispenser_device> m_hopper;
};

#define MAIN_CLOCK      XTAL_21_4772MHz
#define CPU_CLOCK       MAIN_CLOCK/6
#define YM2149_CLOCK    MAIN_CLOCK/6/2  // '/SEL' pin tied to GND, so internal divisor x2 is active

#define HOPPER_PULSE    50          // time between hopper pulses in milliseconds
#define VDP_MEM         0x30000


/*************************************************
*                Video Hardware                  *
*************************************************/

WRITE_LINE_MEMBER(tonton_state::tonton_vdp0_interrupt)
{
	m_maincpu->set_input_line(0, (state ? HOLD_LINE : CLEAR_LINE));
}


/*************************************************
*          Multi-Purpose Output Port             *
*************************************************/

WRITE8_MEMBER(tonton_state::tonton_outport_w)
{
	machine().bookkeeping().coin_counter_w(offset, data & 0x01);
	machine().bookkeeping().coin_lockout_global_w(data & 0x02);  /* Coin Lock */
	m_hopper->write(space, 0, (data & 0x02));    /* Hopper Motor */

//  if(data & 0xfe)
//      logerror("%02x %02x\n",data,offset);
	if (data)
		logerror("tonton_outport_w %02X @ %04X\n", data, space.device().safe_pc());
}


/*************************************************
*                  Memory Map                    *
*************************************************/

static ADDRESS_MAP_START( tonton_map, AS_PROGRAM, 8, tonton_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xe3ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tonton_io, AS_IO, 8, tonton_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0")
	AM_RANGE(0x00, 0x00) AM_WRITE(tonton_outport_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1")
	AM_RANGE(0x01, 0x01) AM_WRITENOP    // write the same to outport 00h
	AM_RANGE(0x02, 0x02) AM_READ_PORT("DSW1")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSW2")
	AM_RANGE(0x88, 0x8b) AM_DEVREADWRITE( "v9938", v9938_device, read, write )
	AM_RANGE(0xa0, 0xa1) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0xa2, 0xa2) AM_DEVREAD("aysnd", ay8910_device, data_r)
ADDRESS_MAP_END


/*************************************************
*            Input Ports Definitions             *
*************************************************/

static INPUT_PORTS_START( tonton )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("1. Pig")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("2. Penguin")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("3. Tiger")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("4. Cow")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("5. Bear")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("6. Elephant")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CODE(KEYCODE_M) PORT_NAME("7. Lion")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_TOGGLE PORT_NAME("Bookkeeping")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("Medal In")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Reset Button")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Unknown A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)    // hopper feedback
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x03, "Coinage A (100 Yen)" )   PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(    0x02, "1 Coin / 3 Medal" )
	PORT_DIPSETTING(    0x06, "1 Coin / 4 Medal" )
	PORT_DIPSETTING(    0x01, "1 Coin / 5 Medal" )
	PORT_DIPSETTING(    0x05, "1 Coin / 6 Medal" )
	PORT_DIPSETTING(    0x03, "1 Coin / 10 Medal" )
	PORT_DIPSETTING(    0x07, "1 Coin / 11 Medal" )
	PORT_DIPSETTING(    0x04, "1 Coin / 20 Medal" )
	PORT_DIPSETTING(    0x00, "1 Coin / 50 Medal" )
	PORT_DIPNAME( 0x18, 0x18, "Coinage B (10 Yen)" )    PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(    0x00, "3 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x10, "2 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x18, "1 Coin / 1 Medal" )
	PORT_DIPSETTING(    0x08, "1 Coin / 2 Medal" )
	PORT_DIPNAME( 0x20, 0x20, "Service Coinage" )       PORT_DIPLOCATION("DSW1:6")
	PORT_DIPSETTING(    0x20, "1 Pulse / 1 Medal" )
	PORT_DIPSETTING(    0x00, "1 Pulse / 2 Medal" )
	PORT_DIPNAME( 0x40, 0x40, "Coinage A Lock" )        PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(    0x40, "Unlocked" )
	PORT_DIPSETTING(    0x00, "Locked" )
	PORT_DIPNAME( 0x80, 0x80, "Payout Mode")            PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Automatic" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x03, "Percentage" )            PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x03, "60%" )
	PORT_DIPSETTING(    0x05, "70%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x06, "80%" )
	PORT_DIPSETTING(    0x02, "85%" )
	PORT_DIPSETTING(    0x04, "90%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END


/*************************************************
*        Machine Start & Reset Routines          *
*************************************************/

void tonton_state::machine_start()
{
}

void tonton_state::machine_reset()
{
}


/*************************************************
*      R/W Handlers and Interrupt Routines       *
*************************************************/

WRITE8_MEMBER(tonton_state::ay_aout_w)
{
	logerror("AY8910: Port A out: %02X\n", data);
}

WRITE8_MEMBER(tonton_state::ay_bout_w)
{
	logerror("AY8910: Port B out: %02X\n", data);
}

/*************************************************
*                 Machine Driver                 *
*************************************************/

static MACHINE_CONFIG_START( tonton, tonton_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, CPU_CLOCK)  /* Guess. According to other MSX2 based gambling games */
	MCFG_CPU_PROGRAM_MAP(tonton_map)
	MCFG_CPU_IO_MAP(tonton_io)

	MCFG_NVRAM_ADD_0FILL("nvram")


	/* video hardware */
	MCFG_V9938_ADD("v9938", "screen", VDP_MEM, MAIN_CLOCK)
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(tonton_state,tonton_vdp0_interrupt))
	MCFG_V99X8_SCREEN_ADD_NTSC("screen", "v9938", MAIN_CLOCK)

	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(HOPPER_PULSE), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd", YM2149, YM2149_CLOCK)   /* Guess. According to other MSX2 based gambling games */
	/*
	  AY8910: Port A out: FF
	  AY8910: Port B out: FF
	  AY8910: Port A out: FF
	  AY8910: Port B out: FF
	  AY8910: Port A out: 00
	  AY8910: Port B out: 00
	*/
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(tonton_state, ay_aout_w))    /* Write all bits twice, and then reset them at boot */
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(tonton_state, ay_bout_w))     /* Write all bits twice, and then reset them at boot */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.70)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tonton )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tonton.bin",   0x0000, 0x10000, CRC(6c9cacfb) SHA1(21afd5a40b785300b013ac8cb31f5e4f480657ef) )
ROM_END


/*    YEAR  NAME     PARENT  MACHINE  INPUT   STATE          INIT  ROT    COMPANY                   FULLNAME                                 FLAGS  */
GAME( 199?, tonton,  0,      tonton,  tonton, driver_device, 0,    ROT0, "Success / Taiyo Jidoki", "Waku Waku Doubutsu Land TonTon (Japan)", 0 )
