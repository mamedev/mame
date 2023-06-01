// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Taito Wolf System

Driver by Ville Linde

Three board system consisting of a P5TX-LA PC motherboard, a Taito main board and a rom board.

TODO:
- Implement proper Super I/O (crashes on CMOS and keyboard checks);
- pf2012: Recheck following statement with the new BIOS dump, may be just standard ISA hook;
\- program ROM is read via parallel port (for offset write, encrypted) and game port.
   It's the first thing that the BIOS does at boot (cfr. accesses at $20x),
   if these ports are fed with proper values then it sets up PnP then tries a DMA ch. 3 transfer,
   otherwise it just boots the normal P5TX-LA bootstrap sequence.
   cfr. PC=e850b, PC=e4fc8, PC=fd84a (reading I/O $0006).
- Above needs to be converted to a proper EISA device, program ROM board is connected on MB thru the only
  available slot option;
- Hookup Voodoo to PCI root;
- Convert P5TX-LA to a proper stand-alone driver;
- According to manual hold A+B+service button for 10 seconds for entering test mode during initial bootup
  sequence. Board and input test menu doesn't seem to have a dedicated test mode switch. This statement needs
  verification once we get there;

Hardware configuration:

P5TX-LA Motherboard:
-CPU: Intel SL27J Pentium MMX @ 200 MHz
-Onboard sound: Crystal CS4237B ISA Audio
-Onboard VGA: ATI Rage II 3D Graph (removed from motherboard)

Chipsets (430TX PCIset):
-82439TX Northbridge
-82371EB PIIX4 PCI-ISA Southbridge

Taito W Main Board:
-AMD M4-128N/64 CPLD stamped 'E58-01'
-AMD MACH231 CPLD stamped 'E58-02'
-AMD MACH211 CPLD stamped 'E58-03'
-Panasonic MN1020019 (MN10200 based) Sound CPU
-Zoom ZFX-2 DSP (TMS57002 DSP)
-Zoom ZSG-2 Sound PCM chip
-Taito TC0510NIO I/O chip
-1x RAM NEC 42S4260
-1x RAM GM71C4400
-12x RAM Alliance AS4C256K16E0-35 (256k x 16)
-Mitsubishi M66220 256 x 8-bit CMOS memory
-Fujitsu MB87078 6-bit, 4-channel Electronic Volume Controller
-Atmel 93C66 EEPROM (4kb probably for high scores, test mode settings etc)
-ICS5342-3 GENDAC 16-Bit Integrated Clock-LUT-DAC
-3DFX 500-0003-03 F805281.1 FBI
-3DFX 500-0004-02 F804701.1 TMU
-Rom: E58-04 (bootscreen)
-XTALs 50MHz (near 3DFX) and 14.31818MHz (near RAMDAC)

Taito W Rom Board:
-AMD M4-128N/64 CPLD stamped 'E58-05'
-Program, Sound roms

*/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "cpu/i386/i386.h"
#include "machine/w83977tf.h"
#include "machine/i82371sb.h"
#include "machine/i82439hx.h"
#include "machine/i82439tx.h"
#include "machine/pci-ide.h"
#include "machine/pci.h"
#include "video/virge_pci.h"


namespace {

class p5txla_state : public driver_device
{
public:
	p5txla_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
	{ }

    void p5txla(machine_config &config);
	void taitowlf(machine_config &config);

private:

	void p5txla_io(address_map &map);
	void p5txla_map(address_map &map);

	static void winbond_superio_config(device_t *device);
};

void p5txla_state::p5txla_map(address_map &map)
{
	map.unmap_value_high();
}

void p5txla_state::p5txla_io(address_map &map)
{
	map.unmap_value_high();
}

/*****************************************************************************/

static void isa_internal_devices(device_slot_interface &device)
{
    // TODO: w83877tf
	device.option_add("w83977tf", W83977TF);
}

void p5txla_state::winbond_superio_config(device_t *device)
{
	w83977tf_device &fdc = *downcast<w83977tf_device *>(device);
//	fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(":maincpu", INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(":maincpu", INPUT_LINE_A20);
	fdc.irq1().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
	fdc.irq8().set(":pci:07.0", FUNC(i82371sb_isa_device::pc_irq8n_w));
//	fdc.txd1().set(":serport0", FUNC(rs232_port_device::write_txd));
//	fdc.ndtr1().set(":serport0", FUNC(rs232_port_device::write_dtr));
//	fdc.nrts1().set(":serport0", FUNC(rs232_port_device::write_rts));
//	fdc.txd2().set(":serport1", FUNC(rs232_port_device::write_txd));
//	fdc.ndtr2().set(":serport1", FUNC(rs232_port_device::write_dtr));
//	fdc.nrts2().set(":serport1", FUNC(rs232_port_device::write_rts));
}


void p5txla_state::p5txla(machine_config &config)
{
	pentium_device &maincpu(PENTIUM(config, "maincpu", 90000000));
	maincpu.set_addrmap(AS_PROGRAM, &p5txla_state::p5txla_map);
	maincpu.set_addrmap(AS_IO, &p5txla_state::p5txla_io);
	maincpu.set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
//	maincpu.smiact().set("pci:00.0", FUNC(i82439tx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
    // 64MB for Taito Wolf HW, to be checked for base p5txla
	I82439TX(config, "pci:00.0", 0, "maincpu", 64*1024*1024);

	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, "maincpu"));
	isa.boot_state_hook().set([](u8 data) { /* printf("%02x\n", data); */ });
	isa.smi().set_inputline("maincpu", INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, "pci:07.1", 0, "maincpu"));
	ide.irq_pri().set("pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371sb_isa_device::pc_mirq0_w));

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "w83977tf", true).set_option_machine_config("w83977tf", winbond_superio_config);
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "pci:07.0:isabus", pc_isa16_cards, nullptr, false);

