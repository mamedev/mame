// license:BSD-3-Clause
// copyright-holders:Luca Elia
	/***************************************************************************

	                        -= Gals Panic II =-

	                driver by   Luca Elia (l.elia@tin.it)

CPU     :   2 x 68000  +  MCU
SOUND   :   2 x OKIM6295
OTHER   :   EEPROM
CUSTOM  :   ?

To Do:

- Simulation of the MCU: it sits between the 2 68000's and passes
	messages along. It is currently incomplete, thus no backgrounds
	and the game is unplayable

- The layers are offset


Gals Panic 2 (Korea)
Kaneko 1993

PCB Layout (for single large PCB version)
----------

Z04G2-004
|--------------------------------------------------------------------|
|LPF6K  GP2-104.U36* GP2-103.U62*  GP2-203.U102   GP2-201.U171       |
|PX4460  M6295         424260  34MHz      GP2-202.U121  GP2-200.U189 |
|   VOL  M6295                 20MHz      G204BK5.U170               |
| LA4460   GP2-100-K.U61      |------|    G204AK5.U169 GP2-204A.U188*|
|   DSW2   GP2-101.U60 424260 |KANEKO| 6264                          |
|   DSW1   GP2-102.U59        |KC-002| 6264            GP2-204B.U187*|
|                      D42101 |L0002 |                               |
|                             |------| TMP68HC000N-16    GROM26.U186*|
|J                                         G000K5.U165               |
|A          D431000                                      GROM20.U185*|
|M  MC-1091            62256               G001K5.U164               |
|M        |------|                                       GROM25.U184*|
|A        |KANEKO|     62256                                         |
|   6116  |KC-SHU|                                       GROM24.U183*|
|   6116  |L0003 |                                                   |
|         |------|                                      GP2-303B.U182|
| V-080D                                  62256                      |
| V-080D                       93C46      62256         GP2-303A.U181|
| V-080D   |------|            |------| |------| GROM22U.U161        |
|          |KANEKO|            |KANEKO| |KANEKO|        GP2-302B.U180|
| |------| |KC-TAS|  27MHz     |KC-SHU| |KC-001| GROM22L.U160        |
| |KANEKO| |L0005 |      16MHz |L0003 | |L0001 |        GP2-302A.U179|
| |KC-BYO| |------| |------|   |------| |------| GROM21U.U159        |
| |L0004 |          |KANEKO|  D431000 |------|        GP2-301B-K.U178|
| |------|          |KC-YUU|          |KANEKO|   GROM21L.U158        |
|          |------| |L0006 |          |KC-001|        GP2-301A-K.U177|
| D431000  |KANEKO| |------|          |L0001 |                       |
| D431000  |KC-BYO|                   |------|        GP2-300B-K.U176|
| 6116     |L0004 |                        |------|                  |
| 6116     |------|                D431000 |KANEKO|   GP2-300A-K.U175|
|          D431000  TMP68HC000N-16 D431000 |PISCES|                  |
|          D431000      G002K5.U64         |451   |     GROM10.U174* |
|          6116 6116    G003K5.U63         |------|                  |
|--------------------------------------------------------------------|
Notes:
	       * - These ROMs not populated. Korean-specific ROMs have a K as part of the label text
	   68000 - Clock 13.500MHz [27/2]
	   M6295 - Clock 2.000MHz [16/8]. Pin 7 HIGH
	  V-080D - Custom Kaneko RGB DAC
	 MC-1091 - Custom Kaneko I/O module
	  LFP-6K - Custom Kaneko sound filter/DAC
	  PX4460 - Custom Kaneko sound filter/DAC
	  PISCES - NEC uPD78324 series MCU with 32k internal rom. Clock 13.500MHz [27/2] on pins 51 & 52
	   VSync - 59.1856Hz
	   HSync - 15.625kHz

	   (TODO: VTOTAL = 264, HTOTAL = 432, pixel clock 27 MHz / 4)

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/galpani2.h"

/***************************************************************************


                                    EEPROM


***************************************************************************/

READ16_MEMBER(galpani2_state::galpani2_eeprom_r)
{
	return (m_eeprom_word & ~1) | (m_eeprom->do_read() & 1);
}

WRITE16_MEMBER(galpani2_state::galpani2_eeprom_w)
{
	COMBINE_DATA( &m_eeprom_word );
	if ( ACCESSING_BITS_0_7 )
	{
		// latch the bit
		m_eeprom->di_write((data & 0x02) >> 1);

		// reset line asserted: reset.
		m_eeprom->cs_write((data & 0x08) ? ASSERT_LINE : CLEAR_LINE );

		// clock line asserted: write latch or select next bit to read
		m_eeprom->clk_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
	}
}


/***************************************************************************


                                MCU Simulation

100010.w    software watchdog?
100020.b    number of tasks for the mcu

***************************************************************************/

void galpani2_state::machine_start()
{
	UINT8 *ROM = memregion("subdata")->base();
	membank("subdatabank")->configure_entries(0, 0x2000000/0x0800000, ROM, 0x0800000);
	membank("subdatabank")->set_entry(0);

	save_item(NAME(m_eeprom_word));
	save_item(NAME(m_old_mcu_nmi1));
	save_item(NAME(m_old_mcu_nmi2));

}

void galpani2_state::machine_reset()
{
	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(50)); //initial mcu xchk
}

static void galpani2_write_kaneko(device_t *device)
{
	address_space &dstspace = device->memory().space(AS_PROGRAM);
	int i,x,tpattidx;
	unsigned char testpattern[] = {0xFF,0x55,0xAA,0xDD,0xBB,0x99};

	/* Write "KANEKO" to 100000-100005, but do not clash with ram test */

	x  = 0;

	for (i = 0x100000; i < 0x100007; i++)
	{
		for (tpattidx = 0; tpattidx < 6; tpattidx++)
		{
			if (dstspace.read_byte(i) == testpattern[tpattidx]) x = 1; //ram test fragment present
		}
	}

	if  ( x == 0 )
	{
		dstspace.write_byte(0x100000,0x4b); //K
		dstspace.write_byte(0x100001,0x41); //A
		dstspace.write_byte(0x100002,0x4e); //N
		dstspace.write_byte(0x100003,0x45); //E
		dstspace.write_byte(0x100004,0x4b); //K
		dstspace.write_byte(0x100005,0x4f); //O
	}
}

WRITE8_MEMBER(galpani2_state::galpani2_mcu_init_w)
{
	address_space &srcspace = m_maincpu->space(AS_PROGRAM);
	address_space &dstspace = m_subcpu->space(AS_PROGRAM);
	UINT32 mcu_address, mcu_data;

	for ( mcu_address = 0x100010; mcu_address < (0x100010 + 6); mcu_address += 1 )
	{
		mcu_data    =   srcspace.read_byte(mcu_address );
		dstspace.write_byte(mcu_address-0x10, mcu_data);
	}
	m_subcpu->set_input_line(INPUT_LINE_IRQ7, HOLD_LINE); //MCU Initialised
}

