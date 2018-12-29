// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Callan Unistar Terminal

        2009-12-09 Skeleton driver.

        Chips used: i8275, AM9513, AM8085A-2, i8237, i8255, 2x 2651. XTAL 20MHz

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/am9513.h"
#include "machine/am9517a.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "video/i8275.h"
#include "emupal.h"
#include "screen.h"


class unistar_state : public driver_device
{
public:
	unistar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	void unistar(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	u8 dma_mem_r(offs_t offset);
	void dma_mem_w(offs_t offset, u8 data);

	void unistar_palette(palette_device &palette) const;
	I8275_DRAW_CHARACTER_MEMBER(draw_character);

	void unistar_io(address_map &map);
	void unistar_mem(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};


u8 unistar_state::dma_mem_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void unistar_state::dma_mem_w(offs_t offset, u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

void unistar_state::unistar_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x2fff).rom();
	map(0x8000, 0x97ff).ram();
}

void unistar_state::unistar_io(address_map &map)
{
	//map.unmap_value_high();
	map(0x00, 0x0f).rw("dmac", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x84, 0x84).portr("CONFIG");
	map(0x8c, 0x8d).rw("stc", FUNC(am9513_device::read8), FUNC(am9513_device::write8));
	map(0x94, 0x97).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x98, 0x99).rw("crtc", FUNC(i8275_device::read), FUNC(i8275_device::write));
	// ports used: 00,02,03(W),08(RW),09,0A,0B,0D,0F(W),80,81(R),82,83(W),84(R),8C,8D(W),94(R),97,98(W),99(RW)
	// if nonzero returned from port 94, it goes into test mode.
}

/* Input ports */
static INPUT_PORTS_START( unistar )
	PORT_START("CONFIG")
	PORT_DIPNAME(0x01, 0x01, "Screen Refresh Rate")
	PORT_DIPSETTING(0x01, "50 Hz")
	PORT_DIPSETTING(0x00, "60 Hz")
	PORT_BIT(0xfe, 0xfe, IPT_UNKNOWN)
INPUT_PORTS_END


void unistar_state::machine_reset()
{
}

void unistar_state::unistar_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0,   0, 0); // Black
	palette.set_pen_color(1, 0, 255, 0); // Full
	palette.set_pen_color(2, 0, 128, 0); // Dimmed
}

I8275_DRAW_CHARACTER_MEMBER(unistar_state::draw_character)
{
}

/* F4 Character Displayer */
static const gfx_layout unistar_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_unistar )
	GFXDECODE_ENTRY( "chargen", 0x0000, unistar_charlayout, 0, 1 )
GFXDECODE_END

void unistar_state::unistar(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 20_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &unistar_state::unistar_mem);
	m_maincpu->set_addrmap(AS_IO, &unistar_state::unistar_io);

	INPUT_MERGER_ANY_HIGH(config, "rst75").output_handler().set_inputline(m_maincpu, I8085_RST75_LINE);

	am9517a_device &dmac(AM9517A(config, "dmac", 20_MHz_XTAL / 4)); // Intel P8237A-5
	dmac.out_hreq_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	dmac.out_hreq_callback().append("dmac", FUNC(am9517a_device::hack_w));
	dmac.out_eop_callback().set("rst75", FUNC(input_merger_device::in_w<1>));
	dmac.in_memr_callback().set(FUNC(unistar_state::dma_mem_r));
	dmac.out_memw_callback().set(FUNC(unistar_state::dma_mem_w));
	dmac.out_iow_callback<2>().set("crtc", FUNC(i8275_device::dack_w));

	am9513_device &stc(AM9513(config, "stc", 8_MHz_XTAL));
	stc.fout_cb().set("stc", FUNC(am9513_device::source1_w));
	// TODO: figure out what OUT1-OUT4 should do (timer 5 is unused)

	I8255A(config, "ppi");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(20_MHz_XTAL, 954, 0, 720, 351, 0, 325);
	//screen.set_raw(20_MHz_XTAL, 990, 0, 720, 405, 0, 375);
	screen.set_screen_update("crtc", FUNC(i8275_device::screen_update));

	i8275_device &crtc(I8275(config, "crtc", 20_MHz_XTAL / 9));
	crtc.set_character_width(9);
	crtc.set_display_callback(FUNC(unistar_state::draw_character), this);
	crtc.set_screen("screen");
	crtc.drq_wr_callback().set("dmac", FUNC(am9517a_device::dreq2_w));
	crtc.irq_wr_callback().set("rst75", FUNC(input_merger_device::in_w<0>));

	GFXDECODE(config, "gfxdecode", "palette", gfx_unistar);
	PALETTE(config, "palette", FUNC(unistar_state::unistar_palette), 3);
}

/* ROM definition */
ROM_START( unistar )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "280010c.u48", 0x0000, 0x1000, CRC(613ef521) SHA1(a77459e91617d2882778ab2dada74fcb5f44e949))
	ROM_LOAD( "280011c.u49", 0x1000, 0x1000, CRC(6cc5e704) SHA1(fb93645f51d5ad0635cbc8a9174c61f96799313d))
	ROM_LOAD( "280012c.u50", 0x2000, 0x1000, CRC(0b9ca5a5) SHA1(20bf4aeacda14ff7a3cf988c7c0bff6ec60406c7))

	ROM_REGION( 0x0800, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "280014a.u1",  0x0000, 0x0800, CRC(a9e1b5b2) SHA1(6f5b597ee1417f1108ac5957b005a927acb5314a))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                FULLNAME                FLAGS
COMP( 198?, unistar, 0,      0,      unistar, unistar, unistar_state, empty_init, "Callan Data Systems", "Unistar 200 Terminal", MACHINE_IS_SKELETON )
