// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Pierpaolo Prazzoli, Quench
/***************************************************************************

Big Twin
World Beach Volley
Excelsior
Hard Times
Hot Mind
Lucky Boom


driver by Nicola Salmoria and Pierpaolo Prazzoli

The games run on different, but similar, hardware. The sprite system is the
same (almost - the tile size is different).

Even if some games are from the same year, World Beach Volley is much more
advanced - more colourful, and stores setting in an EEPROM.

An interesting thing about this hardware is that the same gfx ROMs are used
to generate both 8x8 and 16x16 tiles for different tilemaps.

Hard Times, Hot Mind and Lucky Boom have different tilemaps layout than the other games.

Hard Times was hacked from Blood Bros. program code.

Hot Mind and Lucky Boom are a romswap kit for Hard Times pcb, in fact Hot Mind was
found in a pcb marked as HARD TIMES 28-06-94.

The version of Big Twin without girls seems a conversion of Hard Times pcb.

Lucky Boom has nvram at $ff0000. What kind of chip is it and how large
should it be? Currently 0x201 bytes inclusive are written to.

Original Bugs:
- World Beach Volley histogram functions don't work.


HotMind and Lucky Boom are currently missing the sound MCU internal program dump.
So for now we're using PIC16C57 program code from Excelsior with modifications
to correct the music playback sequencing and also allow sound effect samples to
play on any of the three available sample channels. The fourth channel is reserved
for music playback.

HotMind and presumably Lucky Boom have a Jamma expansion board that plugs onto the
main boards Jamma connector and provides extra I/O using the generic Player-2 pins.
The expansion boards provides I/O for the Token and Ticket dispensers, drive circuit
for Coin-In Counter and Credit-Out Counter. In the case of HotMind, a 93C46 EEPROM
for storing game config/stats. The Expansion board then has a male Jamma connector
that you plug into your Jamma harness.


TODO:
- World Beach Volley and Hard Times sound is controlled by a PIC16C57 whose internal
  ROM is currently missing.
- Lucky Boom has some minor colour issue with the background - see the title screen. The
  game selects the wrong colour for some tiles. The tiles should be colour 0x01 not 0x02.
  Affects the backgrounds in game however it's barely noticeable.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "sound/okim6295.h"
#include "includes/playmark.h"
#include "machine/ticket.h"
#include "machine/nvram.h"


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
		m_eeprom->cs_write((data & 0x20) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->di_write((data & 0x80) >> 7);
		m_eeprom->clk_write((data & 0x40) ? CLEAR_LINE : ASSERT_LINE);
	}
}

WRITE16_MEMBER(playmark_state::hotmind_coin_eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
//      if (data & 0x80) logerror("PC$%06x Writing unknown bits %02x to the Coin/EEPROM port\n", space.device().safe_pcbase(), data);

		if (data) {
			if ((m_dispenser_latch & 0x80) == 0) m_dispenser_latch = 0;
			if (data & 0x10) {
				m_dispenser_latch |= ((data & 0x10) | 0x80);
				machine().device<ticket_dispenser_device>("token")->write(space, 0, 0x80);
			}
		}
		else {
			m_dispenser_latch &= 0x7f;
			machine().device<ticket_dispenser_device>("token")->write(space, 0, (m_dispenser_latch & 0x10) ? 0x80 : 0);
		}
		machine().device<ticket_dispenser_device>("ticket")->write(space, 0, (data & 0x08) ? 0x80 : 0);

		coin_counter_w(machine(), 0, data & 0x20);      /* Coin In counter - transistor driven */
		coin_counter_w(machine(), 1, data & 0x40);      /* Token/Ticket Out counter - transistor driven */

		m_eeprom->cs_write((data & 1) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->di_write((data & 4) >> 2);
		m_eeprom->clk_write((data & 2) ? ASSERT_LINE : CLEAR_LINE );
	}
}

WRITE16_MEMBER(playmark_state::luckboomh_dispenser_w)
{
	if (ACCESSING_BITS_0_7)
	{
//      if (data & 0x87) logerror("PC$%06x Writing unknown bits %02x to the Coin/EEPROM port\n", space.device().safe_pcbase(), data);

		if (data) {
			if ((m_dispenser_latch & 0x80) == 0) m_dispenser_latch = 0;
			if (data & 0x10) {
				m_dispenser_latch |= ((data & 0x10) | 0x80);
				machine().device<ticket_dispenser_device>("token")->write(space, 0, 0x80);
			}
		}
		else {
			m_dispenser_latch &= 0x7f;
			machine().device<ticket_dispenser_device>("token")->write(space, 0, (m_dispenser_latch & 0x10) ? 0x80 : 0);
		}
		machine().device<ticket_dispenser_device>("ticket")->write(space, 0, (data & 0x08) ? 0x80 : 0);

		coin_counter_w(machine(), 0, data & 0x20);      /* Coin In counter - transistor driven */
		coin_counter_w(machine(), 1, data & 0x40);      /* Token/Ticket Out counter - transistor driven */
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
//      logerror("PC$%06x 68K Writing sound command %02x to OKI\n",space.device().safe_pcbase(), data);

		m_snd_command = (data & 0xff);
		m_snd_flag = 1;
		space.device().execute().yield();
	}
}

