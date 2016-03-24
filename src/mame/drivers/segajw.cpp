// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

============================================================================

SEGA GOLDEN POKER SERIES "JOKER'S WILD" (REV.B)
(c) SEGA

MAIN CPU  : 68000 Z-80
CRTC      : HITACHI HD63484 (24KHz OUTPUT)
SOUND     : YM3438

14584B.EPR  ; MAIN BOARD  IC20 EPR-14584B (27C1000 MAIN-ODD)
14585B.EPR  ; MAIN BOARD  IC22 EPR-14585B (27C1000 MAIN-EVEN)
14586.EPR   ; MAIN BOARD  IC26 EPR-14586  (27C4096 BG)
14587A.EPR  ; SOUND BOARD IC51 EPR-14587A (27C1000 SOUND)

------------------------------------------------------------------

***************************************************************************/

/*
Also seem to be running on the same/similar hardware:
* Deuce's Wild (http://topline.royalflush.jp/modules/contents/?%A5%DE%A5%B7%A5%F3%A5%C7%A1%BC%A5%BF%A5%D9%A1%BC%A5%B9%2F%A5%D3%A5%C7%A5%AA%A5%DD%A1%BC%A5%AB%A1%BC%2FSEGA%2FDEUCE%27S_WILD)
* Draw Poker (http://topline.royalflush.jp/modules/contents/?%A5%DE%A5%B7%A5%F3%A5%C7%A1%BC%A5%BF%A5%D9%A1%BC%A5%B9%2F%A5%D3%A5%C7%A5%AA%A5%DD%A1%BC%A5%AB%A1%BC%2FSEGA%2FDRAW_POKER)
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/2612intf.h"
#include "video/h63484.h"
#include "video/ramdac.h"

#include "segajw.lh"

class segajw_state : public driver_device
{
public:
	segajw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu")
	{ }

	DECLARE_READ16_MEMBER(coin_counter_r);
	DECLARE_WRITE16_MEMBER(coin_counter_w);
	DECLARE_READ16_MEMBER(hopper_r);
	DECLARE_WRITE16_MEMBER(hopper_w);
	DECLARE_READ8_MEMBER(lamps_r);
	DECLARE_WRITE8_MEMBER(lamps_w);
	DECLARE_READ16_MEMBER(coinlockout_r);
	DECLARE_WRITE16_MEMBER(coinlockout_w);
	DECLARE_WRITE8_MEMBER(audiocpu_cmd_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_drop_start);
	DECLARE_CUSTOM_INPUT_MEMBER(coin_sensors_r);
	DECLARE_CUSTOM_INPUT_MEMBER(hopper_sensors_r);

protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT64      m_coin_start_cycles;
	UINT64      m_hopper_start_cycles;
	UINT8       m_coin_counter;
	UINT8       m_coin_lockout;
	UINT8       m_hopper_ctrl;
	UINT8       m_lamps[2];
};


READ16_MEMBER(segajw_state::coin_counter_r)
{
	return m_coin_counter ^ 0xff;
}

WRITE16_MEMBER(segajw_state::coin_counter_w)
{
	if(ACCESSING_BITS_0_7)
		m_coin_counter = data;
}

READ16_MEMBER(segajw_state::hopper_r)
{
	return m_hopper_ctrl;
}

WRITE16_MEMBER(segajw_state::hopper_w)
{
	if(ACCESSING_BITS_0_7)
	{
		m_hopper_start_cycles = data & 0x02 ? 0 : m_maincpu->total_cycles();
		m_hopper_ctrl = data;
	}
}

READ8_MEMBER(segajw_state::lamps_r)
{
	return m_lamps[offset];
}

WRITE8_MEMBER(segajw_state::lamps_w)
{
	for(int i=0; i<8; i++)
		output().set_lamp_value((offset * 8) + i, BIT(data, i));

	m_lamps[offset] = data;
}

READ16_MEMBER(segajw_state::coinlockout_r)
{
	return m_coin_lockout;
}

WRITE16_MEMBER(segajw_state::coinlockout_w)
{
	machine().bookkeeping().coin_lockout_w(0, data & 1);
	m_coin_lockout = data;

	for(int i=0; i<3; i++)
		output().set_indexed_value("towerlamp", i, BIT(data, 3 + i));
}

WRITE8_MEMBER(segajw_state::audiocpu_cmd_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INPUT_CHANGED_MEMBER( segajw_state::coin_drop_start )
{
	if (newval && !m_coin_start_cycles)
		m_coin_start_cycles = m_maincpu->total_cycles();
}

CUSTOM_INPUT_MEMBER( segajw_state::hopper_sensors_r )
{
	UINT8 data = 0;

	// if the hopper is active simulate the coin-out sensor
	if (m_hopper_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_hopper_start_cycles);

		if (diff > attotime::from_msec(100))
			data |= 0x01;

		if (diff > attotime::from_msec(200))
			m_hopper_start_cycles = m_maincpu->total_cycles();
	}

	return data;
}

CUSTOM_INPUT_MEMBER( segajw_state::coin_sensors_r )
{
	UINT8 data = 0;

	// simulates the passage of coins through multiple sensors
	if (m_coin_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_coin_start_cycles);

		if (diff > attotime::from_msec(20) && diff < attotime::from_msec(100))
			data |= 0x01;
		if (diff > attotime::from_msec(80) && diff < attotime::from_msec(200))
			data |= 0x02;
		if (diff <= attotime::from_msec(100))
			data |= 0x04;

		if (diff > attotime::from_msec(200))
			m_coin_start_cycles = 0;
	}

	return data;
}

static ADDRESS_MAP_START( segajw_map, AS_PROGRAM, 16, segajw_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM

	AM_RANGE(0x080000, 0x080001) AM_DEVREADWRITE("hd63484", h63484_device, status_r, address_w)
	AM_RANGE(0x080002, 0x080003) AM_DEVREADWRITE("hd63484", h63484_device, data_r, data_w)

	AM_RANGE(0x180000, 0x180001) AM_READ_PORT("DSW0")
	AM_RANGE(0x180004, 0x180005) AM_READWRITE8(soundlatch2_byte_r, audiocpu_cmd_w, 0x00ff)
	AM_RANGE(0x180008, 0x180009) AM_READ_PORT("DSW1")
	AM_RANGE(0x18000a, 0x18000b) AM_READ_PORT("DSW3")
	AM_RANGE(0x18000c, 0x18000d) AM_READ_PORT("DSW2")

	AM_RANGE(0x1a0000, 0x1a0001) AM_WRITE(coin_counter_w)
	AM_RANGE(0x1a0002, 0x1a0005) AM_READWRITE8(lamps_r, lamps_w, 0x00ff)
	AM_RANGE(0x1a0006, 0x1a0007) AM_READWRITE(hopper_r, hopper_w)
	AM_RANGE(0x1a000a, 0x1a000b) AM_READ(coin_counter_r)

	AM_RANGE(0x1a000e, 0x1a000f) AM_NOP

	AM_RANGE(0x1c0000, 0x1c0001) AM_READ_PORT("IN0")
	AM_RANGE(0x1c0002, 0x1c0003) AM_READ_PORT("IN1")
	AM_RANGE(0x1c0004, 0x1c0005) AM_READ_PORT("IN2")
	AM_RANGE(0x1c0006, 0x1c0007) AM_READ_PORT("IN3")
	AM_RANGE(0x1c000c, 0x1c000d) AM_READWRITE(coinlockout_r, coinlockout_w)

	AM_RANGE(0x280000, 0x280001) AM_DEVWRITE8("ramdac", ramdac_device, index_w, 0x00ff)
	AM_RANGE(0x280002, 0x280003) AM_DEVWRITE8("ramdac", ramdac_device, pal_w, 0x00ff)
	AM_RANGE(0x280004, 0x280005) AM_DEVWRITE8("ramdac", ramdac_device, mask_w, 0x00ff)

	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( segajw_audiocpu_map, AS_PROGRAM, 8, segajw_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( segajw_audiocpu_io_map, AS_IO, 8, segajw_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ymsnd", ym3438_device, read, write)
	AM_RANGE(0xc0, 0xc0) AM_READWRITE(soundlatch_byte_r, soundlatch2_byte_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( segajw_hd63484_map, AS_0, 16, segajw_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM
	AM_RANGE(0x80000, 0xbffff) AM_ROM AM_REGION("gfx1", 0)
ADDRESS_MAP_END


static INPUT_PORTS_START( segajw )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("1 Bet")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 )      PORT_NAME("Max Bet")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 )      PORT_NAME("Deal / Draw")

	PORT_START("IN1")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )      PORT_NAME("Double")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 )      PORT_NAME("Change")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reset")     PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x000d, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Meter")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Last Game")   PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("M-Door")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("D-Door")
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_SPECIAL )       PORT_CUSTOM_MEMBER(DEVICE_SELF, segajw_state, hopper_sensors_r, NULL)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Hopper Full")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Hopper Fill")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, segajw_state, coin_sensors_r, NULL)
	PORT_BIT( 0x00f8, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("COIN1") // start the coin drop sequence (see coin_sensors_r)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )   PORT_CHANGED_MEMBER(DEVICE_SELF, segajw_state, coin_drop_start, NULL)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0000, "Progressive" )   PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x0001, "Normal" )
	PORT_DIPSETTING(    0x0000, "Progressive" )
	PORT_DIPNAME( 0x0002, 0x0000, "Double Down" )   PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x0002, "With D.D" )
	PORT_DIPSETTING(    0x0000, "Without D.D" )
	PORT_DIPNAME( 0x0004, 0x0000, "Draw Cards" )    PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x0004, "Display" )
	PORT_DIPSETTING(    0x0000, "No Display" )
	PORT_DIPNAME( 0x0008, 0x0000, "Color Change" )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x0008, "Change" )
	PORT_DIPSETTING(    0x0000, "No Change" )
	PORT_DIPNAME( 0x0010, 0x0000, "Odds Table" )    PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x0010, "No Display" )
	PORT_DIPSETTING(    0x0000, "Display" )
	PORT_DIPNAME( 0x0060, 0x0000, "Play Mode" )     PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x0000, "Coin" )
	PORT_DIPSETTING(    0x0020, "Coin/Credit" )
	// PORT_DIPSETTING(    0x0040, "Coin/Credit" )
	PORT_DIPSETTING(    0x0060, "Credit" )
	PORT_DIPNAME( 0x0080, 0x0000, "Best Choice" )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x0080, "Display" )
	PORT_DIPSETTING(    0x0000, "No Display" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0000, "Denomination" )  PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x0000, "$25" )
	PORT_DIPSETTING(    0x0001, "$5" )
	PORT_DIPSETTING(    0x0002, "$1" )
	PORT_DIPSETTING(    0x0003, "50c" )
	PORT_DIPSETTING(    0x0004, "25c" )
	PORT_DIPSETTING(    0x0005, "10c" )
	PORT_DIPSETTING(    0x0006, "5c" )
	PORT_DIPSETTING(    0x0007, "Medal" )
	PORT_DIPNAME( 0x0008, 0x0000, "Max. Pay" )      PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x0008, "500" )
	PORT_DIPSETTING(    0x0000, "1000" )
	PORT_DIPNAME( 0x0010, 0x0000, "Max. Credit" )   PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x0010, "5000" )
	PORT_DIPSETTING(    0x0000, "1000" )
	PORT_DIPNAME( 0x0020, 0x0000, "$1200" )         PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x0020, "At. Pay" )
	PORT_DIPSETTING(    0x0000, "Credit/At. Pay" )
	PORT_DIPNAME( 0x00c0, 0x0000, "Max. Bet" )      PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x0000, "10" )
	PORT_DIPSETTING(    0x00c0, "5" )
	PORT_DIPSETTING(    0x0040, "3" )
	PORT_DIPSETTING(    0x0080, "1" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0001, 0x0001, "Meter" )         PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x0001, "Nevada" )
	PORT_DIPSETTING(    0x0000, "New Jersey" )
	PORT_DIPNAME( 0x0002, 0x0002, "Card Face" )     PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x0002, "Changeable" )
	PORT_DIPSETTING(    0x0000, "Original" )
	PORT_DIPNAME( 0x0004, 0x0004, "Card Back" )     PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x0004, "Changeable" )
	PORT_DIPSETTING(    0x0000, "Original" )
	PORT_DIPUNUSED( 0x0008, 0x0008)                 PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNUSED( 0x0010, 0x0010)                 PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNUSED( 0x0020, 0x0020)                 PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNUSED( 0x0040, 0x0040)                 PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNUSED( 0x0080, 0x0080)                 PORT_DIPLOCATION("SW3:8")

	PORT_START("DSW0")
	PORT_DIPNAME( 0x0001, 0x0000, "Jumper 1" )      PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "Jumper 2" )      PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0000, "Jumper 3" )      PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "Jumper 4" )      PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0000, "Jumper 5" )      PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0000, "Jumper 6" )      PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Jumper 7" )      PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0000, "Jumper 8" )      PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END


void segajw_state::machine_start()
{
	save_item(NAME(m_coin_start_cycles));
	save_item(NAME(m_hopper_start_cycles));
	save_item(NAME(m_coin_counter));
	save_item(NAME(m_coin_lockout));
	save_item(NAME(m_hopper_ctrl));
	save_item(NAME(m_lamps));
}


void segajw_state::machine_reset()
{
	m_coin_start_cycles = 0;
	m_hopper_start_cycles = 0;
	m_coin_counter = 0xff;
	m_coin_lockout = 0;
	m_hopper_ctrl = 0;
}

static ADDRESS_MAP_START( ramdac_map, AS_0, 8, segajw_state )
	AM_RANGE(0x000, 0x3ff) AM_DEVREADWRITE("ramdac",ramdac_device,ramdac_pal_r,ramdac_rgb666_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( segajw, segajw_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M68000,8000000) // unknown clock
	MCFG_CPU_PROGRAM_MAP(segajw_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", segajw_state, irq4_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000) // unknown clock
	MCFG_CPU_PROGRAM_MAP(segajw_audiocpu_map)
	MCFG_CPU_IO_MAP(segajw_audiocpu_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(2000))

	MCFG_NVRAM_ADD_NO_FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DEVICE("hd63484", h63484_device, update_screen)
	MCFG_SCREEN_SIZE(720, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 720-1, 0, 448-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_RAMDAC_ADD("ramdac", ramdac_map, "palette")

	MCFG_H63484_ADD("hd63484", 8000000, segajw_hd63484_map) // unknown clock

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM3438, 8000000)   // unknown clock
	MCFG_YM2612_IRQ_HANDLER(INPUTLINE("maincpu", 5))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( segajw )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "14584b.epr",   0x00001, 0x20000, CRC(d3a6d63d) SHA1(ce9d4769b7514294a91af1dfd7cd10ee40b3572c) )
	ROM_LOAD16_BYTE( "14585b.epr",   0x00000, 0x20000, CRC(556d0a62) SHA1(d2def433a511cbdebbe2cd0c8e51fc8c4ff1ed7b) )

	ROM_REGION( 0x20000, "audiocpu", 0 )
	ROM_LOAD( "14587a.epr",   0x00000, 0x20000, CRC(66163b6c) SHA1(88e994bcad86c58dc730a93b48226e9296df7667) )

	ROM_REGION16_BE( 0x80000, "gfx1", 0 )
	ROM_LOAD16_WORD_SWAP( "14586.epr",   0x00000, 0x80000, CRC(daeb0616) SHA1(17a8bb7137ad46a7c3ac07d22cbc4430e76e2f71) )
ROM_END


GAMEL( 1991, segajw,  0,   segajw,  segajw, driver_device,  0, ROT0, "Sega", "Joker's Wild (Rev. B)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_segajw )
