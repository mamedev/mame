// license:BSD-3-Clause
// copyright-holders:Roberto Zandona'
/* Super Lucky Roulette?

driver by Roberto Zandona'
thanks to Angelo Salese for some precious advice

TO DO:
- check palette
- check blitter command 0x00
- screen orientation is wrong (should clearly be ROT90 or 270 with blitter mods)

Has 36 pin Cherry master looking edge connector

.u12 2764 stickered 1
.u19 27256 stickered 2
.u15 tibpal16l8-25 (checksum was 0)
.u56 tibpal16l8-25 (checksum was 0)
.u38 82s123
.u53 82s123

Z80 x2
Altera Ep1810LC-45
20.000 MHz crystal
video 464p10 x4 (board silkscreened 4416)
AY-3-8912A

ROM text showed SUPER LUCKY ROULETTE LEISURE ENT


NOTES:

blitter:
there are 4 registers:

reg[0] -> y
reg[1] -> x
reg[3] & 0x0f -> color
reg[3] & 0x10 -> y direction (to the up or to the down)
reg[3] & 0x20 -> x direction (to the right or to the left)
reg[3] & 0xc0 == 0x00 -> filled square
reg[3] & 0xc0 == 0x40 -> width used in y direction
reg[3] & 0xc0 == 0x80 -> width used in x direction
reg[3] & 0xc0 == 0xc0 -> width used in both directions
reg[2] -> width (number of pixel to draw)

with a write in reg[2] the command is executed

not handled commands with reg[3] & 0xc0 == 0x00


Stephh's notes (based on the game Z80 code and some tests) :

  - "Reset" Dip Switch :
      * OFF : no effect if NVRAM isn't corrupted
      * ON  : reset (at least) credits, bets and last numbers (clear NVRAM)
  - When "Coin Assistance" Dip Switch is ON, you can reset the number of credits
    by pressing BOTH "Service" (IN0 bit 2) and "Clear Credits" (IN1 bit 6).
  - When in "Service Mode", press "Add Fiche" (IN1 bit 0) to increase value displayed in green after "R".
  - When in "Service Mode", press RIGHT (IN1 bit 1) to clear statistics (only possible when "R00" is displayed).
  - You need at least 5 credits for outside bets

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "roul.lh"


namespace {

class roul_state : public driver_device
{
public:
	roul_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_soundlatch(*this, "soundlatch"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void roul(machine_config &config);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

private:
	uint8_t blitter_status_r();
	void blitter_cmd_w(offs_t offset, uint8_t data);
	void sound_latch_w(uint8_t data);
	void ball_w(uint8_t data);

	void roul_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void roul_cpu_io_map(address_map &map) ATTR_COLD;
	void roul_map(address_map &map) ATTR_COLD;
	void sound_cpu_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<generic_latch_8_device> m_soundlatch;
	output_finder<256> m_lamps;

	uint8_t m_reg[0x10];
	std::unique_ptr<uint8_t[]> m_videobuf;
	uint8_t m_lamp_old = 0;
};


#define VIDEOBUF_SIZE 256*256


void roul_state::roul_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1;

		int const bit7 = BIT(color_prom[i], 7);
		int const bit6 = BIT(color_prom[i], 6);

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		int const b = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 3);
		int const g = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		bit0 = BIT(color_prom[i], 4);
		bit1 = BIT(color_prom[i], 5);
		int const r = 0x0e * bit6 + 0x1f * bit7 + 0x43 * bit0 + 0x8f * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

uint8_t roul_state::blitter_status_r()
{
/*
code check bit 6 and bit 7
bit 7 -> blitter ready
bit 6 -> ??? (after unknown blitter command : [80][80][08][02])
*/
//  return 0x80; // blitter ready
//  logerror("Read unknown port $f5 at %04x\n",m_maincpu->pc());
	return machine().rand() & 0x00c0;
}

void roul_state::blitter_cmd_w(offs_t offset, uint8_t data)
{
	m_reg[offset] = data;
	if (offset==2)
	{
		int i,j;
		int width   = m_reg[2];
		int y       = m_reg[0];
		int x       = m_reg[1];
		int color   = m_reg[3] & 0x0f;
		int xdirection = 1, ydirection = 1;

		if (m_reg[3] & 0x10) ydirection = -1;
		if (m_reg[3] & 0x20) xdirection = -1;

		if (width == 0x00) width = 0x100;

		switch(m_reg[3] & 0xc0)
		{
			case 0x00: // m_reg[4] used?
				for (i = - width / 2; i < width / 2; i++)
					for (j = - width / 2; j < width / 2; j++)
						m_videobuf[(y + j) * 256 + x + i] = color;
				logerror("Blitter command 0 : [%02x][%02x][%02x][%02x][%02x]\n",m_reg[0],m_reg[1],m_reg[2],m_reg[3],m_reg[4]);
				break;
			case 0x40: // vertical line - m_reg[4] not used
				for (i = 0; i < width; i++ )
					m_videobuf[(y + i * ydirection) * 256 + x] = color;
				break;
			case 0x80: // horizontal line - m_reg[4] not used
				for (i = 0; i < width; i++ )
					m_videobuf[y * 256 + x + i * xdirection] = color;
				break;
			case 0xc0: // diagonal line - m_reg[4] not used
				for (i = 0; i < width; i++ )
					m_videobuf[(y + i * ydirection) * 256 + x + i * xdirection] = color;
		}
	}

}

