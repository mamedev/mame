// license:BSD-3-Clause
// copyright-holders:Curt Coder, Robbbert
/*****************************************************************************************

    ETI project 660, featured in the Australian magazine Electronics Today International.

    Commands:
    R - Reset (use this before any other instruction)
    S - Step
    0 - Enter modify memory mode
    2 - Save memory to cassette
    4 - Load memory from cassette
    6 - start binary program
    8 - start chip-8 program.

    To modify memory, press R, 0, enter 4-digit address, S, enter data, S, continue on.
    R to escape.

    To save a tape, enter the start address into 0400,0401 (big endian), and the end
    address into 0402,0403. Press R. Press record on the tape, press 2.

    To load a tape, enter the start and end addresses as above. Press R, 4. Screen goes
    black. Press play on tape. If the screen is still black after the tape ends, press R.

    All chip-8 programs start at 0600. The manual says the max end address is 7FF (gives
    512 bytes), but you can fill up all of memory (gives 2560 bytes).

    TODO:
    - sometimes there's no sound when started. You may need to hard reset until it beeps.
    - doesn't run programs for other chip-8 computers (this might be normal?)
    - we support BIN files, but have none to test with.
    - possible CPU bugs?:
      - in Invaders, can't shoot them
      - in Maze, the result is rubbish (works in Emma02 emulator v1.21, but not in v1.30)

**************************************************************************************************/

#include "emu.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/ram.h"
#include "machine/6821pia.h"
#include "machine/rescap.h"
#include "sound/cdp1864.h"
#include "imagedev/cassette.h"
#include "imagedev/snapquik.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

#define CDP1802_TAG     "ic3"
#define CDP1864_TAG     "ic4"
#define MC6821_TAG      "ic5"

enum
{
	LED_POWER = 0,
	LED_PULSE
};

class eti660_state : public driver_device
{
public:
	eti660_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, CDP1802_TAG)
		, m_cti(*this, CDP1864_TAG)
		, m_pia(*this, MC6821_TAG)
		, m_cassette(*this, "cassette")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_special(*this, "SPECIAL")
		, m_leds(*this, "led%d", 0U)
	{ }

	void eti660(machine_config &config);

private:
	u8 pia_r();
	void pia_w(u8 data);
	void colorram_w(offs_t offset, u8 data);
	int clear_r();
	int ef2_r();
	int ef4_r();
	void q_w(int state);
	void ca2_w(int state);
	void dma_w(offs_t offset, u8 data);
	u8 pia_pa_r();
	void pia_pa_w(u8 data);
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);
	required_shared_ptr<u8> m_p_videoram;

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1864_device> m_cti;
	required_device<pia6821_device> m_pia;
	required_device<cassette_image_device> m_cassette;
	required_ioport_array<4> m_io_keyboard;
	required_ioport m_special;
	output_finder<2> m_leds;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint16_t m_resetcnt;

	/* keyboard state */
	u8 m_keylatch = 0U;

	/* video state */
	u8 m_color_ram[0xc0]{};
	u8 m_color = 0U;
};


/* Read/Write Handlers */
// Schematic is wrong, PCB layout is correct: D0-7 swapped around on PIA.
u8 eti660_state::pia_r()
{
	u8 pia_offset = m_maincpu->get_memory_address() & 0x03;

	return bitswap<8>(m_pia->read(pia_offset), 0,1,2,3,4,5,6,7);
}

void eti660_state::pia_w(u8 data)
{
	u8 pia_offset = m_maincpu->get_memory_address() & 0x03;
	data = bitswap<8>(data,0,1,2,3,4,5,6,7);
	m_pia->write(pia_offset, data);
}

void eti660_state::ca2_w(int state) // test with Wipeout game - it should start up in colour
{
	m_cti->con_w(state);
}

