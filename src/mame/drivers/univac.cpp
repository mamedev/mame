// license:BSD-3-Clause
// copyright-holders:Robbbert,Vas Crabb
/***************************************************************************

Univac Terminals

2009-05-25 Skeleton driver

The terminals are models UTS20, UTS30, UTS40, UTS50 and SVT1120,
however only the UTS20 is dumped (program roms only).

There were other terminals (Uniscope 100/200/300/400) and UTS60, but
they had different hardware. Uniscope models are believed to use the i8080,
and the UTS60 was a colour graphics terminal with a MC68000 and 2 floppy drives.

The terminal has 2 screens selectable by the operator with the Fn + 1-2
buttons. Thus the user can have two sessions open at once, to different
mainframes or applications. The keyboard connected to the terminal with
a coiled cord and a 9-pin D-connector.

Sound is a beeper.

This driver is all guesswork; Unisys never released technical info
to customers. All parts on the PCBs have internal Unisys part numbers
instead of the manufacturer's numbers.

Notes:
* Port $C6 probably controls serial loopback
  - at a guess, bit 0 enables loopback on both channels
* The NVRAM is 4 bits wide on the LSBs, but (0x81) & 0x10 does something
  - NVRAM nybbles are read/written on the LSBs of 64 ports 0x80 to 0xb4
  - Nybbles are packed/unpacked into 32 bytes starting at 0xd7d7
  - On boot it reads (0x81) & 0x10, and if set preserves 0xd831 to 0xd863
  - This has to be some kind of warm boot detection, but how does it work?

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_GENERAL (1U << 0)
#define LOG_PARITY  (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PARITY)
#include "logmacro.h"

#define LOGPARITY(...)  LOGMASKED(LOG_PARITY, __VA_ARGS__)


class univac_state : public driver_device
{
public:
	univac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_ctc(*this, "ctc")
		, m_uart(*this, "uart")
		, m_beep(*this, "beeper")
		, m_p_chargen(*this, "chargen")
		, m_p_videoram(*this, "videoram")
		, m_p_nvram(*this, "nvram")
		, m_bank_mask(0)
		, m_parity_check(0)
		, m_parity_poison(0)
		, m_framecnt(0)
	{ }

	void uts20(machine_config &config);

private:
	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_READ8_MEMBER(bank_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(nvram_w);

	DECLARE_WRITE8_MEMBER(port43_w);
	DECLARE_WRITE8_MEMBER(portc4_w);
	DECLARE_WRITE8_MEMBER(porte6_w);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_post_load() override;

	required_device<z80_device>     m_maincpu;
	required_device<nvram_device>   m_nvram;
	required_device<z80ctc_device>  m_ctc;
	required_device<z80sio_device>  m_uart;
	required_device<beep_device>    m_beep;

	required_region_ptr<u8> m_p_chargen;
	required_shared_ptr<u8> m_p_videoram;
	required_shared_ptr<u8> m_p_nvram;
	std::unique_ptr<u8 []>  m_p_parity;

	u16 m_bank_mask;
	u8  m_parity_check;
	u8  m_parity_poison;
	u8  m_framecnt;
};



READ8_MEMBER( univac_state::ram_r )
{
	if (BIT(m_p_parity[offset >> 3], offset & 0x07) && !machine().side_effects_disabled())
	{
		LOGPARITY("parity check failed offset = %04X\n", offset);
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
	return m_p_videoram[offset];
}

READ8_MEMBER( univac_state::bank_r )
{
	return space.read_byte((0xc000 | offset) ^ m_bank_mask);
}

WRITE8_MEMBER( univac_state::ram_w )
{
	if (m_parity_poison)
	{
		LOGPARITY("poison parity offset = %04X\n", offset);
		m_p_parity[offset >> 3] |= u8(1) << (offset & 0x07);
	}
	else
	{
		m_p_parity[offset >> 3] &= ~(u8(1) << (offset & 0x07));
	}
	m_p_videoram[offset] = data;
}

WRITE8_MEMBER( univac_state::bank_w )
{
	space.write_byte((0xc000 | offset) ^ m_bank_mask, data);
}

WRITE8_MEMBER( univac_state::nvram_w )
{
	// NVRAM is four bits wide, accessed in the low nybble
	// It's simplest to hack it when writing to make the upper bits read back high on the open bus
	m_p_nvram[offset] = data | 0xf0;
}

WRITE8_MEMBER( univac_state::port43_w )
{
	m_bank_mask = BIT(data, 0) ? 0x2000 : 0x0000;
}

WRITE8_MEMBER( univac_state::portc4_w )
{
	m_parity_poison = BIT(data, 0);
	u8 const check = BIT(data, 1);
	if (check != m_parity_check)
	{
		m_parity_check = check;
		address_space &space(m_maincpu->space(AS_PROGRAM));
		space.unmap_read(0xc000, 0xffff);
		if (check)
		{
			LOGPARITY("parity check enabled\n");
			space.install_read_handler(0xc000, 0xffff, read8_delegate(FUNC(univac_state::ram_r), this));
		}
		else
		{
			LOGPARITY("parity check disabled\n");
			space.install_rom(0xc000, 0xffff, &m_p_videoram[0]);
		}
	}
}

WRITE8_MEMBER( univac_state::porte6_w )
{
	m_beep->set_state(BIT(data, 0));
}


void univac_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x4fff).rom().region("roms", 0);
	map(0x8000, 0xbfff).rw(FUNC(univac_state::bank_r), FUNC(univac_state::bank_w));
	map(0xc000, 0xffff).ram().w(FUNC(univac_state::ram_w)).share("videoram");
}

void univac_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0x00, 0x03).rw(m_uart, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x20, 0x23).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x43, 0x43).w(FUNC(univac_state::port43_w));
	map(0x80, 0xbf).ram().w(FUNC(univac_state::nvram_w)).share("nvram");
	map(0xc4, 0xc4).w(FUNC(univac_state::portc4_w));
	map(0xe6, 0xe6).w(FUNC(univac_state::porte6_w));
}

/* Input ports */
static INPUT_PORTS_START( uts20 )
INPUT_PORTS_END


