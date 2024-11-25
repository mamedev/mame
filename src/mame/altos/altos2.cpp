// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

2017-11-03 Skeleton

Altos II terminal. Green screen.

Chips: Z80A, 2x Z80DART, Z80CTC, X2210D, 2x CRT9006, CRT9007, CRT9021A, 8x 6116

Other: Beeper.  Crystals: 4.9152, 8.000, 40.000

Keyboard: P8035L CPU, undumped 2716 labelled "358_2758", XTAL marked "4608-300-107 KSS4C"

************************************************************************************************************************************/

#include "emu.h"
#include "altos2_kbd.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/x2212.h"
#include "sound/beep.h"
//#include "video/crt9006.h"
#include "video/crt9007.h"
//#include "video/crt9021.h"
#include "screen.h"
#include "speaker.h"


namespace {

class altos2_state : public driver_device
{
public:
	altos2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_novram(*this, "novram")
		, m_vpac(*this, "vpac")
		, m_bell(*this, "bell")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
	{ }

	void altos2(machine_config &config);

private:
	u8 vpac_r(offs_t offset);
	void vpac_w(offs_t offset, u8 data);
	void video_mode_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_reset() override ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<x2210_device> m_novram;
	required_device<crt9007_device> m_vpac;
	required_device<beep_device> m_bell;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
};

void altos2_state::machine_reset()
{
	m_novram->recall(ASSERT_LINE);
	m_novram->recall(CLEAR_LINE);
}

u8 altos2_state::vpac_r(offs_t offset)
{
	return m_vpac->read(offset >> 1);
}

void altos2_state::vpac_w(offs_t offset, u8 data)
{
	m_vpac->write(offset >> 1, data);
}

void altos2_state::video_mode_w(u8 data)
{
	if (BIT(data, 5))
	{
		// D5 = 1 for 132-column mode
		m_vpac->set_unscaled_clock(40_MHz_XTAL / 12);
		m_vpac->set_character_width(6);
	}
	else
	{
		// D5 = 0 for 80-column mode
		m_vpac->set_unscaled_clock(40_MHz_XTAL / 20);
		m_vpac->set_character_width(10);
	}

	m_bell->set_state(BIT(data, 4));

	//logerror("Writing %02X to mode register at %s\n", data, machine().describe_context());
}

u32 altos2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void altos2_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("roms", 0);
	map(0xc000, 0xffff).ram().share("videoram");
}

void altos2_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw("dart1", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x04, 0x07).rw("dart2", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x08, 0x0b).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x0c, 0x0c).w(FUNC(altos2_state::video_mode_w));
	map(0x40, 0x7f).rw(m_novram, FUNC(x2210_device::read), FUNC(x2210_device::write));
	map(0x80, 0xff).rw(FUNC(altos2_state::vpac_r), FUNC(altos2_state::vpac_w));
}

static INPUT_PORTS_START(altos2)
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "dart1" },
	{ "dart2" },
	{ "ctc" },
	{ nullptr }
};

void altos2_state::altos2(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &altos2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &altos2_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	z80ctc_device &ctc(Z80CTC(config, "ctc", 8_MHz_XTAL / 2));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.set_clk<0>(4.9152_MHz_XTAL / 4);
	ctc.set_clk<1>(4.9152_MHz_XTAL / 4);
	ctc.set_clk<2>(4.9152_MHz_XTAL / 4);
	ctc.zc_callback<0>().set("dart1", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<0>().append("dart1", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().set("dart2", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().append("dart2", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<2>().set("dart1", FUNC(z80dart_device::rxtxcb_w));

	z80dart_device &dart1(Z80DART(config, "dart1", 8_MHz_XTAL / 2));
	dart1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dart1.out_txdb_callback().set("keyboard", FUNC(altos2_keyboard_device::rxd_w));

	z80dart_device &dart2(Z80DART(config, "dart2", 8_MHz_XTAL / 2)); // channel B not used for communications
	dart2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dart2.out_dtrb_callback().set(m_novram, FUNC(x2210_device::store)).invert(); // FIXME: no inverter should be needed

	ALTOS2_KEYBOARD(config, "keyboard").txd_callback().set("dart1", FUNC(z80dart_device::rxb_w));

	X2210(config, m_novram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(40_MHz_XTAL / 2, 960, 0, 800, 347, 0, 325);
	screen.set_screen_update(FUNC(altos2_state::screen_update));

	CRT9007(config, m_vpac, 40_MHz_XTAL / 20);
	m_vpac->set_screen("screen");
	m_vpac->set_character_width(10);
	m_vpac->int_callback().set("ctc", FUNC(z80ctc_device::trg3));

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_bell, 1000).add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( altos2 )
	ROM_REGION( 0x4000, "roms", 0 )
	ROM_LOAD( "us_v1.2_15732.u32", 0x0000, 0x2000, CRC(a85f7be0) SHA1(3cfa954c916258d86f7f745d10ec2ff5e33261b3) )
	ROM_LOAD( "us_v1.2_15733.u19", 0x2000, 0x2000, CRC(45ebe88a) SHA1(33f16b382a2b365122ebf5e5f7312f8afa45ad15) )

	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "us_v1.1_14410.u34", 0x0000, 0x2000, CRC(0ebb78bf) SHA1(96a1f7d34ff35037cbbc93049c0e2b9c9f11f1db) )
ROM_END

} // anonymous namespace


COMP(1983, altos2, 0, 0, altos2, altos2, altos2_state, empty_init, "Altos Computer Systems", "Altos II Terminal", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS)
