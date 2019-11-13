// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Final Crash & other CPS1 bootlegs */


/*
	A note reguarding other bootlegs:
	In order to keep the cps source in some sort of order, the idea is to group similar bootleg hardware into seperate
	derived classes and source files.

	Rom swaps, hacks etc.  (on original Capcom hardware)  ->  cps1.cpp
	Sound: Z80, 2x YM2203, 2x m5205 ("Final Crash" h/w)   ->  fcrash.cpp
	Sound: Z80, 1x YM2151, 2x m5205                       ->  cps1bl_5205.cpp
	Sound: PIC, 1x M6295            *1                    ->  cps1bl_pic.cpp
	Sound: Z80, 1x YM2151, 1x M6295 *2                    ->  fcrash.cpp      (for now...)

	*1 these seem to be only CPS1.5/Q sound games?
	*2 this is original configuration, but non-Capcom (usually single-board) hardware.
	
	
	As per the above, this file now only contains games in second and last catergories.
	Eventually only Final Crash, other Final Fight bootlegs and Carrier Air Wing bootlegs will remain here.
*/


/*

Final Crash is a bootleg of Final Fight

Final Fight is by Capcom and runs on CPS1 hardware
The bootleg was manufactured by Playmark of Italy

this driver depends heavily on cps1.cpp, but has been
kept apart in an attempt to keep cps1.cpp clutter free

Sound is very different from CPS1.

---

Final Crash (bootleg of final fight)

1x 68k
1x z80
2x ym2203
2x oki5205
1x osc 10mhz
1x osc 24mhz

EPROMs:
1.bin sound EPROM
from 2.bin to 9.bin program EPROMs
10.bin to 25.bin graphics EPROMs

---

kodb has various graphical issues, mainly with old info not being cleared away.
Also, it should be using a vblank irq value of 4. This triggers the following bootleg read/writes;
 - IN1 is read at 0x992000
 - IN0 is read of 0x992008
 - dips continue to be read at 0x80001a
 - sound command is wrote at 0x992006
 - scroll 1Y is wrote at 0x980000
 - scroll 1X is wrote at 0x980002
 - scroll 2Y is wrote at 0x980004
 - scroll 2X is wrote at 0x980006
 - scroll 3Y is wrote at 0x980008
 - scroll 3X is wrote at 0x98000a
 - the layer enable and layer mask writes continue at 0x98000c and 0x980020-2

These read/writes are identical to those used by a Knights of the Round bootleg which uses the all sf2mdt sound
hardware. This set is currently non-working.

This also prevents the game from toggling the sprite address at m_cps_a_regs[0], similar to other bootlegs.
Currently the game is working somewhat, but only using the code left over from the original. If anyone wants to
do any development work on the set, (eg, find the sprite clearing issue), then this should be changed as the game
likely won't write any sprite clearing values otherwise.

None of this is hooked up currently due to issues with row scroll on the scroll2 layer.



Status of each game:
--------------------
cawingb2, cawingbl: ok

dinopic: no sound   (moved to cps1bl_pic.cpp)

dinopic2: no sound, one bad graphics ROM. Copying 8.bin from dinopic fixes it.   (moved to cps1bl_pic.cpp)

fcrash, kodb: old sprites show on next screen. Patch used.

punipic, punipic2: no sound. Problems in Central Park. Patches used.   (moved to cps1bl_pic.cpp)

punipic3: same as punipic, and doors are missing.   (moved to cps1bl_pic.cpp)

sf2m1: crowd is missing. Plane's tail comes off a bit. Patch used.

sf2mdt, sf2mdtb: ok   (moved to cps1bl_5205.cpp)

sf2mdta, sf2ceb: scroll 2X has strange 0x200 writes that cause missing fighters' portraits at the vs. screen    (moved to cps1bl_5205.cpp)
                 and glitched backgrounds during fights. Masking them out seems a hack.

sgyxz, wofabl: garbage left behind. A priority problem can be seen in 3rd demo where
               the fighters walk through the crowd instead of behind.

slampic: no sound. Some minor gfx issues (sprites on character select screen)   (moved to cps1bl_pic.cpp)

slampic2: no sound. All gfx issues confirmed present on real board.   (moved to cps1bl_pic.cpp)

captcommb2: ok    (moved to cps1bl_5205.cpp)

knightsb3: ok    (moved to cps1bl_5205.cpp)

dinopic3: no sound. Some minor gfx priority issues, confirmed present on real board.   (moved to cps1bl_pic.cpp)

*/

#include "emu.h"
#include "includes/cps1.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "sound/ym2151.h"
#include "sound/okim6295.h"
#include "machine/eepromser.h"
#include "speaker.h"


#define CPS1_ROWSCROLL_OFFS  (0x20/2)    /* base of row scroll offsets in other RAM */
#define CODE_SIZE            0x400000


