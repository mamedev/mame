// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
/***************************************************************************

Splash! (c) 1992 Gaelco
Return of Ladyfrog (c) 1993 Microhard   (hack/bootleg of splash)
Funny Strip (c)199? Microhard / MagicGames
Rebus (c)1995 Microhard

Driver by Manuel Abadia <emumanu+mame@gmail.com>

Return of Lady Frog addition by David Haywood

special thanks to

Pierpaolo Prazzoli
Petr1fied,
ninjakid,
Reznor007,
Malice,
tb2000,
Mr. Do,
Roberto Fresca,
f205v,
XZeriX,
[red],
MOCAS,
Jonemaan
BIOS-D

notes:
Sound not working on Return of Lady Frog

TS 2006.12.22:
- Funny Strip is runing on pSOS RTOS ( http://en.wikipedia.org/wiki/PSOS and http://dr-linux.net/newbase/reference/psosCD/ ) .
  There's copyright text at $480
  Also Rebus and TRoLF are running on it (the same internal code structure - traps, interrupt vectors),
  but copyright messages are removed.
- Rebus protection patch sits at the end of trap $b (rtos call) and in some cases returns 0 in D0.
  It's not a real protection check i think.

More notes about Funny Strip protection issues at the bottom of source file (DRIVER INIT)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "includes/splash.h"

WRITE16_MEMBER(splash_state::splash_sh_irqtrigger_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

WRITE16_MEMBER(splash_state::roldf_sh_irqtrigger_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}

	// give the z80 time to see it
	space.device().execute().spin_until_time(attotime::from_usec(40));
}

WRITE16_MEMBER(splash_state::coin_w)
{
	if (ACCESSING_BITS_8_15)
	{
		switch ((offset >> 3))
		{
			case 0x00:  /* Coin Lockouts */
			case 0x01:
				coin_lockout_w( machine(), (offset >> 3) & 0x01, (data & 0x0400) >> 8);
				break;
			case 0x02:  /* Coin Counters */
			case 0x03:
				coin_counter_w( machine(), (offset >> 3) & 0x01, (data & 0x0100) >> 8);
				break;
		}
	}
}

