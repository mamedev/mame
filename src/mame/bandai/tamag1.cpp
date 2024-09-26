// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:digshadow, Segher
/*******************************************************************************

Bandai Tamagotchi generation 1 hardware

Hardware notes:
- PCB label: TMG-M1
- Seiko Epson E0C6S46 MCU under epoxy
- 32*16 LCD screen + 8 custom segments
- 1-bit sound

TODO:
- change to SVG screen
- add the Mothra version that was recently dumped (has a E0C6S48)

*******************************************************************************/

#include "emu.h"

#include "cpu/e0c6200/e0c6s46.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "tama.lh"


namespace {

class tamag1_state : public driver_device
{
public:
	tamag1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_out_x(*this, "%u.%u", 0U, 0U)
	{ }

	void tama(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void tama_palette(palette_device &palette) const;
	E0C6S46_PIXEL_UPDATE(pixel_update);

	required_device<e0c6s46_device> m_maincpu;
	output_finder<16, 40> m_out_x;
};

void tamag1_state::machine_start()
{
	m_out_x.resolve();
}



/*******************************************************************************
    Video
*******************************************************************************/

E0C6S46_PIXEL_UPDATE(tamag1_state::pixel_update)
{
	// 16 COM(common) pins, 40 SEG(segment) pins from MCU, 32x16 LCD screen:
	static const int seg2x[0x28] =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		35,8, 9, 10,11,12,13,14,
		15,34,33,32,31,30,29,28,
		27,26,25,24,36,23,22,21,
		20,19,18,17,16,37,38,39
	};

	int y = com, x = seg2x[seg];
	if (cliprect.contains(x, y))
		bitmap.pix(y, x) = state;

	// 2 rows of indicators:
	// above screen: 0:meal, 1:lamp, 2:play, 3:medicine
	// under screen: 4:bath, 5:scales, 6:shout, 7:attention
	// they are on pin SEG8(x=35) + COM0-3, pin SEG28(x=36) + COM12-15

	// output to y.x
	m_out_x[y][x] = state;
}

void tamag1_state::tama_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xf1, 0xf0, 0xf9)); // background
	palette.set_pen_color(1, rgb_t(0x3c, 0x38, 0x38)); // lcd pixel
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

INPUT_CHANGED_MEMBER(tamag1_state::input_changed)
{
	// inputs are hooked up backwards here, because MCU input ports are all tied to its interrupt controller
	m_maincpu->set_input_line(param, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( tama )
	PORT_START("K0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tamag1_state, input_changed, E0C6S46_LINE_K00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tamag1_state, input_changed, E0C6S46_LINE_K01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, tamag1_state, input_changed, E0C6S46_LINE_K02)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void tamag1_state::tama(machine_config &config)
{
	// basic machine hardware
	E0C6S46(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->set_pixel_update_cb(FUNC(tamag1_state::pixel_update));
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(32.768_kHz_XTAL/1024);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40, 16);
	screen.set_visarea(0, 32-1, 0, 16-1);
	screen.set_screen_update("maincpu", FUNC(e0c6s46_device::screen_update));
	screen.set_palette("palette");
	config.set_default_layout(layout_tama);

	PALETTE(config, "palette", FUNC(tamag1_state::tama_palette), 2);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( tama )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "tama.bin", 0x0000, 0x3000, CRC(5c864cb1) SHA1(4b4979cf92dc9d2fb6d7295a38f209f3da144f72) )

	ROM_REGION( 0x3000, "maincpu:test", 0 )
	ROM_LOAD( "test.bin", 0x0000, 0x3000, CRC(4372220e) SHA1(6e13d015113e16198c0059b9d0c38d7027ae7324) ) // this rom is on the die too, test pin enables it?
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY   FULLNAME            FLAGS
SYST( 1997, tama, 0,      0,      tama,    tama,  tamag1_state, empty_init, "Bandai", "Tamagotchi (USA)", MACHINE_SUPPORTS_SAVE )
