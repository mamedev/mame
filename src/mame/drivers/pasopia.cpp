// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Toshiba Pasopia

    TODO:
    - machine emulation needs merging with Pasopia 7 (video emulation is
      completely different tho)

****************************************************************************/

#include "emu.h"
#include "includes/pasopia.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"


class pasopia_state : public driver_device
{
public:
	pasopia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
		, m_p_vram(*this, "vram")
		, m_ppi0(*this, "ppi8255_0")
		, m_ppi1(*this, "ppi8255_1")
		, m_ppi2(*this, "ppi8255_2")
		, m_ctc(*this, "z80ctc")
		, m_pio(*this, "z80pio")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_keyboard(*this, "KEY.%u", 0)
	{ }

	void pasopia(machine_config &config);

	void init_pasopia();

private:
	DECLARE_WRITE8_MEMBER(pasopia_ctrl_w);
	DECLARE_WRITE8_MEMBER(vram_addr_lo_w);
	DECLARE_WRITE8_MEMBER(vram_latch_w);
	DECLARE_READ8_MEMBER(vram_latch_r);
	DECLARE_READ8_MEMBER(portb_1_r);
	DECLARE_WRITE8_MEMBER(vram_addr_hi_w);
	DECLARE_WRITE8_MEMBER(screen_mode_w);
	DECLARE_READ8_MEMBER(rombank_r);
	DECLARE_READ8_MEMBER(keyb_r);
	DECLARE_WRITE8_MEMBER(mux_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	TIMER_CALLBACK_MEMBER(pio_timer);

	void pasopia_io(address_map &map);
	void pasopia_map(address_map &map);

	uint8_t m_hblank;
	uint16_t m_vram_addr;
	uint8_t m_vram_latch;
	uint8_t m_attr_latch;
//  uint8_t m_gfx_mode;
	uint8_t m_mux_data;
	bool m_video_wl;
	bool m_ram_bank;
	emu_timer *m_pio_timer;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
	required_region_ptr<u8> m_p_vram;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_device<z80ctc_device> m_ctc;
	required_device<z80pio_device> m_pio;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_ioport_array<12> m_keyboard;
};

// needed to scan the keyboard, as the pio emulation doesn't do it.
TIMER_CALLBACK_MEMBER( pasopia_state::pio_timer )
{
	m_pio->port_b_write(keyb_r(generic_space(),0,0xff));
}

MC6845_UPDATE_ROW( pasopia_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint8_t chr,gfx,fg=7,bg=0; // colours need to be determined
	uint16_t mem,x;
	uint32_t *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		uint8_t inv=0;
		if (x == cursor_x) inv=0xff;
		mem = ma + x;
		chr = m_p_vram[mem & 0x7ff];

		/* get pattern of pixels for that character scanline */
		gfx = m_p_chargen[(chr<<3) | ra] ^ inv;

		/* Display a scanline of a character */
		*p++ = palette[BIT(gfx, 7) ? fg : bg];
		*p++ = palette[BIT(gfx, 6) ? fg : bg];
		*p++ = palette[BIT(gfx, 5) ? fg : bg];
		*p++ = palette[BIT(gfx, 4) ? fg : bg];
		*p++ = palette[BIT(gfx, 3) ? fg : bg];
		*p++ = palette[BIT(gfx, 2) ? fg : bg];
		*p++ = palette[BIT(gfx, 1) ? fg : bg];
		*p++ = palette[BIT(gfx, 0) ? fg : bg];
	}
}

WRITE8_MEMBER( pasopia_state::pasopia_ctrl_w )
{
	m_ram_bank = BIT(data, 1);
	membank("bank1")->set_entry(m_ram_bank);
}

void pasopia_state::pasopia_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank1").bankw("bank2");
	map(0x8000, 0xffff).ram();
}


void pasopia_state::pasopia_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ppi0, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x10).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x11, 0x11).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
//  0x18 - 0x1b pac2
//  0x1c - 0x1f something
	map(0x20, 0x23).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x28, 0x2b).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x33).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
//  0x38 printer
	map(0x3c, 0x3c).w(FUNC(pasopia_state::pasopia_ctrl_w));
}

/* Input ports */
static INPUT_PORTS_START( pasopia )
	PASOPIA_KEYBOARD
INPUT_PORTS_END

void pasopia_state::machine_start()
{
	m_hblank = 0;
	membank("bank1")->set_entry(0);
	membank("bank2")->set_entry(0);
}

void pasopia_state::machine_reset()
{
}

WRITE8_MEMBER( pasopia_state::vram_addr_lo_w )
{
	m_vram_addr = (m_vram_addr & 0x3f00) | data;
}

WRITE8_MEMBER( pasopia_state::vram_latch_w )
{
	m_vram_latch = data;
}

READ8_MEMBER( pasopia_state::vram_latch_r )
{
	return m_p_vram[m_vram_addr];
}

