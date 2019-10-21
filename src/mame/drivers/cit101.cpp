// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Preliminary driver for first-generation C. Itoh video terminals.

CIT-101 (released December 1980)
    C. Itoh's first terminal, based on DEC VT100. ANSI X3.64 and V52 compatible.
    12-inch monochrome screen displaying 24 lines of 80 or 132 characters.
    8 x 10 character cell, 10 x 10 (80 columns)/9 x 10 (132 columns) display cell.
    15,600 Hz horizontal frequency; 50 Hz/60 Hz vertical frequency selectable.
    Cursor may be selected as blinking or solid block/underline, or invisible.
    7 or 8 bit ASCII characters.
    RS232-C or 20 mA current loop communications and auxiliary (printer) ports.
    85-key detachable keyboard with 7 LEDs and settable key click.
CIT-80 (released September 1981)
    "Entry-level version" of CIT-101.
    12-inch monochrome screen displaying 24 lines of 80 characters.
    7-bit characters only.
CIT-161 (released 1982)
    Colorized version of the CIT-101.
    12-inch color screen displaying 24 lines of 80 or 132 characters.
    64 combinations of 8 colors are programmable.
CIT-500 (released 1982)
    Word processing terminal with full page display.
    15-inch vertically oriented monochrome screen with tilt/swivel.
    64 lines of 80 characters (interlaced).
    105-key keyboard.
CIT-101e (released 1983)
    Ergonomic redesign of CIT-101.
    Competitive with DEC VT220 (which was released several months later).
    14-inch monochrome screen with tilt/swivel, 24 lines of 80 or 132 characters.
    85-key low-profile keyboard.
CIG-201
    Plug-in graphics card for CIT-101 and CIT-101e.
    Compatible with Tektronix 4010/4014.
CIG-261
    Plug-in color graphics card for CIT-161.
    Compatible with Tektronix 4010/4014.
CIG-267
    Plug-in color graphics card for CIT-161.
    Compatible with Tektronix 4027A.

Special SET-UP control codes:
* CTRL+S: Save settings to NVR
* CTRL+R: Recall settings from NVR
* CTRL+D: Restore default NVR settings
* CTRL+A: Set answerback message
* CTRL+X: Enable/disable Bidirectional Auxiliary I/O Channel and SET-UP D Mode
          (undocumented; SET-UP B Mode only)

************************************************************************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/er2055.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "screen.h"

#include "machine/cit101_kbd.h"


class cit101_state : public driver_device
{
public:
	cit101_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_nvr(*this, "nvr")
		, m_comuart(*this, "comuart")
		, m_kbduart(*this, "kbduart")
		, m_chargen(*this, "chargen")
		, m_mainram(*this, "mainram")
		, m_extraram(*this, "extraram")
	{ }

	void cit101(machine_config &config);
protected:
	virtual void machine_start() override;
private:
	void draw_line(uint32_t *pixptr, int minx, int maxx, int line, bool last_line, u16 rowaddr, u16 rowattr, u8 scrattr);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(c000_ram_r);
	DECLARE_WRITE8_MEMBER(c000_ram_w);
	DECLARE_READ8_MEMBER(e0_latch_r);
	DECLARE_WRITE8_MEMBER(e0_latch_w);

	DECLARE_WRITE_LINE_MEMBER(blink_w);
	DECLARE_WRITE8_MEMBER(screen_control_w);
	DECLARE_WRITE8_MEMBER(brightness_w);

	DECLARE_WRITE8_MEMBER(nvr_address_w);
	DECLARE_READ8_MEMBER(nvr_data_r);
	DECLARE_WRITE8_MEMBER(nvr_data_w);
	DECLARE_WRITE8_MEMBER(nvr_control_w);

	void mem_map(address_map &map);
	void io_map(address_map &map);

	u8 m_e0_latch;

	bool m_blink;
	u8 m_brightness;