READ8_MEMBER(playmark_state::playmark_snd_command_r)
{
	int data = 0;

	if ((m_oki_control & 0x38) == 0x30)
	{
		data = m_snd_command;
//      logerror("PC$%03x PortB reading %02x from the 68K\n", space.device().safe_pcbase(), data);
	}
	else if ((m_oki_control & 0x38) == 0x28)
	{
		data = (m_oki->read(space, 0) & 0x0f);
//      logerror("PC$%03x PortB reading %02x from the OKI status port\n", space.device().safe_pcbase(), data);
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


WRITE8_MEMBER(playmark_state::playmark_oki_banking_w)
{
	logerror("PC$%03x Writing %02x to PortA  (OKI bank select) previous bank was %02x\n",space.device().safe_pcbase(),data,m_old_oki_bank);

	if (m_old_oki_bank != (data & 7))
	{
		m_old_oki_bank = data & 7;

		if (((m_old_oki_bank - 1) * 0x40000) < memregion("oki")->bytes())
		{
			m_oki->set_bank_base(0x40000 * (m_old_oki_bank - 1));
		}
	}
}

WRITE8_MEMBER(playmark_state::playmark_oki_w)
{
	m_oki_command = data;
}

WRITE8_MEMBER(playmark_state::playmark_snd_control_w)
{
	/*  This port controls communications to and from the 68K and the OKI device.

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
//      logerror("PC$%03x Writing %02x to OKI1, PortC=%02x, Code=%02x\n",space.device().safe_pcbase(),m_oki_command,m_oki_control,m_snd_command);
		m_oki->write(space, 0, m_oki_command);
	}
}

WRITE8_MEMBER(playmark_state::hrdtimes_snd_control_w)
{
	/*  This port controls communications to and from the 68K and the OKI device. See playmark_snd_control_w above. OKI banking is also handled here. */

	if (m_old_oki_bank != (data & 3))
	{
//      logerror("PC$%03x Writing %02x to PortC (OKI bank select bits). Previous bank was %02x\n",space.device().safe_pcbase(),(data&3),m_old_oki_bank);

		m_old_oki_bank = data & 3;

		if ((m_old_oki_bank * 0x40000) < memregion("oki")->bytes())
		{
			m_oki->set_bank_base(0x40000 * m_old_oki_bank);
		}
	}

	m_oki_control = data;

	if ((data & 0x38) == 0x18)
	{
//      logerror("PC$%03x Writing %02x to OKI1, PortC=%02x, Code=%02x\n",space.device().safe_pcbase(),m_oki_command,m_oki_control,m_snd_command);
		m_oki->write(space, 0, m_oki_command);
	}
}


READ_LINE_MEMBER(playmark_state::PIC16C5X_T0_clk_r)
{
	return 0;
}


/***************************** 68000 Memory Maps ****************************/

static ADDRESS_MAP_START( bigtwin_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x304000, 0x304001) AM_NOP             /* watchdog? irq ack? */
	AM_RANGE(0x440000, 0x4403ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x500000, 0x500fff) AM_WRITE(wbeachvl_fgvideoram_w) AM_SHARE("videoram2")
	AM_RANGE(0x501000, 0x501fff) AM_WRITENOP    /* unused RAM? */
	AM_RANGE(0x502000, 0x503fff) AM_WRITE(wbeachvl_txvideoram_w) AM_SHARE("videoram1")
	AM_RANGE(0x504000, 0x50ffff) AM_WRITENOP    /* unused RAM? */
	AM_RANGE(0x510000, 0x51000b) AM_WRITE(bigtwin_scroll_w)
	AM_RANGE(0x51000c, 0x51000d) AM_WRITENOP    /* always 3? */
	AM_RANGE(0x600000, 0x67ffff) AM_RAM AM_SHARE("bgvideoram")
	AM_RANGE(0x700010, 0x700011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x700012, 0x700013) AM_READ_PORT("P1")
	AM_RANGE(0x700014, 0x700015) AM_READ_PORT("P2")
	AM_RANGE(0x700016, 0x700017) AM_WRITE(coinctrl_w)
	AM_RANGE(0x70001a, 0x70001b) AM_READ_PORT("DSW2")
	AM_RANGE(0x70001c, 0x70001d) AM_READ_PORT("DSW1")
	AM_RANGE(0x70001e, 0x70001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x780000, 0x7807ff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
//  AM_RANGE(0xe00000, 0xe00001) ?? written on startup
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigtwinb_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(hrdtimes_bgvideoram_w) AM_SHARE("videoram3")
	AM_RANGE(0x104000, 0x107fff) AM_RAM_WRITE(hrdtimes_fgvideoram_w) AM_SHARE("videoram2")
	AM_RANGE(0x108000, 0x10ffff) AM_RAM_WRITE(hrdtimes_txvideoram_w) AM_SHARE("videoram1")
	AM_RANGE(0x110000, 0x11000d) AM_WRITE(hrdtimes_scroll_w)
	AM_RANGE(0x201000, 0x2013ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x280000, 0x2807ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("P1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("P2")
	AM_RANGE(0x30001a, 0x30001b) AM_READ_PORT("DSW2")
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("DSW1")
	AM_RANGE(0x30001e, 0x30001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP        /* watchdog? irq ack? */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( wbeachvl_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x500000, 0x501fff) AM_RAM_WRITE(wbeachvl_bgvideoram_w) AM_SHARE("videoram3")
	AM_RANGE(0x504000, 0x505fff) AM_RAM_WRITE(wbeachvl_fgvideoram_w) AM_SHARE("videoram2")
	AM_RANGE(0x508000, 0x509fff) AM_RAM_WRITE(wbeachvl_txvideoram_w) AM_SHARE("videoram1")
	AM_RANGE(0x50f000, 0x50ffff) AM_RAM AM_SHARE("rowscroll")
	AM_RANGE(0x510000, 0x51000b) AM_WRITE(wbeachvl_scroll_w)
	AM_RANGE(0x51000c, 0x51000d) AM_WRITENOP    /* 2 and 3 */
//  AM_RANGE(0x700000, 0x700001) ?? written on startup
	AM_RANGE(0x710010, 0x710011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x710012, 0x710013) AM_READ_PORT("P1")
	AM_RANGE(0x710014, 0x710015) AM_READ_PORT("P2")
	AM_RANGE(0x710016, 0x710017) AM_WRITE(wbeachvl_coin_eeprom_w)
	AM_RANGE(0x710018, 0x710019) AM_READ_PORT("P3")
	AM_RANGE(0x71001a, 0x71001b) AM_READ_PORT("P4")
//  AM_RANGE(0x71001c, 0x71001d) AM_READ(playmark_snd_status???)
//  AM_RANGE(0x71001e, 0x71001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x780000, 0x780fff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( excelsr_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x2fffff) AM_ROM
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP                /* watchdog? irq ack? */
	AM_RANGE(0x440000, 0x440cff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x500000, 0x500fff) AM_RAM_WRITE(wbeachvl_fgvideoram_w) AM_SHARE("videoram2")
	AM_RANGE(0x501000, 0x501fff) AM_RAM_WRITE(wbeachvl_txvideoram_w) AM_SHARE("videoram1")
	AM_RANGE(0x510000, 0x51000b) AM_WRITE(excelsr_scroll_w)
	AM_RANGE(0x51000c, 0x51000d) AM_WRITENOP    /* 2 and 3 */
	AM_RANGE(0x600000, 0x67ffff) AM_RAM AM_SHARE("bgvideoram")
	AM_RANGE(0x700010, 0x700011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x700012, 0x700013) AM_READ_PORT("P1")
	AM_RANGE(0x700014, 0x700015) AM_READ_PORT("P2")
	AM_RANGE(0x700016, 0x700017) AM_WRITE(coinctrl_w)
	AM_RANGE(0x70001a, 0x70001b) AM_READ_PORT("DSW2")
	AM_RANGE(0x70001c, 0x70001d) AM_READ_PORT("DSW1")
	AM_RANGE(0x70001e, 0x70001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x780000, 0x7807ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( hrdtimes_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x0bffff) AM_RAM
	AM_RANGE(0x0c0000, 0x0fffff) AM_ROM AM_REGION("maincpu", 0x0c0000)
	AM_RANGE(0x100000, 0x1007ff) AM_RAM_WRITE(hrdtimes_bgvideoram_w) AM_SHARE("videoram3") // 32*32?
	AM_RANGE(0x100800, 0x103fff) AM_RAM
	AM_RANGE(0x104000, 0x105fff) AM_RAM_WRITE(hrdtimes_fgvideoram_w) AM_SHARE("videoram2") // 128*32?
	AM_RANGE(0x106000, 0x107fff) AM_RAM
	AM_RANGE(0x108000, 0x109fff) AM_RAM_WRITE(hrdtimes_txvideoram_w) AM_SHARE("videoram1") // 64*64?
	AM_RANGE(0x10a000, 0x10bfff) AM_RAM
	AM_RANGE(0x10c000, 0x10ffff) AM_RAM // Unused
	AM_RANGE(0x110000, 0x11000d) AM_WRITE(hrdtimes_scroll_w)
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x280000, 0x2807ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x280800, 0x280fff) AM_RAM // Unused
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("P1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("P2")
	AM_RANGE(0x300016, 0x300017) AM_WRITE(hrdtimes_coin_w)
	AM_RANGE(0x30001a, 0x30001b) AM_READ_PORT("DSW2")
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("DSW1")
//  AM_RANGE(0x30001e, 0x30001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP        /* watchdog? irq ack? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( hotmind_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(hrdtimes_bgvideoram_w) AM_SHARE("videoram3")
	AM_RANGE(0x104000, 0x107fff) AM_RAM_WRITE(hrdtimes_fgvideoram_w) AM_SHARE("videoram2")
	AM_RANGE(0x108000, 0x10ffff) AM_RAM_WRITE(hrdtimes_txvideoram_w) AM_SHARE("videoram1")
	AM_RANGE(0x110000, 0x11000d) AM_WRITE(hrdtimes_scroll_w)
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x280000, 0x2807ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("COINS")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("P1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("DISPENSER") AM_WRITE(hotmind_coin_eeprom_w)
	AM_RANGE(0x30001a, 0x30001b) AM_READ_PORT("DSW2")
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("DSW1")
	AM_RANGE(0x30001e, 0x30001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP        /* watchdog? irq ack? */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( luckboomh_main_map, AS_PROGRAM, 16, playmark_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(hrdtimes_bgvideoram_w) AM_SHARE("videoram3")
	AM_RANGE(0x104000, 0x107fff) AM_RAM_WRITE(hrdtimes_fgvideoram_w) AM_SHARE("videoram2")
	AM_RANGE(0x108000, 0x10ffff) AM_RAM_WRITE(hrdtimes_txvideoram_w) AM_SHARE("videoram1")
	AM_RANGE(0x110000, 0x11000d) AM_WRITE(hrdtimes_scroll_w)
	AM_RANGE(0x200000, 0x200fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x280000, 0x2807ff) AM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x300010, 0x300011) AM_READ_PORT("COINS")
	AM_RANGE(0x300012, 0x300013) AM_READ_PORT("P1")
	AM_RANGE(0x300014, 0x300015) AM_READ_PORT("DISPENSER") AM_WRITE(luckboomh_dispenser_w)
	AM_RANGE(0x30001c, 0x30001d) AM_READ_PORT("SERVICE")
	AM_RANGE(0x30001e, 0x30001f) AM_WRITE(playmark_snd_command_w)
	AM_RANGE(0x304000, 0x304001) AM_WRITENOP        /* watchdog? irq ack? */
	AM_RANGE(0xff0000, 0xff03ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xff8000, 0xffffff) AM_RAM
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
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3,4,5")
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
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Credits to Start" )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Italian ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Censor Pictures" )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Rising Ground Level" )  PORT_DIPLOCATION("SW2:4")  // Starts from 55th ball drop on 2nd level
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:5,6")
//  PORT_DIPSETTING(    0x20, DEF_STR( Easy ) ) /* Seems same as Medium */
	PORT_DIPSETTING(    0x30, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
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
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3,4,5")
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
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Credits to Start" )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Language ) )  PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Italian ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Rising Ground Level" )  PORT_DIPLOCATION("SW2:4")  // Starts from 67th ball drop on 2nd level
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:5,6")
//  PORT_DIPSETTING(    0x20, DEF_STR( Easy ) ) /* Seems same as Medium */
	PORT_DIPSETTING(    0x30, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
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
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL )   /* ?? see code at 746a. sound status? */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   /* EEPROM data */

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
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3,4,5")
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
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Credits to Start" )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "Percentage to Reveal" )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x00, "90%" )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )  PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPNAME( 0x0c, 0x00, "Censor Pictures" )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, "50%" )
	PORT_DIPSETTING(    0x0c, "100%" )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
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
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3,4,5")
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
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "Minimum Credits to Start" )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "1 Life If Continue" )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )  PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )  PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )  PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "Every 300k - 500k" )
	PORT_DIPSETTING(    0x08, "Every 500k - 500k" )
	PORT_DIPSETTING(    0x04, "Only 500k" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hotmind )
	PORT_START("COINS")
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

	PORT_START("DISPENSER")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    // Wired to Token dispenser connector, but doesn't seem to affect anything.
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Token Dispenser Empty") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("token",  ticket_dispenser_device, line_r)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Ticket Dispenser Empty") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   /* EEPROM data */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Coin Mode" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x00, "Mode 2" )
	PORT_DIPNAME( 0x1e, 0x1e, "Coinage Mode 1" ) PORT_CONDITION("DSW1", 0x01, EQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3,4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x12, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x1e, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0x1a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x16, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 6C_1C ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin A Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPNAME( 0x18, 0x18, "Coin B Mode 2" ) PORT_CONDITION("DSW1", 0x01, NOTEQUALS, 0x01)  PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Time Per Line" )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, "10 Seconds" )
	PORT_DIPSETTING(    0x00, "5 Seconds" )
	PORT_DIPNAME( 0x80, 0x80, "Clear All Memory" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )  PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x05, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, "Very Hard 1" )
	PORT_DIPSETTING(    0x03, "Very Hard 2" )
	PORT_DIPSETTING(    0x02, "Very Hard 3" )
	PORT_DIPSETTING(    0x01, "Very Hard 4" )
	PORT_DIPSETTING(    0x00, "Very Hard 5" )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Erogatore Gettoni" )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Erogatore Ticket" )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Clear Partial Memory" )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( luckboomh )
	PORT_START("COINS")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("DISPENSER")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )     // Checked during port read, but no effect noticed
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("token",  ticket_dispenser_device, line_r)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_SERVICE_NO_TOGGLE(0x01, IP_ACTIVE_LOW)
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )
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

