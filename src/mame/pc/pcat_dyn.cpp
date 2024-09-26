// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/********************************************************************************************************************

Tournament Solitaire (c) 1995 Dynamo

Unmodified 486 PC-AT HW.

Jet Way Information Co. OP495SLC motherboard
 - AMD Am486-DX40 CPU
 - Trident TVGA9000i video card
 - Breve Technologies audio adapter, WSS+Sound Blaster
 - CH Products RollerMouse serial trackball, Mouse Systems compatible

preliminary driver by Angelo Salese

Notes:
Data from the 99378275.SN file on the rom filesystem is scrambled and written to DF80:0000-0100 then read back.
If the output isn't satisfactory, it prints "I/O BOARD FAILURE".

********************************************************************************************************************/

#include "emu.h"

#include "pcshare.h"

#include "bus/isa/isa.h"
#include "bus/isa/sblaster.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"

#include "cpu/i386/i386.h"

#include "machine/ds128x.h"
#include "machine/ins8250.h"
#include "machine/nvram.h"
#include "sound/ad1848.h"
#include "video/pc_vga_trident.h"

#include "screen.h"


namespace {

class pcat_dyn_state : public pcat_base_state
{
public:
	pcat_dyn_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
		, m_isabus(*this, "isa")
		, m_prgbank(*this, "prgbank")
		, m_nvram_bank(*this, "nvram_bank")
		, m_nvram_mem(0x2000)
	{ }

	void pcat_dyn(machine_config &config);

private:
	required_device<isa8_device> m_isabus;
	required_memory_bank m_prgbank;
	required_memory_bank m_nvram_bank;
	std::vector<uint8_t> m_nvram_mem;
	void bank1_w(uint8_t data);
	void bank2_w(uint8_t data);
	uint8_t audio_r(offs_t offset);
	void dma8237_1_dack_w(uint8_t data);
	virtual void machine_start() override ATTR_COLD;
	void nvram_init(nvram_device &nvram, void *base, size_t size);
	static void pcat_dyn_sb_conf(device_t *device);
	void pcat_io(address_map &map) ATTR_COLD;
	void pcat_map(address_map &map) ATTR_COLD;
};

void pcat_dyn_state::machine_start()
{
	m_prgbank->configure_entries(0, 256, memregion("game_prg")->base(), 0x1000);
	m_nvram_bank->configure_entries(0, 2, &m_nvram_mem[0], 0x1000);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_mem[0], 0x2000);
}

void pcat_dyn_state::nvram_init(nvram_device &nvram, void *base, size_t size)
{
	memcpy(base, memregion("nvram")->base(), size);
}

uint8_t pcat_dyn_state::audio_r(offs_t offset)
{
	switch(offset)
	{
		case 3:
			return 4;
	}
	return 0;
}

void pcat_dyn_state::bank1_w(uint8_t data)
{
	m_prgbank->set_entry(data);
}

void pcat_dyn_state::bank2_w(uint8_t data)
{
	m_nvram_bank->set_entry(data & 1);
}

void pcat_dyn_state::pcat_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000a0000, 0x000bffff).rw("vga", FUNC(tvga9000_device::mem_r), FUNC(tvga9000_device::mem_w));
	map(0x000c0000, 0x000c7fff).rom().region("video_bios", 0);
	map(0x000d0000, 0x000d0fff).rom().region("game_prg", 0x0000).w(FUNC(pcat_dyn_state::bank1_w));
	map(0x000d1000, 0x000d1fff).rom().region("game_prg", 0x1000).w(FUNC(pcat_dyn_state::bank2_w));
	map(0x000d2000, 0x000d2fff).bankr("prgbank");
	map(0x000d3000, 0x000d3fff).bankrw("nvram_bank");
	map(0x000df400, 0x000df8ff).ram(); //I/O board?
	map(0x000f0000, 0x000fffff).rom().region("bios", 0);
	map(0x00100000, 0x001fffff).ram();
	map(0xffff0000, 0xffffffff).rom().region("bios", 0);
}

void pcat_dyn_state::pcat_io(address_map &map)
{
	pcat32_io_common(map);
	map(0x03b0, 0x03df).m("vga", FUNC(tvga9000_device::io_map));
	map(0x03f8, 0x03ff).rw("ns16550", FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x0530, 0x0533).r(FUNC(pcat_dyn_state::audio_r));
	map(0x0534, 0x0537).rw("ad1848", FUNC(ad1848_device::read), FUNC(ad1848_device::write));
}

