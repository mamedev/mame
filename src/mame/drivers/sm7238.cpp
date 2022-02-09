// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    SM 7238 (aka T3300) color/mono text terminal, compatible with DEC VT 240.
    Graphics options add Tek 401x and DEC ReGIS support

    Technical manual and schematics: http://doc.pdp-11.org.ru/Terminals/CM7238/

    To do:
    - graphics options
    - colors
    - document hardware and ROM variants, verify if pixel stretching is done
    - verify blink frequency

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/km035.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"

#include "emupal.h"
#include "screen.h"


static constexpr int KSM_COLUMNS_MAX = 132;

static constexpr int KSM_TOTAL_HORZ = KSM_COLUMNS_MAX * 10;
static constexpr int KSM_DISP_HORZ = KSM_COLUMNS_MAX * 8;

static constexpr int KSM_TOTAL_VERT = 260;
static constexpr int KSM_DISP_VERT = 250;


//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


class sm7238_state : public driver_device
{
public:
	sm7238_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_videobank(*this, "videobank")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_p_charram(*this, "charram")
		, m_pic8259(*this, "pic8259")
		, m_i8251line(*this, "i8251line")
		, m_rs232(*this, "rs232")
		, m_i8251kbd(*this, "i8251kbd")
		, m_keyboard(*this, "keyboard")
		, m_i8251prn(*this, "i8251prn")
		, m_printer(*this, "prtr")
		, m_t_hblank(*this, "t_hblank")
		, m_t_vblank(*this, "t_vblank")
		, m_t_color(*this, "t_color")
		, m_t_iface(*this, "t_iface")
		, m_screen(*this, "screen")
	{ }

	void sm7238(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER(write_keyboard_clock);
	DECLARE_WRITE_LINE_MEMBER(write_printer_clock);

	void control_w(uint8_t data);
	void text_control_w(uint8_t data);
	void vmem_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void sm7238_io(address_map &map);
	void sm7238_mem(address_map &map);
	void videobank_map(address_map &map);

	void recompute_parameters();

	struct
	{
		uint8_t control;
		uint16_t ptr;
		int stride;
		bool reverse;
	} m_video;

	virtual void machine_reset() override;

	required_device<i8080_cpu_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<address_map_bank_device> m_videobank;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_charram;
	required_device<pic8259_device> m_pic8259;
	required_device<i8251_device> m_i8251line;
	required_device<rs232_port_device> m_rs232;
	required_device<i8251_device> m_i8251kbd;
	required_device<km035_device> m_keyboard;
	required_device<i8251_device> m_i8251prn;
	required_device<rs232_port_device> m_printer;
	required_device<pit8253_device> m_t_hblank;
	required_device<pit8253_device> m_t_vblank;
	required_device<pit8253_device> m_t_color;
	required_device<pit8253_device> m_t_iface;
	required_device<screen_device> m_screen;
};

void sm7238_state::sm7238_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x9fff).rom();
	map(0xa000, 0xa7ff).ram();
	map(0xb000, 0xb3ff).ram().share("nvram");
	map(0xb800, 0xb800).w(FUNC(sm7238_state::text_control_w));
	map(0xbc00, 0xbc00).w(FUNC(sm7238_state::control_w));
	map(0xc000, 0xcfff).ram().share("charram");
	map(0xe000, 0xffff).m(m_videobank, FUNC(address_map_bank_device::amap8));
}

void sm7238_state::videobank_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("videoram");
	map(0x2000, 0x2fff).mirror(0x1000).w(FUNC(sm7238_state::vmem_w));
}

