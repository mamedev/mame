// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/******************************************************************************

LLC2 driver by Miodrag Milanovic

2009-04-17 Preliminary driver.

2012-07-?? Updates by Robbbert

The BEL character plays a short tune.
In the monitor, it is case sensitive, most commands are uppercase,
 but some are lowercase.
To start Basic, the command is b. To quit basic, use BYE.
Inside Basic, it is not case-sensitive.

ToDo:
- Unofficial expansions
- Need software

*******************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/k7659kb.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "machine/z80pio.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class llc2_state : public driver_device
{
public:
	llc2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_speaker(*this, "speaker")
		, m_vram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_p_chargen(*this, "chargen")
		, m_cass(*this, "cassette")
		, m_ctc(*this, "ctc")
		, m_bankr(*this, "bankr%u", 0U)
		, m_bankw(*this, "bankw%u", 0U)
	{ }

	void llc2(machine_config &config);

	void init_llc2();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void rom_disable_w(u8 data);
	void basic_enable_w(u8 data);
	u8 port1b_r();
	u8 port2a_r();
	void port1b_w(u8 data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	bool m_rv = 0;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<u8> m_vram;
	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_region_ptr<u8> m_p_chargen;
	required_device<cassette_image_device> m_cass;
	required_device<z80ctc_device> m_ctc;
	required_memory_bank_array<2> m_bankr;
	required_memory_bank_array<2> m_bankw;
};

/* Address maps */
void llc2_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).bankr(m_bankr[0]).bankw(m_bankw[0]);
	map(0x4000, 0x5fff).bankr(m_bankr[1]).bankw(m_bankw[1]);
	map(0x6000, 0xbfff).ram();
	map(0xc000, 0xffff).ram().share("videoram");
}

void llc2_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xE0, 0xE3).w(FUNC(llc2_state::rom_disable_w));
	map(0xE4, 0xE7).rw("pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xE8, 0xEB).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xEC, 0xEC).w(FUNC(llc2_state::basic_enable_w));
	map(0xF8, 0xFB).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

/* Input ports */
static INPUT_PORTS_START( llc2 )
INPUT_PORTS_END


/* Driver initialization */
void llc2_state::init_llc2()
{
	u8 *r = m_ram->pointer();
	u8 *m = memregion("maincpu")->base();

	m_bankr[0]->configure_entry(0, r);
	m_bankr[0]->configure_entry(1, m);
	m_bankw[0]->configure_entry(0, r);
	m_bankr[1]->configure_entry(0, r+0x4000);
	m_bankr[1]->configure_entry(1, m+0x4000);
	m_bankw[1]->configure_entry(0, r+0x4000);
}

void llc2_state::machine_reset()
{
	m_bankr[0]->set_entry(1);
	m_bankw[0]->set_entry(0);
	m_bankr[1]->set_entry(0);
	m_bankw[1]->set_entry(0);
}

void llc2_state::rom_disable_w(u8 data)
{
	m_bankr[0]->set_entry(0);
}

void llc2_state::basic_enable_w(u8 data)
{
	m_bankr[1]->set_entry(BIT(data, 1));
}

u8 llc2_state::port1b_r()
{
	u8 data = 0xfd;

	if (m_cass->input() > 0.03)
		data |= 0x02;

	return data;
}

void llc2_state::port1b_w(u8 data)
{
	m_cass->output(BIT(data, 0) ? -1.0 : +1.0);
	m_speaker->level_w(BIT(data, 6));
	m_rv = BIT(data, 5);
}

u8 llc2_state::port2a_r()
{
	return 0; // bit 2 low or hangs on ^Z^X^C sequence
}

void llc2_state::machine_start()
{
	save_item(NAME(m_rv));
}

u32 llc2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 const inv1 = m_rv ? 0xff : 0;
	u16 sy = 0, ma = 0;

	for (u8 y = 0; y < 32; y++)
	{
		for (u8 ra = 0; ra < 8; ra++)
		{
			u8 inv = 0;
			u16 *p = &bitmap.pix(sy++);

			for (u16 x = ma; x < ma + 64; x++)
			{
				u8 chr = m_vram[x];
				if (chr==0x11) // inverse on
				{
					inv=0xff;
					chr=0x0f; // must not show
				}
				else if (chr==0x10) // inverse off
					inv=0;

				/* get pattern of pixels for that character scanline */
				u8 const gfx = m_p_chargen[ (chr << 3) | ra ] ^ inv ^ inv1;

				/* Display a scanline of a character (8 pixels) */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=64;
	}
	return 0;
}

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "pio1" },
	{ "pio2" },
	{ nullptr }
};

/* F4 Character Displayer */
static const gfx_layout charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_llc2 )
	GFXDECODE_ENTRY( "chargen", 0x0000, charlayout, 0, 1 )
GFXDECODE_END

/* Machine driver */
void llc2_state::llc2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 12_MHz_XTAL / 4);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &llc2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &llc2_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(llc2_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_llc2);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	z80pio_device& pio1(Z80PIO(config, "pio1", 12_MHz_XTAL / 4));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio1.in_pa_callback().set(K7659_KEYBOARD_TAG, FUNC(k7659_keyboard_device::read));
	pio1.in_pb_callback().set(FUNC(llc2_state::port1b_r));
	pio1.out_pb_callback().set(FUNC(llc2_state::port1b_w));

	z80pio_device& pio2(Z80PIO(config, "pio2", 12_MHz_XTAL / 4));
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio2.in_pa_callback().set(FUNC(llc2_state::port2a_r));

	Z80CTC(config, m_ctc, 12_MHz_XTAL / 4);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<0>(12_MHz_XTAL / 8);
	m_ctc->set_clk<1>(12_MHz_XTAL / 8);
	m_ctc->set_clk<2>(50);      // comes from deep in the video section, assumed to be 50Hz
	m_ctc->zc_callback<2>().set(m_ctc, FUNC(z80ctc_device::trg3));

	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);

	K7659_KEYBOARD(config, K7659_KEYBOARD_TAG, 0);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}

/* ROM definition */
ROM_START( llc2 )
	ROM_REGION( 0x6000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "scchmon_91.d35", 0x0000, 0x1000, CRC(218d8236) SHA1(b8297272cc79751afc2eb8688d99b40691346dcb) )
	ROM_LOAD( "gsbasic.bin",    0x4000, 0x2000, CRC(78a5f388) SHA1(e7b475b98dce36b24540ad11eb89046ddb4f02af) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD ("llc2font.d17",   0x0000, 0x0800, CRC(ce53e55d) SHA1(da23d93f14a8a1f8d82bb72470a96b0bfd81ed1b) )
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT       COMPANY  FULLNAME  FLAGS */
COMP( 1984, llc2, llc1,   0,      llc2,    llc2,  llc2_state, init_llc2, "SCCH",  "LLC-2",  MACHINE_SUPPORTS_SAVE )
