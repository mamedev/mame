/***************************************************************************

Big Twin
World Beach Volley
Excelsior
Hot Mind
Hard Times

driver by Nicola Salmoria and Pierpaolo Prazzoli

The games run on different, but similar, hardware. The sprite system is the
same (almost - the tile size is different).

Even if some games are from the same year, World Beach Volley is much more
advanced - more colourful, and stores setting in an EEPROM.

An interesting thing about this hardware is that the same gfx ROMs are used
to generate both 8x8 and 16x16 tiles for different tilemaps.

Hard Times and Hot Mind have different tilemaps layout than the other ones.

Hard Times was hacked from Blood Bros. program code.

Hot Mind is a romswap kit for Hard Times pcb, in fact it was found in a pcb
marked as HARD TIMES 28-06-94.

The version of Big Twin without girls seems a conversion of Hard Times pcb.


Original Bugs:
- World Beach Volley histogram functions don't work.

TODO:
- World Beach Volley sound is controlled by a pic16c57 whose ROM is missing for this game.

- One stage in Hard Times has large white blocks instead of GFX in places, are they using an
  invalid tile number that should be invisible?

- In Hard Times the last boss appears on left side of screen as it scrolls into view, are we
  missing part of the X co-ordinate?

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eeprom.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "includes/playmark.h"


WRITE16_MEMBER(playmark_state::coinctrl_w)
{
	if (ACCESSING_BITS_8_15)
	{
		coin_counter_w(machine(), 0, data & 0x0100);
		coin_counter_w(machine(), 1, data & 0x0200);
	}
	if (data & 0xfcff)
		logerror("Writing %04x to unknown coin control bits\n", data);
}


/***************************************************************************

  EEPROM

***************************************************************************/

static const eeprom_interface eeprom_intf =
{
	6,				/* address bits */
	16,				/* data bits */
	"*110",			/*  read command */
	"*101",			/* write command */
	0,				/* erase command */
	"*10000xxxx",	/* lock command */
	"*10011xxxx",	/* unlock command */
	0,				/* enable_multi_read */
	5				/* reset_delay (otherwise wbeachvl will hang when saving settings) */
};

WRITE16_MEMBER(playmark_state::wbeachvl_coin_eeprom_w)
{

	if (ACCESSING_BITS_0_7)
	{
		/* bits 0-3 are coin counters? (only 0 used?) */
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);
		coin_counter_w(machine(), 2, data & 0x04);
		coin_counter_w(machine(), 3, data & 0x08);

		/* bits 5-7 control EEPROM */
		m_eeprom->set_cs_line((data & 0x20) ? CLEAR_LINE : ASSERT_LINE);
		m_eeprom->write_bit(data & 0x80);
		m_eeprom->set_clock_line((data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	}
}

WRITE16_MEMBER(playmark_state::hotmind_coin_eeprom_w)
{

	if (ACCESSING_BITS_0_7)
	{
		coin_counter_w(machine(), 0,data & 0x20);

		m_eeprom->set_cs_line((data & 1) ? CLEAR_LINE : ASSERT_LINE);
		m_eeprom->write_bit(data & 4);
		m_eeprom->set_clock_line((data & 2) ? ASSERT_LINE : CLEAR_LINE );
	}
}

WRITE16_MEMBER(playmark_state::hrdtimes_coin_w)
{
	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);
}

WRITE16_MEMBER(playmark_state::playmark_snd_command_w)
{

	if (ACCESSING_BITS_0_7)
	{
		m_snd_command = (data & 0xff);
		m_snd_flag = 1;
		device_yield(&space.device());
	}
}

READ8_MEMBER(playmark_state::playmark_snd_command_r)
{
	int data = 0;

	if ((m_oki_control & 0x38) == 0x30)
	{
		data = m_snd_command;
		// logerror("PC$%03x PortB reading %02x from the 68K\n", cpu_get_previouspc(&space.device()), data);
	}
	else if ((m_oki_control & 0x38) == 0x28)
	{
		data = (m_oki->read(space, 0) & 0x0f);
		// logerror("PC$%03x PortB reading %02x from the OKI status port\n", cpu_get_previouspc(&space.device()), data);
	}

	return data;
}

READ8_MEMBER(playmark_state::playmark_snd_flag_r)
{

	if (m_snd_flag)
	{
		m_snd_flag = 0;
		return 0x00;
	}

	return 0x40;
}


static WRITE8_DEVICE_HANDLER( playmark_oki_banking_w )
{
	playmark_state *state = device->machine().driver_data<playmark_state>();

	if (state->m_old_oki_bank != (data & 7))
	{
		state->m_old_oki_bank = data & 7;

		if (((state->m_old_oki_bank - 1) * 0x40000) < device->machine().region("oki")->bytes())
		{
			downcast<okim6295_device *>(device)->set_bank_base(0x40000 * (state->m_old_oki_bank - 1));
		}
	}
}

WRITE8_MEMBER(playmark_state::playmark_oki_w)
{
	m_oki_command = data;
}

WRITE8_MEMBER(playmark_state::playmark_snd_control_w)
{
//  address_&space space = device->machine().device("audiocpu")->memory().&space(AS_PROGRAM);

    /*  This port controls communications to and from the 68K, and the OKI
        device.

        bit legend
        7w  ???  (No read or writes to Port B)
        6r  Flag from 68K to notify the PIC that a command is coming
        5w  Latch write data to OKI? (active low)
        4w  Activate read signal to OKI? (active low)
        3w  Set Port 1 to read sound to play command from 68K. (active low)
        2w  ???  (Read Port B)
        1   Not used
        0   Not used
    */
	m_oki_control = data;

	if ((data & 0x38) == 0x18)
	{
		// logerror("PC$%03x Writing %02x to OKI1, PortC=%02x, Code=%02x\n",cpu_get_previouspc(&space.device()),playmark_oki_command,playmark_oki_control,playmark_snd_command);
		okim6295_device *oki = machine().device<okim6295_device>("oki");
		oki->write(space, 0, m_oki_command);
	}
}


READ8_MEMBER(playmark_state::PIC16C5X_T0_clk_r)
{
	return 0;
}


/***************************** 68000 Memory Maps ****************************/

static ADDRESS_MAP_START( bigtwin_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x304000, 0x304001) AM_NOP				/* watchdog? irq ack? */
	AM_RANGE(0x440000, 0x4403ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x500000, 0x500fff) AM_WRITE(wbeachvl_fgvideoram_w) AM_BASE(m_videoram2)
	AM_RANGE(0x501000, 0x501fff) AM_WRITENOP	/* unused RAM? */
	AM_RANGE(0x502000, 0x503fff) AM_WRITE(wbeachvl_txvideoram_w) AM_BASE(m_videoram1)
	AM_RANGE(0x504000, 0x50ffff) AM_WRITENOP	/* unused RAM? */
	AM_RANGE(0x510000, 0x51000b) AM_WRITE(bigtwin_scroll_w)
	AM_RANGE(0x51000c, 0x51000d) AM_WRITENOP	/* always 3? */
	AM_RANGE(0x600000, 0x67ffff) AM_RAM AM_BASE(m_bgvideoram)
	AM_RANGE(0x700010, 0x700011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x700012, 0x700013) AM_READ_PORT("P1")
	AM_RANGE(0x700014, 0x700015) AM_READ_PORT("P2")
	AM_RANGE(0x700016, 0x700017) AM_WRITE(coinctrl_w)
	AM_RANGE(0x70001a, 0x70001b) AM_READ_PORT("DSW1")
	AM_RANGE(0x70001c, 0x70001d) AM_READ_PORT("DSW2")
	AM_RANGE(0x70001e, 0x70001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x780000, 0x7807ff) AM_WRITE(bigtwin_paletteram_w) AM_SHARE("paletteram")
