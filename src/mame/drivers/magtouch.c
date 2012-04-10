/*
Magical Touch by Micro MFG

Preliminary driver by Mariusz Wojcieszek

Big daughter card
silkscreend         2296
                COMPONENT SIDE
                EPROM/IO GAME BOARD
                JNS-0001 REV-C

sticker on back
                Warranty Void If Removed
                Date: 07-30-1996
                SER# MTC-COM-170065

.u20    27c801  stickered   MTOUCH
                ROM-0
                U20
                041296
                MICRO
                MFG.
                COPYRIGHT (C)

.u21    27c801  stickered   MTOUCH
                ROM-1
                U21
                041296
                MICRO
                MFG.
                COPYRIGHT (C)

.u22    27c801  stickered   MTOUCH
                ROM-2
                U22
                041296
                MICRO
                MFG.
                COPYRIGHT (C)

.u7 ds1225y-150     read as 2764

.u8 tibpal16l8-15       blue dot on it  checksum was 0

.u4 gal20v8b

1.8432 MHz crystal
18.00 Mhz Crystal
ns16450n
es488f d465 wf62225


motherboard
.u13    27c512          stickered   Warranty Void If Removed
                        Date: 07-30-1996
                        SER# MTC-MBD-170065

am386dx-40 ng80386dx-40
ali m1429 a1 9504 ts6 ab3519
ali m1431 a2 9503 ts6ab0511b
jetkey v5.0 fastest keyboard bios

video card
.u6 - unknown chip type     stickered   Warranty Void If Removed
                        Date: 07-30-1996
                        SER# MTC-VGA-170065
                silkscreend Trident Ver. D4.01E
                        (c)'95 Trident Microsystems
                        (C)'90 Phoenix Technologies
                        *605C61W6ANJDH009       * = a triangle character

*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"
#include "machine/ins8250.h"
#include "machine/microtch.h"
#include "video/pc_vga.h"


class magtouch_state : public driver_device
{
public:
	magtouch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_uart(*this, "ns16450_0"),
		  m_microtouch(*this, "microtouch")
	{ }

	required_device<ns16450_device> m_uart;
	required_device<microtouch_serial_device> m_microtouch;

	DECLARE_WRITE_LINE_MEMBER(microtouch_out);
	DECLARE_WRITE_LINE_MEMBER(microtouch_in);
	DECLARE_READ8_MEMBER(magtouch_io_r);
	DECLARE_WRITE8_MEMBER(magtouch_io_w);
};


/*************************************
 *
 *  Microtouch <-> ins8250 interface
 *
 *************************************/

WRITE_LINE_MEMBER(magtouch_state::microtouch_out)
{
	m_microtouch->rx(state);
}

WRITE_LINE_MEMBER(magtouch_state::microtouch_in)
{
	m_uart->rx_w(state);
}

static WRITE_LINE_DEVICE_HANDLER( at_com_interrupt_1 )
{
	pic8259_ir4_w(device->machine().device("pic8259_1"), state);
}

static const ins8250_interface magtouch_com0_interface =
{
	DEVCB_DRIVER_LINE_MEMBER(magtouch_state, microtouch_out),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(at_com_interrupt_1),
	DEVCB_NULL,
	DEVCB_NULL
};

static const microtouch_serial_interface magtouch_microtouch_interface =
{
	DEVCB_DRIVER_LINE_MEMBER(magtouch_state, microtouch_in)
};

/*************************************
 *
 *  ROM banking
 *
 *************************************/

READ8_MEMBER(magtouch_state::magtouch_io_r)
{
	switch(offset)
	{
		case 1:
			return input_port_read(machine(), "IN0");
		default:
			return 0;
	}
}

WRITE8_MEMBER(magtouch_state::magtouch_io_w)
{
	switch(offset)
	{
		case 6:
			memory_set_bank(machine(), "rombank", data & 0x7f );
			break;
	}
}