	required_device<screen_device> m_screen;
	required_device<er2055_device> m_nvr;
	required_device<i8251_device> m_comuart;
	required_device<i8251_device> m_kbduart;
	required_region_ptr<u8> m_chargen;
	required_shared_ptr<u8> m_mainram;
	required_shared_ptr<u8> m_extraram;
};


void cit101_state::machine_start()
{
	m_comuart->write_cts(0);
	m_kbduart->write_cts(0);

	m_brightness = 0xff;

	save_item(NAME(m_e0_latch));
	save_item(NAME(m_blink));
	save_item(NAME(m_brightness));
}


void cit101_state::draw_line(uint32_t *pixptr, int minx, int maxx, int line, bool last_line, u16 rowaddr, u16 rowattr, u8 scrattr)
{
	// Character attribute bit 0: underline (also used to render cursor)
	// Character attribute bit 1: reverse video (also used to render cursor)
	// Character attribute bit 2: boldface
	// Character attribute bit 3: blinking (half intensity)

	const int char_width = BIT(scrattr, 1) ? 10 : 9;
	int c = 0;
	u8 attr = m_extraram[rowaddr];
	u8 char_data = m_chargen[(m_mainram[rowaddr] << 4) | line];
	if (last_line && BIT(attr, 0))
		char_data ^= 0xff;
	rgb_t on_color = (BIT(attr, 1) != BIT(scrattr, 0)) ? rgb_t::black() : rgb_t(m_brightness, m_brightness, m_brightness);
	rgb_t off_color = (BIT(attr, 1) != BIT(scrattr, 0)) ? rgb_t(m_brightness, m_brightness, m_brightness) : rgb_t::black();
	if (BIT(attr, 3) && m_blink)
		on_color = rgb_t(m_brightness * 0.75, m_brightness * 0.75, m_brightness * 0.75);
	bool last_bit = false;
	for (int x = 0; x <= maxx; x++)
	{
		const bool cur_bit = BIT(char_data, 7);
		if (x >= minx)
			pixptr[x] = (cur_bit || (BIT(attr, 2) && last_bit)) ? on_color : off_color;
		last_bit = cur_bit;

		c++;
		if (!BIT(rowattr, 9) || !BIT(c, 0))
		{
			if (c < (BIT(rowattr, 9) ? char_width << 1 : char_width))
				char_data = (char_data << 1) | (char_data & 1);
			else
			{
				c = 0;
				rowaddr = (rowaddr + 1) & 0x3fff;
				attr = m_extraram[rowaddr];
				char_data = m_chargen[(m_mainram[rowaddr] << 4) | line];
				if (last_line && BIT(attr, 0))
					char_data ^= 0xff;
				on_color = BIT(attr, 1) ? rgb_t::black() : rgb_t(m_brightness, m_brightness, m_brightness);
				off_color = BIT(attr, 1) ? rgb_t(m_brightness, m_brightness, m_brightness) : rgb_t::black();
				if (BIT(attr, 3) && m_blink)
					on_color = rgb_t(m_brightness * 0.75, m_brightness * 0.75, m_brightness * 0.75);
				last_bit = false;
			}
		}
	}
}

u32 cit101_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// While screen height is fixed at 240 pixels, the number of character rows and the height of each row are not.
	// The "Set-Up" screens configure 3 ordinary 10-pixel rows on the top (using two for double-height characters)
	// and 2 more on the bottom, separated by 11 blank rows of 16 pixels and 1 blank row of 14 pixels. This is
	// also used to implement the "smooth scroll" option.

	const u8 scrattr = m_extraram[0];

	int y = 0;
	for (u16 rowptr = m_mainram[0]; y <= cliprect.bottom(); rowptr += 2)
	{
		const u16 rowattr = m_mainram[rowptr] | m_extraram[rowptr] << 8;
		const int rowlines = 16 - ((rowattr & 0x0f0) >> 4);

		int z = 0;
		if (y < cliprect.top())
		{
			z = cliprect.top() - y;
			if (z >= rowlines)
			{
				y += rowlines;
				continue;
			}
			y = cliprect.top();
		}

		const u16 rowaddr = m_mainram[rowptr + 1] | (m_extraram[rowptr + 1] & 0x3f) << 8;
		while (y <= cliprect.bottom())
		{
			const int line = ((z / (BIT(rowattr, 8) ? 2 : 1)) + (rowattr & 0x00f)) & 15;
			const bool last_line = z++ == rowlines - 1;
			draw_line(&bitmap.pix32(y++), cliprect.left(), cliprect.right(), line, last_line, rowaddr, rowattr, scrattr);
			if (last_line)
				break;
		}
	}
	return 0;
}


