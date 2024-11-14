// license:GPL-2.0+
// copyright-holders:Peter Trauner
/******************************************************************************
 Peter.Trauner@jk.uni-linz.ac.at September 2000

******************************************************************************/
/*
comquest plus
-------------
team concepts

laptop for childs

language german

lcd black/white, about 128x64, manual contrast control
keyboard and 2 button joypad
speaker 2, manual volume control:2 levels
cartridge slot, serial port

512 kbyte rom on print with little isolation/case
12 pin chip on print with little isolation/case (eeprom? at i2c bus)
cpu on print, soldered so nothing visible
32 kbyte sram

compuest a4 power printer
-------------------------
line oriented ink printer (12 pixel head)
for comquest serial port

3 buttons, 2 leds
bereit
druckqualitaet
zeilenvorschub/seitenvorschub

only chip on board (40? dil)
lsc43331op
team concepts
icq3250a-d
1f71lctctab973

*/

#include "emu.h"

#include "cpu/m6805/m68hc05.h"

#include "emupal.h"
#include "screen.h"


namespace {

class comquest_state : public driver_device
{
public:
	comquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void comquest(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	//uint8_t m_data[128][8];

	[[maybe_unused]] uint8_t comquest_read(offs_t offset);
	[[maybe_unused]] void comquest_write(offs_t offset, uint8_t data);

	uint32_t screen_update_comquest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void comquest_mem(address_map &map) ATTR_COLD;
};


uint8_t comquest_state::comquest_read(offs_t offset)
{
	uint8_t data=0;
	logerror("comquest read %.4x %.2x\n",offset,data);
	return data;
}

void comquest_state::comquest_write(offs_t offset, uint8_t data)
{
	logerror("comquest write %.4x %.2x\n",offset,data);
}

uint32_t comquest_state::screen_update_comquest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 128; y++) {
		for (int x = 0, j = 0; j < 8; j++, x += 8 * 4) {
#if 0
			m_gfxdecode->gfx(0)->opaque(bitmap,0, state->m_data[y][j],0,
					0,0,x,y);
#endif
		}
	}
	return 0;
}


void comquest_state::comquest_mem(address_map &map)
{
	map(0x8000, 0xffff).rom().region("gfx1", 0x4000);
}

static INPUT_PORTS_START( comquest )
	PORT_START("in0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EIN")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_START("in1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Druck") PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AUS")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1          !") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2          \"") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3          Paragraph") PORT_CODE(KEYCODE_3)
	PORT_START("in2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4          $") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5          %%") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6          &") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7          /") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8          (") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9          )") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0          =") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("sharp-s    ?") PORT_CODE(KEYCODE_EQUALS)
	PORT_START("in3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("acute") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("delete") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T          +") PORT_CODE(KEYCODE_T)
	PORT_START("in4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z          -") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U          4") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I          5") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O          6") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Diaresis-U") PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+          *") PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("capslock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_START("in5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G          mul") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H          div") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J          1") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K          2") PORT_CODE(KEYCODE_K)
	PORT_START("in6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L          3") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Diaresis-O")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Diaresis-A")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("left-shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_START("in7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B          root") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N          square") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",          ;") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".          :") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-          _") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("right-shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_START("in8")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Entf") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Strg") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AC")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Spieler    left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Stufe      up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Antwort    down") PORT_CODE(KEYCODE_DOWN)
	PORT_START("in9")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("           right") PORT_CODE(KEYCODE_RIGHT)
#if 0
/*
  left button, right button
  joypad:
  left button, right button
  4 or 8 directions
*/
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("") PORT_CODE(KEYCODE_7)
#endif

INPUT_PORTS_END

static const gfx_layout comquest_charlayout =
{
		8,8,
		256*8,                                    /* 256 characters */
		1,                      /* 1 bits per pixel */
		{ 0 },                  /* no bitplanes; 1 bit per pixel */
		/* x offsets */
		{
			0,
			1,
			2,
			3,
			4,
			5,
			6,
			7,
		},
		/* y offsets */
		{
			0,
			8,
			16,
			24,
			32,
			40,
			48,
			56,
		},
		8*8
};

static GFXDECODE_START( gfx_comquest )
	GFXDECODE_ENTRY( "gfx1", 0x0000, comquest_charlayout, 0, 2 )
GFXDECODE_END


