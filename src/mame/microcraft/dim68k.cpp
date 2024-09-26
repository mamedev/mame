// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

        Micro Craft Dimension 68000

        This computer had the facility for plug-in cards to emulate popular
        computers of the time such as the Apple II Plus, TRS80, Kaypro, etc.
        The floppy disk parameters could be set to be able to read the disks
        of the emulated systems, or you create any other format you wished.

        TODO:
        - Graphics display (including colour and video_control options)
        - RTC (is this a thing?  the manual indicates it just uses the DUART's timers to track time)
        - Centronics printer
        - Video-high
        - Video-reset
        - The plug-in boards
        - Emulator trap function

        DUART I/O port bits:
        INPUT:
        bit 0 = Centronics RDY
        bit 4 = RS-232C DCD

        OUTPUT:
        all bits = Centronics data

        Colors (from COLORDEM.BAS)
        0 = black
        1 = dark blue
        2 = red
        3 = magenta
        4 = brown
        5 = grey
        6 = orange
        7 = pink
        8 = dark aqua
        9 = blue
        A = grey 2
        B = light blue
        C = green
        D = aqua
        E = yellow
        F = white

****************************************************************************/

#include "emu.h"
#include "dim68k_kbd.h"
#include "cpu/m68000/m68000.h"
#include "imagedev/floppy.h"
#include "machine/mc68681.h"
#include "machine/upd765.h"
#include "sound/spkrdev.h"
#include "video/mc6845.h"
#include "bus/pc_joy/pc_joy.h"
#include "bus/rs232/rs232.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class dim68k_state : public driver_device
{
public:
	dim68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_speaker(*this, "speaker")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_gamectrl(*this, "gamectrl")
		, m_ram(*this, "ram")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
		, m_duart(*this, "duart")
		, m_bootview(*this, "bootview")
	{ }

	void dim68k(machine_config &config);

private:
	void dim68k_palette(palette_device &palette);
	u16 dim68k_fdc_r();
	u8 dim68k_game_switches_r(offs_t offset);
	u16 dim68k_speaker_r();
	u16 dim68k_banksw_r();
	void dim68k_banksw_w(u16 data);
	void dim68k_fdc_w(u16 data);
	void dim68k_printer_strobe_w(u16 data);
	void dim68k_speaker_w(u16 data);
	void dim68k_video_control_w(u16 data);
	void dim68k_video_high_w(u16 data);
	void dim68k_video_reset_w(u16 data);
	MC6845_UPDATE_ROW(crtc_update_row);

	void fdc_irq_w(int state);

	void mem_map(address_map &map) ATTR_COLD;

	bool m_speaker_bit = 0;
	u8 m_video_control = 0U;
	u8 m_fdc_irq = 0U;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<speaker_sound_device> m_speaker;
	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<pc_joy_device> m_gamectrl;
	required_shared_ptr<uint16_t> m_ram;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;
	required_device<scn2681_device> m_duart;
	memory_view m_bootview;
};

void dim68k_state::dim68k_palette(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t::black());
	palette.set_pen_color(1, rgb_t(0x40, 0x1c, 0xf7)); /* Dark Blue */
	palette.set_pen_color(2, rgb_t(0xa7, 0x0b, 0x40)); /* Dark Red */
	palette.set_pen_color(3, rgb_t(0xe6, 0x28, 0xff)); /* Purple */
	palette.set_pen_color(4, rgb_t(0x40, 0x63, 0x00)); /* Brown */
	palette.set_pen_color(5, rgb_t(0x80, 0x80, 0x80)); /* Dark Gray */
	palette.set_pen_color(6, rgb_t(0xe6, 0x6f, 0x00)); /* Orange */
	palette.set_pen_color(7, rgb_t(0xff, 0x8b, 0xbf)); /* Pink */
	palette.set_pen_color(8, rgb_t(0x00, 0x74, 0x40)); /* Dark Green */
	palette.set_pen_color(9, rgb_t(0x19, 0x90, 0xff)); /* Medium Blue */
	palette.set_pen_color(10, rgb_t(0x80, 0x80, 0x80)); /* Light Grey */
	palette.set_pen_color(11, rgb_t(0xbf, 0x9c, 0xff)); /* Light Blue */
	palette.set_pen_color(12, rgb_t(0x19, 0xd7, 0x00)); /* Light Green */
	palette.set_pen_color(13, rgb_t(0x58, 0xf4, 0xbf)); /* Aquamarine */
	palette.set_pen_color(14, rgb_t(0xbf, 0xe3, 0x08)); /* Yellow */
	palette.set_pen_color(15, rgb_t(0xff, 0xff, 0xff));  /* White */
};

