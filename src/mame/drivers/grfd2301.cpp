// license:BSD-3-Clause
// copyright-holders:Robbbert
/************************************************************************************************************

Genrad Futuredata 2301 Network Control Processor

2013-08-11 Skeleton

Has a number of plug-in daughter boards, some of which are:
- CPU board: Mostek Z80 CPU, 24MHz crystal, 2 eproms (U2.PB72.2300.4039 1.0 ; U23.N.C.P.2300.4023 3.0)
- IO board: 5.0688MHz crystal, D8253C, 2x S2651
- Memory board: 16k static ram consisting of 32x TMS2147H-7 chips, 1 prom? with sticker U36.2300.4035 1.0
- 2301 board: 2x D8253C, 4x S2651

Back panel has a number of DB25 sockets, labelled thus:
- Station 1-4
- Station 5-8
- EIA 1
- EIA 2
- Printer
- 3 unlabelled ones

A sticker on the back panel says: GenRad, Culver City CA, Model 2301-9001


- No schematic or documents are available. Everything in this driver is a guess.
- Only one rom has been dumped. The best guess would be U23, as this is adjacent to the Z80.
- Although there's no display device, it has display ram. This has been hooked up with a chargen rom
  from another system.

*************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "screen.h"


class grfd2301_state : public driver_device
{
public:
	grfd2301_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_p_chargen(*this, "chargen")
	{ }

	void grfd2301(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void mem_map(address_map &map);

	virtual void machine_reset() override;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_p_chargen;
};

void grfd2301_state::mem_map(address_map &map)
{
	map(0xe000, 0xefff).rom().region("maincpu", 0);
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xffff).ram().share("videoram");
}

void grfd2301_state::io_map(address_map &map)
{
	map.global_mask(0xff);
}

static INPUT_PORTS_START( grfd2301 )
INPUT_PORTS_END

void grfd2301_state::machine_reset()
{
	m_maincpu->set_pc(0xe000);
}

uint32_t grfd2301_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t y,ra,chr,gfx;
	uint16_t sy=0,ma=0,x;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			uint16_t *p = &bitmap.pix16(sy++);

			for (x = 0; x < 80; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[x+ma];

					gfx = m_p_chargen[(chr<<4) | ra ];
				}
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

MACHINE_CONFIG_START(grfd2301_state::grfd2301)
	// basic machine hardware
	MCFG_DEVICE_ADD("maincpu", Z80, 4000000)
	MCFG_DEVICE_PROGRAM_MAP(mem_map)
	MCFG_DEVICE_IO_MAP(io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(grfd2301_state, screen_update)
	MCFG_SCREEN_SIZE(640, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_SCREEN_PALETTE("palette")

	PALETTE(config, "palette", palette_device::MONOCHROME);
MACHINE_CONFIG_END

ROM_START( grfd2301 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "boot2301", 0x000, 0x1000, CRC(feec0cbd) SHA1(ec8138aca7ed489d86aaf2e07225c8d715440db7) )

	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

COMP( 198?, grfd2301, 0, 0, grfd2301, grfd2301, grfd2301_state, empty_init, "Genrad", "Futuredata 2301 Network Processor", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
