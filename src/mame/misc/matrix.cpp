// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/*
'Matrix' slot machine or poker game (the bezel has poker cards) by unidentified manufacturer
Game title is taken from ROM labels and cabinet. Might be incomplete.

TODO:
- KBDC, not from super I/O;
- Don't recognize an attached HDD, check default CMOS settings;
- loops at PC=eda2d, reads the NMI vector;
- game ROMs are encrypted (currently at least partially decrypted), may require a missing boot device;

Hardware consists of:

Motherboard (GXM-530D): (ETA: BIOS boots as SuperTek ST-MGXm3HB, is 530 actually referring to SiS530? -AS)
Cyrix MediaGX GXm-266GP 2.9V
Cyrix GXm Cx5530 with GCT bios
128MB RAM
SMC FDC37C931
5-dip bank

Daughter card (FLASH ROM SSD 374-525-627-33-78J54):
Lattice ispLSI 1032E 70LJ D980B06
Unpopulated spaces marked for: DS5002FP, PIC16C54, 93C56 EEPROM, a couple more unreadable
8-dip bank
6 ROMs
1 RAM
*/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/8042kbdc.h"
#include "machine/mc146818.h"
#include "machine/mediagx_cs5530_bridge.h"
#include "machine/mediagx_cs5530_ide.h"
#include "machine/mediagx_cs5530_video.h"
#include "machine/mediagx_host.h"
#include "machine/pci.h"
#include "machine/zfmicro_usb.h"

namespace {

class matrix_state : public driver_device
{
public:
	matrix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rtc(*this, "rtc")
		, m_kbdc(*this, "kbdc")
	{ }

	void matrix(machine_config &config);

	void init_decryption();

private:
	required_device<cpu_device> m_maincpu;
	required_device<ds1287_device> m_rtc;
	required_device<kbdc8042_device> m_kbdc;

	void main_map(address_map &map) ATTR_COLD;
};


void matrix_state::main_map(address_map &map)
{
}

static INPUT_PORTS_START( matrix )
INPUT_PORTS_END


void matrix_state::matrix(machine_config &config)
{
	MEDIAGX(config, m_maincpu, 233'000'000); // Cyrix MediaGX GXm-266GP
	m_maincpu->set_addrmap(AS_PROGRAM, &matrix_state::main_map);
	m_maincpu->set_irq_acknowledge_callback("pci:12.0:pic8259_master", FUNC(pic8259_device::inta_cb));

	// TODO: from FDC37C93x super I/O?
	// NOTE: both aren't initialized at $3f0 - $370 but accessed anyway, wtf
	DS1287(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->set_binary(true);
	m_rtc->set_epoch(1980);
	m_rtc->irq().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq8n_w));

	KBDC8042(config, m_kbdc, 0);
	// TODO: PS/2 mouse
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	m_kbdc->system_reset_callback().set_inputline(":maincpu", INPUT_LINE_RESET);
	m_kbdc->gate_a20_callback().set_inputline(":maincpu", INPUT_LINE_A20);
	m_kbdc->input_buffer_full_callback().set(":pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq1_w));
	m_kbdc->set_keyboard_tag("at_keyboard");

	at_keyboard_device &at_keyb(AT_KEYB(config, "at_keyboard", pc_keyboard_device::KEYBOARD_TYPE::AT, 1));
	at_keyb.keypress().set(m_kbdc, FUNC(kbdc8042_device::keyboard_w));

	PCI_ROOT(config, "pci", 0);
	MEDIAGX_HOST(config, "pci:00.0", 0, "maincpu", 128*1024*1024);
	// TODO: no clue about the ID used for this, definitely tested
	// Tries to initialize MediaGX F4 -> ISA -> PCI
	// May actually be a ZFMicro PCI Bridge (0x10780400)?
	PCI_BRIDGE(config, "pci:01.0", 0, 0x10780000, 0);

	// "pci:12.0" or "pci:10.0" depending on pin H26 (readable in bridge thru PCI index $44)
	mediagx_cs5530_bridge_device &isa(MEDIAGX_CS5530_BRIDGE(config, "pci:12.0", 0, "maincpu", "pci:12.2"));
	isa.set_kbdc_tag("kbdc");
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	//isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);
	isa.rtcale().set([this](u8 data) { m_rtc->address_w(data); });
	isa.rtccs_read().set([this]() { return m_rtc->data_r(); });
	isa.rtccs_write().set([this](u8 data) { m_rtc->data_w(data); });

	// "pci:12.1" SMI & ACPI

	mediagx_cs5530_ide_device &ide(MEDIAGX_CS5530_IDE(config, "pci:12.2", 0, "maincpu"));
	ide.irq_pri().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq14_w));
	ide.irq_sec().set("pci:12.0", FUNC(mediagx_cs5530_bridge_device::pc_irq15_w));

	// "pci:12.3" XpressAUDIO
	MEDIAGX_CS5530_VIDEO(config, "pci:12.4", 0);

	ZFMICRO_USB(config, "pci:13.0", 0);

	// 2 PCI slots, 2 ISA slots
	ISA16_SLOT(config, "isa1", 0, "pci:12.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:12.0:isabus", pc_isa16_cards, nullptr, false);
}


ROM_START( matrix )
	ROM_REGION32_LE(0x40000, "pci:12.0", 0)
	ROM_LOAD("d586_bios.bin", 0x00000, 0x40000, CRC(39fc093a) SHA1(3376bac4f0d6e729d5939e3078ecdf700464cba3) )

	ROM_REGION(0x300000, "unsorted", 0) // encrypted?
	ROM_LOAD( "matrix_031203u5.bin",  0x000000, 0x080000, CRC(95aa8fb7) SHA1(8cbfa783a887779350609d6f3ea1e88187bd21a4) )
	ROM_LOAD( "matrix_031203u6.bin",  0x080000, 0x080000, CRC(38822bc6) SHA1(b57bd9fa44cab9fa4cef8873454c8be0dc7ab781) )
	ROM_LOAD( "matrix_031203u7.bin",  0x100000, 0x080000, CRC(74d31f1a) SHA1(bf6eae262cab6d24276f43370f3b9e4f687b9a52) )
	ROM_LOAD( "matrix_031203u18.bin", 0x180000, 0x080000, CRC(7b20c6cb) SHA1(51d9a442c510a60f85d9ad7b56cfe67c60f4ab1b) )
	ROM_LOAD( "matrix_031203u19.bin", 0x200000, 0x080000, CRC(c612c80c) SHA1(ef7586369fd1f9c6b8f3e78806c3be16b5aa1a3d) )
	ROM_LOAD( "matrix_031203u20.bin", 0x280000, 0x080000, CRC(f87ac4ae) SHA1(ef9b730a1113d36ef6a041fe36d77edfa255ad98) )
ROM_END


void matrix_state::init_decryption() // at least enough to see strings from various programs like DOS-C, PMODE/W, UPX, etc
{
	uint8_t *rom = memregion("unsorted")->base();
	std::vector<uint8_t> buffer(0x300000);

	memcpy(&buffer[0], rom, 0x300000);


	for (int i = 0; i < 0x300000; i++)
	{
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 0, 9, 8, 7, 6, 5, 4, 3, 1, 10, 2)];
		rom[i] = bitswap<8>(rom[i] ^ 0xda, 7, 6, 5, 4, 1, 2, 0, 3);
	}
}

} // anonymous namespace


GAME( 200?, matrix, 0, matrix, matrix, matrix_state, init_decryption, ROT0, "<unknown>", "Matrix", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