//TODO: use atmb device
void pcat_dyn_state::dma8237_1_dack_w(uint8_t data) { m_isabus->dack_w(1, data); }

static INPUT_PORTS_START( pcat_dyn )
	// M,N,Numpad 6 -- Hang
	// Enter,Numpad 4 -- 5 Credits
	PORT_START("pc_keyboard_0")
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Bookkeeping")

	PORT_START("pc_keyboard_1")
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_BILL1)

	PORT_START("pc_keyboard_2")
	// Don't use IPT_BUTTON1 or the mouse axes are mapped incorrectly
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Mouse Button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_COIN1)
INPUT_PORTS_END

static void pcat_dyn_com(device_slot_interface &device)
{
	device.option_add("msmouse", MSYSTEMS_HLE_SERIAL_MOUSE);
}

static void pcat_dyn_isa8_cards(device_slot_interface &device)
{
	device.option_add("sb15",  ISA8_SOUND_BLASTER_1_5);
}

static DEVICE_INPUT_DEFAULTS_START( pcat_dyn_sb_def )
	DEVICE_INPUT_DEFAULTS("CONFIG", 0x03, 0x01)
DEVICE_INPUT_DEFAULTS_END

void pcat_dyn_state::pcat_dyn_sb_conf(device_t *device)
{
	device->subdevice<pc_joy_device>("pc_joy")->set_default_option(nullptr); // remove joystick
}

void pcat_dyn_state::pcat_dyn(machine_config &config)
{
	/* basic machine hardware */
	I486(config, m_maincpu, 40000000); /* Am486 DX-40 */
	m_maincpu->set_addrmap(AS_PROGRAM, &pcat_dyn_state::pcat_map);
	m_maincpu->set_addrmap(AS_IO, &pcat_dyn_state::pcat_io);
	m_maincpu->set_irq_acknowledge_callback("pic8259_1", FUNC(pic8259_device::inta_cb));

	/* video hardware */
	// TODO: map to ISA bus
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(tvga9000_device::screen_update));

	tvga9000_device &vga(TVGA9000_VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x200000);

	pcat_common_nokeyboard(config);

	DS12885(config.replace(), m_mc146818);
	m_mc146818->irq().set("pic8259_2", FUNC(pic8259_device::ir0_w));
	m_mc146818->set_century_index(0x32);

	ad1848_device &ad1848(AD1848(config, "ad1848", 0));
	ad1848.irq().set("pic8259_1", FUNC(pic8259_device::ir5_w));
	ad1848.drq().set("dma8237_1", FUNC(am9517a_device::dreq0_w));

	m_dma8237_1->out_iow_callback<0>().set("ad1848", FUNC(ad1848_device::dack_w));
	m_dma8237_1->out_iow_callback<1>().set(FUNC(pcat_dyn_state::dma8237_1_dack_w));

	NVRAM(config, "nvram").set_custom_handler(FUNC(pcat_dyn_state::nvram_init));

	ns16550_device &uart(NS16550(config, "ns16550", XTAL(1'843'200)));
	uart.out_tx_callback().set("serport", FUNC(rs232_port_device::write_txd));
	uart.out_dtr_callback().set("serport", FUNC(rs232_port_device::write_dtr));
	uart.out_rts_callback().set("serport", FUNC(rs232_port_device::write_rts));
	uart.out_int_callback().set("pic8259_1", FUNC(pic8259_device::ir4_w));

	rs232_port_device &serport(RS232_PORT(config, "serport", pcat_dyn_com, "msmouse"));
	serport.set_fixed(true);
	serport.rxd_handler().set("ns16550", FUNC(ins8250_uart_device::rx_w));
	serport.dcd_handler().set("ns16550", FUNC(ins8250_uart_device::dcd_w));
	serport.dsr_handler().set("ns16550", FUNC(ins8250_uart_device::dsr_w));
	serport.ri_handler().set("ns16550", FUNC(ins8250_uart_device::ri_w));
	serport.cts_handler().set("ns16550", FUNC(ins8250_uart_device::cts_w));

	ISA8(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->irq2_callback().set("pic8259_2", FUNC(pic8259_device::ir1_w));
	m_isabus->irq3_callback().set("pic8259_1", FUNC(pic8259_device::ir3_w));
	//m_isabus->irq4_callback().set("pic8259_1", FUNC(pic8259_device::ir4_w));
	//m_isabus->irq5_callback().set("pic8259_1", FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set("pic8259_1", FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set("pic8259_1", FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set("dma8237_1", FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set("dma8237_1", FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set("dma8237_1", FUNC(am9517a_device::dreq3_w));

	// FIXME: determine ISA bus clock
	isa8_slot_device &isa1(ISA8_SLOT(config, "isa1", 0, "isa", pcat_dyn_isa8_cards, "sb15", true));
	isa1.set_option_device_input_defaults("sb15", DEVICE_INPUT_DEFAULTS_NAME(pcat_dyn_sb_def));
	isa1.set_option_machine_config("sb15", pcat_dyn_sb_conf);
}

