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

kodb has various graphical issues, mainly with old info not being cleared away.
Also, when you are hit, you should flash, but you go invisible instead.

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "includes/cps1.h"
#include "sound/2203intf.h"
#include "sound/msm5205.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"


static WRITE16_HANDLER( fcrash_soundlatch_w )
{
	cps_state *state = space.machine().driver_data<cps_state>();

	if (ACCESSING_BITS_0_7)
	{
		state->soundlatch_byte_w(space, 0, data & 0xff);
		state->m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

WRITE16_MEMBER(cps_state::cawingbl_soundlatch_w)
{
	cps_state *state = space.machine().driver_data<cps_state>();
	
	if (ACCESSING_BITS_8_15)
	{
		state->soundlatch_byte_w(space, 0, data  >> 8);
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

static WRITE8_HANDLER( sf2mdt_snd_bankswitch_w )
{
	cps_state *state = space.machine().driver_data<cps_state>();

	state->m_msm_1->set_output_gain(0, (data & 0x20) ? 0.0 : 1.0);
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


WRITE16_MEMBER(cps_state::kodb_layer_w)
{
	cps_state *state = space.machine().driver_data<cps_state>();
	
	/* layer enable and mask 1&2 registers are written here - passing them to m_cps_b_regs for now for drawing routines */
	if (offset == 0x06)
		state->m_cps_b_regs[m_layer_enable_reg / 2] = data;
	else
	if (offset == 0x10)
		state->m_cps_b_regs[state->m_layer_mask_reg[1] / 2] = data;
	else
	if (offset == 0x11)
		state->m_cps_b_regs[state->m_layer_mask_reg[2] / 2] = data;
}

WRITE16_MEMBER(cps_state::sf2mdt_layer_w)
{
	cps_state *state = space.machine().driver_data<cps_state>();
	
	/* layer enable and scroll registers are written here - passing them to m_cps_b_regs and m_cps_a_regs for now for drawing routines */
	if (offset == 0x0086)
		state->m_cps_a_regs[0x14 / 2] = data + 0xffce; /* scroll 3x */
	else
	if (offset == 0x0087)
		state->m_cps_a_regs[0x16 / 2] = data; /* scroll 3y */
	else
	if (offset == 0x0088)
		state->m_cps_a_regs[0x10 / 2] = data + 0xffcd; /* scroll 2x */
	else
	if (offset == 0x0089)
		state->m_cps_a_regs[0x0c / 2] = data + 0xffca; /* scroll 1x */
	else
	if (offset == 0x008a) { 
		state->m_cps_a_regs[0x12 / 2] = data; /* scroll 2y */
		state->m_cps_a_regs[0x20 / 2] = data; /* row scroll start */
		state->m_cps_a_regs[0x08 / 2] = m_bootleg_work_ram[0x802e / 2]; /* pretty gross hack?, but the row scroll table address isn't written anywhere else */
	} else	
	if (offset == 0x008b)
		state->m_cps_a_regs[0x0e / 2] = data; /* scroll 1y */
	else
	if (offset == 0x00a6)
		state->m_cps_b_regs[m_layer_enable_reg / 2] = data;
}


/* not verified */
#define CPS1_ROWSCROLL_OFFS     (0x20/2)    /* base of row scroll offsets in other RAM */

static void fcrash_update_transmasks( running_machine &machine )
{
	cps_state *state = machine.driver_data<cps_state>();
	int i;

	for (i = 0; i < 4; i++)
	{
		int mask;

		/* Get transparency registers */
		if (state->m_layer_mask_reg[i])
			mask = state->m_cps_b_regs[state->m_layer_mask_reg[i] / 2] ^ 0xffff;
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
	int base = state->m_sprite_base / 2;
	int num_sprites = machine.gfx[2]->elements();
	int last_sprite_offset = 0x1ffc;
	UINT16 *sprite_ram = state->m_gfxram;

	// sprite base registers need hooking up properly.. on fcrash it is NOT cps1_cps_a_regs[0]
	//  on kodb, it might still be, unless that's just a leftover and it writes somewhere else too
//  if (state->m_cps_a_regs[0] & 0x00ff) base = 0x10c8/2;
//  printf("cps1_cps_a_regs %04x\n", state->m_cps_a_regs[0]);
	
	/* if we have separate sprite ram, use it */
	if (state->m_bootleg_sprite_ram) sprite_ram = state->m_bootleg_sprite_ram;
	
	/* get end of sprite list marker */
	for (pos = 0x1ffc; pos >= 0x0000; pos -= 4)
	{
		if (sprite_ram[base + pos - 1] == state->m_sprite_list_end_marker) last_sprite_offset = pos;
	}
	
	for (pos = last_sprite_offset; pos >= 0x0000; pos -= 4)
	{
		int tileno;
		int xpos;
		int ypos;
		int flipx, flipy;
		int colour;

		tileno = sprite_ram[base + pos];
		if (tileno >= num_sprites) continue; /* don't render anything outside our tiles */
		xpos   = sprite_ram[base + pos + 2] & 0x1ff;
		ypos   = sprite_ram[base + pos - 1] & 0x1ff;
		flipx  = sprite_ram[base + pos + 1] & 0x20;
		flipy  = sprite_ram[base + pos + 1] & 0x40;
		colour = sprite_ram[base + pos + 1] & 0x1f;
		ypos   = 256 - ypos;
		xpos  += state->m_sprite_x_offset;
		
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
	
	// all the bootlegs seem to write the palette offset as usual
	int palettebase = (state->m_cps_a_regs[0x0a / 2] << 8) & 0x1ffff;
	
	for (offset = 0; offset < 32 * 6 * 16; offset++)
	{
		int palette = state->m_gfxram[palettebase / 2 + offset];
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

	layercontrol = m_cps_b_regs[m_layer_enable_reg / 2];

	/* Get video memory base registers */
	cps1_get_video_base(machine());

	/* Build palette */
	fcrash_build_palette(machine());

	fcrash_update_transmasks(machine());

	m_bg_tilemap[0]->set_scrollx(0, m_scroll1x - m_layer_scroll1x_offset);
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
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
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

static ADDRESS_MAP_START( kodb_sound_map, AS_PROGRAM, 8, cps_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xe000, 0xe001) AM_DEVREADWRITE("2151", ym2151_device, read, write)
	AM_RANGE(0xe400, 0xe400) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xe800, 0xe800) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sf2mdt_z80map, AS_PROGRAM, 8, cps_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xd800, 0xd801) AM_DEVREADWRITE("2151", ym2151_device, read, write)
	AM_RANGE(0xdc00, 0xdc00) AM_READ(soundlatch_byte_r)
	AM_RANGE(0xe000, 0xe000) AM_WRITE_LEGACY(sf2mdt_snd_bankswitch_w)
	AM_RANGE(0xe400, 0xe400) AM_WRITE_LEGACY(fcrash_msm5205_0_data_w)
	AM_RANGE(0xe800, 0xe800) AM_WRITE_LEGACY(fcrash_msm5205_1_data_w)
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
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )							// Overrides all other coinage settings
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )							// according to manual
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )						// This switch is not documented

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x04, "Difficulty Level (Enemy's Strength)" )	PORT_DIPLOCATION("SW(B):1,2,3")
	PORT_DIPSETTING(    0x07, "1 (Easiest)" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4 (Normal)" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "8 (Hardest)" )
	PORT_DIPNAME( 0x18, 0x18, "Difficulty Level (Player's Strength)" )	PORT_DIPLOCATION("SW(B):4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )						// This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )						// This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )						// This switch is not documented

	PORT_START("DSWC")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW(C):1" )						// This switch is not documented
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW(C):2" )						// This switch is not documented
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )					PORT_DIPLOCATION("SW(C):3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Freeze" )								PORT_DIPLOCATION("SW(C):4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )					PORT_DIPLOCATION("SW(C):5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )					PORT_DIPLOCATION("SW(C):6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Allow_Continue ) )				PORT_DIPLOCATION("SW(C):7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "Game Mode")								PORT_DIPLOCATION("SW(C):8")
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

static INPUT_PORTS_START( sf2mdt )
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
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Jab Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Strong Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Fierce Punch") PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2 Jab Punch") PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2 Strong Punch") PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2 Fierce Punch") PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )	

	PORT_START("IN2")      /* Extra buttons */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Short Kick") PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Forward Kick") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1 Roundhouse Kick") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2 Short Kick") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2 Forward Kick") PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2 Roundhouse Kick") PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSWA")
	CPS1_COINAGE_1
	PORT_DIPNAME( 0x40, 0x40, "2 Coins to Start, 1 to Continue" )	PORT_DIPLOCATION("SW(A):7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(A):8" )

	PORT_START("DSWB")
	CPS1_DIFFICULTY_1( "SW(B)" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW(B):4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW(B):5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW(B):6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW(B):7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW(B):8" )

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

MACHINE_START_MEMBER(cps_state,kodb)
{
	m_maincpu = machine().device<cpu_device>("maincpu");
	m_audiocpu = machine().device<cpu_device>("soundcpu");
	
	m_layer_enable_reg = 0x20;
	m_layer_mask_reg[0] = 0x2e;
	m_layer_mask_reg[1] = 0x2c;
	m_layer_mask_reg[2] = 0x2a;
	m_layer_mask_reg[3] = 0x28;
	m_layer_scroll1x_offset = 0;
	m_layer_scroll2x_offset = 0;
	m_layer_scroll3x_offset = 0;
	m_sprite_base = 0x50c8;
	m_sprite_list_end_marker = 0xffff;
	m_sprite_x_offset = 0;
}

MACHINE_START_MEMBER(cps_state, cawingbl)
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

MACHINE_START_MEMBER(cps_state, sf2mdt)
{
	UINT8 *ROM = memregion("soundcpu")->base();

	membank("bank1")->configure_entries(0, 8, &ROM[0x10000], 0x4000);

	m_maincpu = machine().device<cpu_device>("maincpu");
	m_audiocpu = machine().device<cpu_device>("soundcpu");
	m_msm_1 = machine().device<msm5205_device>("msm1");
	m_msm_2 = machine().device<msm5205_device>("msm2");
	
	m_layer_enable_reg = 0x26;
	m_layer_mask_reg[0] = 0x28;
	m_layer_mask_reg[1] = 0x2a;
	m_layer_mask_reg[2] = 0x2c;
	m_layer_mask_reg[3] = 0x2e;
	m_layer_scroll1x_offset = 0;
	m_layer_scroll2x_offset = 0;
	m_layer_scroll3x_offset = 0;
	m_sprite_base = 0x1000;
	m_sprite_list_end_marker = 0x8000;
	m_sprite_x_offset = 3;

	save_item(NAME(m_sample_buffer1));
	save_item(NAME(m_sample_buffer2));
	save_item(NAME(m_sample_select1));
	save_item(NAME(m_sample_select2));
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

static MACHINE_CONFIG_DERIVED( cawingbl, fcrash )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps_state,  irq6_line_hold) /* needed to write to scroll values */
	
	MCFG_MACHINE_START_OVERRIDE(cps_state, cawingbl)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( kodb, cps_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(fcrash_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps_state,  cps1_interrupt)

	MCFG_CPU_ADD("soundcpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(kodb_sound_map)

	MCFG_MACHINE_START_OVERRIDE(cps_state,kodb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*8, (64-8)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE_DRIVER(cps_state, screen_update_fcrash)
	MCFG_SCREEN_VBLANK_DRIVER(cps_state, screen_eof_cps1)

	MCFG_GFXDECODE(cps1)
	MCFG_PALETTE_LENGTH(0xc00)

	MCFG_VIDEO_START_OVERRIDE(cps_state,cps1)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("2151", XTAL_3_579545MHz)  /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("soundcpu", 0))
	MCFG_SOUND_ROUTE(0, "mono", 0.35)
	MCFG_SOUND_ROUTE(1, "mono", 0.35)

	/* CPS PPU is fed by a 16mhz clock,pin 117 outputs a 4mhz clock which is divided by 4 using 2 74ls74 */
	MCFG_OKIM6295_ADD("oki", XTAL_16MHz/4/4, OKIM6295_PIN7_HIGH) // pin 7 can be changed by the game code, see f006 on z80
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sf2mdt, cps_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(fcrash_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cps_state,  irq4_line_hold) /* triggers the sprite ram and scroll writes */

	MCFG_CPU_ADD("soundcpu", Z80, 3579545)
	MCFG_CPU_PROGRAM_MAP(sf2mdt_z80map)

	MCFG_MACHINE_START_OVERRIDE(cps_state, sf2mdt)
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

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("2151", 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.35)
	MCFG_SOUND_ROUTE(1, "mono", 0.35)

	/* has 2x MSM5205 instead of OKI6295 */
	MCFG_SOUND_ADD("msm1", MSM5205, 24000000/64)	/* ? */
	MCFG_SOUND_CONFIG(msm5205_interface1)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("msm2", MSM5205, 24000000/64)	/* ? */
	MCFG_SOUND_CONFIG(msm5205_interface2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


ROM_START( fcrash )
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
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
	ROMX_LOAD( "caw7.bin", 0x000000, 0x80000, CRC(a045c689) SHA1(8946c55635121282ea03586a278e50de20d92633) , ROM_SKIP(3) )
	ROMX_LOAD( "caw6.bin", 0x000001, 0x80000, CRC(61192f7c) SHA1(86643c62653a62a5c7541d50cfdecae9b607440d) , ROM_SKIP(3) )
	ROMX_LOAD( "caw5.bin", 0x000002, 0x80000, CRC(30dd78db) SHA1(e0295001d6f5fb4a9276c432f971e88f73c5e39a) , ROM_SKIP(3) )
	ROMX_LOAD( "caw4.bin", 0x000003, 0x80000, CRC(4937fc41) SHA1(dac179715be483a521df8e515afc1fb7a2cd8f13) , ROM_SKIP(3) )
	
	ROM_REGION( 0x30000, "soundcpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "caw3.bin",  0x00000, 0x20000, CRC(ffe16cdc) SHA1(8069ea69f0b89d61c35995c8040a4989d7be9c1f) )
	ROM_RELOAD(            0x10000, 0x20000 )
ROM_END

ROM_START( cawingb2 )
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "8.8",    0x00000, 0x20000, CRC(f655708c) SHA1(9962a1c96ea08bc71b25d4f58e5d1fb1beebf0dc) )
	ROM_LOAD16_BYTE( "4.4",    0x00001, 0x20000, CRC(a02fb5aa) SHA1(c9c064a83899c48f681ac803cfc5886503b9d992) )
	ROM_LOAD16_BYTE( "7.7",    0x40000, 0x20000, CRC(8c6c7430) SHA1(3ed5713caf774b050b41a6adea026e1307b570df) )
	ROM_LOAD16_BYTE( "3.3",    0x40001, 0x20000, CRC(f585bf2c) SHA1(3a3169791f8deace8d9bee1adb08f19fbcd309c6) )
	ROM_LOAD16_BYTE( "6.6",    0x80000, 0x20000, CRC(5fda906e) SHA1(7b3ef17d494a2f92e58ab7e34a3beaad8c149fca) )
	ROM_LOAD16_BYTE( "2.2",    0x80001, 0x20000, CRC(736c1835) SHA1(a91f479fab30603a111304adc0478d430faa80fc) )
	ROM_LOAD16_BYTE( "5.5",    0xc0000, 0x20000, CRC(76458083) SHA1(cbb4ef5f7615c834b2ee1ad3c86e7262f2f62c01) )
	ROM_LOAD16_BYTE( "1.1",    0xc0001, 0x20000, CRC(d3523f34) SHA1(005ea378c2b78782f85ecc591946c027ca2ca023) )
	
	ROM_REGION( 0x200000, "gfx", 0 )
	ROMX_LOAD( "17.17",     0x000000, 0x20000, CRC(0b538062) SHA1(ac6e5dc82efdca311adfe6e6cdda160ad4a0d04d) , ROM_SKIP(3) )
	ROMX_LOAD( "19.19",     0x000001, 0x20000, CRC(3ad62311) SHA1(1c132696b55191d16af30ebd36d2320d979eab36) , ROM_SKIP(3) )
	ROMX_LOAD( "21.21",     0x000002, 0x20000, CRC(1b872a98) SHA1(7a3f72c6d384dfa8e224f93604997a7b6e5c8926) , ROM_SKIP(3) )
	ROMX_LOAD( "23.23",     0x000003, 0x20000, CRC(ad49eecd) SHA1(39909996765391ed734a02c74f683e1bd9ce1561) , ROM_SKIP(3) )
	ROMX_LOAD( "9.9",       0x080000, 0x20000, CRC(8cd4df5b) SHA1(771b6d6a6baa95a669335fe64e2219fe7226e140) , ROM_SKIP(3) )
	ROMX_LOAD( "11.11",     0x080001, 0x20000, CRC(bf14418a) SHA1(7a0e1c65b8825a252338d6c1db59a88966ec6cfb) , ROM_SKIP(3) )
	ROMX_LOAD( "13.13",     0x080002, 0x20000, CRC(cef1aab8) SHA1(677a889b939ff00e95737a4a53053744bb6744c0) , ROM_SKIP(3) )
	ROMX_LOAD( "15.15",     0x080003, 0x20000, CRC(397725dc) SHA1(9450362bbf2f91b4225a088d6e283d7b16407b74) , ROM_SKIP(3) )
	ROMX_LOAD( "18.18",     0x100000, 0x20000, CRC(9b14f7ed) SHA1(72b6e1174d4faab487261aa6739de842d2423e1a) , ROM_SKIP(3) )
	ROMX_LOAD( "20.20",     0x100001, 0x20000, CRC(59bcc1bb) SHA1(c725060e068294dea1d962c54a9018050fa70297) , ROM_SKIP(3) )
	ROMX_LOAD( "22.22",     0x100002, 0x20000, CRC(23dc647a) SHA1(2d8d4c4c7b2d0616430360d1639b07216dd731d6) , ROM_SKIP(3) )
	ROMX_LOAD( "24.24",     0x100003, 0x20000, CRC(eda9fa6b) SHA1(4a3510ce71b015a1ea568fd0bbe61c5c093a2fbf) , ROM_SKIP(3) )
	ROMX_LOAD( "10.10",     0x180000, 0x20000, CRC(17174249) SHA1(71c6424ab4629065dd6af8bb47b18f5b5d0fbe49) , ROM_SKIP(3) )
	ROMX_LOAD( "12.12",     0x180001, 0x20000, CRC(490440b2) SHA1(2597bf16340308f84b32cfa048c426db571b4a35) , ROM_SKIP(3) )
	ROMX_LOAD( "14.14",     0x180002, 0x20000, CRC(344a8270) SHA1(fdb588a7ba60783225e3b5c72446f79625de4f9c) , ROM_SKIP(3) )
	ROMX_LOAD( "16.16",     0x180003, 0x20000, CRC(b991ad91) SHA1(5c59131ddf068cb54d23f8836293360fbc967d58) , ROM_SKIP(3) )
	
	ROM_REGION( 0x30000, "soundcpu", 0 ) /* 64k for the audio CPU (+banks) */
	ROM_LOAD( "5.a",       0x00000, 0x20000, CRC(ffe16cdc) SHA1(8069ea69f0b89d61c35995c8040a4989d7be9c1f) )
	ROM_RELOAD(            0x10000, 0x20000 )
ROM_END

/*
CPU

1x MC68000P12 (main)
1x TPC1020AFN-084C (main)
1x Z0840006PSC-Z80CPU (sound)
1x YM2151 (sound)
1x YM3012 (sound)
2x M5205 (sound)
2x LM324N (sound)
1x TDA2003 (sound)
1x oscillator 24.000000MHz
1x oscillator 30.000MHz
ROMs

14x AM27C040 (1,3,6,7,8,9,10,11,12,13,14,15,16,17)
3x TMS27C010A (2,4,5)
3x PAL 16S20 (ic7,ic72, ic80) (read protected, not dumped)
3x GAL20V8A (ic120, ic121, ic169) (read protected, not dumped)

Note

1x JAMMA edge connector
1x trimmer (volume)
3x 8x2 switches dip
*/

ROM_START( sf2mdt )
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.ic172",   0x000000, 0x80000, CRC(5301b41f) SHA1(6855a57b21e8c5d74e5cb18f9ce6af650d7fb422) )
	ROM_LOAD16_BYTE( "1.ic171",   0x000001, 0x80000, CRC(c1c803f6) SHA1(9fe18ae2553a63d8e4dcc20bafd5a4634f8b93c4) )
	ROM_LOAD16_BYTE( "4.ic176",   0x100000, 0x20000, CRC(1073b7b6) SHA1(81ca1eab65ceac69520584bb23a684ccb9d92f89) )
	ROM_LOAD16_BYTE( "2.ic175",   0x100001, 0x20000, CRC(924c6ce2) SHA1(676a912652bd75da5087f0c7eae047b7681a993c) )

	ROM_REGION( 0x600000, "gfx", 0 ) /* rearranged in init */
	ROMX_LOAD( "7.ic90",    0x000000, 0x80000, CRC(896eaf48) SHA1(5a13ae8b554e05eed3d5749aaf5845d499bce45b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "10.ic88",   0x000002, 0x80000, CRC(ef3f5be8) SHA1(d4e1de7d7caf6977e48544d6701618ae70c717f9) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "13.ic89",   0x000004, 0x80000, CRC(305dd72a) SHA1(c373b517c23f3b019abb06e21f6b9ab6e1e47909) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "16.ic87",   0x000006, 0x80000, CRC(e57f6db9) SHA1(b37f95737804002ec0e237472eaacf0bc1e868e8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "6.ic91",    0x200000, 0x80000, CRC(054cd5c4) SHA1(07f275e118c141a84ca15a2e9edc81694af37cf2) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "9.ic93",    0x200002, 0x80000, CRC(818ca33d) SHA1(dfb707e17c83216f8a62e905f8c7cd6d406b417b) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "12.ic92",   0x200004, 0x80000, CRC(87e069e8) SHA1(cddd3be84f8379134590bfbbb080518f28120e49) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "15.ic94",   0x200006, 0x80000, CRC(5dfb44d1) SHA1(08e44b8efc84f9cfc829aabf704155ddc700de76) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "8.ic86",    0x400000, 0x80000, CRC(34bbb3fa) SHA1(7794e89258f12b17d38c3d302dc15c502a8c8eb6) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "11.ic84",   0x400002, 0x80000, CRC(cea6d1d6) SHA1(9c953db42f0d877e43c0c239f69a00df39a18295) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "14.ic85",   0x400004, 0x80000, CRC(7d9f1a67) SHA1(6deb7fff867c42b13a32bb11eda798cfdb4cbaa8) , ROM_GROUPWORD | ROM_SKIP(6) )
	ROMX_LOAD( "17.ic83",   0x400006, 0x80000, CRC(91a9a05d) SHA1(5266ceddd2df925e79b4200843dec2f7aa9297b3) , ROM_GROUPWORD | ROM_SKIP(6) )

	ROM_REGION( 0x30000, "soundcpu", 0 ) /* Sound program + samples  */
	ROM_LOAD( "5.ic26",    0x00000, 0x20000, CRC(17d5ba8a) SHA1(6ff3b8860d7e1fdee3561846f645eb4d3a8965ec) )
	ROM_RELOAD(            0x10000, 0x20000 )
ROM_END

ROM_START( sf2mdta )
/* unconfirmed if working on real hardware, pf4 is a bad dump (bad pin) */
	ROM_REGION( 0x400000, "maincpu", 0 )      /* 68000 code */
	ROM_LOAD16_BYTE( "3.mdta", 0x000000, 0x80000, CRC(9f544ef4) SHA1(f784809e59a5fcabd6d15d3f1c36250a5528c9f8) )
	ROM_LOAD16_BYTE( "5.mdta", 0x000001, 0x80000, CRC(d76d6621) SHA1(aa9cea9ddace212a7b3c535b8f6e3fbc50da1f94) )
	ROM_LOAD16_BYTE( "2.mdta", 0x100000, 0x20000, CRC(74844192) SHA1(99cd546c78cce7f632007af454d8a55eddb6b19b) )
	ROM_LOAD16_BYTE( "4.mdta", 0x100001, 0x20000, CRC(bd98ff15) SHA1(ed902d949b0b5c5beaaea78a4b418ffa6db9e1df) )

	ROM_REGION( 0x600000, "gfx", 0 )
	ROMX_LOAD( "pf4 sh058.ic89", 0x000000, 0x80000, BAD_DUMP CRC(40fdf624) SHA1(cb928602744bf36e6851527f00d90da29de751e6), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                0x000002, 0x80000)
	ROMX_LOAD( "pf7 sh072.ic92", 0x000004, 0x80000, CRC(fb78022e) SHA1(b8974387056dd52db96b01cc4648edc814398c7e), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                0x000006, 0x80000)
	ROMX_LOAD( "pf5 sh036.ic90", 0x200000, 0x80000, CRC(0a6be48b) SHA1(b7e72c94d4e3eb4a6bba6608d9b9a093c8901ad9), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                0x200002, 0x80000)
	ROMX_LOAD( "pf8 sh074.ic93", 0x200004, 0x80000, CRC(6258c7cf) SHA1(4cd7519245c0aa816934a43e6743160f715d7dc2), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                0x200006, 0x80000)
	ROMX_LOAD( "pf6 sh070.ic88", 0x400000, 0x80000, CRC(9b5b09d7) SHA1(698a6aab41e495bd0c37a19aee16a84f04d15797), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                0x400002, 0x80000)
	ROMX_LOAD( "pf9 sh001.ic91", 0x400004, 0x80000, CRC(9f25090e) SHA1(12ff0431ef6550db446985c8914ac7d78eec6b6d), ROM_GROUPWORD | ROM_SKIP(6) )
	ROM_CONTINUE(                0x400006, 0x80000)

	ROM_REGION( 0x30000, "soundcpu", 0 ) /* Sound program + samples  */
	ROM_LOAD( "1.ic28",    0x00000, 0x20000, CRC(d5bee9cc) SHA1(e638cb5ce7a22c18b60296a7defe8b03418da56c) )
	ROM_RELOAD(            0x10000, 0x20000 )
ROM_END

// 24mhz crystal (maincpu), 28.322 crystal (video), 3.579545 crystal (sound)
// sound cpu is (239 V 249521 VC5006 KABUKI DL-030P-110V) - recycled Kabuki Z80 from genuine Capcom HW?
// 3x8 dsws

static MACHINE_CONFIG_START( sgyxz, cps_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(fcrash_map)
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
	MCFG_SCREEN_UPDATE_DRIVER(cps_state, screen_update_fcrash)
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


DRIVER_INIT_MEMBER(cps_state, kodb)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_port(0x800000, 0x800007, "IN1");
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(0x800018, 0x80001f, read16_delegate(FUNC(cps_state::cps1_dsw_r),this));
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_write_handler(0x800180, 0x800187, write16_delegate(FUNC(cps_state::cps1_soundlatch_w),this));
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_write_handler(0x980000, 0x98002f, write16_delegate(FUNC(cps_state::kodb_layer_w),this));

	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state, cawingbl)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_port(0x882000, 0x882001, "IN1");
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_write_handler(0x882006, 0x882007, write16_delegate(FUNC(cps_state::cawingbl_soundlatch_w),this));
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(0x882008, 0x88200f, read16_delegate(FUNC(cps_state::cps1_dsw_r),this));

	DRIVER_INIT_CALL(cps1);
}

DRIVER_INIT_MEMBER(cps_state, sf2mdt)
{
	int i;
	UINT32 gfx_size = machine().root_device().memregion( "gfx" )->bytes();
	UINT8 *rom = machine().root_device().memregion( "gfx" )->base();
	UINT8 tmp;

	for( i = 0; i < gfx_size; i += 8 )
	{
		tmp = rom[i + 1];
		rom[i + 1] = rom[i + 4];
		rom[i + 4] = tmp;
		tmp = rom[i + 3];
		rom[i + 3] = rom[i + 6];
		rom[i + 6] = tmp;
	}
	
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_handler(0x70c018, 0x70c01f, read16_delegate(FUNC(cps_state::cps1_dsw_r),this));
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_write_handler(0x708000, 0x708fff, write16_delegate(FUNC(cps_state::sf2mdt_layer_w),this));
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_port(0x70c000, 0x70c001, "IN1");
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_read_port(0x70c008, 0x70c009, "IN2");
	
	/* bootleg sprite ram */
	m_bootleg_sprite_ram = (UINT16*)machine().device("maincpu")->memory().space(AS_PROGRAM).install_ram(0x700000, 0x703fff);
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_ram(0x704000, 0x707fff, m_bootleg_sprite_ram); /* both of these need to be mapped */
	machine().device("maincpu")->memory().space(AS_PROGRAM).install_write_handler(0x70c106, 0x70c107, write16_delegate(FUNC(cps_state::cawingbl_soundlatch_w),this));
	
	m_bootleg_work_ram = (UINT16*)machine().device("maincpu")->memory().space(AS_PROGRAM).install_ram(0xff0000, 0xffffff);
	
	machine().device("maincpu")->memory().space(AS_PROGRAM).unmap_write(0x800030, 0x800031); /* coin lockout doesn't work (unmap it) */

	DRIVER_INIT_CALL(cps1);
}


GAME( 1990, fcrash,    ffight,  fcrash,    fcrash,   cps_state, cps1,     ROT0,   "bootleg (Playmark)", "Final Crash (bootleg of Final Fight)", GAME_SUPPORTS_SAVE )
GAME( 1991, kodb,      kod,     kodb,      kodb,     cps_state, kodb,     ROT0,   "bootleg (Playmark)", "The King of Dragons (bootleg)", GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )	// 910731  "ETC"
GAME( 1990, cawingbl,  cawing,  cawingbl,  cawingbl, cps_state, cawingbl, ROT0,   "bootleg", "Carrier Air Wing (bootleg with 2xYM2203 + 2xMSM205 set 1)", GAME_SUPPORTS_SAVE )
GAME( 1990, cawingb2,  cawing,  cawingbl,  cawingbl, cps_state, cawingbl, ROT0,   "bootleg", "Carrier Air Wing (bootleg with 2xYM2203 + 2xMSM205 set 2)", GAME_SUPPORTS_SAVE )
GAME( 1992, sf2mdt,    sf2ce,   sf2mdt,    sf2mdt,   cps_state, sf2mdt,   ROT0,   "bootleg", "Street Fighter II': Magic Delta Turbo (bootleg, set 1)", GAME_NOT_WORKING | GAME_SUPPORTS_SAVE )	// 920313 - based on (heavily modified) World version
GAME( 1992, sf2mdta,   sf2ce,   sf2mdt,    sf2mdt,   cps_state, sf2mdt,   ROT0,   "bootleg", "Street Fighter II': Magic Delta Turbo (bootleg, set 2)", GAME_NOT_WORKING | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )	// 920313 - based on World version
GAME( 199?, sgyxz,     wof,     sgyxz,     fcrash,   cps_state, cps1,     ROT0,   "bootleg (All-In Electronic)", "Warriors of Fate ('sgyxz' bootleg)", GAME_NOT_WORKING | GAME_NO_SOUND )
