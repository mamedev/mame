// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:digshadow, Segher, azya
/*******************************************************************************

Seiko Epson E0C6200-based handhelds, mostly electronic keychain toys from the
late-1990s. The first Tamagotchi games are on this MCU.

These were meant to stay on 24/7, so make sure to use save states if you want
to play the games for a longer time or remember high scores.

For most of the games, external artwork is required for the background inlays.
For the drivers that don't have an SVG screen, use -prescale or -nofilter to
disable bilinear filtering.

TODO:
- in stackch, you can still move around when the game is paused, maybe BTANB?
- digimon external port is unemulated
- alienfev unmapped reads/writes, or are they harmless?
- SVGs could be more accurate? Instead of 1:1 scans like with Game & Watch,
  they were created by tracing segments by hand from macro photos
- redo tamamot SVG, LCD was not available and azya redrew it from online photos
- add LCD deflicker like hh_sm510? see venusdm for example
- hook up LCD contrast, stackch and alienfev support user-defined contrast,
  but it doesn't look like any game uses it for eg. fade-out

*******************************************************************************/

#include "emu.h"

#include "cpu/e0c6200/e0c6s46.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// internal artwork
#include "hh_e0c6200_lcd.lh"


namespace {

class hh_e0c6200_state : public driver_device
{
public:
	hh_e0c6200_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_out_x(*this, "%u.%u", 0U, 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);
	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

protected:
	virtual void machine_start() override ATTR_COLD;

	void lcd_segment_w(offs_t offset, u8 data) { m_out_x[offset & 0xf][offset >> 4] = data; }

	required_device<e0c6s46_device> m_maincpu;
	output_finder<16, 51> m_out_x; // max 16 * 51
};

void hh_e0c6200_state::machine_start()
{
	m_out_x.resolve();
}



/*******************************************************************************

  Helper Functions

*******************************************************************************/

// generic input handlers

#define PORT_CHANGED_CB(x) \
	PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(hh_e0c6200_state::input_changed), E0C6S46_LINE_K00 + x)

