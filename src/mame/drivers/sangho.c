// license:BSD-3-Clause
// copyright-holders:David Haywood, Tomasz Slanina, Mariusz Wojcieszek
/*

Sang Ho Soft 'Puzzle Star' PCB

Driver by David Haywood, Tomasz Slanina and Mariusz Wojcieszek

Each board contains a custom FGPA on a sub-board with
a warning   "WARNING ! NO TOUCH..." printed on the PCB

A battery is connected to the underside of the sub-board
and if the battery dies the PCB is no-longer functional.

It is possible that important game code is stored within
the battery.

The ROMs for "Puzzle Star" don't appear to have code at 0
and all boards found so far have been dead.

The Sexy Boom board was working, but it may only be a
matter of time before that board dies too.

It is thought that these games are based on MSX hardware
as some of the Puzzle Star roms appear to be a hacked
MSX Bios.  If we're lucky then the FGPA may only contain
Sang Ho's MSX simulation, rather than any specific game code.

The FGPA is labeled 'Custom 3'

There is another covered chip on the PCB labeled 'Custom 2'
at U17.  It is unknown what this chip is.

Custom 1 is underneath the sub-board and is a UM3567 which
is a YM2413 compatible chip.

*** the custom chip with the warning appears to control banking etc.

Sexy Boom's DSW setting verified via Z80 code by stephh

TODO:
- pzlestar hangs at snippet 0x2ca0-0x2ca9, patching 0x2ca7 branch makes it to be fully playable (patched for now);
- pzlestar title screen uses sprites with screen 12, has wrong colors due of it;
- sexyboom slows down dramatically, presumably bankswitch related;

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/v9938.h"
#include "sound/2413intf.h"


class sangho_state : public driver_device
{
public:
	sangho_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_v9958(*this, "v9958") ,
		m_maincpu(*this, "maincpu") { }

	UINT8* m_ram;
	UINT8 m_sexyboom_bank[8];
	UINT8 m_pzlestar_mem_bank;
	UINT8 m_pzlestar_rom_bank;
	required_device<v9958_device> m_v9958;
	DECLARE_WRITE8_MEMBER(pzlestar_bank_w);
	DECLARE_WRITE8_MEMBER(pzlestar_mem_bank_w);
	DECLARE_READ8_MEMBER(pzlestar_mem_bank_r);
	DECLARE_WRITE8_MEMBER(sexyboom_bank_w);
	DECLARE_DRIVER_INIT(pzlestar);
	virtual void machine_start();
	DECLARE_MACHINE_RESET(pzlestar);
	DECLARE_MACHINE_RESET(sexyboom);
	TIMER_DEVICE_CALLBACK_MEMBER(sangho_interrupt);
	void pzlestar_map_banks();
	void sexyboom_map_bank(int bank);
	DECLARE_WRITE_LINE_MEMBER(msx_vdp_interrupt);
	required_device<cpu_device> m_maincpu;
	UINT8 m_sec_slot[4];
	DECLARE_READ8_MEMBER(sec_slot_r);
	DECLARE_WRITE8_MEMBER(sec_slot_w);
};

/*
    slot 0 selects RAM
    slot 1 selects ?
    slot 2 selects code ROMs
    slot 3 selects data ROMs
*/
void sangho_state::pzlestar_map_banks()
{
	int slot_select;

	// page 0
	slot_select = (m_pzlestar_mem_bank >> 0) & 0x03;
	switch(slot_select)
	{
		case 0:
			m_maincpu->space(AS_PROGRAM).install_read_bank(0x0000, 0x3fff, "bank1");
			m_maincpu->space(AS_PROGRAM).install_write_bank(0x0000, 0x3fff, "bank5");
			membank("bank1")->set_base(m_ram);
			membank("bank5")->set_base(m_ram);
			break;
		case 2:
			m_maincpu->space(AS_PROGRAM).install_read_bank(0x0000, 0x3fff, "bank1");
			m_maincpu->space(AS_PROGRAM).unmap_write(0x0000, 0x3fff);
			membank("bank1")->set_base(memregion("user1")->base()+ 0x10000);
			break;
		case 1:
		case 3:
			m_maincpu->space(AS_PROGRAM).unmap_read(0x0000, 0x3fff);
			m_maincpu->space(AS_PROGRAM).unmap_write(0x0000, 0x3fff);
			break;
	}

	// page 1
	slot_select = (m_pzlestar_mem_bank >> 2) & 0x03;
	switch(slot_select)
	{
		case 0:
			m_maincpu->space(AS_PROGRAM).install_read_bank(0x4000, 0x7fff, "bank2");
			m_maincpu->space(AS_PROGRAM).install_write_bank(0x4000, 0x7fff, "bank6");
			membank("bank2")->set_base(m_ram + 0x4000);
			membank("bank6")->set_base(m_ram + 0x4000);
			break;
		case 2:
			m_maincpu->space(AS_PROGRAM).install_read_bank(0x4000, 0x7fff, "bank2");
			m_maincpu->space(AS_PROGRAM).unmap_write(0x4000, 0x7fff);
			membank("bank2")->set_base(memregion("user1")->base()+ 0x18000);
			break;
		case 3:
			m_maincpu->space(AS_PROGRAM).install_read_bank(0x4000, 0x7fff, "bank2");
			m_maincpu->space(AS_PROGRAM).unmap_write(0x4000, 0x7fff);
			membank("bank2")->set_base(memregion("user1")->base()+ 0x20000 + (m_pzlestar_rom_bank*0x8000) + 0x4000);
			break;
		case 1:
			m_maincpu->space(AS_PROGRAM).unmap_read(0x4000, 0x7fff);
			m_maincpu->space(AS_PROGRAM).unmap_write(0x4000, 0x7fff);
			break;
	}

	// page 2
	slot_select = (m_pzlestar_mem_bank >> 4) & 0x03;
	switch(slot_select)
	{
		case 0:
			m_maincpu->space(AS_PROGRAM).install_read_bank(0x8000, 0xbfff, "bank3");
			m_maincpu->space(AS_PROGRAM).install_write_bank(0x8000, 0xbfff, "bank7");
			membank("bank3")->set_base(m_ram + 0x8000);
			membank("bank7")->set_base(m_ram + 0x8000);
			break;
		case 3:
			m_maincpu->space(AS_PROGRAM).install_read_bank(0x8000, 0xbfff, "bank3");
			m_maincpu->space(AS_PROGRAM).unmap_write(0x8000, 0xbfff);
			membank("bank3")->set_base(memregion("user1")->base()+ 0x20000 + (m_pzlestar_rom_bank*0x8000));
			break;
		case 1:
		case 2:
			m_maincpu->space(AS_PROGRAM).unmap_read(0x8000, 0xbfff);
			m_maincpu->space(AS_PROGRAM).unmap_write(0x8000, 0xbfff);
			break;
	}

	// page 3
	slot_select = (m_pzlestar_mem_bank >> 6) & 0x03;
	switch(slot_select)
	{
		case 0:
			m_maincpu->space(AS_PROGRAM).install_read_bank(0xc000, 0xffff, "bank4");
			m_maincpu->space(AS_PROGRAM).install_write_bank(0xc000, 0xffff, "bank8");
			membank("bank4")->set_base(m_ram + 0xc000);
			membank("bank8")->set_base(m_ram + 0xc000);
			break;
		case 1:
		case 2:
		case 3:
			m_maincpu->space(AS_PROGRAM).unmap_read(0xc000, 0xffff);
			m_maincpu->space(AS_PROGRAM).unmap_write(0xc000, 0xffff);
			break;
	}

	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xffff, 0xffff, read8_delegate(FUNC(sangho_state::sec_slot_r),this), write8_delegate(FUNC(sangho_state::sec_slot_w),this));
}

