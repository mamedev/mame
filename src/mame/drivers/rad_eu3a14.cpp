// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*
	These use a 6502 derived CPU under a glob
	The CPU die is marked 'ELAN EU3A14'

	There is a second glob surrounded by TSOP48 pads
	this contains the ROM

	Known to be on this hardware

	Golden Tee Golf Home Edition (developed by FarSight Studios)

	Maybe on this hardware

	PlayTV Real Swing Golf (also developed by FarSight, looks similar but with different controls)

*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"

/*

ffb0  rti
ffb4  rti
ffb8  rti
ffbc  rti

ffc0  rti
ffc4  rti
ffc8  rti
ffcc  rti

ffd0  rti
ffd4  main irq?
ffd8  rti
ffdc  rti

ffe0  something with 5045 bit 0x08 and 9d in ram (increase or decrease)
ffe4  something with 5045 bit 0x20 and 9c in ram (increase of decrease)

ffe8  rti
ffec  rti

regular NMI (e3f0 - jump to ($19e2) which seems to point to rti, but could move..)
regular IRQ (e3f3 - points to rti)

*/


class radica_eu3a14_state : public driver_device
{
public:
	radica_eu3a14_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_unkram(*this, "unkram")
	{ }

	READ8_MEMBER(irq_vector_r);

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	
	void radica_eu3a14(machine_config &config);

	int m_custom_irq;
	uint16_t m_custom_irq_vector;

	INTERRUPT_GEN_MEMBER(interrupt);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_unkram;
};


void radica_eu3a14_state::video_start()
{
}

uint32_t radica_eu3a14_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	return 0;
}

// irq controller seems to be like the Radica Space Invaders
READ8_MEMBER(radica_eu3a14_state::irq_vector_r)
{
	if (m_custom_irq)
	{
		return m_custom_irq_vector >> (offset*8);
	}
	else
	{
		uint8_t *rom = memregion("maincpu")->base();
		return rom[0x001ffe + offset];
	}
}

/* 
   code at 0000 maps to e000
   code at 1000 maps to f000

   data at 2000
   data at 3000
   data at 4000
   blank   5000
   blank   6000

   code at 7000 maps to 3000
   code at 8000 maps to 6000
           9000 maps to 7000
           a000 maps to 8000
           b000 maps to 9000
           c000 maps to a000
           d000 maps to b000
           e000 maps to c000
*/
static ADDRESS_MAP_START( radica_eu3a14_map, AS_PROGRAM, 8, radica_eu3a14_state )
	AM_RANGE(0x0000, 0x01ff) AM_RAM
	AM_RANGE(0x0200, 0x1fff) AM_RAM

	AM_RANGE(0x3000, 0x3fff) AM_RAM AM_SHARE("unkram") 

	AM_RANGE(0x4800, 0x4bff) AM_RAM

//	AM_RANGE(0x5000, 0x51ff) AM_RAM

	AM_RANGE(0x6000, 0xdfff) AM_ROM AM_REGION("maincpu", 0x8000)

	AM_RANGE(0xfffe, 0xffff) AM_READ(irq_vector_r)

	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0x0000)
ADDRESS_MAP_END


static INPUT_PORTS_START( radica_eu3a14 )
INPUT_PORTS_END

void radica_eu3a14_state::machine_start()
{
}

void radica_eu3a14_state::machine_reset()
{
	// rather be safe
	m_maincpu->set_state_int(M6502_S, 0x1ff);

	uint8_t *rom = memregion("maincpu")->base();
	// should probably just be a DMA
	for (int i = 0; i < 0x1000; i++)
	{
		m_unkram[i] = rom[0x7000+i];
	}

}

INTERRUPT_GEN_MEMBER(radica_eu3a14_state::interrupt)
{
	m_custom_irq = 1;
	m_custom_irq_vector = 0xffd4;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);
}



MACHINE_CONFIG_START(radica_eu3a14_state::radica_eu3a14)
	/* basic machine hardware */	
	MCFG_CPU_ADD("maincpu",M6502,XTAL(21'477'272)/2) // marked as 21'477'270
	MCFG_CPU_PROGRAM_MAP(radica_eu3a14_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", radica_eu3a14_state,  interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(radica_eu3a14_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
MACHINE_CONFIG_END

ROM_START( rad_gtg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "goldentee.bin", 0x000000, 0x400000, CRC(b1985c63) SHA1(c42a59fcb665eb801d9ca5312b90e39333e52de4) )
ROM_END

CONS( 2006, rad_gtg,  0,   0,  radica_eu3a14,  radica_eu3a14, radica_eu3a14_state, 0, "Radica (licensed from Incredible Technologies)", "Golden Tee Golf: Home Edition", MACHINE_IS_SKELETON )
