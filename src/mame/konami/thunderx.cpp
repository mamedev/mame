// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Manuel Abadia
/***************************************************************************

    Super Contra  (GX775) (c) 1988 Konami
    Thunder Cross (GX873) (c) 1988 Konami
    Gang Busters  (GX878) (c) 1988 Konami

    driver by Bryan McPhail, Manuel Abadia

    K052591 emulation by Eddie Edwards

    These three games run on very similar boards, to the extent that
    Thunder Cross and Gang Busters share the same schematics with
    callouts ("USED ONLY GX873" or "USED ONLY GX878") to indicate the
    per-game differences.

    Super Contra uses an external latch for program ROM banking
    instead of the builtin banking of the Konami custom CPU,
    and is the only one of the three games that uses sprite shadows.

    Thunder Cross has no 007232 sound chip, and has a 052591 PMC,
    a programmable custom chip also used by S.P.Y. and Hexion.
    Here it's used for collision detection.

    Gang Busters uses the 052526 custom CPU instead of 052001, and
    has slightly different tile ROM addressing.

- There was a set in MAME at one time that was given the setname (thndrxja)
  which is supposedly a later revision of the japanese set currently in MAME.
  No roms were ever sourced for this set, so the GAME macro no longer exists.

***************************************************************************/

#include "emu.h"

#include "k051960.h"
#include "k052109.h"
#include "konami_helper.h"
#include "konamipt.h"

#include "cpu/m6809/konami.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "sound/ymopm.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "multibyte.h"

//#define VERBOSE 1
#include "logmacro.h"


namespace {

class thunderx_state_base : public driver_device
{
public:
	thunderx_state_base(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette"),
		m_rombank(*this, "rombank"),
		m_bank5800(*this, "bank5800")
	{ }

protected:
	// devices
	required_device<konami_cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;

	// memory
	required_memory_bank m_rombank;
	memory_view m_bank5800;

	// misc
	uint8_t    m_priority = 0;
	uint8_t    m_1f98_latch = 0;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint8_t _1f98_r();
	void scontra_1f98_w(uint8_t data);

	void common(machine_config &config) ATTR_COLD;

	void scontra_map(address_map &map) ATTR_COLD;

	void thunderx_sound_map(address_map &map) ATTR_COLD;

private:
	void scontra_bankswitch_w(uint8_t data);
	void sh_irqtrigger_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);
};


class scontra_state : public thunderx_state_base
{
public:
	scontra_state(const machine_config &mconfig, device_type type, const char *tag) :
		thunderx_state_base(mconfig, type, tag),
		m_k007232(*this, "k007232")
	{
	}

	void scontra(machine_config &config) ATTR_COLD;
	void gbusters(machine_config &config) ATTR_COLD;

private:
	required_device<k007232_device> m_k007232;

	void gbusters_videobank_w(uint8_t data);
	void k007232_bankswitch_w(uint8_t data);

	K052109_CB_MEMBER(gbusters_tile_callback);
	void volume_callback(uint8_t data);

	void gbusters_map(address_map &map) ATTR_COLD;

	void scontra_sound_map(address_map &map) ATTR_COLD;
};


class thunderx_state : public thunderx_state_base
{
public:
	thunderx_state(const machine_config &mconfig, device_type type, const char *tag) :
		thunderx_state_base(mconfig, type, tag),
		m_pmcram(*this, "pmcram")
	{
	}

