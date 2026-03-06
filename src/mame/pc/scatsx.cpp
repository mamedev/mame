// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SCATsx based PCs

TODO:
- add EMS in chipset (has specific driver for MS-DOS);
- Several remaining sets;
- Check earlier SCAT chipset(s), consider moving here;

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/f82c836.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "softlist.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class scatsx_state : public driver_device
{
public:
	scatsx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chipset(*this, "chipset")
		, m_isabus(*this, "isabus")
		, m_speaker(*this, "speaker")
	{ }

	void mb1320(machine_config &config);
	void scsxaio(machine_config &config);

protected:
	void base_config(machine_config &config);

private:
	required_device<i386sx_device> m_maincpu;
	required_device<f82c836a_device> m_chipset;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

void scatsx_state::main_map(address_map &map)
{
	// TODO: fc0000-fdffff has an optional memory hole in chipset at $4e bit 4
	map(0xfc0000, 0xffffff).rom().region("bios", 0x00000);
}

void scatsx_state::main_io(address_map &map)
{
}

void scatsx_state::base_config(machine_config &config)
{
	// FSB 16 MHz, 20 MHz or 25 MHz
	I386SX(config, m_maincpu, 25'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &scatsx_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &scatsx_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("chipset", FUNC(f82c836a_device::int_ack_r));

	F82C836A(config, m_chipset, XTAL(25'000'000), "maincpu", "bios", "keybc", "ram", "isabus");
	m_chipset->hold().set([this] (int state) {
		// halt cpu
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

		// and acknowledge hold
		m_chipset->hlda_w(state);
	});
	m_chipset->nmi().set_inputline("maincpu", INPUT_LINE_NMI);
	m_chipset->intr().set_inputline("maincpu", INPUT_LINE_IRQ0);
	m_chipset->cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	m_chipset->a20m().set_inputline("maincpu", INPUT_LINE_A20);
	// isa dma
	m_chipset->ior().set([this] (offs_t offset) -> u16 {
		if (offset < 4)
			return m_isabus->dack_r(offset);
		else
			return m_isabus->dack16_r(offset);
	});
	m_chipset->iow().set([this] (offs_t offset, u16 data) {
		if (offset < 4)
			m_isabus->dack_w(offset, data);
		else
			m_isabus->dack16_w(offset, data);
	});
	m_chipset->tc().set([this] (offs_t offset, u8 data) { m_isabus->eop_w(offset, data); });
	// speaker
	m_chipset->spkr().set([this] (int state) { m_speaker->level_w(state); });

	// TODO: 16 MB max, tbd
	RAM(config, "ram").set_default_size("1M").set_extra_options("2M,4M,8M");

	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->iochck_callback().set(m_chipset, FUNC(f82c836a_device::iochck_w));
	m_isabus->irq2_callback().set(m_chipset, FUNC(f82c836a_device::irq09_w));
	m_isabus->irq3_callback().set(m_chipset, FUNC(f82c836a_device::irq03_w));
	m_isabus->irq4_callback().set(m_chipset, FUNC(f82c836a_device::irq04_w));
	m_isabus->irq5_callback().set(m_chipset, FUNC(f82c836a_device::irq05_w));
	m_isabus->irq6_callback().set(m_chipset, FUNC(f82c836a_device::irq06_w));
	m_isabus->irq7_callback().set(m_chipset, FUNC(f82c836a_device::irq07_w));
	m_isabus->irq10_callback().set(m_chipset, FUNC(f82c836a_device::irq10_w));
	m_isabus->irq11_callback().set(m_chipset, FUNC(f82c836a_device::irq11_w));
	m_isabus->irq12_callback().set(m_chipset, FUNC(f82c836a_device::irq12_w));
	m_isabus->irq14_callback().set(m_chipset, FUNC(f82c836a_device::irq14_w));
	m_isabus->irq15_callback().set(m_chipset, FUNC(f82c836a_device::irq15_w));
	m_isabus->drq0_callback().set(m_chipset, FUNC(f82c836a_device::dreq0_w));
	m_isabus->drq1_callback().set(m_chipset, FUNC(f82c836a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_chipset, FUNC(f82c836a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_chipset, FUNC(f82c836a_device::dreq3_w));
	m_isabus->drq5_callback().set(m_chipset, FUNC(f82c836a_device::dreq5_w));
	m_isabus->drq6_callback().set(m_chipset, FUNC(f82c836a_device::dreq6_w));
	m_isabus->drq7_callback().set(m_chipset, FUNC(f82c836a_device::dreq7_w));

	at_kbc_device_base &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	keybc.hot_res().set(m_chipset, FUNC(f82c836a_device::kbrst_w));
	keybc.gate_a20().set(m_chipset, FUNC(f82c836a_device::gatea20_w));
	keybc.kbd_irq().set(m_chipset, FUNC(f82c836a_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, "ms_naturl"));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "win_cdrom_list").set_original("generic_cdrom").set_filter("ibmpc");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}

void scatsx_state::mb1320(machine_config &config)
{
	base_config(config);
	ISA16_SLOT(config, "isa1", 0, "isabus", pc_isa16_cards, "vga", false);
	ISA16_SLOT(config, "isa2", 0, "isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa4", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa6", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa7", 0, "isabus", pc_isa16_cards, nullptr, false);
}

void scatsx_state::scsxaio(machine_config &config)
{
	// TODO: Acer M5105, COM / LPT / IDE ports actually as connectors on MB
	base_config(config);
	ISA16_SLOT(config, "isa1", 0, "isabus", pc_isa16_cards, "vga", false);
	ISA16_SLOT(config, "isa2", 0, "isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa4", 0, "isabus", pc_isa16_cards, "lpt", false);
	ISA16_SLOT(config, "isa5", 0, "isabus", pc_isa16_cards, "com", false);
	ISA16_SLOT(config, "isa6", 0, "isabus", pc_isa16_cards, nullptr, false);
}



ROM_START( mb1320 )
	ROM_REGION16_LE( 0x40000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "amd386.bin", 0x030000, 0x010000, CRC(7c5045af) SHA1(64efea383b2a5beb16586d5fd67eced83068c616) )
ROM_END

// SCsxAIO - Chipset: Chips 82C236 (SCATsx), Acer M5105 A3E - On board: 2xCOM, Floppy, ISA
// BIOS-String: Peacock 386sx Ver. 2.0 24.03.92 30-0000-D01131-00101111-070791-SCATsx-0 - ISA16: 6
ROM_START( scsxaio )
	ROM_REGION16_LE(0x40000, "bios", 0)
	ROM_LOAD( "ami_1131.bin", 0x20000, 0x10000, NO_DUMP ) // keyboard BIOS
	ROM_LOAD( "386-peacock_scsxaio.bin", 0x30000, 0x10000, CRC(54c3cacd) SHA1(b3c821b30052d0c771b5004a3746eb2cfd186c79))
ROM_END

} // anonymous namespace

COMP( 1990, mb1320,    0, 0,    mb1320,    0, scatsx_state, empty_init, "Biostar",                 "MB-1320/25C-B.5 (SCATsx chipset)", MACHINE_NOT_WORKING )
COMP( 1991, scsxaio,   0, 0,    scsxaio,   0, scatsx_state, empty_init, "ECS / Peacock Computer",  "SCsxAIO (SCATsx chipset)", MACHINE_NOT_WORKING )
