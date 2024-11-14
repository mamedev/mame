// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    http://www.sprow.co.uk/bbc/minib.htm

    The board comes with the following features:
    - 6502A processor, at 2MHz (slowed to 1MHz during i/o accesses)
    - 6522 versatile interface adapter for timers and interruptible latched i/o ports
    - Interface for a 20x4 LCD display
    - 32kbyte high speed SRAM
    - 128kbyte in circuit programmable flash ROM (16k for main operating system, 112k for other ROM based applications)
    - PS/2 keyboard interface
    - Real time clock (year 2000 compliant), on a shared I²C bus
    - Full implementation of the BBC micro's 1MHz expansion bus for add on boards
    - Power supply monitor/reset device

    TODO:
    - fix keyboard

******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"

// slot devices
#include "bus/bbc/1mhzbus/1mhzbus.h"
#include "bus/bbc/1mhzbus/24bbc.h"
#include "bus/bbc/1mhzbus/2ndserial.h"
#include "bus/bbc/1mhzbus/ide.h"
#include "bus/bbc/userport/userport.h"
#include "bus/bbc/userport/lcd.h"


namespace {

class minib_state : public driver_device
{
public:
	minib_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
		, m_kbd(*this, "kbd")
	{ }

	void minib(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m6502_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<pc_kbdc_device> m_kbd;

	void mem_map(address_map &map) ATTR_COLD;

	//uint8_t pa_r();
	void pa_w(uint8_t data);
};


void minib_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	map(0x8000, 0xbfff).bankr(m_rombank);
	map(0xc000, 0xffff).rom().region("flash", 0x1c000);
	map(0xfc00, 0xfcff).rw("1mhzbus", FUNC(bbc_1mhzbus_slot_device::fred_r), FUNC(bbc_1mhzbus_slot_device::fred_w));
	map(0xfd00, 0xfdff).rw("1mhzbus", FUNC(bbc_1mhzbus_slot_device::jim_r), FUNC(bbc_1mhzbus_slot_device::jim_w));
	map(0xfe30, 0xfe3f).lw8(NAME([this] (u8 data) { m_rombank->set_entry(data & 0x07); }));
	map(0xfe60, 0xfe6f).m("via", FUNC(via6522_device::map));
}


void minib_state::machine_start()
{
	m_rombank->configure_entries(0, 8, memregion("flash")->base(), 0x4000);
}

void minib_state::machine_reset()
{
	m_rombank->set_entry(0);
}


//uint8_t minib_state::pa_r()
//{
//  uint8_t data = 0x00;
//
//  data |= m_kbd->data_signal() << 1;
//
//  return data;
//}

void minib_state::pa_w(uint8_t data)
{
	// PA0 = SDA drive*
	// PA1 = PS/2 data sense
	// PA2 = SDA sense
	// PA3 = SCL
	// PA4 = PS/2 clock out*
	// PA5 = PS/2 data out*
	// CA1 = PS/2 clock sense
	// CA2, PA6, PA7, are spare and just go to a header

	logerror("pa_w: clk %d data %d\n", BIT(data, 4), BIT(data, 5));
	m_kbd->data_write_from_mb(!BIT(data, 4));
	m_kbd->clock_write_from_mb(!BIT(data, 5));
}


static void minib_1mhzbus_devices(device_slot_interface &device)
{
	device.option_add("24bbc", BBC_24BBC);         // Sprow 24bBC/RAM Disc
	device.option_add("2ndserial", BBC_2NDSERIAL); // Sprow 2nd Serial Port
	device.option_add("beebide", BBC_BEEBIDE);     // Sprow BeebIDE 16-bit
}

static void minib_userport_devices(device_slot_interface &device)
{
	device.option_add("lcd", BBC_LCD);             // Sprow LCD Display
}


void minib_state::minib(machine_config &config)
{
	M6502(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &minib_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "irqs").output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	via6522_device &via(MOS6522(config, "via", 16_MHz_XTAL / 8));
	via.readpb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_r));
	via.writepb_handler().set("userport", FUNC(bbc_userport_slot_device::pb_w));
	via.cb1_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb1));
	via.cb2_handler().set("userport", FUNC(bbc_userport_slot_device::write_cb2));
	//via.readpa_handler().set(FUNC(minib_state::pa_r));
	via.writepa_handler().set(FUNC(minib_state::pa_w));
	via.irq_handler().set("irqs", FUNC(input_merger_device::in_w<0>));

	PC_KBDC(config, m_kbd, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_kbd->out_data_cb().set("via", FUNC(via6522_device::write_pa1));
	m_kbd->out_clock_cb().set("via", FUNC(via6522_device::write_ca1));

	bbc_userport_slot_device &userport(BBC_USERPORT_SLOT(config, "userport", minib_userport_devices, "lcd"));
	userport.cb1_handler().set("via", FUNC(via6522_device::write_cb1));
	userport.cb2_handler().set("via", FUNC(via6522_device::write_cb2));

	bbc_1mhzbus_slot_device &bus(BBC_1MHZBUS_SLOT(config, "1mhzbus", 16_MHz_XTAL / 16, minib_1mhzbus_devices, nullptr));
	bus.irq_handler().set("irqs", FUNC(input_merger_device::in_w<1>));
	bus.nmi_handler().set_inputline(m_maincpu, M6502_NMI_LINE);
}


ROM_START(minib)
	ROM_REGION(0x20000, "flash", ROMREGION_ERASEFF)
	ROM_LOAD("os041.rom",  0x1d000, 0x3000, CRC(57a24945) SHA1(5e521850953be33b9b29995e5f043bbbb45c60e5)) // OS 0.41
	ROM_LOAD("gos015.rom", 0x18000, 0x4000, CRC(e35a93df) SHA1(c91286aec2a2e086cb55ca3e0af576132144831f)) // GOS 0.15
	ROM_LOAD("basic2.rom", 0x14000, 0x4000, CRC(79434781) SHA1(4a7393f3a45ea309f744441c16723e2ef447a281)) // BASIC 2
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT         COMPANY        FULLNAME           FLAGS
COMP (2002, minib,    0,      0,      minib,   0,      minib_state,  empty_init,  "Sprow",       "MiniB Computer",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