WRITE16_MEMBER( fcrash_state::fcrash_soundlatch_w )
{
	if (ACCESSING_BITS_0_7)
	{
		m_soundlatch->write(data & 0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

WRITE8_MEMBER( fcrash_state::fcrash_snd_bankswitch_w )
{
	m_msm_1->set_output_gain(0, (data & 0x08) ? 0.0 : 1.0);
	m_msm_2->set_output_gain(0, (data & 0x10) ? 0.0 : 1.0);

	membank("bank1")->set_entry(data & 0x07);
}

WRITE_LINE_MEMBER(fcrash_state::m5205_int1)
{
	m_msm_1->write_data(m_sample_buffer1 & 0x0f);
	m_sample_buffer1 >>= 4;
	m_sample_select1 ^= 1;
	if (m_sample_select1 == 0)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE_LINE_MEMBER(fcrash_state::m5205_int2)
{
	m_msm_2->write_data(m_sample_buffer2 & 0x0f);
	m_sample_buffer2 >>= 4;
	m_sample_select2 ^= 1;
}

WRITE8_MEMBER( fcrash_state::fcrash_msm5205_0_data_w )
{
	m_sample_buffer1 = data;
}

WRITE8_MEMBER( fcrash_state::fcrash_msm5205_1_data_w )
{
	m_sample_buffer2 = data;
}

WRITE16_MEMBER(fcrash_state::cawingbl_soundlatch_w)
{
	if (ACCESSING_BITS_8_15)
	{
		m_soundlatch->write(data >> 8);
		m_audiocpu->set_input_line(0, HOLD_LINE);
		machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50)); /* boost the interleave or some voices get dropped */
	}
}

WRITE16_MEMBER(fcrash_state::kodb_layer_w)
{
	/* layer enable and mask 1&2 registers are written here - passing them to m_cps_b_regs for now for drawing routines */
	if (offset == 0x06)
		m_cps_b_regs[m_layer_enable_reg / 2] = data;
	else
	if (offset == 0x10)
		m_cps_b_regs[m_layer_mask_reg[1] / 2] = data;
	else
	if (offset == 0x11)
		m_cps_b_regs[m_layer_mask_reg[2] / 2] = data;
}

WRITE16_MEMBER(fcrash_state::mtwinsb_layer_w)
{
	m_cps_a_regs[0x06 / 2] = 0x9100; // bit of a hack - the game never writes this, but does need it

	switch (offset)
	{
	case 0x00:
		m_cps_a_regs[0x0e / 2] = data;
		break;
	case 0x01:
		m_cps_a_regs[0x0c / 2] = data - 0x3e;
		break;
	case 0x02:
		m_cps_a_regs[0x12 / 2] = data;
		m_cps_a_regs[CPS1_ROWSCROLL_OFFS] = data; /* row scroll start */
		break;
	case 0x03:
		m_cps_a_regs[0x10 / 2] = data - 0x3c;
		break;
	case 0x04:
		m_cps_a_regs[0x16 / 2] = data;
		break;
	case 0x05:
		m_cps_a_regs[0x14 / 2] = data - 0x40;
		break;
	default:
		logerror("%s: Unknown layer cmd %X %X\n",machine().describe_context(),offset<<1,data);

	}
}

WRITE16_MEMBER(fcrash_state::sf2m1_layer_w)
{
	switch (offset)
	{
	case 0x00:
		m_cps_a_regs[0x0e / 2] = data;
		break;
	case 0x01:
		m_cps_a_regs[0x0c / 2] = data;
		break;
	case 0x02:
		m_cps_a_regs[0x12 / 2] = data;
		m_cps_a_regs[CPS1_ROWSCROLL_OFFS] = data; /* row scroll start */
		break;
	case 0x03:
		m_cps_a_regs[0x10 / 2] = data;
		break;
	case 0x04:
		m_cps_a_regs[0x16 / 2] = data;
		break;
	case 0x05:
		m_cps_a_regs[0x14 / 2] = data;
		break;
	case 0x06:
			switch (data)
			{
			case 0:
				data = 0x078e;
				break;
			case 1:
				data = 0x12c0;
				break;
			case 2:
				data = 0x06ce;
				break;
			case 3:
				data = 0x09ce;
				break;
			case 4:
				data = 0x12ce;
				break;
			case 5:
				data = 0x0b4e;
				break;
			}
	case 0xb3:
			m_cps_b_regs[m_layer_enable_reg / 2] = data;
			break;
	case 0x0b:
	case 0x1b:
		m_cps_a_regs[0x06 / 2] = data;
		break;
	default:
		logerror("%s: Unknown layer cmd %X %X\n",machine().describe_context(),offset<<1,data);

	}
}

WRITE16_MEMBER(fcrash_state::varthb_layer_w)
{
	switch (offset)
	{
	case 0x00:
		m_cps_a_regs[0x0e / 2] = data;
		break;
	case 0x01:
		m_cps_a_regs[0x0c / 2] = data;
		break;
	case 0x02:
		m_cps_a_regs[0x12 / 2] = data;
		m_cps_a_regs[CPS1_ROWSCROLL_OFFS] = data; /* row scroll start */
		break;
	case 0x03:
		m_cps_a_regs[0x10 / 2] = data;
		break;
	case 0x04:
		m_cps_a_regs[0x16 / 2] = data;
		break;
	case 0x05:
		m_cps_a_regs[0x14 / 2] = data;
		break;
	default:
		logerror("%s: Unknown layer cmd %X %X\n",machine().describe_context(),offset<<1,data);
	}
}

WRITE16_MEMBER(fcrash_state::varthb_layer2_w)
{
	if (data > 0x9000)
		m_cps_a_regs[0x06 / 2] = data;
}


void fcrash_state::fcrash(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &fcrash_state::fcrash_map);
	m_maincpu->set_vblank_int("screen", FUNC(fcrash_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &fcrash_state::cpu_space_map);

	Z80(config, m_audiocpu, 24000000/6); /* ? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &fcrash_state::fcrash_sound_map);

	MCFG_MACHINE_START_OVERRIDE(fcrash_state, fcrash)
	MCFG_MACHINE_RESET_OVERRIDE(fcrash_state, fcrash)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1 );
	m_screen->set_screen_update(FUNC(fcrash_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(fcrash_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(4096);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ym1(YM2203(config, "ym1", 24000000/6));   /* ? */
	ym1.add_route(0, "mono", 0.10);
	ym1.add_route(1, "mono", 0.10);
	ym1.add_route(2, "mono", 0.10);
	ym1.add_route(3, "mono", 1.0);

	ym2203_device &ym2(YM2203(config, "ym2", 24000000/6));   /* ? */
	ym2.add_route(0, "mono", 0.10);
	ym2.add_route(1, "mono", 0.10);
	ym2.add_route(2, "mono", 0.10);
	ym2.add_route(3, "mono", 1.0);

	MSM5205(config, m_msm_1, 24000000/64);  /* ? */
	m_msm_1->vck_legacy_callback().set(FUNC(fcrash_state::m5205_int1)); /* interrupt function */
	m_msm_1->set_prescaler_selector(msm5205_device::S96_4B);    /* 4KHz 4-bit */
	m_msm_1->add_route(ALL_OUTPUTS, "mono", 0.25);

	MSM5205(config, m_msm_2, 24000000/64);  /* ? */
	m_msm_2->vck_legacy_callback().set(FUNC(fcrash_state::m5205_int2)); /* interrupt function */
	m_msm_2->set_prescaler_selector(msm5205_device::S96_4B);    /* 4KHz 4-bit */
	m_msm_2->add_route(ALL_OUTPUTS, "mono", 0.25);
}

void fcrash_state::cawingbl(machine_config &config)
{
	fcrash(config);
	/* basic machine hardware */
	m_maincpu->set_vblank_int("screen", FUNC(fcrash_state::irq6_line_hold)); /* needed to write to scroll values */

	MCFG_MACHINE_START_OVERRIDE(fcrash_state, cawingbl)
}

void fcrash_state::kodb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &fcrash_state::fcrash_map);
	m_maincpu->set_vblank_int("screen", FUNC(fcrash_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &fcrash_state::cpu_space_map);

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fcrash_state::kodb_sound_map);

	MCFG_MACHINE_START_OVERRIDE(fcrash_state, kodb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(fcrash_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(fcrash_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2151_device &ym2151(YM2151(config, "2151", XTAL(3'579'545)));  /* verified on pcb */
	ym2151.irq_handler().set_inputline(m_audiocpu, 0);
	ym2151.add_route(0, "mono", 0.35);
	ym2151.add_route(1, "mono", 0.35);

	/* CPS PPU is fed by a 16mhz clock,pin 117 outputs a 4mhz clock which is divided by 4 using 2 74ls74 */
	OKIM6295(config, m_oki, XTAL(16'000'000)/4/4, okim6295_device::PIN7_HIGH); // pin 7 can be changed by the game code, see f006 on z80
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.30);
}

void fcrash_state::mtwinsb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &fcrash_state::mtwinsb_map);
	m_maincpu->set_vblank_int("screen", FUNC(fcrash_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &fcrash_state::cpu_space_map);

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fcrash_state::sgyxz_sound_map);

	MCFG_MACHINE_START_OVERRIDE(fcrash_state, mtwinsb)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(fcrash_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(fcrash_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym2151_device &ym2151(YM2151(config, "2151", XTAL(3'579'545)));
	ym2151.irq_handler().set_inputline(m_audiocpu, 0);
	ym2151.add_route(0, "mono", 0.35);
	ym2151.add_route(1, "mono", 0.35);

	OKIM6295(config, m_oki, XTAL(16'000'000)/4/4, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.30);
}

void fcrash_state::sf2m1(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(12'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &fcrash_state::sf2m1_map);
	m_maincpu->set_vblank_int("screen", FUNC(fcrash_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &fcrash_state::cpu_space_map);

	Z80(config, m_audiocpu, XTAL(3'579'545));
	m_audiocpu->set_addrmap(AS_PROGRAM, &fcrash_state::sgyxz_sound_map);

	MCFG_MACHINE_START_OVERRIDE(fcrash_state, sf2m1)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(CPS_PIXEL_CLOCK, CPS_HTOTAL, CPS_HBEND, CPS_HBSTART, CPS_VTOTAL, CPS_VBEND, CPS_VBSTART);
	m_screen->set_screen_update(FUNC(fcrash_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(fcrash_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);
	ym2151_device &ym2151(YM2151(config, "2151", XTAL(3'579'545)));
	ym2151.irq_handler().set_inputline(m_audiocpu, 0);
	ym2151.add_route(0, "mono", 0.35);
	ym2151.add_route(1, "mono", 0.35);
	OKIM6295(config, m_oki, XTAL(16'000'000)/4/4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30);
}

void fcrash_state::sgyxz(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &fcrash_state::sgyxz_map);
	m_maincpu->set_vblank_int("screen", FUNC(fcrash_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &fcrash_state::cpu_space_map);

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fcrash_state::sgyxz_sound_map);

	MCFG_MACHINE_START_OVERRIDE(fcrash_state, sgyxz)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1 );
	m_screen->set_screen_update(FUNC(fcrash_state::screen_update_fcrash));
	m_screen->screen_vblank().set(FUNC(fcrash_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	EEPROM_93C46_8BIT(config, "eeprom");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym2151_device &ym2151(YM2151(config, "2151", XTAL(3'579'545)));  /* verified on pcb */
	ym2151.irq_handler().set_inputline(m_audiocpu, 0);
	ym2151.add_route(0, "mono", 0.35);
	ym2151.add_route(1, "mono", 0.35);

	/* CPS PPU is fed by a 16mhz clock,pin 117 outputs a 4mhz clock which is divided by 4 using 2 74ls74 */
	OKIM6295(config, m_oki, XTAL(16'000'000)/4/4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30); // pin 7 can be changed by the game code, see f006 on z80
}

void fcrash_state::wofabl(machine_config &config)
{
	sgyxz(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &fcrash_state::wofabl_map);
}

void fcrash_state::varthb(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &fcrash_state::varthb_map);
	m_maincpu->set_vblank_int("screen", FUNC(fcrash_state::cps1_interrupt));
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &fcrash_state::cpu_space_map);

	Z80(config, m_audiocpu, 3579545);
	m_audiocpu->set_addrmap(AS_PROGRAM, &fcrash_state::sgyxz_sound_map);

	MCFG_MACHINE_START_OVERRIDE(fcrash_state, cps1)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 32*8);
	m_screen->set_visarea(8*8, (64-8)*8-1, 2*8, 30*8-1 );
	m_screen->set_screen_update(FUNC(fcrash_state::screen_update_cps1));
	m_screen->screen_vblank().set(FUNC(fcrash_state::screen_vblank_cps1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cps1);
	PALETTE(config, m_palette, palette_device::BLACK).set_entries(0xc00);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	ym2151_device &ym2151(YM2151(config, "2151", XTAL(3'579'545)));
	ym2151.irq_handler().set_inputline(m_audiocpu, 0);
	ym2151.add_route(0, "mono", 0.35);
	ym2151.add_route(1, "mono", 0.35);

	OKIM6295(config, m_oki, XTAL(16'000'000)/4/4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.30);
}


void fcrash_state::fcrash_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800030, 0x800031).w(FUNC(fcrash_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).ram().share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).ram().share("cps_b_regs");  /* CPS-B custom */
	map(0x880000, 0x880001).portr("IN1");                /* Player input ports */
	map(0x880006, 0x880007).w(FUNC(fcrash_state::fcrash_soundlatch_w));       /* Sound command */
	map(0x880008, 0x88000f).r(FUNC(fcrash_state::cps1_dsw_r));                /* System input ports / Dip Switches */
	map(0x890000, 0x890001).nopw();    // palette related?
	map(0x900000, 0x92ffff).ram().w(FUNC(fcrash_state::cps1_gfxram_w)).share("gfxram");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void fcrash_state::mtwinsb_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800000, 0x800001).portr("IN1");
	map(0x800006, 0x800007).w(FUNC(fcrash_state::cps1_soundlatch_w));
	map(0x800018, 0x80001f).r(FUNC(fcrash_state::cps1_dsw_r));
	map(0x800030, 0x800037).w(FUNC(fcrash_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).w(FUNC(fcrash_state::cps1_cps_a_w)).share("cps_a_regs");
	map(0x800140, 0x80017f).rw(FUNC(fcrash_state::cps1_cps_b_r), FUNC(fcrash_state::cps1_cps_b_w)).share("cps_b_regs");
	map(0x980000, 0x98000b).w(FUNC(fcrash_state::mtwinsb_layer_w));
	map(0x900000, 0x92ffff).ram().w(FUNC(fcrash_state::cps1_gfxram_w)).share("gfxram");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void fcrash_state::sf2m1_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800000, 0x800007).portr("IN1");            /* Player input ports */
	map(0x800006, 0x800007).w(FUNC(fcrash_state::cps1_soundlatch_w));    /* Sound command */
	map(0x800012, 0x800013).r(FUNC(fcrash_state::cps1_in2_r));            /* Buttons 4,5,6 for both players */
	map(0x800018, 0x80001f).r(FUNC(fcrash_state::cps1_dsw_r));            /* System input ports / Dip Switches */
	map(0x800100, 0x80013f).w(FUNC(fcrash_state::cps1_cps_a_w)).share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).rw(FUNC(fcrash_state::cps1_cps_b_r), FUNC(fcrash_state::cps1_cps_b_w)).share("cps_b_regs");
	map(0x800180, 0x800181).nopw(); // only once at boot, for 80010c
	map(0x800188, 0x80018f).w(FUNC(fcrash_state::cps1_soundlatch2_w));   /* Sound timer fade */
	map(0x880000, 0x880001).nopw(); // unknown
	map(0x900000, 0x93ffff).ram().w(FUNC(fcrash_state::cps1_gfxram_w)).share("gfxram");
	map(0x980000, 0x9801ff).w(FUNC(fcrash_state::sf2m1_layer_w));
	map(0x990000, 0x990001).nopw(); // same as 880000
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void fcrash_state::sgyxz_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x800030, 0x800031).w(FUNC(fcrash_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).ram().share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).ram().share("cps_b_regs");  /* CPS-B custom */
	map(0x880000, 0x880001).portr("IN1");            /* Player input ports */
	map(0x880006, 0x88000d).r(FUNC(fcrash_state::cps1_dsw_r));            /* System input ports / Dip Switches */
	map(0x88000e, 0x88000f).w(FUNC(fcrash_state::cps1_soundlatch_w));
	map(0x880e78, 0x880e79).r(FUNC(fcrash_state::cps1_in2_r));            /* Player 3 controls (later games) */
	map(0x890000, 0x890001).w(FUNC(fcrash_state::cps1_soundlatch2_w));
	map(0x900000, 0x92ffff).ram().w(FUNC(fcrash_state::cps1_gfxram_w)).share("gfxram");
	map(0xf1c004, 0xf1c005).w(FUNC(fcrash_state::cpsq_coinctrl2_w));     /* Coin control2 (later games) */
	map(0xf1c006, 0xf1c007).portr("EEPROMIN").portw("EEPROMOUT");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void fcrash_state::wofabl_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x800030, 0x800031).w(FUNC(fcrash_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).ram().share("cps_a_regs");  /* CPS-A custom */
	map(0x800140, 0x80017f).ram().share("cps_b_regs");  /* CPS-B custom */
	map(0x880000, 0x880001).portr("IN1");            /* Player input ports */
	map(0x880006, 0x880007).w(FUNC(fcrash_state::cps1_soundlatch_w));
	map(0x880008, 0x88000f).r(FUNC(fcrash_state::cps1_dsw_r));            /* System input ports / Dip Switches */
	map(0x880e78, 0x880e79).r(FUNC(fcrash_state::cps1_in2_r));            /* Player 3 controls (later games) */
	map(0x890000, 0x890001).w(FUNC(fcrash_state::cps1_soundlatch2_w));
	map(0x900000, 0x92ffff).ram().w(FUNC(fcrash_state::cps1_gfxram_w)).share("gfxram");
	map(0xf1c004, 0xf1c005).w(FUNC(fcrash_state::cpsq_coinctrl2_w));     /* Coin control2 (later games) */
	map(0xf1c006, 0xf1c007).portr("EEPROMIN").portw("EEPROMOUT");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void fcrash_state::varthb_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x800000, 0x800001).portr("IN1");
	map(0x800006, 0x800007).w(FUNC(fcrash_state::cps1_soundlatch_w));
	map(0x800018, 0x80001f).r(FUNC(fcrash_state::cps1_dsw_r));
	map(0x800030, 0x800037).w(FUNC(fcrash_state::cps1_coinctrl_w));
	map(0x800100, 0x80013f).w(FUNC(fcrash_state::cps1_cps_a_w)).share("cps_a_regs");
	map(0x800140, 0x80017f).rw(FUNC(fcrash_state::cps1_cps_b_r), FUNC(fcrash_state::cps1_cps_b_w)).share("cps_b_regs");
	map(0x800188, 0x800189).w(FUNC(fcrash_state::varthb_layer2_w));
	map(0x980000, 0x98000b).w(FUNC(fcrash_state::varthb_layer_w));
	map(0x900000, 0x92ffff).ram().w(FUNC(fcrash_state::cps1_gfxram_w)).share("gfxram");
	map(0xff0000, 0xffffff).ram().share("mainram");
}

