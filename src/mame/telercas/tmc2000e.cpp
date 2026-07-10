// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Telmac 2000E
    ------------
    (c) 1980 Telercas Oy, Finland

    CPU:        CDP1802A    1.75 MHz
    RAM:        8 KB
    ROM:        8 KB

    Video:      CDP1864     1.75 MHz
    Color RAM:  1 KB

    Colors:     8 fg, 4 bg
    Resolution: 64x192
    Sound:      frequency control, volume on/off
    Keyboard:   ASCII (RCA VP-601/VP-611), KB-16/KB-64

    SBASIC:     24.0


    Telmac TMC-121/111/112
    ----------------------
    (c) 198? Telercas Oy, Finland

    CPU:        CDP1802A    ? MHz

    Built from Telmac 2000 series cards. Huge metal box.

*/

#include "emu.h"

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "sound/cdp1864.h"

#include "screen.h"
#include "speaker.h"

#define SCREEN_TAG      "screen"
#define CDP1802_TAG     "cdp1802"
#define CDP1864_TAG     "cdp1864"

#define TMC2000E_COLORRAM_SIZE 0x100 // ???

namespace {


class tmc2000e_state : public driver_device
{
public:
	tmc2000e_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, CDP1802_TAG)
		, m_cti(*this, CDP1864_TAG)
		, m_cassette(*this, "cassette")
		, m_colorram(*this, "colorram")
		, m_key_row(*this, {"Y0", "Y1", "Y2", "Y3", "Y4", "Y5", "Y6", "Y7"})
		, m_run(*this, "RUN")
		, m_led(*this, "led1")
	{ }

	void tmc2000e(machine_config &config);

private:
	uint8_t vismac_r();
	void vismac_w(uint8_t data);
	uint8_t floppy_r();
	void floppy_w(uint8_t data);
	uint8_t ascii_keyboard_r();
	uint8_t io_r();
	void io_w(uint8_t data);
	void io_select_w(uint8_t data);
	void keyboard_latch_w(uint8_t data);
	int rdata_r();
	int bdata_r();
	int gdata_r();
	int clear_r();
	int ef2_r();
	int ef3_r();
	void q_w(int state);
	void dma_w(offs_t offset, uint8_t data);

	/* video state */
	int m_cdp1864_efx = 0;      /* EFx */
	uint8_t m_color = 0;

	/* keyboard state */
	int m_keylatch = 0;         /* key latch */
	void tmc2000e_io_map(address_map &map) ATTR_COLD;
	void tmc2000e_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<cassette_image_device> m_cassette;
	required_shared_ptr<uint8_t> m_colorram;
	required_ioport_array<8> m_key_row;
	required_ioport m_run;
	output_finder<> m_led;
};


/* Read/Write Handlers */

uint8_t tmc2000e_state::vismac_r()
{
	return 0;
}

void tmc2000e_state::vismac_w(uint8_t data)
{
}

uint8_t tmc2000e_state::floppy_r()
{
	return 0;
}

void tmc2000e_state::floppy_w(uint8_t data)
{
}

uint8_t tmc2000e_state::ascii_keyboard_r()
{
	return 0;
}

uint8_t tmc2000e_state::io_r()
{
	return 0;
}

void tmc2000e_state::io_w(uint8_t data)
{
}

void tmc2000e_state::io_select_w(uint8_t data)
{
}

void tmc2000e_state::keyboard_latch_w(uint8_t data)
{
	m_keylatch = data;
}

/* Memory Maps */

void tmc2000e_state::tmc2000e_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0xc000, 0xdfff).rom();
	map(0xfc00, 0xffff).writeonly().share("colorram");
}

void tmc2000e_state::tmc2000e_io_map(address_map &map)
{
	map(0x01, 0x01).w(m_cti, FUNC(cdp1864_device::tone_latch_w));
	map(0x02, 0x02).w(m_cti, FUNC(cdp1864_device::step_bgcolor_w));
	map(0x03, 0x03).rw(FUNC(tmc2000e_state::ascii_keyboard_r), FUNC(tmc2000e_state::keyboard_latch_w));
	map(0x04, 0x04).rw(FUNC(tmc2000e_state::io_r), FUNC(tmc2000e_state::io_w));
	map(0x05, 0x05).rw(FUNC(tmc2000e_state::vismac_r), FUNC(tmc2000e_state::vismac_w));
	map(0x06, 0x06).rw(FUNC(tmc2000e_state::floppy_r), FUNC(tmc2000e_state::floppy_w));
	map(0x07, 0x07).portr("DSW0").w(FUNC(tmc2000e_state::io_select_w));
}

/* Input Ports */