void galpani2_state::galpani2_mcu_nmi1()
{
	address_space &srcspace = m_maincpu->space(AS_PROGRAM);
	address_space &dstspace = m_subcpu->space(AS_PROGRAM);
	UINT32 mcu_list, mcu_command, mcu_address, mcu_extra, mcu_src, mcu_dst, mcu_size;

	for ( mcu_list = 0x100021; mcu_list < (0x100021 + 0x40); mcu_list += 4 )
	{
		mcu_command     =   srcspace.read_byte(mcu_list);

		mcu_address     =   0x100000 +
							(srcspace.read_byte(mcu_list + 1)<<8) +
							(srcspace.read_byte(mcu_list + 2)<<0) ;

		mcu_extra       =   srcspace.read_byte(mcu_list + 3); //0xff for command $A and $2, 0x02 for others

		if (mcu_command != 0)
		{
			logerror("%s : MCU [$%06X] endidx = $%02X / command = $%02X addr = $%04X ? = $%02X.\n",
			machine().describe_context(),
			mcu_list,
			srcspace.read_byte(0x100020),
			mcu_command,
			mcu_address,
			mcu_extra
			);
		}

		switch (mcu_command)
		{
		case 0x00:
			break;

		case 0x02: //Copy N bytes from RAM2 to RAM1?, gp2se is the only one to use it, often!
			mcu_src     =   (srcspace.read_byte(mcu_address + 2)<<8) +
							(srcspace.read_byte(mcu_address + 3)<<0) ;

			mcu_dst     =   (srcspace.read_byte(mcu_address + 6)<<8) +
							(srcspace.read_byte(mcu_address + 7)<<0) ;

			mcu_size    =   (srcspace.read_byte(mcu_address + 8)<<8) +
							(srcspace.read_byte(mcu_address + 9)<<0) ;
			logerror("%s : MCU executes command $%02X, %04X %02X-> %04x\n",machine().describe_context(),mcu_command,mcu_src,mcu_size,mcu_dst);

			for( ; mcu_size > 0 ; mcu_size-- )
			{
				mcu_src &= 0xffff;  mcu_dst &= 0xffff;
				srcspace.write_byte(0x100000 + mcu_dst,dstspace.read_byte(0x100000 + mcu_src));
				mcu_src ++;         mcu_dst ++;
			}

			/* Raise a "job done" flag */
			srcspace.write_byte(mcu_address+0,0xff);
			srcspace.write_byte(mcu_address+1,0xff);

			break;

		case 0x0a:  // Copy N bytes from RAM1 to RAM2
			mcu_src     =   (srcspace.read_byte(mcu_address + 2)<<8) +
							(srcspace.read_byte(mcu_address + 3)<<0) ;

			mcu_dst     =   (srcspace.read_byte(mcu_address + 6)<<8) +
							(srcspace.read_byte(mcu_address + 7)<<0) ;

			mcu_size    =   (srcspace.read_byte(mcu_address + 8)<<8) +
							(srcspace.read_byte(mcu_address + 9)<<0) ;

			logerror("%s : MCU executes command $%02X, %04X %02X-> %04x\n",machine().describe_context(),mcu_command,mcu_src,mcu_size,mcu_dst);

			for( ; mcu_size > 0 ; mcu_size-- )
			{
				mcu_src &= 0xffff;  mcu_dst &= 0xffff;
				dstspace.write_byte(0x100000 + mcu_dst,srcspace.read_byte(0x100000 + mcu_src));
				mcu_src ++;         mcu_dst ++;
			}

			/* Raise a "job done" flag */
			srcspace.write_byte(mcu_address+0,0xff);
			srcspace.write_byte(mcu_address+1,0xff);

			break;

		//case 0x10: //? Clear gal?
		//case 0x14: //? Display gal?
		//until
		//case 0x50: //? Display gal?
		//case 0x68: //? Display "Changed" monster?
		//until
		//case 0x6E: //? Display "Changed" monster?
		//case 0x85: //? Do what?
		default:
			/* Raise a "job done" flag */
			srcspace.write_byte(mcu_address+0,0xff);
			srcspace.write_byte(mcu_address+1,0xff);

			logerror("%s : MCU ERROR, unknown command $%02X\n",machine().describe_context(),mcu_command);
		}

		/* Erase command (so that it won't be processed again)? */
		srcspace.write_byte(mcu_list,0x00);
	}
}

void galpani2_state::galpani2_mcu_nmi2()
{
		galpani2_write_kaneko(m_maincpu);
		//logerror("%s : MCU executes CHECKs synchro\n", machine().describe_context());
}

WRITE8_MEMBER(galpani2_state::galpani2_mcu_nmi1_w)//driven by CPU1's int5 ISR
{
//for galpan2t:
//Triggered from 'maincpu' (00007D60),once, with no command, using alternate line, during init
//Triggered from 'maincpu' (000080BE),once, for unknown command, during init
//Triggered from 'maincpu' (0000741E),from here on...driven by int5, even if there's no command
	if ( (data & 1) && !(m_old_mcu_nmi1 & 1) )  galpani2_mcu_nmi1();
	//if ( (data & 0x10) && !(m_old_mcu_nmi1 & 0x10) )    galpani2_mcu_nmi1(machine());
	//alternate line, same function?
	m_old_mcu_nmi1 = data;
}

WRITE8_MEMBER(galpani2_state::galpani2_mcu_nmi2_w)//driven by CPU2's int5 ISR
{
	if ( (data & 1) && !(m_old_mcu_nmi2 & 1) )  galpani2_mcu_nmi2();
	m_old_mcu_nmi2 = data;
}


/***************************************************************************


                            CPU#1 - Main + Sound


***************************************************************************/

WRITE8_MEMBER(galpani2_state::galpani2_coin_lockout_w)
{
		coin_counter_w(machine(), 0, data & 0x01);
		coin_counter_w(machine(), 1, data & 0x02);
		coin_lockout_w(machine(), 0,~data & 0x04);
		coin_lockout_w(machine(), 1,~data & 0x08);
		// & 0x10     CARD in lockout?
		// & 0x20     CARD in lockout?
		// & 0x40     CARD out
}

WRITE8_MEMBER(galpani2_state::galpani2_oki1_bank_w)
{
	UINT8 *ROM = memregion("oki1")->base();
	logerror("%s : %s bank %08X\n",machine().describe_context(),tag(),data);
	memcpy(ROM + 0x30000, ROM + 0x40000 + 0x10000 * (~data & 0xf), 0x10000);
}

WRITE8_MEMBER(galpani2_state::galpani2_oki2_bank_w)
{
	m_oki2->set_bank_base(0x40000 * (data & 0xf) );
	logerror("%s : %s bank %08X\n",machine().describe_context(),tag(),data);
}


