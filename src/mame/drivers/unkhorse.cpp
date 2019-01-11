// license:BSD-3-Clause
// copyright-holders:hap, Tomasz Slanina
/***************************************************************************

  unknown Japanese horse gambling game
  probably early 80s, manufacturer unknown

  from a broken PCB, labeled EFI TG-007
  8085A CPU + 8155 (for I/O and sound)
  8KB RAM mainly for bitmap video, and 512x4 RAM for color map

TODO:
- identify game!
- improve I/O:
  * more buttons/sensors?
  * horse_output_w bits
  * 6-pos dipswitch on the pcb, only 4 are known at the moment
- confirm colors and sound pitch

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"
#include "sound/spkrdev.h"
#include "screen.h"
#include "speaker.h"


class horse_state : public driver_device
{
public:
	horse_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_vram(*this, "vram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
	required_ioport_array<4> m_inp_matrix;
	required_shared_ptr<uint8_t> m_vram;

	std::unique_ptr<uint8_t[]> m_colorram;
	uint8_t m_output;

	DECLARE_READ8_MEMBER(colorram_r) { return m_colorram[(offset >> 2 & 0x1e0) | (offset & 0x1f)] | 0x0f; }
	DECLARE_WRITE8_MEMBER(colorram_w) { m_colorram[(offset >> 2 & 0x1e0) | (offset & 0x1f)] = data & 0xf0; }
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(output_w);

	virtual void machine_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void horse(machine_config &config);
	void horse_io_map(address_map &map);
	void horse_map(address_map &map);
};

void horse_state::machine_start()
{
	m_colorram = std::make_unique<uint8_t []>(0x200);
	save_pointer(NAME(m_colorram.get()), 0x200);
	save_item(NAME(m_output));
}



/***************************************************************************

  Video

***************************************************************************/

uint32_t horse_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = 0; x < 32; x++)
		{
			uint8_t data = m_vram[y << 5 | x];
			uint8_t color = m_colorram[(y << 1 & 0x1e0) | x] >> 4;

			for (int i = 0; i < 8; i++)
				bitmap.pix16(y, x << 3 | i) = (data >> i & 1) ? color : 0;
		}
	}

	return 0;
}



/***************************************************************************

  I/O

***************************************************************************/

void horse_state::horse_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x4000, 0x40ff).rw("i8155", FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x6000, 0x7fff).ram().share("vram");
	map(0x8000, 0x87ff).mirror(0x0800).rw(this, FUNC(horse_state::colorram_r), FUNC(horse_state::colorram_w));
}

void horse_state::horse_io_map(address_map &map)
{
	map(0x40, 0x47).rw("i8155", FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}


READ8_MEMBER(horse_state::input_r)
{
	return m_inp_matrix[m_output >> 6 & 3]->read();
}

WRITE8_MEMBER(horse_state::output_w)
{
	m_output = data;

	// d4: payout related
	// d6-d7: input mux
	// other bits: ?
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( horse )
	PORT_START("IN.0")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW:1,2,3")
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING( 0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING( 0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING( 0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING( 0x07, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING( 0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x08, 0x08, "UNK04" )             PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK05" )             PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Maximum Bet" )       PORT_DIPLOCATION("SW:6")
	PORT_DIPSETTING(    0x20, "20" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN.3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

MACHINE_CONFIG_START(horse_state::horse)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", I8085A, XTAL(12'000'000) / 2)
	MCFG_DEVICE_PROGRAM_MAP(horse_map)
	MCFG_DEVICE_IO_MAP(horse_io_map)

	MCFG_DEVICE_ADD("i8155", I8155, XTAL(12'000'000) / 4) // port A input, B output, C output but unused
	MCFG_I8155_IN_PORTA_CB(READ8(*this, horse_state, input_r))
	MCFG_I8155_OUT_PORTB_CB(WRITE8(*this, horse_state, output_w))
	MCFG_I8155_OUT_TIMEROUT_CB(WRITELINE("speaker", speaker_sound_device, level_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(horse_state, screen_update)
	MCFG_SCREEN_VBLANK_CALLBACK(INPUTLINE("maincpu", I8085_RST75_LINE))
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_3BIT_BGR("palette")

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("speaker", SPEAKER_SOUND)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( unkhorse )
	ROM_REGION( 0x04000, "maincpu", 0 )
	ROM_LOAD( "h1.bin", 0x0000, 0x0800, CRC(2b6fc963) SHA1(bdbf71bd0994231517ecf8188ea19cc7d42e5333) )
	ROM_LOAD( "h2.bin", 0x0800, 0x0800, CRC(b9653e78) SHA1(fd1369c734a0fec8def5b7819b14e4c0d1361896) )
	ROM_LOAD( "h3.bin", 0x1000, 0x0800, CRC(77ce7149) SHA1(e6a38f9eb676f84ec0e4c28e27d0aa959b97301f) )
	ROM_LOAD( "h4.bin", 0x1800, 0x0800, CRC(7e77d95d) SHA1(0e0b7acd622806b4eee3c691f05a04bd2989dbea) )
	ROM_LOAD( "h5.bin", 0x2000, 0x0800, CRC(e2c6abdb) SHA1(555f759897904e577a0a38abaaa7636e974192da) )
	ROM_LOAD( "h6.bin", 0x2800, 0x0800, CRC(8b179039) SHA1(079bec1ead7e04e29e552e3b48bec740a869751d) )
	ROM_LOAD( "h7.bin", 0x3000, 0x0800, CRC(db21fc82) SHA1(38cf58c4d33da3e919d058abb482566c8f70d276) )
ROM_END


GAME( 1981?, unkhorse, 0, horse, horse, horse_state, empty_init, ROT270, "<unknown>", "unknown Japanese horse gambling game", MACHINE_SUPPORTS_SAVE ) // copyright not shown, datecodes on pcb suggests early-1981