//  AM_RANGE(0xe00000, 0xe00001) ?? written on startup
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( bigtwinb_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(hrdtimes_bgvideoram_w) AM_BASE(m_videoram3)
	AM_RANGE(0x104000, 0x107fff) AM_RAM_WRITE(hrdtimes_fgvideoram_w) AM_BASE(m_videoram2)
	AM_RANGE(0x108000, 0x10ffff) AM_RAM_WRITE(hrdtimes_txvideoram_w) AM_BASE(m_videoram1)
	AM_RANGE(0x110000, 0x11000d) AM_WRITE(hrdtimes_scroll_w)
	AM_RANGE(0x201000, 0x2013ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x280000, 0x2807ff) AM_RAM_WRITE(bigtwin_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("P1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("P2")
	AM_RANGE(0x30001a, 0x30001b) AM_READ_PORT("DSW1")
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("DSW2")
	AM_RANGE(0x30001e, 0x30001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP		/* watchdog? irq ack? */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( wbeachvl_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(wbeachvl_bgvideoram_w) AM_BASE(m_videoram3)
	AM_RANGE(0x504000, 0x505fff) AM_RAM_WRITE(wbeachvl_fgvideoram_w) AM_BASE(m_videoram2)
	AM_RANGE(0x508000, 0x509fff) AM_RAM_WRITE(wbeachvl_txvideoram_w) AM_BASE(m_videoram1)
	AM_RANGE(0x50f000, 0x50ffff) AM_RAM AM_BASE(m_rowscroll)
	AM_RANGE(0x510000, 0x51000b) AM_WRITE(wbeachvl_scroll_w)
	AM_RANGE(0x51000c, 0x51000d) AM_WRITENOP	/* 2 and 3 */
//  AM_RANGE(0x700000, 0x700001) ?? written on startup
	AM_RANGE(0x710010, 0x710011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x710012, 0x710013) AM_READ_PORT("P1")
	AM_RANGE(0x710014, 0x710015) AM_READ_PORT("P2")
	AM_RANGE(0x710016, 0x710017) AM_WRITE(wbeachvl_coin_eeprom_w)
	AM_RANGE(0x710018, 0x710019) AM_READ_PORT("P3")
	AM_RANGE(0x71001a, 0x71001b) AM_READ_PORT("P4")
//  AM_RANGE(0x71001c, 0x71001d) AM_READ_LEGACY(playmark_snd_status???)
//  AM_RANGE(0x71001e, 0x71001f) AM_WRITENOP//playmark_snd_command_w },
	AM_RANGE(0x780000, 0x780fff) AM_WRITE(paletteram16_RRRRRGGGGGBBBBBx_word_w) AM_SHARE("paletteram")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( excelsr_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP				/* watchdog? irq ack? */
	AM_RANGE(0x440000, 0x440cff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(wbeachvl_fgvideoram_w) AM_BASE(m_videoram2)
	AM_RANGE(0x501000, 0x501fff) AM_RAM_WRITE(wbeachvl_txvideoram_w) AM_BASE(m_videoram1)
	AM_RANGE(0x510000, 0x51000b) AM_WRITE(excelsr_scroll_w)
	AM_RANGE(0x51000c, 0x51000d) AM_WRITENOP	/* 2 and 3 */
	AM_RANGE(0x600000, 0x67ffff) AM_RAM AM_BASE(m_bgvideoram)
	AM_RANGE(0x700010, 0x700011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x700012, 0x700013) AM_READ_PORT("P1")
	AM_RANGE(0x700014, 0x700015) AM_READ_PORT("P2")
	AM_RANGE(0x700016, 0x700017) AM_WRITE(coinctrl_w)
	AM_RANGE(0x70001a, 0x70001b) AM_READ_PORT("DSW1")
	AM_RANGE(0x70001c, 0x70001d) AM_READ_PORT("DSW2")
	AM_RANGE(0x70001e, 0x70001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x780000, 0x7807ff) AM_RAM_WRITE(bigtwin_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hotmind_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(hrdtimes_bgvideoram_w) AM_BASE(m_videoram3)
	AM_RANGE(0x104000, 0x107fff) AM_RAM_WRITE(hrdtimes_fgvideoram_w) AM_BASE(m_videoram2)
	AM_RANGE(0x108000, 0x10ffff) AM_RAM_WRITE(hrdtimes_txvideoram_w) AM_BASE(m_videoram1)
	AM_RANGE(0x110000, 0x11000d) AM_WRITE(hrdtimes_scroll_w)
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x280000, 0x2807ff) AM_RAM_WRITE(bigtwin_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("P1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("P2") AM_WRITE(hotmind_coin_eeprom_w)
	AM_RANGE(0x30001a, 0x30001b) AM_READ_PORT("DSW1")
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("DSW2")
	AM_RANGE(0x30001e, 0x30001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP		/* watchdog? irq ack? */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hrdtimes_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x0bffff) AM_RAM
	AM_RANGE(0x0c0000, 0x0fffff) AM_ROM AM_REGION("maincpu", 0x0c0000)
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(hrdtimes_bgvideoram_w) AM_BASE(m_videoram3)
	AM_RANGE(0x104000, 0x107fff) AM_RAM_WRITE(hrdtimes_fgvideoram_w) AM_BASE(m_videoram2)
	AM_RANGE(0x108000, 0x10ffff) AM_RAM_WRITE(hrdtimes_txvideoram_w) AM_BASE(m_videoram1)
	AM_RANGE(0x110000, 0x11000d) AM_WRITE(hrdtimes_scroll_w)
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x280000, 0x2807ff) AM_RAM_WRITE(bigtwin_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x280800, 0x280fff) AM_RAM // unused
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("P1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("P2")
	AM_RANGE(0x300016, 0x300017) AM_WRITE(hrdtimes_coin_w)
	AM_RANGE(0x30001a, 0x30001b) AM_READ_PORT("DSW1")
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("DSW2")
	AM_RANGE(0x30001e, 0x30001f) AM_WRITENOP //(playmark_snd_command_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP		/* watchdog? irq ack? */
ADDRESS_MAP_END



/***************************** PIC16C57 Memory Map **************************/

	/* $000 - 7FF  PIC16C57 Internal Program ROM. Note: code is 12bits wide */
	/* $000 - 07F  PIC16C57 Internal Data RAM */

static ADDRESS_MAP_START( playmark_sound_io_map, AS_IO, 8, playmark_state )
	AM_RANGE(0x00, 0x00) AM_DEVWRITE_LEGACY("oki", playmark_oki_banking_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(playmark_snd_command_r, playmark_oki_w)
	AM_RANGE(0x02, 0x02) AM_READWRITE(playmark_snd_flag_r, playmark_snd_control_w)
	AM_RANGE(PIC16C5x_T0, PIC16C5x_T0) AM_READ(PIC16C5X_T0_clk_r)
ADDRESS_MAP_END


static INPUT_PORTS_START( bigtwin )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Italian ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Censor Pictures" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( Easy ) ) /* Seems same as Medium */
	PORT_DIPSETTING(	0x30, DEF_STR( Medium ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x14, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Credits to Start" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bigtwinb )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Italian ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Difficulty ) )
//  PORT_DIPSETTING(    0x20, DEF_STR( Easy ) ) /* Seems same as Medium */
	PORT_DIPSETTING(	0x30, DEF_STR( Medium ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x14, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Credits to Start" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wbeachvl )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE(0x20, IP_ACTIVE_LOW)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )	/* ?? see code at 746a. sound status? */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)	/* EEPROM data */

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START3 )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START4 )
INPUT_PORTS_END

static INPUT_PORTS_START( excelsr )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x0c, 0x00, "Censor Pictures" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, "50%" )
	PORT_DIPSETTING(    0x0c, "100%" )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x14, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Credits to Start" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "Percentage to Reveal" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static INPUT_PORTS_START( hotmind )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_VBLANK )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_device, read_bit)	/* EEPROM data */

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, "Very Hard 5" )
	PORT_DIPSETTING(    0x01, "Very Hard 4" )
	PORT_DIPSETTING(    0x02, "Very Hard 3" )
	PORT_DIPSETTING(    0x03, "Very Hard 2" )
	PORT_DIPSETTING(    0x04, "Very Hard 1" )
	PORT_DIPSETTING(    0x05, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x07, DEF_STR( Easy ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Erogatore Gettoni" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Erogatore Ticket" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Clear Memory" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x14, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
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

static INPUT_PORTS_START( hrdtimes )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x0c, "Every 300k - 500k" )
	PORT_DIPSETTING(    0x08, "Every 500k - 500k" )
	PORT_DIPSETTING(    0x04, "Only 500k" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_EQUALS, 0x01)
	PORT_DIPSETTING(    0x14, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW2", 0x01, PORTCOND_NOTEQUALS, 0x01)
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Credits to Start" )
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "1 Life If Continue" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	32*8
};


static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static const gfx_layout spritelayout =
{
	32,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
			48*8+0, 48*8+1, 48*8+2, 48*8+3, 48*8+4, 48*8+5, 48*8+6, 48*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
			64*8, 65*8, 66*8, 67*8, 68*8, 69*8, 70*8, 71*8,
			72*8, 73*8, 74*8, 75*8, 76*8, 77*8, 78*8, 79*8 },
	128*8
};

static GFXDECODE_START( playmark )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x200, 16 )	/* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0x000,  8 )	/* colors 0x000-0x07f */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x080,  8 )	/* colors 0x080-0x0ff */
	/* background bitmap uses colors 0x100-0x1ff */
GFXDECODE_END


static const gfx_layout wcharlayout =
{
	8,8,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(5,6), RGN_FRAC(4,6), RGN_FRAC(3,6), RGN_FRAC(2,6), RGN_FRAC(1,6), RGN_FRAC(0,6) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout wtilelayout =
{
	16,16,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(5,6), RGN_FRAC(4,6), RGN_FRAC(3,6), RGN_FRAC(2,6), RGN_FRAC(1,6), RGN_FRAC(0,6) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

/* tiles are 6 bpp, sprites only 5bpp */
static const gfx_layout wspritelayout =
{
	16,16,
	RGN_FRAC(1,6),
	5,
	{ RGN_FRAC(4,6), RGN_FRAC(3,6), RGN_FRAC(2,6), RGN_FRAC(1,6), RGN_FRAC(0,6) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8
};

static GFXDECODE_START( wbeachvl )
	GFXDECODE_ENTRY( "gfx1", 0, wspritelayout, 0x600, 16 )	/* colors 0x600-0x7ff */
	GFXDECODE_ENTRY( "gfx1", 0, wtilelayout,   0x000, 16 )	/* colors 0x000-0x3ff */
	GFXDECODE_ENTRY( "gfx1", 0, wcharlayout,   0x400,  8 )	/* colors 0x400-0x5ff */
GFXDECODE_END

static GFXDECODE_START( excelsr )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0x200, 16 )	/* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0x000,  8 )	/* colors 0x000-0x07f */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0x080,  8 )	/* colors 0x080-0x0ff */
	/* background bitmap uses colors 0x100-0x1ff */
GFXDECODE_END

static const gfx_layout hrdtimes_tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(2,4)+8, RGN_FRAC(2,4), RGN_FRAC(0,4)+8, RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+4, 16*16+5, 16*16+6, 16*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
	8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	32*16
};

static const gfx_layout hrdtimes_charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(2,4)+8, RGN_FRAC(2,4), RGN_FRAC(0,4)+8, RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static GFXDECODE_START( hrdtimes )
	GFXDECODE_ENTRY( "gfx2", 0, hrdtimes_tilelayout, 0x200, 32 )	/* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx1", 0, hrdtimes_tilelayout, 0x000, 16 )	/* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx1", 0, hrdtimes_charlayout, 0x100,  8 )	/* colors 0x100-0x17f */
GFXDECODE_END

static GFXDECODE_START( bigtwinb )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,		 0x300, 16 )	/* colors 0x300-0x3ff */
	GFXDECODE_ENTRY( "gfx1", 0, hrdtimes_tilelayout, 0x000, 16 )	/* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx1", 0, hrdtimes_charlayout, 0x200, 16 )	/* colors 0x200-0x2ff */
GFXDECODE_END

static MACHINE_START( playmark )
{
	playmark_state *state = machine.driver_data<playmark_state>();

	state->m_oki = machine.device<okim6295_device>("oki");
	state->m_eeprom = machine.device<eeprom_device>("eeprom");

	state->save_item(NAME(state->m_bgscrollx));
	state->save_item(NAME(state->m_bgscrolly));
	state->save_item(NAME(state->m_bg_enable));
	state->save_item(NAME(state->m_bg_full_size));
	state->save_item(NAME(state->m_fgscrollx));
	state->save_item(NAME(state->m_fg_rowscroll_enable));
	state->save_item(NAME(state->m_scroll));

	state->save_item(NAME(state->m_snd_command));
	state->save_item(NAME(state->m_snd_flag));
	state->save_item(NAME(state->m_oki_control));
	state->save_item(NAME(state->m_oki_command));
	state->save_item(NAME(state->m_old_oki_bank));
}

static MACHINE_RESET( playmark )
{
	playmark_state *state = machine.driver_data<playmark_state>();

	state->m_bgscrollx = 0;
	state->m_bgscrolly = 0;
	state->m_bg_enable = 0;
	state->m_bg_full_size = 0;
	state->m_fgscrollx = 0;
	state->m_fg_rowscroll_enable = 0;
	memset(state->m_scroll, 0, ARRAY_LENGTH(state->m_scroll));

	state->m_snd_command = 0;
	state->m_oki_control = 0;
	state->m_oki_command = 0;
	state->m_old_oki_bank = 0;
}

static MACHINE_CONFIG_START( bigtwin, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)	/* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(bigtwin_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, 12000000)	/* 3MHz */
	/* Program and Data Maps are internal to the MCU */
	MCFG_CPU_IO_MAP(playmark_sound_io_map)

	MCFG_MACHINE_START(playmark)
	MCFG_MACHINE_RESET(playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(bigtwin)

	MCFG_GFXDECODE(playmark)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(bigtwin)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bigtwinb, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)	/* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(bigtwinb_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, 12000000)	/* 3MHz */
	/* Program and Data Maps are internal to the MCU */
	MCFG_CPU_IO_MAP(playmark_sound_io_map)

	MCFG_MACHINE_START(playmark)
	MCFG_MACHINE_RESET(playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(bigtwinb)

	MCFG_GFXDECODE(bigtwinb)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(bigtwinb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( wbeachvl, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)	/* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(wbeachvl_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

//  MCFG_CPU_ADD("audiocpu", PIC16C57, 12000000)   /* 3MHz */
	/* Program and Data Maps are internal to the MCU */
//  MCFG_CPU_IO_MAP(playmark_sound_io_map)

	MCFG_EEPROM_ADD("eeprom", eeprom_intf)
	MCFG_EEPROM_DEFAULT_VALUE(0)

	MCFG_MACHINE_START(playmark)
	MCFG_MACHINE_RESET(playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(wbeachvl)

	MCFG_GFXDECODE(wbeachvl)
	MCFG_PALETTE_LENGTH(2048)

	MCFG_VIDEO_START(wbeachvl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( excelsr, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)	/* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(excelsr_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, 12000000)	/* 3MHz */
	/* Program and Data Maps are internal to the MCU */
	MCFG_CPU_IO_MAP(playmark_sound_io_map)

	MCFG_MACHINE_START(playmark)
	MCFG_MACHINE_RESET(playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_STATIC(excelsr)

	MCFG_GFXDECODE(excelsr)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(excelsr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hotmind, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)	/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(hotmind_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_24MHz/2)	/* verified on pcb */
	/* Program and Data Maps are internal to the MCU */
	MCFG_CPU_IO_MAP(playmark_sound_io_map)

	MCFG_EEPROM_ADD("eeprom", eeprom_intf)
	MCFG_EEPROM_DEFAULT_VALUE(0)

	MCFG_MACHINE_START(playmark)
	MCFG_MACHINE_RESET(playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(hrdtimes)

	MCFG_GFXDECODE(hrdtimes)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(hotmind)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH)  /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hrdtimes, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)	/* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(hrdtimes_main_map)
	MCFG_CPU_VBLANK_INT("screen", irq6_line_hold)

//  MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_24MHz/2)    /* verified on pcb */
	/* Program and Data Maps are internal to the MCU */
//  MCFG_CPU_IO_MAP(playmark_sound_io_map)

	MCFG_MACHINE_START(playmark)
	MCFG_MACHINE_RESET(playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(hrdtimes)

	MCFG_GFXDECODE(hrdtimes)
	MCFG_PALETTE_LENGTH(1024)

	MCFG_VIDEO_START(hrdtimes)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bigtwin )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "2.302",        0x000000, 0x80000, CRC(e6767f60) SHA1(ec0ba1c786e6fde04601c2f3f619e3c6545f9239) )
	ROM_LOAD16_BYTE( "3.301",        0x000001, 0x80000, CRC(5aba6990) SHA1(4f664a91819fdd27821fa607425701d83fcbd8ce) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (PIC16C57) */
//  ROM_LOAD( "16c57hs.bin",  0x0000, 0x1000, CRC(b4c95cc3) SHA1(7fc9b141e7782aa5c17310ee06db99d884537c30) )
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "16c57hs.015",  0x0000, 0x2d4c, CRC(c07e9375) SHA1(7a6714ab888ea6e37bc037bc7419f0998868cfce) )	/* 16C57 .HEX dump, to be converted */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "4.311",        0x00000, 0x40000, CRC(6f628fbc) SHA1(51cdee457aef79fef5d89d30a173afdf13fbb2ef) )
	ROM_LOAD( "5.312",        0x40000, 0x40000, CRC(6a9b1752) SHA1(7c78157cd6b3d631704d2aca1a5756c69c87d581) )
	ROM_LOAD( "6.313",        0x80000, 0x40000, CRC(411cf852) SHA1(1b66cec672b6ec6974d9e82afc6ec58b78c92ee4) )
	ROM_LOAD( "7.314",        0xc0000, 0x40000, CRC(635c81fd) SHA1(64c787a37fcd1ba7c747ec25ff5b949aad3914ec) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "8.321",        0x00000, 0x20000, CRC(2749644d) SHA1(f506ed1a14ee411eda8a7e639f5572e35b89b13f) )
	ROM_LOAD( "9.322",        0x20000, 0x20000, CRC(1d1897af) SHA1(0ad00906b94443d7ceef383717b39c6aa8cca241) )
	ROM_LOAD( "10.323",       0x40000, 0x20000, CRC(2a03432e) SHA1(44722b83093211d88460cbcd9e9c0b638d24ad3e) )
	ROM_LOAD( "11.324",       0x60000, 0x20000, CRC(2c980c4c) SHA1(77af29a1f5d4302650915f4a7daf2918a2519a6e) )

	ROM_REGION( 0x40000, "oki", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "1.013",        0x00000, 0x40000, CRC(ff6671dc) SHA1(517941946a3edfc2da0b7aa8a106ebb4ae849beb) )
ROM_END


ROM_START( bigtwinb )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "2.u67", 0x00000, 0x20000, CRC(f5cdf1a9) SHA1(974328cf2b4ec5834a519e3300ee1ad8bc4d5c04) )
	ROM_LOAD16_BYTE( "3.u66", 0x00001, 0x20000, CRC(084e990f) SHA1(d7c2e08c7f7c7b453dd19dcf1f30bad46d943c8a) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (PIC16C57) */
//  ROM_LOAD( "16c57hs.bin",  0x0000, 0x1000, CRC(b4c95cc3) SHA1(7fc9b141e7782aa5c17310ee06db99d884537c30) )
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "16c57hs.015",  0x0000, 0x2d4c, CRC(c07e9375) SHA1(7a6714ab888ea6e37bc037bc7419f0998868cfce) )	/* 16C57 .HEX dump, to be converted */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "4.u36", 0x000000, 0x20000, CRC(99aaeacc) SHA1(0281237722d5a94fb9831616ae2ffc8288e78e2c) )
	ROM_CONTINUE(			  0x080000, 0x20000 )
	ROM_LOAD16_BYTE( "5.u42", 0x000001, 0x20000, CRC(5c1dfd72) SHA1(31fab4d3bd4e8ff5a16daeaff0ccaa4fc8f60c92) )
	ROM_CONTINUE(			  0x080001, 0x20000 )
	ROM_LOAD16_BYTE( "6.u39", 0x100000, 0x20000, CRC(788f2df6) SHA1(186f4f9f79c80dc5c6faa9eddc4b3c98b52b374d) )
	ROM_CONTINUE(			  0x180000, 0x20000 )
	ROM_LOAD16_BYTE( "7.u45", 0x100001, 0x20000, CRC(aedb2e6d) SHA1(775e13d328c8ee3c36b9d77ad49fa5a092b85a95) )
	ROM_CONTINUE(			  0x180001, 0x20000 )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "11.u86",       0x00000, 0x20000, CRC(2749644d) SHA1(f506ed1a14ee411eda8a7e639f5572e35b89b13f) )
	ROM_LOAD( "10.u85",       0x20000, 0x20000, CRC(1d1897af) SHA1(0ad00906b94443d7ceef383717b39c6aa8cca241) )
	ROM_LOAD( "9.u84",        0x40000, 0x20000, CRC(2a03432e) SHA1(44722b83093211d88460cbcd9e9c0b638d24ad3e) )
	ROM_LOAD( "8.u83",        0x60000, 0x20000, CRC(2c980c4c) SHA1(77af29a1f5d4302650915f4a7daf2918a2519a6e) )

	ROM_REGION( 0x40000, "oki", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "io13.bin",     0x00000, 0x40000, CRC(ff6671dc) SHA1(517941946a3edfc2da0b7aa8a106ebb4ae849beb) )
ROM_END

ROM_START( wbeachvl )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "wbv_02.bin",   0x000000, 0x40000, CRC(c7cca29e) SHA1(03af361081d688c4204a95f7f5babcc598b72c23) )
	ROM_LOAD16_BYTE( "wbv_03.bin",   0x000001, 0x40000, CRC(db4e69d5) SHA1(119bf35a463d279ddde67ab08f6f1bab9f05cf0c) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (missing) */
	ROM_LOAD( "pic16c57",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD( "wbv_10.bin",   0x000000, 0x80000, CRC(50680f0b) SHA1(ed76ef6ced70ba7e9558162aa94bbe9f19bbabe6) )
	ROM_LOAD( "wbv_04.bin",   0x080000, 0x80000, CRC(df9cbff1) SHA1(7197939d9c4e8666d37266b6326134cfb4c761da) )
	ROM_LOAD( "wbv_11.bin",   0x100000, 0x80000, CRC(e59ad0d1) SHA1(70dfc1ea45246fc8e24c96550563ab7a983f3824) )
	ROM_LOAD( "wbv_05.bin",   0x180000, 0x80000, CRC(51245c3c) SHA1(5ac27d6fc22555766b4cdd532210199f4d7bd8bb) )
	ROM_LOAD( "wbv_12.bin",   0x200000, 0x80000, CRC(36b87d0b) SHA1(702b8139d150c7cc9399dfa38536567aab40dcef) )
	ROM_LOAD( "wbv_06.bin",   0x280000, 0x80000, CRC(9eb808ef) SHA1(0e46557665f1acef0606f22f043a391d1086cfce) )
	ROM_LOAD( "wbv_13.bin",   0x300000, 0x80000, CRC(7021107b) SHA1(088fe3060dbb196e8000a3b4db1cfa3cb0c4b677) )
	ROM_LOAD( "wbv_07.bin",   0x380000, 0x80000, CRC(4fff9fe8) SHA1(e29d3b4895692fd8559c9018432f32170aecdcc3) )
	ROM_LOAD( "wbv_14.bin",   0x400000, 0x80000, CRC(0595e675) SHA1(82aebaedc919fa51b71f5519ee765ce9953d613a) )
	ROM_LOAD( "wbv_08.bin",   0x480000, 0x80000, CRC(07e4b416) SHA1(a780ef0bd11897ab437359985f6e4852030ddbbf) )
	ROM_LOAD( "wbv_15.bin",   0x500000, 0x80000, CRC(4e1a82d2) SHA1(9e66b52ba8e8144f772183396fc1a2fbb37ed2bc) )
	ROM_LOAD( "wbv_09.bin",   0x580000, 0x20000, CRC(894ce354) SHA1(331aeabbe10cd645776da2dc0829acc2275e72dc) )
	/* 5a0000-5fffff is empty */

	ROM_REGION( 0x100000, "user2", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "wbv_01.bin",   0x00000, 0x100000, CRC(ac33f25f) SHA1(5d9ed16650aeb297d565376a99b31c88ab611668) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x1c0000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x080000, 0x0e0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x100000, 0x020000)
	ROM_COPY( "user2", 0x0a0000, 0x120000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x140000, 0x020000)
	ROM_COPY( "user2", 0x0c0000, 0x160000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x180000, 0x020000)
	ROM_COPY( "user2", 0x0e0000, 0x1a0000, 0x020000)
ROM_END

ROM_START( wbeachvl2 )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "2.bin",   0x000000, 0x40000, CRC(8993487e) SHA1(c927ae655807f9046f66ff96a30bd2c6fa671566) )
	ROM_LOAD16_BYTE( "3.bin",   0x000001, 0x40000, CRC(15904789) SHA1(640c80bbf7302529e1a39c2ae60e018ecb176478) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (missing) */
	ROM_LOAD( "pic16c57",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD( "wbv_10.bin",   0x000000, 0x80000, CRC(50680f0b) SHA1(ed76ef6ced70ba7e9558162aa94bbe9f19bbabe6) )
	ROM_LOAD( "wbv_04.bin",   0x080000, 0x80000, CRC(df9cbff1) SHA1(7197939d9c4e8666d37266b6326134cfb4c761da) )
	ROM_LOAD( "wbv_11.bin",   0x100000, 0x80000, CRC(e59ad0d1) SHA1(70dfc1ea45246fc8e24c96550563ab7a983f3824) )
	ROM_LOAD( "wbv_05.bin",   0x180000, 0x80000, CRC(51245c3c) SHA1(5ac27d6fc22555766b4cdd532210199f4d7bd8bb) )
	ROM_LOAD( "wbv_12.bin",   0x200000, 0x80000, CRC(36b87d0b) SHA1(702b8139d150c7cc9399dfa38536567aab40dcef) )
	ROM_LOAD( "wbv_06.bin",   0x280000, 0x80000, CRC(9eb808ef) SHA1(0e46557665f1acef0606f22f043a391d1086cfce) )
	ROM_LOAD( "wbv_13.bin",   0x300000, 0x80000, CRC(7021107b) SHA1(088fe3060dbb196e8000a3b4db1cfa3cb0c4b677) )
	ROM_LOAD( "wbv_07.bin",   0x380000, 0x80000, CRC(4fff9fe8) SHA1(e29d3b4895692fd8559c9018432f32170aecdcc3) )
	ROM_LOAD( "wbv_14.bin",   0x400000, 0x80000, CRC(0595e675) SHA1(82aebaedc919fa51b71f5519ee765ce9953d613a) )
	ROM_LOAD( "wbv_08.bin",   0x480000, 0x80000, CRC(07e4b416) SHA1(a780ef0bd11897ab437359985f6e4852030ddbbf) )
	ROM_LOAD( "wbv_15.bin",   0x500000, 0x80000, CRC(4e1a82d2) SHA1(9e66b52ba8e8144f772183396fc1a2fbb37ed2bc) )
	ROM_LOAD( "wbv_09.bin",   0x580000, 0x20000, CRC(894ce354) SHA1(331aeabbe10cd645776da2dc0829acc2275e72dc) )
	/* 5a0000-5fffff is empty */

	ROM_REGION( 0x100000, "user2", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "wbv_01.bin",   0x00000, 0x100000, CRC(ac33f25f) SHA1(5d9ed16650aeb297d565376a99b31c88ab611668) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x1c0000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x080000, 0x0e0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x100000, 0x020000)
	ROM_COPY( "user2", 0x0a0000, 0x120000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x140000, 0x020000)
	ROM_COPY( "user2", 0x0c0000, 0x160000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x180000, 0x020000)
	ROM_COPY( "user2", 0x0e0000, 0x1a0000, 0x020000)
ROM_END


ROM_START( wbeachvl3 )
	ROM_REGION( 0x80000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "2.u16",   0x000000, 0x40000, CRC(f0f4c282) SHA1(94850b45368c3d09629852adc8ca08164b7a7a94) )
	ROM_LOAD16_BYTE( "3.u15",   0x000001, 0x40000, CRC(99775c21) SHA1(fa80a81c59142abcf751352d7a7f9e0d3b5172c9) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (missing) */
	ROM_LOAD( "pic16c57",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x600000, "gfx1", 0 )
	ROM_LOAD( "wbv_10.bin",   0x000000, 0x80000, CRC(50680f0b) SHA1(ed76ef6ced70ba7e9558162aa94bbe9f19bbabe6) )
	ROM_LOAD( "wbv_04.bin",   0x080000, 0x80000, CRC(df9cbff1) SHA1(7197939d9c4e8666d37266b6326134cfb4c761da) )
	ROM_LOAD( "wbv_11.bin",   0x100000, 0x80000, CRC(e59ad0d1) SHA1(70dfc1ea45246fc8e24c96550563ab7a983f3824) )
	ROM_LOAD( "wbv_05.bin",   0x180000, 0x80000, CRC(51245c3c) SHA1(5ac27d6fc22555766b4cdd532210199f4d7bd8bb) )
	ROM_LOAD( "wbv_12.bin",   0x200000, 0x80000, CRC(36b87d0b) SHA1(702b8139d150c7cc9399dfa38536567aab40dcef) )
	ROM_LOAD( "wbv_06.bin",   0x280000, 0x80000, CRC(9eb808ef) SHA1(0e46557665f1acef0606f22f043a391d1086cfce) )
	ROM_LOAD( "wbv_13.bin",   0x300000, 0x80000, CRC(7021107b) SHA1(088fe3060dbb196e8000a3b4db1cfa3cb0c4b677) )
	ROM_LOAD( "wbv_07.bin",   0x380000, 0x80000, CRC(4fff9fe8) SHA1(e29d3b4895692fd8559c9018432f32170aecdcc3) )
	ROM_LOAD( "wbv_14.bin",   0x400000, 0x80000, CRC(0595e675) SHA1(82aebaedc919fa51b71f5519ee765ce9953d613a) )
	ROM_LOAD( "wbv_08.bin",   0x480000, 0x80000, CRC(07e4b416) SHA1(a780ef0bd11897ab437359985f6e4852030ddbbf) )
	ROM_LOAD( "wbv_15.bin",   0x500000, 0x80000, CRC(4e1a82d2) SHA1(9e66b52ba8e8144f772183396fc1a2fbb37ed2bc) )
	ROM_LOAD( "wbv_09.bin",   0x580000, 0x20000, CRC(894ce354) SHA1(331aeabbe10cd645776da2dc0829acc2275e72dc) )
	/* 5a0000-5fffff is empty */

	ROM_REGION( 0x100000, "user2", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "wbv_01.bin",   0x00000, 0x100000, CRC(ac33f25f) SHA1(5d9ed16650aeb297d565376a99b31c88ab611668) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x1c0000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0a0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x0c0000, 0x020000)
	ROM_COPY( "user2", 0x080000, 0x0e0000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x100000, 0x020000)
	ROM_COPY( "user2", 0x0a0000, 0x120000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x140000, 0x020000)
	ROM_COPY( "user2", 0x0c0000, 0x160000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x180000, 0x020000)
	ROM_COPY( "user2", 0x0e0000, 0x1a0000, 0x020000)
ROM_END



ROM_START( excelsr )
	ROM_REGION( 0x300000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "22.u301", 0x000001, 0x80000, CRC(f0aa1c1b) SHA1(5ed68181defe6cde6f4979508f0cfce9e9743912) )
	ROM_LOAD16_BYTE( "19.u302", 0x000000, 0x80000, CRC(9a8acddc) SHA1(c7868317998bb98c630685a0b242ffd1fbdc54ed) )
	ROM_LOAD16_BYTE( "21.u303", 0x100001, 0x80000, CRC(fdf9bd64) SHA1(783e3b8b70f8751915715e2455990c1c8eec6a71) )
	ROM_LOAD16_BYTE( "18.u304", 0x100000, 0x80000, CRC(fe517e0e) SHA1(fa074c3848046b59f1026f9ce1f264b49560668d) )
	ROM_LOAD16_BYTE( "20.u305", 0x200001, 0x80000, CRC(8692afe9) SHA1(b4411bad64a9a6efd8eb13dcf7c5eebfb5681f3d) )
	ROM_LOAD16_BYTE( "17.u306", 0x200000, 0x80000, CRC(978f9a6b) SHA1(9514b97f071fd20740218a58af877765beffedad) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (PIC16C57) */
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "pic16c57-hs.i015", 0x0000, 0x2d4c, CRC(022c6941) SHA1(8ead40bfa7aa783b1ce62bd6cfa673cb876e29e7) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "26.u311",      0x000000, 0x80000, CRC(c171c059) SHA1(7bc45ef1d588f5f55a461adb91bca382155c1059) )
	ROM_LOAD( "30.u312",      0x080000, 0x80000, CRC(b4a4c510) SHA1(07951a4c18bb25b10f650fd85b6bab566d0ef971) )
	ROM_LOAD( "25.u313",      0x100000, 0x80000, CRC(667eec1b) SHA1(9e5ed82a4966244a97d18c27466179771012b305) )
	ROM_LOAD( "29.u314",      0x180000, 0x80000, CRC(4acb0745) SHA1(6b5feaa5aa088f0cc5799f73ee5f90ed390165a9) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "24.u321",      0x000000, 0x80000, CRC(17f46825) SHA1(6ac0e71498ac668641c0b7134ddd19cc4cc97005) )
	ROM_LOAD( "28.u322",      0x080000, 0x80000, CRC(a823f2bd) SHA1(c7f1b1ee8f7069522787b6916b8c6e4591b55782) )
	ROM_LOAD( "23.u323",      0x100000, 0x80000, CRC(d8e1453b) SHA1(a3edb05abe486d4cce30f5caf14be619b6886f7c) )
	ROM_LOAD( "27.u324",      0x180000, 0x80000, CRC(eca2c079) SHA1(a07957b427d55c8ca1efb0e83ee3b603f06bed58) )

	ROM_REGION( 0x80000, "user2", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "16.i013",      0x000000, 0x80000, CRC(7ed9da5d) SHA1(352f1e89613feb1902b6d87adb996ed1c1d8108e) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0xc0000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0a0000, 0x020000)
ROM_END

/*

Hot Mind
Playmark, 1995

PCB Layout
----------
HARD TIMES 28-06-94
      |---------------------------------------------------------|
      |        ROM20                                            |
      |                                   PAL                   |
      |        M6295                      PAL      ROM23  ROM27 |
      |        1MHz               PAL              ROM24  ROM28 |
|-----|               PIC16C57            62256                 |
|     |                                   62256                 |
|93C46|J       6116                                ROM25  ROM29 |
|     |A       6116                     |--------| ROM26  ROM30 |
|     |M                                |TPC1020 |              |
|J    |M                                |AFN-084C|              |
|A    |A                                |        |              |
|M    |                       26MHz     |--------|              |
|M    |                                                         |
|A    |  DSW1                                                   |
|     |                                                         |
|     |  DSW2        PAL                                        |
|     |                                                         |
|-----|                  68000                            6116  |
      |                                   6116            6116  |
      |  62256    62256                   6116      PAL         |
      |                  24MHz            6116                  |
      |  ROM21    ROM22                   6116                  |
      |---------------------------------------------------------|
Notes:
      68000 CPU clock - 12.000MHz [24/2]
      M6295 clock     - 1.000MHz. Sample rate = 1000000/132
      PIC16C57 clock  - OCS1/CLKIN 12.000MHz (on pin 27, but internally divided by 4 at 3.000MHz)
                        Note PIC is secured, contents can not be read out.
      VSync           - 58Hz
      HSync           - 14.25kHz
*/

ROM_START( hotmind )
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "21.u87",       0x00000, 0x20000, CRC(e9000f7f) SHA1(c19fee7b774d3f30f4d4025a63ec396ec119c855) )
	ROM_LOAD16_BYTE( "22.u68",       0x00001, 0x20000, CRC(2c518ec5) SHA1(6d9e81ddb5793d64e22dc0254519b947f6ec6954) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (PIC16C57) */
	/* ROM will be copied here by the init code from "user1" */

	/* original PIC (which is the one from Hard Times) was protected, but it works with the Excelsior one
       because it uses only 1 bank of samples */
	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "pic16c57-hs.i015", 0x0000, 0x2d4c, BAD_DUMP CRC(022c6941) SHA1(8ead40bfa7aa783b1ce62bd6cfa673cb876e29e7) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "23.u36",       0x000000, 0x10000, CRC(ddcf60b9) SHA1(0c0fbc44131cb7d36c21bf5aead87b498c5684f5) )
	ROM_CONTINUE(			  0x080000, 0x10000 )
	ROM_LOAD16_BYTE( "27.u42",       0x000001, 0x10000, CRC(413bbcf4) SHA1(d82ae9d26df1a69b760b3025048e47ab757d9175) )
	ROM_CONTINUE(			  0x080001, 0x10000 )
	ROM_LOAD16_BYTE( "24.u39",       0x100000, 0x10000, CRC(4baa5b4c) SHA1(ee953ed9a4a45715d1ae39b5bb8b9b6505a4e95d) )
	ROM_CONTINUE(			  0x180000, 0x10000 )
	ROM_LOAD16_BYTE( "28.u49",       0x100001, 0x10000, CRC(8df34d6a) SHA1(ca0d2ca7e0f2a302bc8b1a03c0c18ac72fe105ac) )
	ROM_CONTINUE(			  0x180001, 0x10000 )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "26.u34",       0x00000, 0x20000, CRC(ff8d3b75) SHA1(5427b70a61dee4c125877e040be21cb1cadb1af5) )
	ROM_LOAD16_BYTE( "30.u85",       0x00001, 0x20000, CRC(87a640c7) SHA1(818ff3243cb3ed0189988348e6c2e954f0d3dd4f) )
	ROM_LOAD16_BYTE( "25.u35",       0x40000, 0x20000, CRC(c4fd4445) SHA1(ab0c5a328a312740595b5c92a1050527140518f3) )
	ROM_LOAD16_BYTE( "29.u83",       0x40001, 0x20000, CRC(0bebfb53) SHA1(d4342f808141b70af98c370004153a31d120e2a4) )

	ROM_REGION( 0xc0000, "oki", 0 ) /* Samples */
	ROM_LOAD( "20.io13",      0x00000, 0x40000, CRC(0bf3a3e5) SHA1(2ae06f37a6bcd20bc5fbaa90d970aba2ebf3cf5a) )
ROM_END

ROM_START( hrdtimes )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "31.u67",       0x00000, 0x80000, CRC(53eb041b) SHA1(7437da1ceb26e9518a3085560b8a42f37e77ace9) )
	ROM_LOAD16_BYTE( "32.u66",       0x00001, 0x80000, CRC(f2c6b382) SHA1(d73affed091a261c4bfe17f409657e0a46b6c163) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (PIC16C57) */
	ROM_LOAD( "pic16c57",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "33.u36",       0x000000, 0x80000, CRC(d1239ce5) SHA1(8e966a39a47f66c5e904ec4357c751e896ed47cb) )
	ROM_LOAD16_BYTE( "37.u42",       0x000001, 0x80000, CRC(aa692005) SHA1(1e274da358a25ceebdc71cb8f7228ef39348a895) )
	ROM_LOAD16_BYTE( "34.u39",       0x100000, 0x80000, CRC(e4108c59) SHA1(15f7b53a7bbdc4aefdae31a00be64c419326bfd1) )
	ROM_LOAD16_BYTE( "38.u45",       0x100001, 0x80000, CRC(ff7cacf3) SHA1(5ed93e86fe3b0b594bdd62e314cd9e2ffd3c2a2a) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "36.u86",       0x000000, 0x80000, CRC(f2fc1ca3) SHA1(f70913d9b89338932e62ca6bb60e5f5e412d7f64) )
	ROM_LOAD16_BYTE( "40.u85",       0x000001, 0x80000, CRC(368c15f4) SHA1(8ae95fd672448921964c4d0312d7366903362e27) )
	ROM_LOAD16_BYTE( "35.u84",       0x100000, 0x80000, CRC(7bde46ec) SHA1(1d26d268e1fc937e23ae7d93a1f86386b899a0c2) )
	ROM_LOAD16_BYTE( "39.u83",       0x100001, 0x80000, CRC(a0bae586) SHA1(0b2bb0c5c51b2717b820f0176d5775df21652667) )

	ROM_REGION( 0x80000, "user2", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "30.id13",      0x00000, 0x80000, CRC(fa5e50ae) SHA1(f3bd87c83fca9269cc2f19db1fbf55540c96f931) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0xc0000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0a0000, 0x020000)
ROM_END

/* Different revision of the PCB, uses larger gfx ROMs, however the content is the same */

ROM_START( hrdtimesa )
	ROM_REGION( 0x100000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "u67.bin",       0x00000, 0x80000, CRC(3e1334cb) SHA1(9523c04f92371a35c297280b42b1604e23790a1e) )
	ROM_LOAD16_BYTE( "u66.bin",       0x00001, 0x80000, CRC(041ec30a) SHA1(00476ebd0a64cbd027be159cae7666d2df6d11ba) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 )	/* sound (PIC16C57) */
	ROM_LOAD( "pic16c57",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "fh1_playmark_ht", 0x000000, 0x100000, CRC(3cca02b0) SHA1(22c57f4192bf81dd26caa6adfb1c80665bdc305c) )
	ROM_LOAD( "fh2_playmark_th", 0x100000, 0x100000, CRC(ed699acd) SHA1(23cf1da4e7462f7434e946a80bdd6df0395b3059) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "mh1_playmark_ht", 0x000000, 0x100000, CRC(927e5989) SHA1(b01444a3ff57cc2e10594e23c0343c956ed3ee32) )
	ROM_LOAD( "mh2_playmark_ht", 0x100000, 0x100000, CRC(e76f001b) SHA1(217c06ca3618275c22e33cfe318ec6c970d4862c) )

	ROM_REGION( 0x80000, "user2", 0 )	/* OKIM6295 samples */
	ROM_LOAD( "io13.bin",      0x00000, 0x80000, CRC(fa5e50ae) SHA1(f3bd87c83fca9269cc2f19db1fbf55540c96f931) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0xc0000, "oki", 0 ) /* Samples */
	ROM_COPY( "user2", 0x000000, 0x000000, 0x020000)
	ROM_COPY( "user2", 0x020000, 0x020000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x040000, 0x020000)
	ROM_COPY( "user2", 0x040000, 0x060000, 0x020000)
	ROM_COPY( "user2", 0x000000, 0x080000, 0x020000)
	ROM_COPY( "user2", 0x060000, 0x0a0000, 0x020000)
ROM_END


static UINT8 playmark_asciitohex(UINT8 data)
{
	/* Convert ASCII data to HEX */

	if ((data >= 0x30) && (data < 0x3a)) data -= 0x30;
	data &= 0xdf;			/* remove case sensitivity */
	if ((data >= 0x41) && (data < 0x5b)) data -= 0x37;

	return data;
}


static DRIVER_INIT( bigtwin )
{
	playmark_state *state = machine.driver_data<playmark_state>();
	UINT8 *playmark_PICROM_HEX = machine.region("user1")->base();
	UINT16 *playmark_PICROM = (UINT16 *)machine.region("audiocpu")->base();
	INT32 offs, data;
	UINT16 src_pos = 0;
	UINT16 dst_pos = 0;
	UINT8 data_hi, data_lo;

	state->m_snd_flag = 0;

	/**** Convert the PIC16C57 ASCII HEX dumps to pure HEX ****/
	do
	{
		if ((playmark_PICROM_HEX[src_pos + 0] == ':') &&
			(playmark_PICROM_HEX[src_pos + 1] == '1') &&
			(playmark_PICROM_HEX[src_pos + 2] == '0'))
		{
			src_pos += 9;

			for (offs = 0; offs < 32; offs += 4)
			{
				data_hi = playmark_asciitohex((playmark_PICROM_HEX[src_pos + offs + 0]));
				data_lo = playmark_asciitohex((playmark_PICROM_HEX[src_pos + offs + 1]));
				if ((data_hi <= 0x0f) && (data_lo <= 0x0f))
				{
					data = (data_hi <<  4) | (data_lo << 0);
					data_hi = playmark_asciitohex((playmark_PICROM_HEX[src_pos + offs + 2]));
					data_lo = playmark_asciitohex((playmark_PICROM_HEX[src_pos + offs + 3]));

					if ((data_hi <= 0x0f) && (data_lo <= 0x0f))
					{
						data |= (data_hi << 12) | (data_lo << 8);
						playmark_PICROM[dst_pos] = data;
						dst_pos += 1;
					}
				}
			}
			src_pos += 32;
		}

		/* Get the PIC16C57 Config register data */

		if ((playmark_PICROM_HEX[src_pos + 0] == ':') &&
			(playmark_PICROM_HEX[src_pos + 1] == '0') &&
			(playmark_PICROM_HEX[src_pos + 2] == '2') &&
			(playmark_PICROM_HEX[src_pos + 3] == '1'))
		{
			src_pos += 9;

			data_hi = playmark_asciitohex((playmark_PICROM_HEX[src_pos + 0]));
			data_lo = playmark_asciitohex((playmark_PICROM_HEX[src_pos + 1]));
			data = (data_hi <<  4) | (data_lo << 0);
			data_hi = playmark_asciitohex((playmark_PICROM_HEX[src_pos + 2]));
			data_lo = playmark_asciitohex((playmark_PICROM_HEX[src_pos + 3]));
			data |= (data_hi << 12) | (data_lo << 8);

			pic16c5x_set_config(machine.device("audiocpu"), data);

			src_pos = 0x7fff;		/* Force Exit */
		}
		src_pos += 1;
	} while (src_pos < 0x2d4c);		/* 0x2d4c is the size of the HEX rom loaded */
}

GAME( 1995, bigtwin,   0,        bigtwin,  bigtwin,  bigtwin, ROT0, "Playmark", "Big Twin", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, bigtwinb,  bigtwin,  bigtwinb, bigtwinb, bigtwin, ROT0, "Playmark", "Big Twin (No Girls Conversion)", GAME_NO_COCKTAIL | GAME_SUPPORTS_SAVE )
GAME( 1995, wbeachvl,  0,        wbeachvl, wbeachvl, 0,       ROT0, "Playmark", "World Beach Volley (set 1)", GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1995, wbeachvl2, wbeachvl, wbeachvl, wbeachvl, 0,       ROT0, "Playmark", "World Beach Volley (set 2)",  GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1995, wbeachvl3, wbeachvl, wbeachvl, wbeachvl, 0,       ROT0, "Playmark", "World Beach Volley (set 3)",  GAME_NO_COCKTAIL | GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1996, excelsr,   0,        excelsr,  excelsr,  bigtwin, ROT0, "Playmark", "Excelsior", GAME_SUPPORTS_SAVE )
GAME( 1995, hotmind,   0,        hotmind,  hotmind,  bigtwin, ROT0, "Playmark", "Hot Mind", GAME_SUPPORTS_SAVE )
GAME( 1994, hrdtimes,  0,        hrdtimes, hrdtimes, 0,       ROT0, "Playmark", "Hard Times (set 1)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1994, hrdtimesa, hrdtimes, hrdtimes, hrdtimes, 0,       ROT0, "Playmark", "Hard Times (set 2)", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