void univac_state::machine_start()
{
	// D7DC and D7DD are checked for valid RID and SID (usually 21 and 51) if not valid then NVRAM gets initialised.

	std::size_t const parity_bytes = (m_p_videoram.bytes() + 7) / 8;
	m_p_parity.reset(new u8[parity_bytes]);
	std::fill_n(m_p_parity.get(), parity_bytes, 0);

	save_pointer(NAME(m_p_parity), parity_bytes);
	save_item(NAME(m_bank_mask));
	save_item(NAME(m_parity_check));
	save_item(NAME(m_parity_poison));
	save_item(NAME(m_framecnt));
}

void univac_state::machine_reset()
{
	m_beep->set_state(0);

	m_bank_mask = 0x0000;
	m_parity_check = 0;
	m_parity_poison = 0;
}

void univac_state::device_post_load()
{
	if (m_parity_check)
	{
		address_space &space(m_maincpu->space(AS_PROGRAM));
		space.unmap_read(0xc000, 0xffff);
		space.install_read_handler(0xc000, 0xffff, read8_delegate(FUNC(univac_state::ram_r), this));
	}
}

uint32_t univac_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,x,ma=0; //m_bank_mask; (it isn't port43 that selects the screen)

	m_framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				chr = m_p_videoram[x];

				/* Take care of 'corner' characters */
				if (((chr == 0x1c) || (chr == 0x1d)) && (m_framecnt & 16))
					chr = 0x20;

				gfx = (ra ? m_p_chargen[((chr & 0x7f)<<4) | (ra-1) ] : 0) ^ (BIT(chr, 7) ? 0xff : 0);

				/* Display a scanline of a character */
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
		ma+=80;
	}
	return 0;
}