static ADDRESS_MAP_START( splash_map, AS_PROGRAM, 16, splash_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM                                                 /* ROM */
	AM_RANGE(0x800000, 0x83ffff) AM_RAM AM_SHARE("pixelram")                        /* Pixel Layer */
	AM_RANGE(0x840000, 0x840001) AM_READ_PORT("DSW1")
	AM_RANGE(0x840002, 0x840003) AM_READ_PORT("DSW2")
	AM_RANGE(0x840004, 0x840005) AM_READ_PORT("P1")
	AM_RANGE(0x840006, 0x840007) AM_READ_PORT("P2")
	AM_RANGE(0x84000e, 0x84000f) AM_WRITE(splash_sh_irqtrigger_w)                       /* Sound command */
	AM_RANGE(0x84000a, 0x84003b) AM_WRITE(coin_w)                                /* Coin Counters + Coin Lockout */
	AM_RANGE(0x880000, 0x8817ff) AM_RAM_WRITE(vram_w) AM_SHARE("videoram")   /* Video RAM */
	AM_RANGE(0x881800, 0x881803) AM_RAM AM_SHARE("vregs")                           /* Scroll registers */
	AM_RANGE(0x881804, 0x881fff) AM_RAM                                                 /* Work RAM */
	AM_RANGE(0x8c0000, 0x8c0fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")/* Palette is xRRRRxGGGGxBBBBx */
	AM_RANGE(0x900000, 0x900fff) AM_RAM AM_SHARE("spriteram")                       /* Sprite RAM */
	AM_RANGE(0xffc000, 0xffffff) AM_RAM                                                 /* Work RAM */
ADDRESS_MAP_END

WRITE8_MEMBER(splash_state::splash_adpcm_data_w)
{
	m_adpcm_data = data;
}

WRITE_LINE_MEMBER(splash_state::splash_msm5205_int)
{
	m_msm->data_w(m_adpcm_data >> 4);
	m_adpcm_data = (m_adpcm_data << 4) & 0xf0;
}

static ADDRESS_MAP_START( splash_sound_map, AS_PROGRAM, 8, splash_state )
	AM_RANGE(0x0000, 0xd7ff) AM_ROM                                     /* ROM */
	AM_RANGE(0xd800, 0xd800) AM_WRITE(splash_adpcm_data_w)              /* ADPCM data for the MSM5205 chip */
//  AM_RANGE(0xe000, 0xe000) AM_WRITENOP                                /* ??? */
	AM_RANGE(0xe800, 0xe800) AM_READ(soundlatch_byte_r)                     /* Sound latch */
	AM_RANGE(0xf000, 0xf001) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write) /* YM3812 */
	AM_RANGE(0xf800, 0xffff) AM_RAM                                     /* RAM */
ADDRESS_MAP_END

/* Return of Lady Frog Maps */
/* note, sprite ram has moved, extra protection ram, and extra write for the pixel layer */

READ16_MEMBER(splash_state::roldfrog_bombs_r)
{
	m_ret ^= 0x100;
	return m_ret;
}

WRITE8_MEMBER(splash_state::sound_bank_w)
{
	membank("sound_bank")->set_entry(data & 0xf);
}


void splash_state::roldfrog_update_irq(  )
{
	int irq = (m_sound_irq ? 0x08 : 0) | ((m_vblank_irq) ? 0x18 : 0);
	m_audiocpu->set_input_line_and_vector(0, irq ? ASSERT_LINE : CLEAR_LINE, 0xc7 | irq);
}

WRITE8_MEMBER(splash_state::roldfrog_vblank_ack_w)
{
	m_vblank_irq = 0;
	roldfrog_update_irq();
}


WRITE_LINE_MEMBER(splash_state::ym_irq)
{
	m_sound_irq = state;
	roldfrog_update_irq();
}

static ADDRESS_MAP_START( roldfrog_map, AS_PROGRAM, 16, splash_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM                                                 /* ROM */
	AM_RANGE(0x400000, 0x407fff) AM_ROM AM_SHARE("protdata")                        /* Protection Data */
	AM_RANGE(0x408000, 0x4087ff) AM_RAM                                                 /* Extra Ram */
	AM_RANGE(0x800000, 0x83ffff) AM_RAM AM_SHARE("pixelram")                        /* Pixel Layer */
	AM_RANGE(0x840000, 0x840001) AM_READ_PORT("DSW1")
	AM_RANGE(0x840002, 0x840003) AM_READ_PORT("DSW2")
	AM_RANGE(0x840004, 0x840005) AM_READ_PORT("P1")
	AM_RANGE(0x840006, 0x840007) AM_READ_PORT("P2")
	AM_RANGE(0x84000e, 0x84000f) AM_WRITE(roldf_sh_irqtrigger_w)                        /* Sound command */
	AM_RANGE(0x84000a, 0x84003b) AM_WRITE(coin_w)                                /* Coin Counters + Coin Lockout */
	AM_RANGE(0x880000, 0x8817ff) AM_RAM_WRITE(vram_w) AM_SHARE("videoram")   /* Video RAM */
	AM_RANGE(0x881800, 0x881803) AM_RAM AM_SHARE("vregs")                           /* Scroll registers */
	AM_RANGE(0x881804, 0x881fff) AM_RAM                                                 /* Work RAM */
	AM_RANGE(0x8c0000, 0x8c0fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")/* Palette is xRRRRxGGGGxBBBBx */
	AM_RANGE(0xa00000, 0xa00001) AM_READ(roldfrog_bombs_r)
	AM_RANGE(0xd00000, 0xd00fff) AM_RAM AM_SHARE("spriteram")                       /* Sprite RAM */
	AM_RANGE(0xe00000, 0xe00001) AM_WRITEONLY AM_SHARE("bitmap_mode")           /* Bitmap Mode? */
	AM_RANGE(0xffc000, 0xffffff) AM_RAM                                                 /* Work RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( roldfrog_sound_map, AS_PROGRAM, 8, splash_state )
	AM_RANGE(0x0000, 0x6fff) AM_ROM
	AM_RANGE(0x7000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_ROMBANK("sound_bank")
ADDRESS_MAP_END

READ8_MEMBER(splash_state::roldfrog_unk_r)
{
	// dragon punch leftovers
	return 0xff;
}

static ADDRESS_MAP_START( roldfrog_sound_io_map, AS_IO, 8, splash_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x11) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x40, 0x40) AM_NOP
	AM_RANGE(0x31, 0x31) AM_WRITE(sound_bank_w)
	AM_RANGE(0x37, 0x37) AM_WRITE(roldfrog_vblank_ack_w )
	AM_RANGE(0x70, 0x70) AM_READ(soundlatch_byte_r)

	AM_RANGE(0x0, 0xff) AM_READ(roldfrog_unk_r)
ADDRESS_MAP_END

READ16_MEMBER(splash_state::spr_read)
{
	return m_spriteram[offset]|0xff00;
}

WRITE16_MEMBER(splash_state::spr_write)
{
	COMBINE_DATA(&m_spriteram[offset]);
	m_spriteram[offset]|=0xff00; /* 8 bit, expected 0xffnn when read as 16 bit */
}

WRITE16_MEMBER(splash_state::funystrp_sh_irqtrigger_w)
{
	soundlatch_byte_w(space, 0, data>>8);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static ADDRESS_MAP_START( funystrp_map, AS_PROGRAM, 16, splash_state )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM                                                 /* ROM */
	AM_RANGE(0x100000, 0x1fffff) AM_RAM                                                 /* protection? RAM */
	AM_RANGE(0x800000, 0x83ffff) AM_RAM AM_SHARE("pixelram")                        /* Pixel Layer */
	AM_RANGE(0x84000a, 0x84000b) AM_WRITE(coin_w)                                /* Coin Counters + Coin Lockout */
	AM_RANGE(0x84000e, 0x84000f) AM_WRITE(funystrp_sh_irqtrigger_w)                       /* Sound command */
	AM_RANGE(0x840000, 0x840001) AM_READ_PORT("DSW1")
	AM_RANGE(0x840002, 0x840003) AM_READ_PORT("DSW2")
	AM_RANGE(0x840004, 0x840005) AM_READ_PORT("P1")
	AM_RANGE(0x840006, 0x840007) AM_READ_PORT("P2")
	AM_RANGE(0x840008, 0x840009) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x880000, 0x8817ff) AM_RAM_WRITE(vram_w) AM_SHARE("videoram")   /* Video RAM */
	AM_RANGE(0x881800, 0x881803) AM_RAM AM_SHARE("vregs")                           /* Scroll registers */
	AM_RANGE(0x881804, 0x881fff) AM_WRITENOP
	AM_RANGE(0x8c0000, 0x8c0fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")/* Palette is xRRRRxGGGGxBBBBx */
	AM_RANGE(0xd00000, 0xd01fff) AM_READWRITE(spr_read, spr_write) AM_SHARE("spriteram")        /* Sprite RAM */
	AM_RANGE(0xfe0000, 0xffffff) AM_RAM  AM_MASK(0xffff) /* there's fe0000 <-> ff0000 compare */                /* Work RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( funystrp_sound_map, AS_PROGRAM, 8, splash_state )
	AM_RANGE(0x0000, 0x6fff) AM_ROM
	AM_RANGE(0x7000, 0x7fff) AM_RAM
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_ROMBANK("sound_bank")
ADDRESS_MAP_END

READ8_MEMBER(splash_state::int_source_r)
{
	return ~m_msm_source;
}

WRITE8_MEMBER(splash_state::msm1_data_w)
{
	m_msm_data1=data;
	m_msm_source&=~1;
	m_msm_toggle1=0;
}

WRITE8_MEMBER(splash_state::msm1_interrupt_w)
{
	m_snd_interrupt_enable1=~data;
}

WRITE8_MEMBER(splash_state::msm2_interrupt_w)
{
	m_snd_interrupt_enable2=~data;
}

WRITE8_MEMBER(splash_state::msm2_data_w)
{
	m_msm_data2=data;
	m_msm_source&=~2;
	m_msm_toggle2=0;
}

static ADDRESS_MAP_START( funystrp_sound_io_map, AS_IO, 8, splash_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(msm1_data_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(msm2_data_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(sound_bank_w)
	AM_RANGE(0x03, 0x03) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x04, 0x04) AM_READ(int_source_r)
	AM_RANGE(0x06, 0x06) AM_WRITE(msm1_interrupt_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(msm2_interrupt_w)
ADDRESS_MAP_END

MACHINE_RESET_MEMBER(splash_state,funystrp)
{
	m_adpcm_data = 0;
	m_ret = 0x100;
}


static INPUT_PORTS_START( splash )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "1C/1C or Free Play (if Coin B too)" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "1C/1C or Free Play (if Coin A too)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	/*  according to the manual, Lives = 0x00 is NOT used */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Girls" )
	PORT_DIPSETTING(    0x00, "Light" )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Paint Mode" )
	PORT_DIPSETTING(    0x00, "Paint again" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( funystrp )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "1C/1C or Free Play (if Coin B too)" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "1C/1C or Free Play (if Coin A too)" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, "Bonus" )
	PORT_DIPSETTING(    0x30, "Bonus Chance HI/LO Cards" )
	PORT_DIPSETTING(    0x10, "Life at Stage 10" )
	PORT_DIPSETTING(    0x00, "Life at Stage 20" )
	PORT_DIPSETTING(    0x20, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x80, 0x80, "Strip Mode" )
	PORT_DIPSETTING(    0x80, "Soft" )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_DIPNAME( 0xffff, 0x0000, "Clear EEPROM" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0xffff, DEF_STR( On ) )

INPUT_PORTS_END

static const gfx_layout tilelayout8 =
{
	8,8,                                    /* 8x8 tiles */
	0x20000/8,                              /* number of tiles */
	4,                                      /* bitplanes */
	{ 0*0x20000*8, 1*0x20000*8, 2*0x20000*8, 3*0x20000*8 }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static const gfx_layout tilelayout16 =
{
	16,16,                                  /* 16x16 tiles */
	0x20000/32,                             /* number of tiles */
	4,                                      /* bitplanes */
	{ 0*0x20000*8, 1*0x20000*8, 2*0x20000*8, 3*0x20000*8 }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7, 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};

static GFXDECODE_START( splash )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout8 ,0,128 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout16,0,128 )
GFXDECODE_END


MACHINE_START_MEMBER(splash_state,splash)
{
	save_item(NAME(m_adpcm_data));
}

MACHINE_RESET_MEMBER(splash_state,splash)
{
	m_adpcm_data = 0;
	m_ret = 0x100;
}

static MACHINE_CONFIG_START( splash, splash_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)       /* 12MHz (24/2) */
	MCFG_CPU_PROGRAM_MAP(splash_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", splash_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_30MHz/8)     /* 3.75MHz (30/8) */
	MCFG_CPU_PROGRAM_MAP(splash_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(splash_state, nmi_line_pulse, 60*64)   /* needed for the msm5205 to play the samples */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 48*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(splash_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", splash)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_MACHINE_START_OVERRIDE(splash_state, splash )
	MCFG_MACHINE_RESET_OVERRIDE(splash_state, splash )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_30MHz/8)       /* 3.75MHz (30/8) */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("msm", MSM5205, XTAL_384kHz)
	MCFG_MSM5205_VCLK_CB(WRITELINE(splash_state, splash_msm5205_int)) /* IRQ handler */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 8KHz */     /* Sample rate = 384kHz/48 */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


MACHINE_START_MEMBER(splash_state, roldfrog)
{
	save_item(NAME(m_ret));
	save_item(NAME(m_vblank_irq));
	save_item(NAME(m_sound_irq));
}

INTERRUPT_GEN_MEMBER(splash_state::roldfrog_interrupt)
{
	m_vblank_irq = 1;
	roldfrog_update_irq();
}

static MACHINE_CONFIG_START( roldfrog, splash_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)       /* 12 MHz - verified */
	MCFG_CPU_PROGRAM_MAP(roldfrog_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", splash_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_24MHz/8)     /* 3 MHz - verified */
	MCFG_CPU_PROGRAM_MAP(roldfrog_sound_map)
	MCFG_CPU_IO_MAP(roldfrog_sound_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", splash_state,  roldfrog_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(2*8, 48*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(splash_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", splash)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_MACHINE_START_OVERRIDE(splash_state, roldfrog )
	MCFG_MACHINE_RESET_OVERRIDE(splash_state, splash )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_24MHz / 8)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(splash_state, ym_irq))
	MCFG_SOUND_ROUTE(0, "mono", 0.20)
	MCFG_SOUND_ROUTE(1, "mono", 0.20)
	MCFG_SOUND_ROUTE(2, "mono", 0.20)
	MCFG_SOUND_ROUTE(3, "mono", 1.0)
MACHINE_CONFIG_END

WRITE_LINE_MEMBER(splash_state::adpcm_int1)
{
	if (m_snd_interrupt_enable1  || m_msm_toggle1 == 1)
	{
		m_msm1->data_w(m_msm_data1 >> 4);
		m_msm_data1 <<= 4;
		m_msm_toggle1 ^= 1;
		if (m_msm_toggle1 == 0)
		{
			m_msm_source|=1;
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0x38);
		}
	}
}

WRITE_LINE_MEMBER(splash_state::adpcm_int2)
{
	if (m_snd_interrupt_enable2 || m_msm_toggle2 == 1)
	{
		m_msm2->data_w(m_msm_data2 >> 4);
		m_msm_data2 <<= 4;
		m_msm_toggle2 ^= 1;
		if (m_msm_toggle2 == 0)
		{
			m_msm_source|=2;
			m_audiocpu->set_input_line_and_vector(0, HOLD_LINE, 0x38);
		}
	}
}


MACHINE_START_MEMBER(splash_state, funystrp)
{
	save_item(NAME(m_funystrp_val));
	save_item(NAME(m_funystrp_ff3cc7_val));
	save_item(NAME(m_funystrp_ff3cc8_val));
	save_item(NAME(m_msm_data1));
	save_item(NAME(m_msm_data2));
	save_item(NAME(m_msm_toggle1));
	save_item(NAME(m_msm_toggle2));
	save_item(NAME(m_msm_source));
	save_item(NAME(m_snd_interrupt_enable1));
	save_item(NAME(m_snd_interrupt_enable2));
}

static MACHINE_CONFIG_START( funystrp, splash_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)       /* 12 MHz (24/2) */
	MCFG_CPU_PROGRAM_MAP(funystrp_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", splash_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_24MHz/4)     /* 6MHz (24/4) */
	MCFG_CPU_PROGRAM_MAP(funystrp_sound_map)
	MCFG_CPU_IO_MAP(funystrp_sound_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(splash_state, screen_update_funystrp)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", splash)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)

	MCFG_MACHINE_START_OVERRIDE(splash_state, funystrp )
	MCFG_MACHINE_RESET_OVERRIDE(splash_state, funystrp )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("msm1", MSM5205, XTAL_400kHz)
	MCFG_MSM5205_VCLK_CB(WRITELINE(splash_state, adpcm_int1))         /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)  /* 1 / 48 */       /* Sample rate = 400kHz/64 */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("msm2", MSM5205, XTAL_400kHz)
	MCFG_MSM5205_VCLK_CB(WRITELINE(splash_state, adpcm_int2))         /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S96_4B)  /* 1 / 96 */       /* Sample rate = 400kHz/96 */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END


/***************************************************************************

The Return of Lady Frog
Microhard, 1993

PCB Layout
----------


YM2203                            68000
YM3014    6116           **       2   6
          6116          6116      3   7
6264                              4   8
1  Z80           MACH130          5   9
                 681000        6264  6264


DSW2              2148                10
DSW1              2148  6264  30MHz   11
                  2148  6264  24MHz   12
                  2148                13

Notes:
      68000 Clock = >10MHz (my meter can only read up to 10.000MHz)
        Z80 Clock = 3MHz
               ** = possibly PLD (surface is scratched, type PLCC44)
    Vertical Sync = 60Hz
      Horiz. Sync = 15.56kHz





Lady Frog is a typical 'Korean Style' hack of Splash
the main 68k code is based on 'Splash' with many patches made to the code
most of the patched code includes jumps to the 0x400000 where no rom is
mapped, this data appears to be supplied by a protection device.

The z80 rom (used for sound) is a hack of the main program from dynax's
'Dragon Punch' game.

Notes about roldfrog.009 & 9:
  There is 1 bit difference between the two, so one is a bad dump.
  roldfrom.009 0x3A10C == 2E
             9 0x3A10C == 2F

***************************************************************************/

ROM_START( roldfrog )
	ROM_REGION( 0x408000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "roldfrog.002", 0x000000, 0x080000, CRC(724cf022) SHA1(f8cddfb785ae7900cb95b854811ec3fb250fa7fe) )
	ROM_LOAD16_BYTE( "roldfrog.006", 0x000001, 0x080000, CRC(e52a7ae2) SHA1(5c6ecbc2711376afdd7b8da11f84d36ffc464c8a) )
	ROM_LOAD16_BYTE( "roldfrog.003", 0x100000, 0x080000, CRC(a1d49967) SHA1(54d73c1db1090b7d5109906525ce95ee8c00ad1f) )
	ROM_LOAD16_BYTE( "roldfrog.007", 0x100001, 0x080000, CRC(e5805c4e) SHA1(5691807b711ea5137f91afd6033fcd734d2b5ad0) )
	ROM_LOAD16_BYTE( "roldfrog.004", 0x200000, 0x080000, CRC(709281f5) SHA1(01453168df4dc84069346cecd1fba9adeae6fcb8) )
	ROM_LOAD16_BYTE( "roldfrog.008", 0x200001, 0x080000, CRC(39adcba4) SHA1(6c8c945b6383fa2549e6654b427a7ce4c7ff46b5) )
	ROM_LOAD16_BYTE( "roldfrog.005", 0x300000, 0x080000, CRC(b683160c) SHA1(526a772108a6bf71207a7b6de7cbd14f8e9496bc) )
	ROM_LOAD16_BYTE( "roldfrog.009", 0x300001, 0x080000, CRC(e475fb76) SHA1(9ab56db86530647ea4a5d2109a02119710ff9b7e) ) /* 1 bit difference with "9" which is the BAD_DUMP */
	/* 68000 code - supplied by protection device? */
	ROM_LOAD16_WORD_SWAP( "protdata.bin", 0x400000, 0x8000, CRC(ecaa8dd1) SHA1(b15f583d1a96b6b7ce50bcdca8cb28508f92b6a5) )

	ROM_REGION( 0x40000, "audiocpu", 0 )    /* Z80 Code */
	ROM_LOAD( "roldfrog.001", 0x00000, 0x08000, CRC(ba9eb1c6) SHA1(649d1103f3188554eaa3fc87a1f52c53233932b2) )
	ROM_CONTINUE(             0x10000, 0x10000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "roldfrog.010", 0x00000, 0x20000, CRC(51fd0e1a) SHA1(940c4231b21d16c62cad47c22fe735b18662af4a) )
	ROM_LOAD( "roldfrog.011", 0x20000, 0x20000, CRC(610bf6f3) SHA1(04a7efac2e83c6747d4bd480b1f3118eb44a1f76) )
	ROM_LOAD( "roldfrog.012", 0x40000, 0x20000, CRC(466ede67) SHA1(2d44dba1e76e5ceebf33fa6fc148ed665701a7ff) )
	ROM_LOAD( "roldfrog.013", 0x60000, 0x20000, CRC(fad3e8be) SHA1(eccd7b1440d3a0d433c92ff33213326e0d57422a) )
ROM_END

ROM_START( roldfroga )
	ROM_REGION( 0x408000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "roldfrog.002", 0x000000, 0x080000, CRC(724cf022) SHA1(f8cddfb785ae7900cb95b854811ec3fb250fa7fe) )
	ROM_LOAD16_BYTE( "roldfrog.006", 0x000001, 0x080000, CRC(e52a7ae2) SHA1(5c6ecbc2711376afdd7b8da11f84d36ffc464c8a) )
	ROM_LOAD16_BYTE( "roldfrog.003", 0x100000, 0x080000, CRC(a1d49967) SHA1(54d73c1db1090b7d5109906525ce95ee8c00ad1f) )
	ROM_LOAD16_BYTE( "roldfrog.007", 0x100001, 0x080000, CRC(e5805c4e) SHA1(5691807b711ea5137f91afd6033fcd734d2b5ad0) )
	ROM_LOAD16_BYTE( "roldfrog.004", 0x200000, 0x080000, CRC(709281f5) SHA1(01453168df4dc84069346cecd1fba9adeae6fcb8) )
	ROM_LOAD16_BYTE( "roldfrog.008", 0x200001, 0x080000, CRC(39adcba4) SHA1(6c8c945b6383fa2549e6654b427a7ce4c7ff46b5) )
	ROM_LOAD16_BYTE( "roldfrog.005", 0x300000, 0x080000, CRC(b683160c) SHA1(526a772108a6bf71207a7b6de7cbd14f8e9496bc) )
	ROM_LOAD16_BYTE( "9",            0x300001, 0x080000, CRC(fd515b58) SHA1(7926ab9afbc260219351a02b56b82ede883f9aab) ) /* 1 bit difference with "roldfrog.009" which is the BAD_DUMP */
	/* 68000 code - supplied by protection device? */
	ROM_LOAD16_WORD_SWAP( "protdata.bin", 0x400000, 0x8000, CRC(ecaa8dd1) SHA1(b15f583d1a96b6b7ce50bcdca8cb28508f92b6a5) )

	ROM_REGION( 0x90000, "audiocpu", 0 )    /* Z80 Code */
	ROM_LOAD( "roldfrog.001", 0x00000, 0x08000, CRC(ba9eb1c6) SHA1(649d1103f3188554eaa3fc87a1f52c53233932b2) )
	ROM_CONTINUE(             0x10000, 0x10000 )
	ROM_CONTINUE(             0x38000, 0x08000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "roldfrog.010", 0x00000, 0x20000, CRC(51fd0e1a) SHA1(940c4231b21d16c62cad47c22fe735b18662af4a) )
	ROM_LOAD( "roldfrog.011", 0x20000, 0x20000, CRC(610bf6f3) SHA1(04a7efac2e83c6747d4bd480b1f3118eb44a1f76) )
	ROM_LOAD( "roldfrog.012", 0x40000, 0x20000, CRC(466ede67) SHA1(2d44dba1e76e5ceebf33fa6fc148ed665701a7ff) )
	ROM_LOAD( "roldfrog.013", 0x60000, 0x20000, CRC(fad3e8be) SHA1(eccd7b1440d3a0d433c92ff33213326e0d57422a) )
ROM_END

/*
Game    Rebus
Anno    1995
Produttore  Microhard

CPU

1x MC68000P12-2C91E-QQYS9247 (main)(u1)
1x STZ8400BB1-Z80BCPU (sound)(u162)
1x YAMAHA YM3812-9030EALB (sound)(u165)
1x oscillator 24.000000MHz (osc1)
1x oscillator 30.000MHz (osc2)

ROMs

1x S27C512 (sound)(1)
6x M27C4001 (main)(2,3,4,5,6,7)
1x M27C040Q (main)(8)
1x AM27C040 (main)(9)
2x D27C010 (gfx)(10,11)
1x STM27C1001 (gfx)(12)
1x AM27C010 (gfx)(13)

Note

6x MCM2018AN45 (ram)
5x KM681000ALP (ram)
1x HY18CV8S (PEEL)
5x GAL16V8
2x GAL16V8H
1x PALCE20V8H
001-937
*/

ROM_START( rebus )
	ROM_REGION( 0x408000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "2.u16", 0x000000, 0x080000, CRC(7c8a717f) SHA1(00b1e7986046a7705fc65a5c7d4701a002b2ea6f) )
	ROM_LOAD16_BYTE( "6.u12", 0x000001, 0x080000, CRC(8f73d548) SHA1(210d95dc0db41da3252a09e598719d98bca41983) )
	ROM_LOAD16_BYTE( "3.u17", 0x100000, 0x080000, CRC(7495409b) SHA1(b4d75713d31c0b01d7cb7d50a2a89fb3ea4ea42b) )
	ROM_LOAD16_BYTE( "7.u13", 0x100001, 0x080000, CRC(615bc4dc) SHA1(f1c8ee3eb8a48721f1f2e4f35fdc9bb0bb9d167b) )
	ROM_LOAD16_BYTE( "4.u18", 0x200000, 0x080000, CRC(c27674fc) SHA1(06f3f1543331bd00f08cde51beb73934f2f1e6c8) )
	ROM_LOAD16_BYTE( "8.u14", 0x200001, 0x080000, CRC(f0c04b7e) SHA1(92117b05666afa42d7669e5aa630e5143fac1d74) )
	ROM_LOAD16_BYTE( "5.u19", 0x300000, 0x080000, CRC(2702f341) SHA1(de862cacbb3e8e322128315d87a22c7cdfe4fcb9) )
	ROM_LOAD16_BYTE( "9.u15", 0x300001, 0x080000, CRC(f5ae3d73) SHA1(6a9d955023c704023b722cf863ba19ccb9b34ee1) )

	ROM_REGION( 0x90000, "audiocpu", 0 )    /* Z80 Code */
	ROM_LOAD( "1.u163", 0x00000, 0x10000,  CRC(88a7b1f8) SHA1(b34fa26dbc613bf3b525d19df90fa3ba4efb6e5d) )
	ROM_RELOAD(               0x20000, 0x10000 )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "10.u102", 0x00000, 0x20000, CRC(6f75a28b) SHA1(75f0bd6bd8c04ea9f832c22fbe1d17b0351f1102) )
	ROM_LOAD( "11.u103", 0x20000, 0x20000, CRC(0af65b78) SHA1(9522ad17d26d866e5b11b4fec47781a00a297977) )
	ROM_LOAD( "12.u104", 0x40000, 0x20000, CRC(3ed6ce19) SHA1(0d574071053157e4ef973a844795e48ec69dc7c4) )
	ROM_LOAD( "13.u105", 0x60000, 0x20000, CRC(8b54553d) SHA1(5cb776e551527b0e717fe0d76296f5f895523de5) )
ROM_END


ROM_START( splash )
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code + gfx */
	ROM_LOAD16_BYTE( "4g", 0x000000, 0x020000, CRC(b38fda40) SHA1(37ddf4b6f9f2f6cc58efefc277bc3ae9dc71e6d0) )
	ROM_LOAD16_BYTE( "4i", 0x000001, 0x020000, CRC(02359c47) SHA1(6817424b2b1afffa99cec5b8fae4fb8436db2bb5) )
	ROM_LOAD16_BYTE( "5g", 0x100000, 0x080000, CRC(a4e8ed18) SHA1(64ce47193ee4bb3a8014d7c14c559b4ebb3af083) )
	ROM_LOAD16_BYTE( "5i", 0x100001, 0x080000, CRC(73e1154d) SHA1(2c055ad29a32c6c1e712cc35b5972f1e69cdebb7) )
	ROM_LOAD16_BYTE( "6g", 0x200000, 0x080000, CRC(ffd56771) SHA1(35ad9874b6ea5aa3ba38a31d723093b4dd2cfdb8) )
	ROM_LOAD16_BYTE( "6i", 0x200001, 0x080000, CRC(16e9170c) SHA1(96fc237cb172039df153dc70d15ed7d9ee750363) )
	ROM_LOAD16_BYTE( "8g", 0x300000, 0x080000, CRC(dc3a3172) SHA1(2b322b52e3e8da00f26dd276cb72bd2d48c2deaa) )
	ROM_LOAD16_BYTE( "8i", 0x300001, 0x080000, CRC(2e23e6c3) SHA1(baf9ab4c3261c3f06f5e43c1e50aba9222acb71d) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   /* Z80 code + sound data */
	ROM_LOAD( "5c", 0x00000, 0x10000, CRC(0ed7ebc9) SHA1(28ef16e20d754deef49be6a5c9f63311e9ec94a3) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "18i", 0x000000, 0x020000, CRC(028a4a68) SHA1(19384988e3690886ed55886ecdc4e4c566dbe4ba) )
	ROM_LOAD( "15i", 0x020000, 0x020000, CRC(2a8cb830) SHA1(bc54dfb03fade154085aa2f66784e07664a7a3d8) )
	ROM_LOAD( "16i", 0x040000, 0x020000, CRC(21aeff2c) SHA1(0c307e94f4a814c674ba0ab471a6bdd57e43c265) )
	ROM_LOAD( "13i", 0x060000, 0x020000, CRC(febb9893) SHA1(bb607a608c6c1658748a17a62431e8c30323c7ec) )
ROM_END

ROM_START( splash10 )
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code + gfx */
	ROM_LOAD16_BYTE( "splash10.g4", 0x000000, 0x020000, CRC(38ba6632) SHA1(ca1425120fcb427e1b2c83eb3bf104363d9571be) )
	ROM_LOAD16_BYTE( "splash10.i4", 0x000001, 0x020000, CRC(0edc3373) SHA1(edf28baa6ef2442a37eb81a51ab66485d89f802e) )
	ROM_LOAD16_BYTE( "5g",          0x100000, 0x080000, CRC(a4e8ed18) SHA1(64ce47193ee4bb3a8014d7c14c559b4ebb3af083) )
	ROM_LOAD16_BYTE( "5i",          0x100001, 0x080000, CRC(73e1154d) SHA1(2c055ad29a32c6c1e712cc35b5972f1e69cdebb7) )
	ROM_LOAD16_BYTE( "6g",          0x200000, 0x080000, CRC(ffd56771) SHA1(35ad9874b6ea5aa3ba38a31d723093b4dd2cfdb8) )
	ROM_LOAD16_BYTE( "6i",          0x200001, 0x080000, CRC(16e9170c) SHA1(96fc237cb172039df153dc70d15ed7d9ee750363) )
	ROM_LOAD16_BYTE( "8g",          0x300000, 0x080000, CRC(dc3a3172) SHA1(2b322b52e3e8da00f26dd276cb72bd2d48c2deaa) )
	ROM_LOAD16_BYTE( "8i",          0x300001, 0x080000, CRC(2e23e6c3) SHA1(baf9ab4c3261c3f06f5e43c1e50aba9222acb71d) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   /* Z80 code + sound data */
	ROM_LOAD( "5c", 0x00000, 0x10000, CRC(0ed7ebc9) SHA1(28ef16e20d754deef49be6a5c9f63311e9ec94a3) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "18i", 0x000000, 0x020000, CRC(028a4a68) SHA1(19384988e3690886ed55886ecdc4e4c566dbe4ba) )
	ROM_LOAD( "15i", 0x020000, 0x020000, CRC(2a8cb830) SHA1(bc54dfb03fade154085aa2f66784e07664a7a3d8) )
	ROM_LOAD( "16i", 0x040000, 0x020000, CRC(21aeff2c) SHA1(0c307e94f4a814c674ba0ab471a6bdd57e43c265) )
	ROM_LOAD( "13i", 0x060000, 0x020000, CRC(febb9893) SHA1(bb607a608c6c1658748a17a62431e8c30323c7ec) )
ROM_END

/***************************************************************************

Painted Lady (US, version 1.3)
(Splash! alternative title)
Gaelco, 1992

PCB Layout
----------

REF. 922704
|------------------------------------------------|
|       384kHz                 |----------------||
|       M5205        24MHz     |     68000      ||
|YM3014 YM3812             PAL |                ||
|       6116    PAL  30MHz     |----------------||
|       1.5C                                     |
|       Z80B                    2.4G       6.4I  |
|J                              3.5G       7.5I  |
|A                              4.6G       8.6I  |
|M         6116                 5.8G       9.8I  |
|M         6116                 6264       6264  |
|A                    6116  |----------|         |
|  DSW2               6116  |          |         |
|                           |TPC1020AFN|         |
|        KM681000           |-084C     |  10.13I |
|                           |          |  11.15I |
|  DSW1                     |----------|  12.16I |
|                                  6264   13.18I |
|                     6116                       |
|------------------------------------------------|
Notes:
      68000 clock : 12MHz [24/2)
      Z80 clock   : 3.75MHz [30/8]
      M5205 clock : 384kHz, sample rate = 384000/48
      YM3812 clock: 3.75MHz [30/8]
      6116        : 2k x8 SRAM
      6264        : 8k x8 SRAM
      KM681000    : 128k x8 SRAM
      VSync       : 58Hz

* Found a Splash! PCB (possibly bootleg) with OSCs of 25.175MHz & 32MHz

***************************************************************************/

ROM_START( paintlad )
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code + gfx */
	ROM_LOAD16_BYTE( "2.4g", 0x000000, 0x020000, CRC(cd00864a) SHA1(24cbcf43b7237d1e5374a684aac89dad7e7bb75b) )
	ROM_LOAD16_BYTE( "6.4i", 0x000001, 0x020000, CRC(0f19d830) SHA1(3bfb4c98c87f0bf8d9dc7c7f468e1c58b16356e5) )
	ROM_LOAD16_BYTE( "5g",   0x100000, 0x080000, CRC(a4e8ed18) SHA1(64ce47193ee4bb3a8014d7c14c559b4ebb3af083) )
	ROM_LOAD16_BYTE( "5i",   0x100001, 0x080000, CRC(73e1154d) SHA1(2c055ad29a32c6c1e712cc35b5972f1e69cdebb7) )
	ROM_LOAD16_BYTE( "6g",   0x200000, 0x080000, CRC(ffd56771) SHA1(35ad9874b6ea5aa3ba38a31d723093b4dd2cfdb8) )
	ROM_LOAD16_BYTE( "6i",   0x200001, 0x080000, CRC(16e9170c) SHA1(96fc237cb172039df153dc70d15ed7d9ee750363) )
	ROM_LOAD16_BYTE( "8g",   0x300000, 0x080000, CRC(dc3a3172) SHA1(2b322b52e3e8da00f26dd276cb72bd2d48c2deaa) )
	ROM_LOAD16_BYTE( "8i",   0x300001, 0x080000, CRC(2e23e6c3) SHA1(baf9ab4c3261c3f06f5e43c1e50aba9222acb71d) )

	ROM_REGION( 0x010000, "audiocpu", 0 )   /* Z80 code + sound data */
	ROM_LOAD( "5c", 0x00000, 0x10000, CRC(0ed7ebc9) SHA1(28ef16e20d754deef49be6a5c9f63311e9ec94a3) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "13.18i", 0x000000, 0x020000, CRC(262ee31f) SHA1(1756dfd482c3e889df393d37a5c680aa283702ee) )
	ROM_LOAD( "11.15i", 0x020000, 0x020000, CRC(6e4d598f) SHA1(b5b0d65c50ec469b5ffcd6187ca3aacddd97a477) )
	ROM_LOAD( "12.16i", 0x040000, 0x020000, CRC(15761eb5) SHA1(61a47dad0e70ff4f1ae7f56ee529d2987eab1997) )
	ROM_LOAD( "10.13i", 0x060000, 0x020000, CRC(92a0eff8) SHA1(e27a73791d499b0449251ea0678d9a34040e9883) )
ROM_END

/*

Funny Strip
Microhard, 199?

PCB Layout
----------

|-------------------------------------|
|5205(1) 400kHz    30MHz **    68000  |
|5205(2) 400kHz    24MHz   PAL 12.U87 |
|     11.U130  2018            13.U111|
|J    10.U118                *   62256|
|A 93C46 Z80  PAL                62256|
|M    2148                  PAL       |
|M    2148                  PAL       |
|A    2148  2148            PAL       |
|  PAL                                |
|            PAL                      |
|            PAL                      |
|           6264   2018               |
|DSW1  PAL  6264   2018               |
|                       16.U53  14.U51|
|DSW2  2018         17.U54  15.U52    |
|-------------------------------------|
Notes:
      68000 clock    : 12.000MHz (24/2)
        Z80 clock    : 6.000MHz (24/4)
      M5205(1) clock : 400kHz. Sample Rate = 400000 / 96
      M5205(2) clock : 400kHz. Sample Rate = 400000 / 64
                  ** : possibly FPGA (surface is scratched, type PLCC84)
                   * : possibly CPLD or MCU (surface is scratched, type PLCC52)
       Vertical Sync : 60Hz

*/

ROM_START( funystrp )
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code + gfx */
	ROM_LOAD16_BYTE( "12.u87",  0x000000, 0x010000, CRC(4ac173f3) SHA1(c211bc8528d26d5a96fce4b0ebfddf2aa6a257ef) )
	ROM_LOAD16_BYTE( "13.u111", 0x000001, 0x010000, CRC(1358c60c) SHA1(7142aa6f94cfdfb1b70b37742201b2c213f85137) )

	ROM_REGION( 0x080000, "audiocpu", 0 )   /* Z80 code + sound data */
	ROM_LOAD( "11.u130", 0x000000, 0x040000, CRC(e969ea2b) SHA1(54d5bb59e9909a6b7e66764f91e2f98f8f8832c5) )
	ROM_LOAD( "10.u118", 0x040000, 0x040000, CRC(0894b936) SHA1(cd01eb86e403e20c56492185ecd9bb0f4f27867a) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "17.u54", 0x000000, 0x020000, CRC(e72fd9e9) SHA1(d433e082c27e7c0a1f24cd25a803864834e9affe) )
	ROM_LOAD( "16.u53", 0x020000, 0x020000, CRC(cc793c1c) SHA1(d0798b07d6e6449ad6fa84a95181ec5c75647277) )
	ROM_LOAD( "15.u52", 0x040000, 0x020000, CRC(60f8f305) SHA1(7223c8e02c3d1cc573843c572d286b469ff6f33b) )
	ROM_LOAD( "14.u51", 0x060000, 0x020000, CRC(ed565a0b) SHA1(50789e0f04038d174b5529546c1ff430416b32d6) )
ROM_END

ROM_START( puckpepl )
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code + gfx */
	ROM_LOAD16_BYTE( "pp22.u87", 0x000000, 0x010000, CRC(1ceb522d) SHA1(216cd24bc2cc3fbd389ab96bc8b729c4d919faab) )
	ROM_LOAD16_BYTE( "pp23.111", 0x000001, 0x010000, CRC(84336569) SHA1(4358c48bf65dfdb6f52326ec5f026e1b9614a108) )

	ROM_REGION( 0x080000, "audiocpu", 0 )   /* Z80 code + sound data */
	ROM_LOAD( "pp31.130", 0x000000, 0x040000, CRC(9b6c302f) SHA1(349e5cf16dd2e8b6c0f56ca1b9ce81475c442435) )
	ROM_LOAD( "pp30.181", 0x040000, 0x040000, CRC(a5697b3d) SHA1(28ef3cfea82b3016c7c042a18509ba2bf83048e5) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "pp37.u54", 0x000000, 0x020000, CRC(23c136b0) SHA1(bf3a6379934e205dcb6c0389d9dd5d0ffb92bdd4) )
	ROM_LOAD( "pp36.u53", 0x020000, 0x020000, CRC(210e7579) SHA1(1a2d175a83f5ad933961a3c8b26cd8786527ea7f) )
	ROM_LOAD( "pp35.u52", 0x040000, 0x020000, CRC(cd4cc5df) SHA1(b4dd9e97bdad46340e79e9c7747484aae49680a2) )
	ROM_LOAD( "pp34.u51", 0x060000, 0x020000, CRC(93f2d483) SHA1(eb6981b0228acb1ec92325924d0aa295f9e2cfe1) )
ROM_END


/* DRIVER INITs */

DRIVER_INIT_MEMBER(splash_state,splash)
{
	m_bitmap_type = 0;
	m_sprite_attr2_shift = 8;
}

DRIVER_INIT_MEMBER(splash_state,splash10)
{
	m_bitmap_type = 0;
	m_sprite_attr2_shift = 0;
}

DRIVER_INIT_MEMBER(splash_state,roldfrog)
{
	UINT8 * ROM = (UINT8 *)memregion("audiocpu")->base();
	membank("sound_bank")->configure_entries(0, 16, &ROM[0x10000], 0x8000);

	m_bitmap_type = 1;
	m_sprite_attr2_shift = 8;
}

DRIVER_INIT_MEMBER(splash_state,rebus)
{
	UINT16 *ROM = (UINT16 *)memregion("maincpu")->base();

	m_bitmap_type = 1;
	m_sprite_attr2_shift = 0;

	//d1 clear , regs restore and rte - end of trap $b
	ROM[0x196c0/2] = 0x7200;
	ROM[0x196c2/2] = 0x4cdf;
	ROM[0x196c4/2] = 0x7080;
	ROM[0x196c6/2] = 0x4e73;

	//jumps to above
	ROM[0x3ffcac/2] = 0x4ef9;
	ROM[0x3ffcae/2] = 0x0001;
	ROM[0x3ffcb0/2] = 0x96c0;

	//rom checksum
	ROM[0x3ff2fc/2] = 0x4e71;
	ROM[0x3ff2fe/2] = 0x4e71;
	ROM[0x3ff300/2] = 0x4e71;
	ROM[0x3ff302/2] = 0x4e71;
	ROM[0x3ff304/2] = 0x4e71;
	ROM[0x3ff306/2] = 0x4e71;
}




READ16_MEMBER(splash_state::funystrp_protection_r)
{
	int pc = space.device().safe_pc();

	int ofst = offset+(0x100000/2);

//  logerror ("PROTR: %5.5x, pc: %5.5x\n", ofst*2, pc);

	// don't interfere with ram check.
	if (pc == 0x04770) return 0x00;
	if (pc == 0x04788) return 0x55;

	switch (ofst)
	{
		//-----------------------------------------------------------------
		// sub $7ACC, $C7EE, subtractions, original value from 68k

		case ((0x107001 / 2) + 0x0030): // $7ACE
			m_funystrp_val = m_funystrp_ff3cc7_val & 0x7f;
			return 0;

		case ((0x107001 / 2) + 0x013e): // $7AFC
			return (m_funystrp_val + 0x13) & 0xff;

		case ((0x107001 / 2) + 0x0279): // $7B38
			return (m_funystrp_val + 0x22) & 0xff;

		case ((0x107001 / 2) + 0x0357): // $7B6E
			return (m_funystrp_val + 0x44) & 0xff;

		case ((0x107001 / 2) + 0x03b1): // $7BA4
			return (m_funystrp_val + 0x6a) & 0xff;

		//-----------------------------------------------------------------
		// sub $7E76, subtractions, original value from protection device

		case ((0x110001 / 2) + 0x0013): // $7E80
			m_funystrp_val = 0;
			return 0;

		case ((0x110001 / 2) + 0x0125): // $7E96
			return (m_funystrp_val + 0x03) & 0xff;

		case ((0x110001 / 2) + 0x0261): // $7ECE
			return (m_funystrp_val + 0x08) & 0xff;

		case ((0x110001 / 2) + 0x0322): // $7F00
			return (m_funystrp_val + 0x12) & 0xff;

		case ((0x110001 / 2) + 0x039b): // $7F36
			return (m_funystrp_val + 0x70) & 0xff;

		//-----------------------------------------------------------------
		// sub $7F70, $8038, $116E2, no subtractions, straight compare!, original value from 68k
		// increase ff3cc8 value in sub $116e2

		case ((0x100001 / 2) + 0x0010): // $7F72
			m_funystrp_val = m_funystrp_ff3cc8_val;
			return 0;

		case ((0x100001 / 2) + 0x0123): // $7F9A
			return (m_funystrp_val + 0x00) & 0xff;

		case ((0x100001 / 2) + 0x0257): // $7FC4
			return (m_funystrp_val + 0x00) & 0xff;

		case ((0x100001 / 2) + 0x0312): // $7FEA
			return (m_funystrp_val + 0x00) & 0xff;

		case ((0x100001 / 2) + 0x0395): // $8010
			// increment $ff3cc8 in $117A8
			return (m_funystrp_val + 0x00) & 0xff;

		//-----------------------------------------------------------------
		// sub $8522, subtractions, original value from protection device, weird cases

		case ((0x104801 / 2) + 0x013A): // $8524
			m_funystrp_val = 0;
			return 0;

		// this and above usually swapped... fooling the lazy bootlegger?
		case ((0x104801 / 2) + 0x0017): // $8542
			return (m_funystrp_val + 0x12) & 0xff;

		// first case... weird?
	//  case ((0x104801 / 2) + 0x013A): // $857E
	//      return (m_funystrp_val + 0x00) & 0xff;

		case ((0x104801 / 2) + 0x0277): // $85A4
			return (m_funystrp_val + 0x04) & 0xff;

		case ((0x104801 / 2) + 0x034b): // $85D6
			return (m_funystrp_val + 0x37) & 0xff;

		case ((0x104801 / 2) + 0x03ac): // $860E
			return (m_funystrp_val + 0x77) & 0xff;

		//-----------------------------------------------------------------
		// sub $88F8, subtractions, original value from protection device
		// verified as working!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		case ((0x127001 / 2) + 0x0045): // $88FA
			m_funystrp_val = 0;
			return 0;

		case ((0x127001 / 2) + 0x0145): // $8918
			return (m_funystrp_val + 0x01) & 0xff;

		case ((0x127001 / 2) + 0x028B): // $894A
			return (m_funystrp_val + 0x02) & 0xff;

		case ((0x127001 / 2) + 0x0363): // $8982
			return (m_funystrp_val + 0x03) & 0xff;

		case ((0x127001 / 2) + 0x03BA): // $89B4
			return (m_funystrp_val + 0x00) & 0xff;

		//-----------------------------------------------------------------
		// sub $9DD2, subtractions, original value from protection device

		case ((0x170001 / 2) + 0x006B): // $9DD4
			m_funystrp_val = 0;
			return 0;

		case ((0x170001 / 2) + 0x0162): // $9DF2
			return (m_funystrp_val + 0x00) & 0xff;

		case ((0x170001 / 2) + 0x02A7): // $9E1E
			return (m_funystrp_val + 0x7c) & 0xff;

		case ((0x170001 / 2) + 0x0381): // $9E54
			return (m_funystrp_val + 0x30) & 0xff;

		case ((0x170001 / 2) + 0x03C7): // $9E8A
			return (m_funystrp_val + 0x28) & 0xff;

		//-----------------------------------------------------------------
		// sub $A944, subtractions, original value from protection device
		// verified as working!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		case ((0x177001 / 2) + 0x0079): // $A946
			m_funystrp_val = 0;
			return 0;

		case ((0x177001 / 2) + 0x01A0): // $A964
			return (m_funystrp_val + 0x02) & 0xff;

		case ((0x177001 / 2) + 0x02B2): // $A99C
			return (m_funystrp_val + 0x04) & 0xff;

		case ((0x177001 / 2) + 0x039A): // $A9CE
			return (m_funystrp_val + 0x25) & 0xff;

		case ((0x177001 / 2) + 0x03D3): // $AA04
			return (m_funystrp_val + 0x16) & 0xff;

		//-----------------------------------------------------------------
		// sub $C5E4, subtractions, original value from 68k

	//  these cases are already in sub $7ACC, last one is new!!
	//  case ((0x107001 / 2) + 0x0030): // $7ACE
	//      m_funystrp_val = m_funystrp_ff3cc7_val & 0x7f;
	//      return 0;

	//  case ((0x107001 / 2) + 0x013e): // $7AFC
	//      return (m_funystrp_val + 0x13) & 0xff;

	//  case ((0x107001 / 2) + 0x0279): // $7B38
	//      return (m_funystrp_val + 0x22) & 0xff;

	//  case ((0x107001 / 2) + 0x0357): // $7B6E
	//      return (m_funystrp_val + 0x44) & 0xff;

		case ((0x107001 / 2) + 0x0381): // $7BA4
			return (m_funystrp_val + 0x6a) & 0xff;

		//-----------------------------------------------------------------
		// sub $DBCE, subtractions, original value from protection device

		case ((0x140001 / 2) + 0x0052): // $DBD0
			m_funystrp_val = 0;
			return 0;

		case ((0x140001 / 2) + 0x015C): // $DBEE
			return (m_funystrp_val + 0x15) & 0xff;

		case ((0x140001 / 2) + 0x0293): // $DC2A
			return (m_funystrp_val + 0x03) & 0xff;

		case ((0x140001 / 2) + 0x0374): // $DC5C
			return (m_funystrp_val + 0x55) & 0xff;

		case ((0x140001 / 2) + 0x03C0): // $DC92
			return (m_funystrp_val + 0x44) & 0xff;

		//-----------------------------------------------------------------
		// sub $F72C, subtractions, original value from protection device,
		// routine verified working!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		case ((0x100001 / 2) + 0x0017): // $F72E
			m_funystrp_val = 0;
			return 0;

		case ((0x100001 / 2) + 0x0127): // $F74C
			return (m_funystrp_val + 0x17) & 0xff;

		case ((0x110001 / 2) + 0x0263): // $F788
			return (m_funystrp_val + 0x0f) & 0xff;

		case ((0x110001 / 2) + 0x0324): // $F7BE
			return (m_funystrp_val + 0x12) & 0xff;

		case ((0x110001 / 2) + 0x0399): // $F7F4
			return (m_funystrp_val + 0x70) & 0xff;

		//-----------------------------------------------------------------
		// sub $F82E, subtractions, original value from protection device,

		case ((0x100001 / 2) + 0x0013): // $F830
			m_funystrp_val = 0;
			return 0;

		case ((0x100001 / 2) + 0x0125): // $F84E
			return (m_funystrp_val + 0x17) & 0xff;

	//  used in sub $7E76
	//  case ((0x110001 / 2) + 0x0261): // $F88A
	//      return (m_funystrp_val + 0x0f) & 0xff;

	//  case ((0x110001 / 2) + 0x0322): // $F8C0
	//      return (m_funystrp_val + 0x12) & 0xff;

	//  case ((0x110001 / 2) + 0x039B): // $F8F6
	//      return (m_funystrp_val + 0x70) & 0xff;

		//-----------------------------------------------------------------
		// sub $10FE2, subtractions, original value from protection device
		// routine is different from rest, unoptimized or just poorly coded?
		// examine later to verify this is right

		case ((0x105001 / 2) + 0x0021): // $10FF6
			m_funystrp_val = 0;
			return 0;

		case ((0x105001 / 2) + 0x0131): // $1100C
			return (m_funystrp_val + 0x51) & 0xff;

		case ((0x105001 / 2) + 0x026a): // $11038
			return (m_funystrp_val + 0x22) & 0xff;

		case ((0x105001 / 2) + 0x0331): // $11060
			return (m_funystrp_val + 0x00) & 0xff;

		case ((0x105001 / 2) + 0x03ab): // $11078
			return (m_funystrp_val + 0x03) & 0xff;

		//-----------------------------------------------------------------
		// sub $11F2C, subtractions, original value from protection device,
		// routine is different from rest, unoptimized or just poorly coded?
		// examine later to verify this is right

		case ((0x183001 / 2) + 0x0088): // $11F3C
			m_funystrp_val = 0;
			return 0;

		case ((0x183001 / 2) + 0x01A7): // $11F5A
			return (m_funystrp_val + 0x09) & 0xff;

		case ((0x183001 / 2) + 0x02C4): // $11F86
			return (m_funystrp_val + 0x01) & 0xff;

		case ((0x183001 / 2) + 0x03B3): // $11FAA
			return (m_funystrp_val + 0x63) & 0xff;

		case ((0x183001 / 2) + 0x03E9): // $11FD2
			return (m_funystrp_val + 0x65) & 0xff;
	}

	return 0;
}

WRITE16_MEMBER(splash_state::funystrp_protection_w)
{
	int ofst = (0x100000/2)+offset;

//  logerror ("PROTW: %5.5x, %4.4x, PC: %5.5x m: %4.4x\n", ofst*2, data, space.device().safe_pc(), mem_mask);

//  if (ACCESSING_BITS_0_7) // ??
	{
		switch (ofst)
		{
			case (0x100000/2):
				// on boot?
			return;

			case (0x1007e1/2):
			case (0x1007e3/2):
			// counters written here... ??
			return;

			case (0x1007e5/2):
				m_funystrp_ff3cc8_val = data;
			return;

			case (0x1007e7/2):
				m_funystrp_ff3cc7_val = data;
			return;
		}
	}
}

DRIVER_INIT_MEMBER(splash_state,funystrp)
{
	m_bitmap_type = 0;
	m_sprite_attr2_shift = 0;

	UINT16 *ROM = (UINT16 *)memregion("audiocpu")->base();

	membank("sound_bank")->configure_entries(0, 16, &ROM[0x00000], 0x8000);

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x100000, 0x1fffff, write16_delegate(FUNC(splash_state::funystrp_protection_w), this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x100000, 0x1fffff, read16_delegate(FUNC(splash_state::funystrp_protection_r),this));
}

GAME( 1992, splash,   0,        splash,   splash, splash_state,   splash,   ROT0, "Gaelco / OMK Software", "Splash! (Ver. 1.2 World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, splash10, splash,   splash,   splash, splash_state,   splash10, ROT0, "Gaelco / OMK Software", "Splash! (Ver. 1.0 World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, paintlad, splash,   splash,   splash, splash_state,   splash,   ROT0, "Gaelco / OMK Software", "Painted Lady (Splash) (Ver. 1.3 US)", MACHINE_SUPPORTS_SAVE )

GAME( 1993, roldfrog, 0,        roldfrog, splash, splash_state,   roldfrog, ROT0, "Microhard", "The Return of Lady Frog (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, roldfroga,roldfrog, roldfrog, splash, splash_state,   roldfrog, ROT0, "Microhard", "The Return of Lady Frog (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, rebus,    0,        roldfrog, splash, splash_state,   rebus,    ROT0, "Microhard", "Rebus", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 199?, funystrp, 0,        funystrp, funystrp, splash_state, funystrp, ROT0, "Microhard / MagicGames", "Funny Strip", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 199?, puckpepl, funystrp, funystrp, funystrp, splash_state, funystrp, ROT0, "Microhard", "Puck People", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