void eti660_state::colorram_w(offs_t offset, u8 data)
{
	offset = m_maincpu->get_memory_address() - 0xc80;

	u8 colorram_offset = (((offset & 0x1f0) >> 1) | (offset & 0x07));

	if (colorram_offset < 0xc0)
		m_color_ram[colorram_offset] = data;
}

/* Memory Maps */

void eti660_state::mem_map(address_map &map)
{
	map.global_mask(0xfff);
	map(0x0000, 0x03ff).rom();
	map(0x0400, 0x047f).ram();
	map(0x0480, 0x05ff).ram().share("videoram");
	map(0x0600, 0x0fff).ram();
}

void eti660_state::io_map(address_map &map)
{
	map(0x01, 0x01).rw(m_cti, FUNC(cdp1864_device::dispon_r), FUNC(cdp1864_device::step_bgcolor_w));
	map(0x02, 0x02).rw(FUNC(eti660_state::pia_r), FUNC(eti660_state::pia_w));
	map(0x03, 0x03).w(FUNC(eti660_state::colorram_w));
	map(0x04, 0x04).rw(m_cti, FUNC(cdp1864_device::dispoff_r), FUNC(cdp1864_device::tone_latch_w));
}

/* Input Ports */
static INPUT_PORTS_START( eti660 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RESET") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("STEP") PORT_CODE(KEYCODE_S)
INPUT_PORTS_END

/* CDP1802 Interface */

int eti660_state::clear_r()
{
	// A hack to make the machine reset itself on
	// boot, like the real one does.
	if (m_resetcnt < 0xffff)
		m_resetcnt++;
	if (m_resetcnt == 0xf000)
		return 0;
	return BIT(m_special->read(), 0); // R key
}

int eti660_state::ef2_r()
{
	return m_cassette->input() < 0;
}

int eti660_state::ef4_r()
{
	return BIT(m_special->read(), 1); // S key
}

void eti660_state::q_w(int state)
{
	/* CDP1864 audio output enable */
	m_cti->aoe_w(state);

	/* PULSE led */
	m_leds[LED_PULSE] = state ? 1 : 0;

	/* tape output */
	m_cassette->output(state ? 1.0 : -1.0);
}

void eti660_state::dma_w(offs_t offset, u8 data)
{
	offset -= 0x480;

	m_color = 7;

	u8 colorram_offset = ((offset & 0x1f0) >> 1) | (offset & 0x07);

	if (colorram_offset < 0xc0)
		m_color = m_color_ram[colorram_offset];

	m_cti->dma_w(data);
}

/* PIA6821 Interface */

u8 eti660_state::pia_pa_r()
{
	/*

	    bit     description

	    PA0     keyboard row 0
	    PA1     keyboard row 1
	    PA2     keyboard row 2
	    PA3     keyboard row 3
	    PA4     keyboard column 0
	    PA5     keyboard column 1
	    PA6     keyboard column 2
	    PA7     keyboard column 3

	*/

	u8 i, data = 0xff;

	for (i = 0; i < 4; i++)
		if (BIT(m_keylatch, i))
			return m_io_keyboard[i]->read();

	return data;
}

void eti660_state::pia_pa_w(u8 data)
{
	/*

	    bit     description

	    PA0     keyboard row 0
	    PA1     keyboard row 1
	    PA2     keyboard row 2
	    PA3     keyboard row 3
	    PA4     keyboard column 0
	    PA5     keyboard column 1
	    PA6     keyboard column 2
	    PA7     keyboard column 3

	*/

	m_keylatch = bitswap<8>(data,0,1,2,3,4,5,6,7) ^ 0xff;
}

void eti660_state::machine_reset()
{
	m_resetcnt = 0;
	m_maincpu->reset();  // needed
}

void eti660_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_color_ram));
	save_item(NAME(m_color));
	save_item(NAME(m_keylatch));
	save_item(NAME(m_resetcnt));
}

