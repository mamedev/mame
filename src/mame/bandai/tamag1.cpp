// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:digshadow, Segher, azya
/*******************************************************************************

Bandai Tamagotchi generation 1 hardware

Hardware notes:
- PCB label: TMG-M1
- Seiko Epson E0C6S46 MCU under epoxy
- 32*16 LCD screen + 8 custom segments
- 1-bit sound

TODO:
- add the Mothra version that was recently dumped (has a E0C6S48)

*******************************************************************************/

#include "emu.h"

#include "cpu/e0c6200/e0c6s46.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


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
	void alienfev(machine_config &config);
	void venusdm(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void lcd_segment_w(offs_t offset, u8 data) { m_out_x[offset & 0xf][offset >> 4] = data; }

	required_device<e0c6s46_device> m_maincpu;
	output_finder<16, 40> m_out_x;
};

void tamag1_state::machine_start()
{
	m_out_x.resolve();
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K02)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( alienfev )
	PORT_START("K0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K00) // mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K01) // select
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K02) // sound
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K03) // handle
INPUT_PORTS_END

static INPUT_PORTS_START( venusdm )
	PORT_START("K1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K10)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K11)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(tamag1_state::input_changed), E0C6S46_LINE_K12)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void tamag1_state::tama(machine_config &config)
{
	// basic machine hardware
	E0C6S46(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->write_segs().set(FUNC(tamag1_state::lcd_segment_w));
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(32);
	screen.set_size(1119, 1080);
	screen.set_visarea_full();

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

void tamag1_state::alienfev(machine_config &config)
{
	// basic machine hardware
	E0C6S46(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->write_segs().set(FUNC(tamag1_state::lcd_segment_w));
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(32);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(39, 16);
	screen.set_visarea_full();
	screen.set_screen_update(m_maincpu, FUNC(e0c6s46_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

void tamag1_state::venusdm(machine_config &config)
{
	// basic machine hardware
	E0C6S46(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->write_segs().set(FUNC(tamag1_state::lcd_segment_w));
	m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(32);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32, 20);
	screen.set_visarea_full();
	screen.set_screen_update(m_maincpu, FUNC(e0c6s46_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

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

	ROM_REGION( 139072, "screen", 0)
	ROM_LOAD( "tama.svg", 0, 139072, CRC(9468b964) SHA1(ab49471db21a00a3b3a68da39c40da69da5d7e1b) )
ROM_END

ROM_START( alienfev )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "alienfev.bin", 0x0000, 0x3000, CRC(e561599c) SHA1(7927e198f8989861ba057150e59d1f4ad403c1d2) )
ROM_END

ROM_START( venusdm )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "venusdm.bin", 0x0000, 0x3000, CRC(2228b081) SHA1(22f6a2ede6259e76f1c8b9b50171c54d8a7de502) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS         INIT        COMPANY, FULLNAME, FLAGS
SYST( 1997, tama,     0,      0,      tama,     tama,     tamag1_state, empty_init, "Bandai", "Tamagotchi (Gen. 1, World)", MACHINE_SUPPORTS_SAVE | MACHINE_REQUIRES_ARTWORK )
SYST( 1997, alienfev, 0,      0,      alienfev, alienfev, tamag1_state, empty_init, "Epoch", "Chibi Pachi Alien Fever", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
SYST( 1997, venusdm,  0,      0,      venusdm,  venusdm,  tamag1_state, empty_init, "Nikko", "Beans Collection: Venus Diet Monogatari", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