void comquest_state::machine_reset()
{
//  uint8_t *mem=memregion("user1")->base();
//  membank(1)->set_base(mem+0x00000);
}


void comquest_state::comquest(machine_config &config)
{
	/* basic machine hardware */
	M68HC05L11(config, m_maincpu, 4000000);     /* 4000000? */

/*
    8 bit bus, integrated io, serial io?,

    starts at address zero?

    not saturn, although very similar hardware compared to hp48g (build into big plastic case)
    not sc61860, 62015?
    not cdp1802
    not tms9900?
    not z80
    not 6502, mitsubishi 740
    not i86
    not 6809
    not 68008?
    not tms32010
    not t11
    not arm
    not 8039
    not tms370
    not lh5801
    not fujitsu mb89150
    not epson e0c88
*/

	m_maincpu->set_addrmap(AS_PROGRAM, &comquest_state::comquest_mem);


	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(30);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*4, 128); /* 160 x 102 */
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(comquest_state::screen_update_comquest));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_comquest);
	PALETTE(config, "palette", palette_device::MONOCHROME);


	/* sound hardware */
	/* unknown ? */
}

ROM_START(comquest)
	ROM_REGION(0x1000,"maincpu",0)
	ROM_LOAD("hc05_internal.bin", 0x0000, 0x1000, NO_DUMP)
/*
000 +16kbyte graphics data? (first bytes: 80 0d 04 00 00 08 04 00 0f 02 04 01 00 10 04 01)
040 16kbyte code (first bytes: 00 00 00 00 9a cd 7c 9b cd 7c 98 4f c7 f1 1d 4f)
080 8kbyte code (first bytes: 00 00 00 00 9a cd 7c 9b cd 7c 98 10 a9 4f c7 f1)
0a0 8kbyte code (first bytes: 00 00 00 00 9a a6 00 c7 f5 ca cd 7c 9b cd 7c 98)
0c0 16kbyte code (first bytes: 00 00 00 00 a6 0a cd 7c c8 cd 7c 9b cd 7c 98 a6)
100 8kbyte code (first bytes: 00 00 00 00 a6 0a cd 7c c8 cd 7c 9b cd 7c 98 a6)
120 8kbyte data
140 16kbyte code (first bytes: 00 00 00 00 9a cd 7c 9b cd 7c 98 a6 0d c7 fd 0f)
180 16kbyte code (first bytes: 00 00 00 00 9a cd 7c 9b cd 7c 98 a6 00 c7 f2 2c)
1c0 16kbyte code (first bytes: 00 00 00 00 9a a6 00 c7 f3 00 c7 f3 02 a6 ff c7)
200 16kbyte code (first bytes: 00 00 00 00 9a a6 0d c7 fd 0f cd 7c 92 cd 7c 9b)
240 8kbyte code  (first bytes: 00 00 00 00 9d 9d 9d 9d 9d 9d 9d 9d cd 7c 9b cd)
260 16kbyte code (first bytes: 00 00 00 00 9a cd 7c 9b cd 7c 98 a6 00 b7 75 b7)
2a0 8kbyte code  (first bytes: 00 00 00 00 a6 0a cd 7c c8 cd 7c 9b cd 7c 98 a6)
2c0 8kbyte code  (first bytes: 00 00 00 00 a6 03 c7 fd 0f cd 7c 92 cd 7c 9b cd)
2e0 8kbyte code? (first bytes: 00 00 00 00 a6 0d c7 fd 0f cd 7c 92 cc 80 f1 20)
300 luts?+text   (first bytes: 60 02 fb 08 9a 11 a1 1f 75 26 dc 2f 6a 3b 26 46)
720 empty
740 empty
760 empty
780 16kbyte code (first bytes: 00 00 00 00 cd 7c 9b cd 7c 98 4f cd 7c ad a6 0a)
7c0 16kbyte code (first bytes: 00 00 00 00 9a cd 7c 9b cd 7c 98 4f c7 f1 6e c7)
 */

//  ROM_REGION(0x100,"gfx1",0)
	ROM_REGION(0x80000,"gfx1",0)
	ROM_LOAD("comquest.bin", 0x00000, 0x80000, CRC(2bf4b1a8) SHA1(8d1821cbde37cca2055b18df001438f7d138a8c1))
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY          FULLNAME                  FLAGS
CONS( 1995, comquest, 0,      0,      comquest, comquest, comquest_state, empty_init, "Data Concepts", "ComQuest Plus (German)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