static GFXDECODE_START( bigtwin )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0x200, 16 )   /* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,   0x000,  8 )   /* colors 0x000-0x07f */
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0x080,  8 )   /* colors 0x080-0x0ff */
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
	GFXDECODE_ENTRY( "gfx1", 0, wspritelayout, 0x600, 16 )  /* colors 0x600-0x7ff */
	GFXDECODE_ENTRY( "gfx1", 0, wtilelayout,   0x000, 16 )  /* colors 0x000-0x3ff */
	GFXDECODE_ENTRY( "gfx1", 0, wcharlayout,   0x400,  8 )  /* colors 0x400-0x5ff */
GFXDECODE_END

static GFXDECODE_START( excelsr )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0x200, 16 ) /* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0x000,  8 ) /* colors 0x000-0x07f */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0x080,  8 ) /* colors 0x080-0x0ff */
	/* background bitmap uses colors 0x100-0x1ff */
GFXDECODE_END

static const gfx_layout hrdtimes_full_layout =
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

static const gfx_layout hrdtimes_tilelayout =
{
	16,16,
	RGN_FRAC(1,4),
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
	0x400,
	4,
	{ RGN_FRAC(2,4)+8, RGN_FRAC(2,4), RGN_FRAC(0,4)+8, RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout hotmind_charlayout =
{
	8,8,
	0x1000,
	4,
	{ RGN_FRAC(2,4)+8, RGN_FRAC(2,4), RGN_FRAC(0,4)+8, RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};


static GFXDECODE_START( hrdtimes )
	GFXDECODE_ENTRY( "gfx2", 0,       hrdtimes_full_layout, 0x200, 32 )    /* colors 0x200-0x2ff - Sprites */
	GFXDECODE_ENTRY( "gfx1", 0,       hrdtimes_tilelayout,  0x000, 16 )    /* colors 0x000-0x0ff - BG */
	GFXDECODE_ENTRY( "gfx1", 0x80000, hrdtimes_tilelayout,  0x000, 16 )    /* colors 0x000-0x0ff - FG */
	GFXDECODE_ENTRY( "gfx1", 0xfc000, hrdtimes_charlayout,  0x100,  8 )    /* colors 0x100-0x17f - Text */
GFXDECODE_END

static GFXDECODE_START( hotmind )
	GFXDECODE_ENTRY( "gfx2", 0,       hrdtimes_full_layout, 0x200, 32 )    /* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx1", 0,       hrdtimes_tilelayout,  0x000, 16 )    /* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx1", 0x20000, hrdtimes_tilelayout,  0x000, 16 )    /* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx1", 0x30000, hotmind_charlayout,   0x100,  8 )    /* colors 0x100-0x17f */
GFXDECODE_END

static GFXDECODE_START( luckboomh )
	GFXDECODE_ENTRY( "gfx2", 0,       hrdtimes_full_layout, 0x200, 32 )    /* colors 0x200-0x2ff */
	GFXDECODE_ENTRY( "gfx1", 0,       hrdtimes_full_layout, 0x000, 16 )    /* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx1", 0,       hrdtimes_full_layout, 0x000, 16 )    /* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx1", 0x30000, hotmind_charlayout,   0x100,  8 )    /* colors 0x100-0x17f */
GFXDECODE_END

static GFXDECODE_START( bigtwinb )
	GFXDECODE_ENTRY( "gfx2", 0,       spritelayout,        0x300, 16 )    /* colors 0x300-0x3ff */
	GFXDECODE_ENTRY( "gfx1", 0,       hrdtimes_tilelayout, 0x000, 16 )    /* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx1", 0x40000, hrdtimes_tilelayout, 0x000, 16 )    /* colors 0x000-0x0ff */
	GFXDECODE_ENTRY( "gfx1", 0x40000, hotmind_charlayout,  0x200, 16 )    /* colors 0x200-0x2ff */
GFXDECODE_END

MACHINE_START_MEMBER(playmark_state,playmark)
{
	save_item(NAME(m_bgscrollx));
	save_item(NAME(m_bgscrolly));
	save_item(NAME(m_bg_enable));
	save_item(NAME(m_bg_full_size));
	save_item(NAME(m_fgscrollx));
	save_item(NAME(m_fg_rowscroll_enable));
	save_item(NAME(m_scroll));

	save_item(NAME(m_snd_command));
	save_item(NAME(m_snd_flag));
	save_item(NAME(m_oki_control));
	save_item(NAME(m_oki_command));
	save_item(NAME(m_old_oki_bank));
	save_item(NAME(m_dispenser_latch));
}

MACHINE_RESET_MEMBER(playmark_state,playmark)
{
	m_bgscrollx = 0;
	m_bgscrolly = 0;
	m_bg_enable = 0;
	m_bg_full_size = 0;
	m_fgscrollx = 0;
	m_fg_rowscroll_enable = 0;
	memset(m_scroll, 0, sizeof(m_scroll));

	m_snd_command = 0;
	m_snd_flag = 0;
	m_oki_control = 0;
	m_oki_command = 0;
	m_old_oki_bank = 0;
	m_dispenser_latch = 0;
}

static MACHINE_CONFIG_START( bigtwin, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(bigtwin_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", playmark_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, 12000000)
	MCFG_PIC16C5x_WRITE_A_CB(WRITE8(playmark_state, playmark_oki_banking_w))
	MCFG_PIC16C5x_READ_B_CB(READ8(playmark_state, playmark_snd_command_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(playmark_state, playmark_oki_w))
	MCFG_PIC16C5x_READ_C_CB(READ8(playmark_state, playmark_snd_flag_r))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(playmark_state, playmark_snd_control_w))
	MCFG_PIC16C5x_T0_CB(READLINE(playmark_state, PIC16C5X_T0_clk_r))

	MCFG_MACHINE_START_OVERRIDE(playmark_state,playmark)
	MCFG_MACHINE_RESET_OVERRIDE(playmark_state,playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playmark_state, screen_update_bigtwin)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bigtwin)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(playmark_state,bigtwin)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bigtwinb, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(bigtwinb_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", playmark_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_24MHz/2)
	MCFG_PIC16C5x_WRITE_A_CB(WRITE8(playmark_state, playmark_oki_banking_w))
	MCFG_PIC16C5x_READ_B_CB(READ8(playmark_state, playmark_snd_command_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(playmark_state, playmark_oki_w))
	MCFG_PIC16C5x_READ_C_CB(READ8(playmark_state, playmark_snd_flag_r))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(playmark_state, playmark_snd_control_w))
	MCFG_PIC16C5x_T0_CB(READLINE(playmark_state, PIC16C5X_T0_clk_r))

	MCFG_MACHINE_START_OVERRIDE(playmark_state,playmark)
	MCFG_MACHINE_RESET_OVERRIDE(playmark_state,playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playmark_state, screen_update_bigtwinb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bigtwinb)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(playmark_state,bigtwinb)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( wbeachvl, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(wbeachvl_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", playmark_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_24MHz/2)    /* 12MHz with internal 4x divisor */
	MCFG_PIC16C5x_WRITE_A_CB(WRITE8(playmark_state, playmark_oki_banking_w))
	MCFG_PIC16C5x_READ_B_CB(READ8(playmark_state, playmark_snd_command_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(playmark_state, playmark_oki_w))
	MCFG_PIC16C5x_READ_C_CB(READ8(playmark_state, playmark_snd_flag_r))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(playmark_state, playmark_snd_control_w))
	MCFG_PIC16C5x_T0_CB(READLINE(playmark_state, PIC16C5X_T0_clk_r))
	MCFG_DEVICE_DISABLE()       /* Internal code is not dumped yet */

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DEFAULT_VALUE(0)

	MCFG_MACHINE_START_OVERRIDE(playmark_state,playmark)
	MCFG_MACHINE_RESET_OVERRIDE(playmark_state,playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playmark_state, screen_update_wbeachvl)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", wbeachvl)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(RRRRRGGGGGBBBBBx)

	MCFG_VIDEO_START_OVERRIDE(playmark_state,wbeachvl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1000000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( excelsr, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)   /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(excelsr_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", playmark_state,  irq2_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_24MHz/2)    /* 12MHz with internal 4x divisor */
	MCFG_PIC16C5x_WRITE_A_CB(WRITE8(playmark_state, playmark_oki_banking_w))
	MCFG_PIC16C5x_READ_B_CB(READ8(playmark_state, playmark_snd_command_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(playmark_state, playmark_oki_w))
	MCFG_PIC16C5x_READ_C_CB(READ8(playmark_state, playmark_snd_flag_r))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(playmark_state, playmark_snd_control_w))
	MCFG_PIC16C5x_T0_CB(READLINE(playmark_state, PIC16C5X_T0_clk_r))

	MCFG_MACHINE_START_OVERRIDE(playmark_state,playmark)
	MCFG_MACHINE_RESET_OVERRIDE(playmark_state,playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playmark_state, screen_update_excelsr)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", excelsr)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(playmark_state,excelsr)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH) /* 1MHz resonator */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hrdtimes, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(hrdtimes_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", playmark_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_24MHz/2)    /* verified on pcb */
//  MCFG_PIC16C5x_WRITE_A_CB(WRITE8(playmark_state, playmark_oki_banking_w)) // Banking data output but not wired. Port C is wired to the OKI banking instead
	MCFG_PIC16C5x_READ_B_CB(READ8(playmark_state, playmark_snd_command_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(playmark_state, playmark_oki_w))
	MCFG_PIC16C5x_READ_C_CB(READ8(playmark_state, playmark_snd_flag_r))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(playmark_state, hrdtimes_snd_control_w))
	MCFG_PIC16C5x_T0_CB(READLINE(playmark_state, PIC16C5X_T0_clk_r))
	MCFG_DEVICE_DISABLE()       /* Internal code is not dumped yet */

	MCFG_MACHINE_START_OVERRIDE(playmark_state,playmark)
	MCFG_MACHINE_RESET_OVERRIDE(playmark_state,playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playmark_state, screen_update_hrdtimes)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hrdtimes)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(playmark_state,hrdtimes)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hotmind, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(hotmind_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", playmark_state,  irq6_line_hold) // irq 2 and 6 point to the same location on hotmind

	MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_24MHz/2)    /* verified on pcb */
//  MCFG_PIC16C5x_WRITE_A_CB(WRITE8(playmark_state, playmark_oki_banking_w)) // Banking data output but not wired. Port C is wired to the OKI banking instead
	MCFG_PIC16C5x_READ_B_CB(READ8(playmark_state, playmark_snd_command_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(playmark_state, playmark_oki_w))
	MCFG_PIC16C5x_READ_C_CB(READ8(playmark_state, playmark_snd_flag_r))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(playmark_state, hrdtimes_snd_control_w))
	MCFG_PIC16C5x_T0_CB(READLINE(playmark_state, PIC16C5X_T0_clk_r))

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DEFAULT_VALUE(0)

	MCFG_MACHINE_START_OVERRIDE(playmark_state,playmark)
	MCFG_MACHINE_RESET_OVERRIDE(playmark_state,playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playmark_state, screen_update_hrdtimes)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hotmind)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(playmark_state,hotmind)

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(350), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH )
	MCFG_TICKET_DISPENSER_ADD("token",  attotime::from_msec(350), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH)  /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( luckboomh, playmark_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_24MHz/2)   /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(luckboomh_main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", playmark_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", PIC16C57, XTAL_24MHz/2)    /* verified on pcb */
//  MCFG_PIC16C5x_WRITE_A_CB(WRITE8(playmark_state, playmark_oki_banking_w)) // Banking data output but not wired. Port C is wired to the OKI banking instead
	MCFG_PIC16C5x_READ_B_CB(READ8(playmark_state, playmark_snd_command_r))
	MCFG_PIC16C5x_WRITE_B_CB(WRITE8(playmark_state, playmark_oki_w))
	MCFG_PIC16C5x_READ_C_CB(READ8(playmark_state, playmark_snd_flag_r))
	MCFG_PIC16C5x_WRITE_C_CB(WRITE8(playmark_state, hrdtimes_snd_control_w))
	MCFG_PIC16C5x_T0_CB(READLINE(playmark_state, PIC16C5X_T0_clk_r))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MACHINE_START_OVERRIDE(playmark_state,playmark)
	MCFG_MACHINE_RESET_OVERRIDE(playmark_state,playmark)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(playmark_state, screen_update_hrdtimes)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", luckboomh)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(RRRRGGGGBBBBRGBx)

	MCFG_VIDEO_START_OVERRIDE(playmark_state,luckboomh)

	MCFG_TICKET_DISPENSER_ADD("ticket", attotime::from_msec(350), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH )
	MCFG_TICKET_DISPENSER_ADD("token",  attotime::from_msec(350), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_1MHz, OKIM6295_PIN7_HIGH)  /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bigtwin )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "2.302",        0x000000, 0x80000, CRC(e6767f60) SHA1(ec0ba1c786e6fde04601c2f3f619e3c6545f9239) )
	ROM_LOAD16_BYTE( "3.301",        0x000001, 0x80000, CRC(5aba6990) SHA1(4f664a91819fdd27821fa607425701d83fcbd8ce) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
//  ROM_LOAD( "16c57hs.bin",  0x0000, 0x1000, CRC(b4c95cc3) SHA1(7fc9b141e7782aa5c17310ee06db99d884537c30) )
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "pic16c57-hs_bigtwin_015.hex",  0x0000, 0x2d4c, CRC(c07e9375) SHA1(7a6714ab888ea6e37bc037bc7419f0998868cfce) )

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

	ROM_REGION( 0x40000, "oki", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "1.013",        0x00000, 0x40000, CRC(ff6671dc) SHA1(517941946a3edfc2da0b7aa8a106ebb4ae849beb) )
ROM_END

ROM_START( bigtwinb )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.u67", 0x00000, 0x20000, CRC(f5cdf1a9) SHA1(974328cf2b4ec5834a519e3300ee1ad8bc4d5c04) )
	ROM_LOAD16_BYTE( "3.u66", 0x00001, 0x20000, CRC(084e990f) SHA1(d7c2e08c7f7c7b453dd19dcf1f30bad46d943c8a) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
//  ROM_LOAD( "16c57hs.bin",  0x0000, 0x1000, CRC(b4c95cc3) SHA1(7fc9b141e7782aa5c17310ee06db99d884537c30) )
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "pic16c57-hs_bigtwin_015.hex",  0x0000, 0x2d4c, CRC(c07e9375) SHA1(7a6714ab888ea6e37bc037bc7419f0998868cfce) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "4.u36", 0x000000, 0x20000, CRC(99aaeacc) SHA1(0281237722d5a94fb9831616ae2ffc8288e78e2c) )
	ROM_CONTINUE(             0x040000, 0x20000 )
	ROM_LOAD16_BYTE( "5.u42", 0x000001, 0x20000, CRC(5c1dfd72) SHA1(31fab4d3bd4e8ff5a16daeaff0ccaa4fc8f60c92) )
	ROM_CONTINUE(             0x040001, 0x20000 )
	ROM_LOAD16_BYTE( "6.u39", 0x080000, 0x20000, CRC(788f2df6) SHA1(186f4f9f79c80dc5c6faa9eddc4b3c98b52b374d) )
	ROM_CONTINUE(             0x0c0000, 0x20000 )
	ROM_LOAD16_BYTE( "7.u45", 0x080001, 0x20000, CRC(aedb2e6d) SHA1(775e13d328c8ee3c36b9d77ad49fa5a092b85a95) )
	ROM_CONTINUE(             0x0c0001, 0x20000 )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "11.u86",       0x00000, 0x20000, CRC(2749644d) SHA1(f506ed1a14ee411eda8a7e639f5572e35b89b13f) )
	ROM_LOAD( "10.u85",       0x20000, 0x20000, CRC(1d1897af) SHA1(0ad00906b94443d7ceef383717b39c6aa8cca241) )
	ROM_LOAD( "9.u84",        0x40000, 0x20000, CRC(2a03432e) SHA1(44722b83093211d88460cbcd9e9c0b638d24ad3e) )
	ROM_LOAD( "8.u83",        0x60000, 0x20000, CRC(2c980c4c) SHA1(77af29a1f5d4302650915f4a7daf2918a2519a6e) )

	ROM_REGION( 0x40000, "oki", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "io13.bin",     0x00000, 0x40000, CRC(ff6671dc) SHA1(517941946a3edfc2da0b7aa8a106ebb4ae849beb) )
ROM_END

ROM_START( wbeachvl )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "wbv_02.bin",   0x000000, 0x40000, CRC(c7cca29e) SHA1(03af361081d688c4204a95f7f5babcc598b72c23) )
	ROM_LOAD16_BYTE( "wbv_03.bin",   0x000001, 0x40000, CRC(db4e69d5) SHA1(119bf35a463d279ddde67ab08f6f1bab9f05cf0c) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
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

	ROM_REGION( 0x100000, "user2", 0 )  /* OKIM6295 samples */
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
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.bin",   0x000000, 0x40000, CRC(8993487e) SHA1(c927ae655807f9046f66ff96a30bd2c6fa671566) )
	ROM_LOAD16_BYTE( "3.bin",   0x000001, 0x40000, CRC(15904789) SHA1(640c80bbf7302529e1a39c2ae60e018ecb176478) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
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

	ROM_REGION( 0x100000, "user2", 0 )  /* OKIM6295 samples */
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
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "2.u16",   0x000000, 0x40000, CRC(f0f4c282) SHA1(94850b45368c3d09629852adc8ca08164b7a7a94) )
	ROM_LOAD16_BYTE( "3.u15",   0x000001, 0x40000, CRC(99775c21) SHA1(fa80a81c59142abcf751352d7a7f9e0d3b5172c9) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
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

	ROM_REGION( 0x100000, "user2", 0 )  /* OKIM6295 samples */
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
	ROM_REGION( 0x300000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "22.u301", 0x000001, 0x80000, CRC(f0aa1c1b) SHA1(5ed68181defe6cde6f4979508f0cfce9e9743912) )
	ROM_LOAD16_BYTE( "19.u302", 0x000000, 0x80000, CRC(9a8acddc) SHA1(c7868317998bb98c630685a0b242ffd1fbdc54ed) )
	ROM_LOAD16_BYTE( "21.u303", 0x100001, 0x80000, CRC(fdf9bd64) SHA1(783e3b8b70f8751915715e2455990c1c8eec6a71) )
	ROM_LOAD16_BYTE( "18.u304", 0x100000, 0x80000, CRC(fe517e0e) SHA1(fa074c3848046b59f1026f9ce1f264b49560668d) )
	ROM_LOAD16_BYTE( "20.u305", 0x200001, 0x80000, CRC(8692afe9) SHA1(b4411bad64a9a6efd8eb13dcf7c5eebfb5681f3d) )
	ROM_LOAD16_BYTE( "17.u306", 0x200000, 0x80000, CRC(978f9a6b) SHA1(9514b97f071fd20740218a58af877765beffedad) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "pic16c57-hs_excelsior_i015.hex", 0x0000, 0x2d4c, CRC(022c6941) SHA1(8ead40bfa7aa783b1ce62bd6cfa673cb876e29e7) )

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

	ROM_REGION( 0x80000, "user2", 0 )   /* OKIM6295 samples */
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

ROM_START( excelsra )
	ROM_REGION( 0x300000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "22.u301", 0x000001, 0x80000, CRC(55dca2da) SHA1(b16ce3c12f635e165740b0a72a6cfd838e4ce701) ) // sldh
	ROM_LOAD16_BYTE( "19.u302", 0x000000, 0x80000, CRC(d13990a8) SHA1(4f002c4a9003af9963a601c78be446815e9bae92) ) // sldh
	ROM_LOAD16_BYTE( "21.u303", 0x100001, 0x80000, CRC(fdf9bd64) SHA1(783e3b8b70f8751915715e2455990c1c8eec6a71) )
	ROM_LOAD16_BYTE( "18.u304", 0x100000, 0x80000, CRC(fe517e0e) SHA1(fa074c3848046b59f1026f9ce1f264b49560668d) )
	ROM_LOAD16_BYTE( "20.u305", 0x200001, 0x80000, CRC(8692afe9) SHA1(b4411bad64a9a6efd8eb13dcf7c5eebfb5681f3d) )
	ROM_LOAD16_BYTE( "17.u306", 0x200000, 0x80000, CRC(978f9a6b) SHA1(9514b97f071fd20740218a58af877765beffedad) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "pic16c57-hs_excelsior_i015.hex", 0x0000, 0x2d4c, CRC(022c6941) SHA1(8ead40bfa7aa783b1ce62bd6cfa673cb876e29e7) )

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

	ROM_REGION( 0x80000, "user2", 0 )   /* OKIM6295 samples */
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

ROM_START( hrdtimes )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "31.u67",       0x00000, 0x80000, CRC(53eb041b) SHA1(7437da1ceb26e9518a3085560b8a42f37e77ace9) )
	ROM_LOAD16_BYTE( "32.u66",       0x00001, 0x80000, CRC(f2c6b382) SHA1(d73affed091a261c4bfe17f409657e0a46b6c163) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "pic16c57.hex",     0x0000, 0x1000, NO_DUMP )

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

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_LOAD( "30.io13",      0x00000, 0x20000, CRC(fa5e50ae) SHA1(f3bd87c83fca9269cc2f19db1fbf55540c96f931) )
	ROM_CONTINUE(             0x60000, 0x20000 )
	ROM_CONTINUE(             0xa0000, 0x20000 )
	ROM_CONTINUE(             0xe0000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x20000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x40000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x80000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0xc0000, 0x20000 )
ROM_END

/* Different revision of the PCB, uses larger gfx ROMs, however the content is the same */

ROM_START( hrdtimesa )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "u67.bin",       0x00000, 0x80000, CRC(3e1334cb) SHA1(9523c04f92371a35c297280b42b1604e23790a1e) )
	ROM_LOAD16_BYTE( "u66.bin",       0x00001, 0x80000, CRC(041ec30a) SHA1(00476ebd0a64cbd027be159cae7666d2df6d11ba) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "pic16c57.hex",     0x0000, 0x1000, NO_DUMP )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "fh1_playmark_ht", 0x000000, 0x100000, CRC(3cca02b0) SHA1(22c57f4192bf81dd26caa6adfb1c80665bdc305c) )
	ROM_LOAD( "fh2_playmark_ht", 0x100000, 0x100000, CRC(ed699acd) SHA1(23cf1da4e7462f7434e946a80bdd6df0395b3059) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "mh1_playmark_ht", 0x000000, 0x100000, CRC(927e5989) SHA1(b01444a3ff57cc2e10594e23c0343c956ed3ee32) )
	ROM_LOAD( "mh2_playmark_ht", 0x100000, 0x100000, CRC(e76f001b) SHA1(217c06ca3618275c22e33cfe318ec6c970d4862c) )

	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	ROM_REGION( 0x100000, "oki", 0 ) /* Samples */
	ROM_LOAD( "io13.bin",     0x00000, 0x20000, CRC(fa5e50ae) SHA1(f3bd87c83fca9269cc2f19db1fbf55540c96f931) )
	ROM_CONTINUE(             0x60000, 0x20000 )
	ROM_CONTINUE(             0xa0000, 0x20000 )
	ROM_CONTINUE(             0xe0000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x20000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x40000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x80000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0xc0000, 0x20000 )
