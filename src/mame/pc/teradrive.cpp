// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Sega Teradrive

IBM PS/2 Model 30 (PS/55 5510Z) + Japanese Sega Mega Drive (TMSS MD1 VA3 or VA4)

References:
- https://www.retrodev.com/blastem/trac/wiki/TeradriveHardwareNotes
- https://plutiedev.com/cartridge-slot
- https://plutiedev.com/mirror/teradrive-hardware-info
- https://github.com/RetroSwimAU/TeradriveCode
- https://www.youtube.com/watch?v=yjg3gmTo4WA

NOTES (PC side):
- F1 at POST will bring a setup menu;

NOTES (MD side):
- 16 KiB of Z80 RAM (vs. 8 of stock)
- 128 KiB of VDP RAM (vs. 64)
- 68k can switch between native 7.67 MHz or 10 MHz
- has discrete YM3438 in place of YM2612

TODO:
- keyboard issues on Sega menu (hold arrow and press enter to go to floppy loading);
- "TIMER FAIL" when exiting from setup menu (keyboard?);
- RAM size always gets detected as 2560K;
- Quadtel EMM driver fails recognizing WD76C10 chipset with j4.0 driver disk;
- Cannot HDD format with floppy insthdd.bat, cannot boot from HDD (needs floppy first).
  Attached disk is a WDL-330PS with no geometry info available;
- MD side, as a testbed for rewriting base HW;

**************************************************************************************************/

#include "emu.h"

#include "bus/isa/isa_cards.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "cpu/i86/i286.h"
//#include "cpu/i386/i386.h"
#include "machine/at.h"
#include "machine/ram.h"
#include "machine/wd7600.h"

#include "softlist_dev.h"
#include "speaker.h"

/*
 * ISA16 IBM 79F2661 "bus switch"
 *
 * Motherboard resource also shared with undumped 5510Z Japanese DOS/V
 *
 */

class isa16_ibm_79f2661 : public device_t, public device_isa16_card_interface
{
public:
	// construction/destruction
	isa16_ibm_79f2661(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_romdisk_tag(T &&tag) { m_romdisk.set_tag(std::forward<T>(tag)); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_romdisk;
	memory_bank_creator m_rom_window_bank;

	void io_map(address_map &map) ATTR_COLD;

	void remap(int space_id, offs_t start, offs_t end) override;

	u8 m_rom_bank = 0;
	u8 m_rom_address = 0x0e;
	u8 m_reg_1163 = 0;
	u8 m_reg_1164 = 0;
	u16 m_68k_address = 0;
};

DEFINE_DEVICE_TYPE(ISA16_IBM_79F2661, isa16_ibm_79f2661, "isa16_ibm_79f2661", "ISA16 IBM 79F2661 \"bus switch\"")

isa16_ibm_79f2661::isa16_ibm_79f2661(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_IBM_79F2661, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_romdisk(*this, finder_base::DUMMY_TAG)
	, m_rom_window_bank(*this, "rom_window_bank")
{
}

void isa16_ibm_79f2661::device_start()
{
	set_isa_device();
	m_rom_window_bank->configure_entries(0, 0x100, m_romdisk->base(), 0x2000);
}

void isa16_ibm_79f2661::device_reset()
{
	m_rom_bank = 0;
	m_rom_address = 0x0e;
	m_reg_1163 = 0;
	m_reg_1164 = 0;
	m_68k_address = 0;
	remap(AS_PROGRAM, 0, 0xfffff);
	remap(AS_IO, 0, 0xffff);
}

void isa16_ibm_79f2661::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_PROGRAM)
	{
		u32 base_addr = (m_rom_address << 12) | 0xc0000;
		//printf("%08x %02x\n", base_addr, m_rom_bank);
		m_isa->install_bank(base_addr, base_addr | 0x1fff, m_rom_window_bank);
	}
	if (space_id == AS_IO)
	{
		m_isa->install_device(0x1160, 0x1167, *this, &isa16_ibm_79f2661::io_map);
	}
}

// +$1160 base
//	map(0x1160, 0x1160) romdisk bank * 0x2000, r/w
//	map(0x1161, 0x1161) <undefined>?
//	map(0x1162, 0x1162) romdisk segment start in ISA space (val & 0x1e | 0xc0)
//	map(0x1163, 0x1163) comms and misc handshake bits, partially reflected on 68k $ae0001 register
//	map(0x1164, 0x1164) boot control
//	map(0x1165, 0x1165) switches/bus timeout/TMSS unlock (r/o)
//	map(0x1166, 0x1167) 68k address space select & 0xffffe * 0x2000, r/w
void isa16_ibm_79f2661::io_map(address_map &map)
{
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) {
			return m_rom_bank;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_rom_bank = data;
			m_rom_window_bank->set_entry(m_rom_bank);
			remap(AS_PROGRAM, 0, 0xfffff);
		})
	);
	map(0x01, 0x01).lr8(
		NAME([this] (offs_t offset) {
			logerror("%s: $1161 undefined access\n", machine().describe_context());
			return 0xff;
		})
	);

	map(0x02, 0x02).lrw8(
		NAME([this] (offs_t offset) {
			return m_rom_address | 0xc0;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_rom_address = data & 0x1e;
			remap(AS_PROGRAM, 0, 0xfffff);
		})
	);
	map(0x03, 0x03).lrw8(
		NAME([this] (offs_t offset) {
			return m_reg_1163;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_reg_1163 = data;
		})
	);
	map(0x04, 0x04).lrw8(
		NAME([this] (offs_t offset) {
			return m_reg_1164;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_reg_1164 = data;
		})
	);
