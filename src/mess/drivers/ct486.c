// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    PC/AT 486 with Chips & Technologies CS4031 chipset

***************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/ram.h"
#include "machine/cs4031.h"
#include "machine/at_keybc.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "sound/speaker.h"


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

	virtual void machine_start();

	DECLARE_READ16_MEMBER( cs4031_ior );
	DECLARE_WRITE16_MEMBER( cs4031_iow );
	DECLARE_WRITE_LINE_MEMBER( cs4031_hold );
	DECLARE_WRITE8_MEMBER( cs4031_tc ) { m_isabus->eop_w(offset, data); }
	DECLARE_WRITE_LINE_MEMBER( cs4031_spkr ) { m_speaker->level_w(state); }
};


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ct486_state::machine_start()
{
}

READ16_MEMBER( ct486_state::cs4031_ior )
{
	if (offset < 4)
		return m_isabus->dack_r(offset);
	else
		return m_isabus->dack16_r(offset);
}

WRITE16_MEMBER( ct486_state::cs4031_iow )
{
	if (offset < 4)
		m_isabus->dack_w(offset, data);
	else
		m_isabus->dack16_w(offset, data);
}

WRITE_LINE_MEMBER( ct486_state::cs4031_hold )
{
	// halt cpu
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// and acknowledge hold
	m_cs4031->hlda_w(state);
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( ct486_map, AS_PROGRAM, 32, ct486_state )
ADDRESS_MAP_END

static ADDRESS_MAP_START( ct486_io, AS_IO, 32, ct486_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static MACHINE_CONFIG_START( ct486, ct486_state )
	MCFG_CPU_ADD("maincpu", I486, XTAL_25MHz)
	MCFG_CPU_PROGRAM_MAP(ct486_map)
	MCFG_CPU_IO_MAP(ct486_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("cs4031", cs4031_device, int_ack_r)

	MCFG_CS4031_ADD("cs4031", XTAL_25MHz, "maincpu", "isa", "bios", "keybc")
	// cpu connections
	MCFG_CS4031_HOLD(WRITELINE(ct486_state, cs4031_hold));
	MCFG_CS4031_NMI(INPUTLINE("maincpu", INPUT_LINE_NMI));
	MCFG_CS4031_INTR(INPUTLINE("maincpu", INPUT_LINE_IRQ0));
	MCFG_CS4031_CPURESET(INPUTLINE("maincpu", INPUT_LINE_RESET));
	MCFG_CS4031_A20M(INPUTLINE("maincpu", INPUT_LINE_A20));
	// isa dma
	MCFG_CS4031_IOR(READ16(ct486_state, cs4031_ior))
	MCFG_CS4031_IOW(WRITE16(ct486_state, cs4031_iow))
	MCFG_CS4031_TC(WRITE8(ct486_state, cs4031_tc))
	// speaker
	MCFG_CS4031_SPKR(WRITELINE(ct486_state, cs4031_spkr))

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("1M,2M,8M,16M,32M,64M")

	MCFG_DEVICE_ADD("keybc", AT_KEYBOARD_CONTROLLER, XTAL_12MHz)
	MCFG_AT_KEYBOARD_CONTROLLER_SYSTEM_RESET_CB(DEVWRITELINE("cs4031", cs4031_device, kbrst_w))
	MCFG_AT_KEYBOARD_CONTROLLER_GATE_A20_CB(DEVWRITELINE("cs4031", cs4031_device, gatea20_w))
	MCFG_AT_KEYBOARD_CONTROLLER_INPUT_BUFFER_FULL_CB(DEVWRITELINE("cs4031", cs4031_device, irq01_w))
	MCFG_AT_KEYBOARD_CONTROLLER_KEYBOARD_CLOCK_CB(DEVWRITELINE("pc_kbdc", pc_kbdc_device, clock_write_from_mb))
	MCFG_AT_KEYBOARD_CONTROLLER_KEYBOARD_DATA_CB(DEVWRITELINE("pc_kbdc", pc_kbdc_device, data_write_from_mb))
	MCFG_DEVICE_ADD("pc_kbdc", PC_KBDC, 0)
	MCFG_PC_KBDC_OUT_CLOCK_CB(DEVWRITELINE("keybc", at_keyboard_controller_device, keyboard_clock_w))
	MCFG_PC_KBDC_OUT_DATA_CB(DEVWRITELINE("keybc", at_keyboard_controller_device, keyboard_data_w))
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	MCFG_DEVICE_ADD("isabus", ISA16, 0)
	MCFG_ISA16_CPU(":maincpu")
	MCFG_ISA_BUS_IOCHCK(DEVWRITELINE("cs4031", cs4031_device, iochck_w))
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("cs4031", cs4031_device, irq09_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("cs4031", cs4031_device, irq03_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("cs4031", cs4031_device, irq04_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("cs4031", cs4031_device, irq05_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("cs4031", cs4031_device, irq06_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("cs4031", cs4031_device, irq07_w))
	MCFG_ISA_OUT_IRQ10_CB(DEVWRITELINE("cs4031", cs4031_device, irq10_w))
	MCFG_ISA_OUT_IRQ11_CB(DEVWRITELINE("cs4031", cs4031_device, irq11_w))
	MCFG_ISA_OUT_IRQ12_CB(DEVWRITELINE("cs4031", cs4031_device, irq12_w))
	MCFG_ISA_OUT_IRQ14_CB(DEVWRITELINE("cs4031", cs4031_device, irq14_w))
	MCFG_ISA_OUT_IRQ15_CB(DEVWRITELINE("cs4031", cs4031_device, irq15_w))
	MCFG_ISA_OUT_DRQ0_CB(DEVWRITELINE("cs4031", cs4031_device, dreq0_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE("cs4031", cs4031_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE("cs4031", cs4031_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE("cs4031", cs4031_device, dreq3_w))
	MCFG_ISA_OUT_DRQ5_CB(DEVWRITELINE("cs4031", cs4031_device, dreq5_w))
	MCFG_ISA_OUT_DRQ6_CB(DEVWRITELINE("cs4031", cs4031_device, dreq6_w))
	MCFG_ISA_OUT_DRQ7_CB(DEVWRITELINE("cs4031", cs4031_device, dreq7_w))
	MCFG_ISA16_SLOT_ADD("isabus", "board1", pc_isa16_cards, "fdcsmc", true)
	MCFG_ISA16_SLOT_ADD("isabus", "board2", pc_isa16_cards, "comat", true)
	MCFG_ISA16_SLOT_ADD("isabus", "board3", pc_isa16_cards, "ide", true)
	MCFG_ISA16_SLOT_ADD("isabus", "board4", pc_isa16_cards, "lpt", true)
	MCFG_ISA16_SLOT_ADD("isabus", "isa1", pc_isa16_cards, "svga_et4k", false)
	MCFG_ISA16_SLOT_ADD("isabus", "isa2", pc_isa16_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus", "isa3", pc_isa16_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus", "isa4", pc_isa16_cards, NULL, false)
	MCFG_ISA16_SLOT_ADD("isabus", "isa5", pc_isa16_cards, NULL, false)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// video hardware
	MCFG_PALETTE_ADD("palette", 256) // todo: really needed?

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("pc_disk_list","ibm5150")
	MCFG_SOFTWARE_LIST_ADD("xt_disk_list","ibm5160_flop")
	MCFG_SOFTWARE_LIST_ADD("at_disk_list","ibm5170")
	MCFG_SOFTWARE_LIST_ADD("at_cdrom_list","ibm5170_cdrom")
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ct486 )
	ROM_REGION(0x40000, "isa", ROMREGION_ERASEFF)
	ROM_REGION(0x100000, "bios", 0)
	ROM_LOAD("chips_1.ami", 0xf0000, 0x10000, CRC(a14a7511) SHA1(b88d09be66905ed2deddc26a6f8522e7d2d6f9a8))
ROM_END


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1993, ct486, 0, 0, ct486, 0, driver_device, 0, "<unknown>", "PC/AT 486 with CS4031 chipset", 0 )
