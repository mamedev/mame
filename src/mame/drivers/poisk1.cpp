// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    drivers/poisk1.c

    Driver file for Poisk-1

    to do:
    - cassette i/o and softlist
    - verify palette
    - monochrome output
    - trap: does memory always get written or it's up to NMI ISR to complete writes?
    - keyboard layout for earliest revision (v89r0)

    slot devices:
    - hard disk controllers
    - network cards
    - joystick, mouse, serial, parallel ports
    - sound card

***************************************************************************/

#include "emu.h"
#include "machine/kb_poisk1.h"

#include "bus/isa/isa.h"
#include "bus/isa/xsu_cards.h"
#include "cpu/i86/i86.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "video/cgapal.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_KEYBOARD  (1U <<  1)
#define LOG_DEBUG     (1U <<  2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGKBD(...) LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


#define CGA_PALETTE_SETS 83
/* one for colour, one for mono, 81 for colour composite */

#define BG_COLOR(x) (((x)&7) | (((x)&0x10) >> 1))

#define POISK1_UPDATE_ROW(name) \
	void name(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t *videoram, uint16_t ma, uint8_t ra, uint8_t stride)


class p1_state : public driver_device
{
public:
	p1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic8259(*this, "pic8259")
		, m_pit8253(*this, "pit8253")
		, m_ppi8255n1(*this, "ppi8255n1")
		, m_ppi8255n2(*this, "ppi8255n2")
		, m_isabus(*this, "isa")
		, m_speaker(*this, "speaker")
		, m_cassette(*this, "cassette")
		, m_ram(*this, RAM_TAG)
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_kbdio(*this, "Y%u", 1)
	{ }

	void poisk1(machine_config &config);

	void init_poisk1();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic8259;
	required_device<pit8253_device> m_pit8253;
	required_device<i8255_device> m_ppi8255n1;
	required_device<i8255_device> m_ppi8255n2;
	required_device<isa8_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_ioport_array<8> m_kbdio;

	uint8_t m_p1_spkrdata;
	uint8_t m_p1_input;

	uint8_t m_kbpoll_mask;

	struct
	{
		uint8_t trap[4];
		std::unique_ptr<uint8_t[]> videoram_base;
		uint8_t *videoram;
		uint8_t mode_control_6a;
		uint8_t color_select_68;
		uint8_t palette_lut_2bpp[4];
		int stride;
		void *update_row(bitmap_rgb32 &bitmap, const rectangle &cliprect, uint8_t *videoram, uint16_t ma, uint8_t ra, uint8_t stride);
	} m_video;

	void p1_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void set_palette_luts();
	POISK1_UPDATE_ROW(cga_gfx_2bpp_update_row);
	POISK1_UPDATE_ROW(cga_gfx_1bpp_update_row);
	POISK1_UPDATE_ROW(poisk1_gfx_1bpp_update_row);

	DECLARE_WRITE_LINE_MEMBER(p1_pit8253_out2_changed);
	DECLARE_WRITE_LINE_MEMBER(p1_speaker_set_spkrdata);
	DECLARE_READ8_MEMBER(p1_trap_r);
	DECLARE_WRITE8_MEMBER(p1_trap_w);
	DECLARE_READ8_MEMBER(p1_cga_r);
	DECLARE_WRITE8_MEMBER(p1_cga_w);
	DECLARE_WRITE8_MEMBER(p1_vram_w);

	DECLARE_READ8_MEMBER(p1_ppi_r);
	DECLARE_WRITE8_MEMBER(p1_ppi_w);
	DECLARE_WRITE8_MEMBER(p1_ppi_porta_w);
	DECLARE_READ8_MEMBER(p1_ppi_porta_r);
	DECLARE_READ8_MEMBER(p1_ppi_portb_r);
	DECLARE_READ8_MEMBER(p1_ppi_portc_r);
	DECLARE_WRITE8_MEMBER(p1_ppi_portc_w);
	DECLARE_WRITE8_MEMBER(p1_ppi2_porta_w);
	DECLARE_WRITE8_MEMBER(p1_ppi2_portb_w);
	DECLARE_READ8_MEMBER(p1_ppi2_portc_r);

	void poisk1_io(address_map &map);
	void poisk1_map(address_map &map);
};