	void thunderx(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_pmcram;

	emu_timer *m_thunderx_firq_timer = nullptr;

	uint8_t pmc_bk() const { return BIT(m_1f98_latch, 1); }

	TIMER_CALLBACK_MEMBER(thunderx_firq_cb);
	uint8_t pmc_r(offs_t offset);
	void pmc_w(offs_t offset, uint8_t data);
	void thunderx_videobank_w(uint8_t data);
	void thunderx_1f98_w(uint8_t data);

	void pmc_run();

	void thunderx_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

static const int layer_colorbase[] = { 768 / 16, 0 / 16, 256 / 16 };

K052109_CB_MEMBER(thunderx_state_base::tile_callback)
{
	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

K052109_CB_MEMBER(scontra_state::gbusters_tile_callback)
{
	/* (color & 0x02) is flip y handled internally by the 052109 */
	*code |= ((*color & 0x0d) << 8) | ((*color & 0x10) << 5) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(thunderx_state_base::sprite_callback)
{
	enum { sprite_colorbase = 512 / 16 };

	/* Sprite priority 1 means appear behind background, used only to mask sprites */
	/* in the foreground */
	/* Sprite priority 3 means don't draw (not used) */
	switch (*color & 0x30)
	{
		case 0x00: *priority = 0; break;
		case 0x10: *priority = GFX_PMASK_2 | GFX_PMASK_1; break;
		case 0x20: *priority = GFX_PMASK_2; break;
		case 0x30: *priority = 0xffff; break;
	}

	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t thunderx_state_base::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);

	// The background color is always from layer 1
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 0);

	const int bg = m_priority ? 2 : 1;
	const int fg = m_priority ? 1 : 2;

	m_k052109->tilemap_draw(screen, bitmap, cliprect, bg, 0, 1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, fg, 0, 2);
	m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
	m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);

	return 0;
}


TIMER_CALLBACK_MEMBER(thunderx_state::thunderx_firq_cb)
{
	m_maincpu->set_input_line(KONAMI_FIRQ_LINE, HOLD_LINE);
}

uint8_t thunderx_state::pmc_r(offs_t offset)
{
	if (pmc_bk())
	{
		//logerror("%04x read pmcram %04x\n",m_audiocpu->pc(),offset);
		return m_pmcram[offset];
	}
	else
	{
		return 0; // PMC internal RAM can't be read back
	}
}

void thunderx_state::pmc_w(offs_t offset, uint8_t data)
{
	if (pmc_bk())
	{
		LOG("%04x pmcram %04x = %02x\n", m_audiocpu->pc(), offset, data);
		m_pmcram[offset] = data;
	}
	else
	{
		LOG("%04x pmc set initial PC %04x = %02x\n", m_audiocpu->pc(), offset, data);
		// thunderx only uses one PMC program which has its entry point at address 01
	}
}

/*
This is the 052591 PMC code loaded at startup, it contains a collision check program.
See https://github.com/furrtek/SiliconRE/tree/master/Konami/052591 for details

    common version  thunderxa only
00: e7 00 00 ad 08  00: e7 00 00 ad 08  JP 00, infinite loop
01: 5f 80 05 a0 0c  01: 1f 80 05 a0 0c  Set ext address to 5
                    02: 42 7e 00 8b 04
02: df 00 e2 8b 08  03: df 8e 00 cb 04  r0.b or r0.w = RAM[5]
03: 5f 80 06 a0 0c  04: 5f 80 07 a0 0c  Set ext address to 6 or 7
04: df 7e 00 cb 08  05: df 7e 00 cb 08  r7.b = RAM[6 or 7]
05: 1b 80 00 a0 0c  06: 1b 80 00 a0 0c  Set ext address to r0
06: df 10 00 cb 08  07: df 10 00 cb 08  r1.b = RAM[r0] (flags)
07: 5f 80 03 a0 0c  08: 5f 80 03 a0 0c  Set ext address to 3
08: 1f 20 00 cb 08  09: 1f 20 00 cb 08  acc.b = RAM[3] (collide mask)
09: c4 00 00 ab 0c  0a: c4 00 00 ab 0c  INC r0, set ext address to r0
0a: df 20 00 cb 08  0b: df 20 00 cb 08  r2.b = RAM[r0] (width)
0b: c4 00 00 ab 0c  0c: c4 00 00 ab 0c  INC r0, set ext address to r0
0c: df 30 00 cb 08  0d: df 30 00 cb 08  r3.b = RAM[r0] (height)
0d: c4 00 00 ab 0c  0e: c4 00 00 ab 0c  INC r0, set ext address to r0
0e: df 40 00 cb 08  0f: df 40 00 cb 08  r4.b = RAM[r0] (x)
0f: c4 00 00 ab 0c  10: c4 00 00 ab 0c  INC r0, set ext address to r0
10: df 50 00 cb 08  11: df 50 00 cb 08  r5.b = RAM[r0] (y)
11: 60 22 35 e9 08  12: 60 22 36 e9 08  JP 35 or 36 if r1 AND acc == 0
12: 44 0e 00 ab 08  13: 44 0e 00 ab 08  Set ext address to r7
13: df 60 00 cb 08  14: df 60 00 cb 08  r6.b = RAM[r7] (flags)
14: 5f 80 04 a0 0c  15: 5f 80 04 a0 0c  Set ext address to 4
15: 1f 60 00 cb 08  16: 1f 60 00 cb 08  acc.b = RAM[4] (hit mask)
16: 60 6c 31 e9 08  17: 60 6c 32 e9 08  JP 32 if r6 AND acc == 0
17: 45 8e 01 a0 0c  18: 45 8e 01 a0 0c  Set ext address to r7 + 1
18: c5 64 00 cb 08  19: c5 64 00 cb 08  r6.b = RAM[r7 + 1] + r2 (add widths together)
19: 45 8e 03 a0 0c  1a: 45 8e 03 a0 0c  Set ext address to r7 + 3
1a: 67 00 00 cb 0c  1b: 67 00 00 cb 0c  acc = RAM[r7 + 3] - r4 (x1 - x0)
1b: 15 48 5d c9 0c  1c: 15 48 5e c9 0c  JP 1D or 1E if positive
1c: 12 00 00 eb 0c  1d: 12 00 00 eb 0c  NEG acc
1d: 48 6c 71 e9 0c  1e: 48 6c 72 e9 0c  JP 31 or 32 if r6 < acc
1e: 45 8e 02 a0 0c  1f: 45 8e 02 a0 0c  Set ext address to r7 + 2
1f: c5 66 00 cb 08  20: c5 66 00 cb 08  r6.b = RAM[r7 + 2] + r3 (add heights together)
20: 45 8e 04 a0 0c  21: 45 8e 04 a0 0c  Set ext address to r7 + 4
21: 67 00 00 cb 0c  22: 67 00 00 cb 0c  acc = RAM[r7 + 4] - r5 (y1 - y0)
22: 15 5a 64 c9 0c  23: 15 5a 65 c9 0c  JP 24 or 25 if positive
23: 12 00 00 eb 0c  24: 12 00 00 eb 0c  NEG acc
24: 48 6c 71 e9 0c  25: 48 6c 72 e9 0c  JP 31 or 32 if r6 < acc
25: e5 92 9b e0 0c  26: e5 92 9b e0 0c  AND R1,#$9B
26: dd 92 10 e0 0c  27: dd 92 10 e0 0c  OR R1,#$10
27: 5c fe 00 a0 0c  28: 5c fe 00 a0 0c  Set ext address to r7
28: df 60 00 d3 08  29: df 60 00 d3 08  r6.b = RAM[r7] (flags)
29: e5 ec 9f e0 0c  2a: e5 ec 9f e0 0c  AND r6,#$9F
2a: dd ec 10 00 0c  2b: dd ec 10 00 0c  RAM[r7] = r6 | #$10
2b: 25 ec 04 c0 0c  2c: 25 ec 04 c0 0c  acc = r6 & 4
2c: 18 82 00 00 0c  2d: 18 82 00 00 0c  OR acc,r1
2d: 4d 80 03 a0 0c  2e: 4d 80 03 a0 0c  Set ext address to r7 - 4
2e: df e0 e6 e0 0c  2f: df e0 36 e1 0c  r6 = #$E6 or #$136
2f: 49 60 75 f1 08  30: 49 60 76 f1 08  JP 35 or 36 if r0 < r6
30: 67 00 35 cd 08  31: 67 00 36 cd 08  Write acc to [r7 - 4], JP 35 or 36
31: c5 fe 05 e0 0c  32: c5 fe 05 e0 0c  r7 += 5, next object in set 1
32: 5f 80 02 a0 0c  33: 5f 80 02 a0 0c  Set ext address to 2
33: 1f 00 00 cb 08  34: 1f 00 00 cb 08  acc.b = RAM[2]
34: 48 6e 52 c9 0c  35: 48 6e 53 c9 0c  JP 12 or 13 if r7 < acc
35: c4 00 00 ab 0c  36: c4 00 00 ab 0c  INC r0, next object in set 0
36: 27 00 00 ab 0c  37: 27 00 00 ab 0c  Set ext address to 0
37: 42 00 00 8b 04  38: 42 00 00 8b 04  acc.w = RAM[0]
38: 1f 00 00 cb 00  39: 1f 00 00 cb 00
39: 48 00 43 c9 00  3a: 48 00 44 c9 00  JP 3 or 4 if r0 < acc
3a: 5f fe 00 e0 08  3b: 5f fe 00 e0 08  Set OUT0 low
3b: 5f 7e 00 ed 08  3c: 5f 7e 00 ed 08  JP 0
3c: ff 04 00 ff 06  3d: ff 04 00 ff 06  Garbage
3d: 05 07 ff 02 03  3e: 05 07 ff 02 03  Garbage
3e: 01 01 e0 02 6c  3f: 01 00 60 00 a0  Garbage
3f: 03 6c 04 40 04                      Garbage
*/

// emulates K052591 collision detection

void thunderx_state::pmc_run()
{
	// the data at 0x00 to 0x06 defines the operation
	//
	// 0x00 : word : last byte of set 0
	// 0x02 : byte : last byte of set 1
	// 0x03 : byte : collide mask
	// 0x04 : byte : hit mask
	// 0x05 : byte : first byte of set 0
	// 0x06 : byte : first byte of set 1
	//
	// thunderxa is slightly different:
	//
	// 0x05 : word : first byte of set 0
	// 0x07 : byte : first byte of set 1
	//
	// the operation is to intersect set 0 with set 1
	// masks specify objects to ignore

	const uint16_t e0 = get_u16be(&m_pmcram[0]);
	const uint8_t e1 = m_pmcram[2];

	uint16_t s0, s1, p0_lim;

	// Heuristic to determine version of program based on byte at 0x05
	if (m_pmcram[5] < 16)
	{
		// thunderxa only
		s0 = get_u16be(&m_pmcram[5]);
		s1 = m_pmcram[7];
		p0_lim = 0x136;
	}
	else
	{
		// other sets
		s0 = m_pmcram[5];
		s1 = m_pmcram[6];
		p0_lim = 0xe6;
	}

	const uint8_t cm = m_pmcram[3];
	const uint8_t hm = m_pmcram[4];

	// collide objects from s0 to e0 against objects from s1 to e1
	// only process objects with the specified bits (cm/hm) set in their flags
	//
	// the data format for each object is:
	//
	// +0 : flags
	// +1 : width (4 pixel units)
	// +2 : height (4 pixel units)
	// +3 : x (2 pixel units) of center of object
	// +4 : y (2 pixel units) of center of object
	for (uint8_t *p0 = &m_pmcram[s0]; p0 < &m_pmcram[e0]; p0 += 5)
	{
		// check object 0 flags
		if (!(p0[0] & cm))
			continue;

		for (uint8_t *p1 = &m_pmcram[s1]; p1 < &m_pmcram[e1]; p1 += 5)
		{
			// check object 1 flags
			if (!(p1[0] & hm))
				continue;

			if (p1[1] + p0[1] < abs(p1[3] - p0[3])) continue;
			if (p1[2] + p0[2] < abs(p1[4] - p0[4])) continue;

			// set flags
			p1[0] = (p1[0] & 0x8f) | 0x10;
			if (&p0[4] >= &m_pmcram[p0_lim]) // This address value is hardcoded in the PMC program
				p0[0] = (p0[0] & 0x9b) | (p1[0] & 0x04) | 0x10;
			break;
		}
	}

	// 100 cycle delay is arbitrary
	m_thunderx_firq_timer->adjust(m_maincpu->cycles_to_attotime(100));
}

void thunderx_state_base::scontra_1f98_w(uint8_t data)
{
	// bit 0 = enable char ROM reading through the video RAM
	m_k052109->set_rmrd_line((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);

	m_1f98_latch = data;
}

uint8_t thunderx_state_base::_1f98_r()
{
	// thunderx and gbusters read from here during the gfx ROM test...
	// though it doesn't look like it should be readable based on the schematics
	return m_1f98_latch;
}

void thunderx_state::thunderx_1f98_w(uint8_t data)
{
	// logerror("%04x: 1f98_w %02x\n", m_maincpu->pc(),data);

	// bit 1 = PMC BK (select PMC program or data RAM)
	// handled in pmc_r() and pmc_w()

	// bit 2 = PMC START (do collision detection when 0->1)
	if ((data & 4) && !(m_1f98_latch & 4))
	{
		pmc_run();
	}

	scontra_1f98_w(data);
}

void thunderx_state_base::scontra_bankswitch_w(uint8_t data)
{
	// bits 0-3 select ROM bank at 6000-7fff
	m_rombank->set_entry(data & 0x0f);

	// bit 4 selects work RAM or palette RAM at 5800-5fff
	if (BIT(data, 4))
		m_bank5800.disable();
	else
		m_bank5800.select(0);

	// bits 5-6 coin counters
	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6));

