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
#include "cpu/i386/i386.h"
#include "machine/pcshare.h"
#include "bus/isa/trident.h"
#include "machine/bankdev.h"
#include "machine/ds128x.h"
#include "machine/ins8250.h"
#include "machine/nvram.h"
#include "sound/ad1848.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/ser_mouse.h"
#include "bus/isa/isa.h"
#include "bus/isa/sblaster.h"

class pcat_dyn_state : public pcat_base_state
{
public:
	pcat_dyn_state(const machine_config &mconfig, device_type type, const char *tag)
		: pcat_base_state(mconfig, type, tag)
		, m_isabus(*this, "isa")
		, m_prgbank(*this, "prgbank")
		, m_nvram_bank(*this, "nvram_bank")
		, m_nvram_mem(0x2000){ }

	required_device<isa8_device> m_isabus;
	required_memory_bank m_prgbank;
	required_memory_bank m_nvram_bank;
	std::vector<UINT8> m_nvram_mem;
	DECLARE_WRITE8_MEMBER(bank1_w);
	DECLARE_WRITE8_MEMBER(bank2_w);
	DECLARE_READ8_MEMBER(audio_r);
	DECLARE_WRITE8_MEMBER(dma8237_1_dack_w);
	virtual void machine_start() override;
	void nvram_init(nvram_device &nvram, void *base, size_t size);
};

void pcat_dyn_state::machine_start()
{
	m_prgbank->configure_entries(0, 256, memregion("game_prg")->base(), 0x1000);
	m_nvram_bank->configure_entries(0, 2, &m_nvram_mem[0], 0x1000);
	machine().device<nvram_device>("nvram")->set_base(&m_nvram_mem[0], 0x2000);
}

void pcat_dyn_state::nvram_init(nvram_device &nvram, void *base, size_t size)
{
	memcpy(base, memregion("nvram")->base(), size);
}

READ8_MEMBER(pcat_dyn_state::audio_r)
{
	switch(offset)
	{
		case 3:
			return 4;
	}
	return 0;
}

WRITE8_MEMBER(pcat_dyn_state::bank1_w)
{
	m_prgbank->set_entry(data);
}

WRITE8_MEMBER(pcat_dyn_state::bank2_w)
{
	m_nvram_bank->set_entry(data & 1);
}

static ADDRESS_MAP_START( pcat_map, AS_PROGRAM, 32, pcat_dyn_state )
	AM_RANGE(0x00000000, 0x0009ffff) AM_RAM
	AM_RANGE(0x000a0000, 0x000bffff) AM_DEVREADWRITE8("vga", trident_vga_device, mem_r, mem_w, 0xffffffff)
	AM_RANGE(0x000c0000, 0x000c7fff) AM_ROM AM_REGION("video_bios", 0)
	AM_RANGE(0x000d0000, 0x000d0fff) AM_ROM AM_REGION("game_prg", 0x0000) AM_WRITE8(bank1_w, 0xffffffff)
	AM_RANGE(0x000d1000, 0x000d1fff) AM_ROM AM_REGION("game_prg", 0x1000) AM_WRITE8(bank2_w, 0xffffffff)
	AM_RANGE(0x000d2000, 0x000d2fff) AM_ROMBANK("prgbank")
	AM_RANGE(0x000d3000, 0x000d3fff) AM_RAMBANK("nvram_bank")
	AM_RANGE(0x000df400, 0x000df8ff) AM_RAM //I/O board?
	AM_RANGE(0x000f0000, 0x000fffff) AM_ROM AM_REGION("bios", 0 )
	AM_RANGE(0x00100000, 0x001fffff) AM_RAM
	AM_RANGE(0xffff0000, 0xffffffff) AM_ROM AM_REGION("bios", 0 )
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcat_io, AS_IO, 32, pcat_dyn_state )
	AM_IMPORT_FROM(pcat32_io_common)
	AM_RANGE(0x03b0, 0x03bf) AM_DEVREADWRITE8("vga", trident_vga_device, port_03b0_r, port_03b0_w, 0xffffffff)
	AM_RANGE(0x03c0, 0x03cf) AM_DEVREADWRITE8("vga", trident_vga_device, port_03c0_r, port_03c0_w, 0xffffffff)
	AM_RANGE(0x03d0, 0x03df) AM_DEVREADWRITE8("vga", trident_vga_device, port_03d0_r, port_03d0_w, 0xffffffff)
	AM_RANGE(0x03f8, 0x03ff) AM_DEVREADWRITE8("ns16550", ns16550_device, ins8250_r, ins8250_w, 0xffffffff)
	AM_RANGE(0x0530, 0x0533) AM_READ8(audio_r, 0xffffffff)
	AM_RANGE(0x0534, 0x0537) AM_DEVREADWRITE8("ad1848", ad1848_device, read, write, 0xffffffff)
ADDRESS_MAP_END

//TODO: use atmb device
WRITE8_MEMBER( pcat_dyn_state::dma8237_1_dack_w ){ m_isabus->dack_w(1, data); }

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

static SLOT_INTERFACE_START(pcat_dyn_com)
	SLOT_INTERFACE("msmouse", MSYSTEM_SERIAL_MOUSE)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( pcat_dyn_isa8_cards )
	SLOT_INTERFACE("sb15",  ISA8_SOUND_BLASTER_1_5)
SLOT_INTERFACE_END

static DEVICE_INPUT_DEFAULTS_START( pcat_dyn_sb_def )
	DEVICE_INPUT_DEFAULTS("CONFIG", 0x03, 0x01)
DEVICE_INPUT_DEFAULTS_END