/*
 * onboard devices:
 */

/*
 * Poisk-1 doesn't have a mc6845 and always runs in graphics mode.  Text mode is emulated by BIOS;
 * NMI is triggered on access to video memory and to mc6845 ports.  Address and data are latched into:
 *
 * Port 28H (offset 0) -- lower 8 bits of address
 * Port 29H (offset 1) -- high  -//- and mode bits
 * Port 2AH (offset 2) -- data
 */

READ8_MEMBER(p1_state::p1_trap_r)
{
	uint8_t data = m_video.trap[offset];
	LOG("trap R %.2x $%02x\n", 0x28 + offset, data);
	if (offset == 0) m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return data;
}

WRITE8_MEMBER(p1_state::p1_trap_w)
{
	LOG("trap W %.2x $%02x\n", 0x28 + offset, data);
}

READ8_MEMBER(p1_state::p1_cga_r)
{
	uint16_t port = offset + 0x3d0;

	LOG("cga R %.4x\n", port);
	m_video.trap[1] = 0x40 | ((port >> 8) & 0x3f);
	m_video.trap[0] = port & 255;
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	return 0;
}

WRITE8_MEMBER(p1_state::p1_cga_w)
{
	uint16_t port = offset + 0x3d0;

	LOG("cga W %.4x $%02x\n", port, data);
	m_video.trap[2] = data;
	m_video.trap[1] = 0xC0 | ((port >> 8) & 0x3f);
	m_video.trap[0] = port & 255;
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE8_MEMBER(p1_state::p1_vram_w)
{
	LOG("vram W %.4x $%02x\n", offset, data);
	if (m_video.videoram_base) m_video.videoram_base[offset] = data;
	m_video.trap[2] = data;
	m_video.trap[1] = 0x80 | ((offset >> 8) & 0x3f);
	m_video.trap[0] = offset & 255;
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

// CGA emulator
/*
068h    D42 0..2    R, G, B     XXX Foreground/Background color
        3   NMI DISABLE NMI trap  1: Disabled  0: Enabled
        4   PALETTE     XXX Colour palette  0: XXX  1: XXX
        5   I (INTENS)  XXX Foreground/Background color intensity
        6   DISPLAY BANK    XXX Video RAM page
        7   HIRES       1: 640x200  0: 320x200
*/

WRITE8_MEMBER(p1_state::p1_ppi2_porta_w)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	LOG("color_select_68 W $%02x\n", data);

	// NMI DISABLE
	if (BIT((data ^ m_video.color_select_68), 3))
	{
		program.unmap_readwrite(0xb8000, 0xbbfff, 0);
		if (BIT(data, 3))
		{
			program.install_readwrite_bank(0xb8000, 0xbbfff, "bank11");
		}
		else
		{
			program.install_read_bank(0xb8000, 0xbbfff, "bank11");
			program.install_write_handler(0xb8000, 0xbbfff, WRITE8_DELEGATE(p1_state, p1_vram_w));
		}
	}
	// DISPLAY BANK
	if (BIT((data ^ m_video.color_select_68), 6))
	{
		if (BIT(data, 6))
			m_video.videoram = m_video.videoram_base.get() + 0x4000;
		else
			m_video.videoram = m_video.videoram_base.get();
	}
	// HIRES -- XXX
	if (BIT((data ^ m_video.color_select_68), 7))
	{
		if (BIT(data, 7))
			m_screen->set_visible_area(0, 640 - 1, 0, 200 - 1);
		else
			m_screen->set_visible_area(0, 320 - 1, 0, 200 - 1);
	}
	m_video.color_select_68 = data;
	set_palette_luts();
}

/*
06Ah    Dxx 6   Enable/Disable color burst (?)
        7   Enable/Disable D7H/D7L
*/

WRITE8_MEMBER(p1_state::p1_ppi_portc_w)
{
	LOG("mode_control_6a W $%02x\n", data);

	m_video.mode_control_6a = data;
	set_palette_luts();
}

void p1_state::set_palette_luts(void)
{
	/* Setup 2bpp palette lookup table */
	// HIRES
	if (m_video.color_select_68 & 0x80)
	{
		m_video.palette_lut_2bpp[0] = 0;
	}
	else
	{
		m_video.palette_lut_2bpp[0] = BG_COLOR(m_video.color_select_68);
	}
	// B&W -- XXX
/*
    if ( m_video.mode_control_6a & 0x40 )
    {
        m_video.palette_lut_2bpp[1] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 3;
        m_video.palette_lut_2bpp[2] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 4;
        m_video.palette_lut_2bpp[3] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 7;
    }
    else
*/
	{
		// PALETTE
		if ( m_video.color_select_68 & 0x20 )
		{
			m_video.palette_lut_2bpp[1] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 3;
			m_video.palette_lut_2bpp[2] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 5;
			m_video.palette_lut_2bpp[3] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 7;
		}
		else
		{
			m_video.palette_lut_2bpp[1] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 2;
			m_video.palette_lut_2bpp[2] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 4;
			m_video.palette_lut_2bpp[3] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 6;
		}
	}
}

/***************************************************************************
  Draw graphics mode with 320x200 pixels (default) with 2 bits/pixel.
  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
  cga fetches 2 byte per mc6845 access.
***************************************************************************/

POISK1_UPDATE_ROW(p1_state::cga_gfx_2bpp_update_row)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix32(ra);
	uint16_t odd, offset;
	int i;

	if (ra == 0) LOG("cga_gfx_2bpp_update_row\n");
	odd = (ra & 1) << 13;
	offset = (ma & 0x1fff) | odd;
	for (i = 0; i < stride; i++)
	{
		uint8_t data = videoram[ offset++ ];

		*p = palette[m_video.palette_lut_2bpp[ ( data >> 6 ) & 0x03 ]]; p++;
		*p = palette[m_video.palette_lut_2bpp[ ( data >> 4 ) & 0x03 ]]; p++;
		*p = palette[m_video.palette_lut_2bpp[ ( data >> 2 ) & 0x03 ]]; p++;
		*p = palette[m_video.palette_lut_2bpp[   data        & 0x03 ]]; p++;
	}
}