	// bit 7 controls layer priority
	m_priority = BIT(data, 7);
}

void thunderx_state::thunderx_videobank_w(uint8_t data)
{
	// 0x01 = work RAM at 4000-5fff
	// 0x00 = palette at 5800-5fff
	// 0x10 = PMC at 5800-5fff
	if (BIT(data, 4))
		m_bank5800.select(1);
	else if (BIT(data, 0))
		m_bank5800.disable();
	else
		m_bank5800.select(0);

	// bits 1-2 coin counters
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 2));

	// bit 3 controls layer priority
	m_priority = BIT(data, 3);
}

void scontra_state::gbusters_videobank_w(uint8_t data)
{
	// same as thunderx without the PMC
	if (BIT(data, 0))
		m_bank5800.disable();
	else
		m_bank5800.select(0);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 1));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 2));

	m_priority = BIT(data, 3);
}

void thunderx_state_base::sh_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void scontra_state::k007232_bankswitch_w(uint8_t data)
{
	// b3-b2: bank for channel B
	// b1-b0: bank for channel A
	const int bank_A = data & 0x03;
	const int bank_B = (data >> 2) & 0x03;
	m_k007232->set_bank(bank_A, bank_B);
}

uint8_t thunderx_state_base::k052109_051960_r(offs_t offset)
{
	if (m_k052109->get_rmrd_line() == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return m_k051960->k051937_r(offset - 0x3800);
		else if (offset < 0x3c00)
			return m_k052109->read(offset);
		else
			return m_k051960->k051960_r(offset - 0x3c00);
	}
	else
		return m_k052109->read(offset);
}

void thunderx_state_base::k052109_051960_w(offs_t offset, uint8_t data)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(offset, data);
	else
		m_k051960->k051960_w(offset - 0x3c00, data);
}

/***************************************************************************/

void thunderx_state_base::scontra_map(address_map &map)
{
	map(0x0000, 0x3fff).rw(FUNC(thunderx_state_base::k052109_051960_r), FUNC(thunderx_state::k052109_051960_w));       // video RAM + sprite RAM

	map(0x1f80, 0x1f80).w(FUNC(thunderx_state_base::scontra_bankswitch_w)); // bankswitch control + coin counters
	map(0x1f84, 0x1f84).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x1f88, 0x1f88).w(FUNC(thunderx_state_base::sh_irqtrigger_w));     // cause interrupt on audio CPU
	map(0x1f8c, 0x1f8c).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1f90, 0x1f90).portr("SYSTEM");
	map(0x1f91, 0x1f91).portr("P1");
	map(0x1f92, 0x1f92).portr("P2");
	map(0x1f93, 0x1f93).portr("DSW3");
	map(0x1f94, 0x1f94).portr("DSW1");
	map(0x1f95, 0x1f95).portr("DSW2");
	map(0x1f98, 0x1f98).rw(FUNC(thunderx_state_base::_1f98_r), FUNC(thunderx_state_base::scontra_1f98_w));

	map(0x4000, 0x5fff).ram();
	map(0x6000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0xffff).rom();

	// palette can be mapped over the top quarter of RAM
	map(0x5800, 0x5fff).view(m_bank5800);
	m_bank5800[0](0x5800, 0x5fff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
}

void thunderx_state::thunderx_map(address_map &map)
{
	scontra_map(map);

	map(0x1f80, 0x1f80).w(FUNC(thunderx_state::thunderx_videobank_w));
	map(0x1f98, 0x1f98).rw(FUNC(thunderx_state::_1f98_r), FUNC(thunderx_state::thunderx_1f98_w)); // registers

	// PMC can also be mapped over the top of RAM
	m_bank5800[1](0x5800, 0x5fff).rw(FUNC(thunderx_state::pmc_r), FUNC(thunderx_state::pmc_w)).share(m_pmcram);
}

void scontra_state::gbusters_map(address_map &map)
{
	scontra_map(map);

	map(0x1f80, 0x1f80).w(FUNC(scontra_state::gbusters_videobank_w));
}


void thunderx_state_base::thunderx_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0xa000, 0xa000).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
}

void scontra_state::scontra_sound_map(address_map &map)
{
	thunderx_sound_map(map);

	map(0xb000, 0xb00d).rw(m_k007232, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xf000, 0xf000).w(FUNC(scontra_state::k007232_bankswitch_w));
}

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( scontra )
	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("P1")
	KONAMI8_B12_UNK(1)

	PORT_START("P2")
	KONAMI8_B12_UNK(2)

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW2:3" ) // test mode calls it cabinet type, but this is a 2 players game
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 200000" )
	PORT_DIPSETTING(    0x10, "50000 300000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )
	PORT_SERVICE_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x08, 0x00, "Continue Limit (1Player/2Players)" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, "3times / Twice altogether" )
	PORT_DIPSETTING(    0x00, "5times / 4times altogether" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( thunderx )
	PORT_INCLUDE( scontra )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, "Award Bonus Life" )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 200000" ) // Japanese default
	PORT_DIPSETTING(    0x10, "50000 300000" ) // US default
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW3:2" )
	PORT_SERVICE_DIPLOC(   0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW3:4" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( thnderxj )
	PORT_INCLUDE( thunderx )

	PORT_MODIFY("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:3" ) // manual says "OFF=Table On=Upright", but not work?
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30000 200000" ) // Japanese default
	PORT_DIPSETTING(    0x10, "50000 300000" ) // US default
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x00, "50000" )
INPUT_PORTS_END

static INPUT_PORTS_START( gbusters )
	PORT_INCLUDE( scontra )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x04, "Bullets" )           PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "50k, 200k, Every 400k" )
	PORT_DIPSETTING(    0x10, "70k, 250k, Every 500k" )
	PORT_DIPSETTING(    0x08, "50k Only" )
	PORT_DIPSETTING(    0x00, "70k Only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" ) /* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" ) /* Listed as "Unused" */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************************************************************

    Machine Driver

***************************************************************************/

void scontra_state::volume_callback(uint8_t data)
{
	m_k007232->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void thunderx_state_base::machine_start()
{
	save_item(NAME(m_1f98_latch));
	save_item(NAME(m_priority));

	// verified from both scontra and thunderx/gbusters schematics
	// banks 4-7 must mirror banks 0-3 for gbusters ROM test to pass
	uint8_t *const ROM = memregion("maincpu")->base();
	m_rombank->configure_entries(0, 4, &ROM[0], 0x2000);
	m_rombank->configure_entries(4, 4, &ROM[0], 0x2000);
	m_rombank->configure_entries(8, 8, &ROM[0x10000], 0x2000);

	m_palette->set_shadow_factor(7.0/8.0);
}

void thunderx_state_base::machine_reset()
{
	m_rombank->set_entry(0);
	m_bank5800.select(0);
	m_1f98_latch = 0;
	m_priority = 0;
}

void thunderx_state_base::common(machine_config &config)
{
	// basic machine hardware
	KONAMI(config, m_maincpu, XTAL(24'000'000)/2); // 052001 (verified on PCB)
	m_maincpu->set_addrmap(AS_PROGRAM, &thunderx_state_base::scontra_map);

	Z80(config, m_audiocpu, XTAL(3'579'545)); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &thunderx_state_base::thunderx_sound_map);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.17); // verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(12*8, (64-12)*8-1, 2*8, 30*8-1); // verified on scontra and thunderx PCBs
	screen.set_screen_update(FUNC(thunderx_state_base::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->enable_shadows();

	K052109(config, m_k052109, 0); // 051961 on Super Contra and Thunder Cross schematics
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(thunderx_state_base::tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, KONAMI_IRQ_LINE);

	K051960(config, m_k051960, 0);
	m_k051960->set_palette(m_palette);
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(thunderx_state_base::sprite_callback));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	YM2151(config, "ymsnd", XTAL(3'579'545)).add_route(0, "mono", 1.0).add_route(1, "mono", 1.0);  /* verified on pcb */
}

void scontra_state::scontra(machine_config &config)
{
	common(config);

	m_audiocpu->set_addrmap(AS_PROGRAM, &scontra_state::scontra_sound_map);

	K007232(config, m_k007232, XTAL(3'579'545)); // verified on PCB
	m_k007232->port_write().set(FUNC(scontra_state::volume_callback));
	m_k007232->add_route(0, "mono", 0.20);
	m_k007232->add_route(1, "mono", 0.20);
}


void thunderx_state::machine_start()
{
	thunderx_state_base::machine_start();

	m_thunderx_firq_timer = timer_alloc(FUNC(thunderx_state::thunderx_firq_cb), this);
}

void thunderx_state::thunderx(machine_config &config)
{
	common(config);

	// basic machine hardware
	// CPU type is 052001 (verified on PCB)
	m_maincpu->set_addrmap(AS_PROGRAM, &thunderx_state::thunderx_map);
	m_maincpu->line().set_membank(m_rombank).mask(0x0f);

	m_k052109->nmi_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);
}

