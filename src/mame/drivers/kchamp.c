// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Karate Champ - (c) 1984 Data East

driver by Ernesto Corvi


Changes:
(1999/11/11 Takahiro Nogi)
Changed "karatedo" to "karatevs".
Supported "karatedo".
Corrected DIPSW settings.(kchamp/karatedo)

Currently supported sets:
Karate Champ - 1 Player Version (kchamp)
Karate Champ - VS Version (kchampvs)
Karate Champ - 1 Player Version Japanese (karatedo)
Karate Champ - VS Version Japanese (karatevs)

VS Version Info:
---------------
Memory Map:
Main CPU
0000-bfff ROM (encrypted)
c000-cfff RAM
d000-d3ff char videoram
d400-d7ff color videoram
d800-d8ff sprites
e000-ffff ROM (encrypted)

Sound CPU
0000-5fff ROM
6000-6300 RAM

IO Ports:
Main CPU
INPUT  00 = Player 1 Controls - ( ACTIVE LOW )
INPUT  40 = Player 2 Controls - ( ACTIVE LOW )
INPUT  80 = Coins and Start Buttons - ( ACTIVE LOW )
INPUT  C0 = Dip Switches - ( ACTIVE LOW )
OUTPUT 00 = Screen Flip
OUTPUT 01 = CPU Control
                bit 0 = external nmi enable
OUTPUT 02 = Sound Reset
OUTPUT 40 = Sound latch write

Sound CPU
INPUT  01 = Sound latch read
OUTPUT 00 = AY8910 #1 data write
OUTPUT 01 = AY8910 #1 control write
OUTPUT 02 = AY8910 #2 data write
OUTPUT 03 = AY8910 #2 control write
OUTPUT 04 = MSM5205 write
OUTPUT 05 = CPU Control
                bit 0 = MSM5205 trigger
                bit 1 = external nmi enable

1P Version Info:
---------------
Same as VS version but with a DAC instead of a MSM5205. Also some minor
IO ports and memory map changes. Dip switches differ too.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/msm5205.h"
#include "includes/kchamp.h"


/********************
* VS Version        *
********************/

WRITE8_MEMBER(kchamp_state::control_w)
{
	m_nmi_enable = data & 1;
}

WRITE8_MEMBER(kchamp_state::sound_reset_w)
{
	if (!(data & 1))
		m_audiocpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
}

WRITE8_MEMBER(kchamp_state::sound_control_w)
{
	m_msm->reset_w(!(data & 1));
	m_sound_nmi_enable = ((data >> 1) & 1);
}