/***************************************************************************
  Draw graphics mode with 640x200 pixels (default).
  The cell size is 1x1 (1 scanline is the real default)
  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
***************************************************************************/

POISK1_UPDATE_ROW(p1_state::cga_gfx_1bpp_update_row)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix32(ra);
	uint8_t fg = 15, bg = BG_COLOR(m_video.color_select_68);
	uint16_t odd, offset;
	int i;

	if (ra == 0) LOG("cga_gfx_1bpp_update_row bg %d\n", bg);
	odd = (ra & 1) << 13;
	offset = (ma & 0x1fff) | odd;
	for (i = 0; i < stride; i++)
	{
		uint8_t data = videoram[ offset++ ];

		*p = palette[( data & 0x80 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg ]; p++;
	}
}

/***************************************************************************
  Draw graphics mode with 640x200 pixels + extra highlight color for text
  mode emulation
  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
***************************************************************************/

POISK1_UPDATE_ROW(p1_state::poisk1_gfx_1bpp_update_row)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t *p = &bitmap.pix32(ra);
	uint8_t fg, bg = BG_COLOR(m_video.color_select_68);
	uint16_t odd, offset;
	int i;

	if (ra == 0) LOG("poisk1_gfx_1bpp_update_row bg %d\n", bg);
	odd = (ra & 1) << 13;
	offset = (ma & 0x1fff) | odd;
	for (i = 0; i < stride; i++)
	{
		uint8_t data = videoram[ offset++ ];

		fg = (data & 0x80) ? ( (m_video.color_select_68 & 0x20) ? 10 : 11 ) : 15; // XXX
		*p = palette[bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg ]; p++;
	}
}

// Initialise the cga palette
void p1_state::p1_palette(palette_device &palette) const
{
	for (int i = 0; i < CGA_PALETTE_SETS * 16; i++)
		palette.set_pen_color(i, cga_palette[i][0], cga_palette[i][1], cga_palette[i][2]);
}

void p1_state::video_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	memset(&m_video, 0, sizeof(m_video));
	m_video.videoram_base = std::make_unique<uint8_t[]>(0x8000);
	m_video.videoram = m_video.videoram_base.get();
	m_video.stride = 80;

	space.install_readwrite_bank(0xb8000, 0xbbfff, "bank11");
	machine().root_device().membank("bank11")->set_base(m_video.videoram);
	space.install_readwrite_bank(0xbc000, 0xbffff, "bank12");
	machine().root_device().membank("bank12")->set_base(m_video.videoram + 0x4000);
}

