// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

IBM Palm Top PC-110

TODO:
- Skeleton-ish, needs SCAMP chipset and Super I/O to proceed;
- Hookup specific SW list (4 system disks) once this do anything worthwhile;
- Error 104 (protected mode, tries to r/w $0100'0000)
- Error 8081 (PCMCIA)
- Error 161 (CMOS)
- Error 2401 (video problem, r/w extended VGA regs)
- Error 301 (keyboard)
- Error 604 (floppy disk error)
- Error 163 (date and time not set/bad)
- I9990303 "not OK -> IBM" (may be related to above)

===================================================================================================

Components:
- Intel Flash ROM (unknown type);
- RIOS 89G6403 (Intel 486SX-33);
- RIOS 89G6402 (VLSI VL82C420 "SCAMP IV");
- SMC FDC37C665IR Super I/O;
- Chips and Technologies F55535 VGA chip, 512 KiB RAM;
- ESS488 AudioDrive;
- Ricoh RB5C396 (PCMCIA interface, Intel 82365 compatible?). Type-3 Slot, or two Type-1;
- Dallas DS1669S;
- TI TPS2201 (PC Card Power);
- LCD display for time and battery level;
- Modem board, tied to 128KiB SRAM (Samsung KM610000) and 29F040A-12 flash firmware (undumped). 2400bps Data, 9600bps FAX;
- Infra-red port;
- Maxim 786CAI (PSU Controller);
- 4 or 8 MiB RAM, can go up to 28 MiB thru patches. 1 expansion slot;
- 1 port replicator connector;
- Keyboard 89-key Compact JIS with Fn key;
- "Smart-Pico" Flash Card Slot;
- Headphone jack;

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/ram.h"
#include "machine/vl82c420.h"
#include "sound/spkrdev.h"
#include "video/pc_vga_chips.h"

#include "screen.h"
//#include "softlist.h"
//#include "softlist_dev.h"
#include "speaker.h"

/*
 *
 * ISA16 VGA bindings
 *
 */

class isa16_f65535_lcd_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_f65535_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

	void io_isa_map(address_map &map) ATTR_COLD;

private:
	required_device<f65535_vga_device> m_vga;
};


DEFINE_DEVICE_TYPE(ISA16_F65535_LCD, isa16_f65535_lcd_device, "f65535_lcd_isa16", "CT-65535 Integrated VGA card (LCD variant)")

isa16_f65535_lcd_device::isa16_f65535_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_F65535_LCD, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

void isa16_f65535_lcd_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(f65535_vga_device::screen_update));

	F65535_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(512*1024);
}

void isa16_f65535_lcd_device::io_isa_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(f65535_vga_device::io_map));
}

void isa16_f65535_lcd_device::device_start()
{
	set_isa_device();
}

void isa16_f65535_lcd_device::device_reset()
{
	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

void isa16_f65535_lcd_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(f65535_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(f65535_vga_device::mem_w)));
	}
	else if (space_id == AS_IO)
		m_isa->install_device(0x0000, 0xffff, *this, &isa16_f65535_lcd_device::io_isa_map);
}


namespace {

class ptpc110_state : public driver_device
{
public:
	ptpc110_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chipset(*this, "chipset")
		, m_isabus(*this, "isabus")
		, m_speaker(*this, "speaker")
	{}