WRITE8_MEMBER(sangho_state::pzlestar_bank_w)
{
	logerror("rom bank %02x\n", data);
	m_pzlestar_rom_bank = data;
	pzlestar_map_banks();
}

WRITE8_MEMBER(sangho_state::pzlestar_mem_bank_w)
{
	logerror("mem bank %02x\n", data);
	m_pzlestar_mem_bank = data;
	pzlestar_map_banks();
}

READ8_MEMBER(sangho_state::pzlestar_mem_bank_r)
{
	return m_pzlestar_mem_bank;
}

void sangho_state::sexyboom_map_bank(int bank)
{
	UINT8 banknum, banktype;
	char read_bank_name[6], write_bank_name[6];

	banknum = m_sexyboom_bank[bank*2];
	banktype = m_sexyboom_bank[bank*2 + 1];
	sprintf(read_bank_name, "bank%d", bank+1);
	sprintf(write_bank_name, "bank%d", bank+1+4);

	if (banktype == 0)
	{
		if (banknum & 0x80)
		{
			// ram
			membank(read_bank_name)->set_base(&m_ram[(banknum & 0x7f) * 0x4000]);
			m_maincpu->space(AS_PROGRAM).install_write_bank(bank*0x4000, (bank+1)*0x4000 - 1, write_bank_name );
			membank(write_bank_name)->set_base(&m_ram[(banknum & 0x7f) * 0x4000]);
		}
		else
		{
			// rom 0
			membank(read_bank_name)->set_base(memregion("user1")->base()+0x4000*banknum);
			m_maincpu->space(AS_PROGRAM).unmap_write(bank*0x4000, (bank+1)*0x4000 - 1);
		}
	}
	else if (banktype == 0x82)
	{
		membank(read_bank_name)->set_base(memregion("user1")->base()+0x20000+banknum*0x4000);
		m_maincpu->space(AS_PROGRAM).unmap_write(bank*0x4000, (bank+1)*0x4000 - 1);
	}
	else if (banktype == 0x80)
	{
		membank(read_bank_name)->set_base(memregion("user1")->base()+0x120000+banknum*0x4000);
		m_maincpu->space(AS_PROGRAM).unmap_write(bank*0x4000, (bank+1)*0x4000 - 1);
	}
	else
	{
		logerror("Unknown bank type %02x\n", banktype);
	}
}