WRITE8_MEMBER(kchamp_state::sound_command_w)
{
	soundlatch_byte_w(space, 0, data);
	m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

WRITE8_MEMBER(kchamp_state::sound_msm_w)
{
	m_msm_data = data;
	m_msm_play_lo_nibble = 1;
}

static ADDRESS_MAP_START( kchampvs_map, AS_PROGRAM, 8, kchamp_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xcfff) AM_RAM
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(kchamp_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd400, 0xd7ff) AM_RAM_WRITE(kchamp_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xd800, 0xd8ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd900, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, kchamp_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kchampvs_io_map, AS_IO, 8, kchamp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("P1") AM_WRITE(kchamp_flipscreen_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(control_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(sound_reset_w)
	AM_RANGE(0x40, 0x40) AM_READ_PORT("P2") AM_WRITE(sound_command_w)
	AM_RANGE(0x80, 0x80) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xc0, 0xc0) AM_READ_PORT("DSW")
ADDRESS_MAP_END

static ADDRESS_MAP_START( kchampvs_sound_map, AS_PROGRAM, 8, kchamp_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kchampvs_sound_io_map, AS_IO, 8, kchamp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x01, 0x01) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0x04, 0x04) AM_WRITE(sound_msm_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(sound_control_w)
ADDRESS_MAP_END


/********************
* 1 Player Version  *
********************/
READ8_MEMBER(kchamp_state::sound_reset_r)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
	return 0;
}

WRITE8_MEMBER(kchamp_state::kc_sound_control_w)
{
	if (offset == 0)
		m_sound_nmi_enable = ((data >> 7) & 1);
//  else
//      DAC_set_volume(0, (data == 1) ? 255 : 0, 0);
}

static ADDRESS_MAP_START( kchamp_map, AS_PROGRAM, 8, kchamp_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xe3ff) AM_RAM_WRITE(kchamp_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe400, 0xe7ff) AM_RAM_WRITE(kchamp_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xea00, 0xeaff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xeb00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kchamp_io_map, AS_IO, 8, kchamp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_READ_PORT("DSW") AM_WRITE(kchamp_flipscreen_w)
	AM_RANGE(0x81, 0x81) AM_WRITE(control_w)
	AM_RANGE(0x90, 0x90) AM_READ_PORT("P1")
	AM_RANGE(0x98, 0x98) AM_READ_PORT("P2")
	AM_RANGE(0xa0, 0xa0) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xa8, 0xa8) AM_READWRITE(sound_reset_r, sound_command_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( kchamp_sound_map, AS_PROGRAM, 8, kchamp_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xe2ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( kchamp_sound_io_map, AS_IO, 8, kchamp_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("ay1", ay8910_device, data_address_w)
	AM_RANGE(0x02, 0x03) AM_DEVWRITE("ay2", ay8910_device, data_address_w)
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("dac", dac_device, write_unsigned8)
	AM_RANGE(0x05, 0x05) AM_WRITE(kc_sound_control_w)
	AM_RANGE(0x06, 0x06) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( kchampvs )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/********************
* 1 Player Version  *
********************/

static INPUT_PORTS_START( kchamp )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static const gfx_layout tilelayout =
{
	8,8,    /* tile size */
	256*8,  /* number of tiles */
	2,  /* bits per pixel */
	{ 0x4000*8, 0 }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7 }, /* x offsets */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 }, /* y offsets */
	8*8 /* offset to next tile */
};

static const gfx_layout spritelayout =
{
	16,16,  /* tile size */
	512,    /* number of tiles */
	2,  /* bits per pixel */
	{ 0xC000*8, 0 }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7,
		0x2000*8+0,0x2000*8+1,0x2000*8+2,0x2000*8+3,
		0x2000*8+4,0x2000*8+5,0x2000*8+6,0x2000*8+7 }, /* x offsets */
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
	8*8,9*8,10*8,11*8,12*8,13*8,14*8, 15*8 }, /* y offsets */
	16*8    /* ofset to next tile */
};

static GFXDECODE_START( kchamp )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,   32*4, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, spritelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, spritelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout, 0, 16 )
GFXDECODE_END



INTERRUPT_GEN_MEMBER(kchamp_state::kc_interrupt)
{
	if (m_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE_LINE_MEMBER(kchamp_state::msmint)
{
	if (m_msm_play_lo_nibble)
		m_msm->data_w(m_msm_data & 0x0f);
	else
		m_msm->data_w((m_msm_data >> 4) & 0x0f);

	m_msm_play_lo_nibble ^= 1;

	if (!(m_counter ^= 1))
	{
		if (m_sound_nmi_enable)
			m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

/********************
* 1 Player Version  *
********************/

INTERRUPT_GEN_MEMBER(kchamp_state::sound_int)
{
	if (m_sound_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


MACHINE_START_MEMBER(kchamp_state,kchamp)
{
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_sound_nmi_enable));
}

MACHINE_START_MEMBER(kchamp_state,kchampvs)
{
	MACHINE_START_CALL_MEMBER(kchamp);

	save_item(NAME(m_msm_data));
	save_item(NAME(m_msm_play_lo_nibble));
	save_item(NAME(m_counter));
}

void kchamp_state::machine_reset()
{
	m_nmi_enable = 0;
	m_sound_nmi_enable = 0;
}

static MACHINE_CONFIG_START( kchampvs, kchamp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/4)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(kchampvs_map)
	MCFG_CPU_IO_MAP(kchampvs_io_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kchamp_state,  kc_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/4)    /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(kchampvs_sound_map)
	MCFG_CPU_IO_MAP(kchampvs_sound_io_map)      /* irq's triggered from main cpu */
										/* nmi's from msm5205 */

	MCFG_MACHINE_START_OVERRIDE(kchamp_state,kchampvs)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.10) /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(kchamp_state, screen_update_kchampvs)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kchamp)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(kchamp_state, kchamp)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_12MHz/8)    /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_12MHz/8)    /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("msm", MSM5205, 375000)  /* verified on pcb, discrete circuit clock */
	MCFG_MSM5205_VCLK_CB(WRITELINE(kchamp_state, msmint))         /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)  /* 1 / 96 = 3906.25Hz playback */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/********************
* 1 Player Version  *
********************/