ROM_END

/*

Hot Mind
Playmark, 1995

PCB Layout
----------
HARD TIMES 28-06-94
       |---------------------------------------------------------|
       |        ROM20                                            |
       |                                  22V10A                 |
       |        M6295           22V10A    16V8H     ROM23  ROM27 |
       |        1MHz   PIC16C57                     ROM24  ROM28 |
|------|                                   62256                 |
|Ticket|                                   62256                 |
|93C46 |J       6116                                ROM25  ROM29 |
|      |A       6116                     |--------| ROM26  ROM30 |
|      |M                                |TPC1020 |              |
|J     |M                                |AFN-084C|              |
|A 1   |A                                |        |              |
|M 6   |                       26MHz     |--------|              |
|M V   |                                                         |
|A 8   |  DSW1                                                   |
|  H   |                                                         |
|      |  DSW2       16V8H                                       |
|Token |                                                         |
|------|                                                   6116  |
       |                  68000            6116            6116  |
       |  62256    62256                   6116     22V10A       |
       |                  24MHz            6116                  |
       |  ROM21    ROM22                   6116                  |
       |---------------------------------------------------------|
Notes:
      68000 CPU clock - 12.000MHz [24/2]
      M6295 clock     - 1.000MHz. Sample rate = 1000000/132
      PIC16C57 clock  - OCS1/CLKIN 12.000MHz (on pin 27, but internally divided by 4 at 3.000MHz)
                        Note PIC is secured, contents can not be read out.
      PALCE16V8H-25   - x3
      PAL22V10ACNT    - x3
      VSync           - 58Hz
      HSync           - 14.25kHz
*/

