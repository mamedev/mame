// license:BSD-3-Clause
// copyright-holders:MetalliC
/*************************************************************************

    Istrebiteli preliminary driver by MetalliC

**************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"

#define I8080_TAG   "maincpu"

class istrebiteli_state : public driver_device
{
public:
	istrebiteli_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, I8080_TAG),
		m_ppi0(*this, "ppi0"),
		m_ppi1(*this, "ppi1")
	{
	}

	DECLARE_READ8_MEMBER(ppi0_r);
	DECLARE_WRITE8_MEMBER(ppi0_w);
	DECLARE_READ8_MEMBER(ppi1_r);
	DECLARE_WRITE8_MEMBER(ppi1_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(spr0_ctrl_w);
	DECLARE_WRITE8_MEMBER(spr1_ctrl_w);
	DECLARE_WRITE8_MEMBER(spr_xy_w);

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;

	virtual void machine_start() override { }
	virtual void machine_reset() override { }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	UINT8 m_spr0_ctrl;
	UINT8 m_spr1_ctrl;
	UINT8 m_spr_xy[8];
};

READ8_MEMBER(istrebiteli_state::ppi0_r)
{
	return m_ppi0->read(space, offset ^ 3);
}
WRITE8_MEMBER(istrebiteli_state::ppi0_w)
{
	m_ppi0->write(space, offset ^ 3, data);
}
READ8_MEMBER(istrebiteli_state::ppi1_r)
{
	return m_ppi1->read(space, offset ^ 3);
}
WRITE8_MEMBER(istrebiteli_state::ppi1_w)
{
	m_ppi1->write(space, offset ^ 3, data);
}

WRITE8_MEMBER(istrebiteli_state::sound_w)
{
	// TODO
}

WRITE8_MEMBER(istrebiteli_state::spr0_ctrl_w)
{
	m_spr0_ctrl = data;
}

WRITE8_MEMBER(istrebiteli_state::spr1_ctrl_w)
{
	m_spr1_ctrl = data;
}

WRITE8_MEMBER(istrebiteli_state::spr_xy_w)
{
	m_spr_xy[offset] = data;
}

UINT32 istrebiteli_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, istrebiteli_state)
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0x13ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 8, istrebiteli_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xb0, 0xbf) AM_NOP //AM_WRITE(charram_w)
	AM_RANGE(0xc0, 0xc3) AM_READWRITE(ppi0_r, ppi0_w)
	AM_RANGE(0xc4, 0xc7) AM_READWRITE(ppi1_r, ppi1_w)
	AM_RANGE(0xc8, 0xcf) AM_WRITE(spr_xy_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( istreb )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // sprite collision flag ?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // sprite collision flag ?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNKNOWN )  // TODO read coin counter
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_HBLANK("screen")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
INPUT_PORTS_END

static MACHINE_CONFIG_START( istreb, istrebiteli_state)
	/* basic machine hardware */
	MCFG_CPU_ADD(I8080_TAG, I8080, 8000000 / 4)		// KR580VM80A
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	MCFG_DEVICE_ADD("ppi0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(istrebiteli_state, sound_w))

	MCFG_DEVICE_ADD("ppi1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(istrebiteli_state, spr0_ctrl_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(istrebiteli_state, spr1_ctrl_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(4000000, 256, 0, 256, 312, 0, 256)
	MCFG_SCREEN_UPDATE_DRIVER(istrebiteli_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 4)
MACHINE_CONFIG_END

ROM_START( istreb )
	ROM_REGION( 0x1000, I8080_TAG, ROMREGION_ERASEFF )
	ROM_LOAD( "main.bin", 0x000, 0xa00, CRC(ae67c41c) SHA1(1f7807d486cd4d161ee49be991b81db7dc9b0f3b) )

	ROM_REGION(0x400, "chars", 0 )
	ROM_LOAD("003-g8.bin", 0x000, 0x200, CRC(5cd7ad47) SHA1(2142711c8a3640b7aa258a2059cfb0f14297a5ac) )
	ROM_LOAD("003-w3.bin", 0x200, 0x200, CRC(54eb4893) SHA1(c7a4724045c645ab728074ed7fef1882d9776005) )

	ROM_REGION(0x1000, "sprite", 0)
	ROM_LOAD("001-a11.bin", 0x000, 0x100, CRC(4e05b7dd) SHA1(335e975ae9e8f775c1ac07f60420680ad878c3ae) )
	ROM_LOAD("001-b11.bin", 0x100, 0x100, CRC(4e05b7dd) SHA1(335e975ae9e8f775c1ac07f60420680ad878c3ae) )
	ROM_LOAD("001-g4.bin",  0x200, 0x200, CRC(ca3c531b) SHA1(8295167895d51e626b6d5946b565d5e8b8466ac0) )
	ROM_LOAD("001-g9.bin",  0x400, 0x200, CRC(ca3c531b) SHA1(8295167895d51e626b6d5946b565d5e8b8466ac0) )
ROM_END

/*    YEAR  NAME      PARENT    COMPAT    MACHINE   INPUT     INIT               COMPANY       FULLNAME            FLAGS */
CONS( 198?, istreb,  0,        0,        istreb,  istreb,  driver_device,  0, "<unknown>", "Istrebiteli", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
