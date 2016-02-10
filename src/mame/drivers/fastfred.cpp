// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

  TODO:
  - remove protection hack (protection may be done by the 'H2' chip on the pcb)

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/fastfred.h"
#include "sound/ay8910.h"


void fastfred_state::machine_start()
{
	save_item(NAME(m_charbank));
	save_item(NAME(m_colorbank));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_sound_nmi_mask));
}

// This routine is a big hack, but the only way I can get the game working
// without knowing anything about the way the protection chip works.
// These values were derived based on disassembly of the code. Usually, it
// was pretty obvious what the values should be. Of course, this will have
// to change if a different ROM set ever surfaces.
READ8_MEMBER(fastfred_state::fastfred_custom_io_r)
{
	switch (space.device().safe_pc())
	{
	case 0x03c0: return 0x9d;
	case 0x03e6: return 0x9f;
	case 0x0407: return 0x00;
	case 0x0446: return 0x94;
	case 0x049f: return 0x01;
	case 0x04b1: return 0x00;
	case 0x0dd2: return 0x00;
	case 0x0de4: return 0x20;
	case 0x122b: return 0x10;
	case 0x123d: return 0x00;
	case 0x1a83: return 0x10;
	case 0x1a93: return 0x00;
	case 0x1b26: return 0x00;
	case 0x1b37: return 0x80;
	case 0x2491: return 0x10;
	case 0x24a2: return 0x00;
	case 0x46ce: return 0x20;
	case 0x46df: return 0x00;
	case 0x7b18: return 0x01;
	case 0x7b29: return 0x00;
	case 0x7b47: return 0x00;
	case 0x7b58: return 0x20;
	}

	logerror("Uncaught custom I/O read %04X at %04X\n", 0xc800+offset, space.device().safe_pc());
	return 0x00;
}

READ8_MEMBER(fastfred_state::flyboy_custom1_io_r)
{
	switch (space.device().safe_pc())
	{
		case 0x049d: return 0xad;   /* compare */
		case 0x04b9:            /* compare with 0x9e ??? When ??? */
		case 0x0563: return 0x03;   /* $c085 compare - starts game */
		case 0x069b: return 0x69;   /* $c086 compare         */
		case 0x076b: return 0xbb;   /* $c087 compare         */
		case 0x0852: return 0xd9;   /* $c096 compare         */
		case 0x09d5: return 0xa4;   /* $c099 compare         */
		case 0x0a83: return 0xa4;   /* $c099 compare         */
		case 0x1028:            /* $c08a  bit 0  compare */
		case 0x1051:            /* $c08a  bit 3  compare */
		case 0x107d:            /* $c08c  bit 5  compare */
		case 0x10a7:            /* $c08e  bit 1  compare */
		case 0x10d0:            /* $c08d  bit 2  compare */
		case 0x10f6:            /* $c090  bit 0  compare */
		case 0x3fb6:            /* lddr */

		return 0x00;
	}

	logerror("Uncaught custom I/O read %04X at %04X\n", 0xc085+offset, space.device().safe_pc());
	return 0x00;
}

READ8_MEMBER(fastfred_state::flyboy_custom2_io_r)
{
	switch (space.device().safe_pc())
	{
		case 0x0395: return 0xf7;   /* $C900 compare         */
		case 0x03f5:            /* $c8fd                 */
		case 0x043d:            /* $c8fd                 */
		case 0x0471:            /* $c900                 */
		case 0x1031: return 0x01;   /* $c8fe  bit 0  compare */
		case 0x1068: return 0x04;   /* $c8fe  bit 2  compare */
		case 0x1093: return 0x20;   /* $c8fe  bit 5  compare */
		case 0x10bd: return 0x80;   /* $c8fb  bit 7  compare */
		case 0x103f:            /* $c8fe                 */
		case 0x10e4:            /* $c900                 */
		case 0x110a:            /* $c900                 */
		case 0x3fc8:            /* ld a with c8fc-c900   */

		return 0x00;
	}

	logerror("Uncaught custom I/O read %04X at %04X\n", 0xc8fb+offset, space.device().safe_pc());
	return 0x00;
}


READ8_MEMBER(fastfred_state::jumpcoas_custom_io_r)
{
	if (offset == 0x100)  return 0x63;

	return 0x00;
}

READ8_MEMBER(fastfred_state::boggy84_custom_io_r)
{
	if (offset == 0x100)  return 0x6a;

	return 0x00;
}

/*
    Imago sprites DMA
*/


MACHINE_START_MEMBER(fastfred_state,imago)
{
	machine_start();
	m_gfxdecode->gfx(1)->set_source(m_imago_sprites);
}