u16 dim68k_state::dim68k_fdc_r()
{
	return 0;
}

u8 dim68k_state::dim68k_game_switches_r(offs_t offset)
// Reading the game port switches
// FFCC11 = switch 0; FFCC13 = switch 1, etc to switch 3
// FFCC19 = paddle 0; FFCC1B = paddle 1, etc to paddle 3
{
	return BIT(m_gamectrl->joy_port_r(), offset ^ 4) ? 0 : 0x80;
}

u16 dim68k_state::dim68k_speaker_r()
// Any read or write of this address will toggle the position of the speaker cone
{
	if (!machine().side_effects_disabled())
	{
		m_speaker_bit ^= 1;
		m_speaker->level_w(m_speaker_bit);
	}
	return 0;
}

void dim68k_state::dim68k_speaker_w(u16 data)
{
	m_speaker_bit ^= 1;
	m_speaker->level_w(m_speaker_bit);
}

void dim68k_state::fdc_irq_w(int state)
{
	if (state)
	{
		m_fdc_irq = 0;
	}
	else
	{
		m_fdc_irq = 0x80;
	}
}

void dim68k_state::dim68k_fdc_w(u16 data)
{
	m_fdc->tc_w(BIT(data, 5));

	if (BIT(data, 1))
	{
		m_floppy[BIT(data, 0)^1]->get_device()->mon_w(true);
	}
	else
	{
		m_floppy[BIT(data, 0)^1]->get_device()->mon_w(false);
	}
}

void dim68k_state::dim68k_video_high_w(u16 data)
// "write high byte of address in memory of start of display buffer"
{
}

void dim68k_state::dim68k_video_control_w(u16 data)
{
/* D7 0 = Hires/Graphics; 1= Lores/Text [not emulated yet]
   D6 0 = 8 dots per character; 1 = 7 dots [emulated]
   D5 0 = CRTC and CPU run Asynchronously; 1 = Synchronously [won't be emulated]
   D4,D3  Dot clock: 00=14MHz; 01=3.58MHz; 10=7MHz; 11=1.79MHz [emulated]
   D2 0 = Screen On; 1 = Off [emulated]
   D1 0 = Standard Chars & LoRes; 1 = Alternate Chars & HiRes [not emulated yet]
   D0 0 = Non-Mixed (all text or all Graphics); 1 = Mixed (Colour Graphics and Monochrome Text) [not emulated yet]
*/
	unsigned dots = (data & 0x40) ? 7 : 8;
	m_crtc->set_hpixels_per_column(dots);
	m_video_control = data;

	switch (data & 0x18)
	{
		case 0x00: m_crtc->set_unscaled_clock(XTAL(14'000'000) / dots);
					break;
		case 0x08: m_crtc->set_unscaled_clock(XTAL(3'579'545) / dots);
					break;
		case 0x10: m_crtc->set_unscaled_clock(XTAL(14'000'000) / dots / 2);
					break;
		case 0x18: m_crtc->set_unscaled_clock(XTAL(3'579'545) / dots / 2);
					break;
	}
}

void dim68k_state::dim68k_video_reset_w(u16 data)
{
}

void dim68k_state::dim68k_printer_strobe_w(u16 data)
// anything sent here will trigger a one-shot for a strobe pulse
{
}

u16 dim68k_state::dim68k_banksw_r()
{
	m_bootview.disable();
	return 0xffff;
}

void dim68k_state::dim68k_banksw_w(u16 data)
{
	m_bootview.disable();
}