void sm7238_state::sm7238_io(address_map &map)
{
	map.unmap_value_high();
//  map(0x40, 0x4f).ram() // LUT
	map(0xa0, 0xa1).rw(m_i8251line, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xa4, 0xa5).rw(m_i8251kbd, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xa8, 0xab).rw(m_t_color, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xac, 0xad).rw(m_pic8259, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xb0, 0xb3).rw(m_t_hblank, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xb4, 0xb7).rw(m_t_vblank, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xb8, 0xb9).rw(m_i8251prn, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xbc, 0xbf).rw(m_t_iface, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
}

void sm7238_state::machine_reset()
{
	memset(&m_video, 0, sizeof(m_video));
	m_videobank->set_bank(0);
}

void sm7238_state::control_w(uint8_t data)
{
	LOG("Control Write: %02xh: lut %d nvram %d c2 %d iack %d\n",
		data, BIT(data, 0), BIT(data, 2), BIT(data, 3), BIT(data, 5));
}

void sm7238_state::text_control_w(uint8_t data)
{
	if (data ^ m_video.control)
	{
		LOG("Text Control Write: %02xh: 80/132 %d dma %d clr %d dlt %d inv %d ?? %d\n",
			data, BIT(data, 0), BIT(data, 1), BIT(data, 2), BIT(data, 3), BIT(data, 4), BIT(data, 5));
	}

	if (BIT((data ^ m_video.control), 0))
	{
		m_video.stride = BIT(data, 0) ? 80 : 132;
		recompute_parameters();
	}

	if (BIT((data ^ m_video.control), 2))
	{
		m_videobank->set_bank(1 - BIT(data, 2));
	}

	m_video.reverse = BIT(data, 4);
	m_video.control = data;
}

void sm7238_state::vmem_w(offs_t offset, uint8_t data)
{
	m_p_videoram[offset] = data;
	m_p_videoram[offset + 0x1000] = data;
}

WRITE_LINE_MEMBER(sm7238_state::write_keyboard_clock)
{
	m_i8251kbd->write_txc(state);
	m_i8251kbd->write_rxc(state);
}

WRITE_LINE_MEMBER(sm7238_state::write_printer_clock)
{
	m_i8251prn->write_txc(state);
	m_i8251prn->write_rxc(state);
}

void sm7238_state::recompute_parameters()
{
	rectangle visarea;
	attoseconds_t refresh;

	visarea.set(0, m_video.stride * 8 - 1, 0, KSM_DISP_VERT - 1);

	if (m_video.stride == 80)
	{
		refresh = HZ_TO_ATTOSECONDS(12.5_MHz_XTAL) * m_video.stride * 10 * KSM_TOTAL_VERT;
	}
	else
	{
		refresh = HZ_TO_ATTOSECONDS(20.625_MHz_XTAL) * m_video.stride * 10 * KSM_TOTAL_VERT;
	}

	m_screen->configure(m_video.stride * 10, KSM_TOTAL_VERT, visarea, refresh);
}

uint32_t sm7238_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!BIT(m_video.control, 3))
	{
		bitmap.fill(0);
		return 0;
	}

	uint8_t ctl2 = 0;
	uint16_t sy = 0, ma = 0;
	bool double_width = false, double_height = false, bottom_half = false;
	bool blink((m_screen->frame_number() % 30) > 14);

	for (uint8_t y = 0; y < 26; y++)
	{
		for (uint8_t ra = 0; ra < 10; ra++)
		{
			if (y == 1 && ctl2 && ra < ctl2)
				continue;

			uint16_t *p = &bitmap.pix(sy++, 0);

			for (uint16_t x = ma; x < ma + m_video.stride; x++)
			{
				uint16_t chr = m_p_videoram[x] << 4;
				uint8_t const attr = m_p_videoram[x + 0x1000];

				// alternate font 1
				if (BIT(attr, 6))
				{
					chr += 0x1000;
				}

				uint8_t bg = 0;
				uint8_t fg = 1;

				uint16_t chraddr;
				if (double_height)
				{
					chraddr = chr | (bottom_half ? (5 + (ra >> 1)) : (ra >> 1));
				}
				else
				{
					chraddr = chr | ra;
				}

				// alternate font 2 (downloadable font) -- only in models .05 and .06
				uint8_t gfx;
				if (BIT(attr, 7) && m_p_charram[chr + 15])
				{
					gfx = m_p_charram[chraddr] ^ 255;
				}
				else
				{
					gfx = m_p_chargen[chraddr] ^ 255;
				}

				/* Process attributes */
				if ((BIT(attr, 1)) && (ra == 9))
				{
					gfx = 0xff; // underline
				}
				if (BIT(attr, 2) && blink)
				{
					gfx = 0;
				}
				if (BIT(attr, 3))
				{
					gfx ^= 0xff; // reverse video
				}
				if (BIT(attr, 4))
				{
					fg = 2; // highlight
				}
				else
				{
					fg = 1;
				}
				if (m_video.reverse)
				{
					bg = fg;
					fg = 0;
				}

				for (int i = 7; i >= 0; i--)
				{
					*p++ = BIT(gfx, i) ? fg : bg;
					if (double_width)
						*p++ = BIT(gfx, i) ? fg : bg;
				}

				if (double_width) x++;
			}
		}
		uint8_t const ctl1 = m_p_videoram[ma + 0x1000 + m_video.stride];
		double_width = BIT(ctl1, 6);
		double_height = BIT(ctl1, 7);
		bottom_half = BIT(ctl1, 5);

		ctl2 = m_p_videoram[ma + 0x1000 + m_video.stride + 1] >> 4;

		ma = m_p_videoram[ma + m_video.stride + 1] |
			(m_p_videoram[ma + 0x1000 + m_video.stride + 1] << 8);
		ma &= 0x0fff;
	}

	return 0;
}