void scontra_state::gbusters(machine_config &config)
{
	scontra(config);

	// basic machine hardware
	// CPU type is 052526
	m_maincpu->set_addrmap(AS_PROGRAM, &scontra_state::gbusters_map);
	m_maincpu->line().set_membank(m_rombank).mask(0x0f);

	m_k052109->set_tile_callback(FUNC(scontra_state::gbusters_tile_callback));
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( scontra )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "775-e02.k11",     0x00000, 0x10000, CRC(a61c0ead) SHA1(9a0aadc8d3538fc1d88b761753fffcac8923a218) )   /* banked + fixed ROM */
	ROM_LOAD( "775-e03.k13",     0x10000, 0x10000, CRC(00b02622) SHA1(caf1da53815e437e3fb952d29e71f2c314684cd9) )   /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the SOUND CPU */
	ROM_LOAD( "775-c01.bin", 0x00000, 0x08000, CRC(0ced785a) SHA1(1eebe005a968fbaac595c168499107e34763976c) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "775-a07a.6f", 0x00000, 0x20000, CRC(e716bdf3) SHA1(82e10132f248aed8cc1aea6bb7afe9a1479c8b59) )
	ROM_LOAD32_BYTE( "775-a07e.5f", 0x00001, 0x20000, CRC(0986e3a5) SHA1(61c33a3f2e4fde7d23d440b5c3151fe38e25716b) )
	ROM_LOAD32_BYTE( "775-a08a.4f", 0x00002, 0x20000, CRC(3ddd11a4) SHA1(4831a891d6cb4507053d576eddd658c338318176) )
	ROM_LOAD32_BYTE( "775-a08e.3f", 0x00003, 0x20000, CRC(1007d963) SHA1(cba4ca058dee1c8cdeb019e1cc50cae76bf419a1) )
	//ROM_LOAD32_BYTE( "775a07a.6f", 0x00000, 0x10000, CRC(bf37d991) SHA1(56dd9daebcb8a2a46925348175b4b5cdf9d42845) ) // also seen on a 'PWB350958 ROMBOARD 34M' with the following, smaller ROMs
	//ROM_LOAD32_BYTE( "775a07e.5f", 0x00001, 0x10000, CRC(1927cb28) SHA1(a659d10280d4236478a3740ffeef26602077d86e) )
	//ROM_LOAD32_BYTE( "775a08a.4f", 0x00002, 0x10000, CRC(1fda0814) SHA1(da5541482d60e745a18a5671165cac79ea80711f) )
	//ROM_LOAD32_BYTE( "775a08e.3f", 0x00003, 0x10000, CRC(74ee0609) SHA1(2fa547a17dc14d4d37f120cc7c7e90cd85460160) )
	//ROM_LOAD32_BYTE( "775a07b.6e", 0x40000, 0x10000, CRC(ff057896) SHA1(00ec1f6300478a5fc5bcc0187a6dc826664c5322) )
	//ROM_LOAD32_BYTE( "775a07f.5e", 0x40001, 0x10000, CRC(8488e6ba) SHA1(a6e693aaf2e30fd0c2130d4ba30feb43e208b279) )
	//ROM_LOAD32_BYTE( "775a08b.4e", 0x40002, 0x10000, CRC(55d79ef4) SHA1(f4b2de68c38b4346b9a4a56578cfe4069caaa3a3) )
	//ROM_LOAD32_BYTE( "775a08f.3e", 0x40003, 0x10000, CRC(727aadc1) SHA1(884d11104e3b5795654999b2d07eac12e0931800) )
	ROM_LOAD32_BYTE( "775-f07c.6d",  0x80000, 0x10000, CRC(b0b30915) SHA1(0abd858f93f7cc5383a805a5ae06c086c120f208) )
	ROM_LOAD32_BYTE( "775-f07g.5d",  0x80001, 0x10000, CRC(fbed827d) SHA1(7fcc6cc03ab6238b05799dd50f38c29eb9f98b5a) )
	ROM_LOAD32_BYTE( "775-f08c.4d",  0x80002, 0x10000, CRC(53abdaec) SHA1(0e0f7fe4bb9139a1ae94506a832153b711961564) )
	ROM_LOAD32_BYTE( "775-f08g.3d",  0x80003, 0x10000, CRC(3df85a6e) SHA1(25a49abbf6e9fe63d4ff6bfff9219c98aa1b5e7b) )
	ROM_LOAD32_BYTE( "775-f07d.7f",  0xc0000, 0x10000, CRC(f184be8e) SHA1(c266be12762f7e81edbe4b36f3c96b03f6ec552b) )
	ROM_LOAD32_BYTE( "775-f07h.7e",  0xc0001, 0x10000, CRC(7b56c348) SHA1(f75c1c0962389f204c8cf1a0bc2da01a922cd742) )
	ROM_LOAD32_BYTE( "775-f08d.7d",  0xc0002, 0x10000, CRC(102dcace) SHA1(03036b6d9d66a12cb3e97980f149c09d1efbd6d8) )
	ROM_LOAD32_BYTE( "775-f08h.7c",  0xc0003, 0x10000, CRC(ad9d7016) SHA1(91e9f279b781eefcafffc70afe207f35cc6f4d9d) )

	ROM_REGION( 0x100000, "k051960", 0 )        /* sprites */
	ROM_LOAD32_BYTE( "775-a05a.11f", 0x00000, 0x10000, CRC(a0767045) SHA1(e6df0731a9fb3b3d918607de81844e1f9353aac7) )
	ROM_LOAD32_BYTE( "775-a05e.10f", 0x00001, 0x10000, CRC(2f656f08) SHA1(140e7948c45d27c6705622d588a65b59ebcc624c) )
	ROM_LOAD32_BYTE( "775-a06a.9f",  0x00002, 0x10000, CRC(77a34ad0) SHA1(3653fb8458c1e7eb7d83b5cd63f02343c0f2d93e) )
	ROM_LOAD32_BYTE( "775-a06e.8f",  0x00003, 0x10000, CRC(8a910c94) SHA1(0387a7f412a977fa7a5ca685653ac1bb3dfdbbcb) )
	ROM_LOAD32_BYTE( "775-a05b.11e", 0x40000, 0x10000, CRC(ab8ad4fd) SHA1(c9ae537fa1607fbd11403390d1da923955f0d1ab) )
	ROM_LOAD32_BYTE( "775-a05f.10e", 0x40001, 0x10000, CRC(1c0eb1b6) SHA1(420eb26acd54ff484301aa2dad587f1b6b437363) )
	ROM_LOAD32_BYTE( "775-a06b.9e",  0x40002, 0x10000, CRC(563fb565) SHA1(96a2a95ab02456e53651718a7080f18c252451c8) )
	ROM_LOAD32_BYTE( "775-a06f.8e",  0x40003, 0x10000, CRC(e14995c0) SHA1(1d7fdfb8f9eacb005b0897b2b62b85ce334cd4d6) )
	ROM_LOAD32_BYTE( "775-f05c.11d", 0x80000, 0x10000, CRC(5647761e) SHA1(ff7983cb0c2f84f7be9d44e20b01266db4b2836a) )
	ROM_LOAD32_BYTE( "775-f05g.10d", 0x80001, 0x10000, CRC(a1692cca) SHA1(2cefc4b7532a9d29361843419ee427fb9421b79b) )
	ROM_LOAD32_BYTE( "775-f06c.9d",  0x80002, 0x10000, CRC(5ee6f3c1) SHA1(9138ea3588b63862849f6e783725a711e7e50669) )
	ROM_LOAD32_BYTE( "775-f06g.8d",  0x80003, 0x10000, CRC(2645274d) SHA1(2fd04b0adbcf53562669946259b59f1ec9c52bda) )
	ROM_LOAD32_BYTE( "775-f05d.11c", 0xc0000, 0x10000, CRC(ad676a6f) SHA1(f2ca759c8c8a8007aa022d6c058d0431057a639a) )
	ROM_LOAD32_BYTE( "775-f05h.10c", 0xc0001, 0x10000, CRC(3f925bcf) SHA1(434dd442c0cb5c5c039a69683a3a5f226e49261c) )
	ROM_LOAD32_BYTE( "775-f06d.9c",  0xc0002, 0x10000, CRC(c8b764fa) SHA1(62f7f59ed36dca7346ec9eb019a4e435e8476dc6) )
	ROM_LOAD32_BYTE( "775-f06h.8c",  0xc0003, 0x10000, CRC(d6595f59) SHA1(777ea6da2026c90e7fbbc598275c8f95f2eb99c2) )

	ROM_REGION( 0x80000, "k007232", 0 ) /* k007232 data */
	ROM_LOAD( "775-a04a.11b", 0x00000, 0x10000, CRC(7efb2e0f) SHA1(fb350a056b547fe4f981bc211e2f9518ae5a3499) )
	ROM_LOAD( "775-a04b.10b", 0x10000, 0x10000, CRC(f41a2b33) SHA1(dffa06360b6032f7370fe72698aacad4d8779472) )
	ROM_LOAD( "775-a04c.9b",  0x20000, 0x10000, CRC(e4e58f14) SHA1(23dcb4dfa9a44115d1b730d9efcc314801b811c7) )
	ROM_LOAD( "775-a04d.8b",  0x30000, 0x10000, CRC(d46736f6) SHA1(586e914a35d3d7a71cccec66ca45a5bbbb9e504b) )
	ROM_LOAD( "775-f04e.11a", 0x40000, 0x10000, CRC(fbf7e363) SHA1(53578eb7dab8f723439dc12eefade3edb027c148) )
	ROM_LOAD( "775-f04f.10a", 0x50000, 0x10000, CRC(b031ef2d) SHA1(0124fe15871c3972ef1e2dbaf53d17668c1dccfd) )
	ROM_LOAD( "775-f04g.9a",  0x60000, 0x10000, CRC(ee107bbb) SHA1(e21de761a0dfd3811ddcbc33d8868479010e86d0) )
	ROM_LOAD( "775-f04h.8a",  0x70000, 0x10000, CRC(fb0fab46) SHA1(fcbf904f7cf4d265352dc73ed228390b29784aad) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "775a09.b19",   0x0000, 0x0100, CRC(46d1e0df) SHA1(65dad04a124cc49cbc9bb271f865d77efbc4d57c) )    /* priority encoder (not used) */
