// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Amstrad PenPad PDA 600

        05/11/2009 Skeleton driver.

    Hardware info:
    U1: CPU
        Z8S180
        Enhanced Z80 CPU
        1 MB MMU
        2 DMAs
        2 UARTs (up to 512 Kbps)
        Two 16-Bit Timers
        Clock Serial I/O

    U2: ROM
        contains OS (binary and disassembled source)
        apparently compatible to 27C1001 (128K*8)

    U3: RAM
        128K RAM, static
    U4: RTC
    U5: Amstrad ASIC (unknown functionality )
    U6, U7, U8, U9: Multiplexer
        quad channel dual multiplexer
    U11: RAM
        32K RAM, static
    U12: LCD Controller
    U21: 5V StepUp converter voltage converter
    U27: OA quad operational amplifier
    U30: RS232 Driver
    U31: Amstrad ASIC
    U32: FlipFlop Octal D-Flip-Flop
    U33: RAM
        32K RAM, static
    U35: Tone Dialer
        generates DTMF frequencies
    U36: unknown
    U37: Nand
        Dual Quad-Input Nand
    U43: Bus Buffer
        Octal Bus Buffer

    Connectors:
        LCD
        Digitizer
        Serial
        PCMCIA

    Additional info:
        Two Z8 for power management and character recognition

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/nvram.h"
#include "machine/hd64610.h"
#include "rendlay.h"


class pda600_state : public driver_device
{
public:
	pda600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
		{}

	required_device<cpu_device> m_maincpu;

	virtual void video_start();
	virtual void machine_reset();
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 *     m_video_ram;
};


static ADDRESS_MAP_START(pda600_mem, AS_PROGRAM, 8, pda600_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_ROM
	//AM_RANGE(0x20000, 0x9ffff) AM_RAM // PCMCIA Card
	AM_RANGE(0xa0000, 0xa7fff) AM_RAM   AM_REGION("videoram", 0)
	AM_RANGE(0xe0000, 0xfffff) AM_RAM   AM_REGION("mainram", 0)     AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(pda600_io, AS_IO, 8, pda600_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x3f) AM_NOP /* Z180 internal registers */
	//AM_RANGE(0x40, 0x7f) AM_NOP   /* Z180 internal registers */
	AM_RANGE(0x80, 0x8f) AM_DEVREADWRITE("rtc", hd64610_device, read, write)
	//AM_RANGE(0xC0, 0xC1) AM_NOP   /* LCD */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( pda600 )
INPUT_PORTS_END


void pda600_state::machine_reset()
{
	// the PDA600 soon after start waits for something from the Z180 CSIO, I do not know exactly for what
	// the CSIO is used and for now I forced the CNTR End-Flag bit to 1 for allow the emulation to continue.
	m_maincpu->set_state_int(Z180_CNTR, m_maincpu->state_int(Z180_CNTR) | 0x80);
}

void pda600_state::video_start()
{
	m_video_ram = memregion("videoram")->base();
}

UINT32 pda600_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y=0; y<320; y++)
		for (int x=0; x<30; x++)
		{
			UINT8 data = m_video_ram[y*30 + x];

			for (int px=0; px<8; px++)
			{
				bitmap.pix16(y, (x * 8) + px) = BIT(data, 7);
				data <<= 1;
			}
		}

	return 0;
}

static const gfx_layout pda600_charlayout_8 =
{
	8, 8,
	49,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout pda600_charlayout_13 =
{
	8, 13,
	123,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12 },
	8*13
};

static const gfx_layout pda600_charlayout_13a =
{
	8, 13,
	132,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12 },
	8*13
};

static const gfx_layout pda600_charlayout_19 =
{
	8, 19,
	32,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12, 8*13, 8*14, 8*15, 8*16, 8*17, 8*18 },
	8*19
};

static const gfx_layout pda600_charlayout_19a =
{
	8, 19,
	11,
	1,
	{ 0 },
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 8*9, 8*10, 8*11, 8*12, 8*13, 8*14, 8*15, 8*16, 8*17, 8*18 },
	8*19
};

static GFXDECODE_START( pda600 )
	GFXDECODE_ENTRY( "maincpu", 0x45cd, pda600_charlayout_19, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x4892, pda600_charlayout_19a, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x4d73, pda600_charlayout_8, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x5b8f, pda600_charlayout_13, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x61d3, pda600_charlayout_13a, 0, 1 )
GFXDECODE_END


static MACHINE_CONFIG_START( pda600, pda600_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z180, XTAL_14_31818MHz)
	MCFG_CPU_PROGRAM_MAP(pda600_mem)
	MCFG_CPU_IO_MAP(pda600_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(240, 320)
	MCFG_SCREEN_VISIBLE_AREA(0, 240-1, 0, 320-1)
	MCFG_SCREEN_UPDATE_DRIVER( pda600_state, screen_update )
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pda600)
	MCFG_DEFAULT_LAYOUT(layout_lcd)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	// NVRAM needs to be filled with random data to fail the checksum and be initialized correctly
	MCFG_NVRAM_ADD_RANDOM_FILL("nvram")

	MCFG_DEVICE_ADD("rtc", HD64610, XTAL_32_768kHz)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pda600 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pdarom.bin", 0x00000, 0x20000, CRC(f793a6c5) SHA1(ab14b0fdcedb927c66357368a2bfff605ba758fb))

	// 128KB RAM
	ROM_REGION( 0x20000, "mainram", ROMREGION_ERASE )

	// 32KB Video RAM
	ROM_REGION( 0x8000, "videoram", ROMREGION_ERASE )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY        FULLNAME       FLAGS */
COMP( 1993, pda600,  0,       0,     pda600,    pda600, driver_device,   0,  "Amstrad plc", "PenPad PDA 600", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