	void ptpc110(machine_config &config);

private:
	required_device<i486_device> m_maincpu;
	required_device<vl82c420_device> m_chipset;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


void ptpc110_state::main_map(address_map &map)
{
	map(0x0000'0000, 0x0009'ffff).ram();
//	map(0x000a'0000, 0x000b'ffff).rw(m_vga, FUNC(f65535_vga_device::mem_r), FUNC(f65535_vga_device::mem_w));
//	map(0x000c'0000, 0x000f'ffff).rom().region("bios", 0);
	map(0xfffc'0000, 0xffff'ffff).rom().region("bios", 0);
}

void ptpc110_state::main_io(address_map &map)
{
//	map.unmap_value_high();
	map(0x0024, 0x0027).nopw(); // noisy on $26
//	map(0x03b0, 0x03df).m(m_vga, FUNC(f65535_vga_device::io_map));
//	map(0x15ec, 0x15ef) PCMCIA?
//	map(0x35e8, 0x35eb) ^
}

static void pc_isa_onboard(device_slot_interface &device)
{
	device.option_add_internal("vga",  ISA16_F65535_LCD);
	// TODO: everything else
}


void ptpc110_state::ptpc110(machine_config &config)
{
	const XTAL xtal = XTAL(33'000'000);
	I486(config, m_maincpu, xtal); // i486sx
	m_maincpu->set_addrmap(AS_PROGRAM, &ptpc110_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &ptpc110_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("chipset", FUNC(vl82c420_device::int_ack_r));

	// clock guessed
	VL82C420(config, m_chipset, xtal, "maincpu", "bios", "keybc", "ram", "isabus");
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

	RAM(config, "ram").set_default_size("4M").set_extra_options("8M");

	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace("maincpu", AS_PROGRAM);
	m_isabus->set_iospace("maincpu", AS_IO);
	m_isabus->iochck_callback().set(m_chipset, FUNC(vl82c420_device::iochck_w));
	m_isabus->irq2_callback().set(m_chipset, FUNC(vl82c420_device::irq09_w));
	m_isabus->irq3_callback().set(m_chipset, FUNC(vl82c420_device::irq03_w));
	m_isabus->irq4_callback().set(m_chipset, FUNC(vl82c420_device::irq04_w));
	m_isabus->irq5_callback().set(m_chipset, FUNC(vl82c420_device::irq05_w));
	m_isabus->irq6_callback().set(m_chipset, FUNC(vl82c420_device::irq06_w));
	m_isabus->irq7_callback().set(m_chipset, FUNC(vl82c420_device::irq07_w));
	m_isabus->irq10_callback().set(m_chipset, FUNC(vl82c420_device::irq10_w));
	m_isabus->irq11_callback().set(m_chipset, FUNC(vl82c420_device::irq11_w));
	m_isabus->irq12_callback().set(m_chipset, FUNC(vl82c420_device::irq12_w));
	m_isabus->irq14_callback().set(m_chipset, FUNC(vl82c420_device::irq14_w));
	m_isabus->irq15_callback().set(m_chipset, FUNC(vl82c420_device::irq15_w));
	m_isabus->drq0_callback().set(m_chipset, FUNC(vl82c420_device::dreq0_w));
	m_isabus->drq1_callback().set(m_chipset, FUNC(vl82c420_device::dreq1_w));
	m_isabus->drq2_callback().set(m_chipset, FUNC(vl82c420_device::dreq2_w));
	m_isabus->drq3_callback().set(m_chipset, FUNC(vl82c420_device::dreq3_w));
	m_isabus->drq5_callback().set(m_chipset, FUNC(vl82c420_device::dreq5_w));
	m_isabus->drq6_callback().set(m_chipset, FUNC(vl82c420_device::dreq6_w));
	m_isabus->drq7_callback().set(m_chipset, FUNC(vl82c420_device::dreq7_w));

	ISA16_SLOT(config, "board1", 0, "isabus", pc_isa_onboard, "vga", true);

	// TODO: should not fit
	at_kbc_device_base &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	keybc.hot_res().set(m_chipset, FUNC(vl82c420_device::kbrst_w));
	keybc.gate_a20().set(m_chipset, FUNC(vl82c420_device::gatea20_w));
	keybc.kbd_irq().set(m_chipset, FUNC(vl82c420_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, "ms_naturl"));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( ptpc110 )
	ROM_REGION32_LE( 0x40000, "bios", ROMREGION_ERASEFF )
	ROM_LOAD( "pc110-flash.bin", 0, 0x40000, CRC(d68bb7a4) SHA1(b5c075842b60accae06bc78ddbf6454d9127de4f) )
ROM_END

} // anonymous namespace

COMP( 1995, ptpc110, 0, 0, ptpc110, 0, ptpc110_state, empty_init, "International Business Machines", "Palm Top PC-110 (Japan)",  MACHINE_NOT_WORKING )