/*
 * xx-- ---- <unknown, other bus error?>
 * --x- ---- TMSS unlocked
 * ---- x--- bus timeout on 68k space access from 286
 * ---- -x-- video switch (0) "video" (1) RGB
 * ---- ---x MD/PC switch (0) MD boot (1) PC boot
 */
	map(0x05, 0x05).lr8(
		NAME([] (offs_t offset) {
			return 1 | 4;
		})
	);
	map(0x06, 0x07).lrw16(
		NAME([this] (offs_t offset) {
			return m_68k_address;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			COMBINE_DATA(&m_68k_address);
			m_68k_address &= 0xffffe;
		})
	);
}


namespace {

class teradrive_state : public driver_device
{
public:
	teradrive_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_x86cpu(*this, "x86cpu")
		, m_chipset(*this, "chipset")
		, m_ram(*this, RAM_TAG)
		, m_isabus(*this, "isabus")
		, m_speaker(*this, "speaker")
	{ }

	void teradrive(machine_config &config);
	void at_softlists(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;

	void x86_io(address_map &map) ATTR_COLD;
	void x86_map(address_map &map) ATTR_COLD;
private:
	required_device<i80286_cpu_device> m_x86cpu;
	required_device<wd7600_device> m_chipset;
	required_device<ram_device> m_ram;
	required_device<isa16_device> m_isabus;
	required_device<speaker_sound_device> m_speaker;

	u16 wd7600_ior(offs_t offset)
	{
		if (offset < 4)
			return m_isabus->dack_r(offset);
		else
			return m_isabus->dack16_r(offset);
	}
	void wd7600_iow(offs_t offset, u16 data)
	{
		if (offset < 4)
			m_isabus->dack_w(offset, data);
		else
			m_isabus->dack16_w(offset, data);
	}
	void wd7600_hold(int state)
	{
		// halt cpu
		m_x86cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

		// and acknowledge hold
		m_chipset->hlda_w(state);
	}
	void wd7600_tc(offs_t offset, u8 data) { m_isabus->eop_w(offset, data); }
	void wd7600_spkr(int state) { m_speaker->level_w(state); }

	u16 m_heartbeat = 0;

	static void romdisk_config(device_t *device);
};

void teradrive_state::at_softlists(machine_config &config)
{
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
//  SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
//  SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");

//  TODO: MD portion
//  TODO: Teradrive SW list
}

void teradrive_state::x86_map(address_map &map)
{
	map.unmap_value_high();
}

void teradrive_state::x86_io(address_map &map)
{
	map.unmap_value_high();
	// TODO: what's the origin of this?
	map(0xfc72, 0xfc73).lr16(
		NAME([this] () {
			u16 res = m_heartbeat & 0x8000;
			if (!machine().side_effects_disabled())
				m_heartbeat ^= 0x8000;
			// other bits read
			return 0x7fff | res;
		})
	);
}

//void teradrive_state::md_68k_map(address_map &map)
//{
//	map(0x000000, 0x7fffff).view(m_cart_view);
	// when /CART pin is low
//	m_cart_view[0](0x000000, 0x3fffff).m(m_cart, FUNC(...::cart_map));
//	m_cart_view[0](0x000000, 0x003fff).view(m_tmss_view);
//	m_tmss_view[0](0x000000, 0x003fff).rom().region("tmss", 0);
//	m_cart_view[0](0x400000, 0x7fffff).m(m_exp, FUNC(...::expansion_map));