void dim68k_state::mem_map(address_map &map)
{
	map(0x000000, 0x7fffff).ram().share("ram"); // 8 MiB of RAM
	map(0x000000, 0x00ffff).view(m_bootview);
	m_bootview[0](0x000000, 0x001fff).rom().region("bootrom", 0);
	// graphics VRAM is not used by the stub before the bankswitch occurs, so it's not mapped here
	m_bootview[0](0x008001, 0x008001).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	m_bootview[0](0x008003, 0x008003).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	m_bootview[0](0x008004, 0x008005).w(FUNC(dim68k_state::dim68k_video_high_w));
	m_bootview[0](0x008008, 0x008009).w(FUNC(dim68k_state::dim68k_video_control_w));
	m_bootview[0](0x00800a, 0x00800b).w(FUNC(dim68k_state::dim68k_video_reset_w));
	m_bootview[0](0x00dc00, 0x00dc01).rw(FUNC(dim68k_state::dim68k_banksw_r), FUNC(dim68k_state::dim68k_banksw_w));

	map(0xff0000, 0xff1fff).rom().region("bootrom", 0);
	map(0xff2000, 0xff7fff).ram(); // Graphics Video RAM
	map(0xff8001, 0xff8001).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0xff8003, 0xff8003).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0xff8004, 0xff8005).w(FUNC(dim68k_state::dim68k_video_high_w));
	map(0xff8008, 0xff8009).w(FUNC(dim68k_state::dim68k_video_control_w));
	map(0xff800a, 0xff800b).w(FUNC(dim68k_state::dim68k_video_reset_w));
	map(0xff8800, 0xff8fff).rom().region("cop6512", 0); // slot 1 controller rom
	map(0xff9000, 0xff97ff).rom().region("copz80", 0); // slot 2 controller rom
	map(0xff9800, 0xff9fff).rom().region("cop8086", 0); // slot 3 controller rom
#if 0
	map(0xffa000, 0xffa7ff).rom(); // slot 4 controller rom
	map(0xffa800, 0xffafff).rom(); // slot 5 controller rom
	map(0xffb000, 0xffb7ff).rom(); // slot 6 controller rom
#endif
	map(0xffc400, 0xffc41f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask16(0x00ff);
	map(0xffc800, 0xffc801).rw(FUNC(dim68k_state::dim68k_speaker_r), FUNC(dim68k_state::dim68k_speaker_w));
	map(0xffcc00, 0xffcc00).w(m_gamectrl, FUNC(pc_joy_device::joy_port_w));
	map(0xffcc10, 0xffcc1f).r(FUNC(dim68k_state::dim68k_game_switches_r)).umask16(0x00ff);
	map(0xffd000, 0xffd003).m(m_fdc, FUNC(upd765a_device::map)).umask16(0x00ff); // NEC uPD765A
	map(0xffd000, 0xffd000).lr8([this]() -> u8 { return m_fdc_irq; }, "free0");
	map(0xffd004, 0xffd005).rw(FUNC(dim68k_state::dim68k_fdc_r), FUNC(dim68k_state::dim68k_fdc_w));
	//map(0x00ffd400, 0x00ffd403) emulation trap control
	map(0xffd800, 0xffd801).w(FUNC(dim68k_state::dim68k_printer_strobe_w));
	map(0xffdc00, 0xffdc01).rw(FUNC(dim68k_state::dim68k_banksw_r), FUNC(dim68k_state::dim68k_banksw_w));
}

/* Input ports */
static INPUT_PORTS_START( dim68k )
INPUT_PORTS_END


void dim68k_state::machine_reset()
{
	m_bootview.select(0);
	m_fdc_irq = 0;
}