uint32_t p1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint16_t ra, ma = 0;

	if (!m_video.stride || !m_video.videoram) return 0;

	// bit 6 of 6Ah disables color burst -- not implemented
	for (ra = cliprect.min_y; ra <= cliprect.max_y; ra++)
	{
		if (BIT(m_video.color_select_68, 7))
		{
			if (BIT(m_video.mode_control_6a, 7))
			{
				cga_gfx_1bpp_update_row(bitmap, cliprect, m_video.videoram, ma, ra, m_video.stride);
			}
			else
			{
				poisk1_gfx_1bpp_update_row(bitmap, cliprect, m_video.videoram, ma, ra, m_video.stride);
			}
		}
		else
		{
			cga_gfx_2bpp_update_row(bitmap, cliprect, m_video.videoram, ma, ra, m_video.stride);
		}
		if (ra & 1) ma += m_video.stride;
	}

	return 0;
}

// Timer.  Poisk-1 uses single XTAL for everything? -- check

WRITE_LINE_MEMBER(p1_state::p1_speaker_set_spkrdata)
{
	m_p1_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_p1_spkrdata & m_p1_input);
}

WRITE_LINE_MEMBER(p1_state::p1_pit8253_out2_changed)
{
	m_p1_input = state ? 1 : 0;
	m_speaker->level_w(m_p1_spkrdata & m_p1_input);
}

// Keyboard (via PPI)

WRITE8_MEMBER(p1_state::p1_ppi_porta_w)
{
	m_kbpoll_mask = data;
	LOGDBG("p1_ppi_porta_w %02X <- %02X\n", m_kbpoll_mask, data);
}

READ8_MEMBER(p1_state::p1_ppi_porta_r)
{
	uint8_t ret;

	ret = m_kbpoll_mask;
	LOG("p1_ppi_porta_r = %02X\n", ret);
	return ret;
}

READ8_MEMBER(p1_state::p1_ppi_portb_r)
{
	uint16_t key = 0xffff;
	uint8_t ret = 0;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_kbpoll_mask, i))
		{
			key &= m_kbdio[i]->read();
		}
	}

	ret = key & 0xff;
//  LOG("p1_ppi_portb_r = %02X\n", ret);
	return ret;
}

READ8_MEMBER(p1_state::p1_ppi_portc_r)
{
	uint16_t key = 0xffff;
	uint8_t ret = 0;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_kbpoll_mask, i))
		{
			key &= m_kbdio[i]->read();
		}
	}

	ret = (key >> 8) & 0xff;
	LOGDBG("p1_ppi_portc_r = %02X\n", ret);
	return ret;
}

// XXX

READ8_MEMBER(p1_state::p1_ppi2_portc_r)
{
	int data = 0xff;
	double tap_val = m_cassette->input();

	data = (data & ~0x10) | (tap_val < 0 ? 0x10 : 0x00);

	LOGDBG("p1_ppi_portc_r = %02X (tap_val %f) at %s\n", data, tap_val, machine().describe_context());
	return data;
}

WRITE8_MEMBER(p1_state::p1_ppi2_portb_w)
{
	m_pit8253->write_gate2(BIT(data, 0));
	p1_speaker_set_spkrdata(data & 0x02);
}

READ8_MEMBER(p1_state::p1_ppi_r)
{
	switch (offset)
	{
	case 0:
		return m_ppi8255n1->read(0);
	case 9:
	case 10:
	case 11:
		return m_ppi8255n1->read(offset - 8);
	case 8:
		return m_ppi8255n2->read(0);
	case 1:
	case 2:
	case 3:
		return m_ppi8255n2->read(offset);
	default:
		LOG("p1ppi R %.2x (unimp)\n", 0x60 + offset);
		return 0xff;
	}
}