ROM_END

ROM_START( scontraj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "775-f02.bin", 0x00000, 0x10000, CRC(8d5933a7) SHA1(e13ec62a4209b790b609429d98620ec0d07bd0ee) )   /* banked + fixed ROM */
	ROM_LOAD( "775-f03.bin", 0x10000, 0x10000, CRC(1ef63d80) SHA1(8fa41038ec2928f9572d0d4511a4bb3a3d8de06d) )   /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* 64k for the SOUND CPU */
	ROM_LOAD( "775-c01.bin", 0x00000, 0x08000, CRC(0ced785a) SHA1(1eebe005a968fbaac595c168499107e34763976c) )

	ROM_REGION( 0x100000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "775-a07a.bin", 0x00000, 0x20000, CRC(e716bdf3) SHA1(82e10132f248aed8cc1aea6bb7afe9a1479c8b59) )
	ROM_LOAD32_BYTE( "775-a07e.bin", 0x00001, 0x20000, CRC(0986e3a5) SHA1(61c33a3f2e4fde7d23d440b5c3151fe38e25716b) )
	ROM_LOAD32_BYTE( "775-a08a.bin", 0x00002, 0x20000, CRC(3ddd11a4) SHA1(4831a891d6cb4507053d576eddd658c338318176) )
	ROM_LOAD32_BYTE( "775-a08e.bin", 0x00003, 0x20000, CRC(1007d963) SHA1(cba4ca058dee1c8cdeb019e1cc50cae76bf419a1) )
	ROM_LOAD32_BYTE( "775-f07c.bin", 0x80000, 0x10000, CRC(b0b30915) SHA1(0abd858f93f7cc5383a805a5ae06c086c120f208) )
	ROM_LOAD32_BYTE( "775-f07g.bin", 0x80001, 0x10000, CRC(fbed827d) SHA1(7fcc6cc03ab6238b05799dd50f38c29eb9f98b5a) )
	ROM_LOAD32_BYTE( "775-f08c.bin", 0x80002, 0x10000, CRC(53abdaec) SHA1(0e0f7fe4bb9139a1ae94506a832153b711961564) )
	ROM_LOAD32_BYTE( "775-f08g.bin", 0x80003, 0x10000, CRC(3df85a6e) SHA1(25a49abbf6e9fe63d4ff6bfff9219c98aa1b5e7b) )
	ROM_LOAD32_BYTE( "775-f07d.bin", 0xc0000, 0x10000, CRC(f184be8e) SHA1(c266be12762f7e81edbe4b36f3c96b03f6ec552b) )
	ROM_LOAD32_BYTE( "775-f07h.bin", 0xc0001, 0x10000, CRC(7b56c348) SHA1(f75c1c0962389f204c8cf1a0bc2da01a922cd742) )
	ROM_LOAD32_BYTE( "775-f08d.bin", 0xc0002, 0x10000, CRC(102dcace) SHA1(03036b6d9d66a12cb3e97980f149c09d1efbd6d8) )
	ROM_LOAD32_BYTE( "775-f08h.bin", 0xc0003, 0x10000, CRC(ad9d7016) SHA1(91e9f279b781eefcafffc70afe207f35cc6f4d9d) )

	ROM_REGION( 0x100000, "k051960", 0 )        /* sprites */
	ROM_LOAD32_BYTE( "775-a05a.bin", 0x00000, 0x10000, CRC(a0767045) SHA1(e6df0731a9fb3b3d918607de81844e1f9353aac7) )
	ROM_LOAD32_BYTE( "775-a05e.bin", 0x00001, 0x10000, CRC(2f656f08) SHA1(140e7948c45d27c6705622d588a65b59ebcc624c) )
	ROM_LOAD32_BYTE( "775-a06a.bin", 0x00002, 0x10000, CRC(77a34ad0) SHA1(3653fb8458c1e7eb7d83b5cd63f02343c0f2d93e) )
	ROM_LOAD32_BYTE( "775-a06e.bin", 0x00003, 0x10000, CRC(8a910c94) SHA1(0387a7f412a977fa7a5ca685653ac1bb3dfdbbcb) )
	ROM_LOAD32_BYTE( "775-a05b.bin", 0x40000, 0x10000, CRC(ab8ad4fd) SHA1(c9ae537fa1607fbd11403390d1da923955f0d1ab) )
	ROM_LOAD32_BYTE( "775-a05f.bin", 0x40001, 0x10000, CRC(1c0eb1b6) SHA1(420eb26acd54ff484301aa2dad587f1b6b437363) )
	ROM_LOAD32_BYTE( "775-a06b.bin", 0x40002, 0x10000, CRC(563fb565) SHA1(96a2a95ab02456e53651718a7080f18c252451c8) )
	ROM_LOAD32_BYTE( "775-a06f.bin", 0x40003, 0x10000, CRC(e14995c0) SHA1(1d7fdfb8f9eacb005b0897b2b62b85ce334cd4d6) )
	ROM_LOAD32_BYTE( "775-f05c.bin", 0x80000, 0x10000, CRC(5647761e) SHA1(ff7983cb0c2f84f7be9d44e20b01266db4b2836a) )
	ROM_LOAD32_BYTE( "775-f05g.bin", 0x80001, 0x10000, CRC(a1692cca) SHA1(2cefc4b7532a9d29361843419ee427fb9421b79b) )
	ROM_LOAD32_BYTE( "775-f06c.bin", 0x80002, 0x10000, CRC(5ee6f3c1) SHA1(9138ea3588b63862849f6e783725a711e7e50669) )
	ROM_LOAD32_BYTE( "775-f06g.bin", 0x80003, 0x10000, CRC(2645274d) SHA1(2fd04b0adbcf53562669946259b59f1ec9c52bda) )
	ROM_LOAD32_BYTE( "775-f05d.bin", 0xc0000, 0x10000, CRC(ad676a6f) SHA1(f2ca759c8c8a8007aa022d6c058d0431057a639a) )
	ROM_LOAD32_BYTE( "775-f05h.bin", 0xc0001, 0x10000, CRC(3f925bcf) SHA1(434dd442c0cb5c5c039a69683a3a5f226e49261c) )
	ROM_LOAD32_BYTE( "775-f06d.bin", 0xc0002, 0x10000, CRC(c8b764fa) SHA1(62f7f59ed36dca7346ec9eb019a4e435e8476dc6) )
	ROM_LOAD32_BYTE( "775-f06h.bin", 0xc0003, 0x10000, CRC(d6595f59) SHA1(777ea6da2026c90e7fbbc598275c8f95f2eb99c2) )

	ROM_REGION( 0x80000, "k007232", 0 ) /* k007232 data */
	ROM_LOAD( "775-a04a.bin", 0x00000, 0x10000, CRC(7efb2e0f) SHA1(fb350a056b547fe4f981bc211e2f9518ae5a3499) )
	ROM_LOAD( "775-a04b.bin", 0x10000, 0x10000, CRC(f41a2b33) SHA1(dffa06360b6032f7370fe72698aacad4d8779472) )
	ROM_LOAD( "775-a04c.bin", 0x20000, 0x10000, CRC(e4e58f14) SHA1(23dcb4dfa9a44115d1b730d9efcc314801b811c7) )
	ROM_LOAD( "775-a04d.bin", 0x30000, 0x10000, CRC(d46736f6) SHA1(586e914a35d3d7a71cccec66ca45a5bbbb9e504b) )
	ROM_LOAD( "775-f04e.bin", 0x40000, 0x10000, CRC(fbf7e363) SHA1(53578eb7dab8f723439dc12eefade3edb027c148) )
	ROM_LOAD( "775-f04f.bin", 0x50000, 0x10000, CRC(b031ef2d) SHA1(0124fe15871c3972ef1e2dbaf53d17668c1dccfd) )
	ROM_LOAD( "775-f04g.bin", 0x60000, 0x10000, CRC(ee107bbb) SHA1(e21de761a0dfd3811ddcbc33d8868479010e86d0) )
	ROM_LOAD( "775-f04h.bin", 0x70000, 0x10000, CRC(fb0fab46) SHA1(fcbf904f7cf4d265352dc73ed228390b29784aad) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "775a09.b19",   0x0000, 0x0100, CRC(46d1e0df) SHA1(65dad04a124cc49cbc9bb271f865d77efbc4d57c) )    /* priority encoder (not used) */
