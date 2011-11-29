/***************************************************************************

  The Main Event, (c) 1988 Konami
  Devastators, (c) 1988 Konami

Emulation by Bryan McPhail, mish@tendril.co.uk

Notes:
- Schematics show a palette/work RAM bank selector, but this doesn't seem
  to be used?

- In Devastators, shadows don't work. Bit 7 of the sprite attribute is always 0,
  could there be a global enable flag in the 051960?
  This is particularly evident in level 2 where plane shadows cover other sprites.
  The priority/shadow encoder PROM is quite complex, however bits 5-7 of the sprite
  attribute don't seem to be used, at least not in the first two levels, so the
  PROM just maps to the fixed priority order currently implemented.

- In Devastators, sprite zooming for the planes in level 2 is particularly bad.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/hd6309/hd6309.h"
#include "video/konicdev.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "sound/k007232.h"
#include "sound/upd7759.h"
#include "includes/konamipt.h"
#include "includes/mainevt.h"

static INTERRUPT_GEN( mainevt_interrupt )
{
	mainevt_state *state = device->machine().driver_data<mainevt_state>();

	if (k052109_is_irq_enabled(state->m_k052109))
		irq0_line_hold(device);
}

static WRITE8_HANDLER( dv_nmienable_w )
{
	mainevt_state *state = space->machine().driver_data<mainevt_state>();
	state->m_nmi_enable = data;
}

static INTERRUPT_GEN( dv_interrupt )
{
	mainevt_state *state = device->machine().driver_data<mainevt_state>();

	if (state->m_nmi_enable)
		nmi_line_pulse(device);
}


static WRITE8_HANDLER( mainevt_bankswitch_w )
{
	mainevt_state *state = space->machine().driver_data<mainevt_state>();

	/* bit 0-1 ROM bank select */
	memory_set_bank(space->machine(), "bank1", data & 0x03);

	/* TODO: bit 5 = select work RAM or palette? */
	//palette_selected = data & 0x20;

	/* bit 6 = enable char ROM reading through the video RAM */
	k052109_set_rmrd_line(state->m_k052109, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);

	/* bit 7 = NINITSET (unknown) */

	/* other bits unused */
}

static WRITE8_HANDLER( mainevt_coin_w )
{
	coin_counter_w(space->machine(), 0, data & 0x10);
	coin_counter_w(space->machine(), 1, data & 0x20);
	set_led_status(space->machine(), 0, data & 0x01);
	set_led_status(space->machine(), 1, data & 0x02);
	set_led_status(space->machine(), 2, data & 0x04);
	set_led_status(space->machine(), 3, data & 0x08);
}

static WRITE8_HANDLER( mainevt_sh_irqtrigger_w )
{
	mainevt_state *state = space->machine().driver_data<mainevt_state>();
	device_set_input_line_and_vector(state->m_audiocpu, 0, HOLD_LINE, 0xff);
}

static READ8_DEVICE_HANDLER( mainevt_sh_busy_r )
{
	return upd7759_busy_r(device);
}

static WRITE8_HANDLER( mainevt_sh_irqcontrol_w )
{
	mainevt_state *state = space->machine().driver_data<mainevt_state>();

	upd7759_reset_w(state->m_upd, data & 2);
	upd7759_start_w(state->m_upd, data & 1);

	state->m_sound_irq_mask = data & 4;
}

static WRITE8_HANDLER( devstor_sh_irqcontrol_w )
{
	mainevt_state *state = space->machine().driver_data<mainevt_state>();

	state->m_sound_irq_mask = data & 4;
}

static WRITE8_HANDLER( mainevt_sh_bankswitch_w )
{
	mainevt_state *state = space->machine().driver_data<mainevt_state>();
	int bank_A, bank_B;

//logerror("CPU #1 PC: %04x bank switch = %02x\n",cpu_get_pc(&space->device()),data);

	/* bits 0-3 select the 007232 banks */
	bank_A = (data & 0x3);
	bank_B = ((data >> 2) & 0x3);
	k007232_set_bank(state->m_k007232, bank_A, bank_B);

	/* bits 4-5 select the UPD7759 bank */
	upd7759_set_bank_base(state->m_upd, ((data >> 4) & 0x03) * 0x20000);
}