static ADDRESS_MAP_START( galpani2_mem1, AS_PROGRAM, 16, galpani2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                             // ROM
	AM_RANGE(0x100000, 0x10ffff) AM_RAM AM_SHARE("ram")     // Work RAM
	AM_RANGE(0x110000, 0x11000f) AM_RAM                                             // ? corrupted? stack dumper on POST failure, pc+sr on gp2se
	AM_RANGE(0x300000, 0x301fff) AM_RAM                                             // ?
	AM_RANGE(0x302000, 0x303fff) AM_RAM AM_SHARE("spriteram")   // Sprites
	AM_RANGE(0x304000, 0x30401f) AM_DEVREADWRITE("kan_spr", kaneko16_sprite_device, kaneko16_sprites_regs_r, kaneko16_sprites_regs_w)
//  AM_RANGE(0x308000, 0x308001) AM_WRITENOP                                        // ? 0 at startup
//  AM_RANGE(0x30c000, 0x30c001) AM_WRITENOP                                        // ? hblank effect ?
	AM_RANGE(0x310000, 0x3101ff) AM_RAM_DEVWRITE("bg8palette", palette_device, write) AM_SHARE("bg8palette")    // ?
	AM_RANGE(0x314000, 0x314001) AM_WRITENOP                                        // ? flip backgrounds ?
	AM_RANGE(0x318000, 0x318001) AM_READWRITE(galpani2_eeprom_r, galpani2_eeprom_w) // EEPROM
	AM_RANGE(0x380000, 0x387fff) AM_RAM                                             // Palette?
	AM_RANGE(0x388000, 0x38ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")   // Palette
//  AM_RANGE(0x390000, 0x3901ff) AM_WRITENOP                                        // ? at startup of service mode

	AM_RANGE(0x400000, 0x43ffff) AM_RAM AM_SHARE("bg8.0")    // Background 0
	AM_RANGE(0x440000, 0x440001) AM_RAM AM_SHARE("bg8_scrolly.0")           // Background 0 Scroll Y
	AM_RANGE(0x480000, 0x480001) AM_RAM AM_SHARE("bg8_scrollx.0")           // Background 0 Scroll X
//  AM_RANGE(0x4c0000, 0x4c0001) AM_WRITENOP                                        // ? 0 at startup only
	AM_RANGE(0x500000, 0x53ffff) AM_RAM AM_SHARE("bg8.1")    // Background 1
	AM_RANGE(0x540000, 0x540001) AM_RAM AM_SHARE("bg8_scrolly.1")           // Background 1 Scroll Y
	AM_RANGE(0x580000, 0x580001) AM_RAM AM_SHARE("bg8_scrollx.1")           // Background 1 Scroll X
//  AM_RANGE(0x5c0000, 0x5c0001) AM_WRITENOP                                        // ? 0 at startup only

	AM_RANGE(0x540572, 0x540573) AM_READNOP                                         // ? galpani2 at F0A4
	AM_RANGE(0x54057a, 0x54057b) AM_READNOP                                         // ? galpani2 at F148
	AM_RANGE(0x54059a, 0x54059b) AM_READNOP                                         // ? galpani2 at F0A4
	AM_RANGE(0x5405a2, 0x5405a3) AM_READNOP                                         // ? galpani2 at F0A4 and F148
	AM_RANGE(0x5405aa, 0x5405ab) AM_READNOP                                         // ? galpani2 at F0A4 and F148
	AM_RANGE(0x5405b2, 0x5405b3) AM_READNOP                                         // ? galpani2 at F0A4 and F148
	AM_RANGE(0x5405ba, 0x5405bb) AM_READNOP                                         // ? galpani2 at F0A4 and F148
	AM_RANGE(0x5405c2, 0x5405c3) AM_READNOP                                         // ? galpani2 at F0A4 and F148
	AM_RANGE(0x5405ca, 0x5405cb) AM_READNOP                                         // ? galpani2 at F148

	AM_RANGE(0x600000, 0x600001) AM_NOP                                        // Watchdog
	AM_RANGE(0x640000, 0x640001) AM_WRITE8(galpani2_mcu_init_w, 0x00ff          )   // ? 0 before resetting and at startup, Reset mcu ?
	AM_RANGE(0x680000, 0x680001) AM_WRITE8(galpani2_mcu_nmi1_w, 0x00ff)             // ? 0 -> 1 -> 0 (lev 5) / 0 -> $10 -> 0
	AM_RANGE(0x6c0000, 0x6c0001) AM_WRITE8(galpani2_coin_lockout_w, 0xff00      )   // Coin + Card Lockout
	AM_RANGE(0x780000, 0x780001) AM_READ_PORT("DSW1_P1")
	AM_RANGE(0x780002, 0x780003) AM_READ_PORT("DSW2_P2")
	AM_RANGE(0x780004, 0x780005) AM_READ_PORT("SPECIAL")
	AM_RANGE(0x780006, 0x780007) AM_READ_PORT("SERVICE")
	AM_RANGE(0xc00000, 0xc00001) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x00ff  )   // 2 x OKIM6295
	AM_RANGE(0xc40000, 0xc40001) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x00ff  )   //
	AM_RANGE(0xc80000, 0xc80001) AM_WRITE8(galpani2_oki1_bank_w, 0x00ff         )   //
	AM_RANGE(0xcc0000, 0xcc0001) AM_WRITE8(galpani2_oki2_bank_w, 0x00ff         )   //
ADDRESS_MAP_END


/***************************************************************************


                            CPU#2 - Backgrounds


***************************************************************************/


WRITE16_MEMBER(galpani2_state::subdatabank_select_w)
{
	data &= mem_mask;

	if (data & 0xfffc) printf("subdatabank_select_w %04x\n", data);
	membank("subdatabank")->set_entry(data&3);
}


static ADDRESS_MAP_START( galpani2_mem2, AS_PROGRAM, 16, galpani2_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM                                                             // ROM
	AM_RANGE(0x100000, 0x13ffff) AM_RAM AM_SHARE("ram2")                                        // Work RAM
	AM_RANGE(0x400000, 0x5fffff) AM_RAM AM_SHARE("bg15")  // bg15
//  AM_RANGE(0x600000, 0x600001) AM_NOP // ? 0 at startup only
//  AM_RANGE(0x640000, 0x640001) AM_WRITENOP                                // ? 0 at startup only
//  AM_RANGE(0x680000, 0x680001) AM_WRITENOP                                // ? 0 at startup only
//  AM_RANGE(0x6c0000, 0x6c0001) AM_WRITENOP                                // ? 0 at startup only
	AM_RANGE(0x700000, 0x700001) AM_NOP                                 // Watchdog
//  AM_RANGE(0x740000, 0x740001) AM_WRITENOP                                // ? Reset mcu
	AM_RANGE(0x780000, 0x780001) AM_WRITE8(galpani2_mcu_nmi2_w, 0x00ff)             // ? 0 -> 1 -> 0 (lev 5)
	AM_RANGE(0x7c0000, 0x7c0001) AM_WRITE(subdatabank_select_w)   // Rom Bank
	AM_RANGE(0x800000, 0xffffff) AM_ROMBANK("subdatabank")
ADDRESS_MAP_END

/***************************************************************************


                                Input Ports


***************************************************************************/

static INPUT_PORTS_START( galpani2 )
	PORT_START("DSW1_P1")   /* 780000.w */
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Difficulty ) )       PORT_DIPLOCATION("SW B:8,7,6")
	PORT_DIPSETTING(      0x0007, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0004, "Normal +" )
	PORT_DIPSETTING(      0x0003, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0001, "Ultra Hard" )
	PORT_DIPSETTING(      0x0000, "God Hands" )
	PORT_DIPNAME( 0x0008, 0x0008, "Picture Mode" )          PORT_DIPLOCATION("SW B:5")
	PORT_DIPSETTING(      0x0008, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Adult" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW B:4,3")
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "1" )
	PORT_DIPSETTING(      0x0010, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x00c0, 0x00c0, "Max Unit of Players" )       PORT_DIPLOCATION("SW B:2,1")
	PORT_DIPSETTING(      0x00c0, "9" )
	PORT_DIPSETTING(      0x0080, "1" )
	PORT_DIPSETTING(      0x0040, "4" )
	PORT_DIPSETTING(      0x0000, "6" )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)

	PORT_START("DSW2_P2")   /* 780002.w */
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW A:8,7,6,5")
	PORT_DIPSETTING(      0x000f, "1 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x000e, "2 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x000d, "3 Coin/1 Credit  3/1" )
	PORT_DIPSETTING(      0x000c, "4 Coin/1 Credit  4/1" )
	PORT_DIPSETTING(      0x000b, "5 Coin/1 Credit  5/1" )
	PORT_DIPSETTING(      0x000a, "2 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x0009, "3 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x0008, "4 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x0007, "5 Coin/1 Credit  1/1" )
	PORT_DIPSETTING(      0x0006, "2 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x0005, "3 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x0004, "4 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x0003, "5 Coin/1 Credit  2/1" )
	PORT_DIPSETTING(      0x0002, "1 Coin/2 Credit  1/2" )
	PORT_DIPSETTING(      0x0001, "1 Coin/3 Credit  1/3" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Card Dispenser" )        PORT_DIPLOCATION("SW A:4")
	PORT_DIPSETTING(      0x0010, "Used" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Unused ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Flip_Screen ) )      PORT_DIPLOCATION("SW A:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW A:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW A:1" )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)

	PORT_START("SPECIAL")   /* 780004.w */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SPECIAL )  // CARD full
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SPECIAL )  // CARD full
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SPECIAL )  // CARD empty

	PORT_START("SERVICE")   /* 780006.w */
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_SERVICE2  ) // this button is used in gp2se as an alt way to bring up the service menu, booting with it held down breaks the game tho!
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE1 )

	//missing "test" input
