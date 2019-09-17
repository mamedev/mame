// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        Plan-80

        06/12/2009 Skeleton driver.

        Summary of Monitor commands:

        D - dump memory
        F - fill memory
        G - go (execute program at address)
        I - in from a port and display
        M - move?
        O - out to a port
        S - edit memory

        ToDo:
        - fix autorepeat on the keyboard
        - Add missing devices
        - Picture of unit shows graphics, possibly a PCG

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "emupal.h"
#include "screen.h"
#include "sound/spkrdev.h"
#include "speaker.h"


class plan80_state : public driver_device
{
public:
	plan80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_speaker(*this, "speaker")
	{ }

	void plan80(machine_config &config);

	void init_plan80();

private:
	enum
	{
		TIMER_BOOT
	};

	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_WRITE8_MEMBER(port09_w);
	DECLARE_WRITE8_MEMBER(port10_w);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void plan80_io(address_map &map);
	void plan80_mem(address_map &map);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	uint8_t m_kbd_row;
	bool m_spk_pol;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<speaker_sound_device> m_speaker;
};

READ8_MEMBER( plan80_state::port04_r )
{
	uint8_t data = 0xff;

	if (m_kbd_row == 0xfe)
		data = ioport("LINE0")->read();
	else
	if (m_kbd_row == 0xfd)
		data = ioport("LINE1")->read();
	else
	if (m_kbd_row == 0xfb)
		data = ioport("LINE2")->read();
	else
	if (m_kbd_row == 0xf7)
		data = ioport("LINE3")->read();
	else
	if (m_kbd_row == 0xef)
		data = ioport("LINE4")->read();

	return data;
}

WRITE8_MEMBER( plan80_state::port09_w )
{
	m_kbd_row = data;
}

WRITE8_MEMBER( plan80_state::port10_w )
{
	m_spk_pol ^= 1;
	m_speaker->level_w(m_spk_pol);
}


void plan80_state::plan80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).bankrw("boot");
	map(0x0800, 0xefff).ram();
	map(0xf000, 0xf7ff).ram().share("videoram");
	map(0xf800, 0xffff).rom();
}

void plan80_state::plan80_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x04, 0x04).r(FUNC(plan80_state::port04_r));
	map(0x09, 0x09).w(FUNC(plan80_state::port09_w));
	map(0x10, 0x10).w(FUNC(plan80_state::port10_w));
}

/* Input ports */
static INPUT_PORTS_START( plan80 ) // Keyboard was worked out by trial & error;'F' keys produce foreign symbols
	PORT_START("LINE0") /* FE */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A -") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('-')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q ! 1") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('!') PORT_CHAR('1')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F-shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P ?? 0") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('0')
	PORT_START("LINE1") /* FD */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Numbers") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X /") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('/')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D =") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('=')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E # 3") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('#') PORT_CHAR('3')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M .") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('.')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K [") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('[')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I ( 8") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('(') PORT_CHAR('8')
	PORT_START("LINE2") /* FB */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V ;") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G _") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('_')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T % 5") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('%') PORT_CHAR('5')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B ?") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H <") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y & 6") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('&') PORT_CHAR('6')
	PORT_START("LINE3") /* F7 */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C :") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F ^") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('^')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R $ 4") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('$') PORT_CHAR('4')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N ,") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR(',')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J >") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U \' 7") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('\'') PORT_CHAR('7')
	PORT_START("LINE4") /* EF */
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z *") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S +") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W \" 2") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('\"') PORT_CHAR('2')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("??") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L ]") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR(']')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O ) 9") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR(')') PORT_CHAR('9')
INPUT_PORTS_END


void plan80_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BOOT:
		/* after the first 4 bytes have been read from ROM, switch the ram back in */
		membank("boot")->set_entry(0);
		break;
	default:
		assert_always(false, "Unknown id in plan80_state::device_timer");
	}
}

void plan80_state::machine_reset()
{
	membank("boot")->set_entry(1);
	timer_set(attotime::from_usec(10), TIMER_BOOT);
}

void plan80_state::init_plan80()
{
	uint8_t *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0xf800);
}

uint32_t plan80_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;

	for (y = 0; y < 32; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma+48; x++)
			{
				chr = m_p_videoram[x];
				gfx = m_p_chargen[(chr << 3) | ra] ^ (BIT(chr, 7) ? 0xff : 0);

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
			}
		}
		ma+=64;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout plan80_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_plan80 )
	GFXDECODE_ENTRY( "chargen", 0x0000, plan80_charlayout, 0, 1 )
GFXDECODE_END


void plan80_state::plan80(machine_config &config)
{
	/* basic machine hardware */
	I8080(config, m_maincpu, 2048000);
	m_maincpu->set_addrmap(AS_PROGRAM, &plan80_state::plan80_mem);
	m_maincpu->set_addrmap(AS_IO, &plan80_state::plan80_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(plan80_state::screen_update));
	screen.set_size(48*6, 32*8);
	screen.set_visarea(0, 48*6-1, 0, 32*8-1);
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_plan80);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

/* ROM definition */
ROM_START( plan80 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pl80mon.bin", 0xf800, 0x0800, CRC(433fb685) SHA1(43d53c35544d3a197ab71b6089328d104535cfa5))

	ROM_REGION( 0x10000, "spare", 0 )
	ROM_LOAD_OPTIONAL( "pl80mod.bin", 0xf000, 0x0800, CRC(6bdd7136) SHA1(721eab193c33c9330e0817616d3d2b601285fe50))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "pl80gzn.bin", 0x0000, 0x0800, CRC(b4ddbdb6) SHA1(31bf9cf0f2ed53f48dda29ea830f74cea7b9b9b2))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY        FULLNAME   FLAGS
COMP( 1988, plan80, 0,      0,      plan80,  plan80, plan80_state, init_plan80, "Tesla Eltos", "Plan-80", MACHINE_NOT_WORKING )
