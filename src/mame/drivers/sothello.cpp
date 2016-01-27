// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
/*
 Super Othello (c)1986 Fujiwara/Success

    driver by Tomasz Slanina

         1    2    3    4     5     6     7      8      9     10     11    12
+---------------------------------------------------------------------------------+
|                                                                                 |
+-+    LA460  LA6324  M5205  X3   Z80A    1      2    5816P  74374  74138         |  A
  |                                                                               |
  |                                                                               |
+-+        74367 Y3014 74174 74174       7404   7474  74138  7404   7432          |  B
|                                                                                 |
|                                                                                 |
|            74367 DSW1 YM2203    Z80A    3      4      5           6264          |  C
| J                                                                               |
| A                                                                               |
| M   C1663 74367  DSW2           7408 74125   7404  74138   74139  74174  7408   |  D
| M           X2       7414  7474                                                 |
| A                                                                               |
|     C1663 V9938 41464 41464       X1   7474  74139  7432   74157  74244  7432   |  E
|                                                                                 |
|                                                                                 |
+-+   C1663       41464 41464     6809B   6     6264   6264  6264   74244  74245  |  F
  |                                                                               |
  |                                                                               |
+-+   C1663                                                                       |  H
|                                                                                 |
+---------------------------------------------------------------------------------+

CPU  : Z80A(x2) 68B09
Sound: YM2203?(surface scrached) + M5205
OSC  : 8.0000MHz(X1)   21.477 MHz(X2)   384kHz(X3)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6809/m6809.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "video/v9938.h"


class sothello_state : public driver_device
{
public:
	sothello_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_v9938(*this, "v9938") ,
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_subcpu(*this, "sub"),
		m_msm(*this, "msm"),
		m_bank1(*this, "bank1")
	{ }

	required_device<v9938_device> m_v9938;

	int m_subcpu_status;
	int m_soundcpu_busy;
	int m_msm_data;

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(subcpu_halt_set);
	DECLARE_READ8_MEMBER(subcpu_halt_clear);
	DECLARE_READ8_MEMBER(subcpu_comm_status);
	DECLARE_READ8_MEMBER(soundcpu_status_r);
	DECLARE_WRITE8_MEMBER(msm_data_w);
	DECLARE_WRITE8_MEMBER(soundcpu_busyflag_set_w);
	DECLARE_WRITE8_MEMBER(soundcpu_busyflag_reset_w);
	DECLARE_WRITE8_MEMBER(soundcpu_int_clear_w);
	DECLARE_WRITE8_MEMBER(subcpu_status_w);
	DECLARE_READ8_MEMBER(subcpu_status_r);
	DECLARE_WRITE8_MEMBER(msm_cfg_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_CALLBACK_MEMBER(subcpu_suspend);
	TIMER_CALLBACK_MEMBER(subcpu_resume);
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
	DECLARE_WRITE_LINE_MEMBER(sothello_vdp_interrupt);
	void unlock_shared_ram();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm;
	required_memory_bank m_bank1;
};


#define VDP_MEM             0x40000

#define MAIN_CLOCK          (XTAL_21_4772MHz)
#define MAINCPU_CLOCK       (XTAL_21_4772MHz/6)
#define SOUNDCPU_CLOCK      (XTAL_21_4772MHz/6)
#define YM_CLOCK            (XTAL_21_4772MHz/12)
#define MSM_CLOCK           (XTAL_384kHz)
#define SUBCPU_CLOCK        (XTAL_8MHz/4)


/* main Z80 */

void sothello_state::machine_start()
{
	m_bank1->configure_entries(0, 4, memregion("maincpu")->base() + 0x8000, 0x4000);
}

WRITE8_MEMBER(sothello_state::bank_w)
{
	int bank=0;
	switch(data^0xff)
	{
		case 1: bank=0; break;
		case 2: bank=1; break;
		case 4: bank=2; break;
		case 8: bank=3; break;
	}
	m_bank1->set_entry(bank);
}

TIMER_CALLBACK_MEMBER(sothello_state::subcpu_suspend)
{
	m_subcpu->suspend(SUSPEND_REASON_HALT, 1);
}

