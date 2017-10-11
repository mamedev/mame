// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Univac Terminals

    The terminals are models UTS20, UTS30, UTS40, UTS50 and SVT1120,
    however only the UTS20 is dumped (program roms only).

    25/05/2009 Skeleton driver

    The terminal has 2 screens selectable by the operator with the Fn + 1-2
    buttons. Thus the user can have two sessions open at once, to different
    mainframes or applications.

    Sound is a beeper.

    This driver is all guesswork; Unisys never released technical info
    to customers. All parts on the PCBs have internal Unisys part numbers
    instead of the manufacturer's numbers.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/clock.h"
#include "screen.h"


class univac_state : public driver_device
{
public:
	univac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_nvram(*this, "nvram")
		, m_ctc(*this, "ctc")
		, m_dart(*this, "dart")
		, m_p_chargen(*this, "chargen")
	{ }

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(port43_w);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

private:
	bool m_screen_num;
	uint8_t m_framecnt;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_device<nvram_device> m_nvram;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_region_ptr<u8> m_p_chargen;
};



WRITE8_MEMBER( univac_state::port43_w )
{
	m_screen_num = BIT(data, 0);
}

READ8_MEMBER( univac_state::vram_r )
{
	offs_t offs = (offset & 0x1fff) ^ (BIT(offset, 13) ? 0x2000 : 0) ^ (m_screen_num ? 0x2000 : 0);
	return m_p_videoram[offs];
}

WRITE8_MEMBER( univac_state::vram_w )
{
	offs_t offs = (offset & 0x1fff) ^ (BIT(offset, 13) ? 0x2000 : 0) ^ (m_screen_num ? 0x2000 : 0);
	m_p_videoram[offs] = data;
}


static ADDRESS_MAP_START(uts20_mem, AS_PROGRAM, 8, univac_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x4fff ) AM_ROM AM_REGION("roms", 0)
	AM_RANGE( 0x8000, 0xbfff ) AM_READWRITE(vram_r,vram_w)
	AM_RANGE( 0xc000, 0xffff ) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( uts20_io, AS_IO, 8, univac_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("dart", z80dart_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x43, 0x43) AM_WRITE(port43_w)
	AM_RANGE(0x80, 0xbf) AM_RAM AM_SHARE("nvram")
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( uts20 )
INPUT_PORTS_END


void univac_state::machine_start()
{
// D7DC and D7DD are checked for valid RID and SID (usually 21 and 51) if not valid then NVRAM gets initialised.
}

void univac_state::machine_reset()
{
	m_screen_num = 0;
}

uint32_t univac_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// this is used to get the ctc to pass the test
	bool state = BIT(m_framecnt,0);
	m_ctc->trg0(state);
	m_ctc->trg1(state);
	m_ctc->trg2(state);
	m_ctc->trg3(state);

	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;
	uint8_t *videoram = m_p_videoram;//+(m_screen_num ? 0x2000 : 0);

	m_framecnt++;

	for (y = 0; y < 25; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = ma; x < ma + 80; x++)
			{
				chr = videoram[x];

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

static const z80_daisy_config daisy_chain[] =
{
	{ "dart" },
	{ "ctc" },
	{ nullptr }
};

static MACHINE_CONFIG_START( uts20 )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz) // unknown clock
	MCFG_CPU_PROGRAM_MAP(uts20_mem)
	MCFG_CPU_IO_MAP(uts20_io)
	MCFG_Z80_DAISY_CHAIN(daisy_chain)

	/* video hardware */
	MCFG_SCREEN_ADD_MONOCHROME("screen", RASTER, rgb_t::green())
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(univac_state, screen_update)
	MCFG_SCREEN_SIZE(640, 250)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 249)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_NVRAM_ADD_1FILL("nvram")

	MCFG_DEVICE_ADD("uart_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("dart", z80dart_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("dart", z80dart_device, rxca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("dart", z80dart_device, txcb_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("dart", z80dart_device, rxcb_w))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_4MHz)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_DEVICE_ADD("dart", Z80DART, XTAL_4MHz)
	MCFG_Z80DART_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( uts20 )
	ROM_REGION( 0x5000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "uts20a.rom", 0x0000, 0x1000, CRC(1a7b4b4e) SHA1(c3732e25b4b7c7a80172e3fe55c77b923cf511eb) )
	ROM_LOAD( "uts20b.rom", 0x1000, 0x1000, CRC(7f8de87b) SHA1(a85f404ad9d560df831cc3e651a4b45e4ed30130) )
	ROM_LOAD( "uts20c.rom", 0x2000, 0x1000, CRC(4e334705) SHA1(ff1a730551b42f29d20af8ecc4495fd30567d35b) )
	ROM_LOAD( "uts20d.rom", 0x3000, 0x1000, CRC(76757cf7) SHA1(b0509d9a35366b21955f83ec3685163844c4dbf1) )
	ROM_LOAD( "uts20e.rom", 0x4000, 0x1000, CRC(0dfc8062) SHA1(cd681020bfb4829d4cebaf1b5bf618e67b55bda3) )
	ROM_FILL(0x2bd,1,0xaf) // test 2
	ROM_FILL(0x3c1,2,0xaf) // test 5
	ROM_FILL(0x928,1,0x00)

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD("c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf) )
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY            FULLNAME  FLAGS
COMP( 1980, uts20,  0,      0,       uts20,     uts20, univac_state,   0,      "Sperry Univac",   "UTS-20", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
