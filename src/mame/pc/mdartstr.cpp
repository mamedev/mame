// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Medalist Spectrum HW

https://www.youtube.com/watch?v=-kxk8UtTeIM

TODO:
- error 033 (?)
- Emulate 65535 (S)VGA, same as IBM PC-110;
- ROM disk, in ISA space;
- Sound, from parallel ports?
- (very eventually) requires a dartboard layout;

===================================================================================================

Spectrum Avanti (Medalist Dart Star)
Processor: Intel 386SX (25MHz) (NG80386SX25)

Chipset: CHIPS F82C836 (Single Chip 386sx / SCATsx)

System RAM: 2 x Alliance AS4C256K16F0-50JC (1 MB) [640K Main, 384K Extended]

Video controller: CHIPS F65535/A
VRAM: Alliance AS4C256K16F0-50JC (512 KB)

Additional chips:
2 x NEC D71055C
IDT 7202
Analog Devices AD7224KN
Samsung K6T1008C2E-DL70 near mem roms (mixed ROM/RAM disk?)

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
//#include "bus/isa/isa_cards.h"
#include "cpu/i386/i386.h"
#include "machine/at_keybc.h"
#include "machine/f82c836.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "video/pc_vga_chips.h"

#include "speaker.h"


/*
 *
 * ISA16 VGA bindings
 *
 */

class isa16_f65535_device :
		public device_t,
		public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_f65535_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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


DEFINE_DEVICE_TYPE(ISA16_F65535, isa16_f65535_device, "f65535_isa16", "CT-65535 Integrated VGA card")

isa16_f65535_device::isa16_f65535_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ISA16_F65535, tag, owner, clock),
	device_isa16_card_interface(mconfig, *this),
	m_vga(*this, "vga")
{
}

void isa16_f65535_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 524, 0, 480);
	screen.set_screen_update("vga", FUNC(f65535_vga_device::screen_update));

	F65535_VGA(config, m_vga, 0);
	m_vga->set_screen("screen");
	m_vga->set_vram_size(512*1024);
}

void isa16_f65535_device::io_isa_map(address_map &map)
{
	map(0x03b0, 0x03df).m(m_vga, FUNC(f65535_vga_device::io_map));
}

void isa16_f65535_device::device_start()
{
	set_isa_device();
}

void isa16_f65535_device::device_reset()
{
	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

void isa16_f65535_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		m_isa->install_memory(0xa0000, 0xbffff, read8sm_delegate(*m_vga, FUNC(f65535_vga_device::mem_r)), write8sm_delegate(*m_vga, FUNC(f65535_vga_device::mem_w)));
	}
	else if (space_id == AS_IO)
		m_isa->install_device(0x0000, 0xffff, *this, &isa16_f65535_device::io_isa_map);
}



namespace {

class mdartstr_state : public driver_device
{
public:
	mdartstr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chipset(*this, "chipset")
		, m_isabus(*this, "isabus")
		, m_speaker(*this, "speaker")
	{ }

	void mdartstr(machine_config &config);

private:
	required_device<i386sx_device> m_maincpu;
	required_device<f82c836a_device> m_chipset;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	void main_io(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};

void mdartstr_state::main_map(address_map &map)
{
//  map(0x000000, 0x09ffff).ram();
//  map(0x0a0000, 0x0bffff).rw("vga", FUNC(vga_device::mem_r), FUNC(vga_device::mem_w));
//  map(0x0c0000, 0x0c7fff).rom().region("bios", 0x48000); // VGA BIOS + virtual floppy ISA
//  map(0x0c8000, 0x0cffff).rom().region("bios", 0x18000);
//  map(0x0e0000, 0x0fffff).rom().region("bios", 0x20000);
//  map(0x100000, 0x15ffff).ram();
	// TODO: fc0000-fdffff has an optional memory hole in chipset at $4e bit 4
	map(0xfc0000, 0xffffff).rom().region("bios", 0x00000);
}

void mdartstr_state::main_io(address_map &map)
{
//  map(0x03b0, 0x03df).m("vga", FUNC(vga_device::io_map));
}

static INPUT_PORTS_START( mdartstr )
INPUT_PORTS_END

static void pc_isa_onboard(device_slot_interface &device)
{
	device.option_add_internal("vga", ISA16_F65535);
}

void mdartstr_state::mdartstr(machine_config &config)
{
	I386SX(config, m_maincpu, 25'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &mdartstr_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &mdartstr_state::main_io);
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

	// 640 + 384 KB
	RAM(config, "ram").set_default_size("1024K");

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

	ISA16_SLOT(config, "board1", 0, "isabus", pc_isa_onboard, "vga", true);

	at_kbc_device_base &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", XTAL(12'000'000)));
	keybc.hot_res().set(m_chipset, FUNC(f82c836a_device::kbrst_w));
	keybc.gate_a20().set(m_chipset, FUNC(f82c836a_device::gatea20_w));
	keybc.kbd_irq().set(m_chipset, FUNC(f82c836a_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	// g80_1500 works with this, ms_naturl don't
	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, nullptr));
	pc_kbdc.out_clock_cb().set(keybc, FUNC(at_kbc_device_base::kbd_clk_w));
	pc_kbdc.out_data_cb().set(keybc, FUNC(at_kbc_device_base::kbd_data_w));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
}

ROM_START( mdartstr )
	ROM_REGION16_LE( 0x80000, "rawbios", 0 )
	// 0xxxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD( "system rev 1.0.bin", 0x000000, 0x080000, CRC(cdd36a31) SHA1(4ced7065e0923d9cb414b65d2a2d955da080c46b) )

	ROM_REGION16_LE( 0x40000, "bios", ROMREGION_ERASEFF )
	ROM_COPY( "rawbios", 0x40000, 0x00000, 0x40000 )

	ROM_REGION16_LE( 0x800000, "romdisk", ROMREGION_ERASEFF )
	// TODO: actual socket position filename .ext
	// TODO: verify actual loading
	ROM_LOAD("mem_0 rev 3.25.bin",  0x000000, 0x100000,  CRC(8fa930f1) SHA1(a898b180678086c730dc059f14ddd34a334625c7) )
	ROM_LOAD("mem_1 rev 3.25.bin",  0x100000, 0x100000, CRC(8145bd30) SHA1(70d6a1f7e2ca63431396fd923b6d7d2bdabd56e8) )
	// empty socket mem 2
	// empty socket mem 3
	ROM_LOAD("mem_4 rev 2.0.bin",   0x400000, 0x100000, CRC(61adadd7) SHA1(b1705626e0c47ab213f85d74bc5148c0013e0da9) )
	ROM_LOAD("english rev 3.1.bin", 0x500000, 0x100000, CRC(72ed547b) SHA1(57b80dda3996cd75398c06e4bde92491d4d99c14) ) // mem 5 socket
	// empty socket mem 6
	// static RAM in mem 7
ROM_END

} // anonymous namespace

GAME( 2001, mdartstr, 0, mdartstr, mdartstr, mdartstr_state, empty_init, ROT0, "Medalist Marketing", "Medalist Dart Star (Rev 3.25)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
