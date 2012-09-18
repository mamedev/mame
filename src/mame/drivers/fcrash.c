/* Final Crash & other CPS1 bootlegs */

/*

Final Crash is a bootleg of Final Fight

Final Fight is by Capcom and runs on CPS1 hardware
The bootleg was manufactured by Playmark of Italy

this driver depends heavily on cps1.c, but has been
kept apart in an attempt to keep cps1.c clutter free

Sound is very different from CPS1.

---

Final Crash (bootleg of final fight)

1x 68k
1x z80
2x ym2203
2x oki5205
1x osc 10mhz
1x osc 24mhz

eproms:
1.bin sound eprom
from 2.bin to 9.bin program eproms
10.bin to 25.bin gfx eproms

---

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/cps1.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"


static WRITE16_HANDLER( fcrash_soundlatch_w )
{
	cps_state *state = space.machine().driver_data<cps_state>();

	if (ACCESSING_BITS_0_7)
	{
		state->soundlatch_byte_w(space, 0, data & 0xff);
		state->m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

static WRITE8_HANDLER( fcrash_snd_bankswitch_w )
{
	cps_state *state = space.machine().driver_data<cps_state>();

	state->m_msm_1->set_output_gain(0, (data & 0x08) ? 0.0 : 1.0);
	state->m_msm_2->set_output_gain(0, (data & 0x10) ? 0.0 : 1.0);

	state->membank("bank1")->set_entry(data & 0x07);
}

static void m5205_int1( device_t *device )
{
	cps_state *state = device->machine().driver_data<cps_state>();

	msm5205_data_w(device, state->m_sample_buffer1 & 0x0f);
	state->m_sample_buffer1 >>= 4;
	state->m_sample_select1 ^= 1;
	if (state->m_sample_select1 == 0)
		state->m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static void m5205_int2( device_t *device )
{
	cps_state *state = device->machine().driver_data<cps_state>();

	msm5205_data_w(device, state->m_sample_buffer2 & 0x0f);
	state->m_sample_buffer2 >>= 4;
	state->m_sample_select2 ^= 1;
}


static WRITE8_HANDLER( fcrash_msm5205_0_data_w )
{
	cps_state *state = space.machine().driver_data<cps_state>();
	state->m_sample_buffer1 = data;
}

static WRITE8_HANDLER( fcrash_msm5205_1_data_w )
{
	cps_state *state = space.machine().driver_data<cps_state>();
	state->m_sample_buffer2 = data;
}



/* not verified */
#define CPS1_ROWSCROLL_OFFS     (0x20/2)    /* base of row scroll offsets in other RAM */

static void fcrash_update_transmasks( running_machine &machine )
{
	cps_state *state = machine.driver_data<cps_state>();
	int i;
	int priority[4];

	priority[0] = 0x26;
	priority[1] = 0x30;
	priority[2] = 0x28;
	priority[3] = 0x32;

	for (i = 0; i < 4; i++)
	{
		int mask;

		/* Get transparency registers */
		if (priority[i])
			mask = state->m_cps_b_regs[priority[i] / 2] ^ 0xffff;
		else
			mask = 0xffff;	/* completely transparent if priority masks not defined (mercs, qad) */

		state->m_bg_tilemap[0]->set_transmask(i, mask, 0x8000);
		state->m_bg_tilemap[1]->set_transmask(i, mask, 0x8000);
		state->m_bg_tilemap[2]->set_transmask(i, mask, 0x8000);
	}
}

static void fcrash_render_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	cps_state *state = machine.driver_data<cps_state>();
	int pos;
	int base = 0x50c8 / 2;

	// sprite base registers need hooking up properly.. on fcrash it is NOT cps1_cps_a_regs[0]
	//  on kodb, it might still be, unless that's just a leftover and it writes somewhere else too
