// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
Hardware to run VJ Windows PC side software

Motherboard is similar to EP-5BVPXB but says REV 1.3 which matches NMC-5VXC
but the board used does not have the NMC markings like the NMC-5VXC.

VIA 82C586B PCI ISA/IDE bridge
VIA 82C585VPX
Winbond W83877F IO Core Logic

S3 Virge/DX Q5C2BB (not connected to a monitor)

UPS is connected to COM1 port

CD-ROM: Mitsumi CRMC-FX3210S(?)
Floppy: Unidentified but machine has floppy drive


TODO: None of the VIA chips are emulated in the new PCI code
Windows 95 will ask you to install the new hardware every boot. There's an autoexec.bat that'll
restore the registry and other configs back to the factory setup every boot so
the hardware will never stay "installed" between boots.

Keyboard and mouse should not enabled by default but it'd be impossible to install drivers currently.
It's recommended to disable the keyboard (Input Settings > Keyboard Selection > AT Keyboard > Disabled)
when actually playing the games because otherwise you'll be sending inputs to the Windows PC in the background.
*/

#include "emu.h"
#include "jaleco_vj_pc.h"

#include "jaleco_vj_ups.h"

#include "bus/isa/isa_cards.h"
#include "bus/pci/virge_pci.h"
#include "bus/rs232/rs232.h"
#include "machine/fdc37c93x.h"
#include "machine/i82371sb.h"
#include "machine/i82439hx.h"
#include "machine/i82439tx.h"
#include "machine/pci-ide.h"
#include "machine/pci.h"


jaleco_vj_pc_device::jaleco_vj_pc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, JALECO_VJ_PC, tag, owner, clock),
	device_mixer_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_king_qtaro(*this, "pci:08.0"),
	m_sound(*this, "isa1:vj_sound"),
	m_is_steppingstage(false)
{
}

void jaleco_vj_pc_device::device_start()
{
}

static void isa_internal_devices(device_slot_interface &device)
{
	device.option_add("fdc37c93x", FDC37C93X);
}

static void isa_com(device_slot_interface &device)
{
	// TODO: Stepping Stage uses a different UPS but will still boot unlike VJ without a valid UPS attached
	device.option_add("vj_ups", JALECO_VJ_UPS);
}

static void isa_cards(device_slot_interface &device)
{
	device.option_add("vj_sound", JALECO_VJ_ISA16_SOUND);
}

void jaleco_vj_pc_device::superio_config(device_t &device)
{
	// TODO: This should be a Winbond W83877F Super I/O
	fdc37c93x_device &fdc = downcast<fdc37c93x_device &>(device);
	fdc.set_sysopt_pin(1);
	fdc.gp20_reset().set_inputline(m_maincpu, INPUT_LINE_RESET);
	fdc.gp25_gatea20().set_inputline(m_maincpu, INPUT_LINE_A20);
	fdc.irq1().set(*this, "pci:07.0", FUNC(i82371sb_isa_device::pc_irq1_w));
	fdc.irq8().set(*this, "pci:07.0", FUNC(i82371sb_isa_device::pc_irq8n_w));
	fdc.txd1().set(*this, "serport0", FUNC(rs232_port_device::write_txd));
	fdc.ndtr1().set(*this, "serport0", FUNC(rs232_port_device::write_dtr));
	fdc.nrts1().set(*this, "serport0", FUNC(rs232_port_device::write_rts));
	fdc.txd2().set(*this, "serport1", FUNC(rs232_port_device::write_txd));
	fdc.ndtr2().set(*this, "serport1", FUNC(rs232_port_device::write_dtr));
	fdc.nrts2().set(*this, "serport1", FUNC(rs232_port_device::write_rts));
}

void jaleco_vj_pc_device::sound_config(device_t &device)
{
	jaleco_vj_isa16_sound_device &sound = downcast<jaleco_vj_isa16_sound_device &>(device);
	sound.set_steppingstage_mode(m_is_steppingstage);
	sound.add_route(0, *this, 1.0, 0);
	sound.add_route(1, *this, 1.0, 1);
}

void jaleco_vj_pc_device::boot_state_w(uint8_t data)
{
	logerror("Boot state %02x\n", data);
}