WRITE8_MEMBER(sangho_state::sexyboom_bank_w)
{
	m_sexyboom_bank[offset] = data;
	sexyboom_map_bank(offset>>1);
}

/* secondary slot R/Ws from current primary slot number (see also mess/machine/msx.c) */
READ8_MEMBER(sangho_state::sec_slot_r)
{
	return m_sec_slot[m_pzlestar_mem_bank >> 6] ^ 0xff;
}

WRITE8_MEMBER(sangho_state::sec_slot_w)
{
	m_sec_slot[m_pzlestar_mem_bank >> 6] = data;
}


static ADDRESS_MAP_START( sangho_map, AS_PROGRAM, 8, sangho_state )
	AM_RANGE(0x0000, 0x3fff) AM_READ_BANK("bank1") AM_WRITE_BANK("bank5")
	AM_RANGE(0x4000, 0x7fff) AM_READ_BANK("bank2") AM_WRITE_BANK("bank6")
	AM_RANGE(0x8000, 0xbfff) AM_READ_BANK("bank3") AM_WRITE_BANK("bank7")
	AM_RANGE(0xc000, 0xffff) AM_READ_BANK("bank4") AM_WRITE_BANK("bank8")
ADDRESS_MAP_END

/* Puzzle Star Ports */

static ADDRESS_MAP_START( pzlestar_io_map, AS_IO, 8, sangho_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x7c, 0x7d) AM_DEVWRITE("ymsnd", ym2413_device, write)
	AM_RANGE( 0x91, 0x91) AM_WRITE(pzlestar_bank_w )
	AM_RANGE( 0x98, 0x9b) AM_DEVREADWRITE("v9958", v9958_device, read, write )
	AM_RANGE( 0xa0, 0xa0) AM_READ_PORT("P1")
	AM_RANGE( 0xa1, 0xa1) AM_READ_PORT("P2")
	AM_RANGE( 0xa8, 0xa8) AM_READWRITE(pzlestar_mem_bank_r, pzlestar_mem_bank_w )
	AM_RANGE( 0xf7, 0xf7) AM_READ_PORT("DSW")