void roul_state::sound_latch_w(uint8_t data)
{
	m_soundlatch->write(data & 0xff);
	m_soundcpu->set_input_line(0, HOLD_LINE);
}

void roul_state::ball_w(uint8_t data)
{
	int lamp = data;

	m_lamps[data] = 1;
	m_lamps[m_lamp_old] = 0;
	m_lamp_old = lamp;
}

void roul_state::roul_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x8fff).ram().share("nvram");
}

void roul_state::roul_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf0, 0xf4).w(FUNC(roul_state::blitter_cmd_w));
	map(0xf5, 0xf5).r(FUNC(roul_state::blitter_status_r));
	map(0xf8, 0xf8).portr("DSW");
	map(0xf9, 0xf9).w(FUNC(roul_state::ball_w));
	map(0xfa, 0xfa).portr("IN0");
	map(0xfd, 0xfd).portr("IN1");
	map(0xfe, 0xfe).w(FUNC(roul_state::sound_latch_w));
}

void roul_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x13ff).ram();
}

void roul_state::sound_cpu_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x00, 0x01).w("aysnd", FUNC(ay8910_device::address_data_w));
}

void roul_state::video_start()
{
	m_videobuf = make_unique_clear<uint8_t[]>(VIDEOBUF_SIZE);

	save_item(NAME(m_reg));
	save_pointer(NAME(m_videobuf), VIDEOBUF_SIZE);
	save_item(NAME(m_lamp_old));
}

uint32_t roul_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 256; i++)
		for (int j = 0; j < 256; j++)
			bitmap.pix(j, i) = m_videobuf[j * 256 + 255 - i];
	return 0;
}


/* verified from Z80 code */
static INPUT_PORTS_START( roul )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_NAME("Coin 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Service") PORT_CODE(KEYCODE_X)
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Payout") PORT_CODE(KEYCODE_Z)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Add Fiche")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Remove Fiche")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Clear Credits") PORT_CODE(KEYCODE_C)    /* see notes */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	/* single - split - street - square / double street / dozen - column - 1 to 18 - 19 to 36 - red - black - odd - even */
	PORT_DIPNAME( 0x81, 0x00, "Max Bet" )
	PORT_DIPSETTING(    0x00, "10 / 30 / 30" )
	PORT_DIPSETTING(    0x80, "20 / 40 / 50" )
	PORT_DIPSETTING(    0x01, "30 / 50 / 70" )
	PORT_DIPSETTING(    0x81, "40 / 60 / 90" )
	PORT_DIPNAME( 0x0e, 0x0e, "Percentage Payout" )
	PORT_DIPSETTING(    0x00, "94%" )
	PORT_DIPSETTING(    0x02, "88%" )
	PORT_DIPSETTING(    0x04, "82%" )
	PORT_DIPSETTING(    0x06, "75%" )
	PORT_DIPSETTING(    0x08, "68%" )
	PORT_DIPSETTING(    0x0a, "62%" )
	PORT_DIPSETTING(    0x0c, "56%" )
	PORT_DIPSETTING(    0x0e, "50%" )
	PORT_DIPNAME( 0x10, 0x10, "Reset Machine" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Doubble Odds" )
	PORT_DIPSETTING(    0x20, "With" )
	PORT_DIPSETTING(    0x00, "Without" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Assistance" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

void roul_state::roul(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &roul_state::roul_map);
	m_maincpu->set_addrmap(AS_IO, &roul_state::roul_cpu_io_map);

	Z80(config, m_soundcpu, 4000000);
	m_soundcpu->set_addrmap(AS_PROGRAM, &roul_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &roul_state::sound_cpu_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(roul_state::screen_update));
	screen.set_palette("palette");
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, "palette", FUNC(roul_state::roul_palette), 0x100);

	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	AY8910(config, "aysnd", 1000000).add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START(roul)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("roul.u19",    0x0000, 0x8000, CRC(1ec37876) SHA1(c2877646dad9daebc55db57d513ad448b1f4c923) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD("roul.u12",    0x0000, 0x1000, CRC(356fe025) SHA1(bca69e090a852454e921130afbdd28021b62c44e) )
	ROM_CONTINUE(0x0000,0x1000)

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "roul.u53",   0x0000, 0x0020, CRC(1965dfaa) SHA1(114eccd3e478902ac7dbb10b9425784231ff581e) )
	ROM_LOAD( "roul.u38",   0x0020, 0x0020, CRC(23ae22c1) SHA1(bf0383462976ec6341ffa8a173264ce820bc654a) )
ROM_END

} // anonymous namespace


GAMEL( 1990, roul, 0, roul, roul, roul_state, empty_init, ROT0, "bootleg", "Super Lucky Roulette", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_roul )