READ8_MEMBER(cit101_state::c000_ram_r)
{
	if (!machine().side_effects_disabled())
		m_e0_latch = m_extraram[offset];
	return m_mainram[offset];
}

WRITE8_MEMBER(cit101_state::c000_ram_w)
{
	m_extraram[offset] = m_e0_latch;
	m_mainram[offset] = data;
}

READ8_MEMBER(cit101_state::e0_latch_r)
{
	return m_e0_latch;
}

WRITE8_MEMBER(cit101_state::e0_latch_w)
{
	m_e0_latch = data;
}

WRITE_LINE_MEMBER(cit101_state::blink_w)
{
	m_blink = state;
}

WRITE8_MEMBER(cit101_state::screen_control_w)
{
	if ((m_extraram[0] & 0x06) != (data & 0x06))
	{
		const int height = BIT(data, 2) ? 312 : 260;
		const attoseconds_t frame_period = HZ_TO_ATTOSECONDS(BIT(data, 2) ? 50 : 60);
		if (BIT(data, 1))
		{
			const rectangle visarea(0, 799, 0, 239);
			m_screen->set_unscaled_clock(14.976_MHz_XTAL);
			m_screen->configure(960, height, visarea, frame_period);
		}
		else
		{
			const rectangle visarea(0, 1187, 0, 239);
			m_screen->set_unscaled_clock(22.464_MHz_XTAL);
			m_screen->configure(1440, height, visarea, frame_period);
		}
	}

	m_extraram[0] = data;
}

WRITE8_MEMBER(cit101_state::brightness_w)
{
	// Function of upper 3 bits is unknown
	m_brightness = pal5bit(~data & 0x1f);
}

WRITE8_MEMBER(cit101_state::nvr_address_w)
{
	m_nvr->set_address(data & 0x3f);
	m_nvr->set_clk(BIT(data, 6));
}

READ8_MEMBER(cit101_state::nvr_data_r)
{
	return m_nvr->data();
}

WRITE8_MEMBER(cit101_state::nvr_data_w)
{
	m_nvr->set_data(data);
}

WRITE8_MEMBER(cit101_state::nvr_control_w)
{
	m_nvr->set_control(BIT(data, 5), !BIT(data, 4), BIT(data, 7), BIT(data, 6));
}

void cit101_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x7fff).ram().share("mainram");
	map(0x8000, 0xbfff).ram().share("extraram"); // only 4 bits wide?
	map(0x8000, 0x8000).w(FUNC(cit101_state::screen_control_w));
	map(0xc000, 0xdfff).rw(FUNC(cit101_state::c000_ram_r), FUNC(cit101_state::c000_ram_w));
	map(0xfc00, 0xfc01).rw("auxuart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xfc20, 0xfc21).rw("comuart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xfc40, 0xfc41).rw("kbduart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xfc60, 0xfc63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xfc80, 0xfc83).w("pit0", FUNC(pit8253_device::write));
	map(0xfcc0, 0xfcc3).w("pit1", FUNC(pit8253_device::write));
}

void cit101_state::io_map(address_map &map)
{
	map(0x00, 0x01).rw("auxuart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x20, 0x21).rw("comuart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x40, 0x41).rw("kbduart", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x60, 0x63).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xa0, 0xa0).w(FUNC(cit101_state::brightness_w));
	map(0xe0, 0xe0).rw(FUNC(cit101_state::e0_latch_r), FUNC(cit101_state::e0_latch_w));
}


static INPUT_PORTS_START( cit101 )
INPUT_PORTS_END


