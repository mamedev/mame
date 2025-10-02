// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Cupidon - Russian Video Fruit Machines? */

/*
 seems to be Kupidon in the ROMs?

 these act a bit like the pluto5 ones but with a video system, possibly a variant on that?

 needs 68340 peripherals (irq controller + timer at least) to be fleshed out.

 video might be vga-like?
*/


#include "emu.h"
#include "machine/68340.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cupidon_state : public driver_device
{
public:
	cupidon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_gfxram(*this, "gfxram")
	{ }

	void init_cupidon();
	void init_funnyfm();

	void cupidon(machine_config &config);
	void cupidon_map(address_map &map) ATTR_COLD;

protected:
	// devices
	required_device<m68340_cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_gfxram;

	uint32_t screen_update_cupidon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t cupidon_return_ffffffff()
	{
		return -1; // or it hits an illegal opcode (sleep on the 68340?)
	}
};

uint32_t cupidon_state::screen_update_cupidon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	for (int ytile=0;ytile<16;ytile++)
	{
		for (int xtile=0;xtile<32;xtile++)
		{
			for (int y=0;y<16;y++)
			{
				uint16_t *const destline = &bitmap.pix(ytile*16 + y);

				for (int x=0;x<16;x++)
				{
					destline[(xtile*16)+(x*2)+0] = m_gfxram[count];

					count++;
				}
			}
		}
	}


	return 0;
}

// could be pumped through the get_cs function (if they use the memory protection features we might have to) but that's slow...
void cupidon_state::cupidon_map(address_map &map)
{
	map(0x0000000, 0x07fffff).rom().mirror(0x1000000);

	map(0x1000000, 0x100ffff).ram();
	map(0x1800000, 0x1800003).r(FUNC(cupidon_state::cupidon_return_ffffffff));
	map(0x2000074, 0x2000077).ram(); // port

//  map(0x2000040, 0x200004f).ram();

// might just be 4mb of VRAM
	map(0x3000000, 0x33bffff).ram();
	map(0x33c0000, 0x33fffff).ram().share("gfxram"); // seems to upload graphics to here, tiles etc. if you skip the loop after the romtest in funnyfm
//  map(0x3400000, 0x3400fff).ram();
//  map(0x3F80000, 0x3F80003).ram();
	map(0x3FF0400, 0x3FF0403).ram(); // register? gangrose likes to read this?
}

static INPUT_PORTS_START(  cupidon )
INPUT_PORTS_END


void cupidon_state::cupidon(machine_config &config)
{
	M68340(config, m_maincpu, 16000000);    // The access to 3FF00 at the start would suggest this is a 68340 so probably 16 or 25 mhz?
	m_maincpu->set_addrmap(AS_PROGRAM, &cupidon_state::cupidon_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(cupidon_state::screen_update_cupidon));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x10000);

	SPEAKER(config, "speaker", 2).front();
	/* unknown sound, probably DAC driven using 68340 DMA */
}



ROM_START( tsarevna )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ts_1_29_u2_32m.bin", 0x000000, 0x400000, CRC(e7798a5d) SHA1(5ad876a693c93df79ea5e5672c0a5f3952b2cb36) )
	ROM_LOAD16_WORD_SWAP( "ts_1_29_u1_32m.bin", 0x400000, 0x400000, CRC(5a35ca2a) SHA1(b7beac148190b508469f832d370af082f479527c) )
ROM_END

ROM_START( tsarevnaa )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "v0131-2.bin", 0x000000, 0x400000, CRC(36349e13) SHA1(d82c93b7f19e8b75b0d56653aaaf5da44bb302f5) )
	ROM_LOAD16_WORD_SWAP( "v0131-1.bin", 0x400000, 0x400000, CRC(f502e677) SHA1(84f89f214aeff8544d526c44634672d972714bf6) )
ROM_END

ROM_START( gangrose )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "gangv470m322sec.bin", 0x000000, 0x400000, CRC(c916a292) SHA1(ceac54b06722874f21431834403e49aa2c9c1ded) )
ROM_END


ROM_START( funnyfm )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ff_1_17_u2_32m.bin", 0x000000, 0x400000, CRC(cdd616a7) SHA1(69a9bd73f6f9abb306522071316e1dd770b4ac12) )
	ROM_LOAD16_WORD_SWAP( "ff_1_17_u1_32m.bin", 0x400000, 0x400000, CRC(2073345c) SHA1(33803ebd7720c3436486a383383e99722c2554f4) )
ROM_END

ROM_START( funnyfma )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "ff_1_26_u2_32m.bin", 0x000000, 0x400000, CRC(d813da5c) SHA1(ef82f2c7d0aa21921a25d08555c727a967b1a235) )
	ROM_LOAD16_WORD_SWAP( "ff_1_26_u1_32m.bin", 0x400000, 0x400000, CRC(e3c4f483) SHA1(cc78eadadc13a8f295658b493e47eff3bf719c7e) )
ROM_END

ROM_START( funnyfmb )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "u2.bin", 0x000000, 0x400000, CRC(c8fdc338) SHA1(cd3372988c7a4b35069d6e56e786cecb32e0996e) )
	ROM_LOAD16_WORD_SWAP( "u1.bin", 0x400000, 0x400000, CRC(ca2a5345) SHA1(be7c68fca0534b2d817ac78377f98cda2021c5fa) )
ROM_END

ROM_START( cashtrn )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "cash_train_1_10_u2_32.bin", 0x000000, 0x400000, CRC(ee81a918) SHA1(116e14e8f23517c943f8867498b6105221974ce3) )
	ROM_LOAD16_WORD_SWAP( "cash_train_1_10_u1_32.bin", 0x400000, 0x400000, CRC(4a1704e7) SHA1(18cc87cf54277e61a37cfe9c77164bef9688acf6) )
ROM_END



void cupidon_state::init_cupidon()
{
}

void cupidon_state::init_funnyfm()
{
}

} // anonymous namespace


/* (c) date is from string in ROM, revision date is noted next to sets - Spellings are as found in ROM */
GAME( 2004, tsarevna,  0,        cupidon, cupidon, cupidon_state, init_cupidon, ROT0, "Kupidon", "Tsarevna (v1.29)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 12 Oct 2005
GAME( 2004, tsarevnaa, tsarevna, cupidon, cupidon, cupidon_state, init_cupidon, ROT0, "Kupidon", "Tsarevna (v1.31)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 17 Jan 2007

GAME( 2004, gangrose,  0,        cupidon, cupidon, cupidon_state, init_cupidon, ROT0, "Kupidon", "Gangster's Roses (v4.70)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 01 Sep 2004

GAME( 2004, funnyfm,   0,        cupidon, cupidon, cupidon_state, init_funnyfm, ROT0, "Kupidon", "Funny Farm (v1.17)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 02 Mar 2005
GAME( 2004, funnyfma,  funnyfm,  cupidon, cupidon, cupidon_state, init_cupidon, ROT0, "Kupidon", "Funny Farm (v1.26)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 08 Aug 2005
GAME( 2004, funnyfmb,  funnyfm,  cupidon, cupidon, cupidon_state, init_cupidon, ROT0, "Kupidon", "Funny Farm (v1.30)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 16 May 2006

GAME( 2005, cashtrn,   0,        cupidon, cupidon, cupidon_state, init_cupidon, ROT0, "Kupidon", "Cash Train (v1.10)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // 09 Jan 2006