WRITE8_MEMBER(p1_state::p1_ppi_w)
{
	switch (offset)
	{
	case 0:
		return m_ppi8255n1->write(0, data);
	case 9:
	case 10:
	case 11:
		return m_ppi8255n1->write(offset - 8, data);
	case 8:
		return m_ppi8255n2->write(0, data);
	case 1:
	case 2:
	case 3:
		return m_ppi8255n2->write(offset, data);
	default:
		LOG("p1ppi W %.2x $%02x (unimp)\n", 0x60 + offset, data);
		return;
	}
}

/**********************************************************
 *
 * NMI handling
 *
 **********************************************************/

void p1_state::init_poisk1()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.install_readwrite_bank(0, m_ram->size() - 1, "bank10");
	membank("bank10")->set_base(m_ram->pointer());
}

void p1_state::machine_start()
{
}

void p1_state::machine_reset()
{
	m_kbpoll_mask = 0;
}

/*
 * macros
 */

void p1_state::poisk1_map(address_map &map)
{
	map.unmap_value_high();
	map(0xfc000, 0xfffff).rom().region("bios", 0xc000);
}

void p1_state::poisk1_io(address_map &map)
{
	map(0x0020, 0x0021).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0028, 0x002B).rw(FUNC(p1_state::p1_trap_r), FUNC(p1_state::p1_trap_w));
	map(0x0040, 0x0043).rw(m_pit8253, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	// can't use regular AM_DEVREADWRITE, because THIS IS SPARTA!
	// 1st PPI occupies ports 60, 69, 6A and 6B; 2nd PPI -- 68, 61, 62 and 63.
	map(0x0060, 0x006F).rw(FUNC(p1_state::p1_ppi_r), FUNC(p1_state::p1_ppi_w));
	map(0x03D0, 0x03DF).rw(FUNC(p1_state::p1_cga_r), FUNC(p1_state::p1_cga_w));
}

static INPUT_PORTS_START( poisk1 )
	PORT_INCLUDE( poisk1_keyboard_v91 )
INPUT_PORTS_END

