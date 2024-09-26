// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************************************

Genrad Futuredata 2301 Network Control Processor

2013-08-11 Skeleton

Has a number of plug-in daughter boards, some of which are:
- CPU board: Mostek Z80 CPU, 24MHz crystal, 2 eproms (U2.PB72.2300.4039 1.0 ; U23.N.C.P.2300.4023 3.0)
- IO board: 5.0688MHz crystal, D8253C, 2x S2651
- Memory board: 16k static ram consisting of 32x TMS2147H-7 chips, 1 prom? with sticker U36.2300.4035 1.0
- 2301 board: 2x D8253C, 4x S2651

Back panel has a number of DB25 sockets, labelled thus:
- Station 1-4
- Station 5-8
- EIA 1
- EIA 2
- Printer
- 3 unlabelled ones

A sticker on the back panel says: GenRad, Culver City CA, Model 2301-9001


- No schematic or documents are available. Everything in this driver is a guess.
- Only one rom has been dumped. The best guess would be U23, as this is adjacent to the Z80.
- Although there's no display device, it has display ram. This has been hooked up with a chargen rom
  from another system.

*************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "machine/scn_pci.h"
#include "video/i8275.h"
#include "screen.h"


namespace {

class grfd2301_state : public driver_device
{
public:
	grfd2301_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_p_chargen(*this, "chargen")
		, m_ma(0)
	{ }

	void grfd2301(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void drq_w(int state);
	void vrtc_w(int state);

	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_p_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<i8275_device> m_crtc;
	required_region_ptr<u8> m_p_chargen;

	uint16_t m_ma;
};

void grfd2301_state::drq_w(int state)
{
	if (state)
	{
		m_crtc->dack_w(m_p_videoram[m_ma++]);
		m_ma &= 0x7ff;
	}
}

void grfd2301_state::vrtc_w(int state)
{
	if (state)
		m_ma = 0;
}

I8275_DRAW_CHARACTER_MEMBER(grfd2301_state::draw_character)
{
	using namespace i8275_attributes;

	// HACK: adjust for incorrect character generator
	u8 lc = (linecount - 1) & 0x0f;
	u8 gfx = BIT(attrcode, LTEN) ? 0xff : (BIT(attrcode, VSP) || lc > 8) ? 0 : m_p_chargen[(charcode << 4) | lc];
	if (BIT(attrcode, RVV))
		gfx ^= 0xff;
	for (int i = 8; --i >= 0; )
		bitmap.pix(y, x++) = BIT(gfx, i) ? rgb_t::white() : rgb_t::black();
}

void grfd2301_state::mem_map(address_map &map)
{
	map(0xe000, 0xefff).rom().region("maincpu", 0);
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xffff).ram().share("videoram");
}

void grfd2301_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xc4, 0xc7).w("pci", FUNC(scn2651_device::write));
	map(0xc8, 0xcb).w("pit", FUNC(pit8253_device::write));
	map(0xce, 0xce).nopw(); // ?
	map(0xd8, 0xd8).nopr(); // ?
	map(0xf1, 0xf1).portr("CONFIG");
	map(0xf2, 0xf3).w("crtc", FUNC(i8275_device::write));
}

static INPUT_PORTS_START( grfd2301 )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x04, 0x00, "Refresh rate")
	PORT_CONFSETTING(0x04, "50 Hz")
	PORT_CONFSETTING(0x00, "60 Hz")
INPUT_PORTS_END

void grfd2301_state::machine_start()
{
	save_item(NAME(m_ma));
}

void grfd2301_state::machine_reset()
{
	m_maincpu->set_pc(0xe000);
}

void grfd2301_state::grfd2301(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &grfd2301_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &grfd2301_state::io_map);

	SCN2651(config, "pci", 5.0688_MHz_XTAL);

	PIT8253(config, "pit");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(15874560, 848, 0, 640, 312, 0, 288); // parameters guessed
	screen.set_screen_update(m_crtc, FUNC(i8275_device::screen_update));

	I8275(config, m_crtc, 15874560 / 8); // type and clock unknown
	m_crtc->set_screen("screen");
	m_crtc->set_character_width(8); // also guessed
	m_crtc->drq_wr_callback().set(FUNC(grfd2301_state::drq_w));
	m_crtc->vrtc_wr_callback().set(FUNC(grfd2301_state::vrtc_w));
	m_crtc->set_display_callback(FUNC(grfd2301_state::draw_character));
}

ROM_START( grfd2301 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "boot2301", 0x000, 0x1000, CRC(feec0cbd) SHA1(ec8138aca7ed489d86aaf2e07225c8d715440db7) )

	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

} // anonymous namespace


COMP( 198?, grfd2301, 0, 0, grfd2301, grfd2301, grfd2301_state, empty_init, "Genrad", "Futuredata 2301 Network Processor", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