//  if (state->m_cps_a_regs[0] & 0x00ff) base = 0x10c8/2;
//  printf("cps1_cps_a_regs %04x\n", state->m_cps_a_regs[0]);

	for (pos = 0x1ffc; pos >= 0x0000; pos -= 4)
	{
		int tileno;
		int xpos;
		int ypos;
		int flipx, flipy;
		int colour;

		tileno = state->m_gfxram[base +pos];
		xpos   = state->m_gfxram[base +pos + 2];
		ypos   = state->m_gfxram[base +pos - 1] & 0xff;
		flipx  = state->m_gfxram[base +pos + 1] & 0x20;
		flipy  = state->m_gfxram[base +pos + 1] & 0x40;
		colour = state->m_gfxram[base +pos + 1] & 0x1f;
		ypos = 256 - ypos;

		pdrawgfx_transpen(bitmap, cliprect, machine.gfx[2], tileno, colour, flipx, flipy, xpos + 49, ypos - 16, machine.priority_bitmap, 0x02, 15);

	}

}

static void fcrash_render_layer( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int primask )
{
	cps_state *state = machine.driver_data<cps_state>();

	switch (layer)
	{
		case 0:
			fcrash_render_sprites(machine, bitmap, cliprect);
			break;
		case 1:
		case 2:
		case 3:
			state->m_bg_tilemap[layer - 1]->draw(bitmap, cliprect, TILEMAP_DRAW_LAYER1, primask);
			break;
	}
}

static void fcrash_render_high_layer( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer )
{
	cps_state *state = machine.driver_data<cps_state>();
	bitmap_ind16 dummy_bitmap;

	switch (layer)
	{
		case 0:
			/* there are no high priority sprites */
			break;
		case 1:
		case 2:
		case 3:
			state->m_bg_tilemap[layer - 1]->draw(dummy_bitmap, cliprect, TILEMAP_DRAW_LAYER0, 1);
			break;
	}
}

static void fcrash_build_palette( running_machine &machine )
{
	cps_state *state = machine.driver_data<cps_state>();
	int offset;

	for (offset = 0; offset < 32 * 6 * 16; offset++)
	{
		int palette = state->m_gfxram[0x14000 / 2 + offset];
		int r, g, b, bright;

		// from my understanding of the schematics, when the 'brightness'
		// component is set to 0 it should reduce brightness to 1/3

		bright = 0x0f + ((palette >> 12) << 1);

		r = ((palette >> 8) & 0x0f) * 0x11 * bright / 0x2d;
		g = ((palette >> 4) & 0x0f) * 0x11 * bright / 0x2d;
		b = ((palette >> 0) & 0x0f) * 0x11 * bright / 0x2d;

		palette_set_color (machine, offset, MAKE_RGB(r, g, b));
	}
}

UINT32 cps_state::screen_update_fcrash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layercontrol, l0, l1, l2, l3;
	int videocontrol = m_cps_a_regs[0x22 / 2];


	flip_screen_set(videocontrol & 0x8000);

	layercontrol = m_cps_b_regs[0x20 / 2];

	/* Get video memory base registers */
	cps1_get_video_base(machine());

	/* Build palette */
	fcrash_build_palette(machine());

	fcrash_update_transmasks(machine());

	m_bg_tilemap[0]->set_scrollx(0, m_scroll1x - 62);
	m_bg_tilemap[0]->set_scrolly(0, m_scroll1y);

	if (videocontrol & 0x01)	/* linescroll enable */
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
		m_bg_tilemap[1]->set_scrollx(0, m_scroll2x - 60);
	}
	m_bg_tilemap[1]->set_scrolly(0, m_scroll2y);
	m_bg_tilemap[2]->set_scrollx(0, m_scroll3x - 64);
	m_bg_tilemap[2]->set_scrolly(0, m_scroll3y);


	/* turn all tilemaps on regardless of settings in get_video_base() */
	/* write a custom get_video_base for this bootleg hardware? */
	m_bg_tilemap[0]->enable(1);
	m_bg_tilemap[1]->enable(1);
	m_bg_tilemap[2]->enable(1);

	/* Blank screen */
	bitmap.fill(0xbff, cliprect);

	machine().priority_bitmap.fill(0, cliprect);
	l0 = (layercontrol >> 0x06) & 03;
	l1 = (layercontrol >> 0x08) & 03;
	l2 = (layercontrol >> 0x0a) & 03;
	l3 = (layercontrol >> 0x0c) & 03;

	fcrash_render_layer(machine(), bitmap, cliprect, l0, 0);

	if (l1 == 0)
		fcrash_render_high_layer(machine(), bitmap, cliprect, l0);

	fcrash_render_layer(machine(), bitmap, cliprect, l1, 0);

	if (l2 == 0)
		fcrash_render_high_layer(machine(), bitmap, cliprect, l1);

	fcrash_render_layer(machine(), bitmap, cliprect, l2, 0);

	if (l3 == 0)
		fcrash_render_high_layer(machine(), bitmap, cliprect, l2);

	fcrash_render_layer(machine(), bitmap, cliprect, l3, 0);

	return 0;
}

