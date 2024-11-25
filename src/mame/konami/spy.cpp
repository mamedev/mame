// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    S.P.Y. (c) 1989 Konami

    Similar to Bottom of the Ninth

    driver by Nicola Salmoria


    Revisions:

    31-01-2024 Furrtek
    - updated PMCU collision check code to match what the original program does

    05-10-2002 Acho A. Tang
    - simulated PMCU protection(guess only)
    - changed priority scheme to fix graphics in 3D levels
    - fixed crashes caused by bank switching
    - disabled logging and debug messages

***************************************************************************/

#include "emu.h"

#include "k051960.h"
#include "k052109.h"
#include "konamipt.h"
#include "konami_helper.h"

#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/k007232.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "speaker.h"

#include "multibyte.h"


namespace {

class spy_state : public driver_device
{
public:
	spy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_rombank(*this, "rombank"),
		m_ram(*this, "ram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k052109(*this, "k052109"),
		m_k051960(*this, "k051960"),
		m_palette(*this, "palette")
	{ }

	void spy(machine_config &config);

private:
	/* memory pointers */
	required_memory_bank m_rombank;
	required_shared_ptr<uint8_t> m_ram;
	uint8_t m_pmcram[0x800]{};
	std::vector<uint8_t> m_paletteram{};

	/* misc */
	int        m_rambank = 0;
	int        m_pmcbank = 0;
	uint8_t    m_pmcpc = 0;
	bool       m_video_enable = false;
	int        m_old_3f90 = 0;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k007232_device> m_k007232_1;
	required_device<k007232_device> m_k007232_2;
	required_device<k052109_device> m_k052109;
	required_device<k051960_device> m_k051960;
	required_device<palette_device> m_palette;

	uint8_t spy_bankedram1_r(offs_t offset);
	void spy_bankedram1_w(offs_t offset, uint8_t data);
	void bankswitch_w(uint8_t data);
	void spy_3f90_w(uint8_t data);
	void spy_sh_irqtrigger_w(uint8_t data);
	void sound_bank_w(uint8_t data);
	uint8_t k052109_051960_r(offs_t offset);
	void k052109_051960_w(offs_t offset, uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_spy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void pmc_run();
	void volume_callback0(uint8_t data);
	void volume_callback1(uint8_t data);
	K052109_CB_MEMBER(tile_callback);
	K051960_CB_MEMBER(sprite_callback);

	void spy_map(address_map &map) ATTR_COLD;
	void spy_sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Callbacks for the K052109

***************************************************************************/

K052109_CB_MEMBER(spy_state::tile_callback)
{
	static const int layer_colorbase[] = { 768 / 16, 0 / 16, 256 / 16 };

	*flags = (*color & 0x20) ? TILE_FLIPX : 0;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
}


/***************************************************************************

  Callbacks for the K051960

***************************************************************************/

K051960_CB_MEMBER(spy_state::sprite_callback)
{
	enum { sprite_colorbase = 512 / 16 };

	/* bit 4 = priority over layer A (0 = have priority) */
	/* bit 5 = priority over layer B (1 = have priority) */
	*priority = 0x00;
	if ( *color & 0x10) *priority |= GFX_PMASK_1;
	if (~*color & 0x20) *priority |= GFX_PMASK_2;

	*color = sprite_colorbase + (*color & 0x0f);
}


/***************************************************************************

  Display refresh

***************************************************************************/

uint32_t spy_state::screen_update_spy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_k052109->tilemap_update();

	screen.priority().fill(0, cliprect);

	if (!m_video_enable)
		bitmap.fill(768, cliprect); // ?
	else
	{
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 1, TILEMAP_DRAW_OPAQUE, 1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
		m_k051960->k051960_sprites_draw(bitmap, cliprect, screen.priority(), -1, -1);
		m_k052109->tilemap_draw(screen, bitmap, cliprect, 0, 0, 0);
	}

	return 0;
}

uint8_t spy_state::spy_bankedram1_r(offs_t offset)
{
	if (m_rambank & 1)
	{
		return m_paletteram[offset];
	}
	else if (m_rambank & 2)
	{
		if (m_pmcbank)
		{
			//logerror("%04x read pmcram %04x\n",m_maincpu->pc(), offset);
			return m_pmcram[offset];
		}
		else
		{
			return 0;   // PMC internal RAM can't be read back
		}
	}
	else
		return m_ram[offset];
}

void spy_state::spy_bankedram1_w(offs_t offset, uint8_t data)
{
	if (m_rambank & 1)
	{
		m_palette->write8(offset,data);
	}
	else if (m_rambank & 2)
	{
		if (m_pmcbank)
		{
			//logerror("%04x pmcram %04x = %02x\n", m_maincpu->pc(), offset, data);
			m_pmcram[offset] = data;
		}
		else
		{
			// Set initial PMC PC
			m_pmcpc = data & 0x3f;
		}
	}
	else
		m_ram[offset] = data;
}

/*
This is the 052591 PMC code loaded at startup, it contains both projection and collision check programs.
See https://github.com/furrtek/SiliconRE/tree/master/Konami/052591 for details

Coordinate projection routine:
00: e7 7e 38 fc 08  Clear r7
01: df 36 38 dc 00  r3.w = RAM[0]
02: df 12 3a dc 00  r1.w = RAM[2]
03: df 00 38 dc 08  r0 = RAM[4]
04: 1f 7e 00 db 00
05: 26 fe 00 ff 0c  Acc.w = RAM[5],00
06: 89 03 34 fc 0d  1+16 division steps, CALL 34...
07: 81 03 34 fc 09  add/sub, CALL 34
08: 81 03 34 fc 09  add/sub, CALL 34
09: 81 03 34 fc 09  add/sub, CALL 34
0a: 81 03 2f fc 09  add/sub, CALL 34
0b: cc 36 0e d9 08  r3--, JP 0e if 0
0c: 84 7e 00 ab 0c
0d: 5f 7e 03 cd 08  JP 03

0e: 7f 80 fe ef 08  Set OUT0 low
0f: 5f 7e 0f fd 08  JP 0f, infinite loop

Collision check routine:
10: e7 7e 38 fc 08  r0.w = RAM[0]
11: df 00 3a dc 00
12: df 12 0e d9 08  r1 = RAM[2], JP 0e if 0
13: df ec 10 e0 0c  r6 = 10
14: 1f fe 03 e0 0c  Acc = 3
15: df fe 03 e0 0c  r7 = 3
16: dc 5e 3e fc 08  r5 = 3
17: df 12 2b d9 08  r1 = RAM[r6++], JP 2b if 0
18: 67 25 38 fc 0c  r2 = 8000 ?
19: df 12 3c dc 00
1a: df 36 00 db 00
1b: c1 14 00 fb 08  r1 += r2
1c: c1 34 38 fc 08  r3 += r2
1d: c5 22 37 dc 00
1e: cd 12 3c dc 04
1f: c5 46 3b dc 00
20: cd 36 00 db 04
21: 49 16 ed f9 0c  JP 2d if r1 < r3
22: c9 18 ea f9 0c  JP 2a if r1 < r4
23: dc 12 2a f9 08
24: cc 5a 26 f9 08  r5--, JP 26 if 0
25: 5f 7e 18 fd 08  JP 18
26: 5a 7e 32 f8 08  Clear RAM(r7++) if Acc is 0
27: 84 6c 33 9c 0c  RAM(r6++) = Acc
28: cc 00 0e d9 08  r0--, JP 0e if 0
29: 5f 7e 14 fd 08
2a: 0a 7e 24 fd 08
2b: c5 ec 0d e0 0c  r6 += d, next object
2c: 5f 7e 28 fd 08
2d: dc 16 00 fb 08  r1 = r3
2e: dc 44 22 fd 08  r4 = r2
2f: cd fe 02 e0 0c  r7 -= 3
30: 84 7e 00 bb 0c
31: 5a 7e 00 73 08
32: 84 7e 00 9b 0c
33: 5a 7e 00 36 08

34: 81 03 00 fb 09  add/sub
35: 81 03 00 fb 09  add/sub
36: 81 03 00 fe 09  add/sub, ret

37: cd fe 01 e0 0c  r7 -= 2
38: 84 7e 00 ab 0c
39: 5f 7e 00 db 00  Set MSB as RAM(r7)
3a: 84 7e 3f ad 0c
3b: cd ec 01 e0 0c  r6 -= 2
3c: 84 6c 00 ab 0c
3d: 5f 7e 00 db 00  Set MSB as RAM(r6)
3e: 84 6c 00 ab 0c
3f: 5f 7e 00 ce 08  ret
*/

void spy_state::bankswitch_w(uint8_t data)
{
	/* bit 0 = RAM bank */
	if ((data & 1) == 0)
		popmessage("bankswitch RAM bank 0");

	/* bit 1-4 = ROM bank */
	int bank;
	if (data & 0x10)
		bank = 8 + ((data & 0x06) >> 1);
	else
		bank = (data & 0x0e) >> 1;

	m_rombank->set_entry(bank);
}

void spy_state::pmc_run()
{
	constexpr uint16_t MAX_SPRITES = 64;
	constexpr uint16_t DEF_NEAR_PLANE = 0x6400;

	if (m_pmcpc == 0x00)
	{
		// Projection program
		// Basically divides a list of 16-bit words by a constant, results are 8.8 fixed point
		uint16_t loopend, nearplane;

		loopend = get_u16be(&m_pmcram[0]);
		nearplane = get_u16be(&m_pmcram[2]);

		// fail safe
		if (loopend > MAX_SPRITES)
			loopend = MAX_SPRITES;
		if (!nearplane)
			nearplane = DEF_NEAR_PLANE;

		loopend = (loopend << 1) + 4;

		for (uint16_t i = 4; i < loopend; i += 2)
		{
			uint32_t op = get_u16be(&m_pmcram[i]);
			op = (op << 8) / nearplane;
			put_u16be(&m_pmcram[i], op);
		}

		memset(m_pmcram + loopend, 0, 0x800 - loopend); // clean up for next frame
	}
	else
	{
		// Collision check program
		if (!m_pmcram[0x2])
			return;

		const uint16_t count = get_u16be(&m_pmcram[0]);

		for (uint16_t i = 16; i < 16 + (14 * count); i += 14)
		{
			if (!m_pmcram[i])
				continue;

			// Check all 3 dimensions
			uint8_t tests_failed = 3;
			for (uint16_t j = 0; j < 3 * 4; j += 4)
			{
				const int16_t a_pos = get_s16be(&m_pmcram[j + 3]);      // Object A center position
				const int16_t a_size = get_s16be(&m_pmcram[j + 5]);     // Object A half size

				const int16_t b_pos = get_s16be(&m_pmcram[i + j + 1]);  // Object B center position
				const int16_t b_size = get_s16be(&m_pmcram[i + j + 3]); // Object B half size

				const int16_t a_max = a_pos + a_size;   // Object A right edge
				const int16_t a_min = a_pos - a_size;   // Object A left edge
				const int16_t b_max = b_pos + b_size;   // Object B right edge
				const int16_t b_min = b_pos - b_size;   // Object B left edge

				if (b_min > a_min)
				{
					// Object B left edge is > object A left edge
					// Checks if A.left < B.left <= A.right
					if (a_max >= b_min)
						tests_failed--;
				}
				else
				{
					// Object B left edge is <= object A left edge
					// Checks if B.left <= A.left <= B.right
					if (b_max >= a_min)
						tests_failed--;
				}
			}

			// Mark objects as collided or not
			if (!tests_failed)
				m_pmcram[0xf] = 0;
			m_pmcram[i + 0xd] = tests_failed;
		}
	}
}


void spy_state::spy_3f90_w(uint8_t data)
{
	/*********************************************************************
	*
	* Signals, from schematic:
	*   Bit 0 - CTR1 0x01
	*   Bit 1 - CTR2 0x02
	*   Bit 2 - CHA-RD 0x04
	*   Bit 3 - TV-KILL 0x08  +TV-KILL & COLORBLK to pin 7 of
	*                                    052535 video chips
	*
	*   Bit 4 - COLORBK/RVBK 0x10
	*   Bit 5 - PMCBK 0x20  GX857 053180 PAL20P Pin 7 (MCE1)
	*   Bit 6 - PMC-START 0x40  PMC START
	*   Bit 7 - PMC-BK 0x80  PMC BK
	*
	*   PMC takes AB0-AB12, D0-D7 from 6809E, outputs EA0-EA10, ED0-ED7,
	*   tied to A and D bus of 2128SL
	*
	*   See "MCPU" page of S.P.Y schematics for more...
	*
	*    PMC ERWE -> ~WR of 2128SL
	*    PMC ERCS -> ~CE of 2128SL
	*    PMC EROE -> ~OE of 2128SL
	*
	*    PMCOUTO -> PMCFIRQ -> 6809E ~FIRQ and PORT4, bit 0x08
	*
	*   PMC selected by PMC/RVRAMCS signal: pin 16 of PAL20P 05318
	*
	*    AB0xC -> 0x1000, so if address & 0x1000, appears PMC is selected.
	*
	*   Other apparent selects:
	*
	*    0x0800 -> COLORCS (color enable?)
	*    0x2000 -> ~CS1 on 6264W
	*    0x4000 -> ~OE on S63 27512
	*    0x8000 -> ~OE on S22 27512
	*
	********************************************************************/

	/* bits 0/1 = coin counters */
	machine().bookkeeping().coin_counter_w(0, data & 0x01);
	machine().bookkeeping().coin_counter_w(1, data & 0x02);

	/* bit 2 = enable char ROM reading through the video RAM */
	m_k052109->set_rmrd_line((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 3 = disable video */
	m_video_enable = !(data & 0x08);

	/* bit 4 = read RAM at 0000 (if set) else read color palette RAM */
	/* bit 5 = PMCBK */
	m_rambank = (data & 0x30) >> 4;
	/* bit 7 = PMC-BK */
	m_pmcbank = (data & 0x80) >> 7;

	/* bit 6 = PMC-START */
	if ((data & 0x40) && !(m_old_3f90 & 0x40))
	{
		pmc_run();
		m_maincpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}

	m_old_3f90 = data;
}


void spy_state::spy_sh_irqtrigger_w(uint8_t data)
{
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff); // Z80
}

void spy_state::sound_bank_w(uint8_t data)
{
	m_k007232_1->set_bank(BIT(data, 0, 2), BIT(data, 2, 2));
	m_k007232_2->set_bank(BIT(data, 4, 2), BIT(data, 6, 2));
}


uint8_t spy_state::k052109_051960_r(offs_t offset)
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

void spy_state::k052109_051960_w(offs_t offset, uint8_t data)
{
	if (offset >= 0x3800 && offset < 0x3808)
		m_k051960->k051937_w(offset - 0x3800, data);
	else if (offset < 0x3c00)
		m_k052109->write(offset, data);
	else
		m_k051960->k051960_w(offset - 0x3c00, data);
}

void spy_state::spy_map(address_map &map)
{
	map(0x0000, 0x07ff).rw(FUNC(spy_state::spy_bankedram1_r), FUNC(spy_state::spy_bankedram1_w)).share("ram");
	map(0x0800, 0x1aff).ram();
	map(0x2000, 0x5fff).rw(FUNC(spy_state::k052109_051960_r), FUNC(spy_state::k052109_051960_w));
	map(0x3f80, 0x3f80).w(FUNC(spy_state::bankswitch_w));
	map(0x3f90, 0x3f90).w(FUNC(spy_state::spy_3f90_w));
	map(0x3fa0, 0x3fa0).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x3fb0, 0x3fb0).w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x3fc0, 0x3fc0).w(FUNC(spy_state::spy_sh_irqtrigger_w));
	map(0x3fd0, 0x3fd0).portr("SYSTEM");
	map(0x3fd1, 0x3fd1).portr("P1");
	map(0x3fd2, 0x3fd2).portr("P2");
	map(0x3fd3, 0x3fd3).portr("DSW1");
	map(0x3fe0, 0x3fe0).portr("DSW2");
	map(0x6000, 0x7fff).bankr(m_rombank);
	map(0x8000, 0xffff).rom();
}

void spy_state::spy_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).w(FUNC(spy_state::sound_bank_w));
	map(0xa000, 0xa00d).rw(m_k007232_1, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xb000, 0xb00d).rw(m_k007232_2, FUNC(k007232_device::read), FUNC(k007232_device::write));
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0xd000, 0xd000).r("soundlatch", FUNC(generic_latch_8_device::read));
}


