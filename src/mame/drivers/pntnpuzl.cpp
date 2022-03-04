// license:BSD-3-Clause
// copyright-holders:David Haywood
/* paint & puzzle */
/* video is standard VGA */
/*
OK, here's a somewhat complete rundown of the PCB.

Main PCB
Reb B
Label:
Board Num.
90

The PCB has no silkscreen or reference designators. All locations
provided here are from the published schematics.

U1 64 pin IC
MC68000P12
OB26M8829

U2 32 pin IC
27C010A
Label:
Paint N Puzzle
Ver. 1.09
Odd

U3 32 pin IC
27C010A
Label:
Paint N Puzzle
Ver. 1.09
Even

U4 & U5 28 pin ICs
Mosel
MS62256L-85PC
8911 5D

U14 28 pin IC
MicroTouch Systems
c 1992
19-507 Rev 2
ICS1578N 9334 (ASIC 1578 on schematic)

U15 48 pin IC
P8798 (8797 on schematic)
L3372718E
Intel
Label:
MicroTouch
5603670 REV 1.0

U17 8 pin IC
National 8538A
93C46N
-connected to U15

U35 18 pin IC
PIC16C54-HS/P
9344 CGA

U36 5 pin audio amp
LM383T

U37 8 pin IC
National 8538A
93C46N
-connected to U41

U41 40 pin IC
8926S
UM6522A

Y1
ECS-120
32-1
China -connected to U14
Other side is unreadable (schematic reads 12.000MHz?)

Y2
16.000MHz -connected to U35

CN1 JAMMA
CN2 ISA? Video card slot
CN3 Touchscreen input (12 pins)
CN4 2 pins, unused

1 blue potentiometer connected to audio amp

There is no dedicated sound generator, and sounds are just bleeps
really. The sounds come from a binary-weighted DAC attached to the
PIC's RB outputs:

R34 7.5K
R33 15.4K
R32 30.9K
R31 61.8K
R30 124K
R29 249K
R28 499K
R27 1M

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
#include "cpu/mcs96/i8x9x.h"
#include "machine/6522via.h"
#include "machine/eepromser.h"
#include "video/pc_vga.h"
#include "screen.h"


class pntnpuzl_state : public driver_device
{
public:
	pntnpuzl_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_via(*this, "via")
	{ }

	void pntnpuzl(machine_config &config);

	void init_pip();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	uint16_t m_pntpzl_200000;
	uint16_t m_serial;
	uint16_t m_serial_out;
	uint16_t m_read_count;
	int m_touchscr[5];

	required_device<cpu_device> m_maincpu;
	void pntnpuzl_200000_w(uint16_t data);
	void pntnpuzl_280018_w(uint16_t data);
	uint16_t pntnpuzl_280014_r();
	uint16_t pntnpuzl_28001a_r();
	uint16_t irq1_ack_r();
	uint16_t irq2_ack_r();
	uint16_t irq4_ack_r();
	required_device<via6522_device> m_via;
	void mcu_map(address_map &map);
	void pntnpuzl_map(address_map &map);
};



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

void pntnpuzl_state::pntnpuzl_200000_w(uint16_t data)
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

void pntnpuzl_state::pntnpuzl_280018_w(uint16_t data)
{
// logerror("%04x: 280018: %04x\n",m_maincpu->pc(),data);
	m_serial >>= 1;
	if (data & 0x2000)
		m_serial |= 0x400;

	m_via->write(0x18/2, data >> 8);
}

uint16_t pntnpuzl_state::pntnpuzl_280014_r()
{
	static const int startup[3] = { 0x80, 0x0c, 0x00 };
	int res;

	(void)m_via->read(0x14/2);

	if (m_serial_out == 0x11)
	{
		if (ioport("IN0")->read() & 0x10)
		{
			m_touchscr[0] = 0x1b;
			m_touchscr[2] = bitswap<8>(ioport("TOUCHX")->read(),0,1,2,3,4,5,6,7);
			m_touchscr[4] = bitswap<8>(ioport("TOUCHY")->read(),0,1,2,3,4,5,6,7);
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

uint16_t pntnpuzl_state::pntnpuzl_28001a_r()
{
	return 0x4c00;
}

uint16_t pntnpuzl_state::irq1_ack_r()
{
//  m_maincpu->set_input_line(1, CLEAR_LINE);
	return 0;
}

uint16_t pntnpuzl_state::irq2_ack_r()
{
//  m_maincpu->set_input_line(2, CLEAR_LINE);
	return 0;
}

uint16_t pntnpuzl_state::irq4_ack_r()
{
//  m_maincpu->set_input_line(4, CLEAR_LINE);
	return 0;
}


void pntnpuzl_state::pntnpuzl_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x080000, 0x080001).r(FUNC(pntnpuzl_state::irq1_ack_r));
	map(0x100000, 0x100001).r(FUNC(pntnpuzl_state::irq2_ack_r));
	map(0x180000, 0x180001).r(FUNC(pntnpuzl_state::irq4_ack_r));
	map(0x200000, 0x200001).w(FUNC(pntnpuzl_state::pntnpuzl_200000_w));
	map(0x280000, 0x28001f).m(m_via, FUNC(via6522_device::map)).umask16(0xff00);
	map(0x280014, 0x280015).r(FUNC(pntnpuzl_state::pntnpuzl_280014_r));
	map(0x280018, 0x280019).w(FUNC(pntnpuzl_state::pntnpuzl_280018_w));
	map(0x28001a, 0x28001b).r(FUNC(pntnpuzl_state::pntnpuzl_28001a_r));

	/* standard VGA */
	map(0x3a0000, 0x3bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
	map(0x3c03b0, 0x3c03bf).rw("vga", FUNC(vga_device::port_03b0_r), FUNC(vga_device::port_03b0_w));
	map(0x3c03c0, 0x3c03cf).rw("vga", FUNC(vga_device::port_03c0_r), FUNC(vga_device::port_03c0_w));
	map(0x3c03d0, 0x3c03df).rw("vga", FUNC(vga_device::port_03d0_r), FUNC(vga_device::port_03d0_w));

	map(0x400000, 0x407fff).ram();
}

