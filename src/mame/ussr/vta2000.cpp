// license:BSD-3-Clause
// copyright-holders:Robbbert, AJR
/***************************************************************************

VTA-2000 Terminal
Made at Ukrainian SSR, Vinnitsa Terminal Plant
(info from https://prog.world/dataart-has-opened-the-website-of-the-it-museum/ )

Board images : http://fotki.yandex.ru/users/lodedome/album/93699?p=0

BDP-15 board only

2010-11-29 Skeleton driver.

Better known on the net as BTA2000-15m.
It is a green-screen terminal, using RS232, and supposedly VT100 compatible.
The top line is a status line.

Note: port 0 bit 4 is NOT a speaker bit. See code at 027B.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8257.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "sound/spkrdev.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class vta2000_state : public driver_device
{
public:
	vta2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainpit(*this, "mainpit")
		, m_speaker(*this, "speaker")
		, m_crtc(*this, "crtc%u", 0U)
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
	{ }

	void vta2000(machine_config &config);

private:
	uint8_t memory_r(offs_t offset);
	void crtc_dack_w(offs_t offset, uint8_t data);
	uint8_t crtc_r(offs_t offset);
	void crtc_w(offs_t offset, uint8_t data);

	void output_00(uint8_t data);
	void speaker_w(int state);

	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<i8085a_cpu_device> m_maincpu;
	required_device<pit8253_device> m_mainpit;
	required_device<speaker_sound_device> m_speaker;
	required_device_array<i8275_device, 2> m_crtc;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
};

uint8_t vta2000_state::memory_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset ^ 1);
}

void vta2000_state::crtc_dack_w(offs_t offset, uint8_t data)
{
	if (BIT(offset, 0))
		m_crtc[0]->dack_w(data);
	m_crtc[1]->dack_w(data | 0x80);
}

uint8_t vta2000_state::crtc_r(offs_t offset)
{
	(void)m_crtc[1]->read(offset);
	return m_crtc[0]->read(offset);
}

void vta2000_state::crtc_w(offs_t offset, uint8_t data)
{
	m_crtc[0]->write(offset, data);
	m_crtc[1]->write(offset, data);
}

void vta2000_state::output_00(uint8_t data)
{
	m_mainpit->write_gate0(BIT(data, 4));
}

void vta2000_state::speaker_w(int state)
{
	m_speaker->level_w(state);
}

void vta2000_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x5fff).rom().region("roms", 0);
	map(0x8000, 0xc7ff).ram().share("videoram");
	map(0xc800, 0xc8ff).rom().region("roms", 0x5000); // FIXME: KR1601RR1 EAROM should be mapped here instead
}

void vta2000_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).w(FUNC(vta2000_state::output_00));
	map(0x20, 0x21).rw("pic", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x80, 0x88).rw("dmac", FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0xa0, 0xa1).rw(FUNC(vta2000_state::crtc_r), FUNC(vta2000_state::crtc_w));
	map(0xc0, 0xc0).rw("usart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0xc3, 0xc3).rw("usart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0xc8, 0xcb).w("brgpit", FUNC(pit8253_device::write));
	map(0xd0, 0xd3).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xe0, 0xe3).rw("mainpit", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

/* Input ports */
static INPUT_PORTS_START( vta2000 )
INPUT_PORTS_END


I8275_DRAW_CHARACTER_MEMBER(vta2000_state::draw_character)
{
	using namespace i8275_attributes;

	uint8_t dots = 0;
	if (BIT(attrcode, LTEN + 8))
		dots = 0xff;
	else if (!BIT(attrcode, VSP + 8))
		dots = m_p_chargen[BIT(attrcode, HLGT + 8) << 12 | (charcode & 0x7f) << 4 | linecount];
	if (BIT(attrcode, RVV + 8))
		dots = ~dots;

	uint32_t *p = &bitmap.pix(y, x);
	for (int i = 0; i < 8; i++)
		p[i] = BIT(dots, 7 - i) ? rgb_t::white() : rgb_t::black();
}