/***************************************
*
* ROM definitions
*
***************************************/

ROM_START(toursol)
	ROM_REGION32_LE(0x10000, "bios", 0) /* Motherboard BIOS */
	ROM_LOAD("prom.mb", 0x000000, 0x10000, CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	ROM_REGION32_LE(0x20000, "video_bios", 0)    /* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(               0x00001, 0x04000 )

	ROM_REGION32_LE(0x100000, "game_prg", 0)    /* PromStor 32, mapping unknown */
	ROM_LOAD("sol.u21", 0x00000, 0x40000, CRC(e97724d9) SHA1(995b89d129c371b815c6b498093bd1bbf9fd8755))
	ROM_LOAD("sol.u22", 0x40000, 0x40000, CRC(69d42f50) SHA1(737fe62f3827b00b4f6f3b72ef6c7b6740947e95))
	ROM_LOAD("sol.u23", 0x80000, 0x40000, CRC(d1e39bd4) SHA1(39c7ee43cddb53fba0f7c0572ddc40289c4edd07))
	ROM_LOAD("sol.u24", 0xc0000, 0x40000, CRC(555341e0) SHA1(81fee576728855e234ff7aae06f54ae9705c3ab5))
	ROM_FILL(0x2a3e6, 1, 0xeb) // skip prot(?) check
	ROM_FILL(0x51bd2, 2, 0x90) // opl2 probe expects timer expiration too quickly

	ROM_REGION(0x2000, "nvram", 0)
	ROM_LOAD("sol.u28", 0, 0x2000, CRC(c9374d50) SHA1(49173bc69f70bb2a7e8af9d03e2538b34aa881d8))

	ROM_REGION(128, "rtc", 0)
	ROM_LOAD("rtc", 0, 128, BAD_DUMP CRC(b0906127) SHA1(a771b1e6fc4916c319c7d44391af95bb821e3a7b))
ROM_END


ROM_START(toursol1)
	ROM_REGION32_LE(0x10000, "bios", 0) /* Motherboard BIOS */
	ROM_LOAD("prom.mb", 0x000000, 0x10000, CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	ROM_REGION32_LE(0x20000, "video_bios", 0)    /* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(               0x00001, 0x04000 )

	ROM_REGION32_LE(0x100000, "game_prg", 0)    /* PromStor 32, mapping unknown */
	ROM_LOAD("prom.0", 0x00000, 0x40000, CRC(f26ce73f) SHA1(5516c31aa18716a47f46e412fc273ae8784d2061))
	ROM_LOAD("prom.1", 0x40000, 0x40000, CRC(8f96e2a8) SHA1(bc3ce8b99e6ff40e355df2c3f797f1fe88b3b219))
	ROM_LOAD("prom.2", 0x80000, 0x40000, CRC(8b0ac5cf) SHA1(1c2b6a53c9ff4d18a5227d899facbbc719f40205))
	ROM_LOAD("prom.3", 0xc0000, 0x40000, CRC(9352e965) SHA1(2bfb647ec27c60a8c821fdf7483199e1a444cea8))
	ROM_FILL(0x334f6, 1, 0xeb) // skip prot(?) check

	ROM_REGION(0x2000, "nvram", 0)
	ROM_LOAD("prom.7", 0, 0x2000, CRC(154c8092) SHA1(4439ee82f36d5d5c334494ba7bb4848e839213a7))

	ROM_REGION(128, "rtc", 0)
	ROM_LOAD("rtc", 0, 128, BAD_DUMP CRC(b0906127) SHA1(a771b1e6fc4916c319c7d44391af95bb821e3a7b))
ROM_END

} // anonymous namespace


GAME( 1995, toursol,  0,       pcat_dyn, pcat_dyn, pcat_dyn_state, empty_init, ROT0, "Dynamo", "Tournament Solitaire (V1.06, 08/03/95)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, toursol1, toursol, pcat_dyn, pcat_dyn, pcat_dyn_state, empty_init, ROT0, "Dynamo", "Tournament Solitaire (V1.04, 06/22/95)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
