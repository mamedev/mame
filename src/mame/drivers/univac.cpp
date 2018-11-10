// license:BSD-3-Clause
// copyright-holders:Robbbert,Vas Crabb
/***************************************************************************

Univac Terminals

2009-05-25 Skeleton driver

The terminals are models UTS10, UTS20, UTS30, UTS40, UTS50 and SVT1120.

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

You can use a debug trick to get UTS10 to boot:
- When it stops @0157 halt, pc = 158 and g
- When it loops at @0B33, pc = B35 and g

2018-11-10 Info from AL: cpu clock 3.072MHz; hsync 22.74KHz; vsync 60Hz

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/clock.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "sound/beep.h"
#include "video/dp8350.h"
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
	uint8_t y,ra,chr;
	uint16_t sy=0,x,ma=0; //m_bank_mask; (it isn't port43 that selects the screen)

	m_framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 14; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				chr = m_p_videoram[x];    // bit 7 = rv attribute (or dim, depending on control-page setting)

				uint16_t gfx = m_p_chargen[((chr & 0x7f)<<4) | ra] ^ (BIT(chr, 7) ? 0x1ff : 0);

				// chars 1C, 1D, 1F need special handling
				if ((chr >= 0x1c) && (chr <= 0x1f) && BIT(gfx, 7))
				{
					gfx &= 0x7f;
				// They also blink
					if (m_framecnt & 16)
						gfx = 0;
				}

				/* Display a scanline of a character */
				*p++ = BIT(gfx, 8);
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
static const gfx_layout uts_charlayout =
{
	8, 14,                   /* 8 x 14 characters */
	128,                    /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( gfx_uts )
	GFXDECODE_ENTRY( "chargen", 0x0000, uts_charlayout, 0, 1 )
GFXDECODE_END

static const z80_daisy_config daisy_chain[] =
{
	{ "uart" },
	{ "ctc" },
	{ nullptr }
};

MACHINE_CONFIG_START(univac_state::uts20)
	/* basic machine hardware */
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 6); // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &univac_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &univac_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_UPDATE_DRIVER(univac_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME("palette")
	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_uts)

	dp835x_device &crtc(DP835X_A(config, "crtc", 19'980'000));
	crtc.set_screen("screen");
	crtc.vblank_callback().set(m_ctc, FUNC(z80ctc_device::trg0));
	crtc.vblank_callback().append(m_ctc, FUNC(z80ctc_device::trg3));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	Z80CTC(config, m_ctc, 18.432_MHz_XTAL / 6);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->set_clk<1>(18.432_MHz_XTAL / 12);
	m_ctc->set_clk<2>(18.432_MHz_XTAL / 12);
	m_ctc->zc_callback<1>().set(m_uart, FUNC(z80sio_device::txca_w));
	m_ctc->zc_callback<1>().append(m_uart, FUNC(z80sio_device::rxca_w));
	m_ctc->zc_callback<2>().set(m_uart, FUNC(z80sio_device::rxtxcb_w));

	Z80SIO(config, m_uart, 18.432_MHz_XTAL / 6);
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
ROM_START( uts10 )
	ROM_REGION( 0x5000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "f3577_1.bin",  0x0000, 0x0800, CRC(f7d47484) SHA1(84c01d054df19e8da44c242a67d97f643bdabc4c) )
	ROM_LOAD( "f3577_2.bin",  0x0800, 0x0800, CRC(7c1045f0) SHA1(732e8c111a346476c59bcfda73f0f826cdcd7eb3) )
	ROM_LOAD( "f3577_3.bin",  0x1000, 0x0800, CRC(10f47af2) SHA1(a61b693af264bfa6565c43b4fe473833f8aba046) )
	ROM_LOAD( "f3577_4.bin",  0x1800, 0x0800, CRC(bed8924c) SHA1(1fe3e118cc1c17f4c8b9c0025257822b99fcde38) )
	ROM_LOAD( "f3577_5.bin",  0x2000, 0x0800, CRC(38d671b5) SHA1(3fb3feaaddb08af5ba50a9c08511cbb3949a7985) )
	ROM_LOAD( "f3577_6.bin",  0x2800, 0x0800, CRC(6dbe9c4a) SHA1(11bc4b7c99811bd26423a15b33d02a86fa0bfd17) )

	ROM_REGION( 0x0800, "chargen", 0 ) // possibly some bitrot, see h,m,n in F4 displayer
	ROM_LOAD( "chr_5565.bin", 0x0000, 0x0800, CRC(7d99744f) SHA1(2db330ca94a91f7b2ac2ac088ae9255f5bb0a7b4) )
ROM_END

ROM_START( uts20 )
	ROM_REGION( 0x5000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "uts20a.rom", 0x0000, 0x1000, CRC(1a7b4b4e) SHA1(c3732e25b4b7c7a80172e3fe55c77b923cf511eb) )
	ROM_LOAD( "uts20b.rom", 0x1000, 0x1000, CRC(7f8de87b) SHA1(a85f404ad9d560df831cc3e651a4b45e4ed30130) )
	ROM_LOAD( "uts20c.rom", 0x2000, 0x1000, CRC(4e334705) SHA1(ff1a730551b42f29d20af8ecc4495fd30567d35b) )
	ROM_LOAD( "uts20d.rom", 0x3000, 0x1000, CRC(76757cf7) SHA1(b0509d9a35366b21955f83ec3685163844c4dbf1) )
	ROM_LOAD( "uts20e.rom", 0x4000, 0x1000, CRC(0dfc8062) SHA1(cd681020bfb4829d4cebaf1b5bf618e67b55bda3) )

	/* character generator not dumped, using the one from 'UTS10' for now */
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "chr_5565.bin", 0x0000, 0x0800, BAD_DUMP CRC(7d99744f) SHA1(2db330ca94a91f7b2ac2ac088ae9255f5bb0a7b4) )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY          FULLNAME  FLAGS
COMP( 1979?, uts10, uts20,  0,      uts20,   uts20, univac_state, empty_init, "Sperry Univac", "UTS-10", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1980, uts20, 0,      0,      uts20,   uts20, univac_state, empty_init, "Sperry Univac", "UTS-20", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