WRITE8_MEMBER(fastfred_state::imago_dma_irq_w)
{
	m_maincpu->set_input_line(0, data & 1 ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(fastfred_state::imago_sprites_bank_w)
{
	m_imago_sprites_bank = (data & 2) >> 1;
}

WRITE8_MEMBER(fastfred_state::imago_sprites_dma_w)
{
	UINT8 *rom = (UINT8 *)memregion("gfx2")->base();
	UINT8 sprites_data;

	sprites_data = rom[m_imago_sprites_address + 0x2000*0 + m_imago_sprites_bank * 0x1000];
	m_imago_sprites[offset + 0x800*0] = sprites_data;

	sprites_data = rom[m_imago_sprites_address + 0x2000*1 + m_imago_sprites_bank * 0x1000];
	m_imago_sprites[offset + 0x800*1] = sprites_data;

	sprites_data = rom[m_imago_sprites_address + 0x2000*2 + m_imago_sprites_bank * 0x1000];
	m_imago_sprites[offset + 0x800*2] = sprites_data;

	m_gfxdecode->gfx(1)->mark_dirty(offset/32);
}

READ8_MEMBER(fastfred_state::imago_sprites_offset_r)
{
	m_imago_sprites_address = offset;
	return 0xff; //not really used
}

WRITE8_MEMBER(fastfred_state::nmi_mask_w)
{
	m_nmi_mask = data & 1;
}

WRITE8_MEMBER(fastfred_state::sound_nmi_mask_w)
{
	m_sound_nmi_mask = data & 1;
}

static ADDRESS_MAP_START( fastfred_map, AS_PROGRAM, 8, fastfred_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd3ff) AM_MIRROR(0x400) AM_RAM_WRITE(fastfred_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xd83f) AM_RAM_WRITE(fastfred_attributes_w) AM_SHARE("attributesram")
	AM_RANGE(0xd840, 0xd85f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd860, 0xdbff) AM_RAM // Unused, but initialized
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("BUTTONS") AM_WRITEONLY AM_SHARE("bgcolor")
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("JOYS")
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("DSW") AM_WRITENOP
	AM_RANGE(0xf001, 0xf001) AM_WRITE(nmi_mask_w)
	AM_RANGE(0xf002, 0xf002) AM_WRITE(fastfred_colorbank1_w)
	AM_RANGE(0xf003, 0xf003) AM_WRITE(fastfred_colorbank2_w)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(fastfred_charbank1_w)
	AM_RANGE(0xf005, 0xf005) AM_WRITE(fastfred_charbank2_w)
	AM_RANGE(0xf006, 0xf006) AM_WRITE(fastfred_flip_screen_x_w)
	AM_RANGE(0xf007, 0xf007) AM_WRITE(fastfred_flip_screen_y_w)
	AM_RANGE(0xf116, 0xf116) AM_WRITE(fastfred_flip_screen_x_w)
	AM_RANGE(0xf117, 0xf117) AM_WRITE(fastfred_flip_screen_y_w)
	AM_RANGE(0xf800, 0xf800) AM_READWRITE(watchdog_reset_r, soundlatch_byte_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( jumpcoas_map, AS_PROGRAM, 8, fastfred_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xd000, 0xd03f) AM_RAM_WRITE(fastfred_attributes_w) AM_SHARE("attributesram")
	AM_RANGE(0xd040, 0xd05f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd060, 0xd3ff) AM_RAM
	AM_RANGE(0xd800, 0xdbff) AM_MIRROR(0x400) AM_RAM_WRITE(fastfred_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xe000, 0xe000) AM_WRITEONLY AM_SHARE("bgcolor")
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("DSW1")
	AM_RANGE(0xe801, 0xe801) AM_READ_PORT("DSW2")
	AM_RANGE(0xe802, 0xe802) AM_READ_PORT("BUTTONS")
	AM_RANGE(0xe803, 0xe803) AM_READ_PORT("JOYS")
	AM_RANGE(0xf000, 0xf000) AM_WRITENOP // Unused, but initialized
	AM_RANGE(0xf001, 0xf001) AM_WRITE(nmi_mask_w)
	AM_RANGE(0xf002, 0xf002) AM_WRITE(fastfred_colorbank1_w)
	AM_RANGE(0xf003, 0xf003) AM_WRITE(fastfred_colorbank2_w)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(fastfred_charbank1_w)
	AM_RANGE(0xf005, 0xf005) AM_WRITE(fastfred_charbank2_w)
	AM_RANGE(0xf006, 0xf006) AM_WRITE(fastfred_flip_screen_x_w)
	AM_RANGE(0xf007, 0xf007) AM_WRITE(fastfred_flip_screen_y_w)
	AM_RANGE(0xf116, 0xf116) AM_WRITE(fastfred_flip_screen_x_w)
	AM_RANGE(0xf117, 0xf117) AM_WRITE(fastfred_flip_screen_y_w)
	//AM_RANGE(0xf800, 0xf800) AM_READ(watchdog_reset_r)  // Why doesn't this work???
	AM_RANGE(0xf800, 0xf801) AM_READNOP AM_DEVWRITE("ay8910.1", ay8910_device, address_data_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( imago_map, AS_PROGRAM, 8, fastfred_state )
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x1fff) AM_READ(imago_sprites_offset_r)
	AM_RANGE(0x2000, 0x6fff) AM_ROM
	AM_RANGE(0xb000, 0xb3ff) AM_RAM // same fg videoram (which one of the 2 is really used?)
	AM_RANGE(0xb800, 0xbfff) AM_RAM_WRITE(imago_sprites_dma_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(0xc800, 0xcbff) AM_RAM_WRITE(imago_fg_videoram_w) AM_SHARE("imago_fg_vram")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(fastfred_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd800, 0xd83f) AM_RAM_WRITE(fastfred_attributes_w) AM_SHARE("attributesram")
	AM_RANGE(0xd840, 0xd85f) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xd860, 0xd8ff) AM_RAM // Unused, but initialized
	AM_RANGE(0xe000, 0xe000) AM_READ_PORT("BUTTONS")
	AM_RANGE(0xe800, 0xe800) AM_READ_PORT("JOYS")
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("DSW") AM_WRITENOP // writes 1 when level starts, 0 when game over
	AM_RANGE(0xf001, 0xf001) AM_WRITE(nmi_mask_w)
	AM_RANGE(0xf002, 0xf002) AM_WRITE(fastfred_colorbank1_w)
	AM_RANGE(0xf003, 0xf003) AM_WRITE(fastfred_colorbank2_w)
	AM_RANGE(0xf004, 0xf004) AM_WRITE(imago_dma_irq_w)
	AM_RANGE(0xf005, 0xf005) AM_WRITE(imago_charbank_w)
	AM_RANGE(0xf006, 0xf006) AM_WRITE(fastfred_flip_screen_x_w)
	AM_RANGE(0xf007, 0xf007) AM_WRITE(fastfred_flip_screen_y_w)
	AM_RANGE(0xf400, 0xf400) AM_WRITENOP // writes 0 or 2
	AM_RANGE(0xf401, 0xf401) AM_WRITE(imago_sprites_bank_w)
	AM_RANGE(0xf800, 0xf800) AM_READNOP AM_WRITE(soundlatch_byte_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, fastfred_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_byte_r) AM_WRITE(sound_nmi_mask_w)
	AM_RANGE(0x4000, 0x4000) AM_WRITEONLY  // Reset PSG's
	AM_RANGE(0x5000, 0x5001) AM_DEVWRITE("ay8910.1", ay8910_device, address_data_w)
	AM_RANGE(0x6000, 0x6001) AM_DEVWRITE("ay8910.2", ay8910_device, address_data_w)
	AM_RANGE(0x7000, 0x7000) AM_READNOP // only for Imago, read but not used
ADDRESS_MAP_END


static INPUT_PORTS_START( common )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_START("JOYS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
INPUT_PORTS_END

static INPUT_PORTS_START( fastfred )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x01, "A 2/1 B 2/1" )
	PORT_DIPSETTING(    0x02, "A 2/1 B 1/3" )
	PORT_DIPSETTING(    0x00, "A 1/1 B 1/1" )
	PORT_DIPSETTING(    0x03, "A 1/1 B 1/2" )
	PORT_DIPSETTING(    0x04, "A 1/1 B 1/3" )
	PORT_DIPSETTING(    0x05, "A 1/1 B 1/4" )
	PORT_DIPSETTING(    0x06, "A 1/1 B 1/5" )
	PORT_DIPSETTING(    0x07, "A 1/1 B 1/6" )
	PORT_DIPSETTING(    0x08, "A 1/2 B 1/2" )
	PORT_DIPSETTING(    0x09, "A 1/2 B 1/4" )
	PORT_DIPSETTING(    0x0a, "A 1/2 B 1/5" )
	PORT_DIPSETTING(    0x0e, "A 1/2 B 1/6" )
	PORT_DIPSETTING(    0x0b, "A 1/2 B 1/10" )
	PORT_DIPSETTING(    0x0c, "A 1/2 B 1/11" )
	PORT_DIPSETTING(    0x0d, "A 1/2 B 1/12" )
	PORT_DIPSETTING(    0x0f, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPSETTING(    0x60, "100000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( flyboy )
	PORT_INCLUDE( common )

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x30, "255 (Cheat)")
	PORT_DIPNAME( 0x40, 0x00, "Invulnerability (Cheat)")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static INPUT_PORTS_START( jumpcoas )
	PORT_INCLUDE( common )

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x30, "255 (Cheat)")
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")

INPUT_PORTS_END

static INPUT_PORTS_START( boggy84 )
	PORT_INCLUDE( common )

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("JOYS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x30, "255 (Cheat)")
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( redrobin )
	PORT_INCLUDE( common )

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("JOYS")
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30000" )
	PORT_DIPSETTING(    0x40, "50000" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )  /* most likely "Difficulty" */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )      /* it somehow effects the */
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )       /* monsters */
INPUT_PORTS_END

static INPUT_PORTS_START( imago )
	PORT_INCLUDE( common )

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( imagoa )
	PORT_INCLUDE( common )

	PORT_MODIFY("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("JOYS")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x38, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout imago_spritelayout =
{
	16,16,
	0x40,
	3,
	{ 0x800*8*2, 0x800*8*1, 0x800*8*0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static const gfx_layout imago_char_1bpp =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( fastfred )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( jumpcoas )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout, 0, 32 )
GFXDECODE_END

static GFXDECODE_START( imago )
	GFXDECODE_ENTRY( "gfx1", 0,      charlayout,          0, 32 )
	GFXDECODE_ENTRY( nullptr,   0xb800, imago_spritelayout,  0, 32 )
	GFXDECODE_ENTRY( "gfx3", 0,      charlayout,          0, 32 )
	GFXDECODE_ENTRY( "gfx4", 0,      imago_char_1bpp, 0x140,  1 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(fastfred_state::vblank_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

INTERRUPT_GEN_MEMBER(fastfred_state::sound_timer_irq)
{
	if(m_sound_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

static MACHINE_CONFIG_START( fastfred, fastfred_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12_432MHz/4)   /* 3.108 MHz; xtal from pcb pics, divider not verified */
	MCFG_CPU_PROGRAM_MAP(fastfred_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fastfred_state,  vblank_irq)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_12_432MHz/8)  /* 1.554 MHz; xtal from pcb pics, divider not verified */
	MCFG_CPU_PROGRAM_MAP(sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(fastfred_state, sound_timer_irq, 4*60)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) //CLOCK/16/60
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(fastfred_state, screen_update_fastfred)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fastfred)

	MCFG_PALETTE_ADD("palette", 32*8)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(fastfred_state,fastfred)
	MCFG_VIDEO_START_OVERRIDE(fastfred_state,fastfred)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay8910.1", AY8910, XTAL_12_432MHz/8) /* 1.554 MHz; xtal from pcb pics, divider not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay8910.2", AY8910, XTAL_12_432MHz/8) /* 1.554 MHz; xtal from pcb pics, divider not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( jumpcoas, fastfred )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(jumpcoas_map)

	MCFG_DEVICE_REMOVE("audiocpu")

	/* video hardware */
	MCFG_GFXDECODE_MODIFY("gfxdecode", jumpcoas)

	/* sound hardware */
	MCFG_DEVICE_REMOVE("ay8910.2")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( imago, fastfred )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(imago_map)

	MCFG_MACHINE_START_OVERRIDE(fastfred_state,imago)

	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(256+64+2) /* 256 for characters, 64 for the stars and 2 for the web */
	MCFG_GFXDECODE_MODIFY("gfxdecode", imago)

	MCFG_VIDEO_START_OVERRIDE(fastfred_state,imago)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(fastfred_state, screen_update_imago)
MACHINE_CONFIG_END

#undef CLOCK

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( fastfred )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ffr.01",       0x0000, 0x1000, CRC(15032c13) SHA1(18ae84e87ac430e3f1cbc388ad16fb1d20aaba2f) )
	ROM_LOAD( "ffr.02",       0x1000, 0x1000, CRC(f9642744) SHA1(b086ad284593b7f2ad314ad5002c9a2b293b8103) )
	ROM_LOAD( "ffr.03",       0x2000, 0x1000, CRC(f0919727) SHA1(f16bc7de715acf0396818ce48ebe45b6a301b2cb) )
	ROM_LOAD( "ffr.04",       0x3000, 0x1000, CRC(c778751e) SHA1(7d9df82d2123e4e8565d8d50eed02daf455f96e8) )
	ROM_LOAD( "ffr.05",       0x4000, 0x1000, CRC(cd6e160a) SHA1(fd943aae88e350db192711ad0b75c0a9b21ef9c8) )
	ROM_LOAD( "ffr.06",       0x5000, 0x1000, CRC(67f7f9b3) SHA1(c862c04d97ffd6714c0da197a262e0a540175a65) )
	ROM_LOAD( "ffr.07",       0x6000, 0x1000, CRC(2935c76a) SHA1(acc2eec3c242dc904c5175e4b5b5fb025b956c17) )
	ROM_LOAD( "ffr.08",       0x7000, 0x1000, CRC(0fb79e7b) SHA1(82cc315708064bc498268abb8dbca2e36c3a0dcd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "ffr.09",       0x0000, 0x1000, CRC(a1ec8d7e) SHA1(5b4884381d0df79d3ed4246a9cf78f9b3bb14f79) )
	ROM_LOAD( "ffr.10",       0x1000, 0x1000, CRC(460ca837) SHA1(6d94f04e94ec15cbc5602bb303e9610ad20275fb) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ffr.14",       0x0000, 0x1000, CRC(e8a00e81) SHA1(d93298f677baa4842f6e00b86fab099af1818467) )
	ROM_LOAD( "ffr.17",       0x1000, 0x1000, CRC(701e0f01) SHA1(f1f907386cf1f6676019cee56e6ee85d3117b8c3) )
	ROM_LOAD( "ffr.15",       0x2000, 0x1000, CRC(b49b053f) SHA1(b9f579d51fb9cc72158eef3d2d442c04099c8af1) )
	ROM_LOAD( "ffr.18",       0x3000, 0x1000, CRC(4b208c8b) SHA1(2cc7a1f93cc94fe54f16aa9e581bec91a7ad34ba) )
	ROM_LOAD( "ffr.16",       0x4000, 0x1000, CRC(8c686bc2) SHA1(73f63305209d58883f7b3cd8d766f8ad1bba6eb1) )
	ROM_LOAD( "ffr.19",       0x5000, 0x1000, CRC(75b613f6) SHA1(73d6d505f3ddfe2b897066d0f8e720d2718bf5d4) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "ffr.11",       0x0000, 0x1000, CRC(0e1316d4) SHA1(fa88311cdc6b6db9f892d7a2a6927acf03c8fc8d) )
	ROM_LOAD( "ffr.12",       0x1000, 0x1000, CRC(94c06686) SHA1(a40fa5b539da604750605ba6c8a6d1bac62f6ede) )
	ROM_LOAD( "ffr.13",       0x2000, 0x1000, CRC(3fcfaa8e) SHA1(2b1cf871ebf907fe41dcf1773b29066e4c20e2f3) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "red.9h",       0x0000, 0x0100, CRC(b801e294) SHA1(79926dc69c9088c2a5e5f15e260c644a90071ba0) )
	ROM_LOAD( "green.8h",     0x0100, 0x0100, CRC(7da063d0) SHA1(8e40174c4f6ba4a15edd89a6fe2b98a5e50531ff) )
	ROM_LOAD( "blue.7h",      0x0200, 0x0100, CRC(85c05c18) SHA1(a609a45c593fc6c491624076f7d65da55b5e603f) )
ROM_END

ROM_START( flyboy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "flyboy01.cpu", 0x0000, 0x1000, CRC(b05aa900) SHA1(1ad394a438ddf96974b0b841d916766e45e8f3ba) )
	ROM_LOAD( "flyboy02.cpu", 0x1000, 0x1000, CRC(474867f5) SHA1(b352318eee71218155046bba9f032364e1213c02) )
	ROM_LOAD( "rom3.cpu",     0x2000, 0x1000, CRC(d2f8f085) SHA1(335d53b50c5ad8180bc7d77b808a638604eb7f39) )
	ROM_LOAD( "rom4.cpu",     0x3000, 0x1000, CRC(19e5e15c) SHA1(86c13a518cfb1666d69af73976c2fba89edf0393) )
	ROM_LOAD( "flyboy05.cpu", 0x4000, 0x1000, CRC(207551f7) SHA1(363f73f4a14e2018599f5e6e1ae75042d0b757d7) )
	ROM_LOAD( "rom6.cpu",     0x5000, 0x1000, CRC(f5464c72) SHA1(f4be4055964f523108bc98e3eb855ca1d8323e6f) )
	ROM_LOAD( "rom7.cpu",     0x6000, 0x1000, CRC(50a1baff) SHA1(469913e7652c6a334fb071e65cc00058b411527f) )
	ROM_LOAD( "rom8.cpu",     0x7000, 0x1000, CRC(fe2ae95d) SHA1(e44c36b7726892b4a360a7dc02820a3dbb21b398) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom9.cpu",     0x0000, 0x1000, CRC(5d05d1a0) SHA1(cbf6144bf0b0686e4af41d8aeffd54c25f60eadc) )
	ROM_LOAD( "rom10.cpu",    0x1000, 0x1000, CRC(7a28005b) SHA1(71c5779aec3c40614db3ba2c6f7820e6592bf101) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "rom14.rom",    0x0000, 0x1000, CRC(aeb07260) SHA1(cf8fefa7b5b2413060ffe6a231033d443b4a4c6a) )
	ROM_LOAD( "rom17.rom",    0x1000, 0x1000, CRC(a834325b) SHA1(372054d525edba3e720162f9e2f31d6a1432c795) )
	ROM_LOAD( "rom15.rom",    0x2000, 0x1000, CRC(c10c7ce2) SHA1(bc4ffca80554dd6692b32fd82f93cb74f7f18e96) )
	ROM_LOAD( "rom18.rom",    0x3000, 0x1000, CRC(2f196c80) SHA1(9e1cb567aa3621e92e88e4ab4953c56e2baafb0b) )
	ROM_LOAD( "rom16.rom",    0x4000, 0x1000, CRC(719246b1) SHA1(ca5879289e3c7f04649407b448747fcff6a5ef47) )
	ROM_LOAD( "rom19.rom",    0x5000, 0x1000, CRC(00c1c5d2) SHA1(196e67ca21568b5aafc4befd9f9b6de0a677551b) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "rom11.rom",    0x0000, 0x1000, CRC(ee7ec342) SHA1(936ce03dd5ee05eea78d0e3308ce7d369397c361) )
	ROM_LOAD( "rom12.rom",    0x1000, 0x1000, CRC(84d03124) SHA1(92c7efc4bfe39aa47909071f9a90ec7e5c0fa1a1) )
	ROM_LOAD( "rom13.rom",    0x2000, 0x1000, CRC(fcb33ff4) SHA1(a76addec96b42a06df97eca37f3039f8a4727dfb) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "red.9h",       0x0000, 0x0100, CRC(b801e294) SHA1(79926dc69c9088c2a5e5f15e260c644a90071ba0) )
	ROM_LOAD( "green.8h",     0x0100, 0x0100, CRC(7da063d0) SHA1(8e40174c4f6ba4a15edd89a6fe2b98a5e50531ff) )
	ROM_LOAD( "blue.7h",      0x0200, 0x0100, CRC(85c05c18) SHA1(a609a45c593fc6c491624076f7d65da55b5e603f) )
ROM_END

ROM_START( flyboyb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.cpu",     0x0000, 0x1000, CRC(e9e1f527) SHA1(999b8054751ddaa1b5bad48eaa12fc11a915c74c) )
	ROM_LOAD( "rom2.cpu",     0x1000, 0x1000, CRC(07fbe78c) SHA1(875e29e6ed7525678b52276248f0cf4c885bd521) )
	ROM_LOAD( "rom3.cpu",     0x2000, 0x1000, CRC(d2f8f085) SHA1(335d53b50c5ad8180bc7d77b808a638604eb7f39) )
	ROM_LOAD( "rom4.cpu",     0x3000, 0x1000, CRC(19e5e15c) SHA1(86c13a518cfb1666d69af73976c2fba89edf0393) )
	ROM_LOAD( "rom5.cpu",     0x4000, 0x1000, CRC(d56872ea) SHA1(9908c15496409308c0b862e96a6249198497e1da) )
	ROM_LOAD( "rom6.cpu",     0x5000, 0x1000, CRC(f5464c72) SHA1(f4be4055964f523108bc98e3eb855ca1d8323e6f) )
	ROM_LOAD( "rom7.cpu",     0x6000, 0x1000, CRC(50a1baff) SHA1(469913e7652c6a334fb071e65cc00058b411527f) )
	ROM_LOAD( "rom8.cpu",     0x7000, 0x1000, CRC(fe2ae95d) SHA1(e44c36b7726892b4a360a7dc02820a3dbb21b398) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "rom9.cpu",     0x0000, 0x1000, CRC(5d05d1a0) SHA1(cbf6144bf0b0686e4af41d8aeffd54c25f60eadc) )
	ROM_LOAD( "rom10.cpu",    0x1000, 0x1000, CRC(7a28005b) SHA1(71c5779aec3c40614db3ba2c6f7820e6592bf101) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "rom14.rom",    0x0000, 0x1000, CRC(aeb07260) SHA1(cf8fefa7b5b2413060ffe6a231033d443b4a4c6a) )
	ROM_LOAD( "rom17.rom",    0x1000, 0x1000, CRC(a834325b) SHA1(372054d525edba3e720162f9e2f31d6a1432c795) )
	ROM_LOAD( "rom15.rom",    0x2000, 0x1000, CRC(c10c7ce2) SHA1(bc4ffca80554dd6692b32fd82f93cb74f7f18e96) )
	ROM_LOAD( "rom18.rom",    0x3000, 0x1000, CRC(2f196c80) SHA1(9e1cb567aa3621e92e88e4ab4953c56e2baafb0b) )
	ROM_LOAD( "rom16.rom",    0x4000, 0x1000, CRC(719246b1) SHA1(ca5879289e3c7f04649407b448747fcff6a5ef47) )
	ROM_LOAD( "rom19.rom",    0x5000, 0x1000, CRC(00c1c5d2) SHA1(196e67ca21568b5aafc4befd9f9b6de0a677551b) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "rom11.rom",    0x0000, 0x1000, CRC(ee7ec342) SHA1(936ce03dd5ee05eea78d0e3308ce7d369397c361) )
	ROM_LOAD( "rom12.rom",    0x1000, 0x1000, CRC(84d03124) SHA1(92c7efc4bfe39aa47909071f9a90ec7e5c0fa1a1) )
	ROM_LOAD( "rom13.rom",    0x2000, 0x1000, CRC(fcb33ff4) SHA1(a76addec96b42a06df97eca37f3039f8a4727dfb) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "red.9h",       0x0000, 0x0100, CRC(b801e294) SHA1(79926dc69c9088c2a5e5f15e260c644a90071ba0) )
	ROM_LOAD( "green.8h",     0x0100, 0x0100, CRC(7da063d0) SHA1(8e40174c4f6ba4a15edd89a6fe2b98a5e50531ff) )
	ROM_LOAD( "blue.7h",      0x0200, 0x0100, CRC(85c05c18) SHA1(a609a45c593fc6c491624076f7d65da55b5e603f) )
ROM_END

ROM_START( jumpcoas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jumpcoas.001", 0x0000, 0x2000, CRC(0778c953) SHA1(7def6656532332e56d76700431e4c3199e407e50) )
	ROM_LOAD( "jumpcoas.002", 0x2000, 0x2000, CRC(57f59ce1) SHA1(1508afb34f77c829ed62b16be10b0ebf8e91a62c) )
	ROM_LOAD( "jumpcoas.003", 0x4000, 0x2000, CRC(d9fc93be) SHA1(e13476991720a1e900f4ab65175df7ee40c6960d) )
	ROM_LOAD( "jumpcoas.004", 0x6000, 0x2000, CRC(dc108fc1) SHA1(a238b1b924877167aa8f17e9c9bd450e2c2cc9f6) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "jumpcoas.005", 0x0000, 0x1000, CRC(2dce6b07) SHA1(e7f9e5d68c53ee2433c22d00e69d4b994b44d349) )
	ROM_LOAD( "jumpcoas.006", 0x1000, 0x1000, CRC(0d24aa1b) SHA1(300eba18c69eb693b033562446e7fee764161e07) )
	ROM_LOAD( "jumpcoas.007", 0x2000, 0x1000, CRC(14c21e67) SHA1(1a01dcd917e9c06db5d86cd35146e9ccdad65975) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "jumpcoas.red", 0x0000, 0x0100, CRC(13714880) SHA1(ede901434f3a35138574e65985e5791e6686ef0d) )
	ROM_LOAD( "jumpcoas.gre", 0x0100, 0x0100, CRC(05354848) SHA1(c44f6b4b9c9d58d9ace617dcd36ca197f6d7dd8c) )
	ROM_LOAD( "jumpcoas.blu", 0x0200, 0x0100, CRC(f4662db7) SHA1(638ac15b15ae908581561ff77f446d81ec64c086) )
ROM_END

ROM_START( jumpcoast )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.d1", 0x0000, 0x2000, CRC(8ac220c5) SHA1(714dd34ca6c6c1803778a715b803f81a94286e1c) )
	ROM_LOAD( "jumpcoas.002", 0x2000, 0x2000, CRC(57f59ce1) SHA1(1508afb34f77c829ed62b16be10b0ebf8e91a62c) )
	ROM_LOAD( "3.d3", 0x4000, 0x2000, CRC(17e4deba) SHA1(880689304af9744de3c96936f03345968ab8085c) )
	ROM_LOAD( "jumpcoas.004", 0x6000, 0x2000, CRC(dc108fc1) SHA1(a238b1b924877167aa8f17e9c9bd450e2c2cc9f6) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "jumpcoas.005", 0x0000, 0x1000, CRC(2dce6b07) SHA1(e7f9e5d68c53ee2433c22d00e69d4b994b44d349) )
	ROM_LOAD( "jumpcoas.006", 0x1000, 0x1000, CRC(0d24aa1b) SHA1(300eba18c69eb693b033562446e7fee764161e07) )
	ROM_LOAD( "jumpcoas.007", 0x2000, 0x1000, CRC(14c21e67) SHA1(1a01dcd917e9c06db5d86cd35146e9ccdad65975) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "jumpcoas.red", 0x0000, 0x0100, CRC(13714880) SHA1(ede901434f3a35138574e65985e5791e6686ef0d) )
	ROM_LOAD( "jumpcoas.gre", 0x0100, 0x0100, CRC(05354848) SHA1(c44f6b4b9c9d58d9ace617dcd36ca197f6d7dd8c) )
	ROM_LOAD( "jumpcoas.blu", 0x0200, 0x0100, CRC(f4662db7) SHA1(638ac15b15ae908581561ff77f446d81ec64c086) )
ROM_END

ROM_START( boggy84 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.d1", 0x0000, 0x2000, CRC(722cc0ec) SHA1(7daacffd9dfcbc8a441485943e45cc8958d19167) )
	ROM_LOAD( "p2.d2", 0x2000, 0x2000, CRC(6c096798) SHA1(74ea860ef10cb566bcb07d67e6c79f542a66de91) )
	ROM_LOAD( "p3.d3", 0x4000, 0x2000, CRC(9da59104) SHA1(167af18d50d99e66111e4ebd52d0dd86d5d6d391) )
	ROM_LOAD( "p4.d4", 0x6000, 0x2000, CRC(73ef6807) SHA1(3144285019ab5cc7f2e1ba0a31956964ea1c706c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "g1.h10", 0x0000, 0x1000, CRC(f4238c68) SHA1(a14cedb126e49e40bab6f46870af64c04ccb01f4) )
	ROM_LOAD( "g2.h11", 0x1000, 0x1000, CRC(ce285bd2) SHA1(61e58920553f56448e76d859c1b0f316f299363f) )
	ROM_LOAD( "g3.h12", 0x2000, 0x1000, CRC(02f5f4fa) SHA1(d28dc23cd3a39bb483d05b59869ed2300e5e77a7) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "r.e10", 0x0000, 0x0100, CRC(f3862912) SHA1(128ba48202299ef5852f08fd0f910d8e9f68f22c) )
	ROM_LOAD( "g.e11", 0x0100, 0x0100, CRC(80b87220) SHA1(7bd81060b986d5cd4a27dc8a9394423959deaa05) )
	ROM_LOAD( "b.e12", 0x0200, 0x0100, CRC(52b7f445) SHA1(6395ac705a35e602a355cbf700025ff917e89b37) )
ROM_END

ROM_START( boggy84b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpurom1.bin", 0x0000, 0x2000, CRC(665266c0) SHA1(7785a7d710948718236f9be4b3e2a3fdc00662a5) )
	ROM_LOAD( "cpurom2.bin", 0x2000, 0x2000, CRC(6c096798) SHA1(74ea860ef10cb566bcb07d67e6c79f542a66de91) )
	ROM_LOAD( "cpurom3.bin", 0x4000, 0x2000, CRC(9da59104) SHA1(167af18d50d99e66111e4ebd52d0dd86d5d6d391) )
	ROM_LOAD( "cpurom4.bin", 0x6000, 0x2000, CRC(73ef6807) SHA1(3144285019ab5cc7f2e1ba0a31956964ea1c706c) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "gfx1.bin", 0x0000, 0x1000, CRC(f4238c68) SHA1(a14cedb126e49e40bab6f46870af64c04ccb01f4) )
	ROM_LOAD( "gfx2.bin", 0x1000, 0x1000, CRC(ce285bd2) SHA1(61e58920553f56448e76d859c1b0f316f299363f) )
	ROM_LOAD( "gfx3.bin", 0x2000, 0x1000, CRC(02f5f4fa) SHA1(d28dc23cd3a39bb483d05b59869ed2300e5e77a7) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "r12e", 0x0000, 0x0100, CRC(f3862912) SHA1(128ba48202299ef5852f08fd0f910d8e9f68f22c) )
	ROM_LOAD( "g12e", 0x0100, 0x0100, CRC(80b87220) SHA1(7bd81060b986d5cd4a27dc8a9394423959deaa05) )
	ROM_LOAD( "b12e", 0x0200, 0x0100, CRC(52b7f445) SHA1(6395ac705a35e602a355cbf700025ff917e89b37) )
ROM_END

ROM_START( redrobin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "redro01f.16d", 0x0000, 0x1000, CRC(0788ce10) SHA1(32084714fe005d0489ab1a7e6684a49bd103ce5e) )
	ROM_LOAD( "redrob02.17d", 0x1000, 0x1000, CRC(bf9b95b4) SHA1(55de12c36e193525159ecca6cff883b69709f5ba) )
	ROM_LOAD( "redrob03.14b", 0x2000, 0x1000, CRC(9386e40b) SHA1(c55fe071a68fd8ca19a7919cef790e588d056b74) )
	ROM_LOAD( "redrob04.16b", 0x3000, 0x1000, CRC(5cafffc4) SHA1(910af87d2c002cd825af759c0d7bc91efe5cc08d) )
	ROM_LOAD( "redrob05.17b", 0x4000, 0x1000, CRC(a224d41e) SHA1(d5fc58f5852779adb5468faf312ee2776531c05d) )
	ROM_LOAD( "redrob06.14a", 0x5000, 0x1000, CRC(822e0bd7) SHA1(e273a76fd0d0dc8bb2e02459ec499bd8dfd3e95d) )
	ROM_LOAD( "redrob07.15a", 0x6000, 0x1000, CRC(0deacf17) SHA1(86c00a451f77bc64bd6c7582130de391ddd222de) )
	ROM_LOAD( "redrob08.17a", 0x7000, 0x1000, CRC(095cf908) SHA1(70005f7a1f05e666392f8e9139d125fa97fd9814) )
	ROM_LOAD( "redrob20.15e", 0x8000, 0x4000, CRC(5cce22b7) SHA1(8cc763983766fbb9b995d5c157f66f170cd1d01e) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "redrob09.1f",  0x0000, 0x1000, CRC(21af2d03) SHA1(6a73b85169bb8ae6f3dca5581a39802ca4dd6f58) )
	ROM_LOAD( "redro10f.1e",  0x1000, 0x1000, CRC(bf0e772f) SHA1(e07f7a8876437822bb0c300905455c88125282d7) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "redrob14.17l", 0x0000, 0x1000, CRC(f6c571e0) SHA1(7d0d6fbea8393603cd6a93f0f059222a15799a3c) )
	ROM_LOAD( "redrob17.17j", 0x1000, 0x1000, CRC(86dcdf21) SHA1(d41a4dc118a9054dfa8d06e8e4bc401e347e6891) )
	ROM_LOAD( "redrob15.15k", 0x2000, 0x1000, CRC(05f7df48) SHA1(b17d1e25ea2dce61f7a6c5b65ba13fc1137ae958) )
	ROM_LOAD( "redrob18.16j", 0x3000, 0x1000, CRC(7aeb2bb9) SHA1(c1dd19a0821fcdf33e54898d032b36ae1f2f68f2) )
	ROM_LOAD( "redrob16.14l", 0x4000, 0x1000, CRC(21349d09) SHA1(46d828ab037f823947ce3d8572f6763ea72bc00c) )
	ROM_LOAD( "redrob19.14j", 0x5000, 0x1000, CRC(7184d999) SHA1(5333671643f749dc67d3d6e537757950483f015e) )

	ROM_REGION( 0x3000, "gfx2", 0 )
	ROM_LOAD( "redrob11.17m", 0x0000, 0x1000, CRC(559f7894) SHA1(487b7fa207af34ff4dfa977efc546416fb8d1509) )
	ROM_LOAD( "redrob12.15m", 0x1000, 0x1000, CRC(a763b11d) SHA1(041cc9f582567ee8c5519b3257c03bdab1683388) )
	ROM_LOAD( "redrob13.14m", 0x2000, 0x1000, CRC(d667f45b) SHA1(1e38ac833b572f5af14b5bb98c98135311ca5ceb) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "red.9h",       0x0000, 0x0100, CRC(b801e294) SHA1(79926dc69c9088c2a5e5f15e260c644a90071ba0) )
	ROM_LOAD( "green.8h",     0x0100, 0x0100, CRC(7da063d0) SHA1(8e40174c4f6ba4a15edd89a6fe2b98a5e50531ff) )
	ROM_LOAD( "blue.7h",      0x0200, 0x0100, CRC(85c05c18) SHA1(a609a45c593fc6c491624076f7d65da55b5e603f) )
ROM_END

ROM_START( imago )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "imago11.82", 0x0000, 0x1000, CRC(3cce69b4) SHA1(e7d52e388e09e86abb597493f5807ee088cf7a40) )
	ROM_CONTINUE(           0x2000, 0x1000 )
	ROM_LOAD( "imago12.83", 0x3000, 0x2000, CRC(8dff98c0) SHA1(e7311d9ca4544f1263e894e6d93ca52c87fc83bf) )
	ROM_LOAD( "13.bin",     0x5000, 0x2000, CRC(ae684602) SHA1(d187abbe62ee58a8190d9f428ded0feeb9484abd) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "imago08.60", 0x0000, 0x1000, CRC(4f77c2c9) SHA1(1e046786fbad7fb8c7c462b7bd5d80152c6b8779) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "1.bin",      0x0000, 0x1000, CRC(f80a0b69) SHA1(2b85179942586316eb61614d8697588aa9d26f9a) )
	ROM_LOAD( "imago02.40", 0x1000, 0x1000, CRC(71354480) SHA1(f5f5e1cc336cae1778b7f6c744eb1bdc4226f138) )
	ROM_LOAD( "3.bin",      0x2000, 0x1000, CRC(722fd625) SHA1(6f9a9f4f000cc0b251ca8d496a2ea4a708665dda) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "imago04.51", 0x0000, 0x1000, CRC(ed987b3e) SHA1(2f88a0463b4323adb27467fb3d022144a4943793) )
	ROM_LOAD( "imago05.52", 0x1000, 0x1000, CRC(77ee68ce) SHA1(a47af1bec81977d0f47463bd88e9f526fd2d6611) )
	ROM_LOAD( "imago07.56", 0x2000, 0x1000, CRC(48b35190) SHA1(3a000264aad03f55fe67eed7c868acf87e804c0f) )
	ROM_LOAD( "imago06.55", 0x3000, 0x1000, CRC(136990fc) SHA1(f3ecba92db25fbeb7df83c26667b7447c2d03b58) )
	ROM_LOAD( "imago09.64", 0x4000, 0x1000, CRC(9efb806d) SHA1(504cc27cf071873714ec61835d9da676884fe1c8) )
	ROM_LOAD( "imago10.65", 0x5000, 0x1000, CRC(801a18d3) SHA1(f798978a47124f50be25ab4e5c6a4974d9003634) )

	ROM_REGION( 0x3000, "gfx3", 0 )
	ROM_LOAD( "imago14.170", 0x0000, 0x1000, CRC(eded37f6) SHA1(c2ff5d4c1b001740ec4453467f879035db196a9b) )
	ROM_FILL(                0x1000, 0x2000, 0x00 )

	ROM_REGION( 0x1000, "gfx4", 0 )
	ROM_LOAD( "imago15.191", 0x0000, 0x1000, CRC(85fcc195) SHA1(a76f24201c037d1e6f909fb0ea4ad59b1d6ddd57) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "imago.96", 0x0000, 0x0100, CRC(5ba81edc) SHA1(b64ebbe054052583688cdf0f064794436c095e7e) )
	ROM_LOAD( "imago.95", 0x0100, 0x0100, CRC(e2b7aa09) SHA1(f8edfccdd698793d9a9f423953a582b0f7b9b697) )
	ROM_LOAD( "imago.97", 0x0200, 0x0100, CRC(e28a7f00) SHA1(05b4882c5ea5da332735866d858872bc5eeaca24) )
ROM_END

/* this set has patched out the code to enable the cocktail mode for 2nd player at $5b24 */
ROM_START( imagoa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "imago11.82", 0x0000, 0x1000, CRC(3cce69b4) SHA1(e7d52e388e09e86abb597493f5807ee088cf7a40) )
	ROM_CONTINUE(           0x2000, 0x1000 )
	ROM_LOAD( "imago12.83", 0x3000, 0x2000, CRC(8dff98c0) SHA1(e7311d9ca4544f1263e894e6d93ca52c87fc83bf) )
	ROM_LOAD( "imago13.84", 0x5000, 0x2000, CRC(f0f14b4d) SHA1(92b82080575a9c95df926c404c19875ac66c2b00) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "imago08.60", 0x0000, 0x1000, CRC(4f77c2c9) SHA1(1e046786fbad7fb8c7c462b7bd5d80152c6b8779) )

	ROM_REGION( 0x3000, "gfx1", 0 )
	ROM_LOAD( "imago01.39", 0x0000, 0x1000, CRC(f09fe0d4) SHA1(058af955f1758db81acd021ae3e8464c18de6bb6) )
	ROM_LOAD( "imago02.40", 0x1000, 0x1000, CRC(71354480) SHA1(f5f5e1cc336cae1778b7f6c744eb1bdc4226f138) )
	ROM_LOAD( "imago03.41", 0x2000, 0x1000, CRC(7aba3d98) SHA1(5d058f39bf1339d523fe015b67083d44ff6a81d4) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "imago04.51", 0x0000, 0x1000, CRC(ed987b3e) SHA1(2f88a0463b4323adb27467fb3d022144a4943793) )
	ROM_LOAD( "imago05.52", 0x1000, 0x1000, CRC(77ee68ce) SHA1(a47af1bec81977d0f47463bd88e9f526fd2d6611) )
	ROM_LOAD( "imago07.56", 0x2000, 0x1000, CRC(48b35190) SHA1(3a000264aad03f55fe67eed7c868acf87e804c0f) )
	ROM_LOAD( "imago06.55", 0x3000, 0x1000, CRC(136990fc) SHA1(f3ecba92db25fbeb7df83c26667b7447c2d03b58) )
	ROM_LOAD( "imago09.64", 0x4000, 0x1000, CRC(9efb806d) SHA1(504cc27cf071873714ec61835d9da676884fe1c8) )
	ROM_LOAD( "imago10.65", 0x5000, 0x1000, CRC(801a18d3) SHA1(f798978a47124f50be25ab4e5c6a4974d9003634) )

	ROM_REGION( 0x3000, "gfx3", 0 )
	ROM_LOAD( "imago14.170", 0x0000, 0x1000, CRC(eded37f6) SHA1(c2ff5d4c1b001740ec4453467f879035db196a9b) )
	ROM_FILL(                0x1000, 0x2000, 0x00 )

	ROM_REGION( 0x1000, "gfx4", 0 )
	ROM_LOAD( "imago15.191", 0x0000, 0x1000, CRC(85fcc195) SHA1(a76f24201c037d1e6f909fb0ea4ad59b1d6ddd57) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "imago.96", 0x0000, 0x0100, CRC(5ba81edc) SHA1(b64ebbe054052583688cdf0f064794436c095e7e) )
	ROM_LOAD( "imago.95", 0x0100, 0x0100, CRC(e2b7aa09) SHA1(f8edfccdd698793d9a9f423953a582b0f7b9b697) )
	ROM_LOAD( "imago.97", 0x0200, 0x0100, CRC(e28a7f00) SHA1(05b4882c5ea5da332735866d858872bc5eeaca24) )
ROM_END


DRIVER_INIT_MEMBER(fastfred_state,flyboy)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc085, 0xc099, read8_delegate(FUNC(fastfred_state::flyboy_custom1_io_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc8fb, 0xc900, read8_delegate(FUNC(fastfred_state::flyboy_custom2_io_r),this));
	m_hardware_type = 1;
}

DRIVER_INIT_MEMBER(fastfred_state,flyboyb)
{
	m_hardware_type = 1;
}

DRIVER_INIT_MEMBER(fastfred_state,fastfred)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc800, 0xcfff, read8_delegate(FUNC(fastfred_state::fastfred_custom_io_r),this));
	m_maincpu->space(AS_PROGRAM).nop_write(0xc800, 0xcfff);
	m_hardware_type = 1;
}

DRIVER_INIT_MEMBER(fastfred_state,jumpcoas)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc800, 0xcfff, read8_delegate(FUNC(fastfred_state::jumpcoas_custom_io_r),this));
	m_maincpu->space(AS_PROGRAM).nop_write(0xc800, 0xcfff);
	m_hardware_type = 0;
}

DRIVER_INIT_MEMBER(fastfred_state,boggy84b)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc800, 0xcfff, read8_delegate(FUNC(fastfred_state::jumpcoas_custom_io_r),this));
	m_maincpu->space(AS_PROGRAM).nop_write(0xc800, 0xcfff);
	m_hardware_type = 2;
}

DRIVER_INIT_MEMBER(fastfred_state,boggy84)
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xc800, 0xcfff, read8_delegate(FUNC(fastfred_state::boggy84_custom_io_r),this));
	m_maincpu->space(AS_PROGRAM).nop_write(0xc800, 0xcfff);
	m_hardware_type = 2;
}


DRIVER_INIT_MEMBER(fastfred_state,imago)
{
	m_hardware_type = 3;
}

GAME( 1982, flyboy,   0,        fastfred, flyboy, fastfred_state,   flyboy,   ROT90, "Kaneko", "Fly-Boy", MACHINE_SUPPORTS_SAVE )
GAME( 1982, flyboyb,  flyboy,   fastfred, flyboy, fastfred_state,   flyboyb,  ROT90, "bootleg", "Fly-Boy (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, fastfred, flyboy,   fastfred, fastfred, fastfred_state, fastfred, ROT90, "Kaneko (Atari license)", "Fast Freddie", MACHINE_SUPPORTS_SAVE )
GAME( 1983, jumpcoas, 0,        jumpcoas, jumpcoas, fastfred_state, jumpcoas, ROT90, "Kaneko", "Jump Coaster", MACHINE_SUPPORTS_SAVE )
GAME( 1983, jumpcoast,jumpcoas, jumpcoas, jumpcoas, fastfred_state, jumpcoas, ROT90, "Kaneko (Taito license)", "Jump Coaster (Taito)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, boggy84,  0,        jumpcoas, boggy84, fastfred_state,  boggy84,  ROT90, "Kaneko", "Boggy '84", MACHINE_SUPPORTS_SAVE )
GAME( 1983, boggy84b, boggy84,  jumpcoas, boggy84, fastfred_state,  boggy84b, ROT90, "bootleg (Eddie's Games)", "Boggy '84 (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, redrobin, 0,        fastfred, redrobin, fastfred_state, flyboyb,  ROT90, "Elettronolo", "Red Robin", MACHINE_SUPPORTS_SAVE )
GAME( 1984, imago,    0,        imago,    imago, fastfred_state,    imago,    ROT90, "Acom", "Imago (cocktail set)", 0 )
GAME( 1983, imagoa,   imago,    imago,    imagoa, fastfred_state,   imago,    ROT90, "Acom", "Imago (no cocktail set)", 0 )