void p1_state::poisk1(machine_config &config)
{
	/* basic machine hardware */
	I8088(config, m_maincpu, 5000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &p1_state::poisk1_map);
	m_maincpu->set_addrmap(AS_IO, &p1_state::poisk1_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259", FUNC(pic8259_device::inta_cb));

	PIT8253(config, m_pit8253);
	m_pit8253->set_clk<0>(XTAL(15'000'000)/12); /* heartbeat IRQ */
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8253->set_clk<1>(XTAL(15'000'000)/12); /* keyboard poll -- XXX edge or level triggered? */
	m_pit8253->out_handler<1>().set(m_pic8259, FUNC(pic8259_device::ir6_w));
	m_pit8253->set_clk<2>(XTAL(15'000'000)/12); /* pio port c pin 4, and speaker polling enough */
	m_pit8253->out_handler<2>().set(FUNC(p1_state::p1_pit8253_out2_changed));

	PIC8259(config, m_pic8259);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	I8255A(config, m_ppi8255n1);
	m_ppi8255n1->in_pa_callback().set(FUNC(p1_state::p1_ppi_porta_r)); /*60H*/
	m_ppi8255n1->out_pa_callback().set(FUNC(p1_state::p1_ppi_porta_w));
	m_ppi8255n1->in_pb_callback().set(FUNC(p1_state::p1_ppi_portb_r)); /*69H*/
	m_ppi8255n1->in_pc_callback().set(FUNC(p1_state::p1_ppi_portc_r));
	m_ppi8255n1->out_pc_callback().set(FUNC(p1_state::p1_ppi_portc_w));   /*6AH*/

	I8255A(config, m_ppi8255n2);
	m_ppi8255n2->out_pa_callback().set(FUNC(p1_state::p1_ppi2_porta_w));  /*68H*/
	m_ppi8255n2->out_pb_callback().set(FUNC(p1_state::p1_ppi2_portb_w));  /*61H*/
	m_ppi8255n2->in_pc_callback().set(FUNC(p1_state::p1_ppi2_portc_r));    /*62H*/

	ISA8(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->irq2_callback().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	m_isabus->irq3_callback().set(m_pic8259, FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set(m_pic8259, FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set(m_pic8259, FUNC(pic8259_device::ir5_w));
	m_isabus->irq7_callback().set(m_pic8259, FUNC(pic8259_device::ir7_w));

	ISA8_SLOT(config, "isa1", 0, m_isabus, p1_isa8_cards, "fdc", false); // FIXME: determine ISA bus clock
	ISA8_SLOT(config, "isa2", 0, m_isabus, p1_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa3", 0, m_isabus, p1_isa8_cards, nullptr, false);
	ISA8_SLOT(config, "isa4", 0, m_isabus, p1_isa8_cards, nullptr, false);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);

	SOFTWARE_LIST(config, "flop_list").set_original("poisk1_flop");
//  SOFTWARE_LIST(config, "cass_list").set_original("poisk1_cass");

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(15'000'000), 912,0,640, 262,0,200);
	m_screen->set_screen_update(FUNC(p1_state::screen_update));

	/* XXX verify palette */
	PALETTE(config, m_palette, FUNC(p1_state::p1_palette), CGA_PALETTE_SETS * 16);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("512K");
}

ROM_START( poisk1 )
	ROM_REGION16_LE(0x10000,"bios", 0)

	ROM_DEFAULT_BIOS("v91")
	ROM_SYSTEM_BIOS(0, "v89r0", "1989r0")
	ROMX_LOAD("bios.rf6", 0xe000, 0x2000, CRC(c0f333e3) SHA1(a44f355b7deae3693e1462d57543a42944fd0969), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v89", "1989")
	ROMX_LOAD("biosp1s.rf4", 0xe000, 0x2000, CRC(1a85f671) SHA1(f0e59b2c4d92164abca55a96a58071ce869ff988), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v91", "1991")
	ROMX_LOAD("poisk_1991.bin", 0xe000, 0x2000, CRC(d61c56fd) SHA1(de202e1f7422d585a1385a002a4fcf9d756236e5), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "v91r2", "1991r2")
	ROMX_LOAD("p_bios_nm.bin", 0xe000, 0x2000, CRC(84430b4f) SHA1(3e477962be3cea09662cb2e3ad9966ad01c7455d), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "test1", "Test 1")
	ROMX_LOAD("test1.rf6", 0x00000, 0x2000, CRC(a5f05dff) SHA1(21dd0cea605bd7be22e94f8355d86b2478d9527e), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(5, "test2", "Test 2")
	ROMX_LOAD("test2.rf6", 0x00000, 0x2000, CRC(eff730e4) SHA1(fcbc08de9b8592c974eaea837839f1a9caf36a75), ROM_BIOS(5))
	ROM_SYSTEM_BIOS(6, "test3", "Test 3")
	ROMX_LOAD("test3.rf6", 0x00000, 0x2000, CRC(23025dc9) SHA1(dca4cb580162bb28f6e49ff625b677001d40d573), ROM_BIOS(6))
	ROM_SYSTEM_BIOS(7, "test4", "Test 4")
	ROMX_LOAD("test4.rf6", 0x00000, 0x2000, CRC(aac8fc5e) SHA1(622abb5ac66d38a474ee54fe016aff0ba0b5794f), ROM_BIOS(7))
	ROM_SYSTEM_BIOS(8, "test5", "Test 5")
	ROMX_LOAD("test5.rf6", 0x00000, 0x2000, CRC(f308e679) SHA1(37bd35f62015d338b3347fd4e3ec455eab048b66), ROM_BIOS(8))

	// 0xc0000, sets 80x25 text and loops asking for 'Boot from hard disk (Y or N)?'
	ROM_LOAD("boot_net.rf4", 0x00000, 0x2000, CRC(316c2030) SHA1(d043325596455772252e465b85321f1b5c529d0b)) // NET BIOS
	// 0xc0000, accesses ports 0x90..0x97
	ROM_LOAD("pois_net.bin", 0x00000, 0x2000, CRC(cf9dd80a) SHA1(566bcb40c0cb2c8bfd5b485f0db689fdeaca3e86)) // ??? BIOS

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD("poisk.cga", 0x0000, 0x0800, CRC(f6eb39f0) SHA1(0b788d8d7a8e92cc612d044abcb2523ad964c200))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT   COMPAT  MACHINE  INPUT   CLASS     INIT         COMPANY         FULLNAME   FLAGS
COMP( 1989, poisk1, ibm5150, 0,      poisk1,  poisk1, p1_state, init_poisk1, "Electronmash", "Poisk-1", 0 )
