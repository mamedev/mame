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

#include "pcshare.h"

#include "cpu/i386/i386.h"
#include "machine/pic8259.h"
#include "machine/mc146818.h"
#include "machine/ins8250.h"
#include "machine/microtch.h"
#include "machine/nvram.h"
#include "video/pc_vga_trident.h"
#include "bus/isa/isa.h"
#include "bus/isa/sblaster.h"

namespace {

class magtouch_state : public pcat_base_state
{
public:
	magtouch_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
		, m_isabus(*this, "isa")
		, m_rombank(*this, "rombank")
		, m_shadow(*this, "shadow")
		, m_in0(*this, "IN0")
	{ }

	void magtouch(machine_config &config);

private:
	required_device<isa8_device> m_isabus;
	required_memory_bank m_rombank;
	required_shared_ptr<uint32_t> m_shadow;
	required_ioport m_in0;

	uint8_t magtouch_io_r(offs_t offset);
	void magtouch_io_w(offs_t offset, uint8_t data);
	void dma8237_1_dack_w(uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	static void magtouch_sb_conf(device_t *device);
	void magtouch_io(address_map &map) ATTR_COLD;
	void magtouch_map(address_map &map) ATTR_COLD;
};

/*************************************
 *
 *  ROM banking
 *
 *************************************/

uint8_t magtouch_state::magtouch_io_r(offs_t offset)
{
	switch(offset)
	{
		case 1:
			return m_in0->read();
		default:
			return 0;
	}
}

void magtouch_state::magtouch_io_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 6:
			m_rombank->set_entry(data & 0x7f );
			break;
	}
}

void magtouch_state::magtouch_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000bffff).rw("vga", FUNC(tvga9000_device::mem_r), FUNC(tvga9000_device::mem_w));
	map(0x000c0000, 0x000c7fff).rom().region("video_bios", 0);
	map(0x000d0000, 0x000d1fff).ram().share("nvram");
	map(0x000d8000, 0x000dffff).bankr("rombank");
	map(0x000f0000, 0x000fffff).ram().share(m_shadow);
	map(0xffff0000, 0xffffffff).rom().region("bios", 0);
}

void magtouch_state::magtouch_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x02e0, 0x02e7).rw(FUNC(magtouch_state::magtouch_io_r), FUNC(magtouch_state::magtouch_io_w));
	map(0x03b0, 0x03df).m("vga", FUNC(tvga9000_device::io_map));
	map(0x03f8, 0x03ff).rw("ns16450_0", FUNC(ns16450_device::ins8250_r), FUNC(ns16450_device::ins8250_w));
}

static INPUT_PORTS_START( magtouch )
	PORT_START("IN0")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Clear") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN1) PORT_IMPULSE(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2) PORT_IMPULSE(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN3) PORT_IMPULSE(2)
INPUT_PORTS_END

//TODO: use atmb device
void magtouch_state::dma8237_1_dack_w(uint8_t data) { m_isabus->dack_w(1, data); }

void magtouch_state::machine_start()
{
	m_rombank->configure_entries(0, 0x80, memregion("game_prg")->base(), 0x8000 );
	m_rombank->set_entry(0);
	subdevice<nvram_device>("nvram")->set_base(memshare("nvram")->ptr(), 0x2000);
}

void magtouch_state::machine_reset()
{
	// Rom shadow is not handled, well, at all
	memcpy(m_shadow, memregion("bios")->base(), 0x10000);
}

static void magtouch_isa8_cards(device_slot_interface &device)
{
	device.option_add("sb15",  ISA8_SOUND_BLASTER_1_5);
}

static DEVICE_INPUT_DEFAULTS_START( magtouch_sb_def )
	DEVICE_INPUT_DEFAULTS("CONFIG", 0x03, 0x01)
DEVICE_INPUT_DEFAULTS_END

void magtouch_state::magtouch_sb_conf(device_t *device)
{
	device->subdevice<pc_joy_device>("pc_joy")->set_default_option(nullptr); // remove joystick
}

void magtouch_state::magtouch(machine_config &config)
{
	/* basic machine hardware */
	I386(config, m_maincpu, 14318180*2);   /* I386 ?? Mhz */
	m_maincpu->set_addrmap(AS_PROGRAM, &magtouch_state::magtouch_map);
	m_maincpu->set_addrmap(AS_IO, &magtouch_state::magtouch_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	/* video hardware */
	// TODO: map to ISA bus
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(tvga9000_device::screen_update));

	tvga9000_device &vga(TVGA9000_VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x200000);

	pcat_common(config);

	ns16450_device &uart(NS16450(config, "ns16450_0", XTAL(1'843'200)));
	uart.out_tx_callback().set("microtouch", FUNC(microtouch_device::rx));
	uart.out_int_callback().set("pic8259_1", FUNC(pic8259_device::ir4_w));

	MICROTOUCH(config, "microtouch", 9600).stx().set("ns16450_0", FUNC(ins8250_uart_device::rx_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	m_dma8237_1->out_iow_callback<1>().set(FUNC(magtouch_state::dma8237_1_dack_w));

	ISA8(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->irq2_callback().set("pic8259_2", FUNC(pic8259_device::ir1_w));
	m_isabus->irq3_callback().set("pic8259_1", FUNC(pic8259_device::ir3_w));
	//m_isabus->irq4_callback().set("pic8259_1", FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set("pic8259_1", FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set("pic8259_1", FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set("pic8259_1", FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set("dma8237_1", FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set("dma8237_1", FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set("dma8237_1", FUNC(am9517a_device::dreq3_w));

	// FIXME: determine ISA bus clock
	isa8_slot_device &isa1(ISA8_SLOT(config, "isa1", 0, m_isabus, magtouch_isa8_cards, "sb15", true));
	isa1.set_option_device_input_defaults("sb15", DEVICE_INPUT_DEFAULTS_NAME(magtouch_sb_def));
	isa1.set_option_machine_config("sb15", magtouch_sb_conf);
}


ROM_START(magtouch)
	ROM_REGION32_LE(0x10000, "bios", 0) /* motherboard bios */
	ROM_LOAD("mtouch.u13", 0x00000, 0x10000, CRC(e74fb144) SHA1(abc99e84832c30606374da542fd94f0fbc8cbaa6) )

	ROM_REGION32_LE(0x08000, "video_bios", 0)
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

} // anonymous namespace

GAME( 1995, magtouch, 0, magtouch, magtouch, magtouch_state, empty_init, ROT0, "Micro Manufacturing",     "Magical Touch", MACHINE_UNEMULATED_PROTECTION )
