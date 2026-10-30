// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Relisys TR-175 II

    Color terminal

    Hardware:
    - MC68000P12
    - 3x W24257-70L
    - SCN2681
    - KDA0476BCN-66 (RAMDAC)
    - HM82C11C
    - 28.322, 46.448, 11.0592 and one unknown XTAL
    - Beeper
    - Battery

    TODO:
    - Improve rendering (need hardware references)
    - Hook up AVDC clock changes
    - Verify RAM ranges
    - Implement HM82C11C and hookup printer

    Notes:
    - No exact model number in the firmware, might be a different model
    - Use Shift+NumpadMinus to enter setup

***************************************************************************/

#include "emu.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "sound/spkrdev.h"
#include "video/bt47x.h"
#include "video/scn2674.h"
#include "screen.h"
#include "speaker.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tr175_state : public driver_device
{
public:
	tr175_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ramdac(*this, "ramdac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_chargen(*this, "chargen"),
		m_vram(*this, "vram"),
		m_kbdc(*this, "kbdc"),
		m_duart(*this, "duart"),
		m_mainport(*this, "mainport"),
		m_auxport(*this, "auxport"),
		m_speaker_sound(*this, "speaker_sound"),
		m_nvram(*this, "nvram", 0x80, ENDIANNESS_BIG)
	{ }

	void tr175(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	uint8_t keyboard_r();
	void keyboard_w(uint8_t data);
	void kbd_clk_w(int state);
	void kbd_rxd_w(int state);

	void charram_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	void char_map(address_map &map) ATTR_COLD;
	void attr_map(address_map &map) ATTR_COLD;

	uint8_t avdc_char_r(offs_t offset);
	void avdc_char_w(offs_t offset, uint8_t data);
	uint8_t avdc_attr_r(offs_t offset);
	void avdc_attr_w(offs_t offset, uint8_t data);

	void video_control_w(uint8_t data);

	SCN2674_DRAW_CHARACTER_MEMBER(draw_character);

	void volume_w(uint8_t data);

	void duart_output_w(uint8_t data);

	required_device<m68000_device> m_maincpu;
	required_device<bt476_device> m_ramdac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_chargen;
	required_shared_ptr<uint16_t> m_vram;
	required_device<pc_kbdc_device> m_kbdc;
	required_device<scn2681_device> m_duart;
	required_device<rs232_port_device> m_mainport;
	required_device<rs232_port_device> m_auxport;
	required_device<speaker_sound_device> m_speaker_sound;
	memory_share_creator<uint8_t> m_nvram;

	bool m_kbd_clk = true;
	bool m_kbd_rxd = true;
	uint8_t m_video_control = 0;
	uint8_t m_duart_output = 0;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void tr175_state::mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("maincpu", 0);
	map(0xfe8000, 0xfebfff).ram().w(FUNC(tr175_state::charram_w)).share(m_chargen);
	map(0xfefe00, 0xfefeff).lrw8(
		[this](offs_t offset) { return m_nvram[offset]; }, "nvram_r",
		[this](offs_t offset, uint8_t data) { m_nvram[offset] = data; }, "nvram_w").umask16(0x00ff);
	map(0xff0000, 0xffbfff).ram().share(m_vram);
	map(0xffe000, 0xffe01f).rw("duart", FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0xff00);
	map(0xffe400, 0xffe40f).rw("avdc", FUNC(scn2674_device::read), FUNC(scn2674_device::write)).umask16(0xff00);
	map(0xffe800, 0xffe805).unmaprw(); //.rw("pai", FUNC(um82c11_device::read), FUNC(um82c11_device::write)).umask16(0xff00);
	map(0xffec00, 0xffec00).w(FUNC(tr175_state::volume_w));
	map(0xffec01, 0xffec01).w(FUNC(tr175_state::video_control_w));
	map(0xfff000, 0xfff000).w(FUNC(tr175_state::keyboard_w));
	map(0xfff400, 0xfff400).r(FUNC(tr175_state::keyboard_r));
	map(0xfff800, 0xfff807).r(m_ramdac, FUNC(bt476_device::read)).umask16(0x00ff);
	map(0xfffc00, 0xfffc07).w(m_ramdac, FUNC(bt476_device::write)).umask16(0x00ff);
}


//**************************************************************************
//  INPUT PORT HANDLING
//**************************************************************************

static INPUT_PORTS_START( tr175 )
INPUT_PORTS_END

uint8_t tr175_state::keyboard_r()
{
	// 765432--  unknown
	// ------1-  keyboard serial data
	// -------0  keyboard serial clock

	uint8_t data = 0;

	data |= m_kbd_rxd << 1;
	data |= m_kbd_clk << 0;

	// reading the state clears the interrupt
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(M68K_IRQ_IPL2, CLEAR_LINE);

	return data;
}

void tr175_state::keyboard_w(uint8_t data)
{
	// 765432--  unknown
	// ------1-  serial data to keyboard
	// -------0  serial clock to keyboard

	m_kbdc->data_write_from_mb(BIT(data, 1));
	m_kbdc->clock_write_from_mb(BIT(data, 0));
}

void tr175_state::kbd_clk_w(int state)
{
	// falling edge causes an interrupt
	if (m_kbd_clk && !state)
		m_maincpu->set_input_line(M68K_IRQ_IPL2, ASSERT_LINE);

	m_kbd_clk = state;
}

void tr175_state::kbd_rxd_w(int state)
{
	m_kbd_rxd = state;
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void tr175_state::charram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_chargen[offset]);
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 5);
}