ADDRESS_MAP_END

/* Sexy Boom Ports */

static ADDRESS_MAP_START( sexyboom_io_map, AS_IO, 8, sangho_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE( 0x7c, 0x7d) AM_DEVWRITE("ymsnd", ym2413_device, write)
	AM_RANGE( 0xa0, 0xa0) AM_READ_PORT("P1")
	AM_RANGE( 0xa1, 0xa1) AM_READ_PORT("P2")
	AM_RANGE( 0xf0, 0xf3) AM_DEVREADWRITE("v9958", v9958_device, read, write )
	AM_RANGE( 0xf7, 0xf7) AM_READ_PORT("DSW")
	AM_RANGE( 0xf8, 0xff) AM_WRITE(sexyboom_bank_w )
ADDRESS_MAP_END

static INPUT_PORTS_START( sexyboom )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(    0x03, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x05, "Easiest (duplicate)" )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, "Easy (duplicate)" )
	PORT_DIPSETTING(    0x07, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x06, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:5,4") /* Determined by effect, but matches Puzzle Star's manual listings */
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:3" )   /* Not shown in manual */
	PORT_DIPNAME( 0x40, 0x00, "Display Numbers on Tiles" )  PORT_DIPLOCATION("SW1:2") /* As per manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static INPUT_PORTS_START( pzlestar )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) /* Start buttons don't work for Puzzle Star... not correct? */
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 ) /* Start buttons don't work for Puzzle Star... not correct? */
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:8,7") /* Will need verification, as other dips don't match manual */
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:6,5") /* Shown as SW1:4 & SW1:5 in manual */
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )        /* Actual effect on game, manual shows 1C / 2C */
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )        /* Actual effect on game, manual shows 3C / 1C */
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )        /* Actual effect on game & manual are the same */
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )        /* Actual effect on game, manual shows 2C / 1C */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4") /* Not shown in manual */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:3") /* Not shown in manual */
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Display Numbers on Tiles" )  PORT_DIPLOCATION("SW1:2") /* As per manual */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )      /* Dipswitch 1:1 Not shown in manual */
INPUT_PORTS_END

void sangho_state::machine_start()
{
	m_ram = auto_alloc_array(machine(), UINT8, 0x20000); // TODO: define how much RAM these ones have (MSX2+ can potentially go up to 4MB)
}

MACHINE_RESET_MEMBER(sangho_state,pzlestar)
{
	m_pzlestar_mem_bank = 2;
	pzlestar_map_banks();
}

MACHINE_RESET_MEMBER(sangho_state,sexyboom)
{
	m_sexyboom_bank[0] = 0x00;
	m_sexyboom_bank[1] = 0x00;
	m_sexyboom_bank[2] = 0x01;
	m_sexyboom_bank[3] = 0x00;
	m_sexyboom_bank[4] = 0x80;
	m_sexyboom_bank[5] = 0x00;
	m_sexyboom_bank[6] = 0x80;
	m_sexyboom_bank[7] = 0x01;
	sexyboom_map_bank(0);
	sexyboom_map_bank(1);
	sexyboom_map_bank(2);
	sexyboom_map_bank(3);
}

WRITE_LINE_MEMBER(sangho_state::msx_vdp_interrupt)
{
	m_maincpu->set_input_line(0, (state ? HOLD_LINE : CLEAR_LINE));
}

TIMER_DEVICE_CALLBACK_MEMBER(sangho_state::sangho_interrupt)
{
	int scanline = param;

	if((scanline % 2) == 0)
	{
		m_v9958->interrupt();
	}
}