INPUT_CHANGED_MEMBER(hh_e0c6200_state::input_changed)
{
	// inputs are hooked up backwards here, because MCU input ports are all tied to its interrupt controller
	m_maincpu->set_input_line(param, newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(hh_e0c6200_state::reset_button)
{
	// when an input is directly wired to MCU RESET pin
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
}



/*******************************************************************************

  Minidrivers (subclass, I/O, Inputs, Machine Config, ROM Defs)

*******************************************************************************/

/*******************************************************************************

  Bandai Tamagotchi (Generation 1)
  * PCB label: TMG-M1
  * Seiko Epson E0C6S46 MCU under epoxy
  * 32*16 LCD screen + 8 custom segments, 1-bit sound

  Generation 2 is on the exact same hardware. It's nearly the same game, they
  only changed the graphics.

*******************************************************************************/

class tama_state : public hh_e0c6200_state
{
public:
	tama_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_e0c6200_state(mconfig, type, tag)
	{ }

	void tama(machine_config &config);
};

// inputs

static INPUT_PORTS_START( tama )
	PORT_START("K0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_CB(0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_CB(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_CB(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// config

void tama_state::tama(machine_config &config)
{
	// basic machine hardware
	E0C6S46(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);
	m_maincpu->write_segs().set(FUNC(tama_state::lcd_segment_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1119, 1080);
	screen.set_visarea_full();

	config.set_default_layout(layout_hh_e0c6200_lcd);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tama )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "tama.bin", 0x0000, 0x3000, CRC(5c864cb1) SHA1(4b4979cf92dc9d2fb6d7295a38f209f3da144f72) )

	ROM_REGION( 0x3000, "maincpu:test", 0 )
	ROM_LOAD( "test.bin", 0x0000, 0x3000, CRC(4372220e) SHA1(6e13d015113e16198c0059b9d0c38d7027ae7324) ) // this rom is on the die too, test pin enables it?

	ROM_REGION( 139072, "screen", 0)
	ROM_LOAD( "tama.svg", 0, 139072, CRC(9468b964) SHA1(ab49471db21a00a3b3a68da39c40da69da5d7e1b) )
ROM_END

ROM_START( tamag2 )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "tamag2.bin", 0x0000, 0x3000, CRC(9f97539e) SHA1(09e5101b37636a314fc599d5d69b4846721b3c88) )

	ROM_REGION( 139072, "screen", 0)
	ROM_LOAD( "tama.svg", 0, 139072, CRC(9468b964) SHA1(ab49471db21a00a3b3a68da39c40da69da5d7e1b) )
ROM_END





/*******************************************************************************

  Bandai Tamagotchi Angel (Tenshitchi no Tamagotchi (aka Angel Gotch) in Japan)
  * PCB label: TAL-1, 00-83520-001
  * Seiko Epson E0C6S48 under epoxy
  * 32*16 LCD screen + 8 custom segments, 1-bit sound

  Mothra no Tamagotchi is on similar hardware.

*******************************************************************************/

class tamaang_state : public hh_e0c6200_state
{
public:
	tamaang_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_e0c6200_state(mconfig, type, tag)
	{ }

	void tamaang(machine_config &config);
};

// inputs

static INPUT_PORTS_START( tamaang )
	PORT_INCLUDE( tama )

	PORT_MODIFY("K0")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CHANGED_CB(3) PORT_NAME("Vibration Sensor")
INPUT_PORTS_END

// config

void tamaang_state::tamaang(machine_config &config)
{
	// basic machine hardware
	E0C6S48(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->set_osc3(1'000'000);
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);
	m_maincpu->write_segs().set(FUNC(tamaang_state::lcd_segment_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1119, 1080);
	screen.set_visarea_full();

	config.set_default_layout(layout_hh_e0c6200_lcd);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( tamaang )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "tamaang.bin", 0x0000, 0x4000, CRC(87bcb59f) SHA1(f5899bb7717756ac581451cf16cf97d909961c5c) )

	ROM_REGION( 139978, "screen", 0)
	ROM_LOAD( "tamaang.svg", 0, 139978, CRC(76f27f06) SHA1(b416275a12173316e053fa994c5fd68a4d5c1a5c) )
ROM_END

ROM_START( tamamot )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "tamamot.bin", 0x0000, 0x4000, CRC(85e4bee9) SHA1(74c1f6761724b7cbda8bca3113db78586b786d2d) )

	ROM_REGION( 138289, "screen", 0)
	ROM_LOAD( "tamamot.svg", 0, 138289, CRC(4e8210c2) SHA1(522536ae5bf744889c0d028c3a292bdf649f81e3) )
ROM_END





/*******************************************************************************

  Bandai Digital Monster (retroactively called Ver. 1)
  * PCB label: TDM-1, 00-83830-001
  * Seiko Epson E0C6S48 MCU under epoxy
  * external port, for connecting to another Digimon handheld
  * 32*16 LCD screen + 8 custom segments, 1-bit sound

  The sequels (Ver. 2 to Ver. 4) are on the same hardware, and are compatible
  with each other for battle mode.

*******************************************************************************/

class digimon_state : public hh_e0c6200_state
{
public:
	digimon_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_e0c6200_state(mconfig, type, tag)
	{ }

	void digimon(machine_config &config);

private:
	u8 extport_r();
	void extport_w(u8 data);
};

// handlers

u8 digimon_state::extport_r()
{
	// P20: external port data from linked digimon
	return 0xf;
}

void digimon_state::extport_w(u8 data)
{
	// P20: external port data to linked digimon
}

// inputs

static INPUT_PORTS_START( digimon )
	PORT_INCLUDE( tama )

	PORT_START("RESET")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POWER_ON ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(digimon_state::reset_button), 0) PORT_NAME("Reset")
INPUT_PORTS_END

// config

void digimon_state::digimon(machine_config &config)
{
	// basic machine hardware
	E0C6S48(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->set_osc3(1'000'000);
	m_maincpu->read_p<2>().set(FUNC(digimon_state::extport_r));
	m_maincpu->write_p<2>().set(FUNC(digimon_state::extport_w));
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);
	m_maincpu->write_segs().set(FUNC(digimon_state::lcd_segment_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1113, 1080);
	screen.set_visarea_full();

	config.set_default_layout(layout_hh_e0c6200_lcd);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( digimon )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "digimon.bin", 0x0000, 0x4000, CRC(08ffac1b) SHA1(1dde9b0aa81c8f4a1e22d3a79d4743833fc6cba7) )

	ROM_REGION( 158040, "screen", 0)
	ROM_LOAD( "digimon.svg", 0, 158040, CRC(0a6ad374) SHA1(e0bafc2c907dbe49e366ff76f2aef622e058f915) )
ROM_END

ROM_START( digimonv2 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "digimonv2.bin", 0x0000, 0x4000, CRC(19a9e54e) SHA1(ab860ca9f31f478532122cb9d20f59964a080a27) )

	ROM_REGION( 158040, "screen", 0)
	ROM_LOAD( "digimon.svg", 0, 158040, CRC(0a6ad374) SHA1(e0bafc2c907dbe49e366ff76f2aef622e058f915) )
ROM_END

ROM_START( digimonv3 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "digimonv3.bin", 0x0000, 0x4000, CRC(3000cf30) SHA1(0acd50e623e20d857e13bae150ac03405896cf2b) )

	ROM_REGION( 158040, "screen", 0)
	ROM_LOAD( "digimon.svg", 0, 158040, CRC(0a6ad374) SHA1(e0bafc2c907dbe49e366ff76f2aef622e058f915) )
ROM_END





/*******************************************************************************

  Epoch Chibi Pachi: Alien Fever
  * Seiko Epson E0C6S46 MCU
  * 39*16 LCD screen, 1-bit sound

  It's a Pachislot keychain game, the MCU constantly runs on the higher-speed OSC3.

*******************************************************************************/

class alienfev_state : public hh_e0c6200_state
{
public:
	alienfev_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_e0c6200_state(mconfig, type, tag)
	{ }

	void alienfev(machine_config &config);
};

// inputs

static INPUT_PORTS_START( alienfev )
	PORT_START("K0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_CB(0) PORT_NAME("Mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_CB(1) PORT_NAME("Select")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(2) PORT_NAME("Sound")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_CB(3) PORT_NAME("Handle")
INPUT_PORTS_END

// config

void alienfev_state::alienfev(machine_config &config)
{
	// basic machine hardware
	E0C6S46(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->set_osc3(1'000'000);
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(39, 16);
	screen.set_visarea_full();
	screen.set_screen_update(m_maincpu, FUNC(e0c6s46_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	config.set_default_layout(layout_hh_e0c6200_lcd);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( alienfev )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "alienfev.bin", 0x0000, 0x3000, CRC(e561599c) SHA1(7927e198f8989861ba057150e59d1f4ad403c1d2) )
ROM_END





/*******************************************************************************

  Nikko Beans Collection: Venus Diet Monogatari
  * Seiko Epson E0C6S46 MCU
  * 32*20 LCD screen, 1-bit sound

*******************************************************************************/

class venusdm_state : public hh_e0c6200_state
{
public:
	venusdm_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_e0c6200_state(mconfig, type, tag)
	{ }

	void venusdm(machine_config &config);

private:
	// reorder pixel coordinates
	void pixel_callback(int &dx, int &dy) { int x = dx; dx = dy | (dx / 20) << 4; dy = x % 20; }
};

// inputs

static INPUT_PORTS_START( venusdm )
	PORT_START("K1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_CB(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_CB(5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_CB(6)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// config

void venusdm_state::venusdm(machine_config &config)
{
	// basic machine hardware
	E0C6S46(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);
	m_maincpu->set_pixel_callback(FUNC(venusdm_state::pixel_callback));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32, 20);
	screen.set_visarea_full();
	screen.set_screen_update(m_maincpu, FUNC(e0c6s46_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	config.set_default_layout(layout_hh_e0c6200_lcd);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( venusdm )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "venusdm.bin", 0x0000, 0x3000, CRC(2228b081) SHA1(22f6a2ede6259e76f1c8b9b50171c54d8a7de502) )
ROM_END





/*******************************************************************************

  Tandy (Radio Shack division) Stack Challenge (model 60-2247)
  * Seiko Epson E0C6S46 MCU
  * 10*21 LCD screen + custom segments, 1-bit sound

  It's a brick game clone. The game is supposedly from 1991 (it's included in
  the 1992 Radio Shack catalog). Did E0C6S46 exist already, or is this a newer
  revision?

  BTANB:
  - it doesn't allow 2 button presses at the same time, making fast gameplay
    impossible (eg. left or right + rotate button)

*******************************************************************************/

class stackch_state : public hh_e0c6200_state
{
public:
	stackch_state(const machine_config &mconfig, device_type type, const char *tag) :
		hh_e0c6200_state(mconfig, type, tag)
	{ }

	void stackch(machine_config &config);
};

// inputs

static INPUT_PORTS_START( stackch )
	PORT_START("K0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY PORT_CHANGED_CB(0) PORT_NAME("Left / Level")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY PORT_CHANGED_CB(1) PORT_NAME("Down / Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY PORT_CHANGED_CB(2) PORT_NAME("Right / Height")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_CB(3) PORT_NAME("Rotate / Contrast")

	PORT_START("K1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POWER_ON ) PORT_CHANGED_CB(4) PORT_NAME("On / Off")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SELECT ) PORT_CHANGED_CB(5) PORT_NAME("Pause")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_CHANGED_CB(6) PORT_NAME("Sound")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// config

void stackch_state::stackch(machine_config &config)
{
	// basic machine hardware
	E0C6S46(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->set_osc3(1'000'000);
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);
	m_maincpu->write_segs().set(FUNC(stackch_state::lcd_segment_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(856, 1080);
	screen.set_visarea_full();

	config.set_default_layout(layout_hh_e0c6200_lcd);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

// roms

ROM_START( stackch )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "stackch.bin", 0x0000, 0x3000, CRC(28b9310a) SHA1(52b80d70aa7fc3b6323799403b3aba0e3d957f3b) )

	ROM_REGION( 133022, "screen", 0)
	ROM_LOAD( "stackch.svg", 0, 133022, CRC(caf74ad4) SHA1(2f2e836b0efe377305bb113a550f1cb4ec939273) )
ROM_END

} // anonymous namespace



/*******************************************************************************

  Game driver(s)

*******************************************************************************/

//    YEAR  NAME       PARENT   COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1997, tama,      0,       0,      tama,     tama,     tama_state,     empty_init, "Bandai", "Tamagotchi (Gen. 1, World)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
SYST( 1997, tamag2,    0,       0,      tama,     tama,     tama_state,     empty_init, "Bandai", "Tamagotchi (Gen. 2, Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
SYST( 1997, tamaang,   0,       0,      tamaang,  tamaang,  tamaang_state,  empty_init, "Bandai", "Tenshitchi no Tamagotchi (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
SYST( 1997, tamamot,   0,       0,      tamaang,  tama,     tamaang_state,  empty_init, "Bandai", "Mothra no Tamagotchi (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
SYST( 1997, digimon,   0,       0,      digimon,  digimon,  digimon_state,  empty_init, "Bandai", "Digital Monster (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NODEVICE_LAN )
SYST( 1998, digimonv2, 0,       0,      digimon,  digimon,  digimon_state,  empty_init, "Bandai", "Digital Monster Ver. 2 (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NODEVICE_LAN )
SYST( 1998, digimonv3, 0,       0,      digimon,  digimon,  digimon_state,  empty_init, "Bandai", "Digital Monster Ver. 3 (Japan)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK | MACHINE_NODEVICE_LAN )

SYST( 1997, alienfev,  0,       0,      alienfev, alienfev, alienfev_state, empty_init, "Epoch", "Chibi Pachi: Alien Fever", MACHINE_SUPPORTS_SAVE )

SYST( 1997, venusdm,   0,       0,      venusdm,  venusdm,  venusdm_state,  empty_init, "Nikko", "Beans Collection: Venus Diet Monogatari", MACHINE_SUPPORTS_SAVE )

SYST( 1991, stackch,   0,       0,      stackch,  stackch,  stackch_state,  empty_init, "Tandy Corporation", "Stack Challenge", MACHINE_SUPPORTS_SAVE )