/* F4 Character Displayer */
static const gfx_layout vta2000_charlayout =
{
	8, 12,                  /* 8 x 12 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_vta2000 )
	GFXDECODE_ENTRY( "chargen", 0x0000, vta2000_charlayout, 0, 1 )
GFXDECODE_END

void vta2000_state::vta2000(machine_config &config)
{
	//constexpr auto CPU_CLOCK = XTAL(4'000'000) / 4; // too slow for CRTC DMA
	constexpr auto CPU_CLOCK = 2'000'000;
	constexpr auto DOT_CLOCK = 12'500'000; // guessed

	/* basic machine hardware */
	I8080(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &vta2000_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vta2000_state::io_map);
	m_maincpu->in_inta_func().set("pic", FUNC(pic8259_device::acknowledge));

	//KR1601RR1(config, "earom", 0);

	PIT8253(config, m_mainpit, 0);
	m_mainpit->set_clk<0>(500'000);
	m_mainpit->out_handler<0>().set(FUNC(vta2000_state::speaker_w));
	m_mainpit->set_clk<2>(500'000);
	m_mainpit->out_handler<2>().set("pic", FUNC(pic8259_device::ir7_w));

	pic8259_device &pic(PIC8259(config, "pic", 0));
	pic.in_sp_callback().set_constant(0);
	pic.out_int_callback().set_inputline(m_maincpu, 0);

	i8251_device &usart(I8251(config, "usart", CPU_CLOCK));
	usart.rxrdy_handler().set("pic", FUNC(pic8259_device::ir4_w));
	usart.syndet_handler().set("pic", FUNC(pic8259_device::ir1_w));

	pit8253_device &brgpit(PIT8253(config, "brgpit", 0));
	brgpit.set_clk<0>(1'228'800); // maybe
	brgpit.set_clk<1>(1'228'800);
	brgpit.out_handler<0>().set("usart", FUNC(i8251_device::write_rxc));
	brgpit.out_handler<1>().set("usart", FUNC(i8251_device::write_txc)); // or vice versa?

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pc_callback().set_constant(0xe0);

	i8257_device &dmac(I8257(config, "dmac", CPU_CLOCK));
	dmac.out_hrq_cb().set_inputline(m_maincpu, INPUT_LINE_HALT);
	dmac.out_hrq_cb().append("dmac", FUNC(i8257_device::hlda_w));
	dmac.out_tc_cb().set("pic", FUNC(pic8259_device::ir2_w)).invert();
	dmac.in_memr_cb().set(FUNC(vta2000_state::memory_r));
	dmac.out_iow_cb<2>().set(FUNC(vta2000_state::crtc_dack_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_raw(DOT_CLOCK, 800, 0, 640, 312, 0, 300);
	screen.set_screen_update(m_crtc[0], FUNC(i8275_device::screen_update));
	//screen.set_palette("palette");

	for (auto &crtc : m_crtc)
	{
		I8275(config, crtc, DOT_CLOCK / 8);
		crtc->set_character_width(8);
		crtc->set_screen("screen");
	}
	m_crtc[0]->set_display_callback(FUNC(vta2000_state::draw_character));
	m_crtc[0]->drq_wr_callback().set("dmac", FUNC(i8257_device::dreq2_w));
	m_crtc[0]->irq_wr_callback().set("pic", FUNC(pic8259_device::ir6_w));
	m_crtc[0]->set_next_crtc(m_crtc[1]);

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);
	GFXDECODE(config, "gfxdecode", "palette", gfx_vta2000);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.5);
}


/* ROM definition */
ROM_START( vta2000 )
	ROM_REGION( 0x6000, "roms", 0 )
	ROM_LOAD( "bdp-15_11.rom", 0x4000, 0x2000, CRC(d4abe3e9) SHA1(ab1973306e263b0f66f2e1ede50cb5230f8d69d5) )
	ROM_LOAD( "bdp-15_12.rom", 0x2000, 0x2000, CRC(4a5fe332) SHA1(f1401c26687236184fec0558cc890e796d7d5c77) )
	ROM_LOAD( "bdp-15_13.rom", 0x0000, 0x2000, CRC(b6b89d90) SHA1(0356d7ba77013b8a79986689fb22ef4107ef885b) )

	ROM_REGION(0x2000, "chargen", ROMREGION_INVERT )
	ROM_LOAD( "bdp-15_14.rom", 0x0000, 0x2000, CRC(a1dc4f8e) SHA1(873fd211f44713b713d73163de2d8b5db83d2143) )
ROM_END

} // Anonymous namespace

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY      FULLNAME    FLAGS
COMP( 198?, vta2000, 0,      0,      vta2000, vta2000, vta2000_state, empty_init, "<unknown>", "VTA2000-15m", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