static MACHINE_CONFIG_START( pzlestar, sangho_state )

	MCFG_CPU_ADD("maincpu", Z80,8000000) // ?
	MCFG_CPU_PROGRAM_MAP(sangho_map)
	MCFG_CPU_IO_MAP(pzlestar_io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", sangho_state, sangho_interrupt, "screen", 0, 1)

	MCFG_V9958_ADD("v9958", "screen", 0x20000)
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(sangho_state,msx_vdp_interrupt))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DEVICE("v9958", v9958_device, screen_update)
	MCFG_SCREEN_SIZE(512 + 32, (212 + 28) * 2)
	MCFG_SCREEN_VISIBLE_AREA(0, 512 + 32 - 1, 0, (212 + 28) * 2 - 1)
	MCFG_SCREEN_PALETTE("v9958:palette")

	MCFG_MACHINE_RESET_OVERRIDE(sangho_state,pzlestar)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, 3580000)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( sexyboom, sangho_state )

	MCFG_CPU_ADD("maincpu", Z80,8000000) // ?
	MCFG_CPU_PROGRAM_MAP(sangho_map)
	MCFG_CPU_IO_MAP(sexyboom_io_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", sangho_state, sangho_interrupt, "screen", 0, 1)

	MCFG_V9958_ADD("v9958", "screen", 0x20000)
	MCFG_V99X8_INTERRUPT_CALLBACK(WRITELINE(sangho_state,msx_vdp_interrupt))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DEVICE("v9958", v9958_device, screen_update)
	MCFG_SCREEN_SIZE(512 + 32, (212 + 28) * 2)
	MCFG_SCREEN_VISIBLE_AREA(0, 512 + 32 - 1, 0, (212 + 28) * 2 - 1)
	MCFG_SCREEN_PALETTE("v9958:palette")

	MCFG_MACHINE_RESET_OVERRIDE(sangho_state,sexyboom)

	MCFG_PALETTE_ADD("palette", 19780)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM2413, 3580000)

	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( pzlestar )
	ROM_REGION( 0x20000*16, "user1", 0 ) // 15 sockets, 13 used
	ROM_LOAD( "rom01.bin", 0x000000, 0x20000, CRC(0b327a3b) SHA1(450fd27f9844b9f0b710c1886985bd67cac2716f) ) // seems to be some code at 0x10000
	ROM_LOAD( "rom02.bin", 0x020000, 0x20000, CRC(dc859437) SHA1(e9fe5aab48d80e8857fc679ff7e35298ce4d47c8) )
	ROM_LOAD( "rom03.bin", 0x040000, 0x20000, CRC(f92b5624) SHA1(80be9000fc4326d790801d02959550aada111548) )
	ROM_LOAD( "rom04.bin", 0x060000, 0x20000, CRC(929e7491) SHA1(fb700d3e1d50fefa9b85ccd3702a9854df53a210) )
	ROM_LOAD( "rom05.bin", 0x080000, 0x20000, CRC(8c6f71e5) SHA1(3597b03fe61216256437c56c583d55c7d59b5525) )
	ROM_LOAD( "rom06.bin", 0x0a0000, 0x20000, CRC(84599227) SHA1(d47c6cdbf3b64f83627c768059148e31f8de1f36) )
	ROM_LOAD( "rom08.bin", 0x0c0000, 0x20000, CRC(18d2bfe2) SHA1(cb92ee51d061bc053e296fcba10708f69ba12a61) )
	ROM_LOAD( "rom07.bin", 0x0e0000, 0x20000, CRC(6f64cc35) SHA1(3e3270834ad31e8240748c2b61f9b8f138d22f68) )
	ROM_LOAD( "rom09.bin", 0x100000, 0x20000, CRC(19a31115) SHA1(fa6ead5c8bf6be21d07797f74fcba13f0d041937) )
	ROM_LOAD( "rom10.bin", 0x120000, 0x20000, CRC(c003328b) SHA1(5172e2c48e118ac9f9b9dd4f4df8804245047b33) )
	ROM_LOAD( "rom11.bin", 0x140000, 0x20000, CRC(d36c1f92) SHA1(42b412c1ab99cb14f2e15bd80fede34c0df414b9) )
	ROM_LOAD( "rom12.bin", 0x160000, 0x20000, CRC(baa82727) SHA1(ed3dd1befa615003204f903472ef1af1eb702c38) )
	ROM_LOAD( "rom13.bin", 0x180000, 0x20000, CRC(8b4b6a2c) SHA1(4b9c188260617ccce94cbf6cccb45aab455af09b) )
	/* 14 empty */
	/* 15 empty */