static MACHINE_CONFIG_START( kchamp, kchamp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz/4)  /* 12MHz / 4 = 3.0 MHz */
	MCFG_CPU_PROGRAM_MAP(kchamp_map)
	MCFG_CPU_IO_MAP(kchamp_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", kchamp_state,  kc_interrupt)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12MHz/4) /* 12MHz / 4 = 3.0 MHz */
	MCFG_CPU_PROGRAM_MAP(kchamp_sound_map)
	MCFG_CPU_IO_MAP(kchamp_sound_io_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(kchamp_state, sound_int,  125) /* Hz */
											/* irq's triggered from main cpu */
											/* nmi's from 125 Hz clock */

	MCFG_MACHINE_START_OVERRIDE(kchamp_state,kchamp)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(kchamp_state, screen_update_kchamp)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", kchamp)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(kchamp_state, kchamp)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_12MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_12MHz/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.15) /* guess: using volume 0.50 makes the sound to clip a lot */
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kchamp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b014.bin", 0x0000, 0x2000, CRC(0000d1a0) SHA1(0c584096825e1d3cc718e0bda1abb897a7f4d2df) )
	ROM_LOAD( "b015.bin", 0x2000, 0x2000, CRC(03fae67e) SHA1(3b6a30f39f5ad512415e3b8ba6e07f3118e28d9e) )
	ROM_LOAD( "b016.bin", 0x4000, 0x2000, CRC(3b6e1d08) SHA1(ecf7d2b0f31f04c0be7d5a1f450b9c95d9f54d80) )
	ROM_LOAD( "b017.bin", 0x6000, 0x2000, CRC(c1848d1a) SHA1(eb5f85d88e170e864d0cd4c372be2a193935caa2) )
	ROM_LOAD( "b018.bin", 0x8000, 0x2000, CRC(b824abc7) SHA1(4a30fec025150e889600a78497700155e743c99e) )
	ROM_LOAD( "b019.bin", 0xa000, 0x2000, CRC(3b487a46) SHA1(7837e480fd4648e0d3f792b79fa0019e063abdc6) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "b026.bin", 0x0000, 0x2000, CRC(999ed2c7) SHA1(f01e4ee81f8f7b0d1cf001d3ec01a9210f8109b4) )
	ROM_LOAD( "b025.bin", 0x2000, 0x2000, CRC(33171e07) SHA1(55ee74c9f1d86ec13d92ea0d1b700bbe24b79def) ) /* adpcm */
	ROM_LOAD( "b024.bin", 0x4000, 0x2000, CRC(910b48b9) SHA1(c6a2f8266ff1f14b462b92d47a4a86542df77cdd) ) /* adpcm */
	ROM_LOAD( "b023.bin", 0x6000, 0x2000, CRC(47f66aac) SHA1(484802cfbff7c5f51dd97cb3b2321e137b03481c) )
	ROM_LOAD( "b022.bin", 0x8000, 0x2000, CRC(5928e749) SHA1(a4dbd6f2a6a7aa9597875dfd781e55b0fb14d49b) )
	ROM_LOAD( "b021.bin", 0xa000, 0x2000, CRC(ca17e3ba) SHA1(91a3ccd6045dcef5f3293d669fe5a4df59cd954b) )
	ROM_LOAD( "b020.bin", 0xc000, 0x2000, CRC(ada4f2cd) SHA1(15a4ed7497cb6c06f523ebe38bc4eb6dbcd09549) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b000.bin", 0x00000, 0x2000, CRC(a4fa98a1) SHA1(33b9a1a56d72ffa5f4e16b69e6e19af5a2882b2c) )  /* plane0 */ /* tiles */
	ROM_LOAD( "b001.bin", 0x04000, 0x2000, CRC(fea09f7c) SHA1(174fc8022c455438538e6a3b67c7effc857ae634) )  /* plane1 */ /* tiles */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "b013.bin", 0x00000, 0x2000, CRC(eaad4168) SHA1(f31b05ffb86677157f3a44cdcf0f9a729e0ab259) )  /* top, plane0 */ /* sprites */
	ROM_LOAD( "b004.bin", 0x02000, 0x2000, CRC(10a47e2d) SHA1(97fe2de3ce2b8dc017dceffce494be18695708d2) )  /* bot, plane0 */ /* sprites */
	ROM_LOAD( "b012.bin", 0x04000, 0x2000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )  /* top, plane0 */ /* sprites */
	ROM_LOAD( "b003.bin", 0x06000, 0x2000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )  /* bot, plane0 */ /* sprites */
	ROM_LOAD( "b011.bin", 0x08000, 0x2000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )  /* top, plane0 */ /* sprites */
	ROM_LOAD( "b002.bin", 0x0a000, 0x2000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )  /* bot, plane0 */ /* sprites */
	ROM_LOAD( "b007.bin", 0x0c000, 0x2000, CRC(cb91d16b) SHA1(bf32a03e7882b74280b29fa004429b77ad52e5ee) )  /* top, plane1 */ /* sprites */
	ROM_LOAD( "b010.bin", 0x0e000, 0x2000, CRC(489c9c04) SHA1(d920ef4f03e1b2e871df0cb2d672689c89febe96) )  /* bot, plane1 */ /* sprites */
	ROM_LOAD( "b006.bin", 0x10000, 0x2000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )  /* top, plane1 */ /* sprites */
	ROM_LOAD( "b009.bin", 0x12000, 0x2000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )  /* bot, plane1 */ /* sprites */
	ROM_LOAD( "b005.bin", 0x14000, 0x2000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )  /* top, plane1 */ /* sprites */
	ROM_LOAD( "b008.bin", 0x16000, 0x2000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )  /* bot, plane1 */ /* sprites */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) /* red */
	ROM_LOAD( "br26", 0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) /* green */
	ROM_LOAD( "br25", 0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) /* blue */
ROM_END

ROM_START( karatedo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "be14", 0x0000, 0x2000, CRC(44e60aa0) SHA1(6d007d7082c15182832f947444b00b7feb0e7738) )
	ROM_LOAD( "be15", 0x2000, 0x2000, CRC(a65e3793) SHA1(bf1e8fbc6755e85414eb7629e6fab3bf154f6546) )
	ROM_LOAD( "be16", 0x4000, 0x2000, CRC(151d8872) SHA1(1bb27142fdb33e3aeaf95c7a0ad7e8c258bbcb66) )
	ROM_LOAD( "be17", 0x6000, 0x2000, CRC(8f393b6a) SHA1(f246a6e069a2f562c5b7de05a2b8a6a09c1f4d1b) )
	ROM_LOAD( "be18", 0x8000, 0x2000, CRC(a09046ad) SHA1(665973bffc38e36b8b0f6bc79e10db280be0613e) )
	ROM_LOAD( "be19", 0xa000, 0x2000, CRC(0cdc4da9) SHA1(405454deda311abb8badd58a47529e42ddce5f6a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "be26", 0x0000, 0x2000, CRC(999ab0a3) SHA1(6aa545cee7a261f3dc5774dea2066f1412f63f49) )
	ROM_LOAD( "be25", 0x2000, 0x2000, CRC(253bf0da) SHA1(33bae6401003dc57deaa14bf7f6a7ebad5b7efe3) ) /* adpcm */
	ROM_LOAD( "be24", 0x4000, 0x2000, CRC(e2c188af) SHA1(b7a0801a4c634694f1556873fd21f7e13441be17) ) /* adpcm */
	ROM_LOAD( "be23", 0x6000, 0x2000, CRC(25262de1) SHA1(6264cd82756be9e1cdcd9ad3c3dfc6fef78dab8f) )
	ROM_LOAD( "be22", 0x8000, 0x2000, CRC(38055c48) SHA1(8406a52aaa7e56093a8d8552e928988b6fdd6c95) )
	ROM_LOAD( "be21", 0xa000, 0x2000, CRC(5f0efbe7) SHA1(f831efd02c917adac827fe6db8449ca8707b3d44) )
	ROM_LOAD( "be20", 0xc000, 0x2000, CRC(cbe8a533) SHA1(04cb41c487c2f951417628ed2888e04d59a39d29) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "be00",     0x00000, 0x2000, CRC(cec020f2) SHA1(07c501cc24797000f369fd98a26efe13875107bb) )  /* plane0 */ /* tiles */
	ROM_LOAD( "be01",     0x04000, 0x2000, CRC(cd96271c) SHA1(bcc71010e5489b19ad1553141c7b2e366bbbc68f) )  /* plane1 */ /* tiles */

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "be13",     0x00000, 0x2000, CRC(fb358707) SHA1(37124f1f545787723fecf466d8dcd31b88cdd75d) )  /* top, plane0 */ /* sprites */
	ROM_LOAD( "be04",     0x02000, 0x2000, CRC(48372bf8) SHA1(28231b3bdb1d7226d7856554ba667b6d61f4fe22) )  /* bot, plane0 */ /* sprites */
	ROM_LOAD( "b012.bin", 0x04000, 0x2000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )  /* top, plane0 */ /* sprites */
	ROM_LOAD( "b003.bin", 0x06000, 0x2000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )  /* bot, plane0 */ /* sprites */
	ROM_LOAD( "b011.bin", 0x08000, 0x2000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )  /* top, plane0 */ /* sprites */
	ROM_LOAD( "b002.bin", 0x0a000, 0x2000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )  /* bot, plane0 */ /* sprites */
	ROM_LOAD( "be07",     0x0c000, 0x2000, CRC(40f2b6fb) SHA1(8d9ee04d917a8e143bd00fa7582990213bfa42d3) )  /* top, plane1 */ /* sprites */
	ROM_LOAD( "be10",     0x0e000, 0x2000, CRC(325c0a97) SHA1(0159536ff0ebac8ccf65aac1a524a30b3fca3418) )  /* bot, plane1 */ /* sprites */
	ROM_LOAD( "b006.bin", 0x10000, 0x2000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )  /* top, plane1 */ /* sprites */
	ROM_LOAD( "b009.bin", 0x12000, 0x2000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )  /* bot, plane1 */ /* sprites */
	ROM_LOAD( "b005.bin", 0x14000, 0x2000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )  /* top, plane1 */ /* sprites */
	ROM_LOAD( "b008.bin", 0x16000, 0x2000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )  /* bot, plane1 */ /* sprites */

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) /* red */
	ROM_LOAD( "br26", 0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) /* green */
	ROM_LOAD( "br25", 0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) /* blue */