void jaleco_vj_pc_device::device_add_mconfig(machine_config &config)
{
	// PCI config IDs pulled from Windows registry
	PENTIUM(config, m_maincpu, 20'000'000); // TODO: Pentium 60/90mhz? Underclocked for performance reasons
	m_maincpu->set_irq_acknowledge_callback("pci:07.0:pic8259_master", FUNC(pic8259_device::inta_cb));
	m_maincpu->smiact().set("pci:00.0", FUNC(i82439hx_host_device::smi_act_w));

	PCI_ROOT(config, "pci", 0);
	I82439HX(config, "pci:00.0", 0, m_maincpu, 256*1024*1024); // TODO: Should be 0x05851106 VIA VT82C585 Apollo VP,VPX,VPX-97 System Controller

	i82371sb_isa_device &isa(I82371SB_ISA(config, "pci:07.0", 0, m_maincpu));
	// isa.set_ids(0x05861106, 0x23, 0x060100, 0x00000000); // TODO: Should be VIA VT82C586B, PCI-to-ISA Bridge
	isa.boot_state_hook().set(FUNC(jaleco_vj_pc_device::boot_state_w));
	isa.smi().set_inputline(m_maincpu, INPUT_LINE_SMI);

	i82371sb_ide_device &ide(I82371SB_IDE(config, "pci:07.1", 0, m_maincpu));
	// ide.set_ids(0x05711106, 0x06, 0x01018a, 0x00000000); // TODO: Should be VIA VT82C586B, IDE Controller
	ide.irq_pri().set("pci:07.0", FUNC(i82371sb_isa_device::pc_irq14_w));
	ide.irq_sec().set("pci:07.0", FUNC(i82371sb_isa_device::pc_mirq0_w));

	// TODO: pci:07.3 0x30401106 VIA VT83C572, VT86C586/A/B Power Management Controller

	JALECO_VJ_KING_QTARO(config, m_king_qtaro, 0);

	// TODO: Should actually be pci:0a.0 but it only shows a black screen
	PCI_SLOT(config, "pci:2", pci_cards, 16, 1, 2, 3, 0, "virgedx").set_fixed(true);

	ISA16_SLOT(config, "board4", 0, "pci:07.0:isabus", isa_internal_devices, "fdc37c93x", true).set_option_machine_config("fdc37c93x", [this] (device_t *device) { superio_config(*device); });
	ISA16_SLOT(config, "isa1", 0, "pci:07.0:isabus", isa_cards, "vj_sound", true).set_option_machine_config("vj_sound", [this] (device_t *device) { sound_config(*device); });
	ISA16_SLOT(config, "isa2", 0, "pci:07.0:isabus", isa_cards, nullptr, true);
	ISA16_SLOT(config, "isa3", 0, "pci:07.0:isabus", isa_cards, nullptr, true);

	rs232_port_device& serport0(RS232_PORT(config, "serport0", isa_com, "vj_ups"));
	serport0.rxd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::rxd1_w));
	serport0.dcd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndcd1_w));
	serport0.dsr_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndsr1_w));
	serport0.ri_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::nri1_w));
	serport0.cts_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ncts1_w));

	rs232_port_device &serport1(RS232_PORT(config, "serport1", isa_com, nullptr));
	serport1.rxd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::rxd2_w));
	serport1.dcd_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndcd2_w));
	serport1.dsr_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ndsr2_w));
	serport1.ri_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::nri2_w));
	serport1.cts_handler().set("board4:fdc37c93x", FUNC(fdc37c93x_device::ncts2_w));
}

ROM_START(jaleco_vj_pc)
	// TODO: BIOS undumped. Seems like it should be an Award BIOS which won't boot currently.
	// Placeholder BIOS taken from pcipc
	ROM_REGION32_LE(0x40000, "pci:07.0", 0) // PC bios
	ROM_SYSTEM_BIOS(0, "m55ns04", "m55ns04") // Micronics M55HI-Plus with no sound
	ROMX_LOAD("m55-04ns.rom", 0x20000, 0x20000, CRC(0116b2b0) SHA1(19b0203decfd4396695334517488d488aec3ccde) BAD_DUMP, ROM_BIOS(0))
ROM_END

const tiny_rom_entry *jaleco_vj_pc_device::device_rom_region() const
{
	return ROM_NAME(jaleco_vj_pc);
}

DEFINE_DEVICE_TYPE(JALECO_VJ_PC, jaleco_vj_pc_device, "jaleco_vj_pc", "Jaleco VJ PC")