#if 0
	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, nullptr)); // "microsoft_mouse"));
	serport0.rxd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::rxd1_w));
	serport0.dcd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndcd1_w));
	serport0.dsr_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndsr1_w));
	serport0.ri_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::nri1_w));
	serport0.cts_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::rxd2_w));
	serport1.dcd_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndcd2_w));
	serport1.dsr_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ndsr2_w));
	serport1.ri_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::nri2_w));
	serport1.cts_handler().set("board4:w83977tf", FUNC(fdc37c93x_device::ncts2_w));
#endif

    // TODO: ATI Rage II+DVD
    VIRGE_PCI(config, "pci:12.0", 0);
}

void p5txla_state::taitowlf(machine_config &config)
{
    p5txla_state::p5txla(config);

    // TODO: replace ATI Rage above with the 2x Voodoo setup
}

/*****************************************************************************/

ROM_START(p5txla)
	ROM_REGION32_LE(0x40000, "pci:07.0", 0)
	ROM_LOAD("p5tx-la.bin", 0x00000, 0x40000, CRC(072e6d51) SHA1(70414349b37e478fc28ecbaba47ad1033ae583b7))

ROM_END

ROM_START(pf2012)
	ROM_REGION32_LE(0x40000, "pci:07.0", ROMREGION_ERASEFF)
    // TAITO Ver1.0 1998/5/7
	ROM_LOAD("p5tx-la_861.u16", 0x20000, 0x20000, CRC(a4d4a0fc) SHA1(af3a49a1bee416b58a61af28473f3dac0a4160c8))

	ROM_REGION(0x400000, "user3", 0) // Program ROM (FAT12)
	ROM_LOAD("u1.bin", 0x000000, 0x200000, CRC(8f4c09cb) SHA1(0969a92fec819868881683c580f9e01cbedf4ad2))
	ROM_LOAD("u2.bin", 0x200000, 0x200000, CRC(59881781) SHA1(85ff074ab2a922eac37cf96f0bf153a2dac55aa4))

	ROM_REGION(0x4000000, "user4", 0) // Data ROM (FAT12)
	ROM_LOAD("e59-01.u20", 0x0000000, 0x800000, CRC(701d3a9a) SHA1(34c9f34f4da34bb8eed85a4efd1d9eea47a21d77) )
	ROM_LOAD("e59-02.u23", 0x0800000, 0x800000, CRC(626df682) SHA1(35bb4f91201734ce7ccdc640a75030aaca3d1151) )
	ROM_LOAD("e59-03.u26", 0x1000000, 0x800000, CRC(74e4efde) SHA1(630235c2e4a11f615b5f3b8c93e1e645da09eefe) )
	ROM_LOAD("e59-04.u21", 0x1800000, 0x800000, CRC(c900e8df) SHA1(93c06b8f5082e33f0dcc41f1be6a79283de16c40) )
	ROM_LOAD("e59-05.u24", 0x2000000, 0x800000, CRC(85b0954c) SHA1(1b533d5888d56d1510c79f790e4fa708f77e836f) )
	ROM_LOAD("e59-06.u27", 0x2800000, 0x800000, CRC(0573a113) SHA1(ee76a71dfd31289a9a5428653a36d01d914fc5d9) )
	ROM_LOAD("e59-07.u22", 0x3000000, 0x800000, CRC(1f0ddcdc) SHA1(72ffe08f5effab093bdfe9863f8a11f80e914272) )
	ROM_LOAD("e59-08.u25", 0x3800000, 0x800000, CRC(8db38ffd) SHA1(4b71ea86fb774ba6a8ac45abf4191af64af007e7) )

	ROM_REGION(0x1400000, "samples", 0) // ZOOM sample data
	ROM_LOAD("e59-09.u29", 0x0000000, 0x800000, CRC(d0da5c50) SHA1(56fb3c38f35244720d32a44fed28e6b58c7851f7) )
	ROM_LOAD("e59-10.u32", 0x0800000, 0x800000, CRC(4c0e0a5c) SHA1(6454befa3a1dd532eb2a760129dcd7e611508730) )
	ROM_LOAD("e59-11.u33", 0x1000000, 0x400000, CRC(c90a896d) SHA1(2b62992f20e4ca9634e7953fe2c553906de44f04) )

	ROM_REGION(0x180000, "cpu1", 0) // MN10200 program
	ROM_LOAD("e59-12.u13", 0x000000, 0x80000, CRC(9a473a7e) SHA1(b0ec7b0ae2b33a32da98899aa79d44e8e318ceb7) )
	ROM_LOAD("e59-13.u15", 0x080000, 0x80000, CRC(77719880) SHA1(8382dd2dfb0dae60a3831ed6d3ff08539e2d94eb) )
	ROM_LOAD("e59-14.u14", 0x100000, 0x40000, CRC(d440887c) SHA1(d965871860d757bc9111e9adb2303a633c662d6b) )
	ROM_LOAD("e59-15.u16", 0x140000, 0x40000, CRC(eae8e523) SHA1(8a054d3ded7248a7906c4f0bec755ddce53e2023) )

	ROM_REGION(0x20000, "bootscreen", 0) // bootscreen
	ROM_LOAD("e58-04.u71", 0x000000, 0x20000, CRC(500e6113) SHA1(93226706517c02e336f96bdf9443785158e7becf) )
ROM_END

} // Anonymous namespace


/*****************************************************************************/

COMP(1997, p5txla, 0,   0, p5txla, 0,   p5txla_state, empty_init, "ECS",    "P5TX-LA (i430TX)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

GAME(1998, pf2012, 0,   taitowlf, 0, p5txla_state, empty_init, ROT0, "Taito",  "Psychic Force 2012", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