void tr175_state::char_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(FUNC(tr175_state::avdc_char_r), FUNC(tr175_state::avdc_char_w));
}

void tr175_state::attr_map(address_map &map)
{
	map(0x0000, 0xbfff).rw(FUNC(tr175_state::avdc_attr_r), FUNC(tr175_state::avdc_attr_w));
}

uint8_t tr175_state::avdc_char_r(offs_t offset)
{
	const offs_t index = offset << 1;

	// row table: high byte of the second 16-bit word
	if (offset >= 0x2f80)
		return m_vram[index + 1] >> 8;

	// standard character data
	return m_vram[index] & 0xff;
}

void tr175_state::avdc_char_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = (m_vram[offset] & 0xff00) | data;
}

uint8_t tr175_state::avdc_attr_r(offs_t offset)
{
	return m_vram[offset] >> 8;
}

void tr175_state::avdc_attr_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = (m_vram[offset] & 0x00ff) | (data << 8);
}

void tr175_state::video_control_w(uint8_t data)
{
	// 7-------  display enable
	// -6------  unknown
	// --5-----  unknown
	// ---4----  132/80 column
	// ----3---  unknown, written on startup (avdc reset?)
	// -----2--  vga attributes: 1=blink, 0=bright bg
	// ------1-  vga attributes: 1=underline disabled
	// -------0  attribute format: 0=vga format

	logerror("video_control_w: %02x\n", data);

	m_video_control = data;
}

SCN2674_DRAW_CHARACTER_MEMBER( tr175_state::draw_character )
{
	const pen_t *const pen = m_ramdac->pens();

	for (unsigned i = 0; i < 2; i++)
	{
		const uint16_t cell = m_vram[(address << 1) + i];
		const uint8_t code = cell >> 0;
		const uint8_t attr = cell >> 8;

		uint16_t data = m_chargen[((code << 6 | linecount << 1) + 0x20) >> 1] << 1;

		// line-drawing characters
		if ((code & 0xc0) == 0xc0)
			data |= BIT(data, 1);

		if (cursor && ((BIT(m_duart_output, 6) == i) || dw))
			data = 0x1ff;

		rgb_t fg;
		rgb_t bg;

		if (BIT(m_video_control, 0))
		{
			// ASCII/ANSI mode
			if (ul && (BIT(attr, 2) && BIT(attr, 1)))
				data = 0x1ff;

			if (blink && BIT(attr, 7))
				data = 0x000;

			if ((BIT(attr, 6) && BIT(attr, 5) && BIT(attr, 4)))
				data = ~data;

			fg = pen[0x40 | (BIT(attr, 3) ? 0x00 : 0x08)];
			bg = pen[0x40 | (BIT(attr, 3) ? 0x07 : 0x0f)];
		}
		else
		{
			// VGA mode
			fg = pen[0x40 | ((~attr >> 0) & 0x0f)];
			bg = pen[0x40 | ((~attr >> 4) & 0x0f)];
		}

		for (int p = 0; p < 9; p++)
		{
			const rgb_t color = BIT(data, 8 - p) ? fg : bg;

			if (dw)
			{
				bitmap.pix(y, x + p * 2 + i) = color;
			}
			else
			{
				bitmap.pix(y, x + i * 9 + p) = color;
			}
		}
	}
}

static const gfx_layout tr175_charlayout_8x16 =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0, 1) },
	{ STEP16(0x21 * 8, 2 * 8) },
	8 * 64
};

static GFXDECODE_START( gfxdecode )
	GFXDECODE_RAM("chargen", 0, tr175_charlayout_8x16, 0x3f, 1) // ramdac programmed colors start at 0x40 (0x3f is black)
GFXDECODE_END