INPUT_PORTS_END

static INPUT_PORTS_START( gp2se )
	PORT_INCLUDE( galpani2 )

	PORT_MODIFY("DSW1_P1")  /* 780000.w */
	PORT_DIPUNUSED_DIPLOC( 0x0008, IP_ACTIVE_LOW, "SW B:5" ) // picture mode is "normal fix" and cannot be changed

	PORT_MODIFY("DSW2_P2")  /* 780002.w */
	PORT_DIPNAME( 0x0010, 0x0010, "Card Dispenser" )        PORT_DIPLOCATION("SW A:4") // Reversed compared to other sets.
	PORT_DIPSETTING(      0x0000, "Used" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Unused ) )

	//missing "test" input
INPUT_PORTS_END

/***************************************************************************


                                Graphics Layouts


***************************************************************************/

/*
    16x16x8 made of four 8x8x8 tiles arranged like: 01
                                                    23
*/
static const gfx_layout layout_16x16x8 =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8),   STEP8(8*8*8*1,8)   },
	{ STEP8(0,8*8), STEP8(8*8*8*2,8*8) },
	16*16*8
};

static GFXDECODE_START( galpani2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x8, 0,  0x40    ) // [0] Sprites
GFXDECODE_END

/***************************************************************************


                                Machine Drivers


***************************************************************************/


/* CPU#1 Interrups , lev 3,4 & 5 are tested on power up. The rest is rte, but lev 6 */
TIMER_DEVICE_CALLBACK_MEMBER(galpani2_state::galpani2_interrupt1)
{
	int scanline = param;

	if(scanline == 240)
			m_maincpu->set_input_line(5, HOLD_LINE);

	/* MCU related? */
	if(scanline == 128)
	{
		m_maincpu->set_input_line(3, HOLD_LINE);
		m_maincpu->set_input_line(4, HOLD_LINE);
	}

	if(scanline == 0)
			m_maincpu->set_input_line(6, HOLD_LINE); // hblank?
}

/* CPU#2 interrupts, lev 3,4 & 5 are tested on power up. The rest is rte, but lev 7 */
TIMER_DEVICE_CALLBACK_MEMBER(galpani2_state::galpani2_interrupt2)
{
	int scanline = param;

	if(scanline == 240)
		m_subcpu->set_input_line(5, HOLD_LINE);

	if(scanline == 128)
		m_subcpu->set_input_line(4, HOLD_LINE);

	if(scanline == 0)
		m_subcpu->set_input_line(3, HOLD_LINE);
}

static MACHINE_CONFIG_START( galpani2, galpani2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_27MHz/2)       /* Confirmed on galpani2i PCB */
	MCFG_CPU_PROGRAM_MAP(galpani2_mem1)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("m_scantimer", galpani2_state, galpani2_interrupt1, "screen", 0, 1)
	//MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_CPU_ADD("sub", M68000, XTAL_27MHz/2)           /* Confirmed on galpani2i PCB */
	MCFG_CPU_PROGRAM_MAP(galpani2_mem2)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("s_scantimer", galpani2_state, galpani2_interrupt2, "screen", 0, 1)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 0, 256-1-16)
	MCFG_SCREEN_UPDATE_DRIVER(galpani2_state, screen_update_galpani2)
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", galpani2)
	MCFG_PALETTE_ADD("palette", 0x4000)    // sprites
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)

	MCFG_PALETTE_ADD("bg8palette", 0x200/2) // bg8
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)

	MCFG_PALETTE_ADD("bgpalette", 32768) /* 32768 static colors for the bg */
	MCFG_PALETTE_FORMAT(xGGGGGRRRRRBBBBB)
	MCFG_PALETTE_INIT_OWNER(galpani2_state,galpani2)

	MCFG_DEVICE_ADD_KC002_SPRITES
	kaneko16_sprite_device::set_offsets(*device, 0x10000 - 0x16c0 + 0xc00, 0);
	MCFG_KANEKO16_SPRITE_GFXDECODE("gfxdecode")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", XTAL_20MHz/10, OKIM6295_PIN7_HIGH)    /* Confirmed on galpani2i PCB */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki2", XTAL_20MHz/10, OKIM6295_PIN7_HIGH)    /* Confirmed on galpani2i PCB */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


/***************************************************************************


                                Roms Loading


***************************************************************************/

//POST's displayed checksums (ROM $0-$FFFFF) or (ROM $0-$FFFFF even)/(ROM $1-$FFFFF odd)
//
//galpani2 = 6A6C                bkg layer offset
//galpani2g= 15A3                bkg layer offset
//galpani2i= 54FC                bkg layer offset
//galpani2t= 18A3                bkg layer offset
//galpani2j= 08E1 / A582         bkg layer OK          has demo
//gp2quiz  = 78D6 / 84A0         bkg layer OK          has demo
//gp2se    = 6D8C / FCE4 (Japan) bkg layer unknown