ROM_END

ROM_START( scontraa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "cpu 27c512.k11",     0x00000, 0x10000, CRC(7f2b8001) SHA1(bc741f7c26d6852e90ac691b0daac4cca8254727) )   // banked + fixed ROM
	ROM_LOAD( "cpu 27c512.k13",     0x10000, 0x10000, CRC(2d65c313) SHA1(35bb6f657f054a4711e0ff80df8e2feb3551ed7f) )   // banked ROM

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound_27c256.f9",    0x00000, 0x08000, CRC(0ced785a) SHA1(1eebe005a968fbaac595c168499107e34763976c) )

	ROM_REGION( 0x100000, "k052109", 0 )   // tiles
	ROM_LOAD32_WORD( "775f07.h22", 0x00000, 0x80000, CRC(0e75d2e1) SHA1(10094547576938e7c8ff2e65533354b74b1d3c87) )
	ROM_LOAD32_WORD( "775f08.k22", 0x00002, 0x80000, CRC(d4f2ed1e) SHA1(d7c92828b1f88a6651b61757774d2878393cd694) )

	ROM_REGION( 0x100000, "k051960", 0 )   // sprite
	ROM_LOAD32_WORD( "775f05.h4", 0x00000, 0x80000, CRC(d1c788b0) SHA1(ea178e2f46f97a5332c0b98a64703ad807309a93) )
	ROM_LOAD32_WORD( "775f06.k4", 0x00002, 0x80000, CRC(623a9c9b) SHA1(90595b4db69c4e4027f34989bcffbced0985d7b1) )

	ROM_REGION( 0x80000, "k007232", 0 )
	ROM_LOAD( "sound-775f04.d4", 0x00000, 0x80000, CRC(0447dbae) SHA1(6f356416f18ae3119670432a89f8f1a44568b283) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "775a09.b19",   0x0000, 0x0100, CRC(46d1e0df) SHA1(65dad04a124cc49cbc9bb271f865d77efbc4d57c) )    // priority encoder (not used)
ROM_END