void pntnpuzl_state::mcu_map(address_map &map)
{
	map(0x2000, 0x3fff).rom().region("mcu", 0);
}


INPUT_CHANGED_MEMBER(pntnpuzl_state::coin_inserted)
{
	/* TODO: change this! */
	if(newval)
		m_maincpu->pulse_input_line((uint8_t)param, m_maincpu->minimum_quantum_time());
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x70, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

void pntnpuzl_state::pntnpuzl(machine_config &config)
{
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pntnpuzl_state::pntnpuzl_map);

	EEPROM_93C46_16BIT(config, "eeprom");

	MOS6522(config, m_via, 12_MHz_XTAL / 10);
	m_via->readpa_handler().set_ioport("IN2");
	m_via->readpb_handler().set_ioport("IN1");
	m_via->writepb_handler().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(4);
	m_via->writepb_handler().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(6);
	m_via->writepb_handler().append("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	// CB2 used for serial communication with 8798

	i8x9x_device &mcu(P8798(config, "mcu", 12_MHz_XTAL)); // clock generated by ASIC
	mcu.set_addrmap(AS_PROGRAM, &pntnpuzl_state::mcu_map); // FIXME: this is all internal
	mcu.ach6_cb().set_constant(0x180); // ?
	mcu.ach7_cb().set_constant(0x180); // ?
	mcu.hso_cb().set("mcu_eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(0);
	mcu.hso_cb().append("mcu_eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(1);
	mcu.hso_cb().append("mcu_eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(5);

	EEPROM_93C46_16BIT(config, "mcu_eeprom").do_callback().set_inputline("mcu", i8x9x_device::HSI3_LINE);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(25'174'800),900,0,640,526,0,480);
	screen.set_screen_update("vga", FUNC(vga_device::screen_update));

	vga_device &vga(VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x100000);
}

ROM_START( pntnpuzl )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "pntnpuzl.u2", 0x00001, 0x40000, CRC(dfda3f73) SHA1(cca8ccdd501a26cba07365b1238d7b434559bbc6) )
	ROM_LOAD16_BYTE( "pntnpuzl.u3", 0x00000, 0x40000, CRC(4173f250) SHA1(516fe6f91b925f71c36b97532608b82e63bda436) )

	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "pntnpzl8798.bin", 0x0000, 0x2000, CRC(3ff98e89) SHA1(c48665992cb5377b69902f2a352c9214602a0b84) )

	ROM_REGION( 0x400, "pic", 0 )
	ROM_LOAD( "16c54.bin", 0x000, 0x400, NO_DUMP )

	/* for reference, probably not used in any way by the game */
	ROM_REGION( 0x10000, "video_bios", 0 )
	ROM_LOAD( "trident_quadtel_tvga9000_isa16.bin", 0x0000, 0x10000, BAD_DUMP CRC(ad0e7351) SHA1(eb525460a80e1c1baa34642b93d54caf2607920d) )
ROM_END


void pntnpuzl_state::init_pip()
{
//  uint16_t *rom = (uint16_t *)memregion("maincpu")->base();
//  rom[0x2696/2] = 0x4e71;
//  rom[0x26a0/2] = 0x4e71;

}

GAME( 1993, pntnpuzl, 0, pntnpuzl, pntnpuzl, pntnpuzl_state, init_pip, ROT90, "Century Vending", "Paint 'N Puzzle", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
