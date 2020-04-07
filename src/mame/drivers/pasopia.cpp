// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Toshiba Pasopia

    Machine unusable due to keyboard issues.
    Cassette works.
    Sound uses the beeper, and works. Try SOUND a,b where a=1-82, b=1-255.

    TODO:
    - machine emulation needs merging with Pasopia 7 (video emulation is
      completely different tho)
    - screen resolution switching
    - Centronics printer interface
    - FDC and other I/O expansions
    - fix keyboard
    - colours
    - graphics


****************************************************************************/

#include "emu.h"
#include "includes/pasopia.h"

#include "bus/pasopia/pac2.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "video/mc6845.h"
#include "imagedev/cassette.h"
#include "sound/spkrdev.h"
#include "speaker.h"
#include "emupal.h"
#include "screen.h"
#include "softlist.h"

class pasopia_state : public driver_device
{
public:
	pasopia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_basic(*this, "basic")
		, m_p_chargen(*this, "chargen")
		, m_p_ram(*this, "ram")
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_ctc(*this, "ctc")
		, m_pio(*this, "pio")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_io_keyboard(*this, "KEY.%d", 0)
		, m_cass(*this, "cassette")
		, m_rs232(*this, "rs232")
		, m_speaker(*this, "speaker")
	{ }

	void pasopia(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void pasopia_ctrl_w(u8 data);
	u8 memory_r(offs_t offset);
	void vram_addr_lo_w(u8 data);
	void vram_latch_w(u8 data);
	u8 vram_latch_r();
	u8 portb_1_r();
	u8 portb_2_r();
	void porta_2_w(u8 data);
	void portc_2_w(u8 data);
	void vram_addr_hi_w(u8 data);
	void screen_mode_w(u8 data);
	u8 rombank_r();
	u8 keyb_r();
	void mux_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(speaker_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	TIMER_CALLBACK_MEMBER(pio_timer);

	void pasopia_io(address_map &map);
	void pasopia_map(address_map &map);

	u8 m_hblank;
	u16 m_vram_addr;
	u16 m_vram_latch;
	u8 m_gfx_mode;
	u8 m_mux_data;
	u8 m_porta_2;
	bool m_video_wl;
	bool m_ram_bank;
	bool m_spr_sw;
	emu_timer *m_pio_timer;
	std::unique_ptr<u16[]> m_p_vram;

	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_p_basic;
	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_ram;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_device<z80ctc_device> m_ctc;
	required_device<z80pio_device> m_pio;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_ioport_array<12> m_io_keyboard;
	required_device<cassette_image_device> m_cass;
	required_device<rs232_port_device> m_rs232;
	required_device<speaker_sound_device> m_speaker;
};

// needed to scan the keyboard, as the pio emulation doesn't do it.
TIMER_CALLBACK_MEMBER( pasopia_state::pio_timer )
{
	m_pio->port_b_write(keyb_r());
}

MC6845_UPDATE_ROW( pasopia_state::crtc_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u8 fg=7,bg=0; // colours need to be determined
	uint32_t *p = &bitmap.pix32(y);

	for (u16 x = 0; x < x_count; x++)
	{
		u8 inv = (x == cursor_x) ? 0xff : 0;
		u16 mem = ma + x;
		u16 chr = m_p_vram[mem & 0x7ff];

		/* get pattern of pixels for that character scanline */
		u8 gfx = m_p_chargen[((chr & 0xff)<<3) | ra] ^ inv;

		/* Display a scanline of a character */
		for (u8 xi = 0; xi < 8; xi++)
			*p++ = palette[BIT(gfx, 7-xi) ? fg : bg];
	}
}

void pasopia_state::pasopia_ctrl_w(u8 data)
{
	m_ram_bank = BIT(data, 1);
}

u8 pasopia_state::memory_r(offs_t offset)
{
	if (offset < 0x8000 && !m_ram_bank)
		return m_p_basic[offset];
	else
		return m_p_ram[offset];
}

void pasopia_state::pasopia_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(pasopia_state::memory_r));
	map(0x0000, 0xffff).writeonly().share("ram");
}


void pasopia_state::pasopia_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ppi0, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x10, 0x10).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x11, 0x11).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x18, 0x1b).rw("dtfcst", FUNC(pasopia_pac2_slot_device::read), FUNC(pasopia_pac2_slot_device::write));
	map(0x1c, 0x1f).rw("dtfunt", FUNC(pasopia_pac2_slot_device::read), FUNC(pasopia_pac2_slot_device::write));
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
	m_p_vram = make_unique_clear<u16[]>(0x4000);

	m_pio_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pasopia_state::pio_timer), this));
	m_pio_timer->adjust(attotime::from_hz(50), 0, attotime::from_hz(50));

	m_hblank = 0;
}