	// /CART high (matters for MCD SRAM at very least)
//	m_cart_view[1](0x000000, 0x3fffff).m(m_exp, FUNC(...::expansion_map));
//	m_cart_view[1](0x400000, 0x7fffff).m(m_cart, FUNC(...::cart_map));
//	m_cart_view[1](0x400000, 0x403fff).view(m_tmss_view);
//	m_tmss_view[0](0x400000, 0x403fff).rom().region("tmss", 0);

//	map(0x800000, 0x9fffff) unmapped or 32X
//	map(0xa00000, 0xa07eff).mirror(0x8000) Z80 address space
//	map(0xa07f00, 0xa07fff).mirror(0x8000) Z80 VDP space (freezes machine if accessed from 68k)
//	map(0xa10000, 0xa100ff) I/O
//	map(0xa11000, 0xa110ff) memory mode register
//	map(0xa11100, 0xa111ff) Z80 BUSREQ/BUSACK
//	map(0xa11200, 0xa112ff) Z80 RESET
//	map(0xa11300, 0xa113ff) <open bus>

//	map(0xa11400, 0xa1dfff) <unmapped> (no DTACK generation, freezes machine without additional HW)
//	map(0xa12000, 0xa120ff).m(m_exp, FUNC(...::fdc_map));
//	map(0xa13000, 0xa130ff).m(m_cart, FUNC(...::time_map));
//	map(0xa14000, 0xa14003) TMSS lock
//	map(0xa15100, 0xa153ff) 32X registers if present, <unmapped> otherwise
//	map(0xae0000, 0xae0003) Teradrive bus switch registers
	// TODO: verify requiring swapped endianness
//	map(0xaf0000, 0xafffff).m(m_isabus, FUNC(isa16_device::io16_swap_r), FUNC(isa16_device::io16_swap_w));
	// NOTE: actually bank selectable from $ae0003 in 1 MiB units
//	map(0xb00000, 0xbfffff).m(m_isabus, FUNC(isa16_device::mem16_swap_r), FUNC(isa16_device::mem16_swap_w));
//	map(0xc00000, 0xdfffff) VDP and PSG (with mirrors and holes)
//	map(0xe00000, 0xffffff) Work RAM (with mirrors)
//}

void teradrive_state::machine_start()
{
}

void teradrive_isa_cards(device_slot_interface &device)
{
	device.option_add_internal("bus_switch", ISA16_IBM_79F2661);
}

void teradrive_state::romdisk_config(device_t *device)
{
	isa16_ibm_79f2661 &bus_switch = *downcast<isa16_ibm_79f2661 *>(device);
	bus_switch.set_romdisk_tag("romdisk");
}


void teradrive_state::teradrive(machine_config &config)
{
	I80286(config, m_x86cpu, XTAL(10'000'000));
	m_x86cpu->set_addrmap(AS_PROGRAM, &teradrive_state::x86_map);
	m_x86cpu->set_addrmap(AS_IO, &teradrive_state::x86_io);
	m_x86cpu->set_irq_acknowledge_callback("chipset", FUNC(wd7600_device::intack_cb));

	// WD76C10LP system controller
	// WD76C30 peripheral controller
	WD7600(config, m_chipset, 50_MHz_XTAL / 2);
	m_chipset->set_cputag(m_x86cpu);
	m_chipset->set_ramtag(m_ram);
	m_chipset->set_biostag("bios");
	m_chipset->set_keybctag("keybc");
	m_chipset->hold_callback().set(FUNC(teradrive_state::wd7600_hold));
	m_chipset->nmi_callback().set_inputline(m_x86cpu, INPUT_LINE_NMI);
	m_chipset->intr_callback().set_inputline(m_x86cpu, INPUT_LINE_IRQ0);
	m_chipset->cpureset_callback().set_inputline(m_x86cpu, INPUT_LINE_RESET);
	m_chipset->a20m_callback().set_inputline(m_x86cpu, INPUT_LINE_A20);
	// isa dma
	m_chipset->ior_callback().set(FUNC(teradrive_state::wd7600_ior));
	m_chipset->iow_callback().set(FUNC(teradrive_state::wd7600_iow));
	m_chipset->tc_callback().set(FUNC(teradrive_state::wd7600_tc));
	// speaker
	m_chipset->spkr_callback().set(FUNC(teradrive_state::wd7600_spkr));

	// on board devices
	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace(m_x86cpu, AS_PROGRAM);
	m_isabus->set_iospace(m_x86cpu, AS_IO);
	m_isabus->iochck_callback().set(m_chipset, FUNC(wd7600_device::iochck_w));
	m_isabus->irq2_callback().set(m_chipset, FUNC(wd7600_device::irq09_w));
	m_isabus->irq3_callback().set(m_chipset, FUNC(wd7600_device::irq03_w));
	m_isabus->irq4_callback().set(m_chipset, FUNC(wd7600_device::irq04_w));
	m_isabus->irq5_callback().set(m_chipset, FUNC(wd7600_device::irq05_w));
	m_isabus->irq6_callback().set(m_chipset, FUNC(wd7600_device::irq06_w));
	m_isabus->irq7_callback().set(m_chipset, FUNC(wd7600_device::irq07_w));
	m_isabus->irq10_callback().set(m_chipset, FUNC(wd7600_device::irq10_w));
	m_isabus->irq11_callback().set(m_chipset, FUNC(wd7600_device::irq11_w));
	m_isabus->irq12_callback().set(m_chipset, FUNC(wd7600_device::irq12_w));
	m_isabus->irq14_callback().set(m_chipset, FUNC(wd7600_device::irq14_w));
	m_isabus->irq15_callback().set(m_chipset, FUNC(wd7600_device::irq15_w));
	m_isabus->drq0_callback().set(m_chipset, FUNC(wd7600_device::dreq0_w));
	m_isabus->drq1_callback().set(m_chipset, FUNC(wd7600_device::dreq1_w));
	m_isabus->drq2_callback().set(m_chipset, FUNC(wd7600_device::dreq2_w));
	m_isabus->drq3_callback().set(m_chipset, FUNC(wd7600_device::dreq3_w));
	m_isabus->drq5_callback().set(m_chipset, FUNC(wd7600_device::dreq5_w));
	m_isabus->drq6_callback().set(m_chipset, FUNC(wd7600_device::dreq6_w));
	m_isabus->drq7_callback().set(m_chipset, FUNC(wd7600_device::dreq7_w));

	// NOTE: wants IBM BIOS over IBM keyboard only
	ps2_keyboard_controller_device &keybc(PS2_KEYBOARD_CONTROLLER(config, "keybc", 12_MHz_XTAL));
	keybc.hot_res().set("chipset", FUNC(wd7600_device::kbrst_w));
	keybc.gate_a20().set("chipset", FUNC(wd7600_device::gatea20_w));
	keybc.kbd_irq().set("chipset", FUNC(wd7600_device::irq01_w));
	keybc.kbd_clk().set("kbd", FUNC(pc_kbdc_device::clock_write_from_mb));
	keybc.kbd_data().set("kbd", FUNC(pc_kbdc_device::data_write_from_mb));

	pc_kbdc_device &pc_kbdc(PC_KBDC(config, "kbd", pc_at_keyboards, STR_KBD_IBM_PC_AT_84));
	pc_kbdc.out_clock_cb().set("keybc", FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	pc_kbdc.out_data_cb().set("keybc", FUNC(ps2_keyboard_controller_device::kbd_data_w));

	// FIXME: determine ISA bus clock, unverified configuration
	// WD76C20
	ISA16_SLOT(config, "board1", 0, "isabus", pc_isa16_cards, "fdcsmc", true);
	ISA16_SLOT(config, "board2", 0, "isabus", pc_isa16_cards, "comat", true);
	// TODO: should be ST-506 option, not IDE
	ISA16_SLOT(config, "board3", 0, "isabus", pc_isa16_cards, "side116", true);
	ISA16_SLOT(config, "board4", 0, "isabus", pc_isa16_cards, "lpt", true);
	// TODO: really WD90C10
	ISA16_SLOT(config, "board5", 0, "isabus", pc_isa16_cards, "wd90c11_lr", true);
	ISA16_SLOT(config, "board6", 0, "isabus", teradrive_isa_cards, "bus_switch", true).set_option_machine_config("bus_switch", romdisk_config);
	ISA16_SLOT(config, "isa1",   0, "isabus", pc_isa16_cards, nullptr, false);

	// 2.5MB is the max allowed by the BIOS (even if WD chipset can do more)
	RAM(config, RAM_TAG).set_default_size("1664K").set_extra_options("640K,2688K");

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);

	at_softlists(config);
}

ROM_START( teradrive )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD( "bios-27c010.bin", 0x00000, 0x20000, CRC(32642518) SHA1(6bb6d0325b8e4150c4258fd16f3a870b92e88f75))