void fcrash_state::fcrash_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xd000, 0xd7ff).ram();
	map(0xd800, 0xd801).rw("ym1", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xdc00, 0xdc01).rw("ym2", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xe000, 0xe000).w(FUNC(fcrash_state::fcrash_snd_bankswitch_w));
	map(0xe400, 0xe400).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0xe800, 0xe800).w(FUNC(fcrash_state::fcrash_msm5205_0_data_w));
	map(0xec00, 0xec00).w(FUNC(fcrash_state::fcrash_msm5205_1_data_w));
}

void fcrash_state::kodb_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xd000, 0xd7ff).ram();
	map(0xe000, 0xe001).rw("2151", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe400, 0xe400).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe800, 0xe800).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

void fcrash_state::sgyxz_sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xd000, 0xd7ff).ram();
	map(0xf000, 0xf001).rw("2151", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xf002, 0xf002).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf004, 0xf004).w(FUNC(fcrash_state::cps1_snd_bankswitch_w));
	map(0xf006, 0xf006).w(FUNC(fcrash_state::cps1_oki_pin7_w)); /* controls pin 7 of OKI chip */
	map(0xf008, 0xf008).r(m_soundlatch, FUNC(generic_latch_8_device::read)); /* Sound command */
	map(0xf00a, 0xf00a).r(m_soundlatch2, FUNC(generic_latch_8_device::read)); /* Sound timer fade */
}


