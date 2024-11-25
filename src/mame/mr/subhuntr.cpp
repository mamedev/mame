// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*

QTY     Type        clock   position            function
1x      2621                B8                  Universal Sync Generator (PAL)
3x      2636                H5,L5,N5            Programmable Video Interface
1x      2650        OSC/2 = 1.7897725 MHz       8-bit Microprocessor - main
1x      oscillator  3.579545 MHz

ROMs
QTY     Type    position    status
4x      2708    F7,H7,L7,N7 dumped
1x      N82S115 B2          dumped

RAMs
QTY     Type    position
4x      2112-2  G8,G9,H8,H9,L8,L9,M8,M9
2x      2101-1  B4,C4

Video uses approximate PAL timings derived from a 3.579MHz crystal
using a 2621 USG (227 clocks per line, 43-clock HBLANK, 312 lines,
43-line VBLANK).  This gives a resolution of 184*269, which is used for
the 2636 PVIs.  The clock is doubled to 7.15MHz for the text layer,
giving a resolution of 368*269.  Screen raw parameters are derived by
considering pixel/line zero to be the first period after the end of the
horizontal/vertical sync pulse.

*/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class subhuntr_state : public driver_device
{
public:
	subhuntr_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_pvi1_h5(*this, "pvi1")
		, m_pvi2_l5(*this, "pvi2")
		, m_pvi3_n5(*this, "pvi3")
		, m_gfx(*this, "gfxdecode")
		, m_txtram(*this, "txtram")
		, m_intreq_cnf(*this, "JMP_1_7")
	{
	}

	void subhuntr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void txtram_w(offs_t offset, u8 data);
	u8 intack_r();

	void pvi1_intreq_w(int state);
	void pvi2_intreq_w(int state);
	void pvi3_intreq_w(int state);

	void palette_init(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect);
	TILE_GET_INFO_MEMBER(tile_info);
	TIMER_CALLBACK_MEMBER(video_callback);

	void set_intreq(unsigned bit, u8 mask);
	void clr_intreq(unsigned bit);

	void subhuntr_map(address_map &map) ATTR_COLD;

	required_device<s2650_device>       m_maincpu;
	required_device<screen_device>      m_screen;
	required_device<s2636_device>       m_pvi1_h5;
	required_device<s2636_device>       m_pvi2_l5;
	required_device<s2636_device>       m_pvi3_n5;
	required_device<gfxdecode_device>   m_gfx;
	required_shared_ptr<u8>             m_txtram;

	required_ioport m_intreq_cnf;

	u8              m_intreqs = 0;

	bitmap_ind16    m_bitmap;
	emu_timer       *m_video_timer = nullptr;
	tilemap_t       *m_tilemap = nullptr;
};


/***************************************************************************

  Machine implementation

***************************************************************************/