TIMER_CALLBACK_MEMBER(sothello_state::subcpu_resume)
{
	m_subcpu->resume(SUSPEND_REASON_HALT);
	m_subcpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

READ8_MEMBER(sothello_state::subcpu_halt_set)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(sothello_state::subcpu_suspend),this));
	m_subcpu_status|=2;
	return 0;
}

READ8_MEMBER(sothello_state::subcpu_halt_clear)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(sothello_state::subcpu_resume),this));
	m_subcpu_status&=~1;
	m_subcpu_status&=~2;
	return 0;
}

READ8_MEMBER(sothello_state::subcpu_comm_status)
{
	return m_subcpu_status;
}

READ8_MEMBER(sothello_state::soundcpu_status_r)
{
	return m_soundcpu_busy;
}

static ADDRESS_MAP_START( maincpu_mem_map, AS_PROGRAM, 8, sothello_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_MIRROR(0x1800) AM_SHARE("share1")
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( maincpu_io_map, AS_IO, 8, sothello_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x00, 0x0f) AM_READ_PORT("INPUT1")
	AM_RANGE( 0x10, 0x1f) AM_READ_PORT("INPUT2")
	AM_RANGE( 0x20, 0x2f) AM_READ_PORT("SYSTEM")
	AM_RANGE( 0x30, 0x30) AM_READ(subcpu_halt_set)
	AM_RANGE( 0x31, 0x31) AM_READ(subcpu_halt_clear)
	AM_RANGE( 0x32, 0x32) AM_READ(subcpu_comm_status)
	AM_RANGE( 0x33, 0x33) AM_READ(soundcpu_status_r)
	AM_RANGE( 0x40, 0x4f) AM_WRITE(soundlatch_byte_w)
	AM_RANGE( 0x50, 0x50) AM_WRITE(bank_w)
	AM_RANGE( 0x60, 0x61) AM_MIRROR(0x02) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
						/* not sure, but the A1 line is ignored, code @ $8b8 */
	AM_RANGE( 0x70, 0x73) AM_DEVREADWRITE( "v9938", v9938_device, read, write )
ADDRESS_MAP_END

/* sound Z80 */

WRITE8_MEMBER(sothello_state::msm_cfg_w)
{
/*
     bit 0 = RESET
     bit 1 = 4B/3B 0
     bit 2 = S2    1
     bit 3 = S1    2
*/
	m_msm->playmode_w(BITSWAP8((data>>1), 7,6,5,4,3,0,1,2));
	m_msm->reset_w(data & 1);
}

WRITE8_MEMBER(sothello_state::msm_data_w)
{
	m_msm_data = data;

}

WRITE8_MEMBER(sothello_state::soundcpu_busyflag_set_w)
{
	m_soundcpu_busy=1;
}

WRITE8_MEMBER(sothello_state::soundcpu_busyflag_reset_w)
{
	m_soundcpu_busy=0;
}

WRITE8_MEMBER(sothello_state::soundcpu_int_clear_w)
{
	m_soundcpu->set_input_line(0, CLEAR_LINE );
}

static ADDRESS_MAP_START( soundcpu_mem_map, AS_PROGRAM, 8, sothello_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( soundcpu_io_map, AS_IO, 8, sothello_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x01, 0x01) AM_WRITE(msm_data_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(msm_cfg_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(soundcpu_busyflag_set_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(soundcpu_busyflag_reset_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(soundcpu_int_clear_w)
ADDRESS_MAP_END

/* sub 6809 */

void sothello_state::unlock_shared_ram()
{
	if(!m_subcpu->suspended(SUSPEND_REASON_HALT))
	{
		m_subcpu_status|=1;
	}
	else
	{
		//logerror("Sub cpu active! @%x\n",device().safe_pc());
	}
}

WRITE8_MEMBER(sothello_state::subcpu_status_w)
{
	unlock_shared_ram();
}

READ8_MEMBER(sothello_state::subcpu_status_r)
{
	unlock_shared_ram();
	return 0;
}

static ADDRESS_MAP_START( subcpu_mem_map, AS_PROGRAM, 8, sothello_state )
	AM_RANGE(0x0000, 0x1fff) AM_READWRITE(subcpu_status_r,subcpu_status_w)
	AM_RANGE(0x2000, 0x77ff) AM_RAM
	AM_RANGE(0x7800, 0x7fff) AM_RAM AM_SHARE("share1")  /* upper 0x800 of 6264 is shared  with main cpu */
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( sothello )
	PORT_START("INPUT1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("INPUT2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xf2, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWA")
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x1c, 0x10, "Timer" )
	PORT_DIPSETTING(    0x1c, "15" )
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPSETTING(    0x14, "25" )
	PORT_DIPSETTING(    0x10, "30" )
	PORT_DIPSETTING(    0x0c, "35" )
	PORT_DIPSETTING(    0x08, "40" )
	PORT_DIPSETTING(    0x04, "45" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWB")
	PORT_DIPNAME( 0xc0, 0x80, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x30, 0x10, "Matta" ) /* undo moves */
	PORT_DIPSETTING(    0x30, "0" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x08, 0x08, "Games for 2 players" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_BIT( 0x07, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

WRITE_LINE_MEMBER(sothello_state::irqhandler)
{
	m_subcpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(sothello_state::sothello_vdp_interrupt)
{
	m_maincpu->set_input_line(0, (state ? HOLD_LINE : CLEAR_LINE));
}

WRITE_LINE_MEMBER(sothello_state::adpcm_int)
{
	/* only 4 bits are used */
	m_msm->data_w(m_msm_data & 0x0f );
	m_soundcpu->set_input_line(0, ASSERT_LINE );
}

void sothello_state::machine_reset()
{
}

static MACHINE_CONFIG_START( sothello, sothello_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, MAINCPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(maincpu_mem_map)
	MCFG_CPU_IO_MAP(maincpu_io_map)

	MCFG_CPU_ADD("soundcpu",Z80, SOUNDCPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(soundcpu_mem_map)
	MCFG_CPU_IO_MAP(soundcpu_io_map)

	MCFG_CPU_ADD("sub",M6809, SUBCPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(subcpu_mem_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))

	/* video hardware */
	MCFG_V9938_ADD("v9938", "screen", VDP_MEM, MAIN_CLOCK)
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(sothello_state,sothello_vdp_interrupt))
	MCFG_V99X8_SCREEN_ADD_NTSC("screen", "v9938", MAIN_CLOCK)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2203, YM_CLOCK)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(sothello_state, irqhandler))
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWB"))
	MCFG_SOUND_ROUTE(0, "mono", 0.25)
	MCFG_SOUND_ROUTE(1, "mono", 0.25)
	MCFG_SOUND_ROUTE(2, "mono", 0.25)
	MCFG_SOUND_ROUTE(3, "mono", 0.50)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("msm",MSM5205, MSM_CLOCK)
	MCFG_MSM5205_VCLK_CB(WRITELINE(sothello_state, adpcm_int))      /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)  /* changed on the fly */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sothello )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "3.7c",   0x00000, 0x8000, CRC(47f97bd4) SHA1(52c9638f098fdcf66903fad7dafe3ab171758572) )
	ROM_LOAD( "4.8c",   0x08000, 0x8000, CRC(a98414e9) SHA1(6d14e1f9c79b95101e0aa101034f398af09d7f32) )
	ROM_LOAD( "5.9c",   0x10000, 0x8000, CRC(e5b5d61e) SHA1(2e4b3d85f41d0796a4d61eae40dd824769e1db86) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "1.7a",   0x0000, 0x8000, CRC(6951536a) SHA1(64d07a692d6a167334c825dc173630b02584fdf6) )
	ROM_LOAD( "2.8a",   0x8000, 0x8000, CRC(9c535317) SHA1(b2e69b489e111d6f8105e68fade6e5abefb825f7) )

	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD( "6.7f",   0x8000, 0x8000, CRC(ee80fc78) SHA1(9a9d7925847d7a36930f0761c70f67a9affc5e7c) )
ROM_END

GAME( 1986, sothello,  0,       sothello,  sothello, driver_device,  0, ROT0, "Success / Fujiwara", "Super Othello", 0 )