MACHINE_START_MEMBER(fcrash_state,fcrash)
{
	uint8_t *ROM = memregion("audiocpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x4000);

	m_layer_enable_reg = 0x20;
	m_layer_mask_reg[0] = 0x26;
	m_layer_mask_reg[1] = 0x30;
	m_layer_mask_reg[2] = 0x28;
	m_layer_mask_reg[3] = 0x32;
	m_layer_scroll1x_offset = 62;
	m_layer_scroll2x_offset = 60;
	m_layer_scroll3x_offset = 64;
	m_sprite_base = 0x50c8;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 0;

	save_item(NAME(m_sample_buffer1));
	save_item(NAME(m_sample_buffer2));
	save_item(NAME(m_sample_select1));
	save_item(NAME(m_sample_select2));
}

MACHINE_RESET_MEMBER(fcrash_state,fcrash)
{
	m_sample_buffer1 = 0;
	m_sample_buffer2 = 0;
	m_sample_select1 = 0;
	m_sample_select2 = 0;
}

MACHINE_START_MEMBER(fcrash_state, cawingbl)
{
	MACHINE_START_CALL_MEMBER(fcrash);

	m_layer_enable_reg = 0x0c;
	m_layer_mask_reg[0] = 0x0a;
	m_layer_mask_reg[1] = 0x08;
	m_layer_mask_reg[2] = 0x06;
	m_layer_mask_reg[3] = 0x04;
	m_layer_scroll1x_offset = 63;
	m_layer_scroll2x_offset = 62;
	m_layer_scroll3x_offset = 65;
	m_sprite_base = 0x1000;
}

MACHINE_START_MEMBER(fcrash_state,kodb)
{
	m_layer_enable_reg = 0x20;
	m_layer_mask_reg[0] = 0x2e;
	m_layer_mask_reg[1] = 0x2c;
	m_layer_mask_reg[2] = 0x2a;
	m_layer_mask_reg[3] = 0x28;
	m_layer_scroll1x_offset = 0;
	m_layer_scroll2x_offset = 0;
	m_layer_scroll3x_offset = 0;
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0xffff;
	m_sprite_x_offset = 0;
}

MACHINE_START_MEMBER(fcrash_state, mtwinsb)
{
	m_layer_enable_reg = 0x12;
	m_layer_mask_reg[0] = 0x14;
	m_layer_mask_reg[1] = 0x16;
	m_layer_mask_reg[2] = 0x18;
	m_layer_mask_reg[3] = 0x1a;
	m_layer_scroll1x_offset = 0x00;
	m_layer_scroll2x_offset = 0x00;
	m_layer_scroll3x_offset = 0x00;
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 0;
}

MACHINE_START_MEMBER(fcrash_state, sf2m1)
{
	uint8_t *ROM = memregion("audiocpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x4000);

	m_layer_enable_reg = 0x26;
	m_layer_mask_reg[0] = 0x28;
	m_layer_mask_reg[1] = 0x2a;
	m_layer_mask_reg[2] = 0x2c;
	m_layer_mask_reg[3] = 0x2e;
	m_layer_scroll1x_offset = 0x3e;
	m_layer_scroll2x_offset = 0x3c;
	m_layer_scroll3x_offset = 0x40;
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 0;
}

MACHINE_START_MEMBER(fcrash_state,sgyxz)
{
	MACHINE_START_CALL_MEMBER(kodb);
	m_layer_scroll1x_offset = 0x40;
	m_layer_scroll2x_offset = 0x40;
	m_layer_scroll3x_offset = 0x40;
	membank("bank1")->configure_entries(0, 2, memregion("audiocpu")->base() + 0x10000, 0x4000);
}


void fcrash_state::init_cawingbl()
{
	m_maincpu->space(AS_PROGRAM).install_read_port(0x882000, 0x882001, "IN1");
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x882006, 0x882007, write16_delegate(*this, FUNC(fcrash_state::cawingbl_soundlatch_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x882008, 0x88200f, read16_delegate(*this, FUNC(fcrash_state::cps1_dsw_r)));

	init_cps1();
}

void fcrash_state::init_kodb()
{
	m_maincpu->space(AS_PROGRAM).install_read_port(0x800000, 0x800007, "IN1");
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x800018, 0x80001f, read16_delegate(*this, FUNC(fcrash_state::cps1_dsw_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x800180, 0x800187, write16_delegate(*this, FUNC(fcrash_state::cps1_soundlatch_w)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x980000, 0x98002f, write16_delegate(*this, FUNC(fcrash_state::kodb_layer_w)));

	/* the original game alternates between 2 sprite ram areas to achieve flashing sprites - the bootleg doesn't do the write to the register to achieve this
	mapping both sprite ram areas to the same bootleg sprite ram - similar to how sf2mdt works */
	m_bootleg_sprite_ram = std::make_unique<uint16_t[]>(0x2000);
	m_maincpu->space(AS_PROGRAM).install_ram(0x900000, 0x903fff, m_bootleg_sprite_ram.get());
	m_maincpu->space(AS_PROGRAM).install_ram(0x904000, 0x907fff, m_bootleg_sprite_ram.get()); /* both of these need to be mapped */

	init_cps1();
}

void fcrash_state::init_mtwinsb()
{
	m_bootleg_sprite_ram = std::make_unique<uint16_t[]>(0x2000);
	m_maincpu->space(AS_PROGRAM).install_ram(0x990000, 0x993fff, m_bootleg_sprite_ram.get());
	init_cps1();
}

void fcrash_state::init_sf2m1()
{
	uint16_t *mem16 = (uint16_t *)memregion("maincpu")->base();
	mem16[0x64E/2] = 0x6046; // fix priorities

	init_mtwinsb();
}

void fcrash_state::init_wofabl()
{
	uint32_t gfx_size = memregion( "gfx" )->bytes();
	uint8_t *rom = memregion( "gfx" )->base();
	for (int i = 0; i < gfx_size; i += 8)
	{
		uint8_t tmp = rom[i + 1];
		rom[i + 1] = rom[i + 4];
		rom[i + 4] = tmp;
		tmp = rom[i + 3];
		rom[i + 3] = rom[i + 6];
		rom[i + 6] = tmp;
	}

	init_cps1();
}


#define CPS1_COINAGE_1 \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) ) \
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_6C ) )

#define CPS1_COINAGE_2(diploc) \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_6C ) )

#define CPS1_DIFFICULTY_1(diploc) \
	PORT_DIPNAME( 0x07, 0x04, DEF_STR( Difficulty ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x07, "1 (Easiest)" ) \
	PORT_DIPSETTING(    0x06, "2" ) \
	PORT_DIPSETTING(    0x05, "3" ) \
	PORT_DIPSETTING(    0x04, "4 (Normal)" ) \
	PORT_DIPSETTING(    0x03, "5" ) \
	PORT_DIPSETTING(    0x02, "6" ) \
	PORT_DIPSETTING(    0x01, "7" ) \
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )

static INPUT_PORTS_START( fcrash )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	CPS1_COINAGE_1
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Difficulty Level 1" )
	PORT_DIPSETTING(    0x07, DEF_STR( Easiest ) )      // "01"
	PORT_DIPSETTING(    0x06, DEF_STR( Easier ) )       // "02"
	PORT_DIPSETTING(    0x05, DEF_STR( Easy ) )     // "03"
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )       // "04"
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )       // "05"
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )     // "06"
	PORT_DIPSETTING(    0x01, DEF_STR( Harder ) )       // "07"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      // "08"
	PORT_DIPNAME( 0x18, 0x10, "Difficulty Level 2" )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )     // "01"
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )       // "02"
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )     // "03"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )      // "04"
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x60, "100k" )
	PORT_DIPSETTING(    0x40, "200k" )
	PORT_DIPSETTING(    0x20, "100k and every 200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME ("P1 Button 3 (Cheat)")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME ("P2 Button 3 (Cheat)")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( cawingbl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	CPS1_COINAGE_1
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )                          // Overrides all other coinage settings
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )                           // according to manual
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                       // This switch is not documented

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Difficulty Level (Enemy's Strength)" )   PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easiest)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x18, 0x18, "Difficulty Level (Player's Strength)" )  PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )                      // This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )                      // This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )                      // This switch is not documented

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )                      // This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )                      // This switch is not documented
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                    PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                                PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )                  PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )                  PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )               PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                              PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kodb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	CPS1_COINAGE_2( "SW(A)" )
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )                        PORT_DIPLOCATION("SW(A):4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Play Mode" )                         PORT_DIPLOCATION("SW(A):5")
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x10, "3 Players" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Lives ) )                    PORT_DIPLOCATION("SW(B):4,5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x38, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )               PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING(    0x80, "80k and every 400k" )
	PORT_DIPSETTING(    0xc0, "100k and every 450k" )
	PORT_DIPSETTING(    0x40, "160k and every 450k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )                PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )                            PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )              PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )              PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )           PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")                          PORT_DIPLOCATION("SW(C):8")
	PORT_DIPSETTING(    0x80, "Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Test ) )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

static INPUT_PORTS_START( sgyxz )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START ("DSWA")
	PORT_DIPNAME( 0x03, 0x00, "Play Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x03, "Tournament" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START ("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START ("DSWC")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START ("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START ("IN2")      /* Player 3 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START( "EEPROMIN" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

static INPUT_PORTS_START( wofabl )
	PORT_INCLUDE(sgyxz)

	PORT_MODIFY("DSWA")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW(A):1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW(A):2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW(A):3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(A):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(A):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW(A):7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_MODIFY("DSWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW(B):1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW(B):2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW(B):3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW(B):8" )
INPUT_PORTS_END


void fcrash_state::fcrash_update_transmasks()
{
	int i;

	for (i = 0; i < 4; i++)
	{
		int mask;

		/* Get transparency registers */
		if (m_layer_mask_reg[i])
			mask = m_cps_b_regs[m_layer_mask_reg[i] / 2] ^ 0xffff;
		else
			mask = 0xffff;  /* completely transparent if priority masks not defined (mercs, qad) */

		m_bg_tilemap[0]->set_transmask(i, mask, 0x8000);
		m_bg_tilemap[1]->set_transmask(i, mask, 0x8000);
		m_bg_tilemap[2]->set_transmask(i, mask, 0x8000);
	}
}

void fcrash_state::fcrash_render_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	int pos;
	int base = m_sprite_base / 2;
	int num_sprites = m_gfxdecode->gfx(2)->elements();
	int last_sprite_offset = 0x1ffc;
	uint16_t *sprite_ram = m_gfxram;
	uint16_t tileno,colour,xpos,ypos;
	bool flipx, flipy;

	/* if we have separate sprite ram, use it */
	if (m_bootleg_sprite_ram) sprite_ram = m_bootleg_sprite_ram.get();

	/* get end of sprite list marker */
	for (pos = 0x1ffc - base; pos >= 0x0000; pos -= 4)
		if (sprite_ram[base + pos - 1] == m_sprite_list_end_marker) last_sprite_offset = pos;

	/* If we are using bootleg sprite ram, the index must be less than 0x2000 */
	if (((base + last_sprite_offset) < 0x2000) || (!m_bootleg_sprite_ram))
	{
		for (pos = last_sprite_offset; pos >= 0x0000; pos -= 4)
		{
			tileno = sprite_ram[base + pos];
			if (tileno >= num_sprites) continue; /* don't render anything outside our tiles */
			xpos   = sprite_ram[base + pos + 2] & 0x1ff;
			ypos   = sprite_ram[base + pos - 1] & 0x1ff;
			flipx  = BIT(sprite_ram[base + pos + 1], 5);
			flipy  = BIT(sprite_ram[base + pos + 1], 6);
			colour = sprite_ram[base + pos + 1] & 0x1f;
			ypos   = 256 - ypos - 16;
			xpos   = xpos + m_sprite_x_offset + 49;

			if (flip_screen())
				m_gfxdecode->gfx(2)->prio_transpen(bitmap, cliprect, tileno, colour, !flipx, !flipy, 512-16-xpos, 256-16-ypos, screen.priority(), 2, 15);
			else
				m_gfxdecode->gfx(2)->prio_transpen(bitmap, cliprect, tileno, colour, flipx, flipy, xpos, ypos, screen.priority(), 2, 15);
		}
	}
}

void fcrash_state::fcrash_render_layer( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask )
{
	switch (layer)
	{
		case 0:
			(this->*m_bootleg_sprite_renderer)(screen, bitmap, cliprect);
			break;
		case 1:
		case 2:
		case 3:
			m_bg_tilemap[layer - 1]->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, primask);
			break;
	}
}

void fcrash_state::fcrash_render_high_layer( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer )
{
	bitmap_ind16 dummy_bitmap;

	switch (layer)
	{
		case 0:
			/* there are no high priority sprites */
			break;
		case 1:
		case 2:
		case 3:
			m_bg_tilemap[layer - 1]->draw(screen, dummy_bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
			break;
	}
}

void fcrash_state::fcrash_build_palette()
{
	int offset;

	// all the bootlegs seem to write the palette offset as usual
	int palettebase = (m_cps_a_regs[0x0a / 2] << 8) & 0x1ffff;

	for (offset = 0; offset < 32 * 6 * 16; offset++)
	{
		int palette = m_gfxram[palettebase / 2 + offset];
		int r, g, b, bright;

		// from my understanding of the schematics, when the 'brightness'
		// component is set to 0 it should reduce brightness to 1/3

		bright = 0x0f + ((palette >> 12) << 1);

		r = ((palette >> 8) & 0x0f) * 0x11 * bright / 0x2d;
		g = ((palette >> 4) & 0x0f) * 0x11 * bright / 0x2d;
		b = ((palette >> 0) & 0x0f) * 0x11 * bright / 0x2d;

		m_palette->set_pen_color(offset, rgb_t(r, g, b));
	}
}

uint32_t fcrash_state::screen_update_fcrash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layercontrol, l0, l1, l2, l3;
	int videocontrol = m_cps_a_regs[0x22 / 2];

	flip_screen_set(videocontrol & 0x8000);

	layercontrol = m_cps_b_regs[m_layer_enable_reg / 2];

	/* Get video memory base registers */
	cps1_get_video_base();

	/* Build palette */
	fcrash_build_palette();

	fcrash_update_transmasks();

	m_bg_tilemap[0]->set_scrollx(0, m_scroll1x - m_layer_scroll1x_offset);
	m_bg_tilemap[0]->set_scrolly(0, m_scroll1y);

	if (videocontrol & 0x01)    /* linescroll enable */
	{
		int scrly = -m_scroll2y;
		int i;
		int otheroffs;

		m_bg_tilemap[1]->set_scroll_rows(1024);

		otheroffs = m_cps_a_regs[CPS1_ROWSCROLL_OFFS];

		for (i = 0; i < 256; i++)
			m_bg_tilemap[1]->set_scrollx((i - scrly) & 0x3ff, m_scroll2x + m_other[(i + otheroffs) & 0x3ff]);
	}
	else
	{
		m_bg_tilemap[1]->set_scroll_rows(1);
		m_bg_tilemap[1]->set_scrollx(0, m_scroll2x - m_layer_scroll2x_offset);
	}
	m_bg_tilemap[1]->set_scrolly(0, m_scroll2y);
	m_bg_tilemap[2]->set_scrollx(0, m_scroll3x - m_layer_scroll3x_offset);
	m_bg_tilemap[2]->set_scrolly(0, m_scroll3y);


	/* turn all tilemaps on regardless of settings in get_video_base() */
	/* write a custom get_video_base for this bootleg hardware? */
	m_bg_tilemap[0]->enable(1);
	m_bg_tilemap[1]->enable(1);
	m_bg_tilemap[2]->enable(1);

	/* Blank screen */
	bitmap.fill(0xbff, cliprect);

	screen.priority().fill(0, cliprect);
	l0 = (layercontrol >> 0x06) & 03;
	l1 = (layercontrol >> 0x08) & 03;
	l2 = (layercontrol >> 0x0a) & 03;
	l3 = (layercontrol >> 0x0c) & 03;

	fcrash_render_layer(screen, bitmap, cliprect, l0, 0);

	if (l1 == 0)
		fcrash_render_high_layer(screen, bitmap, cliprect, l0);

	fcrash_render_layer(screen, bitmap, cliprect, l1, 0);

	if (l2 == 0)
		fcrash_render_high_layer(screen, bitmap, cliprect, l1);

	fcrash_render_layer(screen, bitmap, cliprect, l2, 0);

	if (l3 == 0)
		fcrash_render_high_layer(screen, bitmap, cliprect, l2);

	fcrash_render_layer(screen, bitmap, cliprect, l3, 0);

	return 0;
}


// ************************************************************************* FCRASH

ROM_START( fcrash )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "9.bin",  0x00000, 0x20000, CRC(c6854c91) SHA1(29f01cc65be5eaa3f86e99eebdd284104623abb0) )
	ROM_LOAD16_BYTE( "5.bin",  0x00001, 0x20000, CRC(77f7c2b3) SHA1(feea48d9555824a2e5bf5e99ce159edc015f0792) )
	ROM_LOAD16_BYTE( "8.bin",  0x40000, 0x20000, CRC(1895b3df) SHA1(415a26050c50ed79a7ee5ddd1b8d61593b1ce876) )
	ROM_LOAD16_BYTE( "4.bin",  0x40001, 0x20000, CRC(bbd411ee) SHA1(85d50ca72ec46d627f9c88ff0809aa30e164821a) )
	ROM_LOAD16_BYTE( "7.bin",  0x80000, 0x20000, CRC(5b23ebf2) SHA1(8c28c21a72a28ad249170026891c6bb865943f84) )
	ROM_LOAD16_BYTE( "3.bin",  0x80001, 0x20000, CRC(aba2aebe) SHA1(294109b5929ed63859a55bef16643e3ade7da16f) )
	ROM_LOAD16_BYTE( "6.bin",  0xc0000, 0x20000, CRC(d4bf37f6) SHA1(f47e1cc9aa3b3019ee57f59715e3a611acf9fe3e) )
	ROM_LOAD16_BYTE( "2.bin",  0xc0001, 0x20000, CRC(07ac8f43) SHA1(7a41b003c76adaabd3f94929cc163461b70e0ed9) )
	//ROM_FILL(0x2610, 1, 7)  // temporary patch to fix transitions

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Audio CPU + Sample Data */
	ROM_LOAD( "1.bin",   0x00000, 0x20000, CRC(5b276c14) SHA1(73e53c077d4e3c1b919eee28b29e34176ee204f8) )
	ROM_RELOAD(          0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD32_BYTE( "18.bin",     0x000000, 0x20000, CRC(f1eee6d9) SHA1(bee95efbff49c582cff1cc6d9bb5ef4ea5c4a074) )
	ROM_LOAD32_BYTE( "20.bin",     0x000001, 0x20000, CRC(675f4537) SHA1(acc68822da3aafbb62f76cbffa5f3389fcc91447) )
	ROM_LOAD32_BYTE( "22.bin",     0x000002, 0x20000, CRC(db8a32ac) SHA1(b95f73dff291acee239e22e5fd7efe15d0de23be) )
	ROM_LOAD32_BYTE( "24.bin",     0x000003, 0x20000, CRC(f4113e57) SHA1(ff1f443c13494a169b9be24abc361d27a6d01c09) )
	ROM_LOAD32_BYTE( "10.bin",     0x080000, 0x20000, CRC(d478853e) SHA1(91fcf8eb022ccea66d291bec84ace557181cf861) )
	ROM_LOAD32_BYTE( "12.bin",     0x080001, 0x20000, CRC(25055642) SHA1(578cf6a436489cc1f2d1acdb0cba6c1cbee2e21f) )
	ROM_LOAD32_BYTE( "14.bin",     0x080002, 0x20000, CRC(b77d0328) SHA1(42eb1ebfda301f2b09f3add5932e8331f4790706) )
	ROM_LOAD32_BYTE( "16.bin",     0x080003, 0x20000, CRC(ea111a79) SHA1(1b86aa984d2d6c527e96b61274a82263f34d0d89) )
	ROM_LOAD32_BYTE( "19.bin",     0x100000, 0x20000, CRC(b3aa1f48) SHA1(411f3855739992f5967e915f2a5255afcedeac2e) ) // only these 4 differ from ffightbla (new title logo)
	ROM_LOAD32_BYTE( "21.bin",     0x100001, 0x20000, CRC(04d175c9) SHA1(33e6e3fefae4e3977c8c954fbd7feff36e92d723) ) // ^
	ROM_LOAD32_BYTE( "23.bin",     0x100002, 0x20000, CRC(e592ba4f) SHA1(62559481e0da3954a90da0ab0fb51f87f1b3dd9d) ) // ^
	ROM_LOAD32_BYTE( "25.bin",     0x100003, 0x20000, CRC(b89a740f) SHA1(516d73c772e0a904dfb0bd84874919d78bbbd200) ) // ^
	ROM_LOAD32_BYTE( "11.bin",     0x180000, 0x20000, CRC(d4457a60) SHA1(9e956efafa81a81aca92837df03968f5670ffc15) )
	ROM_LOAD32_BYTE( "13.bin",     0x180001, 0x20000, CRC(3b26a37d) SHA1(58d8d0cdef81c938fb1a5595f2d02b228865893b) )
	ROM_LOAD32_BYTE( "15.bin",     0x180002, 0x20000, CRC(6d837e09) SHA1(b4a133ab96c35b689ee692bfcc04981791099b6f) )
	ROM_LOAD32_BYTE( "17.bin",     0x180003, 0x20000, CRC(c59a4d6c) SHA1(59e49c7d24dd333007de4bb621050011a5392bcc) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )   /* stars */
ROM_END

ROM_START( ffightbl )
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "fg-e.bin",  0x00000, 0x80000, CRC(f8ccf27e) SHA1(08ff445d946da81e7dc0cc021f686b5968fa34ab) )
	ROM_LOAD16_BYTE( "fg-f.bin",  0x00001, 0x80000, CRC(d96c76b2) SHA1(3f9ca4625491cab07cf4a1bf001f1325dc3652a3) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Audio CPU + Sample Data */
	ROM_LOAD( "ff1.bin",   0x00000, 0x20000, CRC(5b276c14) SHA1(73e53c077d4e3c1b919eee28b29e34176ee204f8) )
	ROM_RELOAD(          0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD32_BYTE( "fg-d.bin",     0x000000, 0x80000, CRC(4303f863) SHA1(72a3246e14f9c4d1fb4712bd67d087db42d722d9) )
	ROM_LOAD32_BYTE( "fg-c.bin",     0x000001, 0x80000, CRC(d1dfcd2d) SHA1(8796db70459e1e6232a75f5c3f4bf8b227b16f46) )
	ROM_LOAD32_BYTE( "fg-b.bin",     0x000002, 0x80000, CRC(22f2c097) SHA1(bbf2d30d31c5a7802b7f7f164dd51a4584511936) )
	ROM_LOAD32_BYTE( "fg-a.bin",     0x000003, 0x80000, CRC(16a89b2c) SHA1(4d0e1ec6ae9a2bd31fa77140532bbce64d3874e9) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )   /* stars */
ROM_END

/*
	this is identical to the Final Crash bootleg but without the modified gfx.
	it's less common than Final Crash, but is either the original bootleg, or the bootleggers wanted to restore the
	original title.
*/
ROM_START( ffightbla )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "9.bin",  0x00000, 0x20000, CRC(c6854c91) SHA1(29f01cc65be5eaa3f86e99eebdd284104623abb0) )
	ROM_LOAD16_BYTE( "5.bin",  0x00001, 0x20000, CRC(77f7c2b3) SHA1(feea48d9555824a2e5bf5e99ce159edc015f0792) )
	ROM_LOAD16_BYTE( "8.bin",  0x40000, 0x20000, CRC(1895b3df) SHA1(415a26050c50ed79a7ee5ddd1b8d61593b1ce876) )
	ROM_LOAD16_BYTE( "4.bin",  0x40001, 0x20000, CRC(bbd411ee) SHA1(85d50ca72ec46d627f9c88ff0809aa30e164821a) )
	ROM_LOAD16_BYTE( "7.bin",  0x80000, 0x20000, CRC(5b23ebf2) SHA1(8c28c21a72a28ad249170026891c6bb865943f84) )
	ROM_LOAD16_BYTE( "3.bin",  0x80001, 0x20000, CRC(aba2aebe) SHA1(294109b5929ed63859a55bef16643e3ade7da16f) )
	ROM_LOAD16_BYTE( "6.bin",  0xc0000, 0x20000, CRC(d4bf37f6) SHA1(f47e1cc9aa3b3019ee57f59715e3a611acf9fe3e) )
	ROM_LOAD16_BYTE( "2.bin",  0xc0001, 0x20000, CRC(07ac8f43) SHA1(7a41b003c76adaabd3f94929cc163461b70e0ed9) )
	//ROM_FILL(0x2610, 1, 7)  // temporary patch to fix transitions

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* Audio CPU + Sample Data */
	ROM_LOAD( "1.bin",   0x00000, 0x20000, CRC(5b276c14) SHA1(73e53c077d4e3c1b919eee28b29e34176ee204f8) )
	ROM_RELOAD(          0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD32_BYTE( "18.bin",     0x000000, 0x20000, CRC(f1eee6d9) SHA1(bee95efbff49c582cff1cc6d9bb5ef4ea5c4a074) )
	ROM_LOAD32_BYTE( "20.bin",     0x000001, 0x20000, CRC(675f4537) SHA1(acc68822da3aafbb62f76cbffa5f3389fcc91447) )
	ROM_LOAD32_BYTE( "22.bin",     0x000002, 0x20000, CRC(db8a32ac) SHA1(b95f73dff291acee239e22e5fd7efe15d0de23be) )
	ROM_LOAD32_BYTE( "24.bin",     0x000003, 0x20000, CRC(f4113e57) SHA1(ff1f443c13494a169b9be24abc361d27a6d01c09) )
	ROM_LOAD32_BYTE( "10.bin",     0x080000, 0x20000, CRC(d478853e) SHA1(91fcf8eb022ccea66d291bec84ace557181cf861) )
	ROM_LOAD32_BYTE( "12.bin",     0x080001, 0x20000, CRC(25055642) SHA1(578cf6a436489cc1f2d1acdb0cba6c1cbee2e21f) )
	ROM_LOAD32_BYTE( "14.bin",     0x080002, 0x20000, CRC(b77d0328) SHA1(42eb1ebfda301f2b09f3add5932e8331f4790706) )
	ROM_LOAD32_BYTE( "16.bin",     0x080003, 0x20000, CRC(ea111a79) SHA1(1b86aa984d2d6c527e96b61274a82263f34d0d89) )
	ROM_LOAD32_BYTE( "ff-19.bin",  0x100000, 0x20000, CRC(7bc03747) SHA1(6964e5c562d6af5b4327ff828f3d0522c34911bc) ) // only these 4 differ from fcrash
	ROM_LOAD32_BYTE( "ff-21.bin",  0x100001, 0x20000, CRC(0c248e2b) SHA1(28731fe25a8eb39c1e0822cf9074a7a32c6b2978) ) // ^
	ROM_LOAD32_BYTE( "ff-23.bin",  0x100002, 0x20000, CRC(53949d0e) SHA1(1b11134005a47c323917b9892fe44819c36c6ee2) ) // ^
	ROM_LOAD32_BYTE( "ff-25.bin",  0x100003, 0x20000, CRC(8d34a67d) SHA1(69e9f52efb73952313848a6d54dbdc17a2275c59) ) // ^
	ROM_LOAD32_BYTE( "11.bin",     0x180000, 0x20000, CRC(d4457a60) SHA1(9e956efafa81a81aca92837df03968f5670ffc15) )
	ROM_LOAD32_BYTE( "13.bin",     0x180001, 0x20000, CRC(3b26a37d) SHA1(58d8d0cdef81c938fb1a5595f2d02b228865893b) )
	ROM_LOAD32_BYTE( "15.bin",     0x180002, 0x20000, CRC(6d837e09) SHA1(b4a133ab96c35b689ee692bfcc04981791099b6f) )
	ROM_LOAD32_BYTE( "17.bin",     0x180003, 0x20000, CRC(c59a4d6c) SHA1(59e49c7d24dd333007de4bb621050011a5392bcc) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )   /* stars */
ROM_END


// ************************************************************************* KODB

/*
	CPU
	1x TS68000CP12 (main)
	1x TPC1020AFN-084C
	1x Z8400BB1-Z80CPU (sound)
	1x YM2151 (sound)
	1x YM3012A (sound)
	1x OKI-M6295 (sound)
	2x LM324N (sound)
	1x TDA2003 (sound)
	1x oscillator 10.0 MHz
	1x oscillator 22.1184 MHz
	
	ROMs
	1x AM27C512 (1)(sound)
	1x AM27C020 (2)(sound)
	2x AM27C040 (3,4)(main)
	1x Am27C040 (bp)(gfx)
	7x mask ROM (ai,bi,ci,di,ap,cp,dp)(gfx)
	1x GAL20V8A (not dumped)
	3x GAL16V8A (not dumped)
	1x PALCE20V8H (not dumped)
	1x GAL20V8S (not dumped)
	
	Note
	1x JAMMA edge connector
	1x trimmer (volume)
	3x 8 switches dip
*/
ROM_START( kodb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.ic172",    0x00000, 0x080000, CRC(036dd74c) SHA1(489344e56863429e86b4c362b82d89819c1d6afb) )
	ROM_LOAD16_BYTE( "4.ic171",    0x00001, 0x080000, CRC(3e4b7295) SHA1(3245640bae7d141238051dfe5c7683d05c6d3848) )
	//ROM_FILL( 0x952, 1, 7)  // temporary patch to fix transitions

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "1.ic28",        0x00000, 0x08000, CRC(01cae60c) SHA1(b2cdd883fd859f0b701230831aca1f1a74ad6087) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD64_BYTE( "cp.ic90",   0x000000, 0x80000, CRC(e3b8589e) SHA1(775f97e43cb995b93da40063a1f1e4d73b34437c) )
	ROM_LOAD64_BYTE( "dp.ic89",   0x000001, 0x80000, CRC(3eec9580) SHA1(3d8d0cfbeae077544e514a5eb96cc83f716e494f) )
	ROM_LOAD64_BYTE( "ap.ic88",   0x000002, 0x80000, CRC(fdf5f163) SHA1(271ee96886c958accaca9a82484ab80fe32bd38e) )
	ROM_LOAD64_BYTE( "bp.ic87",   0x000003, 0x80000, CRC(4e1c52b7) SHA1(74570e7d577c999c62203c97b3d449e3b61a678a) )
	ROM_LOAD64_BYTE( "ci.ic91",   0x000004, 0x80000, CRC(22228bc5) SHA1(d48a09ee284d9e4b986f5c3c1c865930f76986e2) )
	ROM_LOAD64_BYTE( "di.ic92",   0x000005, 0x80000, CRC(ab031763) SHA1(5bcd89b1debf029b779aa1bb73b3a572d27154ec) )
	ROM_LOAD64_BYTE( "ai.ic93",   0x000006, 0x80000, CRC(cffbf4be) SHA1(f805bafc855d4a656c055a76eaeb26e36835541e) )
	ROM_LOAD64_BYTE( "bi.ic94",   0x000007, 0x80000, CRC(4a1b43fe) SHA1(7957f45b2862825c9509043c63c7da7108bd251b) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )   /* stars */

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "2.ic19",      0x00000, 0x40000, CRC(a2db1575) SHA1(1a4a29e4b045af50700adf1665697feab12cc234) )
ROM_END


// ************************************************************************* CAWINGBL, CAWINGB2

ROM_START( cawingbl )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "caw2.bin",    0x00000, 0x80000, CRC(8125d3f0) SHA1(a0e48c326c6164ca189c9372f5c38a7c103772c1) )
	ROM_LOAD16_BYTE( "caw1.bin",    0x00001, 0x80000, CRC(b19b10ce) SHA1(3c71f1dc830d1e8b8ba26d8a71e12f477659480c) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD32_BYTE( "caw7.bin", 0x000000, 0x80000, CRC(a045c689) SHA1(8946c55635121282ea03586a278e50de20d92633) )
	ROM_LOAD32_BYTE( "caw6.bin", 0x000001, 0x80000, CRC(61192f7c) SHA1(86643c62653a62a5c7541d50cfdecae9b607440d) )
	ROM_LOAD32_BYTE( "caw5.bin", 0x000002, 0x80000, CRC(30dd78db) SHA1(e0295001d6f5fb4a9276c432f971e88f73c5e39a) )
	ROM_LOAD32_BYTE( "caw4.bin", 0x000003, 0x80000, CRC(4937fc41) SHA1(dac179715be483a521df8e515afc1fb7a2cd8f13) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "caw3.bin",  0x00000, 0x20000, CRC(ffe16cdc) SHA1(8069ea69f0b89d61c35995c8040a4989d7be9c1f) )
	ROM_RELOAD(            0x10000, 0x20000 )
ROM_END

ROM_START( cawingb2 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "8.8",    0x00000, 0x20000, CRC(f655708c) SHA1(9962a1c96ea08bc71b25d4f58e5d1fb1beebf0dc) )
	ROM_LOAD16_BYTE( "4.4",    0x00001, 0x20000, CRC(a02fb5aa) SHA1(c9c064a83899c48f681ac803cfc5886503b9d992) )
	ROM_LOAD16_BYTE( "7.7",    0x40000, 0x20000, CRC(8c6c7430) SHA1(3ed5713caf774b050b41a6adea026e1307b570df) )
	ROM_LOAD16_BYTE( "3.3",    0x40001, 0x20000, CRC(f585bf2c) SHA1(3a3169791f8deace8d9bee1adb08f19fbcd309c6) )
	ROM_LOAD16_BYTE( "6.6",    0x80000, 0x20000, CRC(5fda906e) SHA1(7b3ef17d494a2f92e58ab7e34a3beaad8c149fca) )
	ROM_LOAD16_BYTE( "2.2",    0x80001, 0x20000, CRC(736c1835) SHA1(a91f479fab30603a111304adc0478d430faa80fc) )
	ROM_LOAD16_BYTE( "5.5",    0xc0000, 0x20000, CRC(76458083) SHA1(cbb4ef5f7615c834b2ee1ad3c86e7262f2f62c01) )
	ROM_LOAD16_BYTE( "1.1",    0xc0001, 0x20000, CRC(d3523f34) SHA1(005ea378c2b78782f85ecc591946c027ca2ca023) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD32_BYTE( "17.17",     0x000000, 0x20000, CRC(0b538062) SHA1(ac6e5dc82efdca311adfe6e6cdda160ad4a0d04d) )
	ROM_LOAD32_BYTE( "19.19",     0x000001, 0x20000, CRC(3ad62311) SHA1(1c132696b55191d16af30ebd36d2320d979eab36) )
	ROM_LOAD32_BYTE( "21.21",     0x000002, 0x20000, CRC(1b872a98) SHA1(7a3f72c6d384dfa8e224f93604997a7b6e5c8926) )
	ROM_LOAD32_BYTE( "23.23",     0x000003, 0x20000, CRC(ad49eecd) SHA1(39909996765391ed734a02c74f683e1bd9ce1561) )
	ROM_LOAD32_BYTE( "9.9",       0x080000, 0x20000, CRC(8cd4df5b) SHA1(771b6d6a6baa95a669335fe64e2219fe7226e140) )
	ROM_LOAD32_BYTE( "11.11",     0x080001, 0x20000, CRC(bf14418a) SHA1(7a0e1c65b8825a252338d6c1db59a88966ec6cfb) )
	ROM_LOAD32_BYTE( "13.13",     0x080002, 0x20000, CRC(cef1aab8) SHA1(677a889b939ff00e95737a4a53053744bb6744c0) )
	ROM_LOAD32_BYTE( "15.15",     0x080003, 0x20000, CRC(397725dc) SHA1(9450362bbf2f91b4225a088d6e283d7b16407b74) )
	ROM_LOAD32_BYTE( "18.18",     0x100000, 0x20000, CRC(9b14f7ed) SHA1(72b6e1174d4faab487261aa6739de842d2423e1a) )
	ROM_LOAD32_BYTE( "20.20",     0x100001, 0x20000, CRC(59bcc1bb) SHA1(c725060e068294dea1d962c54a9018050fa70297) )
	ROM_LOAD32_BYTE( "22.22",     0x100002, 0x20000, CRC(23dc647a) SHA1(2d8d4c4c7b2d0616430360d1639b07216dd731d6) )
	ROM_LOAD32_BYTE( "24.24",     0x100003, 0x20000, CRC(eda9fa6b) SHA1(4a3510ce71b015a1ea568fd0bbe61c5c093a2fbf) )
	ROM_LOAD32_BYTE( "10.10",     0x180000, 0x20000, CRC(17174249) SHA1(71c6424ab4629065dd6af8bb47b18f5b5d0fbe49) )
	ROM_LOAD32_BYTE( "12.12",     0x180001, 0x20000, CRC(490440b2) SHA1(2597bf16340308f84b32cfa048c426db571b4a35) )
	ROM_LOAD32_BYTE( "14.14",     0x180002, 0x20000, CRC(344a8270) SHA1(fdb588a7ba60783225e3b5c72446f79625de4f9c) )
	ROM_LOAD32_BYTE( "16.16",     0x180003, 0x20000, CRC(b991ad91) SHA1(5c59131ddf068cb54d23f8836293360fbc967d58) )

	ROM_REGION( 0x30000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "5.a",       0x00000, 0x20000, CRC(ffe16cdc) SHA1(8069ea69f0b89d61c35995c8040a4989d7be9c1f) )
	ROM_RELOAD(            0x10000, 0x20000 )
ROM_END


// ************************************************************************* MTWINSB

/* board marked MGT-026 */
ROM_START( mtwinsb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "1-prg-27c4001.bin",     0x00001, 0x80000, CRC(8938a029) SHA1(50104d2afaec8d69d317780c071a4f2248e23e62) )
	ROM_LOAD16_BYTE( "2-prg-27c4001.bin",     0x00000, 0x80000, CRC(7d5b8a97) SHA1(d3e456061a569765d400fc7c9b43e4fdacf17951) )

	ROM_REGION( 0x200000, "gfx", 0 ) // identical to the original, but differently arranged
	ROM_LOAD64_BYTE( "g4.bin",  0x000004, 0x40000, CRC(11493e55) SHA1(0e45f53b034d66ce8d029346d4d88e46021df1a7) )
	ROM_CONTINUE(               0x000000, 0x40000)
	ROM_LOAD64_BYTE( "g3.bin",  0x000005, 0x40000, CRC(feda0f8b) SHA1(59c740478791ce95bf06feeda5173cc283a1eaea) )
	ROM_CONTINUE(               0x000001, 0x40000)
	ROM_LOAD64_BYTE( "g2.bin",  0x000006, 0x40000, CRC(745f0eba) SHA1(1cb07be5df7cc43b5aa236f114d303bf92436c74) )
	ROM_CONTINUE(               0x000002, 0x40000)
	ROM_LOAD64_BYTE( "g1.bin",  0x000007, 0x40000, CRC(8069026f) SHA1(3d5e9b36a349328bcd93d83d8d2fe3cd40e68a3b) )
	ROM_CONTINUE(               0x000003, 0x40000)

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "4-snd-z80-27c512.bin", 0x00000, 0x08000, CRC(4d4255b7) SHA1(81a76b58043af7252a854b7efc4109957ef0e679) ) // identical to the original
	ROM_CONTINUE(          0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "3-snd-27c208.bin", 0x00000, 0x40000, CRC(a0c3de92) SHA1(5135cd982564f898f799ff1bc2bb2a75154be0cd) ) // identical to the original, but one single bigger ROM
ROM_END


// ************************************************************************* SGYXZ

/*
	24mhz crystal (maincpu), 28.322 crystal (video), 3.579545 crystal (sound)
	sound cpu is (239 V 249521 VC5006 KABUKI DL-030P-110V) - recycled Kabuki Z80 from genuine Capcom HW?
	3x8 dsws
*/
ROM_START( sgyxz )
	ROM_REGION( CODE_SIZE, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "sgyxz_prg1.bin", 0x000001, 0x20000, CRC(d8511929) SHA1(4de9263778f327693f4d1e21b48e43806f673487) )
	ROM_CONTINUE( 0x80001, 0x20000 )
	ROM_CONTINUE( 0x40001, 0x20000 )
	ROM_CONTINUE( 0xc0001, 0x20000 )
	ROM_LOAD16_BYTE( "sgyxz_prg2.bin", 0x000000, 0x20000, CRC(95429c83) SHA1(e981624d018132e5625a66113b6ac4fc44e55cf7) )
	ROM_CONTINUE( 0x80000, 0x20000 )
	ROM_CONTINUE( 0x40000, 0x20000 )
	ROM_CONTINUE( 0xc0000, 0x20000 )
	ROM_FILL(0x708da, 4, 0xff) // patch out protections
	ROM_FILL(0xf11ea, 1, 0x60)
	ROM_FILL(0x00007, 1, 0xa2) // start address
	ROM_FILL(0x02448, 1, 0x07) // transitions

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD64_WORD("sgyxz_gfx1.bin", 0x000000, 0x80000, CRC(a60be9f6) SHA1(2298a4b6a2c83b76dc106a1efa19606b298d378a) ) // 'picture 1'
	ROM_CONTINUE(                     0x000004, 0x80000 )
	ROM_CONTINUE(                     0x200000, 0x80000 )
	ROM_CONTINUE(                     0x200004, 0x80000 )
	ROM_LOAD64_WORD("sgyxz_gfx2.bin", 0x000002, 0x80000, CRC(6ad9d048) SHA1(d47212d28d0a1ce349e4c59e5d0d99c541b3458e) ) // 'picture 2'
	ROM_CONTINUE(                     0x000006, 0x80000 )
	ROM_CONTINUE(                     0x200002, 0x80000 )
	ROM_CONTINUE(                     0x200006, 0x80000 )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "sgyxz_snd2.bin", 0x00000, 0x10000,  CRC(210c376f) SHA1(0d937c86078d0a106f5636b7daf5fc0266c2c2ec) )
	ROM_RELOAD(           0x8000, 0x10000 )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sgyxz_snd1.bin", 0x00000, 0x40000,  CRC(c15ac0f2) SHA1(8d9e5519d9820e4ac4f70555088c80e64d052c9d) )
ROM_END


// ************************************************************************* WOFABL

ROM_START( wofabl )
	ROM_REGION( CODE_SIZE, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "5.prg.040", 0x000000, 0x80000, CRC(4d9d2327) SHA1(b8029b117083a1c31546455fa53d9ee83a4ff7ad) )
	ROM_LOAD16_BYTE( "3.prg.040", 0x000001, 0x80000, CRC(ef25fe49) SHA1(d45d3c94cb57187b2f6ac248e9c3c9989be38f99) )
	ROM_LOAD16_BYTE( "6.prg.010", 0x100000, 0x20000, CRC(93eeb161) SHA1(0b8efb7ace59791ffb8a3f7826f0ea74620d7a0f) ) // x111111xxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "4.prg.010", 0x100001, 0x20000, CRC(a0751944) SHA1(84f092992f0f94acffbbb43168fbcee2c45da789) ) // x111111xxxxxxxxxx = 0xFF

	ROM_REGION( 0x400000, "gfx", 0 ) /* rearranged in init */
	ROM_LOAD64_WORD( "gfx13.040",  0x000000, 0x80000, CRC(8e8db215) SHA1(cc85e576bf09c3edab9afc1b5fa0a152f4140c06) )
	ROM_LOAD64_WORD( "gfx15.040",  0x000002, 0x80000, CRC(a5e4f449) SHA1(9956f82818ccc685367b5fe5e4bc8b59b65c31c1) )
	ROM_LOAD64_WORD( "gfx14.040",  0x000004, 0x80000, CRC(f34a7f9d) SHA1(6d67623c93147a779f07ef103188f3e2cb6d6d6e) )
	ROM_LOAD64_WORD( "gfx16.040",  0x000006, 0x80000, CRC(49a3dfc7) SHA1(c14ea91745fd72be936b6db9981d12d958326757) )
	ROM_LOAD64_WORD( "gfx9.040",   0x200000, 0x80000, CRC(f8f33a0e) SHA1(33f172b79499d4a76b53c070c0007bd1604a71bd) )
	ROM_LOAD64_WORD( "gfx11.040",  0x200002, 0x80000, CRC(13324965) SHA1(979754ebd15a2989f92b5b7fc5bae99eb83c3593) )
	ROM_LOAD64_WORD( "gfx10.040",  0x200004, 0x80000, CRC(6a060c6c) SHA1(49e4da9373272e5889caa79a86c39ee34087c480) )
	ROM_LOAD64_WORD( "gfx12.040",  0x200006, 0x80000, CRC(c29f7b70) SHA1(95d22dcd9e2a48ddea7573d0be75225e0aae798f) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) /* Z80 code */
	ROM_LOAD( "sound.512", 0x00000, 0x10000,  CRC(210c376f) SHA1(0d937c86078d0a106f5636b7daf5fc0266c2c2ec) ) // identical to sgyxz
	ROM_RELOAD(           0x8000, 0x10000 )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sound.020", 0x00000, 0x40000,  CRC(672dcb46) SHA1(e76c1ce81689a55b573fb6e5c9a860cb756cd876) ) // almost identical to sgyxz
ROM_END


// ************************************************************************* SF2M1, SF2M9

ROM_START( sf2m1 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "222e",              0x000000, 0x80000, CRC(1e20d0a3) SHA1(5e05b52fd938aff5190bca7e178705d7236aef66) )
	ROM_LOAD16_BYTE( "196e",              0x000001, 0x80000, CRC(88cc38a3) SHA1(6049962f943bd37748a9531cc3254e8b59326eac) )
	ROM_LOAD16_WORD_SWAP( "s92_21a.bin",  0x100000, 0x80000, CRC(925a7877) SHA1(1960dca35f0ca6f2b399a9fccfbc0132ac6425d1) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROM_LOAD64_WORD( "s92_01.bin",   0x000000, 0x80000, CRC(03b0d852) SHA1(f370f25c96ad2b94f8c53d6b7139100285a25bef) )
	ROM_LOAD64_WORD( "s92_02.bin",   0x000002, 0x80000, CRC(840289ec) SHA1(2fb42a242f60ba7e74009b5a90eb26e035ba1e82) )
	ROM_LOAD64_WORD( "s92_03.bin",   0x000004, 0x80000, CRC(cdb5f027) SHA1(4c7d944fef200fdfcaf57758b901b5511188ed2e) )
	ROM_LOAD64_WORD( "s92_04.bin",   0x000006, 0x80000, CRC(e2799472) SHA1(27d3796429338d82a8de246a0ea06dd487a87768) )
	ROM_LOAD64_WORD( "s92_05.bin",   0x200000, 0x80000, CRC(ba8a2761) SHA1(4b696d66c51611e43522bed752654314e76d33b6) )
	ROM_LOAD64_WORD( "s92_06.bin",   0x200002, 0x80000, CRC(e584bfb5) SHA1(ebdf1f5e2638eed3a65dda82b1ed9151a355f4c9) )
	ROM_LOAD64_WORD( "s92_07.bin",   0x200004, 0x80000, CRC(21e3f87d) SHA1(4a4961bb68c3a1ce15f9d393d9c03ecb2466cc29) )
	ROM_LOAD64_WORD( "s92_08.bin",   0x200006, 0x80000, CRC(befc47df) SHA1(520390420da3a0271ba90b0a933e65143265e5cf) )
	ROM_LOAD64_WORD( "s92_10.bin",   0x400000, 0x80000, CRC(960687d5) SHA1(2868c31121b1c7564e9767b9a19cdbf655c7ed1d) )
	ROM_LOAD64_WORD( "s92_11.bin",   0x400002, 0x80000, CRC(978ecd18) SHA1(648a59706b93c84b4206a968ecbdc3e834c476f6) )
	ROM_LOAD64_WORD( "s92_12.bin",   0x400004, 0x80000, CRC(d6ec9a0a) SHA1(ed6143f8737013b6ef1684e37c05e037e7a80dae) )
	ROM_LOAD64_WORD( "s92_13.bin",   0x400006, 0x80000, CRC(ed2c67f6) SHA1(0083c0ffaf6fe7659ff0cf822be4346cd6e61329) )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "s92_09.bin",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "s92_18.bin",    0x00000, 0x20000, CRC(7f162009) SHA1(346bf42992b4c36c593e21901e22c87ae4a7d86d) )
	ROM_LOAD( "s92_19.bin",    0x20000, 0x20000, CRC(beade53f) SHA1(277c397dc12752719ec6b47d2224750bd1c07f79) )
ROM_END

ROM_START( sf2m9 )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "27040.6", 0x000000, 0x80000, CRC(16c6372e) SHA1(5d5a49392f2fb806e66e0ac137df00425ca52e7f) )
	ROM_LOAD16_BYTE( "27040.5", 0x000001, 0x80000, CRC(137d5f2e) SHA1(835e9b767e6499f161c5c4fd9a31a9f54b3ee68f) )
	ROM_LOAD16_BYTE( "27010.4", 0x100000, 0x20000, CRC(8226c11c) SHA1(9588bd64e338901394805aca8a234f880674dc60) )
	ROM_LOAD16_BYTE( "27010.3", 0x100001, 0x20000, CRC(924c6ce2) SHA1(676a912652bd75da5087f0c7eae047b7681a993c) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROM_LOAD64_BYTE( "tat-01.bin",    0x000000, 0x40000, CRC(a887f7d4) SHA1(d7e0c46b3ab1c6352f45033cb9e610d9c34d51fb) )
	ROM_CONTINUE(                     0x000004, 0x40000)
	ROM_LOAD64_BYTE( "tat-05.bin",    0x000001, 0x40000, CRC(9390ff23) SHA1(b234169615aa952e3b15c7b0dfb495e499ba49ef) )
	ROM_CONTINUE(                     0x000005, 0x40000)
	ROM_LOAD64_BYTE( "tat-02.bin",    0x000002, 0x40000, CRC(afb3b589) SHA1(9721fa705d62814e416c38a6c3e698efb9385a98) )
	ROM_CONTINUE(                     0x000006, 0x40000)
	ROM_LOAD64_BYTE( "tat-06.bin",    0x000003, 0x40000, CRC(90f2053e) SHA1(a78710421e702b410650c45c3dec21bf16799fb4) )
	ROM_CONTINUE(                     0x000007, 0x40000)

	ROM_LOAD64_BYTE( "tat-03.bin",    0x200000, 0x40000, CRC(79fa8bf0) SHA1(9f8f7b8dc54a75226beb017b9ca9fd62a9e42f6b) )
	ROM_CONTINUE(                     0x200004, 0x40000)
	ROM_LOAD64_BYTE( "tat-07.bin",    0x200001, 0x40000, CRC(6a5f153c) SHA1(f3d82ad01e2e4bdb2039815747fa14399c69753a) )
	ROM_CONTINUE(                     0x200005, 0x40000)
	ROM_LOAD64_BYTE( "tat-04.bin",    0x200002, 0x40000, CRC(32518120) SHA1(56ffa5fffb714cff8be8be5a3675b8a5fa29b2bc) )
	ROM_CONTINUE(                     0x200006, 0x40000)
	ROM_LOAD64_BYTE( "tat-08.bin",    0x200003, 0x40000, CRC(c16579ae) SHA1(42c9d6df9f3b015f5d1ad4fa2b34ea90bb37bcae) )
	ROM_CONTINUE(                     0x200007, 0x40000)

	ROM_LOAD64_BYTE( "tat-09.bin",    0x400000, 0x40000, CRC(169d85a6) SHA1(dd98c8807e80465858b2eac10825e598c37e1a93) )
	ROM_CONTINUE(                     0x400004, 0x40000)
	ROM_LOAD64_BYTE( "tat-11.bin",    0x400001, 0x40000, CRC(32a3a841) SHA1(6f9a13b8828998d194dd3933b032c75efed9cab3) )
	ROM_CONTINUE(                     0x400005, 0x40000)
	ROM_LOAD64_BYTE( "tat-10.bin",    0x400002, 0x40000, CRC(0c638630) SHA1(709d183d181a0509c7ed839c59214851468d2bb8) )
	ROM_CONTINUE(                     0x400006, 0x40000)
	ROM_LOAD64_BYTE( "tat-12.bin",    0x400003, 0x40000, CRC(6ee19b94) SHA1(c45119d04879b6ca23a3f7749175c56b381b43f2) )
	ROM_CONTINUE(                     0x400007, 0x40000)

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "27512.1",    0x00000, 0x08000, CRC(08f6b60e) SHA1(8258fcaca4ac419312531eec67079b97f471179c) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "27020.2",    0x00000, 0x40000, CRC(6cfffb11) SHA1(995526183ffd35f92e9096500a3fe6237faaa2dd) )

	ROM_REGION( 0x00c8d, "pld", 0 ) /* pal/gal */
	ROM_LOAD( "gal20v8.68kadd",    0x00000, 0x00c8d, CRC(27cdd376) SHA1(9fb5844b33002bec80fb92d3e5d1bbc779087300) )  //68k address decoder
ROM_END


// ************************************************************************* VARTHB

ROM_START( varthb )
	ROM_REGION( CODE_SIZE, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "2",   0x000000, 0x80000, CRC(2f010023) SHA1(bf4b6c0cd82cf1b86e17d6ea2670110c06e6eabe) )
	ROM_LOAD16_BYTE( "1",   0x000001, 0x80000, CRC(0861dff3) SHA1(bf6dfe18ecaeaa1bbea09267014891c4a4a07943) )
	ROM_LOAD16_BYTE( "4",   0x100000, 0x10000, CRC(aa51e43b) SHA1(46b9dab844c55b50a47d048e5bb114911773699c) )
	ROM_LOAD16_BYTE( "3",   0x100001, 0x10000, CRC(f7e4f2f0) SHA1(2ce4eadb2d6a0e0d5745323eff2c899950ad4d3b) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD64_BYTE( "14",  0x000000, 0x40000, CRC(7ca73780) SHA1(26909db32f84157cd05719e5d1e715e76636d292)  )
	ROM_LOAD64_BYTE( "13",  0x000001, 0x40000, CRC(9fb11869) SHA1(a434fb0b588f934aaa68139495e1212a63ccf162)  )
	ROM_LOAD64_BYTE( "12",  0x000002, 0x40000, CRC(afeba416) SHA1(e722c65ea2e2bac3251c32208899484aa5ef6ad2)  )
	ROM_LOAD64_BYTE( "11",  0x000003, 0x40000, CRC(9eef3507) SHA1(ae03064ca5681fbdc43a34c54aaac11c8467428b)  )
	ROM_LOAD64_BYTE( "10",  0x000004, 0x40000, CRC(eeec6183) SHA1(40dc9c86e90d7c1a2ad600c195fe387180d95fd4)  )
	ROM_LOAD64_BYTE( "9",   0x000005, 0x40000, CRC(0e94f718) SHA1(249534f2323abcdb24099d0abc24c229c699ba94)  )
	ROM_LOAD64_BYTE( "8",   0x000006, 0x40000, CRC(c4ddc5b4) SHA1(79c2a42a664e387932b7804e7a80f5753338c3b0)  )
	ROM_LOAD64_BYTE( "7",   0x000007, 0x40000, CRC(8941ca12) SHA1(5ad5d47b8614c2899d05c65dc3b74947d4bac561)  )

	ROM_REGION( 0x18000, "audiocpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "6",    0x00000, 0x08000, CRC(7a99446e) SHA1(ca027f41e3e58be5abc33ad7380746658cb5380a) )
	ROM_CONTINUE(           0x10000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 ) /* Samples */
	ROM_LOAD( "5",    0x00000, 0x40000, CRC(1547e595) SHA1(27f47b1afd9700afd9e8167d7e4e2888be34a9e5) )

	ROM_REGION( 0x1000, "pals", 0 )
	ROM_LOAD_OPTIONAL( "varth1.bin",    0x00000, 0x157, CRC(4c6a0d99) SHA1(081a307ef38675de178dd6221e6c4e55a5bfbd87) )
	ROM_LOAD_OPTIONAL( "varth2.bin",    0x00200, 0x157, NO_DUMP ) // Registered
	ROM_LOAD_OPTIONAL( "varth3.bin",    0x00400, 0x157, NO_DUMP ) // Registered
	ROM_LOAD_OPTIONAL( "varth4.bin",    0x00600, 0x117, CRC(53317bf6) SHA1(f7b8f8b2c40429a517e3be63e5aed9573972ddfb) )
	ROM_LOAD_OPTIONAL( "varth5.bin",    0x00800, 0x157, NO_DUMP ) // Registered
	ROM_LOAD_OPTIONAL( "varth6.bin",    0x00a00, 0x157, NO_DUMP ) // Registered
ROM_END


// ************************************************************************* DRIVER MACROS

GAME( 1990, cawingbl,   cawing,  cawingbl,  cawingbl,  fcrash_state,  init_cawingbl,  ROT0,    "bootleg",  "Carrier Air Wing (bootleg with 2xYM2203 + 2xMSM5205, set 1)",  MACHINE_SUPPORTS_SAVE ) // 901012 ETC
GAME( 1990, cawingb2,   cawing,  cawingbl,  cawingbl,  fcrash_state,  init_cawingbl,  ROT0,    "bootleg",  "Carrier Air Wing (bootleg with 2xYM2203 + 2xMSM5205, set 2)",  MACHINE_SUPPORTS_SAVE ) // 901012 ETC

GAME( 1990, fcrash,     ffight,  fcrash,    fcrash,    fcrash_state,  init_cps1,      ROT0,    "bootleg (Playmark)",  "Final Crash (bootleg of Final Fight)",  MACHINE_SUPPORTS_SAVE )
GAME( 1990, ffightbl,   ffight,  fcrash,    fcrash,    fcrash_state,  init_cps1,      ROT0,    "bootleg",  "Final Fight (bootleg)",  MACHINE_SUPPORTS_SAVE )
GAME( 1990, ffightbla,  ffight,  fcrash,    fcrash,    fcrash_state,  init_cps1,      ROT0,    "bootleg",  "Final Fight (bootleg on Final Crash PCB)",  MACHINE_SUPPORTS_SAVE ) // same as Final Crash without the modified graphics

GAME( 1991, kodb,       kod,     kodb,      kodb,      fcrash_state,  init_kodb,      ROT0,    "bootleg (Playmark)",  "The King of Dragons (bootleg)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 910731  "ETC"

GAME( 1993, mtwinsb,    mtwins,  mtwinsb,   mtwins,    fcrash_state,  init_mtwinsb,   ROT0,    "David Inc. (bootleg)",  "Twins (Mega Twins bootleg)",  MACHINE_SUPPORTS_SAVE ) // based on World version

GAME( 1992, sf2m1,      sf2ce,   sf2m1,     sf2,       fcrash_state,  init_sf2m1,     ROT0,    "bootleg",  "Street Fighter II': Champion Edition (M1, bootleg)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 920313 ETC
GAME( 1992, sf2m9,      sf2ce,   sf2m1,     sf2,       fcrash_state,  init_sf2m1,     ROT0,    "bootleg",  "Street Fighter II': Champion Edition (M9, bootleg)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 920313 ETC

GAME( 1999, sgyxz,      wof,     sgyxz,     sgyxz,     fcrash_state,  init_cps1,      ROT0,    "bootleg (All-In Electronic)",  "Warriors of Fate ('sgyxz' bootleg)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )   // 921005 - Sangokushi 2
GAME( 1999, wofabl,     wof,     wofabl,    wofabl,    fcrash_state,  init_wofabl,    ROT0,    "bootleg",  "Sangokushi II (bootleg)",  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )   // heavy graphics glitches - 921005 - Sangokushi 2

GAME( 1992, varthb,     varth,   varthb,    varth,     fcrash_state,  init_mtwinsb,   ROT270,  "bootleg",  "Varth: Operation Thunderstorm (bootleg)",  MACHINE_SUPPORTS_SAVE )
