// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
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
#include "machine/nvram.h"
#include "bus/isa/trident.h"
#include "bus/isa/isa.h"
#include "bus/isa/sblaster.h"

class magtouch_state : public pcat_base_state
{
public:
	magtouch_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag),
			m_isabus(*this, "isa"),
			m_rombank(*this, "rombank"),
			m_in0(*this, "IN0"){ }

	required_device<isa8_device> m_isabus;
	required_memory_bank m_rombank;
	required_ioport m_in0;

	DECLARE_READ8_MEMBER(magtouch_io_r);
	DECLARE_WRITE8_MEMBER(magtouch_io_w);
	DECLARE_WRITE8_MEMBER(dma8237_1_dack_w);
	virtual void machine_start() override;
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
			return m_in0->read();
		default:
			return 0;
	}
}

WRITE8_MEMBER(magtouch_state::magtouch_io_w)
{
	switch(offset)
	{
		case 6:
			m_rombank->set_entry(data & 0x7f );
			break;
	}
}

static ADDRESS_MAP_START( magtouch_map, AS_PROGRAM, 32, magtouch_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_DEVREADWRITE8("vga", trident_vga_device, mem_r, mem_w, 0xffffffff)
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000d0000, 0x000d1fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x000d8000, 0x000dffff) AM_ROMBANK("rombank")
	AM_RANGE(0x000f0000, 0x000fffff) AM_RAM AM_REGION("bios", 0 )
	AM_RANGE(0xffff0000, 0xffffffff) AM_ROM AM_REGION("bios", 0 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( magtouch_io, AS_IO, 32, magtouch_state )
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x02e0, 0x02e7) AM_READWRITE8(magtouch_io_r, magtouch_io_w, 0xffffffff)
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", trident_vga_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", trident_vga_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", trident_vga_device, port_03d0_r, port_03d0_w, 0xffffffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ns16450_0", ns16450_device, ins8250_r, ins8250_w, 0xffffffff)
ADDRESS_MAP_END

static INPUT_PORTS_START( magtouch )
	PORT_START("IN0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Clear") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2) PORT_IMPULSE(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN3) PORT_IMPULSE(2)
INPUT_PORTS_END

//TODO: use atmb device
WRITE8_MEMBER( magtouch_state::dma8237_1_dack_w ){ m_isabus->dack_w(1, data); }

void magtouch_state::machine_start()
{
	m_rombank->configure_entries(0, 0x80, memregion("game_prg")->base(), 0x8000 );
	m_rombank->set_entry(0);
	machine().device<nvram_device>("nvram")->set_base(memshare("nvram")->ptr(), 0x2000);
}

static SLOT_INTERFACE_START( magtouch_isa8_cards )
	SLOT_INTERFACE("sb15",  ISA8_SOUND_BLASTER_1_5)
SLOT_INTERFACE_END

static DEVICE_INPUT_DEFAULTS_START( magtouch_sb_def )
	DEVICE_INPUT_DEFAULTS("CONFIG", 0x03, 0x01)
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_FRAGMENT( magtouch_sb_conf )
	MCFG_DEVICE_MODIFY("pc_joy")
	MCFG_DEVICE_SLOT_INTERFACE(pc_joysticks, nullptr, true) // remove joystick
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( magtouch, magtouch_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I386, 14318180*2)   /* I386 ?? Mhz */
	MCFG_CPU_PROGRAM_MAP(magtouch_map)
	MCFG_CPU_IO_MAP(magtouch_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_trident_vga )
	MCFG_DEVICE_REPLACE("vga", TVGA9000_VGA, 0)

	MCFG_FRAGMENT_ADD( pcat_common )
	MCFG_DEVICE_ADD( "ns16450_0", NS16450, XTAL_1_8432MHz )
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("microtouch", microtouch_device, rx))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir4_w))
	MCFG_MICROTOUCH_ADD( "microtouch", 9600, DEVWRITELINE("ns16450_0", ins8250_uart_device, rx_w) )

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_MODIFY("dma8237_1")
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(magtouch_state, dma8237_1_dack_w))

	MCFG_DEVICE_ADD("isa", ISA8, 0)
	MCFG_ISA8_CPU(":maincpu")
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("pic8259_2", pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir3_w))
	//MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq3_w))

	MCFG_ISA8_SLOT_ADD("isa", "isa1", magtouch_isa8_cards, "sb15", true)
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("sb15", magtouch_sb_def)
	MCFG_DEVICE_CARD_MACHINE_CONFIG("sb15", magtouch_sb_conf)
MACHINE_CONFIG_END


ROM_START(magtouch)
	ROM_REGION32_LE(0x10000, "bios", 0) /* motherboard bios */
	ROM_LOAD("mtouch.u13", 0x00000, 0x10000, CRC(e74fb144) SHA1(abc99e84832c30606374da542fd94f0fbc8cbaa6) )

	ROM_REGION(0x08000, "video_bios", 0)
	//this is a phoenix standard vga only bios from 1991 despite the notes above saying the machine has a trident svga adapter
	//ROM_LOAD16_BYTE("vga1-bios-ver-b-1.00-07.u8",     0x00000, 0x04000, CRC(a40551d6) SHA1(db38190f06e4af2c2d59ae310e65883bb16cd3d6))
	//ROM_CONTINUE(                                     0x00001, 0x04000 )
	//the game requires an svga mode, 640x480x8bpp so substituting a quadtel (later purchased by phoenix) trident ver. D4.01E bios
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, BAD_DUMP CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(                                     0x00001, 0x04000 )

	ROM_REGION(0x400000, "game_prg", 0) /* proper game */
	ROM_LOAD("mtouch.u20", 0x000000,0x100000, CRC(fb7b529b) SHA1(ecf8792ce7b6b2f59c2178dc1524c3830a4b4ebc) )
	ROM_LOAD("mtouch.u21", 0x100000,0x100000, CRC(af1491a6) SHA1(2d09506a3368fd64b1081017c58065635be5a62f) )
	ROM_LOAD("mtouch.u22", 0x200000,0x100000, CRC(da39c860) SHA1(7648e063ec68575abd808d5dea933f292197a2c2) )
	ROM_FILL(0x511fa, 1, 0xeb) // skip prot(?) check and exe relocation
	ROM_FILL(0x511fb, 1, 0x03)
	ROM_FILL(0x511ba, 1, 0xeb) // skip csum
ROM_END

GAME( 1995, magtouch,   0,         magtouch,  magtouch, driver_device, 0, ROT0, "Micro Manufacturing",     "Magical Touch", MACHINE_UNEMULATED_PROTECTION )