static ADDRESS_MAP_START( magtouch_map, AS_PROGRAM, 32, magtouch_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000d8000, 0x000dffff) AM_ROMBANK("rombank")
	AM_RANGE(0x000f0000, 0x000fffff) AM_RAM AM_REGION("bios", 0 )
	AM_RANGE(0x00100000, 0x0027ffff) AM_ROM AM_REGION("game_prg", 0)
	AM_RANGE(0xffff0000, 0xffffffff) AM_ROM AM_REGION("bios", 0 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( magtouch_io, AS_IO, 32, magtouch_state )
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x02e0, 0x02e7) AM_READWRITE8(magtouch_io_r, magtouch_io_w, 0xffffffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ns16450_0", ns16450_device, ins8250_r, ins8250_w, 0xffffffff)
ADDRESS_MAP_END

static INPUT_PORTS_START( magtouch )
	PORT_START("IN0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Clear") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2) PORT_IMPULSE(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN3) PORT_IMPULSE(1)
INPUT_PORTS_END

static void magtouch_set_keyb_int(running_machine &machine, int state)
{
	pic8259_ir1_w(machine.device("pic8259_1"), state);
}

static READ8_HANDLER( vga_setting ) { return 0xff; } // hard-code to color

static MACHINE_START( magtouch )
{
	device_set_irq_callback(machine.device("maincpu"), pcat_irq_callback);

	init_pc_common(machine, PCCOMMON_KEYBOARD_AT, magtouch_set_keyb_int);

	memory_configure_bank(machine, "rombank", 0, 0x80, machine.region("game_prg")->base(), 0x8000 );
	memory_set_bank(machine, "rombank", 0);

//  microtouch_init(machine, magtouch_microtouch_tx_callback, NULL);
}

static MACHINE_CONFIG_START( magtouch, magtouch_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I386, 14318180*2)	/* I386 ?? Mhz */
	MCFG_CPU_PROGRAM_MAP(magtouch_map)
	MCFG_CPU_IO_MAP(magtouch_io)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_vga )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */

	MCFG_MACHINE_START(magtouch)
	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )

//  MCFG_FRAGMENT_ADD( at_kbdc8042 )
	MCFG_FRAGMENT_ADD( pcat_common )
	MCFG_NS16450_ADD( "ns16450_0", magtouch_com0_interface, XTAL_1_8432MHz )
	MCFG_MICROTOUCH_SERIAL_ADD( "microtouch", magtouch_microtouch_interface, 9600 ) // rate?
MACHINE_CONFIG_END


ROM_START(magtouch)
	ROM_REGION32_LE(0x10000, "bios", 0)	/* motherboard bios */
	ROM_LOAD("mtouch.u13", 0x00000, 0x10000, CRC(e74fb144) SHA1(abc99e84832c30606374da542fd94f0fbc8cbaa6) )

	ROM_REGION(0x08000, "video_bios", 0)
	ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x400000, "game_prg", 0)	/* proper game */
	ROM_LOAD("mtouch.u20", 0x000000,0x100000, CRC(fb7b529b) SHA1(ecf8792ce7b6b2f59c2178dc1524c3830a4b4ebc) )
	ROM_LOAD("mtouch.u21", 0x100000,0x100000, CRC(af1491a6) SHA1(2d09506a3368fd64b1081017c58065635be5a62f) )
	ROM_LOAD("mtouch.u22", 0x200000,0x100000, CRC(da39c860) SHA1(7648e063ec68575abd808d5dea933f292197a2c2) )
ROM_END

static DRIVER_INIT(magtouch)
{
	pc_vga_init(machine, vga_setting, NULL);
	pc_vga_io_init(machine, machine.device("maincpu")->memory().space(AS_PROGRAM), 0xa0000, machine.device("maincpu")->memory().space(AS_IO), 0x0000);
}

GAME( 1995, magtouch,   0,         magtouch,  magtouch, magtouch, ROT0, "Micro Manufacturing",     "Magical Touch", GAME_NOT_WORKING | GAME_NO_SOUND )