static INPUT_PORTS_START( spy )
	PORT_START("P1")
	KONAMI8_ALT_B21(1)  /* button 3 unused */

	PORT_START("P2")
	KONAMI8_ALT_B21(2)  /* button 3 unused */

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "10k and every 20k" )
	PORT_DIPSETTING(    0x10, "20k and every 30k" )
	PORT_DIPSETTING(    0x08, "20k only" )
	PORT_DIPSETTING(    0x00, "30k only" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM )        /* PMCFIRQ signal from the PMC */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW3:2" )
	PORT_SERVICE_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, "5 Times" )
	PORT_DIPSETTING(    0x80, "Unlimited" )
INPUT_PORTS_END



void spy_state::volume_callback0(uint8_t data)
{
	m_k007232_1->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232_1->set_volume(1, 0, (data & 0x0f) * 0x11);
}

void spy_state::volume_callback1(uint8_t data)
{
	m_k007232_2->set_volume(0, (data >> 4) * 0x11, 0);
	m_k007232_2->set_volume(1, 0, (data & 0x0f) * 0x11);
}


void spy_state::machine_start()
{
	uint8_t *const ROM = memregion("maincpu")->base();

	m_rombank->configure_entries(0, 12, &ROM[0x10000], 0x2000);

	m_paletteram.resize(0x800);
	m_palette->basemem().set(m_paletteram, ENDIANNESS_BIG, 2);

	memset(m_pmcram, 0, sizeof(m_pmcram));

	save_item(NAME(m_paletteram));
	save_item(NAME(m_rambank));
	save_item(NAME(m_pmcbank));
	save_item(NAME(m_pmcpc));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_old_3f90));
	save_item(NAME(m_pmcram));
}