/* F4 Character Displayer */
static const gfx_layout sm7238_charlayout =
{
	8, 12,                  /* most chars use 8x10 pixels */
	512,                    /* 512 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	16*8                 /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_sm7238 )
	GFXDECODE_ENTRY("chargen", 0x0000, sm7238_charlayout, 0, 1)
GFXDECODE_END

void sm7238_state::sm7238(machine_config &config)
{
	I8080(config, m_maincpu, 16.5888_MHz_XTAL / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &sm7238_state::sm7238_mem);
	m_maincpu->set_addrmap(AS_IO, &sm7238_state::sm7238_io);
	m_maincpu->in_inta_func().set("pic8259", FUNC(pic8259_device::acknowledge));

	ADDRESS_MAP_BANK(config, "videobank").set_map(&sm7238_state::videobank_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x2000);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER, rgb_t::green());
	m_screen->set_raw(20.625_MHz_XTAL, KSM_TOTAL_HORZ, 0, KSM_DISP_HORZ, KSM_TOTAL_VERT, 0, KSM_DISP_VERT);
	m_screen->set_screen_update(FUNC(sm7238_state::screen_update));
	m_screen->screen_vblank().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_HIGHLIGHT);
	GFXDECODE(config, "gfxdecode", "palette", gfx_sm7238);

	PIC8259(config, m_pic8259, 0);
	m_pic8259->out_int_callback().set_inputline(m_maincpu, 0);

	PIT8253(config, m_t_hblank, 0);
	m_t_hblank->set_clk<1>(16.384_MHz_XTAL / 9); // FIXME -- keyboard is slower and doesn't sync otherwise
	m_t_hblank->out_handler<1>().set(FUNC(sm7238_state::write_keyboard_clock));

	PIT8253(config, m_t_vblank, 0);
	m_t_vblank->set_clk<2>(16.5888_MHz_XTAL / 9);
	m_t_vblank->out_handler<2>().set(FUNC(sm7238_state::write_printer_clock));

	PIT8253(config, m_t_color, 0);

	PIT8253(config, m_t_iface, 0);
	m_t_iface->set_clk<1>(16.5888_MHz_XTAL / 9);
	m_t_iface->out_handler<1>().set(m_i8251line, FUNC(i8251_device::write_txc));
	m_t_iface->set_clk<2>(16.5888_MHz_XTAL / 9);
	m_t_iface->out_handler<2>().set(m_i8251line, FUNC(i8251_device::write_rxc));

	// serial connection to host
	I8251(config, m_i8251line, 0);
	m_i8251line->txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	m_i8251line->dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	m_i8251line->rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	m_i8251line->rxrdy_handler().set("pic8259", FUNC(pic8259_device::ir1_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "null_modem"));
	rs232.rxd_handler().set(m_i8251line, FUNC(i8251_device::write_rxd));
	rs232.cts_handler().set(m_i8251line, FUNC(i8251_device::write_cts));
	rs232.dsr_handler().set(m_i8251line, FUNC(i8251_device::write_dsr));

	// serial connection to KM-035 keyboard
	I8251(config, m_i8251kbd, 0);
	m_i8251kbd->txd_handler().set(m_keyboard, FUNC(km035_device::write_rxd));
	m_i8251kbd->rxrdy_handler().set(m_pic8259, FUNC(pic8259_device::ir3_w));

	KM035(config, m_keyboard, 0);
	m_keyboard->tx_handler().set(m_i8251kbd, FUNC(i8251_device::write_rxd));
	m_keyboard->rts_handler().set(m_i8251kbd, FUNC(i8251_device::write_cts));

	// serial connection to printer
	I8251(config, m_i8251prn, 0);
	m_i8251prn->rxrdy_handler().set("pic8259", FUNC(pic8259_device::ir3_w));

	rs232_port_device &prtr(RS232_PORT(config, "prtr", default_rs232_devices, nullptr));
	prtr.rxd_handler().set(m_i8251prn, FUNC(i8251_device::write_rxd));
	prtr.cts_handler().set(m_i8251prn, FUNC(i8251_device::write_cts));
	prtr.dsr_handler().set(m_i8251prn, FUNC(i8251_device::write_dsr));
}

ROM_START( sm7238 )
	ROM_REGION(0xa000, "maincpu", ROMREGION_ERASE00)
	// version 1.0
	ROM_LOAD( "01_6.064", 0x0000, 0x2000, CRC(10f98d90) SHA1(cbed0b92d8beb558ce27ccd8256e1b3ab7351a58))
	ROM_LOAD( "02_6.064", 0x2000, 0x2000, CRC(b22c0202) SHA1(68a5c45697c4a541a182f0762904d36f4496e344))
	ROM_LOAD( "03_6.064", 0x4000, 0x2000, CRC(3e9d37ad) SHA1(26eb257733a88bd5665a9601813934da27219bc2))
	ROM_LOAD( "04_6.064", 0x6000, 0x2000, CRC(7b8c9e06) SHA1(537bd35749c15ef66656553d9e7ec6a1f9671f98))
	// version 1.1 undumped

	ROM_REGION(0x2000, "chargen", ROMREGION_ERASE00)
	ROM_LOAD( "bsk1_00_2.064", 0x0000, 0x2000, CRC(1e3d5885) SHA1(5afdc10f775f424473c2a78de62e3bfc82bdddd1))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE         INIT        COMPANY  FULLNAME   FLAGS
COMP( 1989, sm7238, 0,      0,      sm7238,  0,     sm7238_state, empty_init, "USSR",  "SM 7238", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS )
