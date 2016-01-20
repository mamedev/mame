// license:BSD-3-Clause
// copyright-holders:David Haywood
/* paint & puzzle */
/* video is standard VGA */
/*
OK, here's a somewhat complete rundown of the PCB. There was 1 IC I didn't
get a pin count of(oops).

Main PCB
Reb B
Label:
Board Num.
90

The PCB has no silkscreen or reference designators, so the numbers I am
providing are made up.

U1 32 pin IC
27C010A
Label:
Paint N Puzzle
Ver. 1.09
Odd

U2 5 pin audio amp
LM383T

U3 40 pin IC
8926S
UM6522A

U4 28 pin IC
Mosel
MS62256L-85PC
8911 5D

U5 18 pin IC
PIC16C54-HS/P
9344 CGA

U6
P8798
L3372718E
Intel
Label:
MicroTouch
5603670 REV 1.0

U7 28 pin IC
MicroTouch Systems
c 1992
19-507 Rev 2
ICS1578N 9334

U8 28 pin IC
Mosel
MS62256L-85PC
8911 5D

U9 32 pin IC
27C010A
Label:
Paint N Puzzle
Ver. 1.09
Even

U10 64 pin IC
MC68000P12
OB26M8829

X1
16.000MHz -connected to U5

X2
ECS-120
32-1
China -connected to U7
Other side is unreadable

CN1 JAMMA
CN2 ISA? Video card slot
CN3 Touchscreen input (12 pins)
CN4 2 pins, unused

1 blue potentiometer connected to audio amp
There doesnt seem to be any dedicated sound chip, and sounds are just bleeps
really.

-----------------------------------------------
Video card (has proper silk screen and designators)
JAX-8327A

X1
40.000MHz

J1 -open
J2 -closed
J3 -open

U1/2 unpopulated but have sockets

U3 20 pin IC
KM44C256BT-8
22BY
Korea

U4 20 pin IC
KM44C256BT-8
22BY
Korea

U5 160 pin QFP
Trident
TVGA9000i
34'3 Japan

U6 28 pin IC
Quadtel
TVGA9000i BIOS Software
c 1993 Ver D3.51 LH

CN1 standard DB15 VGA connector (15KHz)
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "video/pc_vga.h"


class pntnpuzl_state : public driver_device
{
public:
	pntnpuzl_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_eeprom(*this, "eeprom")
		{ }

	UINT16 m_eeprom_data;
	UINT16 m_pntpzl_200000;
	UINT16 m_serial;
	UINT16 m_serial_out;
	UINT16 m_read_count;
	int m_touchscr[5];

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE16_MEMBER(pntnpuzl_200000_w);
	DECLARE_WRITE16_MEMBER(pntnpuzl_280018_w);
	DECLARE_READ16_MEMBER(pntnpuzl_280014_r);
	DECLARE_READ16_MEMBER(pntnpuzl_28001a_r);
	DECLARE_READ16_MEMBER(irq1_ack_r);
	DECLARE_READ16_MEMBER(irq2_ack_r);
	DECLARE_READ16_MEMBER(irq4_ack_r);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_READ16_MEMBER(pntnpuzl_eeprom_r);
	DECLARE_WRITE16_MEMBER(pntnpuzl_eeprom_w);
	DECLARE_DRIVER_INIT(pip);
	required_device<eeprom_serial_93cxx_device> m_eeprom;
};


READ16_MEMBER(pntnpuzl_state::pntnpuzl_eeprom_r)
{
	/* bit 11 is EEPROM data */
	return (m_eeprom_data & 0xf4ff) | (m_eeprom->do_read()<<11) | (ioport("IN1")->read() & 0x0300);
}