QUICKLOAD_LOAD_MEMBER(eti660_state::quickload_cb)
{
	int const quick_length = image.length();
	std::vector<u8> quick_data;
	quick_data.resize(quick_length);
	int const read_ = image.fread( &quick_data[0], quick_length);
	if (read_ != quick_length)
		return std::make_pair(image_error::INVALIDIMAGE, "Cannot read the file");

	constexpr int QUICK_ADDR = 0x600;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	for (int i = 0; i < quick_length; i++)
	{
		if ((QUICK_ADDR + i) < 0x1000)
			space.write_byte(i + QUICK_ADDR, quick_data[i]);
	}

	// display a message about the loaded quickload
	if (image.is_filetype("bin"))
		image.message(" Quickload: size=%04X : start=%04X : end=%04X : Press 6 to start", quick_length, QUICK_ADDR, QUICK_ADDR+quick_length);
	else
		image.message(" Quickload: size=%04X : start=%04X : end=%04X : Press 8 to start", quick_length, QUICK_ADDR, QUICK_ADDR+quick_length);

	return std::make_pair(std::error_condition(), std::string());
}

/* Machine Drivers */

void eti660_state::eti660(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, XTAL(8'867'238)/5);
	m_maincpu->set_addrmap(AS_PROGRAM, &eti660_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &eti660_state::io_map);
	m_maincpu->wait_cb().set_constant(1);
	m_maincpu->clear_cb().set(FUNC(eti660_state::clear_r));
	m_maincpu->ef2_cb().set(FUNC(eti660_state::ef2_r));
	m_maincpu->ef4_cb().set(FUNC(eti660_state::ef4_r));
	m_maincpu->q_cb().set(FUNC(eti660_state::q_w));
	m_maincpu->dma_wr_cb().set(FUNC(eti660_state::dma_w));

	/* video hardware */
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	CDP1864(config, m_cti, XTAL(8'867'238)/5).set_screen("screen");
	m_cti->inlace_cb().set_constant(0);
	m_cti->int_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT);
	m_cti->dma_out_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_DMAOUT);
	m_cti->efx_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_EF1);
	m_cti->rdata_cb().set([this] () { return BIT(m_color, 0); });
	m_cti->bdata_cb().set([this] () { return BIT(m_color, 1); });
	m_cti->gdata_cb().set([this] () { return BIT(m_color, 2); });
	m_cti->set_chrominance(RES_K(2.2), RES_K(1), RES_K(4.7), RES_K(4.7)); // R7, R5, R6, R4
	m_cti->add_route(ALL_OUTPUTS, "mono", 0.25);

	/* devices */
	PIA6821(config, m_pia);
	m_pia->readpa_handler().set(FUNC(eti660_state::pia_pa_r));
	m_pia->writepa_handler().set(FUNC(eti660_state::pia_pa_w));
	m_pia->ca2_handler().set(FUNC(eti660_state::ca2_w));  // not working, bug in pia
	m_pia->irqa_handler().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT).invert(); // FIXME: use an input merger for these lines
	m_pia->irqb_handler().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT).invert();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("3K");

	/* quickload */
	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "bin,c8,ch8", attotime::from_seconds(2)));
	quickload.set_load_callback(FUNC(eti660_state::quickload_cb));
	quickload.set_interface("etiquik");
	SOFTWARE_LIST(config, "quik_list").set_original("eti660_quik");
}

/* ROMs */

ROM_START( eti660 )
	ROM_REGION( 0x10000, CDP1802_TAG, 0 )
	ROM_LOAD( "eti660.bin", 0x0000, 0x0400, CRC(811dfa62) SHA1(c0c4951e02f873f15560bdc3f35cdf3f99653922) )
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                            FULLNAME   FLAGS
COMP( 1981, eti660, 0,      0,      eti660,  eti660, eti660_state, empty_init, "Electronics Today International", "ETI-660 Learners' Microcomputer", MACHINE_SUPPORTS_SAVE )
