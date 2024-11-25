// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
// thanks-to:Phill Harvey-Smith
/***************************************************************************

    CMS 6502 Development System

    This is an industrial 6502 system in a rack chassis consisting of a
    back-plane and plug-in cards, with two 3.5" floppy disk drives, a
    "SIMON" System Monitor, DFS, and an extended version of BBC BASIC
    called MULTI-BASIC. It has an IBM-XT style keyboard.

    The "Eurocard" standard format cards are as follows:
    - 6502 2nd Processor For The BBC Micro (CMS0026 6502)
    - 40/80 Terminal Card (also handles the keyboard) (CMS 0010-3)
    - High Performance Colour Graphics Card (CMS Video-4)
    - Versatile Interface Board (CMS0006 Issue 4)
    - Analogue Digital Interface Board (CMS00092 A/D I/O)
    - Floppy Disk Controller

    System I/O Allocation
      &FA00-&FBFF Advanced graphics card
      &FC00-&FC0F CPU card VIA
      &FC10-&FC1F GDP card control register
      &FC20-&FC2F GDP card colour register
      &FC30-&FC3F Real-Time-Clock registers (EM M3002)
      &FC40-&FC4F Disc controller registers
      &FC50-&FC5F Disc controller drive select register
      &FC60-&FC6F IEEE controller card
      &FC70-&FC7F Memory select register (on CPU card)
      &FC80-&FC8F VIA1 on VIB card
      &FC90-&FC9F VIA2 on VIB card
      &FCA0-&FCAF VIA3 on VIB card
      &FCB0-&FCBF VIA4 on VIB card (Printer interface)
      &FCC0-&FCCF Analogue card ADC
      &FCD0-&FCDF Analogue card DAC
      &FCE0-&FCEF Analogue card VIA
      &FCF0-&FCFF Serial Interface on VIB card
      &FD00-&FD1F Reserved
      &FD20-&FD2F 40/80 column card video processor
      &FD30-&FD3F 40/80 column card VIA (printer interface)
      &FD40-&FD4F 40/80 column card serial interface
      &FD50-&FD5F Reserved for future I/O cards

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "machine/m3002.h"
#include "bus/acorn/bus.h"


namespace {

class cms_state : public driver_device
{
public:
	cms_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "rom")
		, m_bank1(*this, "bank1")
		, m_via(*this, "via")
		, m_irqs(*this, "irqs")
		, m_bus(*this, "bus")
		, m_map_select(0)
	{ }

	void cms6502(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void map_select_w(uint8_t data);
	void page_select_w(uint8_t data);
	void bus_nmi_w(int state);

	required_device<cpu_device> m_maincpu;
	required_memory_region m_rom;
	required_memory_bank m_bank1;
	required_device<via6522_device> m_via;
	required_device<input_merger_device> m_irqs;
	required_device<acorn_bus_device> m_bus;

	void cms6502_mem(address_map &map) ATTR_COLD;

	uint8_t m_map_select;
	uint8_t m_page_select[4];
};


void cms_state::cms6502_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();  /* socket M1 43256C-12 32K RAM */
	map(0x8000, 0xbfff).bankr("bank1").w(FUNC(cms_state::page_select_w));
	map(0xc000, 0xffff).rom().region("mos", 0);
	map(0xfc00, 0xfc0f).m(m_via, FUNC(via6522_device::map));
	map(0xfc30, 0xfc30).mirror(0xf).rw("rtc", FUNC(m3002_device::read), FUNC(m3002_device::write));
	map(0xfc70, 0xfc7f).w(FUNC(cms_state::map_select_w));
}


void cms_state::map_select_w( uint8_t data)
{
	m_map_select = data & 0x03;
	m_bank1->set_entry((m_map_select << 2) | m_page_select[m_map_select]);
}

void cms_state::page_select_w(uint8_t data)
{
	/* 27513 in socket M4 (map 0) */
	if (m_map_select == 0x00)
	{
		m_page_select[m_map_select] = data & 0x03;
	}
	m_bank1->set_entry((m_map_select << 2) | m_page_select[m_map_select]);
}


/* Input ports */
static INPUT_PORTS_START( cms )
INPUT_PORTS_END


void cms_state::machine_start()
{
	m_bank1->configure_entries(0, 16, m_rom->base(), 0x4000);

	/* register for save states */
	save_item(NAME(m_map_select));
	save_item(NAME(m_page_select));
}


void cms_state::machine_reset()
{
	m_map_select = 0x00;
	for (int i = 0; i < 4; i++)
	{
		m_page_select[i] = 0x00;
	}
	m_bank1->set_entry(0);
}


void cms_state::bus_nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void cms_state::cms6502(machine_config &config)
{
	M6502(config, m_maincpu, 1_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cms_state::cms6502_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MOS6522(config, m_via, 1_MHz_XTAL);
	m_via->irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	M3002(config, "rtc", 32.768_kHz_XTAL);

	/* 7 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->set_space(m_maincpu, AS_PROGRAM);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set(FUNC(cms_state::bus_nmi_w));
	ACORN_BUS_SLOT(config, "bus1", m_bus, cms_bus_devices, "4080term");
	ACORN_BUS_SLOT(config, "bus2", m_bus, cms_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus3", m_bus, cms_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus4", m_bus, cms_bus_devices, "hires");
	ACORN_BUS_SLOT(config, "bus5", m_bus, cms_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus6", m_bus, cms_bus_devices, nullptr);
}


/***************************************************************************
    ROM definitions
***************************************************************************/

ROM_START( cms6502 )
	ROM_REGION(0x04000, "mos", 0) /* 27128 */
	ROM_LOAD("6502 system os.m5", 0x0000, 0x4000, CRC(1b6da63d) SHA1(5c0afc55288ad86e1d6e653dda9c5dcccdaf0ff2))

	ROM_REGION(0x40000, "rom", 0)
	/* Socket M4 27513 */
	ROM_LOAD("basic.m4",       0x00000, 0x4000, CRC(e27b6146) SHA1(ae89ee695bed6f49402f009ded16ad3a36f64c28))
	ROM_LOAD("dfs-1.4.m4",     0x04000, 0x4000, CRC(0743ddb5) SHA1(cd61956a10510a6954cba9ba17f70c2991e81d9b))
	ROM_LOAD("multi-basic.m4", 0x08000, 0x4000, CRC(fb785563) SHA1(9f97582ba0b82f314008bf95eacb00e7bced3cac))
	ROM_LOAD("simon-2.02.m4",  0x0c000, 0x4000, CRC(d1f1b633) SHA1(991c3691a70f45fff3c828c8c0a0906b9f5e34fc))
	/* Socket M3 empty */
	/* Socket M2 27128 */
	ROM_LOAD("gdp.m2",         0x20000, 0x4000, CRC(f545293b) SHA1(532c45ac4661643c6c6f633d504ef72e800cc900))
	/* Socket M1 contains RAM, not mapped as ROM */

	ROM_REGION(0x200, "proms", 0)
	ROM_LOAD("bassys2.ic8", 0x0000, 0x0200, CRC(417ff0b4) SHA1(878718accb83f18456fefaf67a0e4a6a407113e4)) // 512x8
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                             FULLNAME                       FLAGS */
COMP( 1986, cms6502, 0,      0,      cms6502, cms,   cms_state, empty_init, "Cambridge Microprocessor Systems", "CMS 6502 Development System", MACHINE_NO_SOUND_HW )
