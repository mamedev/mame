// license:BSD-3-Clause
// copyright-holders:hap
/***************************************************************************

  Bandai Tamagotchi generation 1 hardware
  * PCB label TMG-M1
  * Seiko Epson E0C6S46 MCU under epoxy

***************************************************************************/

#include "emu.h"
#include "cpu/e0c6200/e0c6s46.h"
#include "sound/speaker.h"

#include "tama.lh"


class tamag1_state : public driver_device
{
public:
	tamag1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	required_device<e0c6s46_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	DECLARE_WRITE8_MEMBER(speaker_w);

	DECLARE_PALETTE_INIT(tama);
	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
};


/***************************************************************************

  Video

***************************************************************************/

static E0C6S46_PIXEL_UPDATE_CB(tama_pixel_update)
{
	// 16 COM(common) pins, 40 SEG(segment) pins from MCU,
	// 32x16 LCD screen:
	static const int seg2x[0x28] =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		35, 8, 9,10,11,12,13,14,
		15,34,33,32,31,30,29,28,
		27,26,25,24,36,23,22,21,
		20,19,18,17,16,37,38,39
	};

	int y = com, x = seg2x[seg];
	if (cliprect.contains(x, y))
		bitmap.pix16(y, x) = state;

	// 2 rows of indicators:
	// above screen: 0:meal, 1:lamp, 2:play, 3:medicine
	// under screen: 4:bath, 5:scales, 6:shout, 7:attention

	// they are on pin SEG8(x=35) + COM0-3, pin SEG28(x=36) + COM12-15
	if (x == 35 && y < 4)
		output_set_lamp_value(y, state);
	else if (x == 36 && y >= 12)
		output_set_lamp_value(y-8, state);

	// output for svg2lay
	char buf[0x10];
	sprintf(buf, "%d.%d", y, x);
	output_set_value(buf, state);
}

PALETTE_INIT_MEMBER(tamag1_state, tama)
{
	palette.set_pen_color(0, rgb_t(0xf1, 0xf0, 0xf9)); // background
	palette.set_pen_color(1, rgb_t(0x3c, 0x38, 0x38)); // lcd pixel
}



/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(tamag1_state::speaker_w)
{
	// R43: speaker out
	m_speaker->level_w(data >> 3 & 1);
}



/***************************************************************************

  Inputs

***************************************************************************/

INPUT_CHANGED_MEMBER(tamag1_state::input_changed)
{
	// inputs are hooked up backwards here, because MCU input
	// ports are all tied to its interrupt controller
	int line = (int)(FPTR)param;
	int state = newval ? ASSERT_LINE : CLEAR_LINE;
	m_maincpu->set_input_line(line, state);
}

static INPUT_PORTS_START( tama )
	PORT_START("K0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tamag1_state, input_changed, (void *)E0C6S46_LINE_K00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tamag1_state, input_changed, (void *)E0C6S46_LINE_K01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tamag1_state, input_changed, (void *)E0C6S46_LINE_K02)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/***************************************************************************

  Machine Config

***************************************************************************/

static MACHINE_CONFIG_START( tama, tamag1_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", E0C6S46, XTAL_32_768kHz)
	MCFG_E0C6S46_PIXEL_UPDATE_CB(tama_pixel_update)
	MCFG_E0C6S46_WRITE_R_CB(4, WRITE8(tamag1_state, speaker_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(XTAL_32_768kHz/1024)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40, 16)
	MCFG_SCREEN_VISIBLE_AREA(0, 32-1, 0, 16-1)
	MCFG_DEFAULT_LAYOUT(layout_tama)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", e0c6s46_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(tamag1_state, tama)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( tama )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "test.b", 0x0000, 0x3000, CRC(4372220e) SHA1(6e13d015113e16198c0059b9d0c38d7027ae7324) ) // this rom is on the die too, test pin enables it?
	ROM_LOAD( "tama.b", 0x0000, 0x3000, CRC(5c864cb1) SHA1(4b4979cf92dc9d2fb6d7295a38f209f3da144f72) )
ROM_END


CONS( 1997, tama, 0, 0, tama, tama, driver_device, 0, "Bandai", "Tamagotchi (USA)", MACHINE_SUPPORTS_SAVE )