/***************************************************************************

                                Gals Panic II


Location      Device         File ID       Checksum
---------------------------------------------------
CPU U134      27C4001      G001T1-U134-0     21A3   [ CPU 2 PROG ]
CPU U125       27C010      G002T1-U125-0     8072   [ CPU 1 PROG ]
CPU U126       27C010      G003T1-U126-0     C9C4   [ CPU 1 PROG ]
CPU U133      27C4001      G204T1-U133-0     9EF7   [ CPU 2 PROG ]
ROM U27        27C020      G204T1-U27-00     CA1D
ROM U33        27C020      G204T1-U33-00     AF5D
ROM U3       LH538500      GP2-100-0043      C90A   [ SOUND DATA ]
ROM U7       LH538500      GP2-101-0044      CFEF   [ SOUND DATA ]
ROM U10     KM2316000      GP2-102-0045      1558   [ SOUND DATA ]
ROM U21      LM538500      GP2-200-0046      2E2E
ROM U20      LH538500      GP2-201-0047      DB6E
ROM U19      LH538500      GP2-202-0048      E181
ROM U18      LH538500      GP2-203-0049      E520
ROM U51      LH538500      GP2-300A-0052     50E9
ROM U52      LH538500      GP2-300B-0053     DC51
ROM U60     KM2316000      GP2-301-0035      35F6
ROM U59     KM2316000      GP2-302-0036      BFF5
ROM U58     KM2316000      GP2-303-0037      B860
ROM U57     KM2316000      GP2-304-0038      BD55
ROM U56     KM2316000      GP2-305-0039      D0F4
ROM U55     KM2316000      GP2-306-0040      1311
ROM U54     KM2316000      GP2-307-0041      1874
ROM U53     KM2316000      GP2-308-0042      375F
ROM U46      LH538500      GP2-309A-0050     97ED
ROM U47      LH538500      GP2-309B-0051     2C13
ROM U48      LH538500      GP2-309A-0055     2059
ROM U75      GAL16V8A      S075.JED          08F9
ROM U76      GAL16V8A      S076.JED          0878
ROM U1      PEEL18CV8      S001.JED          03CA
ROM U14     PEEL18CV8      S014.JED          039A


Notes:  CPU - Main PCB   Z04G2-003
        ROM - ROM PCB    Z04G2-SUB3

        Checksums for the PLDs are the JEDEC checksums, not the file checksums

Brief Hardware Overview
-----------------------

CPU1         - 68HC000-16
CPU2         - 68HC000-16
Sound     2x - M6295

Custom ICs   - 10x PQFPs

***************************************************************************/