static WRITE8_DEVICE_HANDLER( dv_sh_bankswitch_w )
{
	int bank_A, bank_B;

//logerror("CPU #1 PC: %04x bank switch = %02x\n",cpu_get_pc(&space->device()),data);

	/* bits 0-3 select the 007232 banks */
	bank_A = (data & 0x3);
	bank_B = ((data >> 2) & 0x3);
	k007232_set_bank(device, bank_A, bank_B);
}

static READ8_HANDLER( k052109_051960_r )
{
	mainevt_state *state = space->machine().driver_data<mainevt_state>();

	if (k052109_get_rmrd_line(state->m_k052109) == CLEAR_LINE)
	{
		if (offset >= 0x3800 && offset < 0x3808)
			return k051937_r(state->m_k051960, offset - 0x3800);
		else if (offset < 0x3c00)
			return k052109_r(state->m_k052109, offset);
		else
			return k051960_r(state->m_k051960, offset - 0x3c00);
	}
	else
		return k052109_r(state->m_k052109, offset);
}

static WRITE8_HANDLER( k052109_051960_w )
{
	mainevt_state *state = space->machine().driver_data<mainevt_state>();

	if (offset >= 0x3800 && offset < 0x3808)
		k051937_w(state->m_k051960, offset - 0x3800, data);
	else if (offset < 0x3c00)
		k052109_w(state->m_k052109, offset, data);
	else
		k051960_w(state->m_k051960, offset - 0x3c00, data);
}


static ADDRESS_MAP_START( mainevt_map, AS_PROGRAM, 8 )
	AM_RANGE(0x1f80, 0x1f80) AM_WRITE(mainevt_bankswitch_w)
	AM_RANGE(0x1f84, 0x1f84) AM_WRITE(soundlatch_w)				/* probably */
	AM_RANGE(0x1f88, 0x1f88) AM_WRITE(mainevt_sh_irqtrigger_w)	/* probably */
	AM_RANGE(0x1f8c, 0x1f8d) AM_WRITENOP	/* ??? */
	AM_RANGE(0x1f90, 0x1f90) AM_WRITE(mainevt_coin_w)	/* coin counters + lamps */

	AM_RANGE(0x1f94, 0x1f94) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1f95, 0x1f95) AM_READ_PORT("P1")
	AM_RANGE(0x1f96, 0x1f96) AM_READ_PORT("P2")
	AM_RANGE(0x1f97, 0x1f97) AM_READ_PORT("DSW1")
	AM_RANGE(0x1f98, 0x1f98) AM_READ_PORT("DSW3")
	AM_RANGE(0x1f99, 0x1f99) AM_READ_PORT("P3")
	AM_RANGE(0x1f9a, 0x1f9a) AM_READ_PORT("P4")
	AM_RANGE(0x1f9b, 0x1f9b) AM_READ_PORT("DSW2")

	AM_RANGE(0x0000, 0x3fff) AM_READWRITE(k052109_051960_r, k052109_051960_w)

	AM_RANGE(0x4000, 0x5dff) AM_RAM
	AM_RANGE(0x5e00, 0x5fff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( devstors_map, AS_PROGRAM, 8 )
	AM_RANGE(0x1f80, 0x1f80) AM_WRITE(mainevt_bankswitch_w)
	AM_RANGE(0x1f84, 0x1f84) AM_WRITE(soundlatch_w)				/* probably */
	AM_RANGE(0x1f88, 0x1f88) AM_WRITE(mainevt_sh_irqtrigger_w)	/* probably */
	AM_RANGE(0x1f90, 0x1f90) AM_WRITE(mainevt_coin_w)	/* coin counters + lamps */
	AM_RANGE(0x1fb2, 0x1fb2) AM_WRITE(dv_nmienable_w)

	AM_RANGE(0x1f94, 0x1f94) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x1f95, 0x1f95) AM_READ_PORT("P1")
	AM_RANGE(0x1f96, 0x1f96) AM_READ_PORT("P2")
	AM_RANGE(0x1f97, 0x1f97) AM_READ_PORT("DSW1")
	AM_RANGE(0x1f98, 0x1f98) AM_READ_PORT("DSW3")
	AM_RANGE(0x1f9b, 0x1f9b) AM_READ_PORT("DSW2")
	AM_RANGE(0x1fa0, 0x1fbf) AM_DEVREADWRITE("k051733", k051733_r, k051733_w)

	AM_RANGE(0x0000, 0x3fff) AM_READWRITE(k052109_051960_r, k052109_051960_w)

	AM_RANGE(0x4000, 0x5dff) AM_RAM
	AM_RANGE(0x5e00, 0x5fff) AM_RAM_WRITE(paletteram_xBBBBBGGGGGRRRRR_be_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( mainevt_sound_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x9000, 0x9000) AM_DEVWRITE("upd", upd7759_port_w)
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("k007232", k007232_r,k007232_w)
	AM_RANGE(0xd000, 0xd000) AM_DEVREAD("upd", mainevt_sh_busy_r)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(mainevt_sh_irqcontrol_w)
	AM_RANGE(0xf000, 0xf000) AM_WRITE(mainevt_sh_bankswitch_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( devstors_sound_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0xa000, 0xa000) AM_READ(soundlatch_r)
	AM_RANGE(0xb000, 0xb00d) AM_DEVREADWRITE("k007232", k007232_r,k007232_w)
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2151_r,ym2151_w)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(devstor_sh_irqcontrol_w)
	AM_RANGE(0xf000, 0xf000) AM_DEVWRITE("k007232", dv_sh_bankswitch_w)
ADDRESS_MAP_END


/*****************************************************************************/

static INPUT_PORTS_START( mainevt )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE4 )

	PORT_START("P1")
	KONAMI8_B21_UNK(1)

	PORT_START("P2")
	KONAMI8_B21_UNK(2)

	PORT_START("P3")
	KONAMI8_B21_UNK(3)

	PORT_START("P4")
	KONAMI8_B21_UNK(4)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )		PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5" )		/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )		/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )		/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )		/* Listed as "Unused" */

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW2:1" )		/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW2:2" )		/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )		/* Listed as "Unused" */
	PORT_DIPNAME( 0x18, 0x10, "Bonus Energy" )		PORT_DIPLOCATION("SW2:4,5")	// Typo on US manual "SW2:1,2"
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPSETTING(    0x08, "70" )
	PORT_DIPSETTING(    0x10, "80" )	// factory default
	PORT_DIPSETTING(    0x18, "90" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )	// factory default
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW3:2" )		/* Listed as "Unused" */
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )		/* Listed as "Unused" */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mainev2p )
	PORT_INCLUDE( mainevt )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no keys for P3 & P4 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")
	KONAMI_COINAGE_ALT