//**************************************************************************
//  AUDIO EMULATION
//**************************************************************************

void tr175_state::volume_w(uint8_t data)
{
	// values written: 0xff (mute), 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01
	m_speaker_sound->set_output_gain(0, (8 - std::popcount(data)) * 12.5);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void tr175_state::machine_start()
{
	// VRAM with all zeros appears as valid nvram checksum
	// overwrite it so that NVRAM is properly initialized
	m_vram[0xb6da / 2] = 0xffff;
	m_vram[0xb6dc / 2] = 0xffff;

	// register for save states
	save_item(NAME(m_kbd_clk));
	save_item(NAME(m_kbd_rxd));
	save_item(NAME(m_video_control));
	save_item(NAME(m_duart_output));
}

void tr175_state::duart_output_w(uint8_t data)
{
	// 7-------  aux port dtr
	// -6------  cursor location bit 0
	// --54----  video timing, selects xtal?
	// ----3---  speaker sound
	// -----2--  aux port rts
	// ------1-  main port dtr
	// -------0  main port rts

	m_duart_output = data;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void tr175_state::tr175(machine_config &config)
{
	M68000(config, m_maincpu, 12'000'000);
	m_maincpu->set_interrupt_mixer(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &tr175_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(28.322_MHz_XTAL, 900, 0, 720, 449, 0, 416); // guess
	screen.set_screen_update("avdc", FUNC(scn2674_device::screen_update));

	BT476(config, m_ramdac); // KDA0476BCN-66

	scn2674_device &avdc(SCN2674(config, "avdc", 28.322_MHz_XTAL / 18)); // guess
	avdc.intr_callback().set_inputline(m_maincpu, M68K_IRQ_IPL1);
	avdc.set_character_width(18); // guess
	avdc.set_display_callback(FUNC(tr175_state::draw_character));
	avdc.set_addrmap(0, &tr175_state::char_map);
	avdc.set_addrmap(1, &tr175_state::attr_map);
	avdc.set_screen("screen");

	GFXDECODE(config, m_gfxdecode, m_ramdac, gfxdecode);

	SCN2681(config, m_duart, 11.0592_MHz_XTAL / 3);
	m_duart->irq_cb().set_inputline(m_maincpu, M68K_IRQ_IPL0);
	m_duart->outport_cb().set(FUNC(tr175_state::duart_output_w));
	m_duart->outport_cb().append(m_mainport, FUNC(rs232_port_device::write_rts)).bit(0);
	m_duart->outport_cb().append(m_mainport, FUNC(rs232_port_device::write_dtr)).bit(1);
	m_duart->outport_cb().append(m_auxport, FUNC(rs232_port_device::write_rts)).bit(2);
	m_duart->outport_cb().append(m_speaker_sound, FUNC(speaker_sound_device::level_w)).bit(3);
	m_duart->outport_cb().append(m_auxport, FUNC(rs232_port_device::write_dtr)).bit(7);
	m_duart->a_tx_cb().set(m_mainport, FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_auxport, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_mainport, default_rs232_devices, nullptr);
	m_mainport->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_a_w));
	m_mainport->cts_handler().set(m_duart, FUNC(scn2681_device::ip0_w));
	m_mainport->dsr_handler().set(m_duart, FUNC(scn2681_device::ip1_w));

	RS232_PORT(config, m_auxport, default_rs232_devices, nullptr);
	m_auxport->rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
	m_auxport->cts_handler().set(m_duart, FUNC(scn2681_device::ip2_w));
	m_auxport->dsr_handler().set(m_duart, FUNC(scn2681_device::ip5_w));

	SPEAKER(config, "speaker").front_center();

	SPEAKER_SOUND(config, m_speaker_sound);
	m_speaker_sound->add_route(ALL_OUTPUTS, "speaker", 1.0);

	PC_KBDC(config, m_kbdc, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_kbdc->out_clock_cb().set(FUNC(tr175_state::kbd_clk_w));
	m_kbdc->out_data_cb().set(FUNC(tr175_state::kbd_rxd_w));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( tr175 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD16_BYTE( "v6.05.u50", 0x00001, 0x10000, CRC(5a33b6b3) SHA1(d673f50dd88f8a154ddaabe34cfcc9ab91435a4c) )
	ROM_LOAD16_BYTE( "v6.05.u45", 0x00000, 0x10000, CRC(e220befe) SHA1(8402280577e6de4b85843222bbd6b06a3f625b3b) )
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY    FULLNAME     FLAGS
COMP( 1994, tr175, 0,      0,      tr175,   tr175, tr175_state, empty_init, "Relisys", "TR-175 II", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