ROM_START( hotmind )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "21.u67",       0x00000, 0x20000, CRC(e9000f7f) SHA1(c19fee7b774d3f30f4d4025a63ec396ec119c855) )
	ROM_LOAD16_BYTE( "22.u66",       0x00001, 0x20000, CRC(2c518ec5) SHA1(6d9e81ddb5793d64e22dc0254519b947f6ec6954) )

	ROM_REGION( 0x1000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
	/* ROM will be copied here by the init code from "user1" */

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "hotmind_pic16c57-hs_io15.hex", 0x0000, 0x2d4c, BAD_DUMP CRC(f3300d13) SHA1(78892453c7374ea3d1606cdb81197cc466e2a8c5) )  // protected, contains upper nibble?
	ROM_LOAD( "hotmind_pic16c57.hex",         0x0000, 0x2d4c, BAD_DUMP CRC(11957803) SHA1(c2f87659819bfcf3a5b43fbccf81988c43b9c9c8) )  // Using modified Excelsior PIC code to make it suite this game

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "23.u36",       0x000000, 0x10000, CRC(ddcf60b9) SHA1(0c0fbc44131cb7d36c21bf5aead87b498c5684f5) )
	ROM_CONTINUE(                    0x020000, 0x10000 )
	ROM_LOAD16_BYTE( "27.u42",       0x000001, 0x10000, CRC(413bbcf4) SHA1(d82ae9d26df1a69b760b3025048e47ab757d9175) )
	ROM_CONTINUE(                    0x020001, 0x10000 )
	ROM_LOAD16_BYTE( "24.u39",       0x040000, 0x10000, CRC(4baa5b4c) SHA1(ee953ed9a4a45715d1ae39b5bb8b9b6505a4e95d) )
	ROM_CONTINUE(                    0x060000, 0x10000 )
	ROM_LOAD16_BYTE( "28.u45",       0x040001, 0x10000, CRC(8df34d6a) SHA1(ca0d2ca7e0f2a302bc8b1a03c0c18ac72fe105ac) )
	ROM_CONTINUE(                    0x060001, 0x10000 )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "26.u86",       0x00000, 0x20000, CRC(ff8d3b75) SHA1(5427b70a61dee4c125877e040be21cb1cadb1af5) )
	ROM_LOAD16_BYTE( "30.u85",       0x00001, 0x20000, CRC(87a640c7) SHA1(818ff3243cb3ed0189988348e6c2e954f0d3dd4f) )
	ROM_LOAD16_BYTE( "25.u84",       0x40000, 0x20000, CRC(c4fd4445) SHA1(ab0c5a328a312740595b5c92a1050527140518f3) )
	ROM_LOAD16_BYTE( "29.u83",       0x40001, 0x20000, CRC(0bebfb53) SHA1(d4342f808141b70af98c370004153a31d120e2a4) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "20.io13",      0x00000, 0x20000, CRC(0bf3a3e5) SHA1(2ae06f37a6bcd20bc5fbaa90d970aba2ebf3cf5a) )
	ROM_CONTINUE(             0x60000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x20000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x40000, 0x20000 )

	ROM_REGION( 0x8000, "plds", 0 )     /* These were read protected */
	ROM_LOAD( "palce16v8h-25-pc4_u58.jed",   0x0000, 0xb89,  BAD_DUMP CRC(ba88c1da) SHA1(9b55e96eee44a467bdfbf760137ccb2fb3afedf0) )
	ROM_LOAD( "palce16v8h-25-pc4_u182.jed",  0x0000, 0xb89,  BAD_DUMP CRC(ba88c1da) SHA1(9b55e96eee44a467bdfbf760137ccb2fb3afedf0) )
	ROM_LOAD( "palce16v8h-25-pc4_jamma.jed", 0x0000, 0xb89,  BAD_DUMP CRC(ba88c1da) SHA1(9b55e96eee44a467bdfbf760137ccb2fb3afedf0) )  // On the Jamma Expansion board
	ROM_LOAD( "tibpal22v10acnt_u113.jed",    0x0000, 0x1e84, BAD_DUMP CRC(94106c63) SHA1(b4b153756398cc1378411a21d503f3ab325c9cf7) )
	ROM_LOAD( "tibpal22v10acnt_u183.jed",    0x0000, 0x1e84, BAD_DUMP CRC(95a446b6) SHA1(e47e39bc51ff16b75acb37983cc307ad421bfcc7) )
	ROM_LOAD( "tibpal22v10acnt_u211.jed",    0x0000, 0x1e84, BAD_DUMP CRC(94106c63) SHA1(b4b153756398cc1378411a21d503f3ab325c9cf7) )