ROM_START( thunderx )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "873-s02.k13", 0x00000, 0x10000, CRC(6619333a) SHA1(1961658d528b0870c57f1cb78e016fb881f50392) )   /* banked + fixed ROM */
	ROM_LOAD( "873-s03.k15", 0x10000, 0x10000, CRC(2aec2699) SHA1(8f52703a6a1ba6417c484925192ce697af9c73f1) )   /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "873-f01.f8",   0x0000, 0x8000, CRC(ea35ffa3) SHA1(91e82b77d4f3af8238fb198db26182bebc5026e4) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "873c06a.f6",   0x00000, 0x10000, CRC(0e340b67) SHA1(a76b1ee4bd4c99826a02b63a705447d0ba4e7b01) )
	ROM_LOAD32_BYTE( "873c06c.f5",   0x00001, 0x10000, CRC(ef0e72cd) SHA1(85b77a303378386f2d395da8707f4b638d37833e) )
	ROM_LOAD32_BYTE( "873c07a.f4",   0x00002, 0x10000, CRC(a8aab84f) SHA1(a68521a9abf45c3292b3090a2483edbf31356c7d) )
	ROM_LOAD32_BYTE( "873c07c.f3",   0x00003, 0x10000, CRC(2521009a) SHA1(6546b88943615389c81b753ff5bb6aa9378c3266) )
	ROM_LOAD32_BYTE( "873c06b.e6",   0x40000, 0x10000, CRC(97ad202e) SHA1(fd155aeb691814950711ead3bc2c93c67b7b0434) )
	ROM_LOAD32_BYTE( "873c06d.e5",   0x40001, 0x10000, CRC(8393d42e) SHA1(ffcb5eca3f58994e05c49d803fa4831c0213e2e2) )
	ROM_LOAD32_BYTE( "873c07b.e4",   0x40002, 0x10000, CRC(12a2b8ba) SHA1(ffa32ca116e0b6ca65bb9ce83dd28f5c027956a5) )
	ROM_LOAD32_BYTE( "873c07d.e3",   0x40003, 0x10000, CRC(fae9f965) SHA1(780c234507835c37bde445ab34f069714cc7a506) )

	ROM_REGION( 0x80000, "k051960", 0 )     /* sprites */
	ROM_LOAD32_BYTE( "873c04a.f11",  0x00000, 0x10000, CRC(f7740bf3) SHA1(f64b7e807f19a9523a517024a9eb56736cdda6bb) )
	ROM_LOAD32_BYTE( "873c04c.f10",  0x00001, 0x10000, CRC(5dacbd2b) SHA1(deb943b99fd296d20be9c4250b2348549f65ba37) )
	ROM_LOAD32_BYTE( "873c05a.f9",   0x00002, 0x10000, CRC(d73e107d) SHA1(ba63b195e20a98c476e7d0f8d0187bc3327a8822) )
	ROM_LOAD32_BYTE( "873c05c.f8",   0x00003, 0x10000, CRC(59903200) SHA1(d076802c53aa604df8c5fdd33cb41876ba2a3385) )
	ROM_LOAD32_BYTE( "873c04b.e11",  0x40000, 0x10000, CRC(9ac581da) SHA1(fd0a603de8586621444055bbff8bb83349b8a0d8) )
	ROM_LOAD32_BYTE( "873c04d.e10",  0x40001, 0x10000, CRC(44a4668c) SHA1(6d1526ed3408ddc763a071604e7b1e0773c87b99) )
	ROM_LOAD32_BYTE( "873c05b.e9",   0x40002, 0x10000, CRC(81059b99) SHA1(1e1a22ca45599abe0dce32fc0b188281deb3b8ac) )
	ROM_LOAD32_BYTE( "873c05d.e8",   0x40003, 0x10000, CRC(7fa3d7df) SHA1(c78b9a949abdf44366d872daa1f2041158fae790) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "873a08.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) )    /* priority encoder (not used) */
ROM_END

ROM_START( thunderxa ) /* Alternate Starting stage then the other 2 sets, Perhaps a US set? */
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "873-k02.k13", 0x00000, 0x10000, CRC(80cc1c45) SHA1(881bc6eea94671e8c3fdb7a10b0e742b18cb7212) )   /* banked + fixed ROM */
	ROM_LOAD( "873-k03.k15", 0x10000, 0x10000, CRC(276817ad) SHA1(34b1beecf2a4c54dd7cd150c5d83b44f67be288a) )   /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "873-h01.f8",    0x0000, 0x8000, CRC(990b7a7c) SHA1(0965e7350c6006a9652cea0f24d836b4979910fd) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "873c06a.f6",   0x00000, 0x10000, CRC(0e340b67) SHA1(a76b1ee4bd4c99826a02b63a705447d0ba4e7b01) )
	ROM_LOAD32_BYTE( "873c06c.f5",   0x00001, 0x10000, CRC(ef0e72cd) SHA1(85b77a303378386f2d395da8707f4b638d37833e) )
	ROM_LOAD32_BYTE( "873c07a.f4",   0x00002, 0x10000, CRC(a8aab84f) SHA1(a68521a9abf45c3292b3090a2483edbf31356c7d) )
	ROM_LOAD32_BYTE( "873c07c.f3",   0x00003, 0x10000, CRC(2521009a) SHA1(6546b88943615389c81b753ff5bb6aa9378c3266) )
	ROM_LOAD32_BYTE( "873c06b.e6",   0x40000, 0x10000, CRC(97ad202e) SHA1(fd155aeb691814950711ead3bc2c93c67b7b0434) )
	ROM_LOAD32_BYTE( "873c06d.e5",   0x40001, 0x10000, CRC(8393d42e) SHA1(ffcb5eca3f58994e05c49d803fa4831c0213e2e2) )
	ROM_LOAD32_BYTE( "873c07b.e4",   0x40002, 0x10000, CRC(12a2b8ba) SHA1(ffa32ca116e0b6ca65bb9ce83dd28f5c027956a5) )
	ROM_LOAD32_BYTE( "873c07d.e3",   0x40003, 0x10000, CRC(fae9f965) SHA1(780c234507835c37bde445ab34f069714cc7a506) )

	ROM_REGION( 0x80000, "k051960", 0 )     /* sprites */
	ROM_LOAD32_BYTE( "873c04a.f11",  0x00000, 0x10000, CRC(f7740bf3) SHA1(f64b7e807f19a9523a517024a9eb56736cdda6bb) )
	ROM_LOAD32_BYTE( "873c04c.f10",  0x00001, 0x10000, CRC(5dacbd2b) SHA1(deb943b99fd296d20be9c4250b2348549f65ba37) )
	ROM_LOAD32_BYTE( "873c05a.f9",   0x00002, 0x10000, CRC(d73e107d) SHA1(ba63b195e20a98c476e7d0f8d0187bc3327a8822) )
	ROM_LOAD32_BYTE( "873c05c.f8",   0x00003, 0x10000, CRC(59903200) SHA1(d076802c53aa604df8c5fdd33cb41876ba2a3385) )
	ROM_LOAD32_BYTE( "873c04b.e11",  0x40000, 0x10000, CRC(9ac581da) SHA1(fd0a603de8586621444055bbff8bb83349b8a0d8) )
	ROM_LOAD32_BYTE( "873c04d.e10",  0x40001, 0x10000, CRC(44a4668c) SHA1(6d1526ed3408ddc763a071604e7b1e0773c87b99) )
	ROM_LOAD32_BYTE( "873c05b.e9",   0x40002, 0x10000, CRC(81059b99) SHA1(1e1a22ca45599abe0dce32fc0b188281deb3b8ac) )
	ROM_LOAD32_BYTE( "873c05d.e8",   0x40003, 0x10000, CRC(7fa3d7df) SHA1(c78b9a949abdf44366d872daa1f2041158fae790) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "873a08.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) )    /* priority encoder (not used) */
ROM_END

ROM_START( thunderxb ) /* Set had no labels, same starting stage as parent set */
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "873-02.k13", 0x00000, 0x10000, CRC(c58b2c34) SHA1(4050d2edc579ffedba3d40782a08e43ac89b1b86) )   /* banked + fixed ROM */
	ROM_LOAD( "873-03.k15", 0x10000, 0x10000, CRC(36680a4e) SHA1(9b3b6bf75a9c04e764448cd958277bd081cc4a53) )   /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "873-f01.f8",   0x0000, 0x8000, CRC(ea35ffa3) SHA1(91e82b77d4f3af8238fb198db26182bebc5026e4) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "873c06a.f6",   0x00000, 0x10000, CRC(0e340b67) SHA1(a76b1ee4bd4c99826a02b63a705447d0ba4e7b01) )
	ROM_LOAD32_BYTE( "873c06c.f5",   0x00001, 0x10000, CRC(ef0e72cd) SHA1(85b77a303378386f2d395da8707f4b638d37833e) )
	ROM_LOAD32_BYTE( "873c07a.f4",   0x00002, 0x10000, CRC(a8aab84f) SHA1(a68521a9abf45c3292b3090a2483edbf31356c7d) )
	ROM_LOAD32_BYTE( "873c07c.f3",   0x00003, 0x10000, CRC(2521009a) SHA1(6546b88943615389c81b753ff5bb6aa9378c3266) )
	ROM_LOAD32_BYTE( "873c06b.e6",   0x40000, 0x10000, CRC(97ad202e) SHA1(fd155aeb691814950711ead3bc2c93c67b7b0434) )
	ROM_LOAD32_BYTE( "873c06d.e5",   0x40001, 0x10000, CRC(8393d42e) SHA1(ffcb5eca3f58994e05c49d803fa4831c0213e2e2) )
	ROM_LOAD32_BYTE( "873c07b.e4",   0x40002, 0x10000, CRC(12a2b8ba) SHA1(ffa32ca116e0b6ca65bb9ce83dd28f5c027956a5) )
	ROM_LOAD32_BYTE( "873c07d.e3",   0x40003, 0x10000, CRC(fae9f965) SHA1(780c234507835c37bde445ab34f069714cc7a506) )

	ROM_REGION( 0x80000, "k051960", 0 )     /* sprites */
	ROM_LOAD32_BYTE( "873c04a.f11",  0x00000, 0x10000, CRC(f7740bf3) SHA1(f64b7e807f19a9523a517024a9eb56736cdda6bb) )
	ROM_LOAD32_BYTE( "873c04c.f10",  0x00001, 0x10000, CRC(5dacbd2b) SHA1(deb943b99fd296d20be9c4250b2348549f65ba37) )
	ROM_LOAD32_BYTE( "873c05a.f9",   0x00002, 0x10000, CRC(d73e107d) SHA1(ba63b195e20a98c476e7d0f8d0187bc3327a8822) )
	ROM_LOAD32_BYTE( "873c05c.f8",   0x00003, 0x10000, CRC(59903200) SHA1(d076802c53aa604df8c5fdd33cb41876ba2a3385) )
	ROM_LOAD32_BYTE( "873c04b.e11",  0x40000, 0x10000, CRC(9ac581da) SHA1(fd0a603de8586621444055bbff8bb83349b8a0d8) )
	ROM_LOAD32_BYTE( "873c04d.e10",  0x40001, 0x10000, CRC(44a4668c) SHA1(6d1526ed3408ddc763a071604e7b1e0773c87b99) )
	ROM_LOAD32_BYTE( "873c05b.e9",   0x40002, 0x10000, CRC(81059b99) SHA1(1e1a22ca45599abe0dce32fc0b188281deb3b8ac) )
	ROM_LOAD32_BYTE( "873c05d.e8",   0x40003, 0x10000, CRC(7fa3d7df) SHA1(c78b9a949abdf44366d872daa1f2041158fae790) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "873a08.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) )    /* priority encoder (not used) */
ROM_END

ROM_START( thunderxj )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "873-n02.k13", 0x00000, 0x10000, CRC(55afa2cc) SHA1(5fb9df0c7c7c0c2029dbe0f3c1e0340234a03e8a) )   /* banked + fixed ROM */
	ROM_LOAD( "873-n03.k15", 0x10000, 0x10000, CRC(a01e2e3e) SHA1(eba0d95dc0c5eed18743a96e4bbda5e60d5d9c97) )   /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "873-f01.f8",   0x0000, 0x8000, CRC(ea35ffa3) SHA1(91e82b77d4f3af8238fb198db26182bebc5026e4) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_BYTE( "873c06a.f6",   0x00000, 0x10000, CRC(0e340b67) SHA1(a76b1ee4bd4c99826a02b63a705447d0ba4e7b01) )
	ROM_LOAD32_BYTE( "873c06c.f5",   0x00001, 0x10000, CRC(ef0e72cd) SHA1(85b77a303378386f2d395da8707f4b638d37833e) )
	ROM_LOAD32_BYTE( "873c07a.f4",   0x00002, 0x10000, CRC(a8aab84f) SHA1(a68521a9abf45c3292b3090a2483edbf31356c7d) )
	ROM_LOAD32_BYTE( "873c07c.f3",   0x00003, 0x10000, CRC(2521009a) SHA1(6546b88943615389c81b753ff5bb6aa9378c3266) )
	ROM_LOAD32_BYTE( "873c06b.e6",   0x40000, 0x10000, CRC(97ad202e) SHA1(fd155aeb691814950711ead3bc2c93c67b7b0434) )
	ROM_LOAD32_BYTE( "873c06d.e5",   0x40001, 0x10000, CRC(8393d42e) SHA1(ffcb5eca3f58994e05c49d803fa4831c0213e2e2) )
	ROM_LOAD32_BYTE( "873c07b.e4",   0x40002, 0x10000, CRC(12a2b8ba) SHA1(ffa32ca116e0b6ca65bb9ce83dd28f5c027956a5) )
	ROM_LOAD32_BYTE( "873c07d.e3",   0x40003, 0x10000, CRC(fae9f965) SHA1(780c234507835c37bde445ab34f069714cc7a506) )

	ROM_REGION( 0x80000, "k051960", 0 )     /* sprites */
	ROM_LOAD32_BYTE( "873c04a.f11",  0x00000, 0x10000, CRC(f7740bf3) SHA1(f64b7e807f19a9523a517024a9eb56736cdda6bb) )
	ROM_LOAD32_BYTE( "873c04c.f10",  0x00001, 0x10000, CRC(5dacbd2b) SHA1(deb943b99fd296d20be9c4250b2348549f65ba37) )
	ROM_LOAD32_BYTE( "873c05a.f9",   0x00002, 0x10000, CRC(d73e107d) SHA1(ba63b195e20a98c476e7d0f8d0187bc3327a8822) )
	ROM_LOAD32_BYTE( "873c05c.f8",   0x00003, 0x10000, CRC(59903200) SHA1(d076802c53aa604df8c5fdd33cb41876ba2a3385) )
	ROM_LOAD32_BYTE( "873c04b.e11",  0x40000, 0x10000, CRC(9ac581da) SHA1(fd0a603de8586621444055bbff8bb83349b8a0d8) )
	ROM_LOAD32_BYTE( "873c04d.e10",  0x40001, 0x10000, CRC(44a4668c) SHA1(6d1526ed3408ddc763a071604e7b1e0773c87b99) )
	ROM_LOAD32_BYTE( "873c05b.e9",   0x40002, 0x10000, CRC(81059b99) SHA1(1e1a22ca45599abe0dce32fc0b188281deb3b8ac) )
	ROM_LOAD32_BYTE( "873c05d.e8",   0x40003, 0x10000, CRC(7fa3d7df) SHA1(c78b9a949abdf44366d872daa1f2041158fae790) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "873a08.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) )    /* priority encoder (not used) */
ROM_END

ROM_START( gbusters )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "878n02.k13", 0x00000, 0x10000, CRC(51697aaa) SHA1(1e6461e2e5e871d44085623a890158a4c1c4c404) )   /* banked + fixed ROM */
	ROM_LOAD( "878j03.k15", 0x10000, 0x10000, CRC(3943a065) SHA1(6b0863f4182e6c973adfaa618f096bd4cc9b7b6d) )   /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "878h01.f8", 0x00000, 0x08000, CRC(96feafaa) SHA1(8b6547e610cb4fa1c1f5bf12cb05e9a12a353903) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "878c07.h27", 0x00000, 0x40000, CRC(eeed912c) SHA1(b2e27610b38f3fc9c2cdad600b03c8bae4fb9138) )
	ROM_LOAD32_WORD( "878c08.k27", 0x00002, 0x40000, CRC(4d14626d) SHA1(226b1d83fb82586302be0a67737a427475856537) )

	ROM_REGION( 0x80000, "k051960", 0 )     /* sprites */
	ROM_LOAD32_WORD( "878c05.h5", 0x00000, 0x40000, CRC(01f4aea5) SHA1(124123823be6bd597805484539d821aaaadde2c0) )
	ROM_LOAD32_WORD( "878c06.k5", 0x00002, 0x40000, CRC(edfaaaaf) SHA1(67468c4ce47e8d43d58de8d3b50b048c66508156) )

	ROM_REGION( 0x40000, "k007232", 0 ) /* samples for 007232 */
	ROM_LOAD( "878c04.d5",  0x00000, 0x40000, CRC(9e982d1c) SHA1(a5b611c67b0f2ac50c679707931ee12ebbf72ebe) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "878a09.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) ) /* priority encoder (not used) */
ROM_END

ROM_START( gbustersa )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "878_02.k13", 0x00000, 0x10000, CRC(57178414) SHA1(89b1403158f6ce18706c8a941109554d03cf77d9) ) /* unknown region/version leter */
	ROM_LOAD( "878_03.k15", 0x10000, 0x10000, CRC(6c59e660) SHA1(66a92eb8a93c9f542489fa31bec6ed1819d174da) ) /* unknown region/version leter */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "878h01.f8", 0x00000, 0x08000, CRC(96feafaa) SHA1(8b6547e610cb4fa1c1f5bf12cb05e9a12a353903) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "878c07.h27", 0x00000, 0x40000, CRC(eeed912c) SHA1(b2e27610b38f3fc9c2cdad600b03c8bae4fb9138) )
	ROM_LOAD32_WORD( "878c08.k27", 0x00002, 0x40000, CRC(4d14626d) SHA1(226b1d83fb82586302be0a67737a427475856537) )

	ROM_REGION( 0x80000, "k051960", 0 )     /* sprites */
	ROM_LOAD32_WORD( "878c05.h5", 0x00000, 0x40000, CRC(01f4aea5) SHA1(124123823be6bd597805484539d821aaaadde2c0) )
	ROM_LOAD32_WORD( "878c06.k5", 0x00002, 0x40000, CRC(edfaaaaf) SHA1(67468c4ce47e8d43d58de8d3b50b048c66508156) )

	ROM_REGION( 0x40000, "k007232", 0 ) /* samples for 007232 */
	ROM_LOAD( "878c04.d5",  0x00000, 0x40000, CRC(9e982d1c) SHA1(a5b611c67b0f2ac50c679707931ee12ebbf72ebe) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "878a09.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) ) /* priority encoder (not used) */
ROM_END

ROM_START( crazycop )
	ROM_REGION( 0x20000, "maincpu", 0 ) /* banked program ROMs */
	ROM_LOAD( "878m02.k13", 0x00000, 0x10000, CRC(9c1c9f52) SHA1(7a60ad20aac92da8258b43b04f8c7f27bb71f1df) )   /* banked + fixed ROM */
	ROM_LOAD( "878j03.k15", 0x10000, 0x10000, CRC(3943a065) SHA1(6b0863f4182e6c973adfaa618f096bd4cc9b7b6d) )   /* banked ROM */

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for the sound CPU */
	ROM_LOAD( "878h01.f8", 0x00000, 0x08000, CRC(96feafaa) SHA1(8b6547e610cb4fa1c1f5bf12cb05e9a12a353903) )

	ROM_REGION( 0x80000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "878c07.h27", 0x00000, 0x40000, CRC(eeed912c) SHA1(b2e27610b38f3fc9c2cdad600b03c8bae4fb9138) )
	ROM_LOAD32_WORD( "878c08.k27", 0x00002, 0x40000, CRC(4d14626d) SHA1(226b1d83fb82586302be0a67737a427475856537) )

	ROM_REGION( 0x80000, "k051960", 0 )     /* sprites */
	ROM_LOAD32_WORD( "878c05.h5", 0x00000, 0x40000, CRC(01f4aea5) SHA1(124123823be6bd597805484539d821aaaadde2c0) )
	ROM_LOAD32_WORD( "878c06.k5", 0x00002, 0x40000, CRC(edfaaaaf) SHA1(67468c4ce47e8d43d58de8d3b50b048c66508156) )

	ROM_REGION( 0x40000, "k007232", 0 ) /* samples for 007232 */
	ROM_LOAD( "878c04.d5",  0x00000, 0x40000, CRC(9e982d1c) SHA1(a5b611c67b0f2ac50c679707931ee12ebbf72ebe) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "878a09.f20",   0x0000, 0x0100, CRC(e2d09a1b) SHA1(a9651e137486b2df367c39eb43f52d0833589e87) ) /* priority encoder (not used) */
ROM_END

} // anonymous namespace