INPUT_PORTS_END

static INPUT_PORTS_START( devstors )
	PORT_INCLUDE( mainev2p )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no service2 */

	PORT_MODIFY("P1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no 3rd button */

	PORT_MODIFY("P2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* no 3rd button */

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW1")		/* like mainevt, but different 0x00 settings */
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "Invalid", SW1)
	/* "Invalid" = both coin slots disabled */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )		// factory default
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW2:3" )		/* Listed as "Unused" */
	PORT_DIPNAME( 0x18, 0x10, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "150 and every 200" )
	PORT_DIPSETTING(    0x10, "150 and every 250" )	// factory default
	PORT_DIPSETTING(    0x08, "150 Only" )
	PORT_DIPSETTING(    0x00, "200 Only" )
INPUT_PORTS_END

/* Same as 'devstors', but additional "Cocktail" Dip Switch (even if I don't see the use) */
static INPUT_PORTS_START( devstor2 )
	PORT_INCLUDE( devstors )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
INPUT_PORTS_END

/*****************************************************************************/

static void volume_callback(device_t *device, int v)
{
	k007232_set_volume(device, 0, (v >> 4) * 0x11, 0);
	k007232_set_volume(device, 1, 0, (v & 0x0f) * 0x11);
}

static const k007232_interface k007232_config =
{
	volume_callback	/* external port callback */
};

static const k052109_interface mainevt_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	mainevt_tile_callback
};

static const k051960_interface mainevt_k051960_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	mainevt_sprite_callback
};

static MACHINE_START( mainevt )
{
	mainevt_state *state = machine.driver_data<mainevt_state>();
	UINT8 *ROM = machine.region("maincpu")->base();

	memory_configure_bank(machine, "bank1", 0, 4, &ROM[0x10000], 0x2000);

	state->m_maincpu = machine.device("maincpu");
	state->m_audiocpu = machine.device("audiocpu");
	state->m_upd = machine.device("upd");
	state->m_k007232 = machine.device("k007232");
	state->m_k052109 = machine.device("k052109");
	state->m_k051960 = machine.device("k051960");

	state->save_item(NAME(state->m_nmi_enable));
}