ROM_END

ROM_START( luckboomh )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "21.u67",       0x00000, 0x20000, CRC(5578dd75) SHA1(ed3c2ea302f8bfe49ab5d8c33e572492daa651ae) )
	ROM_LOAD16_BYTE( "22.u66",       0x00001, 0x20000, CRC(1eb72a39) SHA1(d7ea9985013fd8cb89389829dbff2f2710a2297d) )

	ROM_REGION( 0x2000, "audiocpu", ROMREGION_ERASE00 ) /* sound (PIC16C57) */
	/* ROM will be copied here by the init code from "user1" */
	ROM_LOAD( "luckyboom_pic16c57-hs_io15.bin",  0x00000, 0x2000, BAD_DUMP CRC(c4b9c78e) SHA1(e85766383b22a62f19bf272d86d53c7fb1eb5ac4) ) // protected, contains upper nibble?

	ROM_REGION( 0x3000, "user1", 0 )
	ROM_LOAD( "luckyboom_pic16c57.hex", 0x0000, 0x2d4c, BAD_DUMP CRC(5c4b5c39) SHA1(d24a097bb4a134406dd95d3ad5ed912f81a6a849) )  // Using modified Excelsior PIC code to make it suite this game

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "23.u36",       0x000000, 0x10000, CRC(71840dd9) SHA1(9d0a75555dedb6fd28bb7c04b863f3ef5a1f8aac) )
	ROM_CONTINUE(                    0x020000, 0x10000 )
	ROM_LOAD16_BYTE( "27.u42",       0x000001, 0x10000, CRC(2f86b37f) SHA1(99b1ffc2006f7eb1517f2fcee955391af98ba061) )
	ROM_CONTINUE(                    0x020001, 0x10000 )
	ROM_LOAD16_BYTE( "24.u39",       0x040000, 0x10000, CRC(c6725797) SHA1(b6233dbba956e044aa76104bfffdd7fd6799628c) )
	ROM_CONTINUE(                    0x060000, 0x10000 )
	ROM_LOAD16_BYTE( "28.u40",       0x040001, 0x10000, CRC(40e65ed1) SHA1(bc75eb816c58eb0f983bb0eaee854c54e306e1da) )
	ROM_CONTINUE(                    0x060001, 0x10000 )

	ROM_REGION( 0x80000, "gfx2", 0 )    /* Sprites */
	ROM_LOAD16_BYTE( "26.u86",       0x00000, 0x20000, CRC(d3ee7d82) SHA1(b0b3df19d60430e7a9fa29fdfff2183a32986d2d) )
	ROM_LOAD16_BYTE( "30.u85",       0x00001, 0x20000, CRC(4b8a9558) SHA1(9f0f2d8f50f21cf188ad778c3a0a68ec23380b23) )
	ROM_LOAD16_BYTE( "25.u84",       0x40000, 0x20000, CRC(e1ab5cf5) SHA1(f76d00537cfd6f09439e44071875bf021622fd07) )
	ROM_LOAD16_BYTE( "29.u83",       0x40001, 0x20000, CRC(9572d2d4) SHA1(90d55b1f13dc93041160530e8c1ce8def6e02bcf) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Samples */
	ROM_LOAD( "20.io13",      0x00000, 0x20000, CRC(0d42c0a3) SHA1(1b1d4c7dcbb063e8bf133063770b753947d1a017) )
	ROM_CONTINUE(             0x60000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x20000, 0x20000 )
	ROM_COPY( "oki", 0x000000, 0x40000, 0x20000 )
ROM_END


UINT8 playmark_state::playmark_asciitohex(UINT8 data)
{
	/* Convert ASCII data to HEX */

	if ((data >= 0x30) && (data < 0x3a)) data -= 0x30;
	data &= 0xdf;           /* remove case sensitivity */
	if ((data >= 0x41) && (data < 0x5b)) data -= 0x37;

	return data;
}

void playmark_state::playmark_decode_pic_hex_dump(void)
{
	UINT8 *playmark_PICROM_HEX = memregion("user1")->base();
	UINT16 *playmark_PICROM = (UINT16 *)memregion("audiocpu")->base();
	INT32 offs, data;
	UINT16 src_pos = 0;
	UINT16 dst_pos = 0;
	UINT8 data_hi, data_lo;

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

			m_audiocpu->pic16c5x_set_config(data);

			src_pos = 0x7fff;       /* Force Exit */
		}
		src_pos += 1;
	} while (src_pos < 0x2d4c);     /* 0x2d4c is the size of the HEX rom loaded */
}