void spy_state::machine_reset()
{
	m_rambank = 0;
	m_pmcbank = 0;
	m_pmcpc = 0;
	m_video_enable = false;
	m_old_3f90 = -1;
}

void spy_state::spy(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, XTAL(24'000'000) / 8); // 3 MHz? (divided by 051961)
	m_maincpu->set_addrmap(AS_PROGRAM, &spy_state::spy_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &spy_state::spy_sound_map); /* nmi by the sound chip */

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(spy_state::screen_update_spy));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);
	m_palette->enable_shadows();

	K052109(config, m_k052109, 0); // 051961 on schematics
	m_k052109->set_palette(m_palette);
	m_k052109->set_screen("screen");
	m_k052109->set_tile_callback(FUNC(spy_state::tile_callback));
	m_k052109->irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	K051960(config, m_k051960, 0);
	m_k051960->set_palette(m_palette);
	m_k051960->set_screen("screen");
	m_k051960->set_sprite_callback(FUNC(spy_state::sprite_callback));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ym3812_device &ymsnd(YM3812(config, "ymsnd", 3579545));
	ymsnd.irq_handler().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	K007232(config, m_k007232_1, 3579545);
	m_k007232_1->port_write().set(FUNC(spy_state::volume_callback0));
	m_k007232_1->add_route(0, "mono", 0.20);
	m_k007232_1->add_route(1, "mono", 0.20);

	K007232(config, m_k007232_2, 3579545);
	m_k007232_2->port_write().set(FUNC(spy_state::volume_callback1));
	m_k007232_2->add_route(0, "mono", 0.20);
	m_k007232_2->add_route(1, "mono", 0.20);
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( spy )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms + space for banked ram */
	ROM_LOAD( "857n03.bin",   0x10000, 0x10000, CRC(97993b38) SHA1(0afd561bc85fcbfe30f2d16807424ceec7188ce7) )
	ROM_LOAD( "857n02.bin",   0x20000, 0x08000, CRC(31a97efe) SHA1(6c9ec3954e4d16634bf95835b8b404d3a6ef6e24) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "857d01.bin",   0x0000, 0x8000, CRC(aad4210f) SHA1(bb40b8673939b5ce51012606da86b4dcbfc52a57) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "857b09.bin",   0x00000, 0x40000, CRC(b8780966) SHA1(6c255f1e4d1398fa9010a1ae0f5172dc524df109) )
	ROM_LOAD32_WORD( "857b08.bin",   0x00002, 0x40000, CRC(3e4d8d50) SHA1(70f45a725bf1e9d15285ffb6b280945f7ce7faf0) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "857b06.bin",   0x00000, 0x80000, CRC(7b515fb1) SHA1(3830649d47964940023760b76e2bf94bb9163f23) )
	ROM_LOAD32_WORD( "857b05.bin",   0x00002, 0x80000, CRC(27b0f73b) SHA1(6b6a3da11c3005e3a62e6280818c18ae2ea31800) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "857a10.bin",   0x0000, 0x0100, CRC(32758507) SHA1(c21f89ad253502968a755fb0d23da98319f9cd93) )    /* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* samples for 007232 #0 */
	ROM_LOAD( "857b07.bin",   0x00000, 0x40000, CRC(ce3512d4) SHA1(1e7c3feabfc3ac89056982b76de39e283cf5894d) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* samples for 007232 #1 */
	ROM_LOAD( "857b04.bin",   0x00000, 0x40000, CRC(20b83c13) SHA1(63062f1c0a9adbbced3d3d73682a2cd1217bee7d) )
ROM_END

ROM_START( spyu )
	ROM_REGION( 0x28000, "maincpu", 0 ) /* code + banked roms + space for banked ram */
	ROM_LOAD( "857m03.bin",   0x10000, 0x10000, CRC(3bd87fa4) SHA1(257371ef31c8adcdc04f46e989b7a2f3531c2ab1) )
	ROM_LOAD( "857m02.bin",   0x20000, 0x08000, CRC(306cc659) SHA1(91d150b8d320bf19c12bc46103ffdffacf4387c3) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "857d01.bin",   0x0000, 0x8000, CRC(aad4210f) SHA1(bb40b8673939b5ce51012606da86b4dcbfc52a57) )

	ROM_REGION( 0x080000, "k052109", 0 )    /* tiles */
	ROM_LOAD32_WORD( "857b09.bin",   0x00000, 0x40000, CRC(b8780966) SHA1(6c255f1e4d1398fa9010a1ae0f5172dc524df109) )
	ROM_LOAD32_WORD( "857b08.bin",   0x00002, 0x40000, CRC(3e4d8d50) SHA1(70f45a725bf1e9d15285ffb6b280945f7ce7faf0) )

	ROM_REGION( 0x100000, "k051960", 0 )    /* sprites */
	ROM_LOAD32_WORD( "857b06.bin",   0x00000, 0x80000, CRC(7b515fb1) SHA1(3830649d47964940023760b76e2bf94bb9163f23) )
	ROM_LOAD32_WORD( "857b05.bin",   0x00002, 0x80000, CRC(27b0f73b) SHA1(6b6a3da11c3005e3a62e6280818c18ae2ea31800) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "857a10.bin",   0x0000, 0x0100, CRC(32758507) SHA1(c21f89ad253502968a755fb0d23da98319f9cd93) )    /* priority encoder (not used) */

	ROM_REGION( 0x40000, "k007232_1", 0 ) /* samples for 007232 #0 */
	ROM_LOAD( "857b07.bin",   0x00000, 0x40000, CRC(ce3512d4) SHA1(1e7c3feabfc3ac89056982b76de39e283cf5894d) )

	ROM_REGION( 0x40000, "k007232_2", 0 ) /* samples for 007232 #1 */
	ROM_LOAD( "857b04.bin",   0x00000, 0x40000, CRC(20b83c13) SHA1(63062f1c0a9adbbced3d3d73682a2cd1217bee7d) )
ROM_END

} // anonymous namespace


GAME( 1989, spy,  0,   spy, spy, spy_state, empty_init, ROT0, "Konami", "S.P.Y. - Special Project Y (World ver. N)", MACHINE_SUPPORTS_SAVE )
GAME( 1989, spyu, spy, spy, spy, spy_state, empty_init, ROT0, "Konami", "S.P.Y. - Special Project Y (US ver. M)", MACHINE_SUPPORTS_SAVE )
