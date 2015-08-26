// license:BSD-3-Clause
// copyright-holders:David Haywood, Stephh
/* Xyonix *********************************************************************

driver by David Haywood and Stephh

Notes about the board:

Ram is 2x 6264 (near Z80) and 1x 6264 near UM6845. Xtal is verified 16.000MHz,
I can also see another special chip . PHILKO PK8801. chip looks about the same as a
TMS3615 (though i have no idea what the chip actually is). its located next to the
prom, the 2x 256k roms, and the 1x 6264 ram.
Dip SW is 1 x 8-position

on the PCB is an empty socket. written next to the socket is 68705P3. "oh no" you
say..... well, its unpopulated, so maybe it was never used? (another PCB was
found with the 68705 populated)


TODO:
- there are some more unknown commands for the I/O chip

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "includes/xyonix.h"


void xyonix_state::machine_start()
{
	save_item(NAME(m_e0_data));
	save_item(NAME(m_credits));
	save_item(NAME(m_coins));
	save_item(NAME(m_prev_coin));
}

WRITE8_MEMBER(xyonix_state::irqack_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


/* Inputs ********************************************************************/

void xyonix_state::handle_coins(int coin)
{
	static const int coinage_table[4][2] = {{2,3},{2,1},{1,2},{1,1}};
	int tmp = 0;

	//popmessage("Coin %d", m_coin);

	if (coin & 1)   // Coin 2 !
	{
		tmp = (ioport("DSW")->read() & 0xc0) >> 6;
		m_coins++;
		if (m_coins >= coinage_table[tmp][0])
		{
			m_credits += coinage_table[tmp][1];
			m_coins -= coinage_table[tmp][0];
		}
		coin_lockout_global_w(machine(), 0); /* Unlock all coin slots */
		coin_counter_w(machine(),1,1); coin_counter_w(machine(),1,0); /* Count slot B */
	}

	if (coin & 2)   // Coin 1 !
	{
		tmp = (ioport("DSW")->read() & 0x30) >> 4;
		m_coins++;
		if (m_coins >= coinage_table[tmp][0])
		{
			m_credits += coinage_table[tmp][1];
			m_coins -= coinage_table[tmp][0];
		}
		coin_lockout_global_w(machine(), 0); /* Unlock all coin slots */
		coin_counter_w(machine(),0,1); coin_counter_w(machine(),0,0); /* Count slot A */
	}

	if (m_credits >= 9)
		m_credits = 9;
}


READ8_MEMBER(xyonix_state::io_r)
{
	int regPC = space.device().safe_pc();

	if (regPC == 0x27ba)
		return 0x88;

	if (regPC == 0x27c2)
		return m_e0_data;

	if (regPC == 0x27c7)
	{
		int coin;

		switch (m_e0_data)
		{
			case 0x81 :
				return ioport("P1")->read() & 0x7f;
			case 0x82 :
				return ioport("P2")->read() & 0x7f;
			case 0x91:
				/* check coin inputs */
				coin = ((ioport("P1")->read() & 0x80) >> 7) | ((ioport("P2")->read() & 0x80) >> 6);
				if (coin ^ m_prev_coin && coin != 3)
				{
					if (m_credits < 9) handle_coins(coin);
				}
				m_prev_coin = coin;
				return m_credits;
			case 0x92:
				return ((ioport("P1")->read() & 0x80) >> 7) | ((ioport("P2")->read() & 0x80) >> 6);
			case 0xe0:  /* reset? */
				m_coins = 0;
				m_credits = 0;
				return 0xff;
			case 0xe1:
				m_credits--;
				return 0xff;
			case 0xfe:  /* Dip Switches 1 to 4 */
				return ioport("DSW")->read() & 0x0f;
			case 0xff:  /* Dip Switches 5 to 8 */
				return ioport("DSW")->read() >> 4;
		}
	}

	//logerror ("xyonix_port_e0_r - PC = %04x - port = %02x\n", regPC, m_e0_data);
	//popmessage("%02x",m_e0_data);

	return 0xff;
}

WRITE8_MEMBER(xyonix_state::io_w)
{
	//logerror ("xyonix_port_e0_w %02x - PC = %04x\n", data, space.device().safe_pc());
	m_e0_data = data;
}

/* Mem / Port Maps ***********************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, xyonix_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM_WRITE(vidram_w) AM_SHARE("vidram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( port_map, AS_IO, 8, xyonix_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x20) AM_READNOP AM_DEVWRITE("sn1", sn76496_device, write)   /* SN76496 ready signal */
	AM_RANGE(0x21, 0x21) AM_READNOP AM_DEVWRITE("sn2", sn76496_device, write)
	AM_RANGE(0x40, 0x40) AM_WRITENOP        /* NMI ack? */
	AM_RANGE(0x50, 0x50) AM_WRITE(irqack_w)
	AM_RANGE(0x60, 0x61) AM_WRITENOP        /* mc6845 */
	AM_RANGE(0xe0, 0xe0) AM_READWRITE(io_r, io_w)
ADDRESS_MAP_END

/* Inputs Ports **************************************************************/

static INPUT_PORTS_START( xyonix )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )      /* handled by xyonix_io_r() */

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )      /* handled by xyonix_io_r() */

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )          // DEF_STR( Very_Hard )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
INPUT_PORTS_END

/* GFX Decode ****************************************************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	4*16
};

static GFXDECODE_START( xyonix )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END


/* MACHINE driver *************************************************************/

static MACHINE_CONFIG_START( xyonix, xyonix_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80,16000000 / 4)        /* 4 MHz ? */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_IO_MAP(port_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", xyonix_state,  nmi_line_pulse)
	MCFG_CPU_PERIODIC_INT_DRIVER(xyonix_state, irq0_line_assert, 4*60)  /* ?? controls music tempo */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(80*4, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 80*4-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(xyonix_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", xyonix)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(xyonix_state, xyonix)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("sn1", SN76496, 16000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("sn2", SN76496, 16000000/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/* ROM Loading ***************************************************************/

ROM_START( xyonix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "xyonix3.bin", 0x00000, 0x10000, CRC(1960a74e) SHA1(5fd7bc31ca2f5f1e114d3d0ccf6554ebd712cbd3) )

	ROM_REGION( 0x10000, "mcu", 0 )
	ROM_LOAD( "mc68705p3s.e7", 0x00000, 0x780, BAD_DUMP CRC(f60cdd86) SHA1(e18cc598153b3e108942328ee9c5b9f83b034c41) ) // FIXED BITS (xxxxxx0x)

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "xyonix1.bin", 0x00000, 0x08000, CRC(3dfa9596) SHA1(52cdbbe18f83cea7248c29588ea3a18c4bb7984f) )
	ROM_LOAD( "xyonix2.bin", 0x08000, 0x08000, CRC(db87343e) SHA1(62bc30cd65b2f8976cd73a0b349a9ccdb3faaad2) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "xyonix.pr",   0x0000, 0x0100, CRC(0012cfc9) SHA1(c7454107a1a8083a370b662c617117b769c0dc1c) )
ROM_END

/* GAME drivers **************************************************************/

GAME( 1989, xyonix, 0, xyonix, xyonix, driver_device, 0, ROT0, "Philko", "Xyonix", MACHINE_SUPPORTS_SAVE )