DRIVER_INIT_MEMBER(playmark_state,pic_decode)
{
	playmark_decode_pic_hex_dump();
}



GAME( 1995, bigtwin,   0,        bigtwin,   bigtwin,   playmark_state, pic_decode, ROT0, "Playmark", "Big Twin", MACHINE_SUPPORTS_SAVE )
GAME( 1995, bigtwinb,  bigtwin,  bigtwinb,  bigtwinb,  playmark_state, pic_decode, ROT0, "Playmark", "Big Twin (No Girls Conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, wbeachvl,  0,        wbeachvl,  wbeachvl,  driver_device,  0,          ROT0, "Playmark", "World Beach Volley (set 1)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1995, wbeachvl2, wbeachvl, wbeachvl,  wbeachvl,  driver_device,  0,          ROT0, "Playmark", "World Beach Volley (set 2)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1995, wbeachvl3, wbeachvl, wbeachvl,  wbeachvl,  driver_device,  0,          ROT0, "Playmark", "World Beach Volley (set 3)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1996, excelsr,   0,        excelsr,   excelsr,   playmark_state, pic_decode, ROT0, "Playmark", "Excelsior (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, excelsra,  excelsr,  excelsr,   excelsr,   playmark_state, pic_decode, ROT0, "Playmark", "Excelsior (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, hrdtimes,  0,        hrdtimes,  hrdtimes,  driver_device,  0,          ROT0, "Playmark", "Hard Times (set 1)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1994, hrdtimesa, hrdtimes, hrdtimes,  hrdtimes,  driver_device,  0,          ROT0, "Playmark", "Hard Times (set 2)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1995, hotmind,   0,        hotmind,   hotmind,   playmark_state, pic_decode, ROT0, "Playmark", "Hot Mind (Hard Times hardware)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, luckboomh, luckboom, luckboomh, luckboomh, playmark_state, pic_decode, ROT0, "Playmark", "Lucky Boom (Hard Times hardware)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
