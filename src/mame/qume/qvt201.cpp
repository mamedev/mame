// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Qume QVT-201 & QVT-202 display terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "bus/rs232/rs232.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "video/scn2674.h"
#include "emupal.h"
#include "screen.h"


namespace {

class qvt201_state : public driver_device
{
public:
	qvt201_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainnmi(*this, "mainnmi")
		, m_eia(*this, "eia")
		, m_screen(*this, "screen")
		, m_p_chargen(*this, "chargen")
		, m_dataram(*this, "dataram")
		, m_attram(*this, "attram")
	{ }

	void qvt201(machine_config &config);

private:
	[[maybe_unused]] SCN2672_DRAW_CHARACTER_MEMBER(draw_character);

	void offset_w(uint8_t data);
	void keyboard_w(uint8_t data);
	uint8_t keyboard_r();
	void duart_out_w(uint8_t data);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_mainnmi;
	required_device<rs232_port_device> m_eia;
	required_device<screen_device> m_screen;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_dataram;
	required_shared_ptr<u8> m_attram;
};


SCN2672_DRAW_CHARACTER_MEMBER(qvt201_state::draw_character)
{
}

void qvt201_state::offset_w(uint8_t data)
{
}

void qvt201_state::keyboard_w(uint8_t data)
{
}

uint8_t qvt201_state::keyboard_r()
{
	return 1;
}

void qvt201_state::duart_out_w(uint8_t data)
{
	// OP0 = RTS (EIA pin 4)
	// OP1 = DTR (EIA pin 20)
	// OP2 not used?
	// OP3 = 132/_80
	// OP4 = SRV
	// OP5 = BLOCK/_UL
	// OP6 = preset MBC NMI flipflop
	// OP7 = _DATA/TALK (EIA pin 14)

	m_eia->write_rts(BIT(data, 0));
	m_eia->write_dtr(BIT(data, 1));
	m_mainnmi->in_w<1>(!BIT(data, 6));
}

void qvt201_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8800, 0x8fff).ram().share("nvram");
	map(0x9000, 0x9007).rw("crtc", FUNC(scn2672_device::read), FUNC(scn2672_device::write));
	map(0x9800, 0x980f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xa000, 0xa000).w(FUNC(qvt201_state::offset_w));
	map(0xa800, 0xa800).w(FUNC(qvt201_state::keyboard_w));
	map(0xb000, 0xb000).r(FUNC(qvt201_state::keyboard_r));
	map(0xc000, 0xdfff).ram().share("dataram");
	map(0xe000, 0xffff).ram().share("attram");
}

static INPUT_PORTS_START( qvt201 )
INPUT_PORTS_END

static const gfx_layout char_layout =
{
	8,10,
	RGN_FRAC(1,1), // 256
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	8*16
};

// ascii control code chars
// those are also at 0x80 to 0x9f in the normal char decode
// don't know why they are duplicated here
static const gfx_layout ctrl_char_layout =
{
	8,10,
	RGN_FRAC(1,4), // 32
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ 10*8, 11*8, 12*8, 13*8, 14*8, 16*8+10*8, 16*8+11*8, 16*8+12*8, 16*8+13*8, 16*8+14*8 },
	8*32
};

// 64 bytes of data remain undecoded
// byte 10 and 11 from 0x400 to 0x7ff in the rom
// (0x000 to 0x3ff are the control chars above, 0x800 to 0xfff is 0xff)

static GFXDECODE_START(chars)
	GFXDECODE_ENTRY("chargen", 0, char_layout, 0, 1)
	GFXDECODE_ENTRY("chargen", 0, ctrl_char_layout, 0, 1)
GFXDECODE_END

void qvt201_state::qvt201(machine_config &config)
{
	Z80(config, m_maincpu, 3.6864_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt201_state::mem_map); // IORQ is not used at all

	input_merger_device &mainint(INPUT_MERGER_ANY_HIGH(config, "mainint")); // open collector
	mainint.output_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	input_merger_device &mainnmi(INPUT_MERGER_ALL_HIGH(config, "mainnmi"));
	mainnmi.output_handler().set_inputline("maincpu", INPUT_LINE_NMI);

	scn2681_device &duart(SCN2681(config, "duart", 3.6864_MHz_XTAL)); // XTAL not directly connected
	duart.irq_cb().set("mainint", FUNC(input_merger_device::in_w<1>));
	duart.a_tx_cb().set(m_eia, FUNC(rs232_port_device::write_txd));
	duart.b_tx_cb().set("aux", FUNC(rs232_port_device::write_txd));
	duart.outport_cb().set(FUNC(qvt201_state::duart_out_w));

	RS232_PORT(config, m_eia, default_rs232_devices, nullptr);
	m_eia->rxd_handler().set("duart", FUNC(scn2681_device::rx_a_w));

	rs232_port_device &aux(RS232_PORT(config, "aux", default_rs232_devices, nullptr));
	aux.rxd_handler().set("duart", FUNC(scn2681_device::rx_b_w));
	aux.dsr_handler().set("duart", FUNC(scn2681_device::ip4_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5516APL-2 or uPD446C-2 + battery

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(48.654_MHz_XTAL / 3, 102 * 10, 0, 80 * 10, 265, 0, 250);
	//screen.set_raw(48.654_MHz_XTAL / 2, 170 * 9, 0, 132 * 9, 265, 0, 250);
	screen.set_screen_update("crtc", FUNC(scn2672_device::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);

	GFXDECODE(config, "gfxdecode", "palette", chars);

	scn2672_device &crtc(SCN2672(config, "crtc", 48.654_MHz_XTAL / 30));
	crtc.set_character_width(10); // 9 in 132-column mode
	crtc.intr_callback().set("mainint", FUNC(input_merger_device::in_w<0>));
	crtc.breq_callback().set("mainnmi", FUNC(input_merger_device::in_w<0>));
	crtc.set_screen("screen");
}


/**************************************************************************************************************

Qume QVT-201.
Chips: Z80A, SCN2681A, SCN2672B, 4x HM6116P-2, D446C-2, button battery
Crystals: (from schematics, unreadable on photo) 48.654 MHz (Y1), 3.6864 MHz (Y2)
Board is marked QVT-202 LB10 REV2 74 6 26.
Printed label on PCB: 301488-02 REV.2
                      MFG:607   QC:PASS

***************************************************************************************************************/

ROM_START( qvt201 )
	ROM_REGION(0x8000, "maincpu", 0) // "Program Contents ©1986 Qume Corp."
	ROM_LOAD( "390410-002.u11", 0x0000, 0x4000, CRC(69337561) SHA1(022e49bf5e8d76a3c2cc5af65630d3f77cc32bc1) )
	ROM_LOAD( "390410-001.u10", 0x4000, 0x4000, CRC(977cc138) SHA1(a019980ea6da2dce53617bced420014ab4e03ec8) )

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "301847-01.u42",  0x0000, 0x1000, CRC(546ed236) SHA1(312d57a7012f50327310bd11bda000149f13342e) )
ROM_END

} // anonymous namespace


COMP( 1986, qvt201, 0, 0, qvt201, qvt201, qvt201_state, empty_init, "Qume", "QVT-201 (Rev. T201VE)", MACHINE_IS_SKELETON )