// doesn't have the scroll offsets like fcrash
UINT32 cps_state::screen_update_kodb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layercontrol, l0, l1, l2, l3;
	int videocontrol = m_cps_a_regs[0x22 / 2];

	flip_screen_set(videocontrol & 0x8000);

	layercontrol = m_cps_b_regs[0x20 / 2];

	/* Get video memory base registers */
	cps1_get_video_base(machine());

	/* Build palette */
	fcrash_build_palette(machine());

	fcrash_update_transmasks(machine());

	m_bg_tilemap[0]->set_scrollx(0, m_scroll1x);
	m_bg_tilemap[0]->set_scrolly(0, m_scroll1y);

	if (videocontrol & 0x01)	/* linescroll enable */
	{
		int scrly= -m_scroll2y;
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
		m_bg_tilemap[1]->set_scrollx(0, m_scroll2x);
	}

	m_bg_tilemap[1]->set_scrolly(0, m_scroll2y);
	m_bg_tilemap[2]->set_scrollx(0, m_scroll3x);
	m_bg_tilemap[2]->set_scrolly(0, m_scroll3y);


	/* turn all tilemaps on regardless of settings in get_video_base() */
	/* write a custom get_video_base for this bootleg hardware? */
	m_bg_tilemap[0]->enable(1);
	m_bg_tilemap[1]->enable(1);
	m_bg_tilemap[2]->enable(1);

	/* Blank screen */
	bitmap.fill(0xbff, cliprect);

	machine().priority_bitmap.fill(0, cliprect);
	l0 = (layercontrol >> 0x06) & 03;
	l1 = (layercontrol >> 0x08) & 03;
	l2 = (layercontrol >> 0x0a) & 03;
	l3 = (layercontrol >> 0x0c) & 03;

	fcrash_render_layer(machine(), bitmap, cliprect, l0, 0);

	if (l1 == 0)
		fcrash_render_high_layer(machine(), bitmap, cliprect, l0);

	fcrash_render_layer(machine(), bitmap, cliprect, l1, 0);

	if (l2 == 0)
		fcrash_render_high_layer(machine(), bitmap, cliprect, l1);

	fcrash_render_layer(machine(), bitmap, cliprect, l2, 0);

	if (l3 == 0)
		fcrash_render_high_layer(machine(), bitmap, cliprect, l2);

	fcrash_render_layer(machine(), bitmap, cliprect, l3, 0);

	return 0;
}


static ADDRESS_MAP_START( fcrash_map, AS_PROGRAM, 16, cps_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x800030, 0x800031) AM_WRITE(cps1_coinctrl_w)
	AM_RANGE(0x800100, 0x80013f) AM_RAM AM_SHARE("cps_a_regs")	/* CPS-A custom */
	AM_RANGE(0x800140, 0x80017f) AM_RAM AM_SHARE("cps_b_regs")	/* CPS-B custom */
	AM_RANGE(0x880000, 0x880001) AM_READ_PORT("IN1")				/* Player input ports */
	AM_RANGE(0x880006, 0x880007) AM_WRITE_LEGACY(fcrash_soundlatch_w)		/* Sound command */
	AM_RANGE(0x880008, 0x88000f) AM_READ(cps1_dsw_r)				/* System input ports / Dip Switches */
	AM_RANGE(0x890000, 0x890001) AM_WRITENOP	// palette related?
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_SHARE("gfxram")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, cps_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xd800, 0xd801) AM_DEVREADWRITE_LEGACY("ym1", ym2203_r, ym2203_w)
	AM_RANGE(0xdc00, 0xdc01) AM_DEVREADWRITE_LEGACY("ym2", ym2203_r, ym2203_w)
	AM_RANGE(0xe000, 0xe000) AM_WRITE_LEGACY(fcrash_snd_bankswitch_w)
	AM_RANGE(0xe400, 0xe400) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe800, 0xe800) AM_WRITE_LEGACY(fcrash_msm5205_0_data_w)
	AM_RANGE(0xec00, 0xec00) AM_WRITE_LEGACY(fcrash_msm5205_1_data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( kodb_map, AS_PROGRAM, 16, cps_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x800000, 0x800007) AM_READ_PORT("IN1")			/* Player input ports */
	/* forgottn, willow, cawing, nemo, varth read from 800010. Probably debug input leftover from development */
	AM_RANGE(0x800018, 0x80001f) AM_READ(cps1_dsw_r)			/* System input ports / Dip Switches */
	AM_RANGE(0x800020, 0x800021) AM_READNOP						/* ? Used by Rockman ? not mapped according to PAL */
	AM_RANGE(0x800030, 0x800037) AM_WRITE(cps1_coinctrl_w)
	/* Forgotten Worlds has dial controls on B-board mapped at 800040-80005f. See DRIVER_INIT */
	AM_RANGE(0x800100, 0x80013f) AM_WRITE(cps1_cps_a_w) AM_SHARE("cps_a_regs")	/* CPS-A custom */
	AM_RANGE(0x800140, 0x80017f) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w) AM_SHARE("cps_b_regs")	/* CPS-B custom */
