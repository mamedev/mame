// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

UMC UM8498F/8496 based IBM compatibles

TODO:
- Port over remaining targets from pc/at.cpp
- tku5s throws "Incompatible Keyboard Controller" in BIOS color set submenu.
  May require specific AMIKEY BIOS there?

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/um8498f.h"
#include "sound/spkrdev.h"
#include "video/pc_vga.h"

#include "screen.h"
#include "speaker.h"


namespace {

class pc486vl_state : public driver_device
{
public:
	pc486vl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chipset(*this, "chipset")
		, m_isabus(*this, "isabus")
		, m_speaker(*this, "speaker")
	{ }

	void pccm912(machine_config &config);
	void pc486vl(machine_config &config);

private:
	required_device<i486_device> m_maincpu;
	required_device<um8498f_device> m_chipset;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	void base_config(machine_config &config);

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void pc486vl_state::main_map(address_map &map)
{
	map(0x00100000, 0x03ffffff).noprw();
//	map(0x02000000, 0xfffeffff).noprw();
	map(0xfffe0000, 0xffffffff).rom().region("bios", 0);
}

void pc486vl_state::main_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00e0, 0x00e3).nopw(); // timestamp stuff?
}

void pc486vl_state::base_config(machine_config &config)
{
	/* basic machine hardware */
	I486(config, m_maincpu, XTAL(40'000'000)); // um486sxlc-40
	m_maincpu->set_addrmap(AS_PROGRAM, &pc486vl_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &pc486vl_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("chipset", FUNC(um8498f_device::int_ack_r));

	UM8498F(config, m_chipset, XTAL(25'000'000), "maincpu", "bios", "keybc", "ram", "isabus");
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

	// TODO: Chipset can go up to 32M, but will go "memory fail" with that (?)
	RAM(config, "ram").set_default_size("64M");

	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->iochck_callback().set(m_chipset, FUNC(um8498f_device::iochck_w));
	m_isabus->irq2_callback().set(m_chipset, FUNC(um8498f_device::irq09_w));
	m_isabus->irq3_callback().set(m_chipset, FUNC(um8498f_device::irq03_w));
	m_isabus->irq4_callback().set(m_chipset, FUNC(um8498f_device::irq04_w));
	m_isabus->irq5_callback().set(m_chipset, FUNC(um8498f_device::irq05_w));
	m_isabus->irq6_callback().set(m_chipset, FUNC(um8498f_device::irq06_w));
	m_isabus->irq7_callback().set(m_chipset, FUNC(um8498f_device::irq07_w));
	m_isabus->irq10_callback().set(m_chipset, FUNC(um8498f_device::irq10_w));
	m_isabus->irq11_callback().set(m_chipset, FUNC(um8498f_device::irq11_w));
	m_isabus->irq12_callback().set(m_chipset, FUNC(um8498f_device::irq12_w));
	m_isabus->irq14_callback().set(m_chipset, FUNC(um8498f_device::irq14_w));
	m_isabus->irq15_callback().set(m_chipset, FUNC(um8498f_device::irq15_w));
	m_isabus->drq0_callback().set(m_chipset, FUNC(um8498f_device::dreq0_w));
	m_isabus->drq1_callback().set(m_chipset, FUNC(um8498f_device::dreq1_w));
	m_isabus->drq2_callback().set(m_chipset, FUNC(um8498f_device::dreq2_w));
	m_isabus->drq3_callback().set(m_chipset, FUNC(um8498f_device::dreq3_w));
	m_isabus->drq5_callback().set(m_chipset, FUNC(um8498f_device::dreq5_w));
	m_isabus->drq6_callback().set(m_chipset, FUNC(um8498f_device::dreq6_w));
	m_isabus->drq7_callback().set(m_chipset, FUNC(um8498f_device::dreq7_w));

	at_kbc_device_base &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	keybc.hot_res().set(m_chipset, FUNC(um8498f_device::kbrst_w));
	keybc.gate_a20().set(m_chipset, FUNC(um8498f_device::gatea20_w));
	keybc.kbd_irq().set(m_chipset, FUNC(um8498f_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, "ms_naturl"));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

void pc486vl_state::pccm912(machine_config &config)
{
	base_config(config);
	// TODO: actually 4 + 3 VLB
	ISA16_SLOT(config, "isa1", 0, "isabus", pc_isa16_cards, "vga", false);
	ISA16_SLOT(config, "isa2", 0, "isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa4", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa6", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa7", 0, "isabus", pc_isa16_cards, nullptr, false);
}

void pc486vl_state::pc486vl(machine_config &config)
{
	base_config(config);
	ISA16_SLOT(config, "isa1", 0, "isabus", pc_isa16_cards, "vga", false);
	ISA16_SLOT(config, "isa2", 0, "isabus", pc_isa16_cards, "fdc", false);
	ISA16_SLOT(config, "isa3", 0, "isabus", pc_isa16_cards, "ide", false);
	ISA16_SLOT(config, "isa4", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa6", 0, "isabus", pc_isa8_cards,  nullptr, false);
}

// PC-Chips M912 - Chipset: UM8498F, UM8496F - CPU: 486 - BIOS: AMI - ISA16: 4, ISA16/VL: 3
ROM_START( pccm912 )
	ROM_REGION32_LE( 0x20000, "bios", 0)
	// 0:
	ROM_SYSTEM_BIOS( 0, "072594", "07/25/94")
	ROMX_LOAD( "m912.bin", 0x10000, 0x10000, CRC(7784aaf5) SHA1(f54935c5da12160251104d0273040fea22ccbc70), ROM_BIOS(0))
	// 1:
	ROM_SYSTEM_BIOS( 1, "120295", "12/02/95")
	ROMX_LOAD( "m912_12-02-1995x.bin", 0x10000, 0x10000, CRC(28a4a140) SHA1(a58989ab5ad5d040ad4f25888c5b7d77f31a4d82), ROM_BIOS(1))

	// BIOS-String: 40-P101-001437-00101111-072594-GREEN-H - CPU: Socket 3 - RAM: 4xSIMM30, 2xSIMM72, Cache: 9xUM61256AK-15
	ROM_SYSTEM_BIOS( 2, "ami_120194a", "12/01/1994A")
	ROMX_LOAD( "4umm001.bin", 0x10000, 0x10000, CRC(a5b768b4) SHA1(904ce2814d6542b65acec0c84532946172f2296d), ROM_BIOS(2))
ROM_END

// ECS UA4985
// Soyo SY-019S (onboard U5SX)
ROM_START( sy019s )
	ROM_REGION32_LE(0x20000, "bios", 0)
	// Chipset: UM8498F + UM8496F - BIOS label: Award BIOS ISA 486 485427 - BIOS version: Award Modular BIOS v4.50G - CPU: UMC U55X 486-33F, solder pads for 80486socket
	// RAM: 4xSIMM30, 2xSIMM72 - ISA8: 1, ISA16: 5
	// BIOS-String: 12/08/94-UMC-498GP-2C4X6S21-00 / REV A1
	ROM_SYSTEM_BIOS(0, "498gp", "498GP")
	ROMX_LOAD( "award_bios_isa_486.bin", 0x10000, 0x10000, CRC(ce3ccaa4) SHA1(3fdc9282d9934e18e45b46b50644022fc0387f33), ROM_BIOS(0))
ROM_END

// Lucky Star UMC 498 U5
// TK U5S-TK-V03A
// PC Chips/Hsin Tech <unknown> chip16/18
// QDI I4U498GRN V1.0
// 40-P000-001453-00101111-072594-UMC498
ROM_START( tku5s )
	ROM_REGION32_LE(0x20000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "um849801", "UM8498 #1") // no display
	ROMX_LOAD( "um8498.ami", 0x10000, 0x10000, CRC(51f71bc7) SHA1(0986d60081d2c578a66789c0c53fe1d5919c553f),ROM_BIOS(0))
ROM_END

} // anonymous namespace

COMP( 1994, pccm912,   0, 0,       pccm912,     0,     pc486vl_state,     empty_init,        "PC-Chips", "M912 (UMC UM8498F & UM8496 chipset)", MACHINE_NOT_WORKING )
COMP( 199?, sy019s,    0, 0,       pc486vl,     0,     pc486vl_state,     empty_init,        "Soyo",     "SY-019S (UMC UM8498F & UM8496 chipset)", MACHINE_NOT_WORKING )
COMP( 1994, tku5s,     0, 0,       pc486vl,     0,     pc486vl_state,     empty_init,        "TK",       "U5S-TK-V03A (UMC UM8498F & UM8496 chipset)", MACHINE_NOT_WORKING )