ROM_END

ROM_START( sexyboom )
	ROM_REGION( 0x20000*16, "user1", 0 ) // 15 sockets, 12 used
	ROM_LOAD( "rom1.bin",  0x000000, 0x20000, CRC(7827a079) SHA1(a48e7c7d16e50de24c8bf77883230075c1fad858) )
	ROM_LOAD( "rom2.bin",  0x020000, 0x20000, CRC(7028aa61) SHA1(77d5e5945b90d3e15ba2c1364b76f6643247592d) )
	ROM_LOAD( "rom3.bin",  0x040000, 0x20000, CRC(340edac9) SHA1(47ffc4553cb34ff932d3d54d5cefe82571c6ddbf) )
	ROM_LOAD( "rom4.bin",  0x060000, 0x20000, CRC(25f76d7f) SHA1(caff03ba4ca9ad44e488618141c4e1f43a0cdc48) )
	ROM_LOAD( "rom5.bin",  0x080000, 0x20000, CRC(3a3dda85) SHA1(b174cf87be5dd7a7430ff29c70c8093c577f4033) )
	ROM_LOAD( "rom6.bin",  0x0a0000, 0x20000, CRC(d0428a82) SHA1(4409c2ebd2f70828286769c9367cbccac483cdaf) )
	ROM_LOAD( "rom7.bin",  0x0c0000, 0x20000, CRC(2d2e4df2) SHA1(8ec36c8c021c2b9d9be7b61e09e31a7a18a06dad) )
	ROM_LOAD( "rom8.bin",  0x0e0000, 0x20000, CRC(283ba870) SHA1(98f35d95caf58595f002d57a4bafc39b6d67ed1a) )
	ROM_LOAD( "rom9.bin",  0x100000, 0x20000, CRC(a78310f4) SHA1(7a14cabd371d6ba4e335f0e00135de3dd8a4e642) )
	ROM_LOAD( "rom10.bin", 0x120000, 0x20000, CRC(b20fabd2) SHA1(a6a3bac1ac19e1ecd2fd0aeb17fbf16ffa07df13) )
	ROM_LOAD( "rom11.bin", 0x140000, 0x20000, CRC(e4aa16bc) SHA1(c5889b813ceb7a1c0deb8a9d4d006932b266a482) )
	ROM_LOAD( "rom12.bin", 0x160000, 0x20000, CRC(cd8b6b5d) SHA1(ffddc7781e13146e198ad12a9c89504f270857d8) )
	/* 13 empty */
	/* 14 empty */
	/* 15 empty */
ROM_END

DRIVER_INIT_MEMBER(sangho_state,pzlestar)
{
	UINT8 *ROM = memregion("user1")->base();

	/* patch nasty looping check, related to sound? */
	ROM[0x12ca7] = 0x00;
	ROM[0x12ca8] = 0x00;
}

GAME( 1991, pzlestar,  0,    pzlestar, pzlestar, sangho_state,  pzlestar,   ROT270, "Sang Ho Soft", "Puzzle Star (Sang Ho Soft)", GAME_IMPERFECT_COLORS | GAME_IMPERFECT_SOUND )
GAME( 1992, sexyboom,  0,    sexyboom, sexyboom, driver_device, 0,          ROT270, "Sang Ho Soft", "Sexy Boom", 0 )
