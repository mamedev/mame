// license:BSD-3-Clause
// copyright-holders:

/*
Astro Corp. 'Hummer' (VGA)
Main components:
Astro V102PX-0XX CPU (004 for Jack's Venture, 013 for Penguin Party)
DigiArt AM001 ADPCM & MP3 sound chip
22.579 MHz XTAL
Actel Igloo AGLP125-CSG289
Astro ROHS (GFX?)
4x LY61L25616ML-20 SRAM
2x LY621024SL-70LL SRAM
4-DIP bank

+*** ASTRO PASSWORDS ***
pengprty : Funny Penguins / Penguin Party
  → Taken from manual, Version: USA-20111216
+------------+---------+------------+---------+------------+---------+
| Password # | Code    | Password # | Code    | Password # | Code    |
+------------+---------+------------+---------+------------+---------+
| OFF (0)    | No use  | 1          | 724670  | 2          | 540810  |
| 3          | 167008  | 4          | 281630  | 5          | 403432  |
| 6          | 685640  | 7          | 961012  |            |         |
+------------+---------+------------+---------+------------+---------+

jackvent : Jack's Venture / Inca Treasure
  → Taken from manual, Version: USA-090728
+------------+---------+------------+---------+------------+---------+
| Password # | Code    | Password # | Code    | Password # | Code    |
+------------+---------+------------+---------+------------+---------+
| OFF (0)    | No use  | 1          | 548516  | 2          | 754248  |
| 3          | 936415  | 4          | 864578  | 5          | 102647  |
| 6          | 748652  | 7          | 664852  |            |         |
+------------+---------+------------+---------+------------+---------+

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class hummer_state : public driver_device
{
public:
	hummer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void jackvent(machine_config &config) ATTR_COLD;
	void pengprty(machine_config &config) ATTR_COLD;

	void init_px004() ATTR_COLD { decrypt_rom(v102_px004_table); }
	void init_px013() ATTR_COLD { decrypt_rom(v102_px013_table); }

private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void jackvent_map(address_map &map) ATTR_COLD;
	void pengprty_map(address_map &map) ATTR_COLD;

	struct decryption_info {
		struct {
			// Address bits used for bitswap/xor selection
			u8 bits[3];
			struct {
				// 8-8 Bitswap
				u8 bits[8];
				// Xor
				u8 xor_mask;
			} entries[8];
		} rom[2];
		// Global address bitswap (src -> dest, some sets use bits 12-8 only, while others 12-2)
		u8 bits[11];
	};

	static const decryption_info v102_px004_table;
	static const decryption_info v102_px013_table;

	void decrypt_rom(const decryption_info &table) ATTR_COLD;

	void hummer(machine_config &config) ATTR_COLD;
};


uint32_t hummer_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void hummer_state::jackvent_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x800000, 0x87ffff).rom().region("encrypted_rom", 0); // POST checks for encrypted ROM checksum here
	// map(0xc00000, 0xc00001).w // EEPROM / CPU code w?
	map(0xc80000, 0xc8ffff).ram(); // NVRAM?
}

void hummer_state::pengprty_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x800000, 0x87ffff).rom().region("encrypted_rom", 0); // POST checks for encrypted ROM checksum here
	// map(0xb80000, 0xb80001).w // EEPROM / CPU code w?
	map(0xe00000, 0xe0ffff).ram(); // NVRAM?
}


static INPUT_PORTS_START( hummer )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void hummer_state::hummer(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 22.579_MHz_XTAL / 2);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(120_MHz_XTAL / 10 * 2, 781, 0, 512, 261*2, 0, 240*2); // TODO
	screen.set_screen_update(FUNC(hummer_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::BGR_565, 0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// AM001 for sound
}

void hummer_state::jackvent(machine_config &config)
{
	hummer(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hummer_state::jackvent_map);
}

void hummer_state::pengprty(machine_config &config)
{
	hummer(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hummer_state::pengprty_map);
}


ROM_START( pengprty ) // PCBHR REV:E + Flash Card V1.1 riser board for GFX ROMs + CS350P093 TSOP to DIP riser board for sound ROM
	ROM_REGION16_BE( 0x80000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION16_BE( 0x80000, "encrypted_rom", 0 )
	ROM_LOAD16_BYTE( "2-tm_fpus01.01b.tu3", 0x00000, 0x40000, CRC(23cda107) SHA1(f2c5cba9a3c2c8bfea6bce0b221fd9209810fdf3) )
	ROM_LOAD16_BYTE( "1-tm_fpus01.01b.tu1", 0x00001, 0x40000, CRC(2569e2b9) SHA1(dcec1e9bfe73a062b891812f2c8eb8407066b993) )

	ROM_REGION( 0x8000000, "sprites", 0 ) // TODO: probably interleaved
	ROM_LOAD( "mx29gl128eh.u1", 0x0000000, 0x1000000, CRC(80f0d70f) SHA1(82de9bb82a2c5901d5e2dc8f93cd2eee5d65a20b) )
	ROM_LOAD( "mx29gl128eh.u2", 0x1000000, 0x1000000, CRC(9a9da6b4) SHA1(41d95e2de41a99c172ba210e51170e14219d393e) )
	ROM_LOAD( "mx29gl128eh.u3", 0x2000000, 0x1000000, CRC(458f22e5) SHA1(cf3b3084a980568646c588d33123576bd25261a5) )
	ROM_LOAD( "mx29gl128eh.u4", 0x3000000, 0x1000000, CRC(91f1069c) SHA1(67c7e56345f63c01279d39527b31fa9eeb7b4bf0) )
	ROM_LOAD( "mx29gl128eh.u5", 0x4000000, 0x1000000, CRC(fd4bc0d5) SHA1(0829ed0762311b3afd3b2a86c128077138793b53) )
	ROM_LOAD( "mx29gl128eh.u6", 0x5000000, 0x1000000, CRC(2280b3db) SHA1(601a7c1a7b868a0bb9395f38999426b45f008199) )
	ROM_LOAD( "mx29gl128eh.u7", 0x6000000, 0x1000000, CRC(8cfc06f9) SHA1(cc4386c3cde41145672102e3e29cb5c949246f62) )
	ROM_LOAD( "mx29gl128eh.u8", 0x7000000, 0x1000000, CRC(83907de9) SHA1(480df6092e9a78da69b220bfd0f2727a58d677c0) )

	ROM_REGION( 0x800000, "am001", 0 )
	ROM_LOAD( "mx29lv640eb.u53", 0x000000, 0x800000, CRC(0289fef0) SHA1(f349abd69fcbb9b92ba1362f01c3a83a521fdcc3) )

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "pengprty_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END

ROM_START( jackvent ) // PCBHR REV:C + Flash Card V1.1 riser board for GFX ROMs + CS350P093 TSOP to DIP riser board for sound ROM
	ROM_REGION16_BE( 0x80000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION16_BE( 0x80000, "encrypted_rom", 0 )
	ROM_LOAD16_BYTE( "2_jvus02.01a.tu3", 0x00000, 0x40000, CRC(945a01b1) SHA1(e621c1dd0db573dd5e3bc2202b04c997d84af4fc) )
	ROM_LOAD16_BYTE( "1_jvus02.01a.tu1", 0x00001, 0x40000, CRC(71471ff7) SHA1(b342d93417b9f8d2e5e36152a31acb09b6a5acd3) )

	ROM_REGION( 0x4000000, "sprites", 0 ) // TODO: probably interleaved
	ROM_LOAD( "29lv640.u1", 0x0000000, 0x0800000, CRC(5be4c27d) SHA1(f16bb283e7d28148efacae7d42091985d96825b8) )
	ROM_LOAD( "29lv640.u2", 0x0800000, 0x0800000, CRC(1dd17d97) SHA1(a8e3b9f47bc8cf85f1008e16223245bbe6dc7f79) )
	ROM_LOAD( "29lv640.u3", 0x1000000, 0x0800000, CRC(970f6dd2) SHA1(b5db6c64e3ad6c8b0c0014b9cbdc2efadeb9b900) )
	ROM_LOAD( "29lv640.u4", 0x1800000, 0x0800000, CRC(d92453da) SHA1(612aacbb6724c195cf28ed25a73a6220c2e1fc32) )
	ROM_LOAD( "29lv640.u5", 0x2000000, 0x0800000, CRC(113218d6) SHA1(1182fe1ce29fbbbbfdd7ea0a5f6f6f85eab7acc6) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u6", 0x2800000, 0x0800000, CRC(e8e06adb) SHA1(4c7871c405d6caa5ab5516410f730017c713984d) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u7", 0x3000000, 0x0800000, CRC(ef777222) SHA1(5a53c72584c1a8b098df085bade866d887f48b29) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "29lv640.u8", 0x3800000, 0x0800000, CRC(a48acc31) SHA1(4d35fd060dc86a10bd69715e17a9399ff97e4343) ) // 1xxxxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800000, "am001", 0 )
	ROM_LOAD( "29lv640.u53", 0x000000, 0x800000, CRC(c891b5ff) SHA1(5e0dce5b33230bd181f3eed94f64da328d11be28) )

	ROM_REGION16_LE( 0x02, "astro_cpucode", 0 )
	ROM_LOAD( "jackvent_cpucode.key", 0x00, 0x02, NO_DUMP )
ROM_END


void hummer_state::decrypt_rom(const decryption_info &table)
{
	const u32 size = memregion("maincpu")->bytes();
	u16 * const rom = (u16 *)memregion("encrypted_rom")->base();
	u16 * const decrypted = (u16 *)memregion("maincpu")->base();
	std::unique_ptr<u16[]> tmp = std::make_unique<u16[]>(size/2);

	// Pass 1: decrypt high and low byte independently.  They go
	// through a bitswap and an xor, choosing between 8 possibilities
	// through address bits.

	for (u32 i = 0; i != size; i += 2) {
		u16 orig = rom[i >> 1];
		u16 result = 0;
		for (u32 rb = 0; rb < 2; rb ++) {
			u8 val = orig >> (rb ? 0 : 8);
			u32 index =
				(BIT(i, table.rom[rb].bits[0]) << 2) |
				(BIT(i, table.rom[rb].bits[1]) << 1) |
				BIT(i, table.rom[rb].bits[2]);
			val = bitswap(val,
						  table.rom[rb].entries[index].bits[0],
						  table.rom[rb].entries[index].bits[1],
						  table.rom[rb].entries[index].bits[2],
						  table.rom[rb].entries[index].bits[3],
						  table.rom[rb].entries[index].bits[4],
						  table.rom[rb].entries[index].bits[5],
						  table.rom[rb].entries[index].bits[6],
						  table.rom[rb].entries[index].bits[7]);
			val = val ^ table.rom[rb].entries[index].xor_mask;

			result |= val << (rb ? 0 : 8);
		}
		tmp[i >> 1] = result;
	}

	// Pass 2: copy back the decrypted data following the address
	// scrambling
	for (u32 i = 0; i != size; i += 2) {
		u32 dest =
			(i & 0xffffe003) |
			(BIT(i, table.bits[0])  << 12) |
			(BIT(i, table.bits[1])  << 11) |
			(BIT(i, table.bits[2])  << 10) |
			(BIT(i, table.bits[3])  <<  9) |
			(BIT(i, table.bits[4])  <<  8) |
			(BIT(i, table.bits[5])  <<  7) |
			(BIT(i, table.bits[6])  <<  6) |
			(BIT(i, table.bits[7])  <<  5) |
			(BIT(i, table.bits[8])  <<  4) |
			(BIT(i, table.bits[9])  <<  3) |
			(BIT(i, table.bits[10]) <<  2);
		decrypted[dest >> 1] = tmp[i >> 1];
	}

	// TODO: are these overlays provided by the custom CPU? It does seem so.
	// both the encrypted and decrypted bytes at these positions make no sense.
	// gostopac explicitly checks that 0x4-0x7 are 0x0400'0400
	decrypted[0x00004/2] = 0x0400;
	decrypted[0x00006/2] = 0x0400;
}

// TODO: this is the same as v102_px001_table, shouldn't be?
const hummer_state::decryption_info hummer_state::v102_px004_table = {
	{
		{
			{ 8, 9, 10 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 9, 11 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 10, 8, 11, 9, 7, 2, 4, 6, 5, 3 }
};

const hummer_state::decryption_info hummer_state::v102_px013_table = {
	{
		{
			{ 8, 9, 10 },
			{
				{ { 7, 5, 4, 6,  0, 3, 2, 1 }, 0x00 },
				{ { 1, 4, 6, 0,  2, 5, 3, 7 }, 0xd0 },
				{ { 1, 7, 4, 3,  6, 5, 0, 2 }, 0x88 },
				{ { 6, 5, 2, 3,  7, 1, 0, 4 }, 0xd1 },
				{ { 6, 1, 7, 2,  4, 0, 3, 5 }, 0x64 },
				{ { 1, 7, 2, 6,  5, 4, 3, 0 }, 0x83 },
				{ { 6, 7, 4, 2,  5, 0, 1, 3 }, 0x81 },
				{ { 7, 5, 1, 0,  2, 4, 6, 3 }, 0xea },
			}
		},
		{
			{ 12, 9, 11 },
			{
				{ { 6, 5, 4, 3,  2, 1, 0, 7 }, 0x90 },
				{ { 2, 4, 0, 7,  5, 6, 3, 1 }, 0x32 },
				{ { 7, 1, 0, 6,  5, 2, 3, 4 }, 0xa9 },
				{ { 2, 0, 3, 5,  1, 4, 6, 7 }, 0xa2 },
				{ { 3, 0, 6, 5,  2, 1, 4, 7 }, 0x02 },
				{ { 0, 1, 6, 4,  5, 2, 7, 3 }, 0x30 },
				{ { 3, 5, 2, 7,  6, 1, 4, 0 }, 0x0a },
				{ { 0, 6, 4, 2,  7, 3, 1, 5 }, 0x81 },
			}
		}
	},
	{ 12, 10, 8, 11, 9, 7, 6, 5, 4, 3, 2 }
};

} // anonymous namespace


GAME ( 2012,  pengprty, 0, pengprty, hummer, hummer_state, init_px013, ROT0, "Astro Corp.", "Penguin Party (TM.01.01.B, 2012/01/16)",                  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME ( 2010,  jackvent, 0, jackvent, hummer, hummer_state, init_px004, ROT0, "Astro Corp.", "Jack's Venture - Inca Treasure (US.02.01.A, 2010/01/21)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