READ8_MEMBER( pasopia_state::portb_1_r )
{
	/*
	x--- ---- attribute latch
	-x-- ---- hblank
	--x- ---- vblank
	---x ---- LCD system mode, active low
	*/
	uint8_t grph_latch,lcd_mode;

	m_hblank ^= 0x40; //TODO
	grph_latch = (m_p_vram[m_vram_addr | 0x4000] & 0x80);
	lcd_mode = 0x10;

	return m_hblank | lcd_mode | grph_latch; //bit 4: LCD mode
}


WRITE8_MEMBER( pasopia_state::vram_addr_hi_w )
{
	m_attr_latch = (data & 0x80) | (m_attr_latch & 0x7f);
	if ( BIT(data, 6) && !m_video_wl )
	{
		m_p_vram[m_vram_addr] = m_vram_latch;
		m_p_vram[m_vram_addr | 0x4000] = m_attr_latch;
	}

	m_video_wl = BIT(data, 6);
	m_vram_addr = (m_vram_addr & 0xff) | ((data & 0x3f) << 8);
}

WRITE8_MEMBER( pasopia_state::screen_mode_w )
{
	//m_gfx_mode = (data & 0xe0) >> 5; unused variable
	m_attr_latch = (m_attr_latch & 0x80) | (data & 7);
	printf("Screen Mode=%02x\n",data);
}

READ8_MEMBER( pasopia_state::rombank_r )
{
	return (m_ram_bank) ? 4 : 0;
}

READ8_MEMBER( pasopia_state::keyb_r )
{
	uint8_t i,j,res = 0;
	for (j=0; j<3; j++)
	{
		if (BIT(m_mux_data, 4+j))
		{
			for (i=0; i<4; i++)
			{
				if (BIT(m_mux_data, i))
					res |= m_keyboard[j*4+i]->read();
			}
		}
	}

	return res ^ 0xff;
}

WRITE8_MEMBER( pasopia_state::mux_w )
{
	m_mux_data = data;
}

static const gfx_layout p7_chars_8x8 =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( gfx_pasopia )
	GFXDECODE_ENTRY( "chargen", 0x0000, p7_chars_8x8, 0, 4 )
GFXDECODE_END

static const z80_daisy_config pasopia_daisy[] =
{
	{ "z80ctc" },
	{ "z80pio" },
//  { "upd765" }, /* TODO */
	{ nullptr }
};



void pasopia_state::init_pasopia()
{
/*
We preset all banks here, so that bankswitching will incur no speed penalty.
0000 indicates ROMs, 10000 indicates RAM.
*/
	uint8_t *ram = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ram[0x00000], 0x10000);
	membank("bank2")->configure_entry(0, &ram[0x10000]);

	m_pio_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pasopia_state::pio_timer), this));
	m_pio_timer->adjust(attotime::from_hz(50), 0, attotime::from_hz(50));
}

void pasopia_state::pasopia(machine_config &config)
{
	/* basic machine hardware */

	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pasopia_state::pasopia_map);
	m_maincpu->set_addrmap(AS_IO, &pasopia_state::pasopia_io);
	m_maincpu->set_daisy_config(pasopia_daisy);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, gfx_pasopia);
	PALETTE(config, m_palette).set_entries(8);

	/* Devices */
	MC6845(config, m_crtc, XTAL(4'000'000)/4);   /* unknown variant, unknown clock, hand tuned to get ~60 fps */
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(pasopia_state::crtc_update_row), this);

	I8255A(config, m_ppi0);
	m_ppi0->out_pa_callback().set(FUNC(pasopia_state::vram_addr_lo_w));
	m_ppi0->out_pb_callback().set(FUNC(pasopia_state::vram_latch_w));
	m_ppi0->in_pc_callback().set(FUNC(pasopia_state::vram_latch_r));

	I8255A(config, m_ppi1);
	m_ppi1->out_pa_callback().set(FUNC(pasopia_state::screen_mode_w));
	m_ppi1->in_pb_callback().set(FUNC(pasopia_state::portb_1_r));
	m_ppi1->out_pc_callback().set(FUNC(pasopia_state::vram_addr_hi_w));

	I8255A(config, m_ppi2);
	m_ppi2->in_pc_callback().set(FUNC(pasopia_state::rombank_r));

	Z80CTC(config, m_ctc, XTAL(4'000'000));
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(m_ctc, FUNC(z80ctc_device::trg1));
	m_ctc->zc_callback<1>().set(m_ctc, FUNC(z80ctc_device::trg2));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	Z80PIO(config, m_pio, XTAL(4'000'000));
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->out_pa_callback().set(FUNC(pasopia_state::mux_w));
	m_pio->in_pb_callback().set(FUNC(pasopia_state::keyb_r));
}

/* ROM definition */
ROM_START( pasopia )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "tbasic.rom", 0x0000, 0x8000, CRC(f53774ff) SHA1(bbec45a3bad8d184505cc6fe1f6e2e60a7fb53f2))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, BAD_DUMP CRC(a91c45a9) SHA1(a472adf791b9bac3dfa6437662e1a9e94a88b412)) //stolen from pasopia7

	ROM_REGION( 0x8000, "vram", ROMREGION_ERASE00 )
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY    FULLNAME   FLAGS
COMP( 1986, pasopia, 0,      0,      pasopia, pasopia, pasopia_state, init_pasopia, "Toshiba", "Pasopia", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