// Text-only; graphics isn't emulated yet. Need to find out if hardware cursor is used.
MC6845_UPDATE_ROW( dim68k_state::crtc_update_row )
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint16_t chr16=0x2020; // set to spaces if screen is off
	uint32_t *p = &bitmap.pix(y);
	u8 screen_on = ~m_video_control & 4;
	u8 dot8 = ~m_video_control & 40;

	// need to divide everything in half to cater for 16-bit reads
	x_count /= 2;
	ma /= 2;
	u8 xx = 0;

	for (u8 x = 0; x < x_count; x++)
	{
		u8 inv, chr, gfx;
		if (screen_on)
			chr16 = m_ram[ma+x]; // reads 2 characters

		inv=0;
		if (xx == cursor_x && screen_on) inv=0xff;
		xx++;

		chr = chr16>>8;
		if (m_video_control & 0x80)
		{
			gfx = m_p_chargen[(chr<<4) | ra] ^ inv ^ ((chr & 0x80) ? 0xff : 0);
			*p++ = palette[BIT(gfx, 7)*15];
			*p++ = palette[BIT(gfx, 6)*15];
			*p++ = palette[BIT(gfx, 5)*15];
			*p++ = palette[BIT(gfx, 4)*15];
			*p++ = palette[BIT(gfx, 3)*15];
			*p++ = palette[BIT(gfx, 2)*15];
			*p++ = palette[BIT(gfx, 1)*15];
			if (dot8) *p++ = palette[BIT(gfx, 0)*15];
		}
		else
		{
			*p++ = palette[(chr>>4) & 0xf];
			*p++ = palette[(chr>>4) & 0xf];
			*p++ = palette[(chr>>4) & 0xf];
			*p++ = palette[(chr>>4) & 0xf];
			*p++ = palette[chr & 0xf];
			*p++ = palette[chr & 0xf];
			*p++ = palette[chr & 0xf];
			if (dot8) *p++ = palette[chr & 0xf];
		}

		inv = 0;
		if (xx == cursor_x) inv=0xff;
		xx++;

		chr = chr16;
		if (m_video_control & 0x80)
		{
			gfx = m_p_chargen[(chr << 4) | ra] ^ inv ^ ((chr & 0x80) ? 0xff : 0);
			*p++ = palette[BIT(gfx, 7)*15];
			*p++ = palette[BIT(gfx, 6)*15];
			*p++ = palette[BIT(gfx, 5)*15];
			*p++ = palette[BIT(gfx, 4)*15];
			*p++ = palette[BIT(gfx, 3)*15];
			*p++ = palette[BIT(gfx, 2)*15];
			*p++ = palette[BIT(gfx, 1)*15];
			if (dot8) *p++ = palette[BIT(gfx, 0)*15];
		}
		else
		{
			*p++ = palette[(chr >> 4) & 0xf];
			*p++ = palette[(chr >> 4) & 0xf];
			*p++ = palette[(chr >> 4) & 0xf];
			*p++ = palette[(chr >> 4) & 0xf];
			*p++ = palette[chr & 0xf];
			*p++ = palette[chr & 0xf];
			*p++ = palette[chr & 0xf];
			if (dot8) *p++ = palette[chr & 0xf];
		}
	}
}

/* F4 Character Displayer */
static const gfx_layout dim68k_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_dim68k )
	GFXDECODE_ENTRY( "chargen", 0x0000, dim68k_charlayout, 0, 1 )
GFXDECODE_END

static void dim68k_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void dim68k_state::machine_start()
{
	save_item(NAME(m_speaker_bit));
	save_item(NAME(m_video_control));
}

void dim68k_state::dim68k(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 14.318181_MHz_XTAL / 2); // part rated for 8 MHz; clock verified from manual
	m_maincpu->set_addrmap(AS_PROGRAM, &dim68k_state::mem_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.318181_MHz_XTAL, 896, 0, 640, 279, 0, 224);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	PALETTE(config, m_palette, FUNC(dim68k_state::dim68k_palette), 16);
	GFXDECODE(config, "gfxdecode", m_palette, gfx_dim68k);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* Devices */
	UPD765A(config, m_fdc, 16_MHz_XTAL / 4, true, true); // clocked through FDC9229BT; options unknown
	FLOPPY_CONNECTOR(config, m_floppy[0], dim68k_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], dim68k_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	m_fdc->intrq_wr_callback().set(FUNC(dim68k_state::fdc_irq_w));

	MC6845(config, m_crtc, 14.318181_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(dim68k_state::crtc_update_row));

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);

	DIM68K_KEYBOARD(config, "keyboard").txd_callback().set(m_duart, FUNC(scn2681_device::rx_a_w));

	PC_JOY(config, m_gamectrl);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_duart, FUNC(scn2681_device::rx_b_w));
	rs232.dcd_handler().set(m_duart, FUNC(scn2681_device::ip4_w));

	m_duart->b_tx_cb().set(rs232, FUNC(rs232_port_device::write_txd));

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("dim68k");
}