static MACHINE_CONFIG_FRAGMENT( pcat_dyn_sb_conf )
	MCFG_DEVICE_MODIFY("pc_joy")
	MCFG_DEVICE_SLOT_INTERFACE(pc_joysticks, nullptr, true) // remove joystick
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pcat_dyn, pcat_dyn_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I486, 40000000) /* Am486 DX-40 */
	MCFG_CPU_PROGRAM_MAP(pcat_map)
	MCFG_CPU_IO_MAP(pcat_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_1", pic8259_device, inta_cb)

	/* video hardware */
	MCFG_FRAGMENT_ADD( pcvideo_trident_vga )
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_DEVICE_REPLACE("vga", TVGA9000_VGA, 0)

	MCFG_FRAGMENT_ADD( pcat_common )

	MCFG_DEVICE_REMOVE("rtc")
	MCFG_DS12885_ADD("rtc")
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir0_w))
	MCFG_MC146818_CENTURY_INDEX(0x32)

	MCFG_SOUND_ADD("ad1848", AD1848, 0)
	MCFG_AD1848_IRQ_CALLBACK(DEVWRITELINE("pic8259_1", pic8259_device, ir5_w))
	MCFG_AD1848_DRQ_CALLBACK(DEVWRITELINE("dma8237_1", am9517a_device, dreq0_w))

	MCFG_DEVICE_MODIFY("dma8237_1")
	MCFG_I8237_OUT_IOW_0_CB(DEVWRITE8("ad1848", ad1848_device, dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(pcat_dyn_state, dma8237_1_dack_w))

	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram", pcat_dyn_state, nvram_init)

	MCFG_DEVICE_ADD( "ns16550", NS16550, XTAL_1_8432MHz )
	MCFG_INS8250_OUT_TX_CB(DEVWRITELINE("serport", rs232_port_device, write_txd))
	MCFG_INS8250_OUT_DTR_CB(DEVWRITELINE("serport", rs232_port_device, write_dtr))
	MCFG_INS8250_OUT_RTS_CB(DEVWRITELINE("serport", rs232_port_device, write_rts))
	MCFG_INS8250_OUT_INT_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir4_w))
	MCFG_RS232_PORT_ADD( "serport", pcat_dyn_com, "msmouse" )
	MCFG_SLOT_FIXED(true)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, rx_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, dcd_w))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, dsr_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, ri_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("ns16550", ins8250_uart_device, cts_w))

	MCFG_DEVICE_ADD("isa", ISA8, 0)
	MCFG_ISA8_CPU(":maincpu")
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("pic8259_2", pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir3_w))
	//MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir4_w))
	//MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE("dma8237_1", am9517a_device, dreq3_w))

	MCFG_ISA8_SLOT_ADD("isa", "isa1", pcat_dyn_isa8_cards, "sb15", true)
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("sb15", pcat_dyn_sb_def)
	MCFG_DEVICE_CARD_MACHINE_CONFIG("sb15", pcat_dyn_sb_conf)
MACHINE_CONFIG_END

/***************************************
*
* ROM definitions
*
***************************************/

ROM_START(toursol)
	ROM_REGION32_LE(0x10000, "bios", 0) /* Motherboard BIOS */
	ROM_LOAD("prom.mb", 0x000000, 0x10000, CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	ROM_REGION(0x20000, "video_bios", 0)    /* Trident TVGA9000 BIOS */
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
	ROM_LOAD("rtc", 0, 128, BAD_DUMP CRC(732f64c8) SHA1(5386eac3afef9b16af8dd7766e577f7ac700d9cc))
ROM_END


ROM_START(toursol1)
	ROM_REGION32_LE(0x10000, "bios", 0) /* Motherboard BIOS */
	ROM_LOAD("prom.mb", 0x000000, 0x10000, CRC(e44bfd3c) SHA1(c07ec94e11efa30e001f39560010112f73cc0016) )

	ROM_REGION(0x20000, "video_bios", 0)    /* Trident TVGA9000 BIOS */
	ROM_LOAD16_BYTE("prom.vid", 0x00000, 0x04000, CRC(ad7eadaf) SHA1(ab379187914a832284944e81e7652046c7d938cc) )
	ROM_CONTINUE(               0x00001, 0x04000 )

	ROM_REGION32_LE(0x100000, "game_prg", 0)    /* PromStor 32, mapping unknown */
	ROM_LOAD("prom.0", 0x00000, 0x40000, CRC(f26ce73f) SHA1(5516c31aa18716a47f46e412fc273ae8784d2061))
	ROM_LOAD("prom.1", 0x40000, 0x40000, CRC(8f96e2a8) SHA1(bc3ce8b99e6ff40e355df2c3f797f1fe88b3b219))
	ROM_LOAD("prom.2", 0x80000, 0x40000, CRC(8b0ac5cf) SHA1(1c2b6a53c9ff4d18a5227d899facbbc719f40205))
	ROM_LOAD("prom.3", 0xc0000, 0x40000, CRC(9352e965) SHA1(2bfb647ec27c60a8c821fdf7483199e1a444cea8))

	ROM_REGION(0x2000, "nvram", 0)
	ROM_LOAD("prom.7", 0, 0x2000, CRC(154c8092) SHA1(4439ee82f36d5d5c334494ba7bb4848e839213a7))

	ROM_REGION(128, "rtc", 0)
	ROM_LOAD("rtc", 0, 128, BAD_DUMP CRC(732f64c8) SHA1(5386eac3afef9b16af8dd7766e577f7ac700d9cc))
ROM_END


GAME( 1995, toursol,  0,       pcat_dyn, pcat_dyn, driver_device, 0, ROT0, "Dynamo", "Tournament Solitaire (V1.06, 08/03/95)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1995, toursol1, toursol, pcat_dyn, pcat_dyn, driver_device, 0, ROT0, "Dynamo", "Tournament Solitaire (V1.04, 06/22/95)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
