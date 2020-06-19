// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/******************************************************************************

        LLC driver by Miodrag Milanovic

        17/04/2009 Preliminary driver.

        July 2012, updates by Robbbert

        Very little info available on these computers.

        LLC2:
        The BEL character plays a short tune.
        In the monitor, it is case sensitive, most commands are uppercase,
        but some are lowercase.
        To start Basic, the command is b. To quit basic, use BYE.
        Inside Basic, it is not case-sensitive.

        ToDo:
        - LLC2: Keyboard is incomplete
        - Lots of other things

*******************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/k7659kb.h"
#include "machine/ram.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


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
	{ }

	void llc2(machine_config &config);

	void init_llc2();

private:
	void machine_start() override;
	void machine_reset() override;
	void llc2_rom_disable_w(u8 data);
	void llc2_basic_enable_w(u8 data);
	u8 llc2_port1_b_r();
	u8 llc2_port2_a_r();
	void llc2_port1_b_w(u8 data);
	u32 screen_update_llc2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	bool m_rv;
	optional_device<speaker_sound_device> m_speaker;
	optional_shared_ptr<u8> m_vram;
	required_device<z80_device> m_maincpu;
	optional_device<ram_device> m_ram;
	required_region_ptr<u8> m_p_chargen;
};

/* Address maps */
void llc2_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0x5fff).bankrw("bank2");
	map(0x6000, 0xbfff).bankrw("bank3");
	map(0xc000, 0xffff).bankrw("bank4");
}

void llc2_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xE0, 0xE3).w(FUNC(llc2_state::llc2_rom_disable_w));
	map(0xE4, 0xE7).rw("z80pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xE8, 0xEB).rw("z80pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0xEC, 0xEC).w(FUNC(llc2_state::llc2_basic_enable_w));
	map(0xF8, 0xFB).rw("z80ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

/* Input ports */
static INPUT_PORTS_START( llc2 )
INPUT_PORTS_END


/* Driver initialization */
void llc2_state::init_llc2()
{
	m_vram.set_target( m_ram->pointer() + 0xc000,m_vram.bytes());
}

void llc2_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	u8 *ram = m_ram->pointer();

	space.unmap_write(0x0000, 0x3fff);
	membank("bank1")->set_base(memregion("maincpu")->base());

	space.unmap_write(0x4000, 0x5fff);
	membank("bank2")->set_base(ram + 0x4000);

	space.unmap_write(0x6000, 0xbfff);
	membank("bank3")->set_base(ram + 0x6000);

	space.install_write_bank(0xc000, 0xffff, "bank4");
	membank("bank4")->set_base(ram + 0xc000);
}

void llc2_state::llc2_rom_disable_w(u8 data)
{
	address_space &mem_space = m_maincpu->space(AS_PROGRAM);
	u8 *ram = m_ram->pointer();

	mem_space.install_write_bank(0x0000, 0xbfff, "bank1");
	membank("bank1")->set_base(ram);

	mem_space.install_write_bank(0x4000, 0x5fff, "bank2");
	membank("bank2")->set_base(ram + 0x4000);

	mem_space.install_write_bank(0x6000, 0xbfff, "bank3");
	membank("bank3")->set_base(ram + 0x6000);

	mem_space.install_write_bank(0xc000, 0xffff, "bank4");
	membank("bank4")->set_base(ram + 0xc000);

}

void llc2_state::llc2_basic_enable_w(u8 data)
{
	address_space &mem_space = m_maincpu->space(AS_PROGRAM);
	if (BIT(data, 1))
	{
		mem_space.unmap_write(0x4000, 0x5fff);
		membank("bank2")->set_base(memregion("maincpu")->base() + 0x4000);
	}
	else
	{
		mem_space.install_write_bank(0x4000, 0x5fff, "bank2");
		membank("bank2")->set_base(m_ram->pointer() + 0x4000);
	}

}

u8 llc2_state::llc2_port1_b_r()
{
	return 0;
}

void llc2_state::llc2_port1_b_w(u8 data)
{
	m_speaker->level_w(BIT(data, 6));
	m_rv = BIT(data, 5);
}

u8 llc2_state::llc2_port2_a_r()
{
	return 0; // bit 2 low or hangs on ^Z^X^C sequence
}

void llc2_state::machine_start()
{
	save_item(NAME(m_rv));
}

u32 llc2_state::screen_update_llc2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 y,ra,chr,gfx,inv, inv1=m_rv ? 0xff : 0;
	u16 sy=0,ma=0,x;

	for (y = 0; y < 32; y++)
	{
		for (ra = 0; ra < 8; ra++)
		{
			inv = 0;
			u16 *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 64; x++)
			{
				chr = m_vram[x];
				if (chr==0x11) // inverse on
				{
					inv=0xff;
					chr=0x0f; // must not show
				}
				else
				if (chr==0x10) // inverse off
					inv=0;

				/* get pattern of pixels for that character scanline */
				gfx = m_p_chargen[ (chr << 3) | ra ] ^ inv ^ inv1;

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

static const z80_daisy_config llc2_daisy_chain[] =
{
	{ "z80pio1" },
	{ "z80ctc" },
	{ nullptr }
};

/* F4 Character Displayer */
static const gfx_layout llc2_charlayout =
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
	GFXDECODE_ENTRY( "chargen", 0x0000, llc2_charlayout, 0, 1 )
GFXDECODE_END

/* Machine driver */
void llc2_state::llc2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(3'000'000));
	m_maincpu->set_daisy_config(llc2_daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &llc2_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &llc2_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(llc2_state::screen_update_llc2));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_llc2);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.15);

	z80pio_device& pio1(Z80PIO(config, "z80pio1", XTAL(3'000'000)));
	pio1.in_pa_callback().set(K7659_KEYBOARD_TAG, FUNC(k7659_keyboard_device::read));
	pio1.in_pb_callback().set(FUNC(llc2_state::llc2_port1_b_r));
	pio1.out_pb_callback().set(FUNC(llc2_state::llc2_port1_b_w));

	z80pio_device& pio2(Z80PIO(config, "z80pio2", XTAL(3'000'000)));
	pio2.in_pa_callback().set(FUNC(llc2_state::llc2_port2_a_r));

	Z80CTC(config, "z80ctc", XTAL(3'000'000));

	K7659_KEYBOARD(config, K7659_KEYBOARD_TAG, 0);

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("64K");
}

/* ROM definition */
ROM_START( llc2 )
	ROM_REGION( 0x6000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "scchmon_91.bin", 0x0000, 0x1000, CRC(218d8236) SHA1(b8297272cc79751afc2eb8688d99b40691346dcb) )
	ROM_LOAD( "gsbasic.bin",    0x4000, 0x2000, CRC(78a5f388) SHA1(e7b475b98dce36b24540ad11eb89046ddb4f02af) )

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD ("llc2font.bin",   0x0000, 0x0800, CRC(ce53e55d) SHA1(da23d93f14a8a1f8d82bb72470a96b0bfd81ed1b) )
ROM_END


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT       COMPANY  FULLNAME  FLAGS */
COMP( 1984, llc2, llc1,   0,      llc2,    llc2,  llc2_state, init_llc2, "SCCH",  "LLC-2",  MACHINE_SUPPORTS_SAVE )