	ROM_REGION16_LE(0x200000, "board6:romdisk", ROMREGION_ERASEFF)
	// contains bootable PC-DOS 3.x + a MENU.EXE
	ROM_LOAD( "tru-27c800.bin", 0x00000, 0x100000,  CRC(c2fe9c9e) SHA1(06ec0461dab425f41fb5c3892d9beaa8fa53bbf1))

	// MD 68k initial boot code, "TERA286 INITIALIZE" in header
	// shows Sega logo + TMSS "produced by" + 1990 copyright at bottom if loaded thru megadrij
	// + non-canonical accesses in $a***** range
	// TODO: it's actually in romdisk space at $43000, remove me
	ROM_REGION16_BE(0x4000, "tmss", ROMREGION_ERASEFF)
	ROM_LOAD( "tera_tmss.bin", 0x0000,  0x1000, CRC(424a9d11) SHA1(1c470a9a8d0b211c5feea1c1c2376aa1f7934b16) )
ROM_END

ROM_START( teradrive3 )
	// TODO: may just be a BIOS dump that needs to be trimmed
	ROM_REGION(0x80000, "rawbios", 0)
	ROM_LOAD( "model3.bin", 0x00000, 0x80000, CRC(dc757cb3) SHA1(c1489cc2d554fc62f986464604f7f7fbb219b438))

	ROM_REGION(0x20000, "bios", 0)
	ROM_COPY("rawbios", 0x60000, 0x00000, 0x20000)

	ROM_REGION16_LE(0x200000, "board6:romdisk", ROMREGION_ERASEFF)
	// contains bootable PC-DOS 3.x + a MENU.EXE
	ROM_LOAD( "tru-27c800.bin", 0x00000, 0x100000,  CRC(c2fe9c9e) SHA1(06ec0461dab425f41fb5c3892d9beaa8fa53bbf1))
ROM_END


} // anonymous namespace

COMP( 1991, teradrive,  0,         0,       teradrive, 0, teradrive_state, empty_init, "Sega / International Business Machines", "Teradrive (Japan, Model 2)", MACHINE_NOT_WORKING )
COMP( 1991, teradrive3, teradrive, 0,       teradrive, 0, teradrive_state, empty_init, "Sega / International Business Machines", "Teradrive (Japan, Model 3)", MACHINE_NOT_WORKING )