static INPUT_PORTS_START( tmc2000e )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DSW0")  // System Configuration DIPs
	PORT_DIPNAME( 0x80, 0x00, "Keyboard Type" )
	PORT_DIPSETTING(    0x00, "ASCII" )
	PORT_DIPSETTING(    0x80, "Matrix" )
	PORT_DIPNAME( 0x40, 0x00, "Operating System" )
	PORT_DIPSETTING(    0x00, "TOOL-2000-E" )
	PORT_DIPSETTING(    0x40, "Load from disk" )
	PORT_DIPNAME( 0x30, 0x00, "Display Interface" )
	PORT_DIPSETTING(    0x00, "PAL" )
	PORT_DIPSETTING(    0x10, "CDG-80" )
	PORT_DIPSETTING(    0x20, "VISMAC" )
	PORT_DIPSETTING(    0x30, "UART" )
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Run/Reset") PORT_CODE(KEYCODE_R) PORT_TOGGLE
INPUT_PORTS_END

/* Video */

int tmc2000e_state::rdata_r()
{
	return BIT(m_color, 2);
}

int tmc2000e_state::bdata_r()
{
	return BIT(m_color, 1);
}

int tmc2000e_state::gdata_r()
{
	return BIT(m_color, 0);
}

/* CDP1802 Interface */

int tmc2000e_state::clear_r()
{
	return BIT(m_run->read(), 0);
}

int tmc2000e_state::ef2_r()
{
	return m_cassette->input() < 0;
}

int tmc2000e_state::ef3_r()
{
	uint8_t data = ~(m_key_row[m_keylatch / 8])->read();

	return BIT(data, m_keylatch % 8);
}

void tmc2000e_state::q_w(int state)
{
	// turn CDP1864 sound generator on/off
	m_cti->aoe_w(state);

	// set Q led status
	m_led = state ? 1 : 0;

	// tape out
	m_cassette->output(state ? -1.0 : +1.0);

	// floppy control (FDC-6)
}

void tmc2000e_state::dma_w(offs_t offset, uint8_t data)
{
	m_color = (m_colorram[offset & 0x3ff]) & 0x07; // 0x04 = R, 0x02 = B, 0x01 = G

	m_cti->con_w(0); // HACK
	m_cti->dma_w(data);
}


/* Machine Initialization */

void tmc2000e_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_cdp1864_efx));
	save_item(NAME(m_keylatch));
}

void tmc2000e_state::machine_reset()
{
	m_cti->reset();

	// reset program counter to 0xc000
}

/* Machine Drivers */

void tmc2000e_state::tmc2000e(machine_config &config)
{
	// basic system hardware
	CDP1802(config, m_maincpu, 1.75_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmc2000e_state::tmc2000e_map);
	m_maincpu->set_addrmap(AS_IO, &tmc2000e_state::tmc2000e_io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(tmc2000e_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(tmc2000e_state::ef2_r));
	m_maincpu->ef3_cb().set(FUNC(tmc2000e_state::ef3_r));
	m_maincpu->q_cb().set(FUNC(tmc2000e_state::q_w));
	m_maincpu->dma_wr_cb().set(FUNC(tmc2000e_state::dma_w));

	// video hardware
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	CDP1864(config, m_cti, 1.75_MHz_XTAL).set_screen(SCREEN_TAG);
	m_cti->inlace_cb().set_constant(0);
	m_cti->int_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT);
	m_cti->dma_out_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_DMAOUT);
	m_cti->efx_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_EF1);
	m_cti->rdata_cb().set(FUNC(tmc2000e_state::rdata_r));
	m_cti->bdata_cb().set(FUNC(tmc2000e_state::bdata_r));
	m_cti->gdata_cb().set(FUNC(tmc2000e_state::gdata_r));
	m_cti->set_chrominance(RES_K(2.2), RES_K(1), RES_K(5.1), RES_K(4.7)); // unverified
	m_cti->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	CASSETTE(config, m_cassette).set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("8K").set_extra_options("40K");
}

/* ROMs */

ROM_START( tmc2000e )
	ROM_REGION( 0x10000, CDP1802_TAG, 0 )
	ROM_LOAD( "1", 0xc000, 0x0800, NO_DUMP )
	ROM_LOAD( "2", 0xc800, 0x0800, NO_DUMP )
	ROM_LOAD( "3", 0xd000, 0x0800, NO_DUMP )
	ROM_LOAD( "4", 0xd800, 0x0800, NO_DUMP )
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY        FULLNAME        STATE
COMP( 1980, tmc2000e, 0,      0,      tmc2000e, tmc2000e, tmc2000e_state, empty_init, "Telercas Oy", "Telmac 2000E", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
