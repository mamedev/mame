// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Univac Terminals

The terminals are models UTS20, UTS30, UTS40, UTS50 and SVT1120,
however only the UTS20 is dumped (program roms only).

2009-05-25 Skeleton driver

The terminal has 2 screens selectable by the operator with the Fn + 1-2
buttons. Thus the user can have two sessions open at once, to different
mainframes or applications. The keyboard connected to the terminal with
a coiled cord and a 9-pin D-connector.

Sound is a beeper.

This driver is all guesswork; Unisys never released technical info
to customers. All parts on the PCBs have internal Unisys part numbers
instead of the manufacturer's numbers.

Notes on failures in power-on test:
* Test 2 checks RAM parity. Patched out
* Test 5 tests ROM checksum. It seems one or more roms is a bad dump.
* Test 7 tests SIO ch B. The SIO should cause 9 interrupts. The CTC is
  programmed as a time-out. But SIO only does first interrupt then hangs.
  CTC then fires causing the failure message. It's unknown which CTC
  channel drives Ch B, none seem to work. CTC trigger frequencies are
  all guesswork too, but the present setup allows Tests 3 and 6 to pass.


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"
#include "machine/clock.h"
#include "screen.h"
#include "sound/beep.h"
#include "speaker.h"


class univac_state : public driver_device
{
public:
	univac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_videoram(*this, "videoram")
		, m_nvram(*this, "nvram")
		, m_ctc(*this, "ctc")
		, m_uart(*this, "uart")
		, m_p_chargen(*this, "chargen")
		, m_beep(*this, "beeper")
	{ }

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_WRITE8_MEMBER(port43_w);
	DECLARE_WRITE8_MEMBER(porte6_w);
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
	required_device<z80sio_device> m_uart;
	required_region_ptr<u8> m_p_chargen;
	required_device<beep_device> m_beep;
};



WRITE8_MEMBER( univac_state::porte6_w )
{
	m_beep->set_state(BIT(data, 0));
}

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
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("uart", z80sio_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("ctc", z80ctc_device, read, write)
	AM_RANGE(0x43, 0x43) AM_WRITE(port43_w)
	AM_RANGE(0x80, 0xbf) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xe6, 0xe6) AM_WRITE(porte6_w)
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
	m_beep->set_state(0);
}

uint32_t univac_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
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
	{ "uart" },
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

	MCFG_DEVICE_ADD("ctc_clock", CLOCK, 2000000)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("ctc", z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg2))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("ctc", z80ctc_device, trg3))

	//MCFG_DEVICE_ADD("uart_clock", CLOCK, 307200)
	//MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart", z80sio_device, txcb_w))
	//MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart", z80sio_device, rxcb_w))

	MCFG_DEVICE_ADD("ctc", Z80CTC, XTAL_4MHz)
	MCFG_Z80CTC_INTR_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80CTC_ZC1_CB(DEVWRITELINE("uart", z80sio_device, txca_w))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart", z80sio_device, rxca_w))
	MCFG_Z80CTC_ZC2_CB(DEVWRITELINE("uart", z80sio_device, rxtxcb_w))

	MCFG_DEVICE_ADD("uart", Z80SIO, XTAL_4MHz)
	MCFG_Z80SIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	/* Sound */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 950) // guess
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( uts20 )
	ROM_REGION( 0x5000, "roms", ROMREGION_ERASEFF )
	ROM_LOAD( "uts20a.rom", 0x0000, 0x1000, CRC(1a7b4b4e) SHA1(c3732e25b4b7c7a80172e3fe55c77b923cf511eb) )
	ROM_LOAD( "uts20b.rom", 0x1000, 0x1000, CRC(7f8de87b) SHA1(a85f404ad9d560df831cc3e651a4b45e4ed30130) )
	ROM_LOAD( "uts20c.rom", 0x2000, 0x1000, CRC(4e334705) SHA1(ff1a730551b42f29d20af8ecc4495fd30567d35b) )
	ROM_LOAD( "uts20d.rom", 0x3000, 0x1000, CRC(76757cf7) SHA1(b0509d9a35366b21955f83ec3685163844c4dbf1) )
	ROM_LOAD( "uts20e.rom", 0x4000, 0x1000, CRC(0dfc8062) SHA1(cd681020bfb4829d4cebaf1b5bf618e67b55bda3) )
	// Patches, see notes at top.
	ROM_FILL(0x2bd,1,0xaf) // test 2
	ROM_FILL(0x48f1,1,0xa3) // test 5, patch unused byte to fix checksum
	ROM_FILL(0x928,1,0x00) // upon failure keep going

	/* character generator not dumped, using the one from 'c10' for now */
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD("c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf) )
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY            FULLNAME  FLAGS
COMP( 1980, uts20,  0,      0,       uts20,     uts20, univac_state,   0,      "Sperry Univac",   "UTS-20", MACHINE_NOT_WORKING )