ROM_END

ROM_START( kchampvs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bs24.d13", 0x00000, 0x02000, CRC(829da69b) SHA1(3266e7686e537f34ee5ce4cccc349eb12fc65038) )
	ROM_LOAD( "bs23.d11", 0x02000, 0x02000, CRC(091f810e) SHA1(283edb08ce106835185a1c2d6b88f7544d75f3b4) )
	ROM_LOAD( "bs22.d10", 0x04000, 0x02000, CRC(d4df2a52) SHA1(60d6cb1cb51c6f80a0f88913d4152ab8bda752d6) )
	ROM_LOAD( "bs21.d8",  0x06000, 0x02000, CRC(3d4ef0da) SHA1(228c8e47bb7123b69746506402edb875a43d7af5) )
	ROM_LOAD( "bs20.d7",  0x08000, 0x02000, CRC(623a467b) SHA1(5f150c67632f8e32769b75aa0615d0eb018afdc4) )
	ROM_LOAD( "bs19.d6",  0x0a000, 0x02000, CRC(43e196c4) SHA1(8029798ea0a560603c3dcde56db5a1ccde58c514) )
	ROM_CONTINUE(         0x0e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "bs18.d4",  0x00000, 0x02000, CRC(eaa646eb) SHA1(cbd48f4d5d225b71c2dd0b14f420838561e3f83e) )
	ROM_LOAD( "bs17.d2",  0x02000, 0x02000, CRC(d71031ad) SHA1(b168f4ef4feb4195305404df699acecb731eab02) )
	ROM_LOAD( "bs16.d1",  0x04000, 0x02000, CRC(6f811c43) SHA1(1d33ac8129562ab709bd7396b4c2457b6db99277) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bs12.k1",  0x00000, 0x02000, CRC(4c574ecd) SHA1(86914eef33da73463ba6261eecae75209d24fac1) )
	ROM_LOAD( "bs13.k3",  0x02000, 0x02000, CRC(750b66af) SHA1(c7824994b977d4e846f3ecadfcfc51331f52b6f4) )
	ROM_LOAD( "bs14.k5",  0x04000, 0x02000, CRC(9ad6227c) SHA1(708af5e70927040cf7f2ae6f792344c19099530c) )
	ROM_LOAD( "bs15.k6",  0x06000, 0x02000, CRC(3b6d5de5) SHA1(288fffcbc9369db5c75e7e0d6181612de6f12da3) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "bs00.a1",  0x00000, 0x02000, CRC(51eda56c) SHA1(31438e115e95c2a684ec65ed2bdb9125e3675226) )
	ROM_LOAD( "bs06.c1",  0x02000, 0x02000, CRC(593264cf) SHA1(866469f37b6c90afc65e53e6589b67ac4b25997e) )
	ROM_LOAD( "bs01.a3",  0x04000, 0x02000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )
	ROM_LOAD( "bs07.c3",  0x06000, 0x02000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )
	ROM_LOAD( "bs02.a5",  0x08000, 0x02000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )
	ROM_LOAD( "bs08.c5",  0x0a000, 0x02000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )
	ROM_LOAD( "bs03.a6",  0x0c000, 0x02000, CRC(8dcd271a) SHA1(0abeaa46433a59c110815ecf188c7afd6fa387a4) )
	ROM_LOAD( "bs09.c6",  0x0e000, 0x02000, CRC(4ee1dba7) SHA1(717ce9a4e20f6e02adf678b1400af4aaecdbfb40) )
	ROM_LOAD( "bs04.a8",  0x10000, 0x02000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )
	ROM_LOAD( "bs10.c8",  0x12000, 0x02000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )
	ROM_LOAD( "bs05.a10", 0x14000, 0x02000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )
	ROM_LOAD( "bs11.c10", 0x16000, 0x02000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27.k10", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) /* red */
	ROM_LOAD( "br26.k9",  0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) /* green */
	ROM_LOAD( "br25.k8",  0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) /* blue */
ROM_END

ROM_START( kchampvs2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lt.d13",   0x00000, 0x02000, CRC(eef41aa8) SHA1(6d4e8159e9c3cd629337863c0397ff90b4c8d3fa) )
	ROM_LOAD( "lt.d11",   0x02000, 0x02000, CRC(091f810e) SHA1(283edb08ce106835185a1c2d6b88f7544d75f3b4) )
	ROM_LOAD( "lt.d10",   0x04000, 0x02000, CRC(d4df2a52) SHA1(60d6cb1cb51c6f80a0f88913d4152ab8bda752d6) )
	ROM_LOAD( "lt.d8",    0x06000, 0x02000, CRC(3d4ef0da) SHA1(228c8e47bb7123b69746506402edb875a43d7af5) )
	ROM_LOAD( "lt.d7",    0x08000, 0x02000, CRC(623a467b) SHA1(5f150c67632f8e32769b75aa0615d0eb018afdc4) )
	ROM_LOAD( "lt.d6",    0x0a000, 0x02000, CRC(c3bc6e46) SHA1(a7b9420592905b0df5ff00c392d887f40395179f) )
	ROM_CONTINUE(         0x0e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "lt.d4",    0x00000, 0x02000, CRC(eaa646eb) SHA1(cbd48f4d5d225b71c2dd0b14f420838561e3f83e) )
	ROM_LOAD( "lt.d2",    0x02000, 0x02000, CRC(d71031ad) SHA1(b168f4ef4feb4195305404df699acecb731eab02) )
	ROM_LOAD( "lt.d1",    0x04000, 0x02000, CRC(6f811c43) SHA1(1d33ac8129562ab709bd7396b4c2457b6db99277) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "lt.k1",    0x00000, 0x02000, CRC(4c574ecd) SHA1(86914eef33da73463ba6261eecae75209d24fac1) )
	ROM_LOAD( "lt.k3",    0x02000, 0x02000, CRC(750b66af) SHA1(c7824994b977d4e846f3ecadfcfc51331f52b6f4) )
	ROM_LOAD( "lt.k5",    0x04000, 0x02000, CRC(9ad6227c) SHA1(708af5e70927040cf7f2ae6f792344c19099530c) )
	ROM_LOAD( "lt.k6",    0x06000, 0x02000, CRC(3b6d5de5) SHA1(288fffcbc9369db5c75e7e0d6181612de6f12da3) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "lt.a1",    0x00000, 0x02000, CRC(51eda56c) SHA1(31438e115e95c2a684ec65ed2bdb9125e3675226) )
	ROM_LOAD( "lt.c1",    0x02000, 0x02000, CRC(593264cf) SHA1(866469f37b6c90afc65e53e6589b67ac4b25997e) )
	ROM_LOAD( "lt.a3",    0x04000, 0x02000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )
	ROM_LOAD( "lt.c3",    0x06000, 0x02000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )
	ROM_LOAD( "lt.a5",    0x08000, 0x02000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )
	ROM_LOAD( "lt.c5",    0x0a000, 0x02000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )
	ROM_LOAD( "lt.a6",    0x0c000, 0x02000, CRC(8dcd271a) SHA1(0abeaa46433a59c110815ecf188c7afd6fa387a4) )
	ROM_LOAD( "lt.c6",    0x0e000, 0x02000, CRC(4ee1dba7) SHA1(717ce9a4e20f6e02adf678b1400af4aaecdbfb40) )
	ROM_LOAD( "lt.a8",    0x10000, 0x02000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )
	ROM_LOAD( "lt.c8",    0x12000, 0x02000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )
	ROM_LOAD( "lt.a10",   0x14000, 0x02000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )
	ROM_LOAD( "lt.c10",   0x16000, 0x02000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "lt.k10",   0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) /* red */
	ROM_LOAD( "lt.k9",    0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) /* green */
	ROM_LOAD( "lt.k8",    0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) /* blue */
ROM_END

ROM_START( karatevs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "br24.d13", 0x00000, 0x02000, CRC(ea9cda49) SHA1(7d753a8d391418d0fe5231eb88b3627f7d3fd99e) )
	ROM_LOAD( "br23.d11", 0x02000, 0x02000, CRC(46074489) SHA1(5593f819b6893820ef0c0fece13cf3ca83e1ab85) )
	ROM_LOAD( "br22.d10", 0x04000, 0x02000, CRC(294f67ba) SHA1(45f13a7deb75bb167176c5405128de3ca76e22f0) )
	ROM_LOAD( "br21.d8",  0x06000, 0x02000, CRC(934ea874) SHA1(dbc139715a1598033beedbf4f8fec73703b016d6) )
	ROM_LOAD( "br20.d7",  0x08000, 0x02000, CRC(97d7816a) SHA1(e02f9306fc3539f4feaedfcabea66d172d09a510) )
	ROM_LOAD( "br19.d6",  0x0a000, 0x02000, CRC(dd2239d2) SHA1(0533d5abf8e25a4aeec2f7832b657eab56fd11f0) )
	ROM_CONTINUE(         0x0e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound CPU */
	ROM_LOAD( "br18.d4",  0x00000, 0x02000, CRC(00ccb8ea) SHA1(83d69684dc3ad37aca03c901fd23c7652134766f) )
	ROM_LOAD( "bs17.d2",  0x02000, 0x02000, CRC(d71031ad) SHA1(b168f4ef4feb4195305404df699acecb731eab02) )
	ROM_LOAD( "br16.d1",  0x04000, 0x02000, CRC(2512d961) SHA1(f0cd1be112b915d700e0587759606d48d115a83f) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "br12.k1",  0x00000, 0x02000, CRC(9ed6f00d) SHA1(3def985deb29a7644309ede3bd82c225b4ae23f8) )
	ROM_LOAD( "bs13.k3",  0x02000, 0x02000, CRC(750b66af) SHA1(c7824994b977d4e846f3ecadfcfc51331f52b6f4) )
	ROM_LOAD( "br14.k5",  0x04000, 0x02000, CRC(fc399229) SHA1(e8d633151b0d7fa49c455920c4b0588575a7084e) )
	ROM_LOAD( "bs15.k6",  0x06000, 0x02000, CRC(3b6d5de5) SHA1(288fffcbc9369db5c75e7e0d6181612de6f12da3) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "br00.a1",  0x00000, 0x02000, CRC(c46a8b88) SHA1(a47e56a6dc7f36b896b8156e77a1da7e8be2332e) )
	ROM_LOAD( "br06.c1",  0x02000, 0x02000, CRC(cf8982ff) SHA1(aafb249503ad51f64b1f31ea2d869dfc0e065d19) )
	ROM_LOAD( "bs01.a3",  0x04000, 0x02000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )
	ROM_LOAD( "bs07.c3",  0x06000, 0x02000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )
	ROM_LOAD( "bs02.a5",  0x08000, 0x02000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )
	ROM_LOAD( "bs08.c5",  0x0a000, 0x02000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )
	ROM_LOAD( "br03.a6",  0x0c000, 0x02000, CRC(bde8a52b) SHA1(1a0800472caf8c79a15cc977dad1a7bc97c74b2b) )
	ROM_LOAD( "br09.c6",  0x0e000, 0x02000, CRC(e9a5f945) SHA1(e6b21912bee97de06819c8ac85a45bbc70030f88) )
	ROM_LOAD( "bs04.a8",  0x10000, 0x02000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )
	ROM_LOAD( "bs10.c8",  0x12000, 0x02000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )
	ROM_LOAD( "bs05.a10", 0x14000, 0x02000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )
	ROM_LOAD( "bs11.c10", 0x16000, 0x02000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27.k10", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) /* red */
	ROM_LOAD( "br26.k9",  0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) /* green */
	ROM_LOAD( "br25.k8",  0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) /* blue */
ROM_END


void kchamp_state::decrypt_code()
{
	UINT8 *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
		m_decrypted_opcodes[A] = (rom[A] & 0x55) | ((rom[A] & 0x88) >> 2) | ((rom[A] & 0x22) << 2);
}


DRIVER_INIT_MEMBER(kchamp_state,kchampvs)
{
	decrypt_code();

	UINT8 *rom = memregion("maincpu")->base();
	int A;

	/*
	    Note that the first 4 opcodes that the program
	    executes aren't encrypted for some obscure reason.
	    The address for the 2nd opcode (a jump) is encrypted too.
	    It's not clear what the 3rd and 4th opcode are supposed to do,
	    they just write to a RAM location. This write might be what
	    turns the encryption on, but this doesn't explain the
	    encrypted address for the jump.
	 */
	m_decrypted_opcodes[0] = rom[0];  /* this is a jump */
	A = rom[1] + 256 * rom[2];
	m_decrypted_opcodes[A] = rom[A];  /* fix opcode on first jump address (again, a jump) */
	rom[A+1] ^= 0xee;       /* fix address of the second jump */
	A = rom[A+1] + 256 * rom[A+2];
	m_decrypted_opcodes[A] = rom[A];  /* fix third opcode (ld a,$xx) */
	A += 2;
	m_decrypted_opcodes[A] = rom[A];  /* fix fourth opcode (ld ($xxxx),a */
	/* and from here on, opcodes are encrypted */

	m_counter = 0;
	m_msm_data = 0;
	m_msm_play_lo_nibble = 0;
}


DRIVER_INIT_MEMBER(kchamp_state,kchampvs2)
{
	decrypt_code();
	m_counter = 0;
	m_msm_data = 0;
	m_msm_play_lo_nibble = 1;
}



GAME( 1984, kchamp,    0,      kchamp,   kchamp,   driver_device, 0,         ROT90, "Data East USA",         "Karate Champ (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, karatedo,  kchamp, kchamp,   kchamp,   driver_device, 0,         ROT90, "Data East Corporation", "Karate Dou (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, kchampvs,  kchamp, kchampvs, kchampvs, kchamp_state,  kchampvs,  ROT90, "Data East USA",         "Karate Champ (US VS version, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, kchampvs2, kchamp, kchampvs, kchampvs, kchamp_state,  kchampvs2, ROT90, "Data East USA",         "Karate Champ (US VS version, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, karatevs,  kchamp, kchampvs, kchampvs, kchamp_state,  kchampvs,  ROT90, "Data East Corporation", "Taisen Karate Dou (Japan VS version)", MACHINE_SUPPORTS_SAVE )