ROM_START( galpani2 )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000a2.u165-1", 0x000000, 0x080000, CRC(0c6dfe3f) SHA1(22b16eaa3fee7f8f8434c6775255b25c8d960620) )
	ROM_LOAD16_BYTE( "g001a2.u164-1", 0x000001, 0x080000, CRC(b3a5951f) SHA1(78cf2d85a8b3cd46c5e30fd13b474af2ed2ee09b) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002a2.u64-1", 0x000000, 0x020000, CRC(c0b94eaf) SHA1(4f3a65b238b31ee8d256b7025253f01eaf6e55d5) )
	ROM_LOAD16_BYTE( "g003a2.u63-1", 0x000001, 0x020000, CRC(0d30725d) SHA1(d4614f9ffb930c4ea36cb3fbacffe63060e92402) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD( "gp2-204a.188", 0x400000, 0x080000, CRC(613ad1d5) SHA1(0ea1d4306c3e1eca3d207be2f72214fb36db0d75) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

ROM_START( galpani2e )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000a2.u165-1", 0x000000, 0x080000, CRC(0c6dfe3f) SHA1(22b16eaa3fee7f8f8434c6775255b25c8d960620) )
	ROM_LOAD16_BYTE( "g001a2.u164-1", 0x000001, 0x080000, CRC(b3a5951f) SHA1(78cf2d85a8b3cd46c5e30fd13b474af2ed2ee09b) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002a1-u125-1.bin", 0x000000, 0x020000, CRC(100e76b3) SHA1(24a259ee427cd7a6e487520a712dc7ef632dc5d6) )
	ROM_LOAD16_BYTE( "g003a1-u126-1.bin", 0x000001, 0x020000, CRC(0efe7835) SHA1(c7eecacdf101c0515da504cc77512f27b61b2ab7) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD( "gp2-204a.188", 0x400000, 0x080000, CRC(613ad1d5) SHA1(0ea1d4306c3e1eca3d207be2f72214fb36db0d75) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

// from Single board PCB
ROM_START( galpani2i )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000i1u165", 0x000000, 0x080000, CRC(b802ad64) SHA1(a0bef1f037a72c379f43ff6d22e441d988b68fcc) )
	ROM_LOAD16_BYTE( "g001i1u164", 0x000001, 0x080000, CRC(d342fe5c) SHA1(7add3488d1e8eaec9e1f5cc47e4e7147822923bc) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002a2.u64-1", 0x000000, 0x020000, CRC(c0b94eaf) SHA1(4f3a65b238b31ee8d256b7025253f01eaf6e55d5) )
	ROM_LOAD16_BYTE( "g003a2.u63-1", 0x000001, 0x020000, CRC(0d30725d) SHA1(d4614f9ffb930c4ea36cb3fbacffe63060e92402) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD( "gp2-204a.u188", 0x400000, 0x080000, CRC(ba83c918) SHA1(04a70dc7e33d853d84b88dc82c9b066696475cee) ) // different from Asia set

	ROM_REGION( 0x1c0000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )
	/* Sound samples: unknown load position, but included here to retain the rom in this set */
	ROM_LOAD( "g104i1u062-0", 0x140000, 0x080000, CRC(117ee59e) SHA1(7deb9b71363ff0bf239f9ad21171ddd9bfc49eb4) )    // $8 x $10000, 1st is just audio data, no header (Italian samples data)

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

// Single board PCB
ROM_START( galpani2gs ) // basically the same as the Italy set but with different region byte and German sample rom
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000g1.u165-2", 0x000000, 0x080000, CRC(9f6952bb) SHA1(d7dde397589978ec88958ce191570d6aa5789bc6) )
	ROM_LOAD16_BYTE( "g001g1.u164-2", 0x000001, 0x080000, CRC(d342fe5c) SHA1(7add3488d1e8eaec9e1f5cc47e4e7147822923bc) ) // == italian set

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g003g1.u65-2", 0x000000, 0x020000, CRC(c0b94eaf) SHA1(4f3a65b238b31ee8d256b7025253f01eaf6e55d5) )
	ROM_LOAD16_BYTE( "g002g1.u64-2", 0x000001, 0x020000, CRC(0d30725d) SHA1(d4614f9ffb930c4ea36cb3fbacffe63060e92402) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD( "gp2-204a.u188", 0x400000, 0x080000, CRC(ba83c918) SHA1(04a70dc7e33d853d84b88dc82c9b066696475cee) ) // different from Asia set

	ROM_REGION( 0x1c0000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )
	/* Sound samples: unknown load position, but included here to retain the rom in this set */
	ROM_LOAD( "g104g1.u62-0", 0x140000, 0x080000, CRC(03539013) SHA1(36b96c59c59d0e747eb000472c22a30de0810902) )   // $8 x $10000, 1st is just audio data, no header (German sample data)

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

ROM_START( galpani2g )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000g1.u133-0", 0x000000, 0x080000, CRC(5a9c4886) SHA1(6fbc443612e72bafc5cac30de78c72815db20c4c) )
	ROM_LOAD16_BYTE( "g001g1.u134-0", 0x000001, 0x080000, CRC(c92937c3) SHA1(0c9e894c0e23e319bd2d01ec573f02ed510e3ed6) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002t1.125", 0x000000, 0x020000, CRC(a3034e1c) SHA1(493e4be36f2aea0083d5d37e16486ed66dab952e) )
	ROM_LOAD16_BYTE( "g003t1.126", 0x000001, 0x020000, CRC(20d3a2ad) SHA1(93450e5a23456c242ebf1a3560013a17c6b05354) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD16_BYTE( "g300a0.u44-00", 0x0000000, 0x080000, CRC(50406294) SHA1(fc1165b7b31a44ab204cd5ac3e7b2733ed6b1534) )
	ROM_LOAD16_BYTE( "g300a1.u41-00", 0x0000001, 0x080000, CRC(d26b7c4f) SHA1(b491170010977ba1e5111893937cc6bab0539e7d) )
	ROM_LOAD16_BYTE( "g300b0.u45-00", 0x0100000, 0x080000, CRC(9637934c) SHA1(d3b39d9f44825bdf24d4aa39ca32035bc5af4905) )
	ROM_LOAD16_BYTE( "g300b1.u42-00", 0x0100001, 0x080000, CRC(d72e154b) SHA1(e367c8f9af47b999fcba4afcd293565bad2038ec) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD16_BYTE( "g204a0.u33-00", 0x400000, 0x040000, CRC(2867cbfd) SHA1(89af600fb33ce72a7a3fbdf9ff05a4916454a205) )
	ROM_LOAD16_BYTE( "g204a1.u27-00", 0x400001, 0x040000, CRC(c50503bc) SHA1(5003aa414660358900857901d5e9eca6739f14e3) )

	ROM_REGION( 0x1c0000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )
	/* Sound samples: unknown load position, but included here to retain the rom in this set */
	ROM_LOAD( "g104g1.u04-00", 0x140000, 0x080000, CRC(03539013) SHA1(36b96c59c59d0e747eb000472c22a30de0810902) )   // $8 x $10000, 1st is just audio data, no header (German sample data)

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END


ROM_START( galpani2e2 )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000i1-u133-0.u133", 0x000000, 0x080000, CRC(7df7b759) SHA1(2479a6389649ee6042b175b71d7ed54bc116add5) )   //english version specific
	ROM_LOAD16_BYTE( "g001i1-u134-0.u134", 0x000001, 0x080000, CRC(c92937c3) SHA1(0c9e894c0e23e319bd2d01ec573f02ed510e3ed6) )   //same as german version

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002i1.125", 0x000000, 0x020000, CRC(a3034e1c) SHA1(493e4be36f2aea0083d5d37e16486ed66dab952e) )
	ROM_LOAD16_BYTE( "g003i1.126", 0x000001, 0x020000, CRC(20d3a2ad) SHA1(93450e5a23456c242ebf1a3560013a17c6b05354) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD16_BYTE( "g300a0.u44-00", 0x0000000, 0x080000, CRC(50406294) SHA1(fc1165b7b31a44ab204cd5ac3e7b2733ed6b1534) )
	ROM_LOAD16_BYTE( "g300a1.u41-00", 0x0000001, 0x080000, CRC(d26b7c4f) SHA1(b491170010977ba1e5111893937cc6bab0539e7d) )
	ROM_LOAD16_BYTE( "g300b0.u45-00", 0x0100000, 0x080000, CRC(9637934c) SHA1(d3b39d9f44825bdf24d4aa39ca32035bc5af4905) )
	ROM_LOAD16_BYTE( "g300b1.u42-00", 0x0100001, 0x080000, CRC(d72e154b) SHA1(e367c8f9af47b999fcba4afcd293565bad2038ec) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD16_BYTE( "g204a0.u33-00", 0x400000, 0x040000, CRC(2867cbfd) SHA1(89af600fb33ce72a7a3fbdf9ff05a4916454a205) )
	ROM_LOAD16_BYTE( "g204a1.u27-00", 0x400001, 0x040000, CRC(c50503bc) SHA1(5003aa414660358900857901d5e9eca6739f14e3) )

	ROM_REGION( 0x1c0000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )
	/* Sound samples: unknown load position, but included here to retain the rom in this set */
	//ROM_LOAD( "g104g1.u04-00", 0x140000, 0x080000, CRC(03539013) SHA1(36b96c59c59d0e747eb000472c22a30de0810902) )   // $8 x $10000, 1st is just audio data, no header // this is the german sound data? shouldn't be loaded in an English set??...

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END


ROM_START( galpani2t )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000t1.133", 0x000000, 0x080000, CRC(332048e7) SHA1(1a353d4b29f7a08158fc454309dc496df6b5b108) )
	ROM_LOAD16_BYTE( "g001t1.134", 0x000001, 0x080000, CRC(c92937c3) SHA1(0c9e894c0e23e319bd2d01ec573f02ed510e3ed6) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002t1.125", 0x000000, 0x020000, CRC(a3034e1c) SHA1(493e4be36f2aea0083d5d37e16486ed66dab952e) )
	ROM_LOAD16_BYTE( "g003t1.126", 0x000001, 0x020000, CRC(20d3a2ad) SHA1(93450e5a23456c242ebf1a3560013a17c6b05354) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300a.052", 0x0000000, 0x100000, CRC(09ebedba) SHA1(3c06614633f0da03facb5199deac492b8ce07257) )
	ROM_LOAD( "gp2-300b.053", 0x0100000, 0x100000, CRC(d7d12920) SHA1(4b6e01cc0ac5192758f4b3d26f102905b2b5e8ac) )
	ROM_LOAD( "gp2-301.035", 0x0200000, 0x200000, CRC(e71e749d) SHA1(420c4c085e89d9641a84e34fa870df2bc02165b6) )
	ROM_LOAD( "gp2-302.036", 0x0400000, 0x200000, CRC(832ebbb0) SHA1(a753285d874fcab979e70d6a289cf9fcd48affc6) )
	ROM_LOAD( "gp2-303.037", 0x0600000, 0x200000, CRC(36c872d0) SHA1(e0aa3089dfa1765ba70ce60e8696b1ba87c95703) )
	ROM_LOAD( "gp2-304.038", 0x0800000, 0x200000, CRC(7200f918) SHA1(6d23bd371b32319fdd08923deb81278b36b9cd79) )
	ROM_LOAD( "gp2-305.039", 0x0a00000, 0x200000, CRC(a308dc4b) SHA1(db40329c383c765471941ab89fded6b8789d29c7) )
	ROM_LOAD( "gp2-306.040", 0x0c00000, 0x200000, CRC(cd294225) SHA1(c51c95d5edd5e5d7191ccbfa1ba2e92199bb04b9) )
	ROM_LOAD( "gp2-307.041", 0x0e00000, 0x200000, CRC(0fda01af) SHA1(ca30d995ff8d83b46c05898a2ecde3f08a95c788) )
	ROM_LOAD( "gp2-308.042", 0x1000000, 0x200000, CRC(3c806376) SHA1(5c440a0cfd5d5c07ff074bc0c2563956d256a80e) )
	ROM_LOAD16_BYTE( "gp2-309a.050", 0x1200000, 0x100000, CRC(2c025ec3) SHA1(bc25ad92415e662d6b0f845aa4621a733fbf5a48) )
	ROM_LOAD16_BYTE( "gp2-309b.051", 0x1200001, 0x100000, CRC(e8bf1730) SHA1(0d9a446aecc19a43368550348745c9b167ec4941) )
	ROM_LOAD( "gp2-310a.055", 0x1400000, 0x100000, CRC(01eca246) SHA1(19cb35d7873b84486f9105127a1e3cf3235d3109) )

	ROM_REGION( 0x480000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200.046", 0x080000, 0x080000, CRC(11b49470) SHA1(d11c2374a7c9b9b0d1f27c29759b16630700561d) )
	ROM_CONTINUE(            0x000000, 0x080000             )
	ROM_LOAD( "gp2-201.047", 0x180000, 0x080000, CRC(2f6392b4) SHA1(67446974c00481a7a806f4bc5b10eb6e442a1186) )
	ROM_CONTINUE(            0x100000, 0x080000             )
	ROM_LOAD( "gp2-202.048", 0x280000, 0x080000, CRC(c8177181) SHA1(30d0a49334e370eb1b45d2eb6501df3f857a95d5) )
	ROM_CONTINUE(            0x200000, 0x080000             )
	ROM_LOAD( "gp2-203.049", 0x380000, 0x080000, CRC(14e0cb38) SHA1(d9a778ebf0c6b67bee5f6f7016cb9ead96c6a992) )
	ROM_CONTINUE(            0x300000, 0x080000             )
	ROM_LOAD16_BYTE( "g204t1.33", 0x400000, 0x040000, CRC(65a1f838) SHA1(ccc3bb4a4f4ea1677caa1a3a51bc0a13b4b619c7) )
	ROM_LOAD16_BYTE( "g204t1.27", 0x400001, 0x040000, CRC(39059f66) SHA1(6bf41738033a13b63d96babf827c73c914323425) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100.043", 0x040000, 0x100000, CRC(4235ac5b) SHA1(7e35831523fbb2d0587b9ab93c13b2b43dc481a8) ) // $10 x $10000
	ROM_COPY( "oki1", 0x0c0000, 0, 0x40000 )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.045", 0x180000, 0x080000, CRC(b4bee779) SHA1(a41098e4b8e48577719dc4bd7f09f5e893e8b388) ) //  $8 x $40000
	ROM_CONTINUE(            0x000000, 0x180000 )
	ROM_LOAD( "gp2-101.044", 0x280000, 0x080000, CRC(f75ba6a0) SHA1(91cc0c019a7ebfa2562bbe570af029f00b5e0699) ) //  $4 x $40000
	ROM_CONTINUE(            0x200000, 0x080000 )
ROM_END

/*

Gals Panic II - Quiz Version (c) 1993 Kaneko

CPUs: 68HC000-16 (x2)
Sound: OKI6295 (x2)
Customs: KC-BYO KA05-1068 (x2), KC-TAS KA07-1209, KC-YUU KA06-0041, KC-SHU KA03-1849 (x2), KC-001 (x2), PISCES, KC-002
RAM: 6116 (x6), 52B256 (x4), 42101 (x2), 42426 (x2), 6264 (x2), 431000 (x8)
X1: 27 MHz
X2: 16 MHz
X3: 33.333 MHz
X4: 20 MHz

--- NOTE 07/01/07
this just appears to be a regular japanese version, NOT the quiz version, unless it's using the wrong roms..

*/

ROM_START( galpani2j )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000j2.165", 0x000000, 0x080000, CRC(e0c5a03d) SHA1(e12457400ca8cd78674b44d7f4d664cfc0afc8c9) )
	ROM_LOAD16_BYTE( "g001j2.164", 0x000001, 0x080000, CRC(c8e12223) SHA1(0e0160565e95cb33dc6ad796225e995ed3baf8eb) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002j1.64",  0x000000, 0x020000, CRC(5e523829) SHA1(dad11e4a3348c988ff658609cf78a3fbee58064e) )
	ROM_LOAD16_BYTE( "g003j1.63",  0x000001, 0x020000, CRC(2a0d5f89) SHA1(0a7031c4b8b7bc757da25250dbb5fa1004205aeb) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300j.175", 0x000000, 0x200000, CRC(3a0afc1d) SHA1(91fba9074cc3c28e919053f0ea07b28d88b2ce5f) )
	ROM_LOAD( "gp2-301j.176", 0x200000, 0x200000, CRC(5b6d1709) SHA1(a7d35247fe71895f2b6169409aa0bdaef446804c) )
	ROM_LOAD16_BYTE( "gp2-302a.177", 0x400000, 0x100000, CRC(311fa273) SHA1(c2adeac45be701f6f474841755fac4347d44f844) )
	ROM_LOAD16_BYTE( "gp2-302b.178", 0x400001, 0x100000, CRC(80cb211b) SHA1(7567c9d1309edddb9c1fa68346506de48e91ca6a) )
	ROM_LOAD16_BYTE( "gp2-303a.179", 0x600000, 0x100000, CRC(162d83b7) SHA1(16daf2ba09e63eaca5e50c944472773b1774c946) )
	ROM_LOAD16_BYTE( "gp2-303b.180", 0x600001, 0x100000, CRC(458a1fbc) SHA1(971548ec8cce592773e762a0c972264013b7cb8d) )

	ROM_REGION( 0x480000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200j.189", 0x000000, 0x200000, CRC(2f81e519) SHA1(c07f4dad15b6f7f1fb867f773c0ada309d172326) )
	ROM_LOAD( "gp2-201j.171", 0x200000, 0x200000, CRC(bbe404e0) SHA1(198db9a6c6ec97ed8fd32d946051ba4d6e4bd354) )
	ROM_LOAD16_BYTE( "g204j0.169", 0x400000, 0x040000, CRC(212d8aab) SHA1(459f556978ef9a103279cf633fcc1cacb367ea61) )
	ROM_LOAD16_BYTE( "g204j1.170", 0x400001, 0x040000, CRC(bfd89343) SHA1(884d17b3302643d86f84a4a4917de850c5bf8924) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100j.61", 0x000000, 0x100000, CRC(60382cbf) SHA1(766c50a3302bc11d54de49a2850522d93fc36ba2) )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102.59",  0x000000, 0x200000, CRC(1bed6ecd) SHA1(3208aacac64ac95fcb6eeef59986c3154c1c415b) )
	ROM_LOAD( "gp2-101.60",  0x200000, 0x100000, CRC(3c45134f) SHA1(a5362bfcc6beb6e776c1bce4544475f8947fccea) )
ROM_END

ROM_START( gp2se )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000j4.u165", 0x000000, 0x080000, CRC(d8258a7a) SHA1(12991392d7e70bfba394ec4ad49b427959ca019e) )
	ROM_LOAD16_BYTE( "g001j4.u164", 0x000001, 0x080000, CRC(23f706bf) SHA1(960c6e6c17f03072cecabfd52018e0351ff4b661) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002j4.u64",  0x000000, 0x020000, CRC(bcd4edd9) SHA1(17ae6fbf75d8e5333133737de926a36f5cd29661) )
	ROM_LOAD16_BYTE( "g003j4.u63",  0x000001, 0x020000, CRC(2fbe0194) SHA1(52da771ba813b27ec1a996b237c14dab9b33db82) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300-j-0071.u175", 0x000000, 0x200000, CRC(3a0afc1d) SHA1(91fba9074cc3c28e919053f0ea07b28d88b2ce5f) )
	ROM_LOAD( "gp2-301-j-0072.u176", 0x200000, 0x200000, CRC(5b6d1709) SHA1(a7d35247fe71895f2b6169409aa0bdaef446804c) )
	ROM_LOAD16_BYTE( "gp2-302a-0057.u177", 0x400000, 0x100000, CRC(311fa273) SHA1(c2adeac45be701f6f474841755fac4347d44f844) )
	ROM_LOAD16_BYTE( "gp2-302b-0058.u178", 0x400001, 0x100000, CRC(80cb211b) SHA1(7567c9d1309edddb9c1fa68346506de48e91ca6a) )
	ROM_LOAD16_BYTE( "gp2-303a-0063.u179", 0x600000, 0x100000, CRC(162d83b7) SHA1(16daf2ba09e63eaca5e50c944472773b1774c946) )
	ROM_LOAD16_BYTE( "gp2-303b-0064.u180", 0x600001, 0x100000, CRC(458a1fbc) SHA1(971548ec8cce592773e762a0c972264013b7cb8d) )
	ROM_LOAD16_BYTE( "g304aj4.u158", 0x800000, 0x080000, CRC(3a1c9d53) SHA1(058311da5f47036f5e388895e41abc1757ed8518) )
	ROM_LOAD16_BYTE( "g304bj4.u159", 0x800001, 0x080000, CRC(ae87cf2c) SHA1(a397e9441048ae6b5699b7e45f420c92903e8a96) )
	ROM_LOAD16_BYTE( "g305aj4.u160", 0x900000, 0x080000, CRC(2d4a8fbb) SHA1(8a00e6ba4e061678da4c41446df7278c9b4f26c2) )
	ROM_LOAD16_BYTE( "g305bj4.u161", 0x900001, 0x080000, CRC(53d13974) SHA1(29ca4d36f2a8153228c2eec8e9ef6a6bf712cb59) )

	ROM_REGION( 0x500000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200-j-0073.u189", 0x000000, 0x200000, CRC(2f81e519) SHA1(c07f4dad15b6f7f1fb867f773c0ada309d172326) )
	ROM_LOAD( "gp2-201-j-0074.u171", 0x200000, 0x200000, CRC(bbe404e0) SHA1(198db9a6c6ec97ed8fd32d946051ba4d6e4bd354) )
	ROM_LOAD16_BYTE( "g204aj4.u169", 0x400000, 0x080000, CRC(e5e32820) SHA1(9bdc0717feb8983c0d6d5edaa08bcebad4baace0) )
	ROM_LOAD16_BYTE( "g204bj4.u170", 0x400001, 0x080000, CRC(0bd46a73) SHA1(78b163431648db6bfa453e440584e781063529a9) )

	ROM_REGION( 0x140000, "oki1", 0 )   /* Samples */
	/* no u61 on this one */
	ROM_LOAD( "g104j4.u62", 0x000000, 0x080000, CRC(0546ea41) SHA1(cf351b496d93648a50fc0e84badb5bb855b681b4) )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102-0045.u59",  0x000000, 0x200000, CRC(1bed6ecd) SHA1(3208aacac64ac95fcb6eeef59986c3154c1c415b) )
	ROM_LOAD( "gp2-101-0044.u60",  0x200000, 0x100000, CRC(3c45134f) SHA1(a5362bfcc6beb6e776c1bce4544475f8947fccea) )
ROM_END


ROM_START( gp2quiz )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* CPU#1 Code */
	ROM_LOAD16_BYTE( "g000e3.u165-3", 0x000000, 0x080000, CRC(b6de2653) SHA1(a24daf5e6b6b268f60b1dbb374861c85f642cea5) )
	ROM_LOAD16_BYTE( "g001e3.u164-3", 0x000001, 0x080000, CRC(74e8d0e8) SHA1(d131be9f52ee79e1b82f46721c2ad5d71b3da649) )

	ROM_REGION( 0x40000, "sub", 0 )         /* CPU#2 Code */
	ROM_LOAD16_BYTE( "g002e3.u64-3",  0x000000, 0x020000, CRC(5e523829) SHA1(dad11e4a3348c988ff658609cf78a3fbee58064e) )
	ROM_LOAD16_BYTE( "g003e3.u63-3",  0x000001, 0x020000, CRC(2a0d5f89) SHA1(0a7031c4b8b7bc757da25250dbb5fa1004205aeb) )

	ROM_REGION16_BE( 0x2000000, "subdata", ROMREGION_ERASEFF )    /* Backgrounds (CPU2) */
	ROM_LOAD( "gp2-300-j-0071.u175", 0x000000, 0x200000, CRC(3a0afc1d) SHA1(91fba9074cc3c28e919053f0ea07b28d88b2ce5f) )
	ROM_LOAD( "gp2-301-j-0072.u176", 0x200000, 0x200000, CRC(5b6d1709) SHA1(a7d35247fe71895f2b6169409aa0bdaef446804c) )
	ROM_LOAD16_BYTE( "gp2-302a-0057.u177", 0x400000, 0x100000, CRC(311fa273) SHA1(c2adeac45be701f6f474841755fac4347d44f844) )
	ROM_LOAD16_BYTE( "gp2-302b-0058.u178", 0x400001, 0x100000, CRC(80cb211b) SHA1(7567c9d1309edddb9c1fa68346506de48e91ca6a) )
	ROM_LOAD16_BYTE( "gp2-303a-0063.u179", 0x600000, 0x100000, CRC(162d83b7) SHA1(16daf2ba09e63eaca5e50c944472773b1774c946) )
	ROM_LOAD16_BYTE( "gp2-303b-0064.u180", 0x600001, 0x100000, CRC(458a1fbc) SHA1(971548ec8cce592773e762a0c972264013b7cb8d) )

	ROM_REGION( 0x500000, "gfx1", 0 )   /* Sprites */
	ROM_LOAD( "gp2-200-j-0073.u189", 0x000000, 0x200000, CRC(2f81e519) SHA1(c07f4dad15b6f7f1fb867f773c0ada309d172326) )
	ROM_LOAD( "gp2-201-j-0074.u171", 0x200000, 0x200000, CRC(bbe404e0) SHA1(198db9a6c6ec97ed8fd32d946051ba4d6e4bd354) )
	ROM_LOAD16_BYTE( "g204a3.u169-3", 0x400000, 0x080000, CRC(92a837b7) SHA1(f581e1f7754f1fb20255c6c55ffc4e486d867111) )
	ROM_LOAD16_BYTE( "g204a4.u170-3", 0x400001, 0x080000, CRC(3c2dd1cd) SHA1(d5267ad6f51283191174988ac0519c0e0aa6552f) )

	ROM_REGION( 0x180000, "oki1", 0 )   /* Samples */
	ROM_LOAD( "gp2-100-0043.u61", 0x000000, 0x100000, CRC(a61e8868) SHA1(ad84ae00ebe7c70a36b1aa75e743686a0193e5d9) )
	ROM_LOAD( "g104a3.u62-3", 0x100000, 0x080000, CRC(42b3470e) SHA1(c121ea6c98e6ff452f4bcc49c3a5179e99237128) )

	ROM_REGION( 0x300000, "oki2", 0 )   /* Samples */
	ROM_LOAD( "gp2-102-0045.u59",  0x000000, 0x200000, CRC(1bed6ecd) SHA1(3208aacac64ac95fcb6eeef59986c3154c1c415b) )
	ROM_LOAD( "gp2-101-0044.u60",  0x200000, 0x100000, CRC(3c45134f) SHA1(a5362bfcc6beb6e776c1bce4544475f8947fccea) )
ROM_END

GAME( 1993, galpani2,  0,        galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II (Asia)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2e, galpani2, galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II (English)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2e2,galpani2, galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II (English, 2 PCB ver.)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2g, galpani2, galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II (Germany, 2 PCB ver.)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2i, galpani2, galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II (Italy, single PCB)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2gs,galpani2, galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II (Germany, single PCB)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2t, galpani2, galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II (Taiwan)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1993, galpani2j, galpani2, galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II (Japan)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) // it is a 'quiz edition' but the title screen doesn't say, maybe all Japanese versions have the Quiz

GAME( 1993, gp2quiz,  0,        galpani2, galpani2, driver_device, 0, ROT90, "Kaneko", "Gals Panic II - Quiz Version", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) // this one has 'quiz edition' on the title screen

GAME( 1994, gp2se,    0,        galpani2, gp2se, driver_device,    0, ROT90, "Kaneko", "Gals Panic II' - Special Edition (Japan)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