/*
68000

MC101A  82S100
MC102B  82S100
MC103E  2732A
MC104   2732A   label "4050" underneath
MC105   2732A   char gen

6512

MC106   82LS135 U24
MC107   82LS135 U20
MC108   82S137  U23
MC109   82S131  U16
MC110   82LS135 U35

Z80

MC111   82S123  U11

8086

MC112   82LS135 U18
MC113   82S153  U16
*/
/* ROM definition */
ROM_START( dim68k )
	ROM_REGION16_BE( 0x2000, "bootrom", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mc103e.bin", 0x0001, 0x1000, CRC(4730c902) SHA1(5c4bb79ad22def721a22eb63dd05e0391c8082be))
	ROM_LOAD16_BYTE( "mc104.bin",  0x0000, 0x1000, CRC(14b04575) SHA1(43e15d9ebe1c9c1bf1bcfc1be3899a49e6748200))

	ROM_REGION( 0x1000, "chargen", ROMREGION_ERASEFF )
	ROM_LOAD( "mc105e.bin", 0x0000, 0x1000, CRC(7a09daa8) SHA1(844bfa579293d7c3442fcbfa21bda75fff930394))

	// The remaining roms may not be in the correct positions or being loaded correctly
	ROM_REGION16_BE( 0x1000, "cop6512", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD( "mc106.bin", 0x0000, 0x0100, CRC(11530d8a) SHA1(e3eae266535383bcaee2d84d7bed6052d40e4e4a))
	ROM_LOAD16_WORD( "mc107.bin", 0x0100, 0x0100, CRC(966db11b) SHA1(3c3105ac842602d8e01b0f924152fd672a85f00c))
	ROM_LOAD16_WORD( "mc108.bin", 0x0200, 0x0400, CRC(687f9b0a) SHA1(ed9f1265b25f89f6d3cf8cd0a7b0fb73cb129f9f))
	ROM_LOAD16_WORD( "mc109.bin", 0x0600, 0x0200, CRC(4a857f98) SHA1(9f2bbc2171fc49f65aa798c9cd7799a26afd2ddf))
	ROM_LOAD16_WORD( "mc110.bin", 0x0800, 0x0100, CRC(e207b457) SHA1(a8987ba3d1bbdb3d8b3b11cec90c532ff09e762e))

	ROM_REGION16_BE( 0x1000, "copz80", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD( "mc111.bin", 0x0000, 0x0020, CRC(6a380057) SHA1(6522a7b3e0af9db14a6ed04d4eec3ee6e44c2dab))

	ROM_REGION16_BE( 0x1000, "cop8086", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD( "mc112.bin", 0x0000, 0x0100, CRC(dfd4cdbb) SHA1(a7831d415943fa86c417066807038bccbabb2573))
	ROM_LOAD( "mc113.bin", 0x0100, 0x00ef, CRC(594bdf05) SHA1(36db911a27d930e023fa12683e86e9eecfffdba6))

	ROM_REGION( 0x1000, "mb", ROMREGION_ERASEFF )   // mainboard unknown
	ROM_LOAD( "mc102.bin", 0x0000, 0x00fa, CRC(38e2abac) SHA1(0d7e730b46fc162764c69c51dea3bbe8337b1a7d))
	ROM_LOAD( "mc101.bin", 0x0100, 0x00fa, CRC(caffb3a0) SHA1(36f5140b306565794c4d856e0c20589b8f2a37f4))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY        FULLNAME           FLAGS
COMP( 1984, dim68k, 0,      0,      dim68k,  dim68k, dim68k_state, empty_init, "Micro Craft", "Dimension 68000", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