//  AM_RANGE(0x800180, 0x800187) AM_WRITE_LEGACY(cps1_soundlatch_w)    /* Sound command */
//  AM_RANGE(0x800188, 0x80018f) AM_WRITE_LEGACY(cps1_soundlatch2_w)   /* Sound timer fade */
	AM_RANGE(0x8001c0, 0x8001ff) AM_READWRITE(cps1_cps_b_r, cps1_cps_b_w)	/* mirror (SF2 revision "E" US 910228) */
	AM_RANGE(0x900000, 0x92ffff) AM_RAM_WRITE(cps1_gfxram_w) AM_SHARE("gfxram")	/* SF2CE executes code from here */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


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

#define CPS1_COINAGE_3(diploc) \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" ) \
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) ) \
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION(diploc ":4,5,6") \
	PORT_DIPSETTING(    0x08, DEF_STR( 4C_1C ) ) \
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_1C ) ) \
	PORT_DIPSETTING(    0x18, DEF_STR( 2C_1C ) ) \
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit (1 to continue)" ) \
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) ) \
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) ) \
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) ) \
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )

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

#define CPS1_DIFFICULTY_2(diploc) \
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) ) PORT_DIPLOCATION(diploc ":1,2,3") \
	PORT_DIPSETTING(    0x04, "1 (Easiest)" ) \
	PORT_DIPSETTING(    0x05, "2" ) \
	PORT_DIPSETTING(    0x06, "3" ) \
	PORT_DIPSETTING(    0x07, "4 (Normal)" ) \
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
	PORT_DIPSETTING(    0x07, DEF_STR( Easiest ) )		// "01"
	PORT_DIPSETTING(    0x06, DEF_STR( Easier ) )		// "02"
	PORT_DIPSETTING(    0x05, DEF_STR( Easy ) )		// "03"
	PORT_DIPSETTING(    0x04, DEF_STR( Normal ) )		// "04"
	PORT_DIPSETTING(    0x03, DEF_STR( Medium ) )		// "05"
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )		// "06"
	PORT_DIPSETTING(    0x01, DEF_STR( Harder ) )		// "07"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )		// "08"
	PORT_DIPNAME( 0x18, 0x10, "Difficulty Level 2" )
	PORT_DIPSETTING(    0x18, DEF_STR( Easy ) )		// "01"
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )		// "02"
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )		// "03"
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )		// "04"
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
	PORT_DIPNAME( 0x08, 0x08, "Coin Slots" )						PORT_DIPLOCATION("SW(A):4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x10, 0x10, "Play Mode" )							PORT_DIPLOCATION("SW(A):5")
	PORT_DIPSETTING(    0x00, "2 Players" )
	PORT_DIPSETTING(    0x10, "3 Players" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(A):6" )
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )	PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Lives ) )					PORT_DIPLOCATION("SW(B):4,5,6")
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x38, "2" )
	PORT_DIPSETTING(    0x28, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "6" )
	PORT_DIPSETTING(    0x08, "7" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )				PORT_DIPLOCATION("SW(B):7,8")
	PORT_DIPSETTING(    0x80, "80k and every 400k" )
	PORT_DIPSETTING(    0xc0, "100k and every 450k" )
	PORT_DIPSETTING(    0x40, "160k and every 450k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )				PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )							PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )				PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )				PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )			PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")							PORT_DIPLOCATION("SW(C):8")
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