void pasopia_state::machine_reset()
{
	m_porta_2 = 0xFF;
	m_cass->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

void pasopia_state::vram_addr_lo_w(u8 data)
{
	m_vram_addr = (m_vram_addr & 0x3f00) | data;
}

void pasopia_state::vram_latch_w(u8 data)
{
	m_vram_latch = (m_vram_latch & 0x100) | data;
}

u8 pasopia_state::vram_latch_r()
{
	return m_p_vram[m_vram_addr] & 0xff;
}

u8 pasopia_state::portb_1_r()
{
	/*
	x--- ---- attribute latch
	-x-- ---- hblank
	--x- ---- vblank
	---x ---- LCD system mode, active low
	*/
	u8 grph_latch,lcd_mode;

	m_hblank ^= 0x40; //TODO
	grph_latch = (m_p_vram[m_vram_addr] & 0x100) >> 1;
	lcd_mode = 0x10;

	return m_hblank | lcd_mode | grph_latch; //bit 4: LCD mode
}

u8 pasopia_state::portb_2_r()
{
	u8 result = (m_cass->input() > +0.04) ? 0x20 : 0;
	result |= m_rs232->dcd_r() << 3;
	result |= m_rs232->dsr_r() << 2;
	result |= m_rs232->cts_r() << 1;
	result |= m_rs232->rxd_r() << 0;
	return result;
}

void pasopia_state::porta_2_w(u8 data)
{
	m_cass->output(BIT(data, 4) ? -1.0 : +1.0);
	u8 changed = data ^ m_porta_2;
	m_porta_2 = data;
	if (BIT(changed, 5))
	{
		m_cass->change_state(BIT(data, 5) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	}
}

void pasopia_state::portc_2_w(u8 data)
{
	m_rs232->write_dtr(BIT(data, 6));
	m_rs232->write_rts(BIT(data, 5));
	m_rs232->write_txd(BIT(data, 4));
}

WRITE_LINE_MEMBER( pasopia_state::speaker_w )
{
	if (state)
	{
		m_spr_sw ^= 1;
		if (BIT(m_mux_data, 7))
			m_speaker->level_w(m_spr_sw);
	}
}

void pasopia_state::vram_addr_hi_w(u8 data)
{
	m_vram_latch = u16(data & 0x80) << 1 | m_vram_latch;
	if ( BIT(data, 6) && !m_video_wl )
	{
		m_p_vram[m_vram_addr] = m_vram_latch;
	}

	m_video_wl = BIT(data, 6);
	m_vram_addr = (m_vram_addr & 0xff) | ((data & 0x3f) << 8);
}

void pasopia_state::screen_mode_w(u8 data)
{
	m_gfx_mode = (data & 0xe0) >> 5;
	//m_attr_latch = (m_attr_latch & 0x80) | (data & 7);
	printf("Screen Mode=%02x\n",data);
}

u8 pasopia_state::rombank_r()
{
	return (m_ram_bank) ? 4 : 0;
}

u8 pasopia_state::keyb_r()
{
	u8 data = 0xff;
	for (u8 i = 0; i < 3; i++)
		if (BIT(m_mux_data, i+4))
			for (u8 j = 0; j < 4; j++)
				if (BIT(m_mux_data, j))
					data &= m_io_keyboard[i*4+j]->read();

	return data;
}

void pasopia_state::mux_w(u8 data)
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
	{ "ctc" },
	{ "pio" },
//  { "fdc" }, /* TODO */
	{ nullptr }
};



void pasopia_state::pasopia(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 15.9744_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &pasopia_state::pasopia_map);
	m_maincpu->set_addrmap(AS_IO, &pasopia_state::pasopia_io);
	m_maincpu->set_daisy_config(pasopia_daisy);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.318181_MHz_XTAL / 2, 456, 0, 296, 262, 0, 192);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));
	GFXDECODE(config, "gfxdecode", m_palette, gfx_pasopia);
	PALETTE(config, m_palette).set_entries(8);

	/* Devices */
	HD6845S(config, m_crtc, 14.318181_MHz_XTAL / 16);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(pasopia_state::crtc_update_row));

	I8255A(config, m_ppi0);
	m_ppi0->out_pa_callback().set(FUNC(pasopia_state::vram_addr_lo_w));
	m_ppi0->out_pb_callback().set(FUNC(pasopia_state::vram_latch_w));
	m_ppi0->in_pc_callback().set(FUNC(pasopia_state::vram_latch_r));

	I8255A(config, m_ppi1);
	m_ppi1->out_pa_callback().set(FUNC(pasopia_state::screen_mode_w));
	m_ppi1->in_pb_callback().set(FUNC(pasopia_state::portb_1_r));
	m_ppi1->out_pc_callback().set(FUNC(pasopia_state::vram_addr_hi_w));

	I8255A(config, m_ppi2);
	m_ppi2->out_pa_callback().set(FUNC(pasopia_state::porta_2_w));
	m_ppi2->in_pb_callback().set(FUNC(pasopia_state::portb_2_r));
	m_ppi2->in_pc_callback().set(FUNC(pasopia_state::rombank_r));
	m_ppi2->out_pc_callback().set(FUNC(pasopia_state::portc_2_w));

	Z80CTC(config, m_ctc, 15.9744_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<1>().set(FUNC(pasopia_state::speaker_w));
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	Z80PIO(config, m_pio, 15.9744_MHz_XTAL / 4);
	m_pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio->out_pa_callback().set(FUNC(pasopia_state::mux_w));
	m_pio->in_pb_callback().set(FUNC(pasopia_state::keyb_r));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	PASOPIA_PAC2(config, "dtfcst", pac2_default_devices, nullptr); // "Data File Cassette"
	PASOPIA_PAC2(config, "dtfunt", pac2_default_devices, nullptr); // "Data File Unit"

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);

	SOFTWARE_LIST(config, "cass_list").set_original("pasopia_cass");
}

/* ROM definition */
ROM_START( pasopia )
	ROM_REGION( 0x8000, "basic", 0 )
	ROM_LOAD( "tbasic.rom", 0x0000, 0x8000, CRC(f53774ff) SHA1(bbec45a3bad8d184505cc6fe1f6e2e60a7fb53f2))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "font.rom", 0x0000, 0x0800, BAD_DUMP CRC(a91c45a9) SHA1(a472adf791b9bac3dfa6437662e1a9e94a88b412)) //stolen from pasopia7
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY    FULLNAME   FLAGS
COMP( 1982, pasopia, 0,      0,      pasopia, pasopia, pasopia_state, empty_init, "Toshiba", "Personal Computer Pasopia PA7010", MACHINE_NOT_WORKING )
