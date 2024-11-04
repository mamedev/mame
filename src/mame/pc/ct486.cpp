// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    PC/AT 486 with Chips & Technologies CS4031 chipset

***************************************************************************/

#include "emu.h"

#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/cs4031.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"

#include "emupal.h"
#include "softlist.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ct486_state : public driver_device
{
public:
	ct486_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cs4031(*this, "cs4031"),
		m_isabus(*this, "isabus"),
		m_speaker(*this, "speaker")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cs4031_device> m_cs4031;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	virtual void machine_start() override ATTR_COLD;

	uint16_t cs4031_ior(offs_t offset);
	void cs4031_iow(offs_t offset, uint16_t data);
	void cs4031_hold(int state);
	void cs4031_tc(offs_t offset, uint8_t data) { m_isabus->eop_w(offset, data); }
	void cs4031_spkr(int state) { m_speaker->level_w(state); }
	void ct486(machine_config &config);
	void ct486_io(address_map &map) ATTR_COLD;
	void ct486_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ct486_state::machine_start()
{
}

uint16_t ct486_state::cs4031_ior(offs_t offset)
{
	if (offset < 4)
		return m_isabus->dack_r(offset);
	else
		return m_isabus->dack16_r(offset);
}

void ct486_state::cs4031_iow(offs_t offset, uint16_t data)
{
	if (offset < 4)
		m_isabus->dack_w(offset, data);
	else
		m_isabus->dack16_w(offset, data);
}

void ct486_state::cs4031_hold(int state)
{
	// halt cpu
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// and acknowledge hold
	m_cs4031->hlda_w(state);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ct486_state::ct486_map(address_map &map)
{
}

void ct486_state::ct486_io(address_map &map)
{
	map.unmap_value_high();
}


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void ct486_state::ct486(machine_config &config)
{
	I486(config, m_maincpu, XTAL(25'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &ct486_state::ct486_map);
	m_maincpu->set_addrmap(AS_IO, &ct486_state::ct486_io);
	m_maincpu->set_irq_acknowledge_callback("cs4031", FUNC(cs4031_device::int_ack_r));

	CS4031(config, m_cs4031, XTAL(25'000'000), "maincpu", "isa", "bios", "keybc", RAM_TAG);
	// cpu connections
	m_cs4031->hold().set(FUNC(ct486_state::cs4031_hold));
	m_cs4031->nmi().set_inputline("maincpu", INPUT_LINE_NMI);
	m_cs4031->intr().set_inputline("maincpu", INPUT_LINE_IRQ0);
	m_cs4031->cpureset().set_inputline("maincpu", INPUT_LINE_RESET);
	m_cs4031->a20m().set_inputline("maincpu", INPUT_LINE_A20);
	// isa dma
	m_cs4031->ior().set(FUNC(ct486_state::cs4031_ior));
	m_cs4031->iow().set(FUNC(ct486_state::cs4031_iow));
	m_cs4031->tc().set(FUNC(ct486_state::cs4031_tc));
	// speaker
	m_cs4031->spkr().set(FUNC(ct486_state::cs4031_spkr));

	RAM(config, RAM_TAG).set_default_size("4M").set_extra_options("1M,2M,8M,16M,32M,64M");

	at_kbc_device_base &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	keybc.hot_res().set("cs4031", FUNC(cs4031_device::kbrst_w));
	keybc.gate_a20().set("cs4031", FUNC(cs4031_device::gatea20_w));
	keybc.kbd_irq().set("cs4031", FUNC(cs4031_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace(m_maincpu, AS_PROGRAM);
	m_isabus->set_iospace(m_maincpu, AS_IO);
	m_isabus->iochck_callback().set(m_cs4031, FUNC(cs4031_device::iochck_w));
	m_isabus->irq2_callback().set(m_cs4031, FUNC(cs4031_device::irq09_w));
	m_isabus->irq3_callback().set(m_cs4031, FUNC(cs4031_device::irq03_w));
	m_isabus->irq4_callback().set(m_cs4031, FUNC(cs4031_device::irq04_w));
	m_isabus->irq5_callback().set(m_cs4031, FUNC(cs4031_device::irq05_w));
	m_isabus->irq6_callback().set(m_cs4031, FUNC(cs4031_device::irq06_w));
	m_isabus->irq7_callback().set(m_cs4031, FUNC(cs4031_device::irq07_w));
	m_isabus->irq10_callback().set(m_cs4031, FUNC(cs4031_device::irq10_w));
	m_isabus->irq11_callback().set(m_cs4031, FUNC(cs4031_device::irq11_w));
	m_isabus->irq12_callback().set(m_cs4031, FUNC(cs4031_device::irq12_w));
	m_isabus->irq14_callback().set(m_cs4031, FUNC(cs4031_device::irq14_w));
	m_isabus->irq15_callback().set(m_cs4031, FUNC(cs4031_device::irq15_w));
	m_isabus->drq0_callback().set(m_cs4031, FUNC(cs4031_device::dreq0_w));
	m_isabus->drq1_callback().set(m_cs4031, FUNC(cs4031_device::dreq1_w));
	m_isabus->drq2_callback().set(m_cs4031, FUNC(cs4031_device::dreq2_w));
	m_isabus->drq3_callback().set(m_cs4031, FUNC(cs4031_device::dreq3_w));
	m_isabus->drq5_callback().set(m_cs4031, FUNC(cs4031_device::dreq5_w));
	m_isabus->drq6_callback().set(m_cs4031, FUNC(cs4031_device::dreq6_w));
	m_isabus->drq7_callback().set(m_cs4031, FUNC(cs4031_device::dreq7_w));
	ISA16_SLOT(config, "board1", 0, "isabus", pc_isa16_cards, "fdcsmc", true);
	ISA16_SLOT(config, "board2", 0, "isabus", pc_isa16_cards, "comat", true);
	ISA16_SLOT(config, "board3", 0, "isabus", pc_isa16_cards, "ide", true);
	ISA16_SLOT(config, "board4", 0, "isabus", pc_isa16_cards, "lpt", true);
	ISA16_SLOT(config, "isa1", 0, "isabus", pc_isa16_cards, "svga_et4k", false);
	ISA16_SLOT(config, "isa2", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa3", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa4", 0, "isabus", pc_isa16_cards, nullptr, false);
	ISA16_SLOT(config, "isa5", 0, "isabus", pc_isa16_cards, nullptr, false);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* software lists */
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ct486 )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x100000, "bios", 0)
	ROM_LOAD("chips_1.ami", 0xf0000, 0x10000, CRC(a14a7511) SHA1(b88d09be66905ed2deddc26a6f8522e7d2d6f9a8))
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1993, ct486, 0, 0, ct486, 0, ct486_state, empty_init, "<unknown>", "PC/AT 486 with CS4031 chipset", 0 )
