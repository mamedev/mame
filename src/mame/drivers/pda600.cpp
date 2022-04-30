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
#include "emupal.h"
#include "screen.h"


class pda600_state : public driver_device
{
public:
	pda600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_video_ram(*this, "videoram")
	{
	}

	void pda600(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_video_ram;

	virtual void video_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pda600_io(address_map &map);
	void pda600_mem(address_map &map);
};


void pda600_state::pda600_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000, 0x1ffff).rom();
	//map(0x20000, 0x9ffff).ram(); // PCMCIA Card
	map(0xa0000, 0xa7fff).ram().share("videoram");
	map(0xe0000, 0xfffff).ram().share("nvram");
}

void pda600_state::pda600_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x3f).noprw(); /* Z180 internal registers */
	//map(0x40, 0x7f).noprw();   /* Z180 internal registers */
	map(0x80, 0x8f).rw("rtc", FUNC(hd64610_device::read), FUNC(hd64610_device::write));
	//map(0xc0, 0xc1).noprw();  /* LCD */
}

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
}

uint32_t pda600_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y=0; y<320; y++)
		for (int x=0; x<30; x++)
		{
			uint8_t data = m_video_ram[y*30 + x];

			for (int px=0; px<8; px++)
			{
				bitmap.pix(y, (x * 8) + px) = BIT(data, 7);
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

static GFXDECODE_START( gfx_pda600 )
	GFXDECODE_ENTRY( "maincpu", 0x45cd, pda600_charlayout_19, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x4892, pda600_charlayout_19a, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x4d73, pda600_charlayout_8, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x5b8f, pda600_charlayout_13, 0, 1 )
	GFXDECODE_ENTRY( "maincpu", 0x61d3, pda600_charlayout_13a, 0, 1 )
GFXDECODE_END


void pda600_state::pda600(machine_config &config)
{
	/* basic machine hardware */
	Z8S180(config, m_maincpu, XTAL(14'318'181));
	m_maincpu->set_addrmap(AS_PROGRAM, &pda600_state::pda600_mem);
	m_maincpu->set_addrmap(AS_IO, &pda600_state::pda600_io);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(240, 320);
	screen.set_visarea(0, 240-1, 0, 320-1);
	screen.set_screen_update(FUNC(pda600_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_pda600);
	PALETTE(config, "palette", palette_device::MONOCHROME);

	// NVRAM needs to be filled with random data to fail the checksum and be initialized correctly
	NVRAM(config, "nvram", nvram_device::DEFAULT_RANDOM);

	HD64610(config, "rtc", XTAL(32'768));
}

/* ROM definition */
ROM_START( pda600 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "pdarom.bin", 0x00000, 0x20000, CRC(f793a6c5) SHA1(ab14b0fdcedb927c66357368a2bfff605ba758fb))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY        FULLNAME          FLAGS */
COMP( 1993, pda600, 0,      0,      pda600,  pda600, pda600_state, empty_init, "Amstrad plc", "PenPad PDA 600", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