void cit101_state::cit101(machine_config &config)
{
	i8085a_cpu_device &maincpu(I8085A(config, "maincpu", 6.144_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &cit101_state::mem_map);
	maincpu.set_addrmap(AS_IO, &cit101_state::io_map);
	maincpu.in_sid_func().set_constant(0); // used to time NVR reads
	maincpu.out_sod_func().set(FUNC(cit101_state::blink_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	//m_screen->set_raw(14.976_MHz_XTAL, 960, 0, 800, 260, 0, 240);
	m_screen->set_raw(22.464_MHz_XTAL, 1440, 0, 1188, 260, 0, 240);
	m_screen->set_screen_update(FUNC(cit101_state::screen_update));
	m_screen->screen_vblank().set_inputline("maincpu", I8085_RST75_LINE);

	I8251(config, m_comuart, 6.144_MHz_XTAL / 2);
	m_comuart->txd_handler().set("comm", FUNC(rs232_port_device::write_txd));
	m_comuart->dtr_handler().set("comm", FUNC(rs232_port_device::write_dtr));
	m_comuart->rts_handler().set("comm", FUNC(rs232_port_device::write_rts));
	m_comuart->rxrdy_handler().set("uartint", FUNC(input_merger_device::in_w<0>));
	m_comuart->txrdy_handler().set("uartint", FUNC(input_merger_device::in_w<2>));

	rs232_port_device &comm(RS232_PORT(config, "comm", default_rs232_devices, nullptr));
	comm.rxd_handler().set("comuart", FUNC(i8251_device::write_rxd));
	comm.dsr_handler().set("comuart", FUNC(i8251_device::write_dsr));
	// CTS can be disabled in SET-UP Mode C
	// DSR, CD, SI, RI are examined only during the modem test, not "always ignored" as the User's Manual claims

	i8251_device &auxuart(I8251(config, "auxuart", 6.144_MHz_XTAL / 2));
	auxuart.txd_handler().set("printer", FUNC(rs232_port_device::write_txd));
	auxuart.rxrdy_handler().set("uartint", FUNC(input_merger_device::in_w<1>));
	auxuart.txrdy_handler().set("uartint", FUNC(input_merger_device::in_w<3>));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set("auxuart", FUNC(i8251_device::write_rxd));
	printer.cts_handler().set("auxuart", FUNC(i8251_device::write_cts));

	INPUT_MERGER_ANY_HIGH(config, "uartint").output_handler().set_inputline("maincpu", I8085_RST55_LINE);

	I8251(config, m_kbduart, 6.144_MHz_XTAL / 2);
	m_kbduart->txd_handler().set("keyboard", FUNC(cit101_hle_keyboard_device::write_rxd));
	m_kbduart->rxrdy_handler().set_inputline("maincpu", I8085_RST65_LINE);

	CIT101_HLE_KEYBOARD(config, "keyboard").txd_callback().set("kbduart", FUNC(i8251_device::write_rxd));

	pit8253_device &pit0(PIT8253(config, "pit0", 0));
	pit0.set_clk<0>(6.144_MHz_XTAL / 4);
	pit0.set_clk<1>(6.144_MHz_XTAL / 4);
	//pit0.set_clk<2>(6.144_MHz_XTAL / 4);
	pit0.out_handler<0>().set("auxuart", FUNC(i8251_device::write_txc));
	pit0.out_handler<1>().set("auxuart", FUNC(i8251_device::write_rxc));
	// OUT2 might be used for an internal expansion similar to the VT100 STP.
	// The output appears to be fixed to a 307.2 kHz rate; turning this off boosts driver performance.

	pit8253_device &pit1(PIT8253(config, "pit1", 0));
	pit1.set_clk<0>(6.144_MHz_XTAL / 4);
	pit1.set_clk<1>(6.144_MHz_XTAL / 4);
	pit1.set_clk<2>(6.144_MHz_XTAL / 4);
	pit1.out_handler<0>().set("comuart", FUNC(i8251_device::write_txc));
	pit1.out_handler<1>().set("comuart", FUNC(i8251_device::write_rxc));
	pit1.out_handler<2>().set("kbduart", FUNC(i8251_device::write_txc));
	pit1.out_handler<2>().append("kbduart", FUNC(i8251_device::write_rxc));

	i8255_device &ppi(I8255A(config, "ppi", 0));
	ppi.out_pa_callback().set(FUNC(cit101_state::nvr_address_w));
	ppi.in_pb_callback().set(FUNC(cit101_state::nvr_data_r));
	ppi.out_pb_callback().set(FUNC(cit101_state::nvr_data_w));
	ppi.in_pc_callback().set("comm", FUNC(rs232_port_device::cts_r)).lshift(0);
	ppi.in_pc_callback().append("comm", FUNC(rs232_port_device::dcd_r)).lshift(1); // tied to DSR for loopback test
	ppi.in_pc_callback().append("comm", FUNC(rs232_port_device::ri_r)).lshift(2); // tied to CTS for loopback test
	ppi.in_pc_callback().append("comm", FUNC(rs232_port_device::si_r)).lshift(3); // tied to CTS for loopback test
	ppi.out_pc_callback().set(FUNC(cit101_state::nvr_control_w));

	ER2055(config, m_nvr);
}


// PCB ID: HAV-2P005B / CIT-101 / C. ITOH
// CPU: NEC D8085AC
// RAM: 12x NEC D416C-2 (16 positions labeled 8116E, including 4 unpopulated ones)
// Peripherals: 3x M5L8251AP-5 (2M, 7J, 7K); 2x NEC D8253C-2 (7I, 7L); NEC D8255AC-2 (7N); GI ER-2055 (8P)
// Oscillators: 6.144 (XTAL1), 14.976 (XTAL2), 22.464 (XTAL3)
ROM_START( cit101 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD( "ic1_=3g04=_v10a.bin", 0x0000, 0x1000, CRC(5601fcac) SHA1(cad0d0335d133dd43993bc718ff72d12b8445cd1) ) // TMM2732D-45
	ROM_LOAD( "ic2_=3h04=_v10a.bin", 0x1000, 0x1000, CRC(23d263e0) SHA1(586e8185f9804987e0a4081724c060e74769d41d) ) // TMM2732D-45
	ROM_LOAD( "ic3_=3i04=_v10a.bin", 0x2000, 0x1000, CRC(15994b1d) SHA1(6d125db4ef5e1dd4d5a4d2f4d6f6bdf574e5bad8) ) // TMM2732D-45
	ROM_LOAD( "ic4_=3j04=_v10a.bin", 0x3000, 0x0800, CRC(d786995f) SHA1(943b521dcc7abc0662d6e136169b7db480ae1e5c) ) // MB8516
	ROM_RELOAD(0x3800, 0x0800)

	ROM_REGION(0x1000, "chargen", 0)
	ROM_LOAD( "1h =5h 1 02= char rom.bin", 0x0000, 0x1000, CRC(ee0ff889) SHA1(a74ada19d19041b29e1b49aaf57ba7d9d54575e1) ) // TMM2332P

	ROM_REGION(0x180, "proms", 0)
	ROM_LOAD( "2i_=3a00=.bin", 0x000, 0x100, NO_DUMP ) // position labeled (MB)7052
	ROM_LOAD( "2f_=6g00=.bin", 0x100, 0x020, NO_DUMP ) // position labeled TBP18S030
	ROM_LOAD( "2e_=7i00=.bin", 0x120, 0x020, NO_DUMP ) // position labeled TBP18S030
	ROM_LOAD( "5d_=4l00=.bin", 0x140, 0x020, NO_DUMP ) // position labeled TBP18S030
	ROM_LOAD( "5g_=7f00=.bin", 0x160, 0x020, NO_DUMP ) // position labeled TBP18S030
ROM_END

COMP( 1980, cit101, 0, 0, cit101, cit101, cit101_state, empty_init, "C. Itoh Electronics", "CIT-101", MACHINE_NOT_WORKING )