void subhuntr_state::machine_start()
{
	m_video_timer = timer_alloc(FUNC(subhuntr_state::video_callback), this);
	m_tilemap = &machine().tilemap().create(*m_gfx, tilemap_get_info_delegate(*this, FUNC(subhuntr_state::tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 8);
	m_tilemap->set_transparent_pen(0);

	save_item(NAME(m_intreqs));

	m_screen->register_screen_bitmap(m_bitmap);

	m_video_timer->adjust(m_screen->time_until_pos(1, 0));
}

void subhuntr_state::machine_reset()
{
}

void subhuntr_state::txtram_w(offs_t offset, u8 data)
{
	if (m_txtram[offset] != data)
	{
		m_txtram[offset] = data;
		m_tilemap->mark_tile_dirty(offset);
	}
}

u8 subhuntr_state::intack_r()
{
	unsigned const source = count_leading_zeros_32(m_intreqs) - 24;
	u8 const vector = ((m_intreq_cnf->read() & 0x01) ? 0x91 : 0x11) | (source << 1);
	switch (source)
	{
	case 1:
		m_pvi1_h5->write_intack(ASSERT_LINE);
		m_pvi1_h5->write_intack(CLEAR_LINE);
		break;
	case 2:
		m_pvi2_l5->write_intack(ASSERT_LINE);
		m_pvi2_l5->write_intack(CLEAR_LINE);
		break;
	case 4:
		m_pvi3_n5->write_intack(ASSERT_LINE);
		m_pvi3_n5->write_intack(CLEAR_LINE);
		break;
	case 3:
	case 5:
	case 6:
		clr_intreq(7 - source);
		break;
	}
	return vector;
}

void subhuntr_state::pvi1_intreq_w(int state)
{
	if (state)  set_intreq(6, 0x02);
	else        clr_intreq(6);
}

void subhuntr_state::pvi2_intreq_w(int state)
{
	if (state)  set_intreq(5, 0x04);
	else        clr_intreq(5);
}

void subhuntr_state::pvi3_intreq_w(int state)
{
	if (state)  set_intreq(3, 0x08);
	else        clr_intreq(3);
}

void subhuntr_state::palette_init(palette_device &palette) const
{
	palette.set_pen_color(0, 0x00, 0x00, 0x00);
	palette.set_pen_color(1, 0x00, 0xff, 0x00);
	palette.set_pen_color(2, 0x00, 0x00, 0x00);
	palette.set_pen_color(3, 0x00, 0xff, 0xff);
	palette.set_pen_color(4, 0x00, 0x00, 0x00);
	palette.set_pen_color(5, 0xff, 0xff, 0x00);
	palette.set_pen_color(6, 0x00, 0x00, 0x00);
	palette.set_pen_color(7, 0xff, 0xff, 0xff);
}

u32 subhuntr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, rectangle const &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

TILE_GET_INFO_MEMBER(subhuntr_state::tile_info)
{
	u8 const tile = m_txtram[tile_index];
	u8 const code = tile & 0x3f;
	u8 const colour = (tile >> 6) & 0x03;
	tileinfo.set(0, code, colour, 0);
}

TIMER_CALLBACK_MEMBER(subhuntr_state::video_callback)
{
	int const y = m_screen->vpos();

	if (!y)
	{
		m_pvi1_h5->render_first_line();
		m_pvi2_l5->render_first_line();
		m_pvi3_n5->render_first_line();
	}
	else
	{
		m_pvi1_h5->render_next_line();
		m_pvi2_l5->render_next_line();
		m_pvi3_n5->render_next_line();
	}

	u16 const *src1 = &m_pvi1_h5->bitmap().pix(y);
	u16 const *src2 = &m_pvi2_l5->bitmap().pix(y);
	u16 const *src3 = &m_pvi3_n5->bitmap().pix(y);
	u16 *dst = &m_bitmap.pix(y);
	for (unsigned x = 0; x < m_bitmap.width(); x++, src1++, src2++, src3++, dst++)
	{
		u16 const pvi_val = S2636_PIXEL_COLOR(*src1 | *src2 | *src3);
		*dst = pvi_val;
		if (S2636_IS_PIXEL_DRAWN(*src1) && S2636_IS_PIXEL_DRAWN(*src2)) set_intreq(4, 0x10);
		if (S2636_IS_PIXEL_DRAWN(*src1) && S2636_IS_PIXEL_DRAWN(*src3)) set_intreq(2, 0x20);
		if (S2636_IS_PIXEL_DRAWN(*src2) && S2636_IS_PIXEL_DRAWN(*src3)) set_intreq(1, 0x40);
	}

	m_video_timer->adjust(m_screen->time_until_pos(y + 1, 0));
}

void subhuntr_state::set_intreq(unsigned bit, u8 mask)
{
	u8 const shifted = ((m_intreq_cnf->read() & mask) ? 1U : 0U) << bit;
	if (shifted & ~m_intreqs)
	{
		if (!m_intreqs)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
		m_intreqs |= shifted;
	}
}

void subhuntr_state::clr_intreq(unsigned bit)
{
	u8 const shifted = 1U << bit;
	if (shifted & m_intreqs)
	{
		m_intreqs &= ~shifted;
		if (!m_intreqs)
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

void subhuntr_state::subhuntr_map(address_map &map)
{
	map.global_mask(0x1fff);
	map.unmap_value_high();

	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x10ff).mirror(0x0100).ram(); // G8/G9
	map(0x1200, 0x12ff).mirror(0x0100).ram(); // H8/H9
	map(0x1400, 0x14ff).mirror(0x0100).ram(); // L8/L9
	map(0x1600, 0x16ff).mirror(0x0100).ram(); // M8/M9
	map(0x1800, 0x18ff).mirror(0x0100).w(FUNC(subhuntr_state::txtram_w)).share(m_txtram); // B4/C4
	map(0x1b00, 0x1bff).rw(m_pvi3_n5, FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	map(0x1d00, 0x1dff).rw(m_pvi2_l5, FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
	// Some kind of I/O at 1e00-1eff (enabled by PVI1 CE2)
	map(0x1f00, 0x1fff).rw(m_pvi1_h5, FUNC(s2636_device::read_data), FUNC(s2636_device::write_data));
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( subhuntr )

	// Factory jumpers controlling interrupt masking
	PORT_START("JMP_1_7")
	PORT_CONFNAME(0x01, 0x00, "JMP1")       // interrupt vector MSB select (indirect bit)
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x01, DEF_STR(On))
	PORT_CONFNAME(0x02, 0x02, "JMP2")       // PVI1 VBLANK/completion interrupt enable
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x02, DEF_STR(On))
	PORT_CONFNAME(0x04, 0x04, "JMP3")       // PVI2 VBLANK/completion interrupt enable
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x04, DEF_STR(On))
	PORT_CONFNAME(0x08, 0x08, "JMP4")       // PVI3 VBLANK/completion interrupt enable
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x08, DEF_STR(On))
	PORT_CONFNAME(0x10, 0x10, "JMP5")       // PVI1/PVI2 collision interrupt enable
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x10, DEF_STR(On))
	PORT_CONFNAME(0x20, 0x20, "JMP6")       // PVI1/PVI3 collision interrupt enable
	PORT_CONFSETTING(0x00, DEF_STR(Off))
	PORT_CONFSETTING(0x20, DEF_STR(On))
	PORT_CONFNAME(0x40, 0x40, "JMP7")       // PVI2/PVI3 collision interrupt enable
	PORT_CONFSETTING(0x00, DEF_STR(Off));
	PORT_CONFSETTING(0x40, DEF_STR(On));

INPUT_PORTS_END


/***************************************************************************

  Machine Config/Interface

***************************************************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( gfx_subhuntr )
	GFXDECODE_ENTRY("gfx1", 0, tiles8x8_layout, 0, 4)
GFXDECODE_END


void subhuntr_state::subhuntr(machine_config &config)
{
	S2650(config, m_maincpu, 3.579545_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &subhuntr_state::subhuntr_map);
	m_maincpu->sense_handler().set(m_screen, FUNC(screen_device::vblank));
	m_maincpu->intack_handler().set(FUNC(subhuntr_state::intack_r));

	S2636(config, m_pvi1_h5, 3.579545_MHz_XTAL);
	m_pvi1_h5->set_divider(2);
	m_pvi1_h5->intreq_cb().set(FUNC(subhuntr_state::pvi1_intreq_w));

	S2636(config, m_pvi2_l5, 3.579545_MHz_XTAL);
	m_pvi2_l5->set_divider(2);
	m_pvi2_l5->intreq_cb().set(FUNC(subhuntr_state::pvi2_intreq_w));

	S2636(config, m_pvi3_n5, 3.579545_MHz_XTAL);
	m_pvi3_n5->set_divider(2);
	m_pvi3_n5->intreq_cb().set(FUNC(subhuntr_state::pvi3_intreq_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(3.579545_MHz_XTAL*2, 227*2, 21*2, 205*2, 312, 29, 298);
	m_screen->set_palette("palette");
	m_screen->set_screen_update(FUNC(subhuntr_state::screen_update));

	GFXDECODE(config, m_gfx, "txtpal", gfx_subhuntr);

	PALETTE(config, "palette", palette_device::RGB_3BIT);
	PALETTE(config, "txtpal", FUNC(subhuntr_state::palette_init), 8);

	SPEAKER(config, "mono").front_center();
}



/******************************************************************************/

ROM_START( subhuntr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mr21.f7",   0x0000, 0x0400, CRC(27847939) SHA1(e6b41b511fefac1e1e207eff2dac8c2963d47c5c) )
	ROM_LOAD( "mr22.g7",   0x0400, 0x0400, CRC(e9af1ee8) SHA1(451e88407a120444377a58b06b65152c57503533) )
	ROM_LOAD( "mr25.l7",   0x0800, 0x0400, CRC(8271c975) SHA1(c7192658b50d781ab1b94c2e8cb75c5be3539820) )
	ROM_LOAD( "mr24.n7",   0x0c00, 0x0400, CRC(385c4944) SHA1(84050b0356c9a3a36528dba768f2684e28c6c7c4) )

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "82s115.b2", 0x0000, 0x0200, CRC(6946c9de) SHA1(956b4bebe6960a73609deb75e1493c4127fd7f77) )
ROM_END

} // anonymous namespace

GAME(1979, subhuntr,  0,        subhuntr, subhuntr, subhuntr_state, empty_init, ROT0, "Model Racing", "Sub Hunter (Model Racing)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