static MACHINE_RESET( mainevt )
{
	mainevt_state *state = machine.driver_data<mainevt_state>();

	state->m_nmi_enable = 0;
}

static INTERRUPT_GEN( mainevt_sound_timer_irq )
{
	mainevt_state *state = device->machine().driver_data<mainevt_state>();

	if(state->m_sound_irq_mask)
		device_set_input_line(device, 0, HOLD_LINE);
}

static INTERRUPT_GEN( devstors_sound_timer_irq )
{
	mainevt_state *state = device->machine().driver_data<mainevt_state>();

	if(state->m_sound_irq_mask)
		device_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( mainevt, mainevt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, 3000000*4)	/* ?? */
	MCFG_CPU_PROGRAM_MAP(mainevt_map)
	MCFG_CPU_VBLANK_INT("screen", mainevt_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)	/* 3.579545 MHz */
	MCFG_CPU_PROGRAM_MAP(mainevt_sound_map)
	MCFG_CPU_PERIODIC_INT(mainevt_sound_timer_irq,8*60)	/* ??? */

	MCFG_MACHINE_START(mainevt)
	MCFG_MACHINE_RESET(mainevt)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE(mainevt)

	MCFG_PALETTE_LENGTH(256)

	MCFG_VIDEO_START(mainevt)

	MCFG_K052109_ADD("k052109", mainevt_k052109_intf)
	MCFG_K051960_ADD("k051960", mainevt_k051960_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("k007232", K007232, 3579545)
	MCFG_SOUND_CONFIG(k007232_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)

	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static const k052109_interface dv_k052109_intf =
{
	"gfx1", 0,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	dv_tile_callback
};

static const k051960_interface dv_k051960_intf =
{
	"gfx2", 1,
	NORMAL_PLANE_ORDER,
	KONAMI_ROM_DEINTERLEAVE_2,
	dv_sprite_callback
};

static MACHINE_CONFIG_START( devstors, mainevt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", HD6309, 3000000*4)	/* ?? */
	MCFG_CPU_PROGRAM_MAP(devstors_map)
	MCFG_CPU_VBLANK_INT("screen", dv_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, 3579545)	/* 3.579545 MHz */
	MCFG_CPU_PROGRAM_MAP(devstors_sound_map)
	MCFG_CPU_PERIODIC_INT(devstors_sound_timer_irq,4*60) /* ??? */

	MCFG_MACHINE_START(mainevt)
	MCFG_MACHINE_RESET(mainevt)

	/* video hardware */
	MCFG_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(13*8, (64-13)*8-1, 2*8, 30*8-1 )
	MCFG_SCREEN_UPDATE(dv)

	MCFG_PALETTE_LENGTH(256)

	MCFG_VIDEO_START(dv)

	MCFG_K052109_ADD("k052109", dv_k052109_intf)
	MCFG_K051960_ADD("k051960", dv_k051960_intf)
	MCFG_K051733_ADD("k051733")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2151, 3579545)
	MCFG_SOUND_ROUTE(0, "mono", 0.30)
	MCFG_SOUND_ROUTE(1, "mono", 0.30)

	MCFG_SOUND_ADD("k007232", K007232, 3579545)
	MCFG_SOUND_CONFIG(k007232_config)
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
MACHINE_CONFIG_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

/* 4 players - English title screen - No "Warning" message in the ROM */

ROM_START( mainevt )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "799c02.k11",   0x10000, 0x08000, CRC(e2e7dbd5) SHA1(80314cd42a9f47f7bb82a2160fb5ef2ddc6dff30) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "799c01.f7",    0x00000, 0x08000, CRC(447c4c5c) SHA1(86e42132793c59cc6feece143516f7ecd4ed14e8) )

	ROM_REGION( 0x20000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "799c06.f22",   0x00000, 0x08000, CRC(f839cb58) SHA1(b36202ca2b68b6249c3f972ad09501e28a0162f7) )
	ROM_LOAD16_BYTE( "799c07.h22",   0x00001, 0x08000, CRC(176df538) SHA1(379e1de81afb85b1559de170cd2ab9f4af2b137e) )
	ROM_LOAD16_BYTE( "799c08.j22",   0x10000, 0x08000, CRC(d01e0078) SHA1(7ac242eb24271ac2783ec4d9e97ae051f1f3363a) )
	ROM_LOAD16_BYTE( "799c09.k22",   0x10001, 0x08000, CRC(9baec75e) SHA1(a8f6102c8fd46f18678f336bc44be31458ca9256) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "799b04.h4",    0x00000, 0x80000, CRC(323e0c2b) SHA1(c108d656b6ceff13c910739e4ca760acbb640de3) )
	ROM_LOAD( "799b05.k4",    0x80000, 0x80000, CRC(571c5831) SHA1(2a18f0bcf6946ada6e0bde7edbd11afd4db1c170) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14",  0x0000, 0x0100, CRC(61f6c8d1) SHA1(c70f1f8e434aaaffb89e30e2230a08374ef324ad) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 512k for 007232 samples */
	ROM_LOAD( "799b03.d4",    0x00000, 0x80000, CRC(f1cfd342) SHA1(079afc5c631de7f5b652d0ce6fd44b3aacd14a1b) )

	ROM_REGION( 0x80000, "upd", 0 )	/* 512k for the UPD7759C samples */
	ROM_LOAD( "799b06.c22",   0x00000, 0x80000, CRC(2c8c47d7) SHA1(18a899767177ddfd870df9ed156d8bbc04b58a19) )
ROM_END


/* 4 players - English title screen - No "Warning" message in the ROM */

ROM_START( mainevto )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "799_02.k11",   0x10000, 0x08000, CRC(c143596b) SHA1(5da7efaf0f7c7a493cc242eae115f278bc9c134b) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "799c01.f7",    0x00000, 0x08000, CRC(447c4c5c) SHA1(86e42132793c59cc6feece143516f7ecd4ed14e8) )

	ROM_REGION( 0x20000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "799c06.f22",   0x00000, 0x08000, CRC(f839cb58) SHA1(b36202ca2b68b6249c3f972ad09501e28a0162f7) )
	ROM_LOAD16_BYTE( "799c07.h22",   0x00001, 0x08000, CRC(176df538) SHA1(379e1de81afb85b1559de170cd2ab9f4af2b137e) )
	ROM_LOAD16_BYTE( "799c08.j22",   0x10000, 0x08000, CRC(d01e0078) SHA1(7ac242eb24271ac2783ec4d9e97ae051f1f3363a) )
	ROM_LOAD16_BYTE( "799c09.k22",   0x10001, 0x08000, CRC(9baec75e) SHA1(a8f6102c8fd46f18678f336bc44be31458ca9256) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "799b04.h4",    0x00000, 0x80000, CRC(323e0c2b) SHA1(c108d656b6ceff13c910739e4ca760acbb640de3) )
	ROM_LOAD( "799b05.k4",    0x80000, 0x80000, CRC(571c5831) SHA1(2a18f0bcf6946ada6e0bde7edbd11afd4db1c170) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14",  0x0000, 0x0100, CRC(61f6c8d1) SHA1(c70f1f8e434aaaffb89e30e2230a08374ef324ad) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 512k for 007232 samples */
	ROM_LOAD( "799b03.d4",    0x00000, 0x80000, CRC(f1cfd342) SHA1(079afc5c631de7f5b652d0ce6fd44b3aacd14a1b) )

	ROM_REGION( 0x80000, "upd", 0 )	/* 512k for the UPD7759C samples */
	ROM_LOAD( "799b06.c22",   0x00000, 0x80000, CRC(2c8c47d7) SHA1(18a899767177ddfd870df9ed156d8bbc04b58a19) )
ROM_END


/* 2 players - English title screen - "Warning" message in the ROM (not displayed) */

ROM_START( mainevt2p )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "799x02.k11",   0x10000, 0x08000, CRC(42cfc650) SHA1(2d1918ebc0d93a2356ad995a6854dbde7c3b8daf) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "799c01.f7",    0x00000, 0x08000, CRC(447c4c5c) SHA1(86e42132793c59cc6feece143516f7ecd4ed14e8) )

	ROM_REGION( 0x20000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "799c06.f22",   0x00000, 0x08000, CRC(f839cb58) SHA1(b36202ca2b68b6249c3f972ad09501e28a0162f7) )
	ROM_LOAD16_BYTE( "799c07.h22",   0x00001, 0x08000, CRC(176df538) SHA1(379e1de81afb85b1559de170cd2ab9f4af2b137e) )
	ROM_LOAD16_BYTE( "799c08.j22",   0x10000, 0x08000, CRC(d01e0078) SHA1(7ac242eb24271ac2783ec4d9e97ae051f1f3363a) )
	ROM_LOAD16_BYTE( "799c09.k22",   0x10001, 0x08000, CRC(9baec75e) SHA1(a8f6102c8fd46f18678f336bc44be31458ca9256) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "799b04.h4",    0x00000, 0x80000, CRC(323e0c2b) SHA1(c108d656b6ceff13c910739e4ca760acbb640de3) )
	ROM_LOAD( "799b05.k4",    0x80000, 0x80000, CRC(571c5831) SHA1(2a18f0bcf6946ada6e0bde7edbd11afd4db1c170) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14",  0x0000, 0x0100, CRC(61f6c8d1) SHA1(c70f1f8e434aaaffb89e30e2230a08374ef324ad) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 512k for 007232 samples */
	ROM_LOAD( "799b03.d4",    0x00000, 0x80000, CRC(f1cfd342) SHA1(079afc5c631de7f5b652d0ce6fd44b3aacd14a1b) )

	ROM_REGION( 0x80000, "upd", 0 )	/* 512k for the UPD7759C samples */
	ROM_LOAD( "799b06.c22",   0x00000, 0x80000, CRC(2c8c47d7) SHA1(18a899767177ddfd870df9ed156d8bbc04b58a19) )
ROM_END


/* 2 players - Japan title screen - "Warning" message in the ROM (displayed) */

ROM_START( ringohja )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "799n02.k11",   0x10000, 0x08000, CRC(f9305dd0) SHA1(7135053be9d46ac9c09ab63eca1eb71825a71a13) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "799c01.f7",    0x00000, 0x08000, CRC(447c4c5c) SHA1(86e42132793c59cc6feece143516f7ecd4ed14e8) )

	ROM_REGION( 0x20000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "799c06.f22",   0x00000, 0x08000, CRC(f839cb58) SHA1(b36202ca2b68b6249c3f972ad09501e28a0162f7) )
	ROM_LOAD16_BYTE( "799c07.h22",   0x00001, 0x08000, CRC(176df538) SHA1(379e1de81afb85b1559de170cd2ab9f4af2b137e) )
	ROM_LOAD16_BYTE( "799c08.j22",   0x10000, 0x08000, CRC(d01e0078) SHA1(7ac242eb24271ac2783ec4d9e97ae051f1f3363a) )
	ROM_LOAD16_BYTE( "799c09.k22",   0x10001, 0x08000, CRC(9baec75e) SHA1(a8f6102c8fd46f18678f336bc44be31458ca9256) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "799b04.h4",    0x00000, 0x80000, CRC(323e0c2b) SHA1(c108d656b6ceff13c910739e4ca760acbb640de3) )
	ROM_LOAD( "799b05.k4",    0x80000, 0x80000, CRC(571c5831) SHA1(2a18f0bcf6946ada6e0bde7edbd11afd4db1c170) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14",  0x0000, 0x0100, CRC(61f6c8d1) SHA1(c70f1f8e434aaaffb89e30e2230a08374ef324ad) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 512k for 007232 samples */
	ROM_LOAD( "799b03.d4",    0x00000, 0x80000, CRC(f1cfd342) SHA1(079afc5c631de7f5b652d0ce6fd44b3aacd14a1b) )

	ROM_REGION( 0x80000, "upd", 0 )	/* 512k for the UPD7759C samples */
	ROM_LOAD( "799b06.c22",   0x00000, 0x80000, CRC(2c8c47d7) SHA1(18a899767177ddfd870df9ed156d8bbc04b58a19) )
ROM_END

ROM_START( devstors )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "890z02.k11",  0x10000, 0x08000, CRC(ebeb306f) SHA1(838fcfe95dfedd61f21f34301d48e337db765ab2) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD16_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD16_BYTE( "890f08.j22",  0x20000, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD16_BYTE( "890f09.k22",  0x20001, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD( "890f05.k4",  0x80000, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END

ROM_START( devstors2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "890x02.k11",  0x10000, 0x08000, CRC(e58ebb35) SHA1(4253b6a7128534cc0866bc910a271d91ac8b40fd) )
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD16_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD16_BYTE( "890f08.j22",  0x20000, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD16_BYTE( "890f09.k22",  0x20001, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD( "890f05.k4",  0x80000, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END

ROM_START( devstors3 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "890k02.k11",   0x10000, 0x08000, CRC(52f4ccdd) SHA1(074e526ed170a5f2083c8c0808734291a2ea7403) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD16_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD16_BYTE( "890f08.j22",  0x20000, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD16_BYTE( "890f09.k22",  0x20001, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD( "890f05.k4",  0x80000, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END

ROM_START( garuka )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "890w02.k11",   0x10000, 0x08000, CRC(b2f6f538) SHA1(95dad3258a2e4c5648d0fc22c06fa3e2da3b5ed1) )
	ROM_CONTINUE(             0x08000, 0x08000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "890k01.f7",  0x00000, 0x08000, CRC(d44b3eb0) SHA1(26109fc56668b65f1a5aa6d8ec2c08fd70ca7c51) )

	ROM_REGION( 0x40000, "gfx1", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD16_BYTE( "890f06.f22",  0x00000, 0x10000, CRC(26592155) SHA1(aa1f8662f091ca1eb495223e41a35edd861ae9e9) )
	ROM_LOAD16_BYTE( "890f07.h22",  0x00001, 0x10000, CRC(6c74fa2e) SHA1(419a2ad31d269fafe4c474bf512e935d5e018846) )
	ROM_LOAD16_BYTE( "890f08.j22",  0x20000, 0x10000, CRC(29e12e80) SHA1(6d09e190055218e2dfd07838f1446dfb5f801206) )
	ROM_LOAD16_BYTE( "890f09.k22",  0x20001, 0x10000, CRC(67ca40d5) SHA1(ff719f55d2534ff076fbdd2bcb7d12c683bfe958) )

	ROM_REGION( 0x100000, "gfx2", 0 )	/* graphics (addressable by the main CPU) */
	ROM_LOAD( "890f04.h4",  0x00000, 0x80000, CRC(f16cd1fa) SHA1(60ea19c19918a71aded3c9ea398c956908e217f1) )
	ROM_LOAD( "890f05.k4",  0x80000, 0x80000, CRC(da37db05) SHA1(0b48d1021cf0dec78dae0ef183b4c61fea783533) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "63s141n.k14", 0x0000, 0x0100, CRC(d3620106) SHA1(528a0a34754902d0f262a9619c6105da6de99354) )	/* priority encoder (not used) */

	ROM_REGION( 0x80000, "k007232", 0 )	/* 512k for 007232 samples */
	ROM_LOAD( "890f03.d4",  0x00000, 0x80000, CRC(19065031) SHA1(12c47fbe28f85fa2f901fe52601188a5e9633f22) )
ROM_END



GAME( 1988, mainevt,  0,        mainevt,  mainevt,  0, ROT0,  "Konami", "The Main Event (4 Players ver. Y)", GAME_SUPPORTS_SAVE )
GAME( 1988, mainevto, mainevt,  mainevt,  mainevt,  0, ROT0,  "Konami", "The Main Event (4 Players ver. F)", GAME_SUPPORTS_SAVE )
GAME( 1988, mainevt2p,mainevt,  mainevt,  mainev2p, 0, ROT0,  "Konami", "The Main Event (2 Players ver. X)", GAME_SUPPORTS_SAVE )
GAME( 1988, ringohja, mainevt,  mainevt,  mainev2p, 0, ROT0,  "Konami", "Ring no Ohja (Japan 2 Players ver. N)", GAME_SUPPORTS_SAVE )
GAME( 1988, devstors, 0,        devstors, devstors, 0, ROT90, "Konami", "Devastators (ver. Z)", GAME_SUPPORTS_SAVE )
GAME( 1988, devstors2,devstors, devstors, devstor2, 0, ROT90, "Konami", "Devastators (ver. X)", GAME_SUPPORTS_SAVE )
GAME( 1988, devstors3,devstors, devstors, devstors, 0, ROT90, "Konami", "Devastators (ver. V)", GAME_SUPPORTS_SAVE )
GAME( 1988, garuka,   devstors, devstors, devstor2, 0, ROT90, "Konami", "Garuka (Japan ver. W)", GAME_SUPPORTS_SAVE )