static const msm5205_interface msm5205_interface1 =
{
	m5205_int1,	/* interrupt function */
	MSM5205_S96_4B		/* 4KHz 4-bit */
};

static const msm5205_interface msm5205_interface2 =
{
	m5205_int2,	/* interrupt function */
	MSM5205_S96_4B		/* 4KHz 4-bit */
};


MACHINE_START_MEMBER(cps_state,fcrash)
{
	UINT8 *ROM = memregion("soundcpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x4000);

	m_maincpu = machine().device<cpu_device>("maincpu");
	m_audiocpu = machine().device<cpu_device>("soundcpu");
	m_msm_1 = machine().device<msm5205_device>("msm1");
	m_msm_2 = machine().device<msm5205_device>("msm2");

	save_item(NAME(m_sample_buffer1));
	save_item(NAME(m_sample_buffer2));
	save_item(NAME(m_sample_select1));
	save_item(NAME(m_sample_select2));
}

MACHINE_START_MEMBER(cps_state,kodb)
{

	m_maincpu = machine().device<cpu_device>("maincpu");
	m_audiocpu = machine().device<cpu_device>("soundcpu");
}

MACHINE_RESET_MEMBER(cps_state,fcrash)
{

	m_sample_buffer1 = 0;
	m_sample_buffer2 = 0;
	m_sample_select1 = 0;
	m_sample_select2 = 0;
}

static MACHINE_CONFIG_START( fcrash, cps_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(fcrash_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps_state,  cps1_interrupt)

	MCFG_CPU_ADD("soundcpu", Z80, 24000000/6) /* ? */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	MCFG_MACHINE_START_OVERRIDE(cps_state,fcrash)
	MCFG_MACHINE_RESET_OVERRIDE(cps_state,fcrash)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(cps_state, screen_update_fcrash)
	MCFG_SCREEN_VBLANK_DRIVER(cps_state, screen_eof_cps1)

	MCFG_GFXDECODE(cps1)
	MCFG_PALETTE_LENGTH(4096)

	MCFG_VIDEO_START_OVERRIDE(cps_state,cps1)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 24000000/6)	/* ? */
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 1.0)

	MCFG_SOUND_ADD("ym2", YM2203, 24000000/6)	/* ? */
	MCFG_SOUND_ROUTE(0, "mono", 0.10)
	MCFG_SOUND_ROUTE(1, "mono", 0.10)
	MCFG_SOUND_ROUTE(2, "mono", 0.10)
	MCFG_SOUND_ROUTE(3, "mono", 1.0)

	MCFG_SOUND_ADD("msm1", MSM5205, 24000000/64)	/* ? */
	MCFG_SOUND_CONFIG(msm5205_interface1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("msm2", MSM5205, 24000000/64)	/* ? */
	MCFG_SOUND_CONFIG(msm5205_interface2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( kodb, cps_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(kodb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps_state,  cps1_interrupt)

//  MCFG_CPU_ADD("soundcpu", Z80, 3579545)
//  MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_MACHINE_START_OVERRIDE(cps_state,kodb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(cps_state, screen_update_kodb)
	MCFG_SCREEN_VBLANK_DRIVER(cps_state, screen_eof_cps1)

	MCFG_GFXDECODE(cps1)
	MCFG_PALETTE_LENGTH(0xc00)

	MCFG_VIDEO_START_OVERRIDE(cps_state,cps1)

	/* sound hardware */
//  MCFG_SPEAKER_STANDARD_MONO("mono")

//  MCFG_SOUND_ADD("2151", YM2151, 3579545)
//  MCFG_SOUND_CONFIG(ym2151_config)
//  MCFG_SOUND_ROUTE(0, "mono", 0.35)
//  MCFG_SOUND_ROUTE(1, "mono", 0.35)

//  MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH) // pin 7 can be changed by the game code, see f006 on z80
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END



ROM_START( fcrash )
	ROM_REGION( 0x200000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "9.bin",  0x00000, 0x20000, CRC(c6854c91) SHA1(29f01cc65be5eaa3f86e99eebdd284104623abb0) )
	ROM_LOAD16_BYTE( "5.bin",  0x00001, 0x20000, CRC(77f7c2b3) SHA1(feea48d9555824a2e5bf5e99ce159edc015f0792) )
	ROM_LOAD16_BYTE( "8.bin",  0x40000, 0x20000, CRC(1895b3df) SHA1(415a26050c50ed79a7ee5ddd1b8d61593b1ce876) )
	ROM_LOAD16_BYTE( "4.bin",  0x40001, 0x20000, CRC(bbd411ee) SHA1(85d50ca72ec46d627f9c88ff0809aa30e164821a) )
	ROM_LOAD16_BYTE( "7.bin",  0x80000, 0x20000, CRC(5b23ebf2) SHA1(8c28c21a72a28ad249170026891c6bb865943f84) )
	ROM_LOAD16_BYTE( "3.bin",  0x80001, 0x20000, CRC(aba2aebe) SHA1(294109b5929ed63859a55bef16643e3ade7da16f) )
	ROM_LOAD16_BYTE( "6.bin",  0xc0000, 0x20000, CRC(d4bf37f6) SHA1(f47e1cc9aa3b3019ee57f59715e3a611acf9fe3e) )
	ROM_LOAD16_BYTE( "2.bin",  0xc0001, 0x20000, CRC(07ac8f43) SHA1(7a41b003c76adaabd3f94929cc163461b70e0ed9) )

	ROM_REGION( 0x30000, "soundcpu", 0 ) /* Audio CPU + Sample Data */
	ROM_LOAD( "1.bin",   0x00000, 0x20000, CRC(5b276c14) SHA1(73e53c077d4e3c1b919eee28b29e34176ee204f8) )
	ROM_RELOAD(          0x10000, 0x20000 )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "18.bin",     0x000000, 0x20000, CRC(f1eee6d9) SHA1(bee95efbff49c582cff1cc6d9bb5ef4ea5c4a074) , ROM_SKIP(3) )
	ROMX_LOAD( "20.bin",     0x000001, 0x20000, CRC(675f4537) SHA1(acc68822da3aafbb62f76cbffa5f3389fcc91447) , ROM_SKIP(3) )
	ROMX_LOAD( "22.bin",     0x000002, 0x20000, CRC(db8a32ac) SHA1(b95f73dff291acee239e22e5fd7efe15d0de23be) , ROM_SKIP(3) )
	ROMX_LOAD( "24.bin",     0x000003, 0x20000, CRC(f4113e57) SHA1(ff1f443c13494a169b9be24abc361d27a6d01c09) , ROM_SKIP(3) )
	ROMX_LOAD( "10.bin",     0x080000, 0x20000, CRC(d478853e) SHA1(91fcf8eb022ccea66d291bec84ace557181cf861) , ROM_SKIP(3) )
	ROMX_LOAD( "12.bin",     0x080001, 0x20000, CRC(25055642) SHA1(578cf6a436489cc1f2d1acdb0cba6c1cbee2e21f) , ROM_SKIP(3) )
	ROMX_LOAD( "14.bin",     0x080002, 0x20000, CRC(b77d0328) SHA1(42eb1ebfda301f2b09f3add5932e8331f4790706) , ROM_SKIP(3) )
	ROMX_LOAD( "16.bin",     0x080003, 0x20000, CRC(ea111a79) SHA1(1b86aa984d2d6c527e96b61274a82263f34d0d89) , ROM_SKIP(3) )
	ROMX_LOAD( "19.bin",     0x100000, 0x20000, CRC(b3aa1f48) SHA1(411f3855739992f5967e915f2a5255afcedeac2e) , ROM_SKIP(3) )
	ROMX_LOAD( "21.bin",     0x100001, 0x20000, CRC(04d175c9) SHA1(33e6e3fefae4e3977c8c954fbd7feff36e92d723) , ROM_SKIP(3) )
	ROMX_LOAD( "23.bin",     0x100002, 0x20000, CRC(e592ba4f) SHA1(62559481e0da3954a90da0ab0fb51f87f1b3dd9d) , ROM_SKIP(3) )
	ROMX_LOAD( "25.bin",     0x100003, 0x20000, CRC(b89a740f) SHA1(516d73c772e0a904dfb0bd84874919d78bbbd200) , ROM_SKIP(3) )
	ROMX_LOAD( "11.bin",     0x180000, 0x20000, CRC(d4457a60) SHA1(9e956efafa81a81aca92837df03968f5670ffc15) , ROM_SKIP(3) )
	ROMX_LOAD( "13.bin",     0x180001, 0x20000, CRC(3b26a37d) SHA1(58d8d0cdef81c938fb1a5595f2d02b228865893b) , ROM_SKIP(3) )
	ROMX_LOAD( "15.bin",     0x180002, 0x20000, CRC(6d837e09) SHA1(b4a133ab96c35b689ee692bfcc04981791099b6f) , ROM_SKIP(3) )
	ROMX_LOAD( "17.bin",     0x180003, 0x20000, CRC(c59a4d6c) SHA1(59e49c7d24dd333007de4bb621050011a5392bcc) , ROM_SKIP(3) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )	/* stars */
ROM_END

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
7x maskrom (ai,bi,ci,di,ap,cp,dp)(gfx)
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
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.ic172",    0x00000, 0x080000, CRC(036dd74c) SHA1(489344e56863429e86b4c362b82d89819c1d6afb) )
	ROM_LOAD16_BYTE( "4.ic171",    0x00001, 0x080000, CRC(3e4b7295) SHA1(3245640bae7d141238051dfe5c7683d05c6d3848) )

	ROM_REGION( 0x18000, "soundcpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "1.ic28",        0x00000, 0x08000, CRC(01cae60c) SHA1(b2cdd883fd859f0b701230831aca1f1a74ad6087) )
	ROM_CONTINUE(              0x10000, 0x08000 )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROMX_LOAD( "cp.ic90",   0x000000, 0x80000, CRC(e3b8589e) SHA1(775f97e43cb995b93da40063a1f1e4d73b34437c), ROM_SKIP(7) )
	ROMX_LOAD( "dp.ic89",   0x000001, 0x80000, CRC(3eec9580) SHA1(3d8d0cfbeae077544e514a5eb96cc83f716e494f), ROM_SKIP(7) )
	ROMX_LOAD( "ap.ic88",   0x000002, 0x80000, CRC(fdf5f163) SHA1(271ee96886c958accaca9a82484ab80fe32bd38e), ROM_SKIP(7) )
	ROMX_LOAD( "bp.ic87",   0x000003, 0x80000, CRC(4e1c52b7) SHA1(74570e7d577c999c62203c97b3d449e3b61a678a), ROM_SKIP(7) )
	ROMX_LOAD( "ci.ic91",   0x000004, 0x80000, CRC(22228bc5) SHA1(d48a09ee284d9e4b986f5c3c1c865930f76986e2), ROM_SKIP(7) )
	ROMX_LOAD( "di.ic92",   0x000005, 0x80000, CRC(ab031763) SHA1(5bcd89b1debf029b779aa1bb73b3a572d27154ec), ROM_SKIP(7) )
	ROMX_LOAD( "ai.ic93",   0x000006, 0x80000, CRC(cffbf4be) SHA1(f805bafc855d4a656c055a76eaeb26e36835541e), ROM_SKIP(7) )
	ROMX_LOAD( "bi.ic94",   0x000007, 0x80000, CRC(4a1b43fe) SHA1(7957f45b2862825c9509043c63c7da7108bd251b), ROM_SKIP(7) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_COPY( "gfx", 0x000000, 0x000000, 0x8000 )	/* stars */

	ROM_REGION( 0x40000, "oki", 0 )	/* Samples */
	ROM_LOAD( "2.ic19",      0x00000, 0x40000, CRC(a2db1575) SHA1(1a4a29e4b045af50700adf1665697feab12cc234) )
ROM_END

ROM_START( cawingbl )
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "caw2.bin",    0x00000, 0x80000, CRC(8125d3f0) SHA1(a0e48c326c6164ca189c9372f5c38a7c103772c1) )
	ROM_LOAD16_BYTE( "caw1.bin",    0x00001, 0x80000, CRC(b19b10ce) SHA1(3c71f1dc830d1e8b8ba26d8a71e12f477659480c) )

	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "caw4.bin", 0x000000, 0x80000, CRC(4937fc41) SHA1(dac179715be483a521df8e515afc1fb7a2cd8f13) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "caw5.bin", 0x000002, 0x80000, CRC(30dd78db) SHA1(e0295001d6f5fb4a9276c432f971e88f73c5e39a) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "caw6.bin", 0x000004, 0x80000, CRC(61192f7c) SHA1(86643c62653a62a5c7541d50cfdecae9b607440d) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "caw7.bin", 0x000006, 0x80000, CRC(a045c689) SHA1(8946c55635121282ea03586a278e50de20d92633) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x30000, "soundcpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "caw3.bin",  0x00000, 0x20000, CRC(ffe16cdc) SHA1(8069ea69f0b89d61c35995c8040a4989d7be9c1f) )
	ROM_RELOAD(          0x10000, 0x20000 )
ROM_END

// 24mhz crystal (maincpu), 28.322 crystal (video), 3.579545 crystal (sound)
// sound cpu is (239 V 249521 VC5006 KABUKI DL-030P-110V) - recycled Kabuki Z80 from genuine Capcom HW?
// 3x8 dsws

static MACHINE_CONFIG_START( sgyxz, cps_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(kodb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps_state,  cps1_interrupt)

//  MCFG_CPU_ADD("soundcpu", Z80, 3579545)
//  MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_MACHINE_START_OVERRIDE(cps_state,kodb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(cps_state, screen_update_kodb)
	MCFG_SCREEN_VBLANK_DRIVER(cps_state, screen_eof_cps1)

	MCFG_GFXDECODE(cps1)
	MCFG_PALETTE_LENGTH(0xc00)

	MCFG_VIDEO_START_OVERRIDE(cps_state,cps1)
MACHINE_CONFIG_END



ROM_START( sgyxz )
	ROM_REGION( 0x400000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "sgyxz_prg1.bin", 0x000001, 0x80000, CRC(d8511929) SHA1(4de9263778f327693f4d1e21b48e43806f673487) )
	ROM_LOAD16_BYTE( "sgyxz_prg2.bin", 0x000000, 0x80000, CRC(95429c83) SHA1(e981624d018132e5625a66113b6ac4fc44e55cf7) )

	ROM_REGION( 0x400000, "gfx", 0 )
	ROM_LOAD16_BYTE("sgyxz_gfx1.bin", 0x000000, 0x200000, CRC(a60be9f6) SHA1(2298a4b6a2c83b76dc106a1efa19606b298d378a) ) // 'picture 1'
	ROM_LOAD16_BYTE("sgyxz_gfx2.bin", 0x000001, 0x200000, CRC(6ad9d048) SHA1(d47212d28d0a1ce349e4c59e5d0d99c541b3458e) ) // 'picture 2'

	ROM_REGION( 0x10000, "soundcpu", 0 ) /* Z80 code */
	ROM_LOAD( "sgyxz_snd2.bin", 0x00000, 0x10000,  CRC(210c376f) SHA1(0d937c86078d0a106f5636b7daf5fc0266c2c2ec) )

	ROM_REGION( 0x040000, "oki", 0 ) /* Samples */
	ROM_LOAD( "sgyxz_snd1.bin", 0x00000, 0x40000,  CRC(c15ac0f2) SHA1(8d9e5519d9820e4ac4f70555088c80e64d052c9d) )
ROM_END






GAME( 1990, fcrash,   ffight,  fcrash,     fcrash, cps_state,   cps1,     ROT0,   "bootleg (Playmark)", "Final Crash (bootleg of Final Fight)", GAME_SUPPORTS_SAVE )
GAME( 1991, kodb,     kod,     kodb,       kodb, cps_state,     cps1,     ROT0,   "bootleg (Playmark)", "The King of Dragons (bootleg)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )	// 910731  "ETC"
GAME( 1990, cawingbl, cawing,  fcrash,     fcrash, cps_state,   cps1,     ROT0,   "bootleg", "Carrier Air Wing (bootleg with 2xYM2203)", GAME_NOT_WORKING )
GAME( 199?, sgyxz,    wof,     sgyxz,     fcrash, cps_state,    cps1,     ROT0,   "bootleg (All-In Electronic)", "Warriors of Fate ('sgyxz' bootleg)", GAME_NOT_WORKING | GAME_NO_SOUND )