/* F4 Character Displayer */
static const gfx_layout c10_charlayout =
{
	8, 9,                   /* 8 x 9 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_c10 )
	GFXDECODE_ENTRY( "chargen", 0x0000, c10_charlayout, 0, 1 )
GFXDECODE_END

static const z80_daisy_config daisy_chain[] =
{
	{ "uart" },
	{ "ctc" },
	{ nullptr }
};

MACHINE_CONFIG_START(univac_state::uts20)
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(4'000'000)); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &univac_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &univac_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(univac_state, screen_update)
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_c10)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	clock_device &ctc_clock(CLOCK(config, "ctc_clock", 2000000));
	ctc_clock.signal_handler().set(m_ctc, FUNC(z80ctc_device::trg0));
	ctc_clock.signal_handler().append(m_ctc, FUNC(z80ctc_device::trg1));
	ctc_clock.signal_handler().append(m_ctc, FUNC(z80ctc_device::trg2));
	ctc_clock.signal_handler().append(m_ctc, FUNC(z80ctc_device::trg3));

	Z80CTC(config, m_ctc, 4_MHz_XTAL);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<1>().set(m_uart, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().append(m_uart, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<2>().set(m_uart, FUNC(z80sio_device::rxtxcb_w));

	Z80SIO(config, m_uart, 4_MHz_XTAL);
	m_uart->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_uart->out_txda_callback().set(m_uart, FUNC(z80sio_device::rxa_w)); // FIXME: hacked in permanent loopback to pass test
	m_uart->out_txdb_callback().set(m_uart, FUNC(z80sio_device::rxb_w)); // FIXME: hacked in permanent loopback to pass test
	m_uart->out_wrdyb_callback().set(m_uart, FUNC(z80sio_device::dcdb_w)); // FIXME: hacked in permanent loopback to pass test
	m_uart->out_wrdyb_callback().append(m_uart, FUNC(z80sio_device::ctsb_w)); // FIXME: hacked in permanent loopback to pass test

	/* Sound */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("beeper", BEEP, 950) // guess
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.05)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( uts20 )
	ROM_REGION( 0x5000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "uts20a.rom", 0x0000, 0x1000, CRC(1a7b4b4e) SHA1(c3732e25b4b7c7a80172e3fe55c77b923cf511eb) )
	ROM_LOAD( "uts20b.rom", 0x1000, 0x1000, CRC(7f8de87b) SHA1(a85f404ad9d560df831cc3e651a4b45e4ed30130) )
	ROM_LOAD( "uts20c.rom", 0x2000, 0x1000, CRC(4e334705) SHA1(ff1a730551b42f29d20af8ecc4495fd30567d35b) )
	ROM_LOAD( "uts20d.rom", 0x3000, 0x1000, CRC(76757cf7) SHA1(b0509d9a35366b21955f83ec3685163844c4dbf1) )
	ROM_LOAD( "uts20e.rom", 0x4000, 0x1000, CRC(0dfc8062) SHA1(cd681020bfb4829d4cebaf1b5bf618e67b55bda3) )

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD("c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf) )
	// create special unisys gfx
	ROM_FILL(0x1C0, 0x30, 0)
	// left corner
	ROM_FILL(0x1C0, 1, 0xF8)
	ROM_FILL(0x1C1, 1, 0xF0)
	ROM_FILL(0x1C2, 1, 0xE0)
	ROM_FILL(0x1C3, 1, 0xC0)
	ROM_FILL(0x1C4, 1, 0x80)
	// right corner
	ROM_FILL(0x1D0, 1, 0x1F)
	ROM_FILL(0x1D1, 1, 0x0F)
	ROM_FILL(0x1D2, 1, 0x07)
	ROM_FILL(0x1D3, 1, 0x03)
	ROM_FILL(0x1D4, 1, 0x01)
	// SOE
	ROM_FILL(0x1E0, 1, 0x80)
	ROM_FILL(0x1E1, 1, 0xC0)
	ROM_FILL(0x1E2, 1, 0xE0)
	ROM_FILL(0x1E3, 1, 0xF0)
	ROM_FILL(0x1E4, 1, 0xF8)
	ROM_FILL(0x1E5, 1, 0xF0)
	ROM_FILL(0x1E6, 1, 0xE0)
	ROM_FILL(0x1E7, 1, 0xC0)
	ROM_FILL(0x1E8, 1, 0x80)
	// cursor
	ROM_FILL(0x000, 1, 0x7F)
	ROM_FILL(0x001, 1, 0x7E)
	ROM_FILL(0x002, 1, 0x7C)
	ROM_FILL(0x003, 1, 0x79)
	ROM_FILL(0x004, 1, 0x73)
	ROM_FILL(0x005, 1, 0x67)
	ROM_FILL(0x006, 1, 0x4F)
	ROM_FILL(0x007, 1, 0x1F)
	ROM_FILL(0x008, 1, 0x7F)
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY          FULLNAME  FLAGS
COMP( 1980, uts20, 0,      0,      uts20,   uts20, univac_state, empty_init, "Sperry Univac", "UTS-20", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