/***************************************************************************/

GAME( 1988, scontra,   0,        scontra,  scontra,  scontra_state,  empty_init, ROT90, "Konami", "Super Contra (set 1)",  MACHINE_SUPPORTS_SAVE )
GAME( 1988, scontraa,  scontra,  scontra,  scontra,  scontra_state,  empty_init, ROT90, "Konami", "Super Contra (set 2)",  MACHINE_SUPPORTS_SAVE )
GAME( 1988, scontraj,  scontra,  scontra,  scontra,  scontra_state,  empty_init, ROT90, "Konami", "Super Contra - Alien no Gyakushuu (Japan)",  MACHINE_SUPPORTS_SAVE )
GAME( 1988, thunderx,  0,        thunderx, thunderx, thunderx_state, empty_init, ROT0,  "Konami", "Thunder Cross (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, thunderxa, thunderx, thunderx, thunderx, thunderx_state, empty_init, ROT0,  "Konami", "Thunder Cross (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, thunderxb, thunderx, thunderx, thunderx, thunderx_state, empty_init, ROT0,  "Konami", "Thunder Cross (set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, thunderxj, thunderx, thunderx, thnderxj, thunderx_state, empty_init, ROT0,  "Konami", "Thunder Cross (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, gbusters,  0,        gbusters, gbusters, scontra_state,  empty_init, ROT90, "Konami", "Gang Busters (set 1)",  MACHINE_SUPPORTS_SAVE ) // N02 & J03 program ROMs
GAME( 1988, gbustersa, gbusters, gbusters, gbusters, scontra_state,  empty_init, ROT90, "Konami", "Gang Busters (set 2)",  MACHINE_SUPPORTS_SAVE ) // unknown region program ROMs
GAME( 1988, crazycop,  gbusters, gbusters, gbusters, scontra_state,  empty_init, ROT90, "Konami", "Crazy Cop (Japan)",     MACHINE_SUPPORTS_SAVE ) // M02 & J03 program ROMs