WRITE16_MEMBER(pntnpuzl_state::pntnpuzl_eeprom_w)
{
	m_eeprom_data = data;

	/* bit 12 is data */
	/* bit 13 is clock (active high) */
	/* bit 14 is cs (active high) */

	m_eeprom->di_write((data & 0x1000) >> 12);
	m_eeprom->cs_write((data & 0x4000) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write((data & 0x2000) ? ASSERT_LINE : CLEAR_LINE);
}



/*
reading works this way:
280016 = 00
28001A = 08
wait for bit 3 of 28001A to be 1 (after a timeout, fail)
280010 = 3d
280012 = 00
280016 = 04
read 280014 (throw away result)
wait for bit 2 of 28001A to be 1
read data from 280014

during startup it expects this series:
write                                     read
01 52 0d <pause> 01 50 4e 38 31 0d   ---> 80 0c
01 4d 51 0d                          ---> 80 0c
01 46 54 0d                          ---> 80 0c
01 46 4e 30 38 0d                    ---> 80 0c
01 53 45 32 0d                       ---> 80 0c
01 03 46 31 38 0d                    ---> 80 0c
*/

WRITE16_MEMBER(pntnpuzl_state::pntnpuzl_200000_w)
{
// logerror("200000: %04x\n",data);
	// bit 12: set to 1 when going to serial output to 280018
	if ((m_pntpzl_200000 & 0x1000) && !(data & 0x1000))
	{
		m_serial_out = (m_serial>>1) & 0xff;
		m_read_count = 0;
		logerror("serial out: %02x\n",m_serial_out);
	}

	m_pntpzl_200000 = data;
}

WRITE16_MEMBER(pntnpuzl_state::pntnpuzl_280018_w)
{
// logerror("%04x: 280018: %04x\n",space.device().safe_pc(),data);
	m_serial >>= 1;
	if (data & 0x2000)
		m_serial |= 0x400;
}

READ16_MEMBER(pntnpuzl_state::pntnpuzl_280014_r)
{
	static const int startup[3] = { 0x80, 0x0c, 0x00 };
	int res;

	if (m_serial_out == 0x11)
	{
		if (ioport("IN0")->read() & 0x10)
		{
			m_touchscr[0] = 0x1b;
			m_touchscr[2] = BITSWAP8(ioport("TOUCHX")->read(),0,1,2,3,4,5,6,7);
			m_touchscr[4] = BITSWAP8(ioport("TOUCHY")->read(),0,1,2,3,4,5,6,7);
		}
		else
			m_touchscr[0] = 0;

		if (m_read_count >= 10) m_read_count = 0;
		res = m_touchscr[m_read_count/2];
		m_read_count++;
	}
	else
	{
		if (m_read_count >= 6) m_read_count = 0;
		res = startup[m_read_count/2];
		m_read_count++;
	}
	logerror("read 280014: %02x\n",res);
	return res << 8;
}

READ16_MEMBER(pntnpuzl_state::pntnpuzl_28001a_r)
{
	return 0x4c00;
}

READ16_MEMBER(pntnpuzl_state::irq1_ack_r)
{
//  m_maincpu->set_input_line(1, CLEAR_LINE);
	return 0;
}

READ16_MEMBER(pntnpuzl_state::irq2_ack_r)
{
//  m_maincpu->set_input_line(2, CLEAR_LINE);
	return 0;
}

READ16_MEMBER(pntnpuzl_state::irq4_ack_r)
{
//  m_maincpu->set_input_line(4, CLEAR_LINE);
	return 0;
}


static ADDRESS_MAP_START( pntnpuzl_map, AS_PROGRAM, 16, pntnpuzl_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x080001) AM_READ(irq1_ack_r)
	AM_RANGE(0x100000, 0x100001) AM_READ(irq2_ack_r)
	AM_RANGE(0x180000, 0x180001) AM_READ(irq4_ack_r)
	AM_RANGE(0x200000, 0x200001) AM_WRITE(pntnpuzl_200000_w)
	AM_RANGE(0x280000, 0x280001) AM_READ(pntnpuzl_eeprom_r)
	AM_RANGE(0x280002, 0x280003) AM_READ_PORT("IN2")
	AM_RANGE(0x280000, 0x280001) AM_WRITE(pntnpuzl_eeprom_w)
	AM_RANGE(0x280008, 0x280009) AM_WRITENOP
	AM_RANGE(0x28000a, 0x28000b) AM_WRITENOP
	AM_RANGE(0x280010, 0x280011) AM_WRITENOP
	AM_RANGE(0x280012, 0x280013) AM_WRITENOP
	AM_RANGE(0x280014, 0x280015) AM_READ(pntnpuzl_280014_r)
	AM_RANGE(0x280016, 0x280017) AM_WRITENOP
	AM_RANGE(0x280018, 0x280019) AM_WRITE(pntnpuzl_280018_w)
	AM_RANGE(0x28001a, 0x28001b) AM_READ(pntnpuzl_28001a_r)
	AM_RANGE(0x28001a, 0x28001b) AM_WRITENOP

	/* standard VGA */
	AM_RANGE(0x3a0000, 0x3bffff) AM_DEVREADWRITE8("vga", vga_device, mem_r, mem_w, 0xffff)
	AM_RANGE(0x3c03b0, 0x3c03bf) AM_DEVREADWRITE8("vga", vga_device, port_03b0_r, port_03b0_w, 0xffff)
	AM_RANGE(0x3c03c0, 0x3c03cf) AM_DEVREADWRITE8("vga", vga_device, port_03c0_r, port_03c0_w, 0xffff)
	AM_RANGE(0x3c03d0, 0x3c03df) AM_DEVREADWRITE8("vga", vga_device, port_03d0_r, port_03d0_w, 0xffff)

	AM_RANGE(0x400000, 0x407fff) AM_RAM
ADDRESS_MAP_END


INPUT_CHANGED_MEMBER(pntnpuzl_state::coin_inserted)
{
	/* TODO: change this! */
	if(newval)
		generic_pulse_irq_line(m_maincpu, (UINT8)(FPTR)param, 1);
}

static INPUT_PORTS_START( pntnpuzl )
	PORT_START("IN0")   /* fake inputs */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, pntnpuzl_state,coin_inserted, 1) PORT_IMPULSE(1)
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_HIGH )PORT_CHANGED_MEMBER(DEVICE_SELF, pntnpuzl_state,coin_inserted, 2) PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, pntnpuzl_state,coin_inserted, 4) PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	/* game uses a touch screen */
	PORT_START("TOUCHX")
	PORT_BIT( 0x7f, 0x40, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,0x7f) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCHY")
	PORT_BIT( 0x7f, 0x40, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_MINMAX(0,0x7f) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)

	PORT_START("IN2")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static MACHINE_CONFIG_START( pntnpuzl, pntnpuzl_state )
	MCFG_CPU_ADD("maincpu", M68000, 12000000)//??
	MCFG_CPU_PROGRAM_MAP(pntnpuzl_map)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )
MACHINE_CONFIG_END

ROM_START( pntnpuzl )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "pntnpuzl.u2", 0x00001, 0x40000, CRC(dfda3f73) SHA1(cca8ccdd501a26cba07365b1238d7b434559bbc6) )
	ROM_LOAD16_BYTE( "pntnpuzl.u3", 0x00000, 0x40000, CRC(4173f250) SHA1(516fe6f91b925f71c36b97532608b82e63bda436) )

	/* for reference, probably not used in any way by the game */
	ROM_REGION( 0x10000, "video_bios", 0 )
	ROM_LOAD( "trident_quadtel_tvga9000_isa16.bin", 0x0000, 0x10000, BAD_DUMP CRC(ad0e7351) SHA1(eb525460a80e1c1baa34642b93d54caf2607920d) )
ROM_END


DRIVER_INIT_MEMBER(pntnpuzl_state,pip)
{
//  UINT16 *rom = (UINT16 *)memregion("maincpu")->base();
//  rom[0x2696/2] = 0x4e71;
//  rom[0x26a0/2] = 0x4e71;

}

GAME( 199?, pntnpuzl,    0, pntnpuzl,    pntnpuzl, pntnpuzl_state,    pip, ROT90,  "Century?", "Paint & Puzzle",MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
